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


enum BitmapCompression {
        BI_RGB = 0x0000,
        BI_RLE8 = 0x0001,
        BI_RLE4 = 0x0002,
        BI_BITFIELDS = 0x0003,
        BI_JPEG = 0x0004,
        BI_PNG = 0x0005,
        BI_CMYK = 0x000B,
        BI_CMYKRLE8 = 0x000C,
        BI_CMYKRLE4 = 0x000D
};
inline
std::string to_string(BitmapCompression bc) {
        switch (bc) {
        case BI_RGB: return "BI_RGB";
        case BI_RLE8: return "BI_RLE8";
        case BI_RLE4: return "BI_RLE4";
        case BI_BITFIELDS: return "BI_BITFIELDS";
        case BI_JPEG: return "BI_JPEG";
        case BI_PNG: return "BI_PNG";
        case BI_CMYK: return "BI_CMYK";
        case BI_CMYKRLE8: return "BI_CMYKRLE8";
        case BI_CMYKRLE4: return "BI_CMYKRLE4";
        default: return "<unknown>";
        }
}
inline
std::ostream& operator<< (std::ostream &os, BitmapCompression bc) {
        return os << to_string(bc);
}


std::set<BitmapVersion> determineBitmapVersion(std::istream &f) {
        // "The FileType field of the file header is where we start. If
        //  these two byte values are 424Dh ("BM"), then you have a
        //  single-image BMP file that may have been created under
        //  Windows or OS/2. If FileType is the value 4142h ("BA"), then
        //  you have an OS/2 bitmap array file. Other OS/2 BMP
        //  variations have the file extensions .ICO and .PTR.
        //
        //  If your file type is "BM", then you must now read the Size
        //  field of the bitmap header to determine the version of the
        //  file. Size will be 12 for Windows 2.x BMP and OS/2 1.x BMP,
        //  40 for Windows 3.x and Windows NT BMP, 12 to 64 for OS/2 2.x
        //  BMP, and 108 for Windows 4.x BMP. A Windows NT BMP file will
        //  always have a Compression value of 3; otherwise, it is read
        //  as a Windows 3.x BMP file.
        //
        //  Note that the only difference between Windows 2.x BMP and
        //  OS/2 1.x BMP is the data type of the Width and Height
        //  fields. For Windows 2.x, they are signed shorts and for
        //  OS/2 1.x, they are unsigned shorts. Windows 3.x, Windows NT,
        //  and OS/2 2.x BMP files only vary in the number of fields in
        //  the bitmap header and in the interpretation of the
        //  Compression field."

        const std::ostream::pos_type startPos = f.tellg();
        f.seekg(0, std::ios_base::beg);

        std::set<BitmapVersion> ret;

        const uint16_t signature = impl::read_uint16_be(f);

        switch (signature) {
        // ---------------------------------------------------------------------
        case 0x0000: {
                ret.insert(BMPv_Win_1x);
                break;
        }
        // -- 'BM' -------------------------------------------------------------
        case 0x424d: {
                f.seekg(14, std::ios_base::beg);
                const uint32_t infoHeaderSize = impl::read_uint32_le(f);

                switch (infoHeaderSize) {
                // -- [Windows 2.x] or [OS/2 1.x]   --  --  --  --  --  --  --
                case 12: {
                        // signed width/height for Win 2.x, unsigned for OS2 1.x
                        ret.insert(BMPv_Win_2x);
                        ret.insert(BMPv_OS2_1x);
                        break;
                }
                // -- [Windows 3.x] or [Windows NT] --  --  --  --  --  --  --
                case 40: {
                        f.seekg(12, std::ios_base::cur);
                        const uint32_t compression =
                                static_cast<BitmapCompression>(
                                        impl::read_uint32_le(f));
                        ret.insert(BMPv_Win_3x);
                        // It can only be NT if compression is BITFIELDS:
                        if (compression == BI_BITFIELDS)
                                ret.insert(BMPv_WinNT);
                        break;
                }
                // -- [Windows 4.x] --  --  --  --  --  --  --  --  --  --  --
                case 108: {
                        //
                        ret.insert(BMPv_Win_4x);
                        break;
                }
                // -- [OS/2 2.x] -  --  --  --  --  --  --  --  --  --  --  --
                default:
                        if (infoHeaderSize >= 12 && infoHeaderSize <= 64) {
                                ret.insert(BMPv_OS2_2x);
                        }
                        break;
                };
                break;
        }
        // -- 'BA' -------------------------------------------------------------
        case 0x4241: {
                // OS/2 Bitmap Array
                throw std::logic_error("not implemented");
                break;
        }
        };
        if (ret.size() == 0)
                ret.insert(BMPv_Unknown);
        f.seekg(startPos);
        return ret;
}


