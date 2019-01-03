//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Usage notes
// (you can find implementer's not at the bottom of this file).
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#include "puffin/bitmap.hh"
#include "puffin/exceptions.hh"
#include "puffin/impl/io_util.hh"
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
        BitmapCompression compression; // 0 = BI_RGB (none), 1 = BI_RLE8, 2 = BI_RLE4
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

        BitmapInfoHeader(/*BitmapHeader const &header, */std::istream &f) {
                reset(f);
        }

        void reset(std::istream &f) {
                infoHeaderSize = impl::read_uint32_le(f);
                width = impl::read_uint32_le(f);
                height = impl::read_int32_le(f);
                planes = impl::read_uint16_le(f);
                bitsPerPixel = impl::read_uint16_le(f);
                compression = static_cast<BitmapCompression >(impl::read_uint32_le(f));
                compressedImageSize = impl::read_uint32_le(f);
                xPixelsPerMeter = impl::read_uint32_le(f);
                yPixelsPerMeter = impl::read_uint32_le(f);
                colorsUsed = impl::read_uint32_le(f);
                importantColors = impl::read_uint32_le(f);

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

        BitmapColorTable(BitmapInfoHeader const &info, std::istream &f) :
                entries_(readEntries(info, f)) {}

        void reset(BitmapInfoHeader const &info, std::istream &f) {
                entries_ = readEntries(info, f);
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
                BitmapInfoHeader const &info,
                std::istream &f
        ) {
                const auto numColors = info.colorsUsed != 0 ? info.colorsUsed :
                                       info.bitsPerPixel == 1 ? 2 :
                                       info.bitsPerPixel == 2 ? 4 :
                                       info.bitsPerPixel == 4 ? 16 :
                                       info.bitsPerPixel == 8 ? 256 : 0;

                std::vector<Color32> ret;
                ret.reserve(numColors);
                // TODO: Probably need to rework the table size determination
                for (unsigned int i = 0; i < numColors; ++i) {
                        const uint8_t blue = impl::read_uint8_le(f),
                                      green = impl::read_uint8_le(f),
                                      red = impl::read_uint8_le(f),
                                      reserved = impl::read_uint8_le(f);
                        ret.push_back(Color32(red, green, blue));
                }
                return ret;
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
        for (int i=0; i!=v.size(); ++i) {
                os << "  [" << std::setw(3) << i << "]:" << v[i] << "\n";
        }
        os << "}\n";
        return os;
}



struct BitmapColorMasks {
        uint32_t blue;
        uint32_t green;
        uint32_t red;

        BitmapColorMasks() : blue(0), green(0), red(0) {}

        BitmapColorMasks(BitmapInfoHeader const &info, std::istream &f) {
                reset(info, f);
        }

        void reset(BitmapInfoHeader const &info, std::istream &f) {
                if (info.compression != BitmapCompression::BI_BITFIELDS)
                        return;
                blue = impl::read_uint32_le(f);
                green = impl::read_uint32_le(f);
                red = impl::read_uint32_le(f);
        }
};
inline
std::ostream& operator<< (std::ostream &os, BitmapColorMasks const &v) {
        return os << "ColorMask{\n"
                  << "  red..:" << v.red << "\n"
                  << "  green:" << v.green << "\n"
                  << "  blue.:" << v.blue << "\n"
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
                width_ = infoHeader.width;

                const uint32_t chunk_width = infoHeader.bitsPerPixel == 24 ? 24 : 32;
                pixel_width = infoHeader.bitsPerPixel;

                pixels_per_chunk = chunk_width / pixel_width;
                bytes_per_chunk = chunk_width / 8;
                pixel_mask = (1U << pixel_width) - 1U; // TODO: This may fail for large types
                nonsignificant_bits = chunk_width % pixel_width;

                const uint32_t numChunks = width_to_chunk_count(infoHeader.width);
                chunks_.clear();
                chunks_.reserve(numChunks);
                for (uint32_t i=0U; i!=numChunks; ++i) {

                        // TODO: make chunk-type more generic.
                        const chunk_type chunk =
                                read_bytes_to_uint32_le(f, bytes_per_chunk) >>
                                nonsignificant_bits;

                        // Flip the order of pixels in this chunk (which will
                        //  result in a more trivial extract_pixel() function):
                        chunk_type flipped = 0U;
                        for (uint32_t j=0U; j!=pixels_per_chunk; ++j) {
                                const chunk_type pixel = extract_pixel(chunk, j);
                                flipped = (flipped<<pixel_width) | pixel;
                        }
                        chunks_.push_back(flipped);
                }

                // Round up to multiple of 4.
                // Example:
                //   2 chunks, 3 bytes each -> 6 bytes read
                //   Need to round 6 to 8 (=2*4).
                //
                //   Attempt 1)   4*(6/4) = 4  -> no.
                //   Attempt 2)   4*((6+3)/4) = 8 -> maybe.
                //   round(x)_m = m*((x+m-1)/m)
                //
                //   Tests:
                //       round(7)_4 = 4*((7+3)/4) = 8
                //       round(8)_4 = 4*((8+3)/4) = 8
                //       round(9)_4 = 4*((9+3)/4) = 12
                // TODO: Make a reusable function of round().
                const uint32_t
                        bytes_read = numChunks * bytes_per_chunk,
                        next_mul4 = 4U*((bytes_read+3U)/4U),
                        pad_bytes = next_mul4 - bytes_read;
                f.ignore(pad_bytes);
        }

        uint32_t get32 (int x) const {
                const uint32_t
                        chunk_index = x_to_chunk_index(x),
                        chunk_ofs   = x_to_chunk_offset(x),
                        chunk = chunks_[chunk_index],
                        value = extract_pixel(chunk, chunk_ofs);
                return value;
        }

private:
        // -- data -----------------------------------------------------
        uint32_t pixels_per_chunk;
        uint32_t bytes_per_chunk;
        uint32_t pixel_width;
        uint32_t pixel_mask;
        uint32_t nonsignificant_bits;

        uint32_t width_;
        container_type chunks_;

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
        ChunkT extract_pixel(ChunkT chunk, uint32_t ofs) const {
                const ChunkT
                        rshift = ofs * pixel_width,
                        value = (chunk >> rshift) & pixel_mask;
                return value;

                // Here's the code for [pixel_0, pixel_1, ..., pixel_n]:
                //
                // const uint32_t
                //        rshift = chunk_width-pixel_width - ofs*pixel_width,
                //        value = (chunk >> rshift) & pixel_mask;
                // return value;
        }
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
};

