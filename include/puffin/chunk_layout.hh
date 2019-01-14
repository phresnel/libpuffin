#ifndef CHUNK_LAYOUT_HH_INCLUDED_20190114
#define CHUNK_LAYOUT_HH_INCLUDED_20190114

#include <cstdint>

namespace puffin {

struct ChunkLayout {
        uint32_t chunk_width;
        uint32_t pixel_width;
        uint32_t pixels_per_chunk;
        uint32_t bytes_per_chunk;
        uint32_t pixel_mask;
        uint32_t nonsignificant_bits;
        bool     little_endian;

        ChunkLayout() {
                chunk_width = 0;
                pixel_width = 0;
                pixels_per_chunk = 0;
                bytes_per_chunk = 0;
                pixel_mask = 0;
                nonsignificant_bits = 0;
                little_endian = false;
        }

        ChunkLayout(uint32_t chunk_width, uint32_t pixel_width) {
                reset(chunk_width, pixel_width);
        }

        void reset(uint32_t chunk_width_, uint32_t pixel_width_) {
                chunk_width = chunk_width_;
                pixel_width = pixel_width_;
                pixels_per_chunk = chunk_width / pixel_width;
                bytes_per_chunk = chunk_width / 8;
                pixel_mask = uint32_t((uint64_t(1) << uint64_t(pixel_width)) - uint64_t(1));
                nonsignificant_bits = chunk_width % pixel_width;
                little_endian = false;//(pixel_width == 16) ? false : true;

                // https://www.fileformat.info/format/bmp/egff.htm ->
                //   16 bit + Win NT --> big endian
                //   16 bit + v4 BMP --> little endian
                //   32 bit + v4 BMP --> little endian
                //
        }


        // -- functions ------------------------------------------------
        uint32_t width_to_chunk_count(uint32_t x) const {
                // This adjusted division ensures that any result with
                // a non-zero fractional part is rounded up:
                //      width=64  ==>  c = (64 + 31) / 32 = 95 / 32 = 2
                //      width=65  ==>  c = (65 + 31) / 32 = 96 / 32 = 3
                return (x + pixels_per_chunk - 1U) / pixels_per_chunk;
        }

        uint32_t x_to_chunk_index(uint32_t x) const {
                return x / pixels_per_chunk;
        }

        uint32_t x_to_chunk_offset(uint32_t x) const {
                return x - x_to_chunk_index(x) * pixels_per_chunk;
        }

        template <typename ChunkT>
        ChunkT extract_value(ChunkT chunk, uint32_t ofs) const {
                const ChunkT
                        rshift = static_cast<ChunkT>(ofs * pixel_width),
                        value = (chunk >> rshift) & pixel_mask;
                return value;

                // Here's the code for [pixel_0, pixel_1, ..., pixel_n]:
                //
                // const uint32_t
                //        rshift = chunk_width-pixel_width - ofs*pixel_width,
                //        value = (chunk >> rshift) & pixel_mask;
                // return value;
        }

        template <typename ChunkT>
        ChunkT write_value(ChunkT chunk, uint32_t ofs, ChunkT val) const {
                const ChunkT
                        rshift = static_cast<uint32_t>(ofs * pixel_width),
                        renew_mask = (pixel_mask<<rshift),
                        keep_mask  = ~renew_mask,
                        bits_kept    = chunk         & keep_mask,
                        bits_renewed = (val<<rshift) & renew_mask,
                        new_chunk = bits_kept | bits_renewed;
                return new_chunk;
        }
};
inline
std::ostream& operator<< (std::ostream& os, ChunkLayout const &v) {
        return os << "ChunkLayout:\n"
                  << "  chunk_width:" << v.chunk_width << "\n"
                  << "  pixel_width:" << v.pixel_width << "\n"
                  << "  pixels_per_chunk:" << v.pixels_per_chunk << "\n"
                  << "  bytes_per_chunk:" << v.bytes_per_chunk << "\n"
                  << "  pixel_mask:" << std::bitset<32>(v.pixel_mask) << "\n"
                  << "  little_endian:" << (v.little_endian?"little":"big") << " endian\n"
                  << "  nonsignificant_bits:" << v.nonsignificant_bits << "\n";
}

}

#endif //CHUNK_LAYOUT_HH_INCLUDED_20190114
