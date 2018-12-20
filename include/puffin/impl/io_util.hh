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

} }

#endif //IO_UTIL_HH_INCLUDED_20181220
