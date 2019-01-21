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

        enum {
                size_in_file = (16 + 32 + 16 + 16 + 32) / 8
        };
};

inline
std::ostream &operator<<(std::ostream &os, BitmapHeader const &v) {
        return os << "Header{\n"
                  << "  signature:" << std::hex << v.signature << std::dec
                  << " ('"
                  << static_cast<char>(v.signature >> 8)
                  << static_cast<char>(v.signature & 0xff)
                  << "')\n"
                  << "  size:" << v.size << "\n"
                  << "  reserved1:" << v.reserved1 << "\n"
                  << "  reserved2:" << v.reserved2 << "\n"
                  << "  dataOffset:" << v.dataOffset << "\n"
                  << "}\n";
}

} }