inline
std::ostream& operator<< (std::ostream &os, BitmapImageData const &data) {
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

                b_shift_(0), b_mask_(0),
                g_shift_(0), g_mask_(0),
                r_shift_(0), r_mask_(0),
                a_shift_(0), a_mask_(0),

                valid_(0)
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

        Color32 get32(int x, int y) const {
                if (x<0 || x>=width() || y<0 || y>=height()) {
                        return Color32();
                }

                const uint32_t raw = imageData_.get32(x, y);
                if (is_rgb()) {
                        Color32 col = rawToColor32(raw);
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
                        return rawToColor32(raw);
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
                if (true) {
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
                os << "rgb:" << (v.is_rgb()?"yes":"no") << "\n"
                   << "paletted:" << (v.is_paletted()?"yes":"no") << "\n"
                   << "alpha:" << (v.has_alpha()?"yes":"no") << "\n"
                   << "valid:" << (v.valid()?"yes":"no") << "\n";
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

        uint32_t b_shift_, b_mask_;
        uint32_t g_shift_, g_mask_;
        uint32_t r_shift_, r_mask_;
        uint32_t a_shift_, a_mask_;

        bool valid_;

        bool reset(std::istream &f, bool exceptions) {
                reset();

                // load bmp file
                header_.reset(f);
                const auto headerPos = f.tellg();
                infoHeader_.reset(f);
                f.seekg(headerPos);
                switch (infoHeader_.compression) {
                case BI_RGB:
                        break;
                case BI_RLE8:
                case BI_RLE4:
                case BI_BITFIELDS:
                case BI_JPEG:
                case BI_PNG:
                case BI_CMYK:
                case BI_CMYKRLE8:
                case BI_CMYKRLE4:
                default:
                        if (exceptions) {
                                throw exceptions::unsupported_bitmap_compression(
                                        infoHeader_.compression,
                                        to_string(infoHeader_.compression));
                        } else {
                                return false;
                        }
                }

                f.seekg(infoHeader_.infoHeaderSize, std::ios_base::cur);
                colorTable_.reset(infoHeader_, f);
                colorMask_.reset(infoHeader_, f);
                imageData_.reset(header_, infoHeader_, f);

                // set bitmasks
                switch (bpp()) {
                case 24:
                        a_mask_ = 0x0;
                        b_mask_ = 0xFF;
                        g_mask_ = 0xFF;
                        r_mask_ = 0xFF;
                        a_shift_ = 0;
                        b_shift_ = 16;
                        g_shift_ = 8;
                        r_shift_ = 0;
                        break;
                case 32:
                        a_mask_ = 0xFF;
                        b_mask_ = 0xFF;
                        g_mask_ = 0xFF;
                        r_mask_ = 0xFF;
                        a_shift_ = 24;
                        b_shift_ = 16;
                        g_shift_ = 8;
                        r_shift_ = 0;
                        break;
                default:
                        a_mask_ = 0x0;
                        b_mask_ = 0x0;
                        g_mask_ = 0x0;
                        r_mask_ = 0x0;
                        a_shift_ = 0;
                        b_shift_ = 0;
                        g_shift_ = 0;
                        r_shift_ = 0;
                        break;
                }

                // detect alpha (non-standard; in ICO files, transparency is
                // defined if any "reserved" value is non-zero)
                has_alpha_ = false;
                if (is_rgb()) {
                        for (int y = 0; y < height(); ++y) {
                                for (int x = 0; x < width(); ++x) {
                                        const uint32_t raw = imageData_.get32(x, y);
                                        const Color32 col = rawToColor32(raw);
                                        if (col.a() != 0) {
                                                has_alpha_ = true;
                                                break;
                                        }
                                }
                                if (has_alpha_)
                                        break;
                        }
                }

                valid_ = true;
                return true;
        }

        Color32 rawToColor32(uint32_t raw) const {
                const uint8_t
                        b = static_cast<uint8_t>((raw >> b_shift_) & b_mask_),
                        g = static_cast<uint8_t>((raw >> g_shift_) & g_mask_),
                        r = static_cast<uint8_t>((raw >> r_shift_) & r_mask_),
                        a = static_cast<uint8_t>((raw >> a_shift_) & a_mask_)
                ;
                return Color32(r, g, b, a);
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
                throw exceptions::file_not_found(filename);
        InvalidBitmap ret;
        ret.partial_reset(f);
        return ret;
}

}

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