// Corresponds to BITMAPFILEHEADER on Windows.
// See https://docs.microsoft.com/en-us/windows/desktop/api/wingdi/ns-wingdi-tagbitmapfileheader
struct BitmapHeader {
        uint16_t signature;   // 'BM'
        uint32_t size;        // Size in bytes.
        uint16_t reserved1;
        uint16_t reserved2;
        uint32_t dataOffset;  // Offset from beginning of header to pixel data.

        BitmapHeader() :
                signature(0),
                size(0),
                reserved1(0),
                reserved2(0),
                dataOffset(0) {
        }

        BitmapHeader(std::istream &f) {
                reset(f);
        }

        void reset(std::istream &f) {
                signature = impl::read_uint16_be(f);
                size = impl::read_uint32_le(f);
                reserved1 = impl::read_uint16_le(f);
                reserved2 = impl::read_uint16_le(f);
                dataOffset = impl::read_uint32_le(f);
        }

        enum { size_in_file = (16 + 32 + 16 + 16 + 32)/8 };
};
inline
std::ostream& operator<< (std::ostream &os, BitmapHeader const &v) {
        return os << "Header{\n"
                  << "  signature:" << std::hex << v.signature << std::dec
                  << " ('"
                  << static_cast<char>(v.signature>>8)
                  << static_cast<char>(v.signature & 0xff)
                  << "')\n"
                  << "  size:" << v.size << "\n"
                  << "  reserved1:" << v.reserved1 << "\n"
                  << "  reserved2:" << v.reserved2 << "\n"
                  << "  dataOffset:" << v.dataOffset << "\n"
                  << "}\n";
}



// Corresponds to BITMAPINFOHEADER on Windows.
// See https://docs.microsoft.com/en-us/previous-versions/dd183376(v%3Dvs.85)
struct BitmapInfoHeader {
        // read from file
        uint32_t infoHeaderSize;
        uint32_t width;
        int32_t height;
        uint16_t planes;
        uint16_t bitsPerPixel;
        BitmapCompression compression;
        uint32_t compressedImageSize; // 0 if compression is 0
        uint32_t xPixelsPerMeter;
        uint32_t yPixelsPerMeter;
        uint32_t colorsUsed; // Actually used colors (256 for 8 bit, 0=max number of colors according to bitsPerPixel)
        uint32_t importantColors; // Number of colors required for display. 0=max.

        // computed fields
        bool isBottomUp;

        BitmapInfoHeader() :
                infoHeaderSize(0),
                width(0),
                height(0),
                planes(0),
                bitsPerPixel(0),
                compression(BI_RGB),
                compressedImageSize(0),
                xPixelsPerMeter(0),
                yPixelsPerMeter(0),
                colorsUsed(0),
                importantColors(0),
                isBottomUp(0) {}

        BitmapInfoHeader(std::set<BitmapVersion> const &v, std::istream &f) {
                reset(v, f);
        }

        void reset() {
                using std::swap;
                BitmapInfoHeader tmp;
                swap(*this, tmp);
        }

