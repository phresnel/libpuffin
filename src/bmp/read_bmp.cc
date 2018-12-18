#include "puffin/bmp/read_bmp.hh"
#include <fstream>
#include <iostream>
#include <array>
#include <iomanip>
#include <cstdint>
#include <vector>

namespace puffin {

//------------------------------------------------------------------------------
// Based, among others, on these:
// * https://docs.microsoft.com/en-us/previous-versions/dd183376(v%3Dvs.85)
// * http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
// * https://en.wikipedia.org/wiki/BMP_file_format
//------------------------------------------------------------------------------

enum BitmapCompression
{
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

struct Bitmap {
        struct Header {
                uint16_t signature;   // 'BM'
                uint32_t size;        // Size in bytes.
                uint16_t reserved1;
                uint16_t reserved2;
                uint32_t dataOffset;  // Offset from beginning of file to pixel data.

                Header(std::ifstream &f);
        };

        struct InfoHeader {
                // read from file
                uint32_t infoHeaderSize;
                uint32_t width;
                uint32_t height;
                uint16_t planes;
                uint16_t bitsPerPixel;
                uint32_t compression; // 0 = BI_RGB (none), 1 = BI_RLE8, 2 = BI_RLE4
                uint32_t compressedImageSize; // 0 if compression is 0
                uint32_t xPixelsPerMeter;
                uint32_t yPixelsPerMeter;
                uint32_t colorsUsed; // Actually used colors (256 for 8 bit, 0=max number of colors according to bitsPerPixel)
                uint32_t importantColors; // Number of colors required for display. 0=max.

                // computed fields
                bool isBottomUp;

                InfoHeader(std::ifstream &f);
        };

        struct ColorTable {
                struct Entry {
                        uint8_t red;
                        uint8_t green;
                        uint8_t blue;
                        uint8_t reserved;

                        Entry(std::ifstream &);
                };
                std::vector<Entry> entries;

                ColorTable (InfoHeader const &info, std::ifstream &f);
        private:
                static std::vector<Entry> readEntries(InfoHeader const &info, std::ifstream &f);
        };

        struct ColorMask {
                uint32_t red = 0;
                uint32_t green = 0;
                uint32_t blue = 0;

                ColorMask (InfoHeader const &info, std::ifstream &f);
        };

        struct RowData1bit {
                RowData1bit(InfoHeader const &, std::ifstream &);
                int operator[] (int index) const;
        private:
                std::vector<uint32_t> data_;

                enum { pixels_per_chunk = 32 };
                static uint32_t width_to_chunk_count(uint32_t x) noexcept;
                static uint32_t x_to_chunk_index(uint32_t x) noexcept;
                static uint32_t x_to_chunk_offset(uint32_t x) noexcept;
        };

        struct ImageData1bit {
                ImageData1bit(Header const &, InfoHeader const &, std::ifstream &);

                int operator() (int x, int y) const;

