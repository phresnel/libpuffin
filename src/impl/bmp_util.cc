#include "puffin/bmp.hh"
#include "puffin/impl/bmp_util.hh"
#include "puffin/impl/io_util.hh"
#include "puffin/exceptions.hh"

#include <bitset>

#include <iomanip>
#include <fstream>

namespace puffin { namespace impl {

BitmapHeader::BitmapHeader() :
        signature(0),
        size(0),
        reserved1(0),
        reserved2(0),
        dataOffset(0)
{
}

BitmapHeader::BitmapHeader(std::istream &f) {
        reset(f);
}

void BitmapHeader::reset(std::istream &f) {
        signature = impl::read_uint16_be(f);
        size = impl::read_uint32_le(f);
        reserved1 = impl::read_uint16_le(f);
        reserved2 = impl::read_uint16_le(f);
        dataOffset = impl::read_uint32_le(f);
}

BitmapInfoHeader::BitmapInfoHeader() :
        infoHeaderSize(0),
        width(0),
        height(0),
        planes(0),
        bitsPerPixel(0),
        compression(0),
        compressedImageSize(0),
        xPixelsPerMeter(0),
        yPixelsPerMeter(0),
        colorsUsed(0),
        importantColors(0),
        isBottomUp(0)
{
}

BitmapInfoHeader::BitmapInfoHeader(
//        BitmapHeader const &header, // TODO: should be here, but need an init function to properly tellg/seekg
        std::istream &f
) {
        reset(f);
}
void BitmapInfoHeader::reset(
//        BitmapHeader const &header, // TODO: should be here, but need an init function to properly tellg/seekg
        std::istream &f
) {
        infoHeaderSize = impl::read_uint32_le(f);
        width = impl::read_uint32_le(f);
        height = impl::read_int32_le(f);
        planes = impl::read_uint16_le(f);
        bitsPerPixel = impl::read_uint16_le(f);
        compression = impl::read_uint32_le(f);
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

BitmapColorTable::BitmapColorTable() {
}

BitmapColorTable::BitmapColorTable(
        puffin::impl::BitmapInfoHeader const &info,
        std::istream &f
) :
        entries_(readEntries(info, f))
{
        reset(info, f);
}

void BitmapColorTable::reset(
        puffin::impl::BitmapInfoHeader const &info,
        std::istream &f
) {
        entries_ = readEntries(info, f);
}

std::vector<BitmapColorTable::Entry>
BitmapColorTable::readEntries(BitmapInfoHeader const &info, std::istream &f) {
        const auto numColors = info.colorsUsed != 0 ? info.colorsUsed :
                               info.bitsPerPixel == 1 ? 2 :
                               info.bitsPerPixel == 2 ? 4 :
                               info.bitsPerPixel == 4 ? 16 :
                               info.bitsPerPixel == 8 ? 256 : 0;

        std::vector<Entry> ret;
        ret.reserve(numColors);
        // TODO: Probably need to rework the table size determination
        for (unsigned int i=0; i<numColors; ++i) {
                ret.push_back(Entry{f});
        }
        return ret;
}

BitmapColorTable::Entry::Entry() :
        blue(0),
        green(0),
        red(0)
{
}

BitmapColorTable::Entry::Entry(std::istream &f)
{
        reset(f);
}

void BitmapColorTable::Entry::reset(std::istream &f) {
        blue = impl::read_uint8_le(f);
        green = impl::read_uint8_le(f);
        red = impl::read_uint8_le(f);
        reserved = impl::read_uint8_le(f);
}

BitmapColorMasks::BitmapColorMasks() :
        blue(0),
        green(0),
        red(0)
{
}

BitmapColorMasks::BitmapColorMasks(
        puffin::impl::BitmapInfoHeader const &info,
        std::istream &f
) {
        reset(info, f);
}

void BitmapColorMasks::reset(
        puffin::impl::BitmapInfoHeader const &info,
        std::istream &f
) {
        if (info.compression != BitmapCompression::BI_BITFIELDS)
                return;
        blue = impl::read_uint32_le(f);
        green = impl::read_uint32_le(f);
        red = impl::read_uint32_le(f);
}

} }