        void reset(std::set<BitmapVersion> const &v, std::istream &f) {
                reset();
                const bool is_win2 = v.find(BMPv_OS2_1x) != v.end()
                                  || v.find(BMPv_Win_2x) != v.end();
                if (is_win2) {
                        infoHeaderSize = impl::read_uint32_le(f);
                        width = impl::read_uint16_le(f);
                        height = impl::read_int16_le(f);
                        planes = impl::read_uint16_le(f);
                        bitsPerPixel = impl::read_uint16_le(f);
                } else {
                        infoHeaderSize = impl::read_uint32_le(f);
                        width = impl::read_uint32_le(f);
                        height = impl::read_int32_le(f);
                        planes = impl::read_uint16_le(f);
                        bitsPerPixel = impl::read_uint16_le(f);
                        compression = static_cast<BitmapCompression>(impl::read_uint32_le(f));
                        compressedImageSize = impl::read_uint32_le(f);
                        xPixelsPerMeter = impl::read_uint32_le(f);
                        yPixelsPerMeter = impl::read_uint32_le(f);
                        colorsUsed = impl::read_uint32_le(f);
                        importantColors = impl::read_uint32_le(f);
                }

                if (height >= 0) {
                        isBottomUp = true;
                } else {
                        height = -height;
                        isBottomUp = false;
                }
        }
};
inline
std::ostream& operator<< (std::ostream &os, BitmapInfoHeader const &v) {
        return os << "InfoHeader{\n"
                  << "  infoHeaderSize:" << v.infoHeaderSize << "\n"
                  << "  width:" << v.width << "\n"
                  << "  height:" << v.height << "\n"
                  << "  isBottomUp:" << v.isBottomUp << "\n"
                  << "  planes:" << v.planes << "\n"
                  << "  bitsPerPixel:" << v.bitsPerPixel << "\n"
                  << "  compression:" << v.compression << "\n"
                  << "  compressedImageSize:" << v.compressedImageSize << "\n"
                  << "  xPixelsPerMeter:" << v.xPixelsPerMeter << "\n"
                  << "  yPixelsPerMeter:" << v.yPixelsPerMeter << "\n"
                  << "  colorsUsed:" << v.colorsUsed << "\n"
                  << "  importantColors:" << v.importantColors << "\n"
                  << "}\n";
}



struct BitmapColorTable {
        BitmapColorTable() {}

        BitmapColorTable(
                BitmapHeader const &header,
                BitmapInfoHeader const &infoHeader,
                std::set<BitmapVersion> const &v,
                std::istream &f
        ) {
                reset(header, infoHeader, v, f);
        }

        void reset(
                BitmapHeader const &header,
                BitmapInfoHeader const &infoHeader,
                std::set<BitmapVersion> const &v,
                std::istream &f
        ) {
                entries_ = readEntries(header, infoHeader, v, f);
        }

        Color32 operator[](int i) const {
                return entries_[i];
        }

        Color32 at(int i) const {
                return entries_.at(i);
        }

        std::vector<Color32>::size_type size() const {
                return entries_.size();
        }

        bool empty() const {
                return entries_.empty();
        }

private:
        std::vector<Color32> entries_;

        static std::vector<Color32> readEntries(
                BitmapHeader const &header,
                BitmapInfoHeader const &infoHeader,
                std::set<BitmapVersion> const &v,
                std::istream &f
        ) {
                const uint32_t size = computeSize(header, infoHeader, v);
                const bool fourChannel = isFourChannel(v);

                std::vector<Color32> ret;
                ret.reserve(size);
                for (uint32_t i = 0; i < size; ++i) {
                        const uint8_t blue = impl::read_uint8_le(f),
                                      green = impl::read_uint8_le(f),
                                      red = impl::read_uint8_le(f);
                        if (fourChannel) {
                                impl::read_uint8_le(f); // reserved
                        }
                        ret.push_back(Color32(red, green, blue));
                }
                return ret;
        }

        static bool isFourChannel(std::set<BitmapVersion> const &v) {
                const bool not_win_2 = v.find(BMPv_Win_2x) == v.end(),
                           not_os2_1 = v.find(BMPv_OS2_1x) == v.end();
                return not_win_2 && not_os2_1;

        }

