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

#include "bitmap/BitmapCompression.hh"
#include "bitmap/BitmapHeader.hh"
#include "bitmap/BitmapInfoHeader.hh"
#include "bitmap/determineBitmapVersion.hh"
#include "bitmap/BitmapColorMasks.hh"
#include "bitmap/BitmapColorTable.hh"
#include "bitmap/BitmapRowData.hh"
#include "bitmap/BitmapImageData.hh"
#include "bitmap/Bitmap.hh"

namespace puffin {

// -- class Bitmap -------------------------------------------------------------
Bitmap::Bitmap() :
        impl_(new impl::Bitmap())
{
}

Bitmap::Bitmap(std::istream &f) :
        impl_(new impl::Bitmap(f))
{
}

Bitmap::~Bitmap() {
        delete impl_;
}

Bitmap::Bitmap(Bitmap const &v) :
        impl_(new impl::Bitmap(*v.impl_))
{
}

Bitmap& Bitmap::operator= (Bitmap const &v) {
        using std::swap;
        Bitmap tmp(v);
        swap(*this, tmp);
        return *this;
}

void Bitmap::reset() {
        using std::swap;
        Bitmap tmp;
        swap(*this, tmp);
}

void Bitmap::reset(std::istream &f) {
        impl_->reset(f);
}

int Bitmap::width() const {
        return impl_->width();
}

int Bitmap::height() const {
        return impl_->height();
}

int Bitmap::bpp() const {
        return impl_->bpp();
}

bool Bitmap::is_paletted() const {
        return impl_->is_paletted();
}

bool Bitmap::is_rgb() const {
        return impl_->is_rgb();
}

bool Bitmap::has_alpha() const {
        return impl_->has_alpha();
}

unsigned int Bitmap::x_pixels_per_meter() const {
        return impl_->x_pixels_per_meter();
}

unsigned int Bitmap::y_pixels_per_meter() const {
        return impl_->y_pixels_per_meter();
}

bool Bitmap::has_square_pixels() const {
        return impl_->has_square_pixels();
}

std::set<BitmapVersion> Bitmap::version() const {
        return impl_->version();
}

Color32 Bitmap::operator()(int x, int y) const {
        return impl_->get32(x, y);
}

Color32 Bitmap::at(int x, int y) const {
        return impl_->at32(x, y);
}

std::ostream& operator<< (std::ostream &os, Bitmap const &v) {
        return os << *v.impl_;
}

// -- class InvalidBitmap ------------------------------------------------------
InvalidBitmap::InvalidBitmap() :
        impl_(new impl::Bitmap())
{
}

InvalidBitmap::InvalidBitmap(std::istream &f) :
        impl_(new impl::Bitmap(f))
{
}

InvalidBitmap::~InvalidBitmap() {
        delete impl_;
}

InvalidBitmap::InvalidBitmap(InvalidBitmap const &v) :
        impl_(new impl::Bitmap(*v.impl_))
{
}

InvalidBitmap& InvalidBitmap::operator= (InvalidBitmap const &v) {
        using std::swap;
        InvalidBitmap tmp(v);
        swap(*this, tmp);
        return *this;
}

void InvalidBitmap::reset() {
        using std::swap;
        InvalidBitmap tmp;
        swap(*this, tmp);
}

void InvalidBitmap::reset(std::istream &f) {
        impl_->reset(f);
}

bool InvalidBitmap::partial_reset(std::istream &f) {
        return impl_->partial_reset(f);
}

int InvalidBitmap::width() const {
        return impl_->width();
}

int InvalidBitmap::height() const {
        return impl_->height();
}

int InvalidBitmap::bpp() const {
        return impl_->bpp();
}

bool InvalidBitmap::is_paletted() const {
        return impl_->is_paletted();
}

bool InvalidBitmap::is_rgb() const {
        return impl_->is_rgb();
}

bool InvalidBitmap::has_alpha() const {
        return impl_->has_alpha();
}

bool InvalidBitmap::valid() const {
        return impl_->valid();
}

unsigned int InvalidBitmap::x_pixels_per_meter() const {
        return impl_->x_pixels_per_meter();
}

unsigned int InvalidBitmap::y_pixels_per_meter() const {
        return impl_->y_pixels_per_meter();
}

bool InvalidBitmap::has_square_pixels() const {
        return impl_->has_square_pixels();
}

std::set<BitmapVersion> InvalidBitmap::version() const {
        return impl_->version();
}

Color32 InvalidBitmap::operator()(int x, int y) const {
        return impl_->get32(x, y);
}

Color32 InvalidBitmap::at(int x, int y) const {
        return impl_->at32(x, y);
}

std::ostream& operator<< (std::ostream &os, InvalidBitmap const &v) {
        return os << *v.impl_;
}


// -- read_bmp() ---------------------------------------------------------------
Bitmap read_bmp(std::string const &filename) {
        std::ifstream f(filename, std::ios::binary);
        if (!f.is_open())
                throw exceptions::file_not_found(filename);
        return Bitmap(f);
}

InvalidBitmap read_invalid_bmp(std::string const &filename) {
        std::ifstream f(filename, std::ios::binary);
        if (!f.is_open())
                return InvalidBitmap();
        InvalidBitmap ret;
        ret.partial_reset(f);
        return ret;
}

}

