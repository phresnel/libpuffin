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

struct BitmapColorTable {
        BitmapColorTable() {}

        BitmapColorTable(
                BitmapHeader const &header,
                BitmapInfoHeader const &infoHeader,
                std::set<BitmapVersion> const &v,
                std::istream &f
        ) {
                reset(header, infoHeader, v, f);
        }

        void reset(
                BitmapHeader const &header,
                BitmapInfoHeader const &infoHeader,
                std::set<BitmapVersion> const &v,
                std::istream &f
        ) {
                entries_ = readEntries(header, infoHeader, v, f);
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
                BitmapHeader const &header,
                BitmapInfoHeader const &infoHeader,
                std::set<BitmapVersion> const &v,
                std::istream &f
        ) {
                const uint32_t size = computeSize(header, infoHeader, v);
                const bool fourChannel = isFourChannel(v);

                std::vector<Color32> ret;
                ret.reserve(size);
                for (uint32_t i = 0; i < size; ++i) {
                        const uint8_t blue = impl::read_uint8_le(f),
                                green = impl::read_uint8_le(f),
                                red = impl::read_uint8_le(f);
                        if (fourChannel) {
                                impl::read_uint8_le(f); // reserved
                        }
                        ret.push_back(Color32(red, green, blue));
                }
                return ret;
        }

        static bool isFourChannel(std::set<BitmapVersion> const &v) {
                const bool not_win_2 = v.find(BMPv_Win_2x) == v.end(),
                        not_os2_1 = v.find(BMPv_OS2_1x) == v.end();
                return not_win_2 && not_os2_1;

        }

        static uint32_t computeSize(
                BitmapHeader const &header,
                BitmapInfoHeader const &infoHeader,
                std::set<BitmapVersion> const &v
        ) {
                /*
                const uint32_t decl =
                        infoHeader.colorsUsed != 0 ? infoHeader.colorsUsed :
                        infoHeader.bitsPerPixel == 1 ? 2 :
                        infoHeader.bitsPerPixel == 2 ? 4 :
                        infoHeader.bitsPerPixel == 4 ? 16 :
                        infoHeader.bitsPerPixel == 8 ? 256 : 0;
                */

                const uint32_t
                        headersSize = BitmapHeader::size_in_file +
                                      infoHeader.infoHeaderSize,
                        tableSize = header.dataOffset - headersSize,
                        elemSize = isFourChannel(v) ? 4 : 3;
                return tableSize / elemSize;
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

        //for (int i=0; i!=v.size(); ++i) {
        //        os << "  [" << std::setw(3) << i << "]:" << v[i] << "\n";
        //}
        os << "  size:" << v.size() << "\n";
        os << "}\n";
        return os;
}

} }
