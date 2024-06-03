#include "packet.hxx"
#include <netinet/in.h>
#include <cstring>

bool operator==(const packet::id_t& lhs, const packet::id_t& rhs) {
  for (size_t i = 0; i < lhs.data.size(); ++i) {
    if (lhs.data[i] != rhs.data[i])
      return false;
  }
  return true;
}

namespace {

// byte_idx: 5  4  3  2  1  0
//   number: aa bb cc dd ee ff
std::byte extract_byte(unsigned long long number, size_t byte_idx) {
  constexpr auto byte_mask = 0xff;
  constexpr auto byte_shift = 3;
  return std::byte{static_cast<std::underlying_type_t<std::byte>>(
      (number >> (byte_idx << byte_shift)) & byte_mask)};
}

}  // namespace

packet::id_t& packet::id_t::operator=(unsigned long long id) {
  std::memcpy(this, &id, sizeof(id));
  return *this;
}

packet::id_t::operator unsigned long long() const {
  return *reinterpret_cast<const unsigned long long*>(data.data());
}

packet::id_t::id_t(unsigned long long id) {
  std::memcpy(this, &id, sizeof(id));
}

void correct_endianness(packet& packet) {
  packet.seq_number = ntohl(packet.seq_number);
  packet.seq_total = ntohl(packet.seq_total);
  packet.id = be64toh(packet.id);
}