// TODO: See http://www.fileformat.info/format/bmp/egff.htm:
//       "Windows BMP File Types
//
//        Each new version of BMP has added new information to the bitmap header. In
//        some cases, the newer versions have changed the size of the color palette
//        and added features to the format itself. Unfortunately, a field wasn't
//        included in the header to easily indicate the version of the file's format
//        or the type of operating system that created the BMP file. If we add
//        Windows' four versions of BMP to OS/2's two versions--each with four
//        possible variations--we find that as many as twelve different related file
//        formats all have the file extension ".BMP".
//
//        It is clear that you cannot know the internal format of a BMP file based on
//        the file extension alone. But, fortunately, you can use a short algorithm to
//        determine the internal format of BMP files.
//
//        The FileType field of the file header is where we start. If these two byte
//        values are 424Dh ("BM"), then you have a single-image BMP file that may have
//        been created under Windows or OS/2. If FileType is the value 4142h ("BA"),
//        then you have an OS/2 bitmap array file. Other OS/2 BMP variations have the
//        file extensions .ICO and .PTR.
//
//        If your file type is "BM", then you must now read the Size field of the
//        bitmap header to determine the version of the file. Size will be 12 for
//        Windows 2.x BMP and OS/2 1.x BMP, 40 for Windows 3.x and Windows NT BMP, 12
//        to 64 for OS/2 2.x BMP, and 108 for Windows 4.x BMP. A Windows NT BMP file
//        will always have a Compression value of 3; otherwise, it is read as a Windows
//        3.x BMP file.
//
//        Note that the only difference between Windows 2.x BMP and OS/2 1.x BMP is
//        the data type of the Width and Height fields. For Windows 2.x, they are
//        signed shorts and for OS/2 1.x, they are unsigned shorts. Windows 3.x,
//        Windows NT, and OS/2 2.x BMP files only vary in the number of fields in the
//        bitmap header and in the interpretation of the Compression field."


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Implementer's notes.
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// On "pixel-endianness"
// ------------------------
//
// Given a 4-byte chunk of pixel data, the pixel value with the smallest
// x-coordinate is left-most when read unaltered from a BMP file.
//
// This is sub-optimal, as this imposes an additional subtraction
// (yes, I was tempted to write "imposes subtraction addition", but
// I dislike comment humor - irony is alright, tho.)
//
// Now for an explanation:
//
// Given
//
//    chunk size: 16 bits
//    pixel size:  4 bist,
//
// we get this layout and pixel equations:
//
//         p0   p1   p2   p3
//        [1111 0110 0010 0000]
//
//        p0 =  (chunk>>12) & 1111b = (chunk>>(16-4 - 0*4)) & 1111b
//        p1 =  (chunk>> 8) & 1111b = (chunk>>(16-4 - 1*4)) & 1111b
//        p2 =  (chunk>> 4) & 1111b = (chunk>>(16-4 - 2*4)) & 1111b
//        p3 =  (chunk>> 0) & 1111b = (chunk>>(16-4 - 3*4)) & 1111b.
//
// But with "pixel-endianness" flipped, the layout and equations
//
//         p3   p2   p1   p0
//        [0000 0010 0110 1111]
//
//        p0 =  (chunk>> 0) & 1111b = (chunk>>(0*4)) & 1111b
//        p1 =  (chunk>> 4) & 1111b = (chunk>>(1*4)) & 1111b
//        p2 =  (chunk>> 8) & 1111b = (chunk>>(2*4)) & 1111b
//        p3 =  (chunk>>12) & 1111b = (chunk>>(3*4)) & 1111b
//
// spare us a confusing shift and a subtraction.
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