        static uint32_t computeSize(
                BitmapHeader const &header,
                BitmapInfoHeader const &infoHeader,
                std::set<BitmapVersion> const &v
        ) {
                /*
                const uint32_t decl =
                        infoHeader.colorsUsed != 0 ? infoHeader.colorsUsed :
                        infoHeader.bitsPerPixel == 1 ? 2 :
                        infoHeader.bitsPerPixel == 2 ? 4 :
                        infoHeader.bitsPerPixel == 4 ? 16 :
                        infoHeader.bitsPerPixel == 8 ? 256 : 0;
                */

                const uint32_t
                        headersSize = BitmapHeader::size_in_file +
                                      infoHeader.infoHeaderSize,
                        tableSize = header.dataOffset - headersSize,
                        elemSize = isFourChannel(v) ? 4 : 3;
                return tableSize / elemSize;
        }
};
inline
std::ostream& operator<< (std::ostream &os, Color32 const &v) {
        return os << "[" << std::setw(3) << (unsigned int)v.r()
                  << "|" << std::setw(3) << (unsigned int)v.g()
                  << "|" << std::setw(3) << (unsigned int)v.b()
                  << "]";
}
inline
std::ostream& operator<< (std::ostream &os, BitmapColorTable const &v) {
        os << "ColorTable{\n";

        //for (int i=0; i!=v.size(); ++i) {
        //        os << "  [" << std::setw(3) << i << "]:" << v[i] << "\n";
        //}
        os << "  size:" << v.size() << "\n";
        os << "}\n";
        return os;
}



struct BitmapColorMasks {
        uint32_t red;
        uint32_t green;
        uint32_t blue;

        BitmapColorMasks() {
                reset();
        }

        BitmapColorMasks(BitmapInfoHeader const &info, std::istream &f) {
                reset(info, f);
        }

        void reset() {
                blue = green = red = 0;
        }

        void reset(BitmapInfoHeader const &info, std::istream &f) {
                reset();

                if (info.compression != BitmapCompression::BI_BITFIELDS)
                        return;
                red = impl::read_uint32_le(f);
                green = impl::read_uint32_le(f);
                blue = impl::read_uint32_le(f);
        }
};
inline
std::ostream& operator<< (std::ostream &os, BitmapColorMasks const &v) {
        return os << "ColorMask{\n"
                  << "  red..:" << std::bitset<32>(v.red) << " / 0x"
                                << std::hex << v.red << "\n"
                  << "  green:" << std::bitset<32>(v.green) << " / 0x"
                                << std::hex << v.green << "\n"
                  << "  blue.:" << std::bitset<32>(v.blue) << " / 0x"
                                << std::hex << v.blue << "\n"
                                << std::dec
                  << "}\n";
}

struct BitmapRowData {
        // -- types ----------------------------------------------------
        typedef uint32_t chunk_type;
        typedef std::vector<chunk_type> container_type;

