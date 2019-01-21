//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Usage notes
// (you can find implementer's not at the bottom of this file).
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "puffin/bitmap.hh"
#include "puffin/exceptions.hh"
#include "puffin/experimental/bitfield.hh"
#include "puffin/rgba_bitmask.hh"
#include "puffin/chunk_layout.hh"

#include <fstream>
#include <iomanip>
#include <vector>

namespace puffin { namespace impl {

struct BitmapRowData {
        // -- types ----------------------------------------------------
        typedef uint32_t chunk_type;
        typedef std::vector<chunk_type> container_type;

        // -- members --------------------------------------------------
        BitmapRowData() : width_(0) {}

        BitmapRowData(BitmapInfoHeader const &infoHeader, std::istream &f) {
                reset(infoHeader, f);
        }

        void reset(
                BitmapInfoHeader const &infoHeader,
                std::istream &f
        ) {
                //std::cout << "BitmapRowData::reset()\n";

                width_ = infoHeader.width;
                layout_ = ChunkLayout(
                        infoHeader.bitsPerPixel > 8 ? infoHeader.bitsPerPixel : 8,
                        infoHeader.bitsPerPixel
                );
                chunks_.clear();

                const std::ostream::pos_type startPos = f.tellg();

                const uint32_t numChunks = layout_.width_to_chunk_count(infoHeader.width);
                if (!(infoHeader.compression == BI_RGB ||
                      infoHeader.compression == BI_BITFIELDS)) {
                        chunks_.resize(numChunks);
                        return;
                }

                // std::cout << " chunk-layout:" << layout_ << "\n";

                chunks_.reserve(numChunks);
                for (uint32_t i = 0U; i != numChunks; ++i) {
                        const chunk_type chunk_raw =
                                layout_.little_endian ?
                                read_bytes_to_uint32_le(f, layout_.bytes_per_chunk) :
                                read_bytes_to_uint32_be(f, layout_.bytes_per_chunk);
                        const chunk_type chunk = chunk_raw >> layout_.nonsignificant_bits;

                        // This flips the pixel order when there are multiple
                        // pixels per chunk (only the case in 1..8 bit bmp's)
                        const uint32_t flipped = flip_endianness_uint32(
                                static_cast<uint32_t>(layout_.pixel_width),
                                static_cast<uint32_t>(layout_.chunk_width),
                                static_cast<uint32_t>(chunk)
                        );
                        chunks_.push_back(flipped);
                }

                const std::ostream::pos_type
                        endPos = f.tellg(),
                        bytes_read = endPos - startPos,
                        next_mul4 = 4U * ((bytes_read + std::ostream::pos_type(3U)) / 4U),
                        pad_bytes = next_mul4 - bytes_read;
                f.ignore(pad_bytes);
        }

        uint32_t get32(int x) const {
                const uint32_t
                        chunk_index = layout_.x_to_chunk_index(x),
                        chunk_ofs = layout_.x_to_chunk_offset(x),
                        chunk = chunks_[chunk_index],
                        value = layout_.extract_value(chunk, chunk_ofs);
                return value;
        }

        void set32(int x, uint32_t value) {
                const uint32_t
                        chunk_index = layout_.x_to_chunk_index(x),
                        chunk_ofs = layout_.x_to_chunk_offset(x),
                        chunk_old = chunks_[chunk_index],
                        chunk_new = layout_.write_value(chunk_old, chunk_ofs, value);
                chunks_[chunk_index] = chunk_new;
        }

private:
        // -- data -----------------------------------------------------
        ChunkLayout layout_;

        uint32_t width_;
        container_type chunks_;
};

} }
