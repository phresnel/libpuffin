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

struct BitmapImageData {
        // -- types ----------------------------------------------------
        typedef BitmapRowData row_type;
        typedef std::vector<row_type> container_type;
        typedef typename container_type::size_type size_type;

        // -- members --------------------------------------------------
        BitmapImageData() : width_(0) {}

        BitmapImageData(
                BitmapHeader const &header,
                BitmapInfoHeader const &infoHeader,
                std::istream &f
        ) {
                reset(header, infoHeader, f);
        }

        void reset(
                BitmapHeader const &header,
                BitmapInfoHeader const &infoHeader,
                std::istream &f
        ) {
                f.seekg(header.dataOffset);
                loadUncompressed(infoHeader, f);
                loadRLE(infoHeader, f);
                width_ = infoHeader.width;

        }

        bool empty() const {
                return rows_.size() == 0;
        }

        size_type width() const {
                return width_;
        }

        size_type height() const {
                return rows_.size();
        }

        uint32_t get32(int x, int y) const {
                return rows_[y].get32(x);
        }

        void set32(int x, int y, uint32_t val) {
                //std::cout << val << " => " << std::bitset<32>(get32(x, y)) << " --> ";
                rows_[y].set32(x, val);
                //std::cout << std::bitset<32>(get32(x, y)) << std::endl;
        }

private:
        container_type rows_;
        size_type width_;

        void clear_and_shrink(int newSize = 0) {
                rows_.clear();
                //  Explicit typing here to prevent incompatible container_type
                // constructor:
                std::vector<row_type> tmp(newSize);
                tmp.swap(rows_);
        }

        void loadUncompressed(BitmapInfoHeader const &infoHeader, std::istream &f) {
                if (infoHeader.isBottomUp) {
                        // Least evil approach: Fill from behind while keeping
                        // the advantages of std::vector and not needing a
                        // temporary container object:
                        clear_and_shrink(infoHeader.height);
                        for (int y = infoHeader.height - 1; y >= 0; --y) {
                                rows_[y].reset(infoHeader, f);
                        }
                } else {
                        // Can construct upon reading:
                        clear_and_shrink();
                        rows_.reserve(infoHeader.height);
                        for (int y = 0; y != infoHeader.height; ++y) {
                                rows_.emplace_back(infoHeader, f);
                        }
                }
        }

