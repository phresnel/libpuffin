//------------------------------------------------------------------------------
// BMP support is based, among others, on these:
// * https://docs.microsoft.com/en-us/previous-versions/dd183376(v%3Dvs.85)
// * https://docs.microsoft.com/en-us/windows/desktop/gdi/bitmap-storage
// * http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
// * https://en.wikipedia.org/wiki/BMP_file_format
//------------------------------------------------------------------------------

#ifndef BMP_HH_INCLUDED_20181220
#define BMP_HH_INCLUDED_20181220

#include "color.hh"
#include "impl/compiler.hh"
#include <cstdint>
#include <string>
#include <istream>

namespace puffin {

class Bitmap32 /* final */ {
public:
        Bitmap32(std::istream &);
        ~Bitmap32();

        int width() const;
        int height() const;

        Color32 operator() (int x, int y) const;
private:
        Bitmap32(Bitmap32 const &); // TODO: Define a public copy-ctor
        Bitmap32& operator= (Bitmap32 const &); // TODO: Define a public copy-assgnmt

#if PUFFIN_HAS_MOVE_SEMANTICS
        Bitmap32(Bitmap32 &&); // TODO: Define a public move-ctor
        Bitmap32& operator= (Bitmap32 &&); // TODO: Define a public move-assgnmt
#endif

        struct Impl;
        Impl* impl_;
};

Bitmap32* read_bmp32(std::string const &filename);
}

#endif //BMP_HH_INCLUDED_20181220
