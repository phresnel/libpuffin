#include "puffin/bmp.hh"
#include "puffin/impl/bmp_util.hh"
#include "puffin/exceptions.hh"
#include <iostream>
#include <fstream>
#include <bitset>

namespace puffin {


// -- Bitmap32::Impl -----------------------------------------------------------
struct Bitmap32::Impl {
        Impl(std::istream &f) {
                header.reset(f);
                const auto headerPos = f.tellg();
                infoHeader.reset(f);
                f.seekg(headerPos);
                f.seekg(infoHeader.infoHeaderSize, std::ios_base::cur);
                colorTable.reset(infoHeader, f);
                colorMask.reset(infoHeader, f);
                imageData1bit.reset(header, infoHeader, f);
                imageData2bit.reset(header, infoHeader, f); // non standard
                imageData4bit.reset(header, infoHeader, f);
                imageData8bit.reset(header, infoHeader, f);
                imageData24bit.reset(header, infoHeader, f);

                // TODO: add some debug-info facility
                // TODO: make degree of debugging output depend on some parameter
                // TODO: make output extensible (in a way that allows for all
                //       major protocols, e.g. json, XML, proto-bufs)
                std::cout << header;
                std::cout << infoHeader;
                if (true) {
                        if (colorTable.empty())
                                std::cout << "ColorTable:<empty>\n";
                        else
                                std::cout << "ColorTable:[...]\n";
                } else {
                        std::cout << colorTable;
                }
                std::cout << colorMask;
                if (true) {
                        std::cout << "ImageData1bit:" << (imageData1bit.empty() ? "no" : "yes") << "\n";
                        std::cout << "ImageData2bit:" << (imageData2bit.empty() ? "no" : "yes") << "\n";
                        std::cout << "ImageData4bit:" << (imageData4bit.empty() ? "no" : "yes") << "\n";
                        std::cout << "ImageData8bit:" << (imageData8bit.empty() ? "no" : "yes") << "\n";
                        std::cout << "ImageData24bit:" << (imageData24bit.empty() ? "no" : "yes") << "\n";
                } else {
                        std::cout << imageData1bit ;
                        std::cout << imageData2bit ;
                        std::cout << imageData4bit ;
                        std::cout << imageData8bit ;
                        std::cout << imageData24bit;
                }
        }

        int width() const {
                return infoHeader.width;
        }

        int height() const {
                return infoHeader.height;
        }

        Color32 get32(int x, int y) const {
                if (is_paletted()) {
                        return get_paletted_color32<false>(x, y);
                }
                return get_rgb_color32<false>(x, y);
        }

        Color32 at32(int x, int y) const {
                if (is_paletted()) {
                        return get_paletted_color32<true>(x, y);
                }
                return get_rgb_color32<true>(x, y);
        }

        bool is_paletted() const {
                return infoHeader.colorsUsed != 0 ||
                       infoHeader.bitsPerPixel <= 8;
        }

private:
        // -- types ------------------------------------------------------------
        typedef bitfield4<8, 8, 8, 0> color_type_24bit;

        // -- data -------------------------------------------------------------
        std::string filename;
        impl::BitmapHeader header;
        impl::BitmapInfoHeader infoHeader;
        impl::BitmapColorTable colorTable;
        impl::BitmapColorMasks colorMask;
        impl::BitmapImageData<impl::BitmapRowData<32, 1, int> > imageData1bit;
        impl::BitmapImageData<impl::BitmapRowData<32, 2, int> > imageData2bit;
        impl::BitmapImageData<impl::BitmapRowData<32, 4, int> > imageData4bit;
        impl::BitmapImageData<impl::BitmapRowData<32, 8, int> > imageData8bit;
        impl::BitmapImageData<impl::BitmapRowData<24, 24, color_type_24bit> >
                                                                imageData24bit;

        // TODO: 32 bit:
        //       impl::BitmapImageData<64, 16> imageData32bit;
        // TODO: 64 bit (e.g. as supported by GDI+):
        //       impl::BitmapImageData<64, 16> imageData16bit;

        // -- methods ----------------------------------------------------------
        int get_palette_index(int x, int y) const {
                if (!imageData1bit.empty()) {
                        return imageData1bit(x, y);
                } else if (!imageData2bit.empty()) {
                        return imageData2bit(x, y);
                } else if (!imageData4bit.empty()) {
                        return imageData4bit(x, y);
                } else if (!imageData8bit.empty()) {
                        return imageData8bit(x, y);
                }
                return -1;
        }

        template <bool EnableExceptions>
        Color32 get_paletted_color32(int x, int y) const {
                const int pal = get_palette_index(x, y);
                if (pal == -1) {
                        if (EnableExceptions) {
                                throw std::logic_error(
                                        "unsupported bpp for paletted bmp");
                        } else {
                                return Color32();
                        }
                } else if (pal >= colorTable.size()) {
                        if (EnableExceptions) {
                                throw exceptions::palette_index_out_of_range(
                                        pal,
                                        static_cast<int>(colorTable.size()));
                        } else {
                                return Color32();
                        }
                }

                const impl::BitmapColorTable::Entry entry = colorTable[pal];
                return Color32(
                        entry.red,
                        entry.green,
                        entry.blue
                );
        }

        template <bool EnableExceptions>
        Color32 get_rgb_color32(int x, int y) const {
                if (!imageData24bit.empty()) {
                        const color_type_24bit col = imageData24bit(x, y);
                        const uint8_t
                                b = static_cast<uint8_t>(col.value1()),
                                g = static_cast<uint8_t>(col.value2()),
                                r = static_cast<uint8_t>(col.value3());
                        return Color32(r, g, b);
                }

                if (EnableExceptions) {
                        throw std::logic_error(
                                "unsupported bpp for rgb bmp");
                }
                return Color32();
        }
};


// -- Bitmap32 -----------------------------------------------------------------
Bitmap32::Bitmap32(std::istream &is) :
        impl_(new Impl(is))
{}

Bitmap32::~Bitmap32() {
        delete impl_;
}

int Bitmap32::width() const {
        return impl_->width();
}

int Bitmap32::height() const {
        return impl_->height();
}

Color32 Bitmap32::operator()(int x, int y) const {
        return impl_->get32(x, y);
}

Color32 Bitmap32::at(int x, int y) const {
        return impl_->at32(x, y);
}


// -- read_bmp() ---------------------------------------------------------------
Bitmap32* read_bmp32(std::string const &filename) {
        using namespace impl;

        std::ifstream f(filename, std::ios::binary);
        if (!f.is_open())
                throw exceptions::file_not_found(filename);

        Bitmap32 *ret = new Bitmap32(f);
        return ret;
}

}