        // -- members --------------------------------------------------
        BitmapRowData() : width_(0) { }

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
                      infoHeader.compression == BI_BITFIELDS))
                {
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
                        next_mul4 = 4U*((bytes_read + std::ostream::pos_type(3U))/4U),
                        pad_bytes = next_mul4 - bytes_read;
                f.ignore(pad_bytes);
        }

        uint32_t get32 (int x) const {
                const uint32_t
                        chunk_index = layout_.x_to_chunk_index(x),
                        chunk_ofs   = layout_.x_to_chunk_offset(x),
                        chunk = chunks_[chunk_index],
                        value = layout_.extract_value(chunk, chunk_ofs);
                return value;
        }

        void set32 (int x, uint32_t value) {
                const uint32_t
                        chunk_index = layout_.x_to_chunk_index(x),
                        chunk_ofs   = layout_.x_to_chunk_offset(x),
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
        size_type width_ ;

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
                        for (int y=infoHeader.height-1; y>=0; --y) {
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

                while(true) {
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
                                                        set32(x, (infoHeader.height-1) - y, v);
                                                } else {
                                                        set32(x, y, v);
                                                }
                                                ++x;
                                        }
                                }

                                // Pad to 16 bit boundary:
                                const std::ostream::pos_type
                                        curr = f.tellg(),
                                        next_mul2 = 2U*((curr + std::ostream::pos_type(1U))/2U),
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
                                                        set32(x, (infoHeader.height-1) - y, v);
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
std::ostream& operator<< (std::ostream &os, BitmapImageData const &) {
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

struct Bitmap {
public:
        Bitmap() :
                filename_(),
                header_(),
                infoHeader_(),
                colorTable_(),
                colorMask_(),
                imageData_(),

                has_alpha_(),

                bitmask_(),

                valid_(false),
                bitmapVersion_()
        { }

        Bitmap(std::istream &f) {
                reset(f);
        }

        bool partial_reset(std::istream &f) {
                return reset(f, false);
        }

        void reset() {
                using std::swap;
                Bitmap tmp;
                swap(*this, tmp);
        }

        void reset(std::istream &f) {
                reset(f, true);
        }

        int width() const { return infoHeader_.width; }
        int height() const { return infoHeader_.height; }
        int bpp() const { return infoHeader_.bitsPerPixel; }
        bool valid() const {
                return valid_;
        }

        bool is_paletted() const {
                switch (bpp()) {
                case 1:
                case 2:
                case 4:
                case 8:
                        return true;
                default:
                        return false;
                }
        }

        // TODO: is_rgb is not really unambiguous ("not rgb = maybe hsv?")
        bool is_rgb() const {
                return !is_paletted();
        }

        bool has_alpha() const {
                return has_alpha_;
        }

        unsigned int Bitmap::x_pixels_per_meter() const {
                return infoHeader_.xPixelsPerMeter;
        }

        unsigned int Bitmap::y_pixels_per_meter() const {
                return infoHeader_.yPixelsPerMeter;
        }

        bool Bitmap::has_square_pixels() const {
                return x_pixels_per_meter() == y_pixels_per_meter();
        }

        std::set<BitmapVersion> version() const {
                return bitmapVersion_;
        }

        Color32 get32(int x, int y) const {
                if (x<0 || x>=width() || y<0 || y>=height()) {
                        return Color32();
                }

                const uint32_t raw = imageData_.get32(x, y);
                if (is_rgb()) {
                        Color32 col = bitmask_.rawToColor(raw);
                        col.a(has_alpha() ? col.a() : 255);
                        return col;
                } else if(is_paletted()) {
                        return colorTable_[raw];
                } else {
                        return Color32();
                }
        }

        Color32 at32(int x, int y) const {
                // TODO: use puffin exceptions
                if (x<0 || x>=width()) {
                        throw std::logic_error(
                                "BitmapImageData.at32(): x out of range");
                }
                if (y<0 || y>=height()) {
                        throw std::logic_error(
                                "BitmapImageData.at32(): y out of range");
                }

                const uint32_t raw = imageData_.get32(x, y);
                if (is_rgb()) {
                        return bitmask_.rawToColor(raw);
                } else if(is_paletted()) {
                        return colorTable_.at(raw);
                } else {
                        throw std::runtime_error("Neither paletted, nor RGB");
                }
        }

        friend
        std::ostream& operator<< (std::ostream &os, Bitmap const &v) {
                os << v.header_;
                os << v.infoHeader_;
                if (false) {
                        if (v.colorTable_.empty())
                                os << "ColorTable:<empty>\n";
                        else
                                os << "ColorTable:[...]\n";
                } else {
                        os << v.colorTable_;
                }
                os << v.colorMask_;
                os << "ImageData:"
                   << (v.imageData_.empty() ? "no" : "yes")
                   << "\n";
                os << "bpp:" << v.bpp() << "\n"
                   << "rgb:" << (v.is_rgb()?"yes":"no") << "\n"
                   << "paletted:" << (v.is_paletted()?"yes":"no") << "\n"
                   << "alpha:" << (v.has_alpha()?"yes":"no") << "\n"
                   << "valid:" << (v.valid()?"yes":"no") << "\n"
                   << "square pixels:" << (v.has_square_pixels()?"yes":"no") << "\n"
                   << "bitmap version:" << v.version() << "\n"
                   << "mask r:" << std::bitset<32>(v.bitmask_.r().mask()<<v.bitmask_.r().shift()) << ", r_width_:" << (int)v.bitmask_.r().width() << "\n"
                   << "mask g:" << std::bitset<32>(v.bitmask_.g().mask()<<v.bitmask_.g().shift()) << ", g_width_:" << (int)v.bitmask_.g().width() << "\n"
                   << "mask b:" << std::bitset<32>(v.bitmask_.b().mask()<<v.bitmask_.b().shift()) << ", b_width_:" << (int)v.bitmask_.b().width() << "\n"
                   << "mask a:" << std::bitset<32>(v.bitmask_.a().mask()<<v.bitmask_.a().shift()) << ", a_width_:" << (int)v.bitmask_.a().width() << "\n"
                ;
                return os;
        }
private:
        std::string filename_;
        impl::BitmapHeader header_;
        impl::BitmapInfoHeader infoHeader_;
        impl::BitmapColorTable colorTable_;
        impl::BitmapColorMasks colorMask_;
        impl::BitmapImageData  imageData_;

        bool has_alpha_;
        RgbaBitmask32 bitmask_;

        bool valid_;
        std::set<BitmapVersion> bitmapVersion_;

private:
        static bool compressionSupported(BitmapCompression v) {
                switch (v) {
                case BI_RGB:
                case BI_RLE8:
                case BI_RLE4:
                case BI_BITFIELDS:
                        return true;
                case BI_JPEG:
                case BI_PNG:
                case BI_CMYK:
                case BI_CMYKRLE8:
                case BI_CMYKRLE4:
                default:
                        return false;
                }
        }

        bool reset(std::istream &f, bool exceptions) {
                reset();

                bitmapVersion_ = determineBitmapVersion(f);
                loadHeaders(f);

                if (!compressionSupported(infoHeader_.compression)) {
                        if (exceptions) {
                                throw exceptions::unsupported_bitmap_compression(
                                        infoHeader_.compression,
                                        to_string(infoHeader_.compression));
                        } else {
                                return false;
                        }
                }

                initBitmasks(f);
                colorTable_.reset(header_, infoHeader_, bitmapVersion_, f);
                imageData_.reset(header_, infoHeader_, f);
                initAlpha();

                valid_ = true;
                return true;
        }

        void loadHeaders(std::istream &f) {
                header_.reset(f);
                const auto headerPos = f.tellg();
                infoHeader_.reset(bitmapVersion_, f);
                f.seekg(headerPos);
                f.seekg(infoHeader_.infoHeaderSize, std::ios_base::cur);
        }

        void initBitmasks(std::istream &f) {
                colorMask_.reset(infoHeader_, f);

                if (infoHeader_.compression == BI_BITFIELDS) {
                        bitmask_.reset(colorMask_.red, colorMask_.green, colorMask_.blue);
                } else {
                        switch (bpp()) {
                        case 16:
                                bitmask_.reset(0x1F << 10, 0x1F << 5, 0x1F);
                                break;
                        case 24:
                                bitmask_.reset(0xFF << 16, 0xFF << 8, 0xFF);
                                break;
                        case 32:
                                bitmask_.reset(0xFF << 16, 0xFF << 8, 0xFF);
                                break;
                        default:
                                bitmask_.reset();
                                break;
                        }
                }
        }

        void initAlpha() {
                // detect alpha (non-standard; in ICO files, transparency is
                // defined if any "reserved" value is non-zero)
                has_alpha_ = false;
                if (is_rgb()) {
                        for (int y = 0; y < height(); ++y) {
                                for (int x = 0; x < width(); ++x) {
                                        const uint32_t raw = imageData_.get32(x, y);
                                        const Color32 col = bitmask_.rawToColor(raw);
                                        if (col.a() != 0) {
                                                has_alpha_ = true;
                                                break;
                                        }
                                }
                                if (has_alpha_)
                                        break;
                        }
                }
        }
};



} }


