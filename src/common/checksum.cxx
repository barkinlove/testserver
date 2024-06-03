#include "checksum.hxx"

constexpr auto polynomial = 0x82f63b78;

namespace checksum {

std::uint32_t crc32(std::uint32_t crc,
                    const std::byte* buffer,
                    std::size_t length) {
  int k;

  crc = ~crc;
  while (length--) {
    crc ^= std::to_integer<std::uint32_t>(*buffer++);
    for (k = 0; k < 8; k++)
      crc = crc & 1 ? (crc >> 1) ^ polynomial : crc >> 1;
  }
  return ~crc;
}

}  // namespace checksum
