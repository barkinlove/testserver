#include "server.hxx"
#include <arpa/inet.h>
#include <endian.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstddef>
#include <cstring>
#include <iostream>
#include "checksum.hxx"
#include "packet.hxx"

server::server(std::string_view ip, size_t port) {
  _socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  std::memset(&_self, 0, sizeof(_self));
  _self.sin_addr.s_addr = inet_addr(ip.data());
  _self.sin_port = htons(port);
  _self.sin_family = AF_INET;
  auto res =
      bind(_socket_desc, reinterpret_cast<sockaddr*>(&_self), sizeof(_self));
  // todo: error handling?
}

server::~server() {
  close(_socket_desc);
}

bool error_occured(ssize_t value) {
  if (value == -1) {
    perror("An error occured");
    return true;
  }
  return false;
}

void save(const std::byte* buffer) {}

bool skip_first = true;

void server::process_put(const packet& pack, size_t read_bytes) {
  auto id = pack.id;

  std::uint32_t seq_total = pack.seq_total;

  auto it = _transfers.find(id);
  if (it == _transfers.end()) {
    _transfers[id].data.resize(packet::max_data_size * seq_total);
    _transfers[id].total_parts = seq_total;
  }

  auto transfer_it = _transfers.find(id);
  auto& transfer = transfer_it->second;

  std::uint32_t part_idx = pack.seq_number - 1;
  size_t offset = packet::max_data_size * part_idx;

  auto* dest = _transfers[id].data.data() + offset;
  size_t payload_size = read_bytes - packet::header_size;

  // std::memmove ?
  std::memcpy(dest, pack.payload.data(), payload_size);

  std::cout << transfer.recieved_parts << '\n';

  transfer.recieved_parts += 1;

  packet ack_packet;
  if (transfer.recieved_parts == seq_total) {
    auto length = packet::max_data_size * (transfer.total_parts - 1);
    auto it = transfer.data.cbegin() + length;
    while (*it != std::byte{0}) {
      ++length;
      ++it;
    }
    auto crc32 = htonl(checksum::crc32(0, transfer.data.data(), length));
    std::cout << "checksum: " << ntohl(crc32) << '\n';

    ack_packet.type = packet::operation_t::ACK;
    ack_packet.seq_total = htonl(transfer.total_parts);
    std::memcpy(ack_packet.payload.data(), &crc32, sizeof(crc32));
    // save(ack_packet.payload);
    _transfers.erase(transfer_it);
  } else {
    ack_packet.type = packet::operation_t::ACK;
    ack_packet.seq_number = ntohl(pack.seq_number);
    ack_packet.id = htobe64(id);
  }
  std::cout << "sending to client acknowledgement\n";
  if (skip_first) {
    skip_first = false;
    return;
  }
  sendto(_socket_desc, &ack_packet, packet::max_data_size, 0,
         reinterpret_cast<sockaddr*>(&_client), sizeof(_client));
}

void server::process_packet(const packet& packet, size_t read_bytes) {
  auto type = packet.type;
  switch (type) {
    case packet::operation_t::ACK:
      break;
    case packet::operation_t::PUT:
      process_put(packet, read_bytes);
      break;
  }
}

void server::listen() {
  std::cout << "Starting listening on port: " << ntohs(_self.sin_port) << '\n';

  packet packet;

  socklen_t client_size = sizeof(_client);

  while (true) {
    ssize_t read_bytes =
        recvfrom(_socket_desc, &packet, sizeof(packet), 0,
                 reinterpret_cast<sockaddr*>(&_client), &client_size);
    std::cout << "recieved bytes: " << read_bytes << '\n';

    if (error_occured(read_bytes))
      continue;

    correct_endianness(packet);

    process_packet(packet, read_bytes);
  }
}
