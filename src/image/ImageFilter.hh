#ifndef IMAGEFILTER_HH_INCLUDED_20181211
#define IMAGEFILTER_HH_INCLUDED_20181211

#include "coords.hh"
#include "Image.hh"
#include <tuple>

namespace puffin { namespace image {

enum class Filtering {
        Nearest,
        Bilinear,
        // TODO: Anisotropic
};

struct ImageFilter {

        ImageFilter() = default;

        ImageFilter(ImageFilter const &) = default;
        ImageFilter& operator= (ImageFilter const &) = default;

        ImageFilter(ImageFilter &&) = default;
        ImageFilter& operator= (ImageFilter &&) = default;

        ImageFilter(Filtering filt, Wrapping wrap_x, Wrapping wrap_y)
                : filtering_{filt}, wrap_{wrap_x, wrap_y}
        {}

        ImageFilter(Filtering filt, Wrapping wrap)
                : ImageFilter{filt, wrap, wrap}
        {}

        template <typename ImageT>
        auto operator() (
                ImageT const &img,
                Real u,
                Real v
        ) const noexcept -> typename ImageT::value_type {
                switch (filtering_) {
                case Filtering::Bilinear:
                        return bilinear(img, u, v);
                case Filtering::Nearest:
                default:
                        return nearest(img, u, v);
                }
        }

private:
        template <typename ImageT>
        auto nearest(
                ImageT const &img,
                Real u, Real v
        ) const noexcept -> typename ImageT::value_type {
                const auto coords = wrap_(img, {
                        static_cast<int>(round(u * Real{img.width()})),
                        static_cast<int>(round(v * Real{img.height()}))
                });
                return img(coords.x, coords.y);
        }

        template <typename ImageT>
        auto bilinear(
                ImageT const &img,
                Real u, Real v
        ) const noexcept -> typename ImageT::value_type {
                const auto fx = u * Real{img.width()};
                const auto fy = v * Real{img.height()};
                const auto ix = static_cast<int>(floor(fx));
                const auto iy = static_cast<int>(floor(fy));

                const Real frac_x = fx - Real{ix};
                const Real frac_y = fy - Real{iy};

                const Coords
                        w00 {wrap_(img, Coords{ix, iy})},
                        w10 {wrap_(img, Coords{ix+1, iy})},
                        w01 {wrap_(img, Coords{ix, iy+1})},
                        w11 {wrap_(img, Coords{ix+1, iy+1})}
                ;

                const auto
                        C00 = img(w00),
                        C10 = img(w10),
                        C01 = img(w01),
                        C11 = img(w11)
                ;

                const auto A = (1_R-frac_x)*C00 + frac_x*C10;
                const auto B = (1_R-frac_x)*C01 + frac_x*C11;
                const auto C = (1_R-frac_y)*A + frac_y*B;
                return C;
        }

private:
        Filtering filtering_ {Filtering::Nearest};
        CoordWrapper wrap_ {Wrapping::Wrap};
};

} }

#endif // IMAGEFILTER_HH_INCLUDED_20181211