namespace puffin {

// -- class Bitmap -------------------------------------------------------------
Bitmap::Bitmap() :
        impl_(new impl::Bitmap())
{
}

Bitmap::Bitmap(std::istream &f) :
        impl_(new impl::Bitmap(f))
{
}

Bitmap::~Bitmap() {
        delete impl_;
}

Bitmap::Bitmap(Bitmap const &v) :
        impl_(new impl::Bitmap(*v.impl_))
{
}

Bitmap& Bitmap::operator= (Bitmap const &v) {
        using std::swap;
        Bitmap tmp(v);
        swap(*this, tmp);
        return *this;
}

void Bitmap::reset() {
        using std::swap;
        Bitmap tmp;
        swap(*this, tmp);
}

void Bitmap::reset(std::istream &f) {
        impl_->reset(f);
}

int Bitmap::width() const {
        return impl_->width();
}

int Bitmap::height() const {
        return impl_->height();
}

int Bitmap::bpp() const {
        return impl_->bpp();
}

bool Bitmap::is_paletted() const {
        return impl_->is_paletted();
}

bool Bitmap::is_rgb() const {
        return impl_->is_rgb();
}

bool Bitmap::has_alpha() const {
        return impl_->has_alpha();
}

unsigned int Bitmap::x_pixels_per_meter() const {
        return impl_->x_pixels_per_meter();
}

unsigned int Bitmap::y_pixels_per_meter() const {
        return impl_->y_pixels_per_meter();
}

bool Bitmap::has_square_pixels() const {
        return impl_->has_square_pixels();
}

std::set<BitmapVersion> Bitmap::version() const {
        return impl_->version();
}

Color32 Bitmap::operator()(int x, int y) const {
        return impl_->get32(x, y);
}

Color32 Bitmap::at(int x, int y) const {
        return impl_->at32(x, y);
}

std::ostream& operator<< (std::ostream &os, Bitmap const &v) {
        return os << *v.impl_;
}

// -- class InvalidBitmap ------------------------------------------------------
InvalidBitmap::InvalidBitmap() :
        impl_(new impl::Bitmap())
{
}

InvalidBitmap::InvalidBitmap(std::istream &f) :
        impl_(new impl::Bitmap(f))
{
}

InvalidBitmap::~InvalidBitmap() {
        delete impl_;
}

InvalidBitmap::InvalidBitmap(InvalidBitmap const &v) :
        impl_(new impl::Bitmap(*v.impl_))
{
}

InvalidBitmap& InvalidBitmap::operator= (InvalidBitmap const &v) {
        using std::swap;
        InvalidBitmap tmp(v);
        swap(*this, tmp);
        return *this;
}

void InvalidBitmap::reset() {
        using std::swap;
        InvalidBitmap tmp;
        swap(*this, tmp);
}

void InvalidBitmap::reset(std::istream &f) {
        impl_->reset(f);
}

bool InvalidBitmap::partial_reset(std::istream &f) {
        return impl_->partial_reset(f);
}

int InvalidBitmap::width() const {
        return impl_->width();
}

int InvalidBitmap::height() const {
        return impl_->height();
}

int InvalidBitmap::bpp() const {
        return impl_->bpp();
}

bool InvalidBitmap::is_paletted() const {
        return impl_->is_paletted();
}

bool InvalidBitmap::is_rgb() const {
        return impl_->is_rgb();
}

bool InvalidBitmap::has_alpha() const {
        return impl_->has_alpha();
}

bool InvalidBitmap::valid() const {
        return impl_->valid();
}

unsigned int InvalidBitmap::x_pixels_per_meter() const {
        return impl_->x_pixels_per_meter();
}

unsigned int InvalidBitmap::y_pixels_per_meter() const {
        return impl_->y_pixels_per_meter();
}

bool InvalidBitmap::has_square_pixels() const {
        return impl_->has_square_pixels();
}

std::set<BitmapVersion> InvalidBitmap::version() const {
        return impl_->version();
}

Color32 InvalidBitmap::operator()(int x, int y) const {
        return impl_->get32(x, y);
}

Color32 InvalidBitmap::at(int x, int y) const {
        return impl_->at32(x, y);
}

std::ostream& operator<< (std::ostream &os, InvalidBitmap const &v) {
        return os << *v.impl_;
}


// -- read_bmp() ---------------------------------------------------------------
Bitmap read_bmp(std::string const &filename) {
        std::ifstream f(filename, std::ios::binary);
        if (!f.is_open())
                throw exceptions::file_not_found(filename);
        return Bitmap(f);
}

InvalidBitmap read_invalid_bmp(std::string const &filename) {
        std::ifstream f(filename, std::ios::binary);
        if (!f.is_open())
                return InvalidBitmap();
        InvalidBitmap ret;
        ret.partial_reset(f);
        return ret;
}

}

