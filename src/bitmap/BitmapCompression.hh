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
        case BI_RGB:
                return "BI_RGB";
        case BI_RLE8:
                return "BI_RLE8";
        case BI_RLE4:
                return "BI_RLE4";
        case BI_BITFIELDS:
                return "BI_BITFIELDS";
        case BI_JPEG:
                return "BI_JPEG";
        case BI_PNG:
                return "BI_PNG";
        case BI_CMYK:
                return "BI_CMYK";
        case BI_CMYKRLE8:
                return "BI_CMYKRLE8";
        case BI_CMYKRLE4:
                return "BI_CMYKRLE4";
        default:
                return "<unknown>";
        }
}

inline
std::ostream &operator<<(std::ostream &os, BitmapCompression bc) {
        return os << to_string(bc);
}

} }
