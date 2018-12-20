#include "puffin/bmp.hh"
#include "puffin/impl/bmp_util.hh"
#include "puffin/exceptions.hh"
#include <iostream>
#include <fstream>

namespace puffin {

void read_bmp(std::string const &filename) {
        using namespace impl;

        std::ifstream f(filename, std::ios::binary);
        if (!f.is_open())
                throw exceptions::file_not_found(filename);

        const BitmapHeader header(f);
        const auto headerPos = f.tellg();
        const BitmapInfoHeader infoHeader(f);
        f.seekg(headerPos); // this seeking should be in the InfoHeader.
        f.seekg(infoHeader.infoHeaderSize, std::ios_base::cur);
        const BitmapColorTable colorTable(infoHeader, f);
        const BitmapColorMask colorMask(infoHeader, f);
        const BitmapImageData<32, 1> imageData1bit(header, infoHeader, f);
        const BitmapImageData<32, 2> imageData2bit(header, infoHeader, f); // non standard
        const BitmapImageData<32, 4> imageData4bit(header, infoHeader, f);
        const BitmapImageData<32, 8> imageData8bit(header, infoHeader, f);

        std::cout << header;
        std::cout << infoHeader;
        std::cout << colorTable;
        std::cout << colorMask;
        std::cout << imageData1bit;
        std::cout << imageData2bit;
        std::cout << imageData4bit;
        std::cout << imageData8bit;
}

}