// TODO: See http://www.fileformat.info/format/bmp/egff.htm:
//       "Windows BMP File Types
//
//        Each new version of BMP has added new information to the bitmap header. In
//        some cases, the newer versions have changed the size of the color palette
//        and added features to the format itself. Unfortunately, a field wasn't
//        included in the header to easily indicate the version of the file's format
//        or the type of operating system that created the BMP file. If we add
//        Windows' four versions of BMP to OS/2's two versions--each with four
//        possible variations--we find that as many as twelve different related file
//        formats all have the file extension ".BMP".
//
//        It is clear that you cannot know the internal format of a BMP file based on
//        the file extension alone. But, fortunately, you can use a short algorithm to
//        determine the internal format of BMP files.
//
//        The FileType field of the file header is where we start. If these two byte
//        values are 424Dh ("BM"), then you have a single-image BMP file that may have
//        been created under Windows or OS/2. If FileType is the value 4142h ("BA"),
//        then you have an OS/2 bitmap array file. Other OS/2 BMP variations have the
//        file extensions .ICO and .PTR.
//
//        If your file type is "BM", then you must now read the Size field of the
//        bitmap header to determine the version of the file. Size will be 12 for
//        Windows 2.x BMP and OS/2 1.x BMP, 40 for Windows 3.x and Windows NT BMP, 12
//        to 64 for OS/2 2.x BMP, and 108 for Windows 4.x BMP. A Windows NT BMP file
//        will always have a Compression value of 3; otherwise, it is read as a Windows
//        3.x BMP file.
//
//        Note that the only difference between Windows 2.x BMP and OS/2 1.x BMP is
//        the data type of the Width and Height fields. For Windows 2.x, they are
//        signed shorts and for OS/2 1.x, they are unsigned shorts. Windows 3.x,
//        Windows NT, and OS/2 2.x BMP files only vary in the number of fields in the
//        bitmap header and in the interpretation of the Compression field."


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Implementer's notes.
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// On "pixel-endianness"
// ------------------------
//
// Given a 4-byte chunk of pixel data, the pixel value with the smallest
// x-coordinate is left-most when read unaltered from a BMP file.
//
// This is sub-optimal, as this imposes an additional subtraction
// (yes, I was tempted to write "imposes subtraction addition", but
// I dislike comment humor - irony is alright, tho.)
//
// Now for an explanation:
//
// Given
//
//    chunk size: 16 bits
//    pixel size:  4 bist,
//
// we get this layout and pixel equations:
//
//         p0   p1   p2   p3
//        [1111 0110 0010 0000]
//
//        p0 =  (chunk>>12) & 1111b = (chunk>>(16-4 - 0*4)) & 1111b
//        p1 =  (chunk>> 8) & 1111b = (chunk>>(16-4 - 1*4)) & 1111b
//        p2 =  (chunk>> 4) & 1111b = (chunk>>(16-4 - 2*4)) & 1111b
//        p3 =  (chunk>> 0) & 1111b = (chunk>>(16-4 - 3*4)) & 1111b.
//
// But with "pixel-endianness" flipped, the layout and equations
//
//         p3   p2   p1   p0
//        [0000 0010 0110 1111]
//
//        p0 =  (chunk>> 0) & 1111b = (chunk>>(0*4)) & 1111b
//        p1 =  (chunk>> 4) & 1111b = (chunk>>(1*4)) & 1111b
//        p2 =  (chunk>> 8) & 1111b = (chunk>>(2*4)) & 1111b
//        p3 =  (chunk>>12) & 1111b = (chunk>>(3*4)) & 1111b
//
// spare us a confusing shift and a subtraction.
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
