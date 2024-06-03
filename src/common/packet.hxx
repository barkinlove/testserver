#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

#pragma pack(push, 1)
struct packet {
  static constexpr std::size_t max_size = 1472;

  enum class operation_t : std::uint8_t { ACK = 0, PUT = 1 };

  std::uint32_t seq_number;
  std::uint32_t seq_total;
  operation_t type;

  class id_t {
   public:
    id_t() = default;
    id_t(unsigned long long);
    id_t& operator=(unsigned long long id);
    operator unsigned long long() const;

   private:
    template <typename T>
    friend struct std::hash;

    friend bool operator==(const id_t& lhs, const id_t& rhs);

    std::array<std::byte, 8> data;
  };
  static_assert(sizeof(id_t) == 8);

  id_t id;

  static constexpr size_t header_size =
      sizeof(seq_number) + sizeof(seq_total) + sizeof(type) + sizeof(id);

  static constexpr size_t max_data_size = max_size - header_size;

  std::array<std::byte, max_data_size> payload;
};
#pragma pack(pop)

static_assert(sizeof(packet) == packet::max_size);

bool operator==(const packet::id_t& lhs, const packet::id_t& rhs);

template <>
struct std::hash<packet::id_t> {
  size_t operator()(const packet::id_t& id) const {
    size_t hash = 0, size = id.data.size();
    for (size_t i = 0; i < size; ++i) {
      hash += std::hash<std::byte>{}(id.data[i]);
    }
    return hash;
  }
};

void correct_endianness(packet& packet);
