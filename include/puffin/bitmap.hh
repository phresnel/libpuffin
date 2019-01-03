//------------------------------------------------------------------------------
// BMP support is based, among others, on these:
// * https://docs.microsoft.com/en-us/previous-versions/dd183376(v%3Dvs.85)
// * https://docs.microsoft.com/en-us/windows/desktop/gdi/bitmap-storage
// * http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
// * https://en.wikipedia.org/wiki/BMP_file_format
//------------------------------------------------------------------------------

#ifndef BMP2_HH_INCLUDED_20190102
#define BMP2_HH_INCLUDED_20190102

#include "color.hh"
#include "impl/compiler.hh"
#include <cstdint>
#include <string>
#include <istream>

namespace puffin {

namespace impl { struct Bitmap; }

class Bitmap /* final */ {
public:
        Bitmap(std::istream &);
        ~Bitmap();

        int width() const;
        int height() const;

        int bpp() const;
        bool is_paletted() const;
        bool is_rgb() const;
        bool has_alpha() const;

        Color32 operator() (int x, int y) const;
        Color32 at (int x, int y) const;
private:
        Bitmap(Bitmap const &); // TODO: Define a public copy-ctor
        Bitmap& operator= (Bitmap const &); // TODO: Define a public copy-assgnmt

#if PUFFIN_HAS_MOVE_SEMANTICS
        Bitmap(Bitmap &&); // TODO: Define a public move-ctor
        Bitmap& operator= (Bitmap &&); // TODO: Define a public move-assgnmt
#endif
        impl::Bitmap *impl_;
};

Bitmap* read_bmp(std::string const &filename);
}

#endif //BMP2_HH_INCLUDED_20190102
