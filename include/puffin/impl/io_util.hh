#ifndef IO_UTIL_HH_INCLUDED_20181220
#define IO_UTIL_HH_INCLUDED_20181220

#include <iostream>

namespace puffin { namespace impl {

// Little endian helpers:
inline uint8_t read_uint8_le(std::istream &f) {
        return static_cast<uint8_t>(f.get());
}
inline uint16_t read_uint16_le(std::istream &f) {
        return read_uint8_le(f) |
               static_cast<uint16_t>(read_uint8_le(f)) << 8;
}
inline uint32_t read_uint32_le(std::istream &f) {
        return read_uint16_le(f) |
               static_cast<uint32_t>(read_uint16_le(f)) << 16;
}
inline uint64_t read_uint64_le(std::istream &f) {
        return read_uint32_le(f) |
               static_cast<uint64_t>(read_uint32_le(f)) << 32;
}
inline int32_t read_int32_le(std::istream &f) {
        return static_cast<int32_t>(read_uint32_le(f));
}

inline uint64_t read_bytes_to_uint64_le(std::istream &f, int num_bytes) {
        uint64_t bytes = 0;
        for (int i=0; i!=num_bytes; ++i) {
                const uint64_t byte = read_uint8_le(f);
                bytes = (bytes<<8) | byte;
        }
        return bytes;
}
inline uint32_t read_bytes_to_uint32_le(std::istream &f, int num_bytes) {
        return static_cast<uint32_t>(read_bytes_to_uint64_le(f, num_bytes));
}

// Big endian helpers:
inline uint8_t read_uint8_be(std::istream &f) {
        return static_cast<uint8_t>(f.get());
}
inline uint16_t read_uint16_be(std::istream &f) {
        return static_cast<uint16_t>(read_uint8_be(f)) << 8 |
               read_uint8_be(f);
}
inline uint32_t read_uint32_be(std::istream &f) {
        return static_cast<uint32_t>(read_uint16_be(f)) << 16 |
               read_uint16_be(f);
}
inline uint64_t read_uint64_be(std::istream &f) {
        return static_cast<uint64_t>(read_uint32_be(f)) << 32 |
               read_uint32_be(f);
}

inline uint64_t read_bytes_to_uint64_be(std::istream &f, int num_bytes) {
        uint64_t bytes = 0;
        for (int i=0; i!=num_bytes; ++i) {
                const uint64_t byte = read_uint8_be(f);
                bytes |= byte << (8*i);
        }
        return bytes;
}
inline uint32_t read_bytes_to_uint32_be(std::istream &f, int num_bytes) {
        return static_cast<uint32_t>(read_bytes_to_uint64_be(f, num_bytes));
}

inline
uint8_t extract_value_uint8(
        uint8_t bits_per_value,
        uint8_t chunk,
        uint8_t ofs
) {
        const uint8_t
                chunk_width = 8,
                values_per_chunk = chunk_width / bits_per_value,
                rshift = ofs * bits_per_value,
                value_mask = (1U << bits_per_value) - 1U,
                value = (chunk >> rshift) & value_mask;
        return value;
}

inline
uint32_t extract_value_uint32(
        uint32_t bits_per_value,
        uint32_t chunk,
        uint32_t ofs
) {
        const uint32_t
                chunk_width = 32,
                values_per_chunk = chunk_width / bits_per_value,
                rshift = ofs * bits_per_value,
                value_mask = (1U << bits_per_value) - 1U,
                value = (chunk >> rshift) & value_mask;
        return value;
}

inline
uint8_t flip_endianness_uint8(
        uint8_t bits_per_value,
        uint8_t chunk
) {
        const uint8_t
                chunk_width = 8,
                values_per_chunk = chunk_width / bits_per_value
        ;
        uint8_t flipped = 0U;
        for (uint8_t j = 0U; j != values_per_chunk; ++j) {
                const uint8_t value =
                        extract_value_uint8(bits_per_value, chunk, j);
                flipped = (flipped << bits_per_value) | value;
        }
        return flipped;
}

inline
uint32_t flip_endianness_uint32(
        uint32_t value_width,
        uint32_t chunk
) {
        const uint32_t
                chunk_width = 32,
                values_per_chunk = chunk_width / value_width
        ;
        uint32_t flipped = 0U;
        for (uint32_t j = 0U; j != values_per_chunk; ++j) {
                const uint32_t value =
                        extract_value_uint32(value_width, chunk, j);
                flipped = (flipped << value_width) | value;
        }
        return flipped;
}

inline
uint32_t flip_endianness_uint32(
        uint32_t value_width,
        uint32_t chunk_width,
        uint32_t chunk
) {
        const uint32_t
                values_per_chunk = chunk_width / value_width
        ;
        uint32_t flipped = 0U;
        for (uint32_t j = 0U; j != values_per_chunk; ++j) {
                const uint32_t value =
                        extract_value_uint32(value_width, chunk, j);
                flipped = (flipped << value_width) | value;
        }
        return flipped;
}

inline
uint32_t any_bit_set_uint32(uint32_t value) {
        return value != 0;
}

inline
uint32_t no_bit_set_uint32(uint32_t value) {
        return value == 0;
}

inline
uint32_t first_bit_set_uint32(uint32_t value) {
        if (no_bit_set_uint32(value))
                return 0;

        uint32_t ret = 0;
        while(((value>>ret)&1) == 0)
                ++ret;
        return ret;
}

inline
uint32_t last_bit_set_uint32(uint32_t value) {
        if (no_bit_set_uint32(value))
                return 0;

        uint32_t ret = 31;
        while(((value>>ret)&1) == 0)
                --ret;
        return ret;
}

} }

#endif //IO_UTIL_HH_INCLUDED_20181220
