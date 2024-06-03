#pragma once
#include <cstddef>
#include <cstdint>

namespace checksum {

std::uint32_t crc32(std::uint32_t crc,
                    const std::byte* buffer,
                    std::size_t length);

}  // namespace checksum
