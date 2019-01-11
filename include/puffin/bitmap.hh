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
#include <set>

namespace puffin {

// TODO: de-uglify this enum. Should be just ditch C++03?
enum BitmapVersion {
        BMPv_Unknown = 0,

        BMPv_Win_1x, // DDB
        BMPv_Win_2x,
        BMPv_Win_3x,
        BMPv_Win_4x,

        BMPv_WinNT,

        BMPv_OS2_1x,
        BMPv_OS2_2x
};
inline
std::ostream& operator<< (std::ostream &os, BitmapVersion v) {
        switch (v) {
        case BMPv_Unknown: return os << "BMPv_Unknown";
        case BMPv_Win_2x: return os << "BMPv_Win_2x";
        case BMPv_Win_3x: return os << "BMPv_Win_3x";
        case BMPv_Win_4x: return os << "BMPv_Win_4x";
        case BMPv_WinNT: return os << "BMPv_WinNT";
        case BMPv_OS2_1x: return os << "BMPv_OS2_1x";
        case BMPv_OS2_2x: return os << "BMPv_OS2_2x";
        }
        return os << "BMPv_<unkwn" << (int)v << ">";
}
inline
std::ostream& operator<< (std::ostream &os, std::set<BitmapVersion> const& v) {
        bool first = true;
        for (std::set<BitmapVersion>::const_iterator it=v.begin(), end=v.end();
             it != end; ++it
        ) {
                if (!first) {
                        os << " or ";
                }
                os << *it;
                first = false;
        }
        return os;
}

namespace impl { struct Bitmap; }

class Bitmap {
public:
        explicit Bitmap(std::istream &);
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
        std::set<BitmapVersion> version() const;

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
        explicit InvalidBitmap(std::istream &);
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
        std::set<BitmapVersion> version() const;
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