        void loadRLE(BitmapInfoHeader const &infoHeader, std::istream &f) {
                if (infoHeader.compression != BI_RLE4 &&
                    infoHeader.compression != BI_RLE8)
                        return;

                const std::ostream::pos_type startPos = f.tellg();
                const ChunkLayout ch(
                        8,
                        infoHeader.compression == BI_RLE4 ? 4 : 8
                );

                int x = 0;
                int y = 0;

                while (true) {
                        const std::ostream::pos_type runStartPos = f.tellg();
                        const uint8_t
                                first = read_uint8_le(f),
                                second = read_uint8_le(f);

                        //std::cout << "[" << (unsigned)first << ":" << (unsigned)second << "]: ";

                        if (first == 0 && second == 0) {
                                x = 0;
                                ++y;
                                //std::cout << "end of line (x=" << x << ", y=" << y << ")\n";
                        } else if (first == 0 && second == 1) {
                                //std::cout << "end of bitmap (x=" << x << ", y=" << y << ")\n";
                                goto done;
                        } else if (first == 0 && second == 2) {
                                const uint8_t x_rel = read_uint8_le(f),
                                        y_rel = read_uint8_le(f);
                                //std::cout << "delta (x_rel=" << x_rel << ", y_rel=" << y_rel << ")\n";
                                x += x_rel;
                                y += y_rel;
                        } else if (first == 0) {
                                // absolute mode
                                const uint8_t numPixels = second;

                                //std::cout << "absolute mode (numPixels: " << (unsigned) numPixels << ")\n";
                                for (uint32_t i = 0; i < numPixels; i += ch.pixels_per_chunk) {
                                        const uint8_t chunk_raw = read_uint8_le(f);
                                        const uint8_t chunk = flip_endianness_uint8(
                                                static_cast<uint8_t>(ch.pixel_width),
                                                static_cast<uint8_t>(chunk_raw)
                                        );

                                        const uint32_t
                                                left = (numPixels - i),
                                                len = left < ch.pixels_per_chunk ? left : ch.pixels_per_chunk;
                                        for (uint32_t o = 0; o != len; ++o) {
                                                const uint8_t v = ch.extract_value(chunk, o);
                                                //std::cout << "    [" << (i+o) << "] set32(" << x << ", " << y << ", " << (unsigned) v << ")\n";
                                                if (infoHeader.isBottomUp) {
                                                        set32(x, (infoHeader.height - 1) - y, v);
                                                } else {
                                                        set32(x, y, v);
                                                }
                                                ++x;
                                        }
                                }

                                // Pad to 16 bit boundary:
                                const std::ostream::pos_type
                                        curr = f.tellg(),
                                        next_mul2 = 2U * ((curr + std::ostream::pos_type(1U)) / 2U),
                                        pad_bytes = next_mul2 - curr;
                                f.ignore(pad_bytes);
                                //std::cout << " padding by " << pad_bytes << " to " << f.tellg() << "\n";
                        } else {
                                // encoded mode
                                const uint8_t numPixels = first;
                                const uint8_t chunk = flip_endianness_uint8(
                                        static_cast<uint8_t>(ch.pixel_width),
                                        static_cast<uint8_t>(second)
                                );
                                //std::cout << "encoded mode (numPixels: " << (unsigned)numPixels << ", values: ";
                                //for (uint32_t o = 0; o != ch.pixels_per_chunk; ++o) {
                                //        const uint8_t v = ch.extract_value(chunk, o);
                                //        if (o) std::cout << "|";
                                //        std::cout << (unsigned)v;
                                //}
                                //std::cout << ")\n";

                                for (uint32_t i = 0; i < numPixels; i += ch.pixels_per_chunk) {
                                        const uint32_t
                                                left = (numPixels - i),
                                                len = left < ch.pixels_per_chunk ? left : ch.pixels_per_chunk;
                                        for (uint32_t o = 0; o != len; ++o) {
                                                const uint8_t v = ch.extract_value(chunk, o);
                                                //std::cout << "    [" << (i+o) << "] set32(" << x << ", " << y << ", " << (unsigned) v << ")\n";
                                                if (infoHeader.isBottomUp) {
                                                        set32(x, (infoHeader.height - 1) - y, v);
                                                } else {
                                                        set32(x, y, v);
                                                }
                                                ++x;
                                        }
                                }
                        }

                        /*
                        // Pad to 16 bit boundary:
                        const std::ostream::pos_type
                                runEndPos = f.tellg(),
                                bytes_read = runEndPos - runStartPos,
                                next_mul4 = 4U*((bytes_read + std::ostream::pos_type(3U))/4U),
                                pad_bytes = next_mul4 - bytes_read;
                        f.ignore(pad_bytes);
                        std::cout << " padding by " << pad_bytes << " to " << f.tellg() << "\n";
                        // TODO: Is it valid to just use f.tellg() for alignment?
                         */
                }
                done:
                const std::ostream::pos_type endPos = f.tellg();
        }
};

inline
std::ostream &operator<<(std::ostream &os, BitmapImageData const &) {
        return os << "BitmapImageData(...)";
        /*
        if (data.empty())
                return os;
        os << type_str(data) << " {\n";
        for (int y=0; y!=data.height(); ++y) {
                os << " [";
                for (int x=0; x!=data.width(); ++x) {
                        if (x) {
                                os << ", ";
                        }
                        os << data(x,y);
                }
                os << "]\n";
        }
        os << "}\n";
        return os;
        */
}

} }
