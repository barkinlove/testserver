#pragma once
#include <netinet/in.h>
#include <string_view>
#include "packet.hxx"

class server {
 public:
  server(std::string_view ip, size_t port);
  ~server();

  void listen();
  void process_put(const packet& packet, size_t read_bytes);
  void process_packet(const packet& packet, size_t read_bytes);

 private:
  int _socket_desc = 0;
  sockaddr_in _self;
  sockaddr_in _client;

  struct transfer {
    size_t recieved_parts;
    size_t total_parts;
    std::vector<std::byte> data;
  };

  std::unordered_map<packet::id_t, transfer> _transfers;
};
