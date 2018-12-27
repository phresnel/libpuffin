#include "puffin/bmp.hh"
#include "puffin/impl/bmp_util.hh"
#include "puffin/exceptions.hh"
#include <iostream>
#include <fstream>

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
                        std::cout << "ColorTable:[...]\n";
                } else {
                        std::cout << colorTable;
                }
                std::cout << colorMask;
                if (false) {
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
                int pal = -1;
                if (!imageData1bit.empty()) {
                        pal = imageData1bit(x, y);
                } else if (!imageData2bit.empty()) {
                        pal = imageData2bit(x, y);
                } else if (!imageData4bit.empty()) {
                        pal = imageData4bit(x, y);
                } else if (!imageData8bit.empty()) {
                        pal = imageData8bit(x, y);
                } else {
                        // error
                        return Color32();
                }
                if (-1 == pal) {
                        // error
                        return Color32();
                }

                impl::BitmapColorTable::Entry entry = colorTable[pal];
                return Color32(
                        entry.red,
                        entry.green,
                        entry.blue
                );
        }

        Color32 at32(int x, int y) const {
                int pal = -1;
                if (!imageData1bit.empty()) {
                        pal = imageData1bit(x, y);
                } else if (!imageData2bit.empty()) {
                        pal = imageData2bit(x, y);
                } else if (!imageData4bit.empty()) {
                        pal = imageData4bit(x, y);
                } else if (!imageData8bit.empty()) {
                        pal = imageData8bit(x, y);
                } else if (!imageData24bit.empty()) {
                        pal = imageData24bit(x, y);
                } else {
                        throw std::logic_error("unsupported bpp");
                }
                if (pal < 0 || pal >= 1) {
                        throw exceptions::palette_index_out_of_range(
                                pal,
                                static_cast<int>(colorTable.size()));
                }

                impl::BitmapColorTable::Entry entry = colorTable[pal];
                return Color32(
                        entry.red,
                        entry.green,
                        entry.blue
                );
        }

private:
        std::string filename;
        impl::BitmapHeader header;
        impl::BitmapInfoHeader infoHeader;
        impl::BitmapColorTable colorTable;
        impl::BitmapColorMasks colorMask;
        impl::BitmapImageData<impl::BitmapRowDataPaletted<32, 1>> imageData1bit;
        impl::BitmapImageData<impl::BitmapRowDataPaletted<32, 2>> imageData2bit; // non standard
        impl::BitmapImageData<impl::BitmapRowDataPaletted<32, 4>> imageData4bit;
        impl::BitmapImageData<impl::BitmapRowDataPaletted<32, 8>> imageData8bit;
        impl::BitmapImageData<impl::BitmapRowDataPaletted<32, 24>> imageData24bit;
        // TODO: 32 bit (e.g. as supported by GDI+):
        //       impl::BitmapImageData<64, 16> imageData16bit;
        // TODO: 64 bit (e.g. as supported by GDI+):
        //       impl::BitmapImageData<64, 16> imageData16bit;
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