                bool empty() const;
                int width() const;
                int height() const;
        private:
                std::vector<RowData1bit> rows_;
                uint32_t width_;
        };
};


// Wikipedia says: "All of the integer values are stored in little-endian format"
// Little endian helpers:
uint8_t read1_le(std::ifstream &f) {
        return static_cast<uint8_t>(f.get());
}
uint16_t read2_le(std::ifstream &f) {
        return read1_le(f) | static_cast<uint16_t>(read1_le(f)) << 8;
}
uint32_t read4_le(std::ifstream &f) {
        return read2_le(f) | static_cast<uint32_t>(read2_le(f)) << 16;
}
uint64_t read8_le(std::ifstream &f) {
        return read4_le(f) | static_cast<uint64_t>(read4_le(f)) << 32;
}

// Big endian helpers:
uint8_t read1_be(std::ifstream &f) {
        return static_cast<uint8_t>(f.get());
}
uint16_t read2_be(std::ifstream &f) {
        return static_cast<uint16_t>(read1_be(f)) << 8 | read1_be(f);
}
uint32_t read4_be(std::ifstream &f) {
        return static_cast<uint32_t>(read2_be(f)) << 16 | read2_be(f);
}
uint64_t read8_be(std::ifstream &f) {
        return static_cast<uint64_t>(read4_be(f)) << 32 | read4_be(f);
}

Bitmap::Header::Header(std::ifstream &f) :
        signature{read2_be(f)},
        size{read4_le(f)},
        reserved1{read2_le(f)},
        reserved2{read2_le(f)},
        dataOffset{read4_le(f)}
{
}

Bitmap::InfoHeader::InfoHeader(std::ifstream &f) :
        infoHeaderSize{read4_le(f)},
        width{read4_le(f)},
        height{read4_le(f)},
        planes{read2_le(f)},
        bitsPerPixel{read2_le(f)},
        compression{read4_le(f)},
        compressedImageSize{read4_le(f)},
        xPixelsPerMeter{read4_le(f)},
        yPixelsPerMeter{read4_le(f)},
        colorsUsed{read4_le(f)},
        importantColors{read4_le(f)}
{
        if (height >= 0) {
                isBottomUp = true;
        } else {
                height = -height;
                isBottomUp = false;
        }
}

Bitmap::ColorTable::ColorTable(
        puffin::Bitmap::InfoHeader const &info,
        std::ifstream &f
) :
        entries(readEntries(info, f))
{
}

std::vector<Bitmap::ColorTable::Entry>
Bitmap::ColorTable::readEntries(InfoHeader const &info, std::ifstream &f) {
        const auto numColors = info.colorsUsed != 0 ? info.colorsUsed :
                               info.bitsPerPixel == 1 ? 2 :
                               info.bitsPerPixel == 2 ? 4 :
                               info.bitsPerPixel == 4 ? 16 :
                               info.bitsPerPixel == 8 ? 256 : 0;

        std::vector<Entry> ret;
        ret.reserve(numColors);
        for (unsigned int i=0; i<numColors; ++i) {
                ret.push_back(Entry{f});
        }
        return ret;
}


Bitmap::ColorTable::ColorTable::Entry::Entry(std::ifstream &f) :
        blue{read1_le(f)},
        green{read1_le(f)},
        red{read1_le(f)},
        reserved{read1_le(f)}
{

}

Bitmap::ColorMask::ColorMask(
        puffin::Bitmap::InfoHeader const &info,
        std::ifstream &f
) {
        if (info.compression != BitmapCompression::BI_BITFIELDS)
                return;
        red = read4_le(f);
        green = read4_le(f);
        blue = read4_le(f);
}

Bitmap::RowData1bit::RowData1bit(
        puffin::Bitmap::InfoHeader const &infoHeader,
        std::ifstream &f
) {
        // 4 byte alignment, best explained and visualized by example:
        //      width = 3, height = 1
        //      pixels = [011x xxxx][xxxx xxxx][xxxx xxxx][xxxx xxxx]
        //
        // Terminology used in the following:
        //      Pixels = Row_1 Row_2 ... Row_imageHeight
        //      Row    = Chunk_1 Chunk_2 ... Chunk_numChunks
        //      Chunk  = Pixel_1 Pixel_2 ... Pixel_pixelsPerChunk Padding
        // A chunk here aligns to 4 byte (see above).

        const auto numChunks = width_to_chunk_count(infoHeader.width);
        for (auto chunk=0U; chunk!=numChunks; ++chunk) {
                uint32_t chunkData;
                f >> chunkData;
                data_.push_back(chunkData);
        }
}

int Bitmap::RowData1bit::operator[] (int x) const {
        std::cerr << "given x=" << x << std::endl;

        const uint32_t chunk_index = x_to_chunk_index(x);
        std::cerr << "  chunk_index=" << chunk_index << std::endl;

        const uint32_t chunk_ofs = x_to_chunk_offset(x);
        std::cerr << "  chunk_ofs=" << chunk_ofs << std::endl;

        const uint32_t chunk_data = data_[chunk_index];
        std::cerr << "  chunk_data=" << chunk_data << std::endl;

        const uint32_t value = chunk_data >> chunk_ofs;
        std::cerr << "  value=" << value << std::endl;

        return static_cast<int>(value);
}


// TODO: the following two functions should be engraved in a template
uint32_t Bitmap::RowData1bit::width_to_chunk_count(uint32_t x) noexcept {
        // This adjusted division ensures that any result with a non-zero
        // fractional part is rounded up:
        //      width=64  ==>  c = (64 + 31) / 32 = 95 / 32 = 2
        //      width=65  ==>  c = (65 + 31) / 32 = 96 / 32 = 3
        return (x + pixels_per_chunk - 1U) / pixels_per_chunk;
}

uint32_t Bitmap::RowData1bit::x_to_chunk_index(uint32_t x) noexcept {
        return x / pixels_per_chunk;
}

uint32_t Bitmap::RowData1bit::x_to_chunk_offset(uint32_t x) noexcept {
        return x - x_to_chunk_index(x) * pixels_per_chunk;
}

Bitmap::ImageData1bit::ImageData1bit(
        puffin::Bitmap::Header const &header,
        puffin::Bitmap::InfoHeader const &infoHeader,
        std::ifstream &f
) {
        if (infoHeader.bitsPerPixel != 1)
                return;

        f.seekg(header.dataOffset);

        // TODO: Discuss if it's better to trust infoHeader.ImageSize.
        // TODO: honor 'infoHeader.isBottomUp'
        rows_.reserve(infoHeader.height);
        for (int y=0; y!=infoHeader.height; ++y) {
                rows_.emplace_back(infoHeader, f);
        }

        width_ = infoHeader.width;
}

int Bitmap::ImageData1bit::operator()(int x, int y) const {
        return rows_[y][x];
}

bool Bitmap::ImageData1bit::empty() const {
        return rows_.size() == 0;
}

int Bitmap::ImageData1bit::width() const {
        return width_;
}

int Bitmap::ImageData1bit::height() const {
        return rows_.size();
}



std::ostream& operator<< (std::ostream &os, BitmapCompression bc) {
        switch (bc) {
        case BI_RGB: return os << "BI_RGB";
        case BI_RLE8: return os << "BI_RLE8";
        case BI_RLE4: return os << "BI_RLE4";
        case BI_BITFIELDS: return os << "BI_BITFIELDS";
        case BI_JPEG: return os << "BI_JPEG";
        case BI_PNG: return os << "BI_PNG";
        case BI_CMYK: return os << "BI_CMYK";
        case BI_CMYKRLE8: return os << "BI_CMYKRLE8";
        case BI_CMYKRLE4: return os << "BI_CMYKRLE4";
        default: return os << "<unknown>";
        }
}

std::ostream& operator<< (std::ostream &os, Bitmap::Header const &v) {
        return os << "Header{\n"
                  << "  signature:" << std::hex << v.signature << "\n"
                  << "  size:" << std::dec << v.size << "\n"
                  << "  reserved1:" << v.reserved1 << "\n"
                  << "  reserved2:" << v.reserved2 << "\n"
                  << "  dataOffset:" << v.dataOffset << "\n"
                  << "}\n";
}

std::ostream& operator<< (std::ostream &os, Bitmap::InfoHeader const &v) {
        return os << "InfoHeader{\n"
                  << "  infoHeaderSize:" << v.infoHeaderSize << "\n"
                  << "  width:" << v.width << "\n"
                  << "  height:" << v.height << "\n"
                  << "  isBottomUp:" << v.isBottomUp << "\n"
                  << "  planes:" << v.planes << "\n"
                  << "  bitsPerPixel:" << v.bitsPerPixel << "\n"
                  << "  compression:" << static_cast<BitmapCompression>(v.compression) << "\n"
                  << "  compressedImageSize:" << v.compressedImageSize << "\n"
                  << "  xPixelsPerMeter:" << v.xPixelsPerMeter << "\n"
                  << "  yPixelsPerMeter:" << v.yPixelsPerMeter << "\n"
                  << "  colorsUsed:" << v.colorsUsed << "\n"
                  << "  importantColors:" << v.importantColors << "\n"
                  << "}\n";
}

std::ostream& operator<< (std::ostream &os, Bitmap::ColorTable::Entry const &v) {
        return os << "[" << std::setw(3) << (unsigned int)v.red
                  << "|" << std::setw(3) << (unsigned int)v.green
                  << "|" << std::setw(3) << (unsigned int)v.blue
                  << "|" << std::setw(3) << (unsigned int)v.reserved
                  << "]";
}

std::ostream& operator<< (std::ostream &os, Bitmap::ColorTable const &v) {
        os << "ColorTable{\n";
        for (int i=0; i!=v.entries.size(); ++i) {
                os << "  [" << std::setw(3) << i << "]:" << v.entries[i] << "\n";
        }
        os << "}\n";
        return os;
}

std::ostream& operator<< (std::ostream &os, Bitmap::ColorMask const &v) {
        return os << "ColorMask{\n"
                  << "  red..:" << v.red << "\n"
                  << "  green:" << v.green << "\n"
                  << "  blue.:" << v.blue << "\n"
                  << "}\n";
}

std::ostream& operator<< (std::ostream &os, Bitmap::ImageData1bit const &data) {
        if (data.empty())
                return os;
        os << "ImageData1bit{\n";
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
}

void read_bmp(std::string const &filename) {
        std::ifstream f(filename, std::ios::binary);
        if (!f.is_open())
                throw exceptions::file_not_found(filename);

        const Bitmap::Header header(f);
        const auto headerPos = f.tellg();
        const Bitmap::InfoHeader infoHeader(f);
        f.seekg(headerPos);
        f.seekg(infoHeader.infoHeaderSize, std::ios_base::cur);
        const Bitmap::ColorTable colorTable(infoHeader, f);
        const Bitmap::ColorMask colorMask(infoHeader, f);
        const Bitmap::ImageData1bit imageData1bit(header, infoHeader, f);

        std::cout << header << "\n----\n";
        std::cout << infoHeader << "\n----\n";
        std::cout << colorTable << "\n----\n";
        std::cout << colorMask << "\n----\n";
        std::cout << imageData1bit << "\n----\n";
}

}
