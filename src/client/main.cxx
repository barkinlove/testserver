#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <bits/types/struct_timeval.h>
#include <endian.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <unordered_map>
#include <vector>
#include "checksum.hxx"
#include "packet.hxx"

std::vector<std::byte> read_file(std::filesystem::path filepath) {
  std::ifstream source{filepath, std::ios::binary};
  auto size = std::filesystem::file_size(filepath);
  std::vector<std::byte> buffer{size};
  source.read((char*)buffer.data(), size);
  return buffer;
}

size_t compute_packet_nubmer(size_t raw_byte_number) {
  return raw_byte_number % packet::max_data_size == 0
             ? raw_byte_number / packet::max_data_size
             : raw_byte_number / packet::max_data_size + 1;
}

std::unordered_map<packet::id_t, size_t> file_parts;

std::vector<packet> create_packets(unsigned long long id,
                                   const std::vector<std::byte>& buffer) {
  std::vector<packet> packets;
  size_t packet_number = compute_packet_nubmer(buffer.size());
  file_parts.insert({id, packet_number});
  packets.resize(packet_number);

  auto buffer_it = buffer.cbegin();
  for (auto it = packets.begin(); it != packets.end(); ++it) {
    std::uint32_t seq_number = std::distance(packets.begin(), it) + 1;

    auto& packet = *it;
    packet.id = id;
    packet.seq_number = seq_number;
    packet.seq_total = packet_number;
    packet.type = packet::operation_t::PUT;

    for (auto& byte : packet.payload) {
      if (buffer_it == buffer.cend())
        break;
      byte = *buffer_it;
      ++buffer_it;
    }
  }

  return packets;
}

void host_to_network(packet& pack) {
  pack.id = htobe64(pack.id);
  pack.seq_number = htonl(pack.seq_number);
  pack.seq_total = htonl(pack.seq_total);
}

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    std::cout << "specify a file to transfer\n";
    std::cout << "usage: test_client \"filename.txt\"\n";
    return EXIT_FAILURE;
  }

  std::string filepath = argv[1];
  if (!std::filesystem::exists(filepath)) {
    std::cout << "specified file \"" << filepath << "\" does not exist\n";
    return EXIT_FAILURE;
  }

  const std::vector<std::byte> file_buffer = read_file(filepath);
  const auto checksum =
      checksum::crc32(0, file_buffer.data(), file_buffer.size());

  const std::vector<std::byte> file_buffer2{file_buffer.rbegin(),
                                            file_buffer.rend()};
  const auto checksum2 =
      checksum::crc32(0, file_buffer2.data(), file_buffer2.size());

  std::vector<packet> packets1 = create_packets(1, file_buffer);
  std::vector<packet> packets2 = create_packets(2, file_buffer2);

  std::vector<packet> packets{packets1.begin(), packets2.end()};
  packets.insert(packets.end(), packets2.begin(), packets2.end());

  // TODO: move packets endiannes transformation to a separate function
  std::cout << "file 1 checksum: " << checksum << '\n';
  std::cout << "file 2 checksum: " << checksum2 << '\n';

  int socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
  timeval time;
  std::memset(&time, 0, sizeof(time));
  time.tv_usec = 500000;
  setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time));

  sockaddr_in server;
  std::memset(&server, 0, sizeof(server));

  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons(1234);

  for (auto& packet : packets) {
    host_to_network(packet);
  }

  std::shuffle(packets.begin(), packets.end(),
               std::mt19937{std::random_device{}()});

  for (const auto& pack : packets) {
    sendto(socket_desc, &pack, sizeof(pack), 0,
           reinterpret_cast<sockaddr*>(&server), sizeof(server));
    packet ack_packet;
    socklen_t server_size = sizeof(server);
    auto n = recvfrom(socket_desc, &ack_packet, sizeof(ack_packet), 0,
                      reinterpret_cast<sockaddr*>(&server), &server_size);
    correct_endianness(ack_packet);

    auto it = file_parts.find(ack_packet.id);
    if (ack_packet.type == packet::operation_t::ACK && it != file_parts.end() &&
        ack_packet.seq_total == it->second) {
      std::uint32_t checksum;
      std::memcpy(&checksum, ack_packet.payload.data(), sizeof(checksum));
      std::cout << "recieved checksum: " << ntohl(checksum) << '\n';
    }
  }

  return EXIT_SUCCESS;
}
