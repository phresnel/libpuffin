//------------------------------------------------------------------------------
// BMP support is based, among others, on these:
// * https://docs.microsoft.com/en-us/previous-versions/dd183376(v%3Dvs.85)
// * https://docs.microsoft.com/en-us/windows/desktop/gdi/bitmap-storage
// * http://www.ece.ualberta.ca/~elliott/ee552/studentAppNotes/2003_w/misc/bmp_file_format/bmp_file_format.htm
// * https://en.wikipedia.org/wiki/BMP_file_format
//------------------------------------------------------------------------------

#ifndef BMP_HH_INCLUDED_20181220
#define BMP_HH_INCLUDED_20181220

#include <cstdint>
#include <string>
#include "image.hh"

namespace puffin {
/*Image64*/ void read_bmp(std::string const &filename);
}

#endif //BMP_HH_INCLUDED_20181220
