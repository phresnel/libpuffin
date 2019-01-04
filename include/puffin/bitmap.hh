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

class Bitmap {
public:
        Bitmap(std::istream &);
        ~Bitmap();

        Bitmap(Bitmap const &);
        Bitmap& operator= (Bitmap const &);

        void reset();
        void reset(std::istream &);

        int width() const;
        int height() const;

        int bpp() const;
        bool is_paletted() const;
        bool is_rgb() const;
        bool has_alpha() const;

        unsigned int x_pixels_per_meter() const;
        unsigned int y_pixels_per_meter() const;
        bool has_square_pixels() const;

        Color32 operator() (int x, int y) const;
        Color32 at (int x, int y) const;

        friend std::ostream& operator<< (std::ostream &os, Bitmap const &v);

private:
        impl::Bitmap *impl_;
        Bitmap();
#if PUFFIN_HAS_MOVE_SEMANTICS
        Bitmap(Bitmap &&); // TODO: Define a public move-ctor
        Bitmap& operator= (Bitmap &&); // TODO: Define a public move-assgnmt
#endif
};

class InvalidBitmap {
public:
        InvalidBitmap();
        InvalidBitmap(std::istream &);
        ~InvalidBitmap();

        InvalidBitmap(InvalidBitmap const &);
        InvalidBitmap& operator= (InvalidBitmap const &);

        void reset();
        void reset(std::istream &);
        bool partial_reset(std::istream &);

        int width() const;
        int height() const;

        int bpp() const;
        bool is_paletted() const;
        bool is_rgb() const;
        bool has_alpha() const;
        bool valid() const;

        unsigned int x_pixels_per_meter() const;
        unsigned int y_pixels_per_meter() const;
        bool has_square_pixels() const;

        Color32 operator() (int x, int y) const;
        Color32 at (int x, int y) const;

        friend std::ostream& operator<< (std::ostream &, InvalidBitmap const &);

private:
        impl::Bitmap *impl_;
#if PUFFIN_HAS_MOVE_SEMANTICS
        InvalidBitmap(InvalidBitmap &&); // TODO: Define a public move-ctor
        InvalidBitmap& operator= (InvalidBitmap &&); // TODO: Define a public move-assgnmt
#endif
};


Bitmap read_bmp(std::string const &filename);
InvalidBitmap read_invalid_bmp(std::string const &filename);
}

#endif //BMP2_HH_INCLUDED_20190102
