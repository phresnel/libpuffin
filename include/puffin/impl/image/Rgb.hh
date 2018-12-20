#ifndef RGB_HH_INCLUDED_20181025
#define RGB_HH_INCLUDED_20181025

#include "math/Real.hh"
#include "tukan/LinearRGB.hh"
#include "tukan/LinearRGBA.hh"

namespace oneiric { namespace image {
using Rgb = tukan::LinearRGB<math::Real, tukan::sRGB>;
using Rgba = tukan::LinearRGBA<math::Real, tukan::sRGB>;
} }

namespace oneiric {
using image::Rgb;
using image::Rgba;
}

#endif // RGB_HH_INCLUDED_20181025
