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

} }
