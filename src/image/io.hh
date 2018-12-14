#ifndef IO_HH_INCLUDED_20181130
#define IO_HH_INCLUDED_20181130

#include "Image.hh"

namespace puffin { namespace image {

RgbImage loadRgbImageFromFile(std::string const &filename);
RgbaImage loadRgbaImageFromFile(std::string const &filename);

} }

#endif //IO_HH_INCLUDED_20181130
