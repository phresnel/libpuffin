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
