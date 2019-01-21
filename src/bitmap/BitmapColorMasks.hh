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

} }
