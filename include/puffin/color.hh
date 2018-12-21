#ifndef COLOR_HH_INCLUDED_20181221
#define COLOR_HH_INCLUDED_20181221

#include "impl/type_traits.hh"
#include "impl/compiler.hh"
#include <limits>

namespace puffin {

// __ rgba_scalar_traits<T> ____________________________________________________
template <typename ScalarT, typename=void>
struct rgba_scalar_traits;

template <typename ScalarT>
struct rgba_scalar_traits<
        ScalarT,
        typename impl::enable_if<
                impl::is_floating_point<ScalarT>::value
        >::type
> {
        typedef ScalarT value_type;

        static value_type alpha_invisible() { return value_type(0); }
        static value_type alpha_opaque()    { return value_type(1); }

        static value_type color_black()     { return value_type(0); }
        static value_type color_white()     { return value_type(1); }

        static value_type alpha_default()   { return alpha_opaque(); }
        static value_type color_default()   { return color_black(); }
};

template <typename ScalarT>
struct rgba_scalar_traits<
        ScalarT,
        typename impl::enable_if<
                impl::is_unsigned_integer<ScalarT>::value
        >::type
> {
        typedef ScalarT value_type;

        static value_type alpha_invisible() { return numeric_limits::min(); }
        static value_type alpha_opaque()    { return numeric_limits::max(); }

        static value_type color_black()     { return numeric_limits::min(); }
        static value_type color_white()     { return numeric_limits::max(); }

        static value_type alpha_default()   { return alpha_opaque(); }
        static value_type color_default()   { return color_black(); }
private:
        typedef std::numeric_limits<value_type> numeric_limits;
};

// __ basic_rgba_base<T> _______________________________________________________
namespace impl {
template<typename RedT, typename GreenT, typename BlueT, typename AlphaT>
struct basic_rgba_base {
        PUFFIN_DECLARE_COMPILE_TIME_INT(bool, rgb_have_common_type,  false)
        PUFFIN_DECLARE_COMPILE_TIME_INT(bool, rgba_have_common_type, false)
};

template<typename RgbCommonT, typename AlphaT>
struct basic_rgba_base<RgbCommonT, RgbCommonT, RgbCommonT, AlphaT> {
        typedef RgbCommonT rgb_common_type;

        PUFFIN_DECLARE_COMPILE_TIME_INT(bool, rgb_have_common_type,  true)
        PUFFIN_DECLARE_COMPILE_TIME_INT(bool, rgba_have_common_type, false)
};

template<typename RgbaCommonT>
struct basic_rgba_base<RgbaCommonT, RgbaCommonT, RgbaCommonT, RgbaCommonT> {
        typedef RgbaCommonT      rgba_common_type;
        typedef rgba_common_type rgb_common_type;

        PUFFIN_DECLARE_COMPILE_TIME_INT(bool, rgb_have_common_type,  true)
        PUFFIN_DECLARE_COMPILE_TIME_INT(bool, rgba_have_common_type, true)
};
}

// __ basic_rgba<T> ____________________________________________________________
template <typename RedT, typename GreenT, typename BlueT, typename AlphaT>
struct basic_rgba
        : impl::basic_rgba_base<RedT, GreenT, BlueT, AlphaT>
{
        // -- types ------------------------------------------------------------
        typedef RedT   red_type;
        typedef GreenT green_type;
        typedef BlueT  blue_type;
        typedef AlphaT alpha_type;

        typedef rgba_scalar_traits<red_type>   red_traits_type;
        typedef rgba_scalar_traits<green_type> green_traits_type;
        typedef rgba_scalar_traits<blue_type>  blue_traits_type;
        typedef rgba_scalar_traits<alpha_type> alpha_traits_type;

        // -- conditional definitions ------------------------------------------
        //
        // type rgb_common_type:
        //   - condition: RedT, GreenT and BlueT are the same.
        //   - value: identical to RedT, GreenT and BlueT
        //
        // type rgba_common_type:
        //   - condition: RedT, GreenT, BlueT and AlphaT are the same.
        //   - value: identical to RedT, GreenT, BlueT and AlphaT
        //
        // constant rgb_have_common_type =
        //   - true, if RedT, GreenT and BlueT are the same,
        //   - false, otherwise.
        // rgba_have_common_type =
        //   - true, if RedT, GreenT, BlueT and AlphaT are the same,
        //   - false, otherwise.
        //

        // -- fields -----------------------------------------------------------
        red_type   r;
        green_type g;
        blue_type  b;
        alpha_type a;

        // -- ctor/dtor/copy/move ----------------------------------------------
        basic_rgba(basic_rgba const &v) :
                r(v.r),
                g(v.g),
                b(v.b),
                b(v.b)
        {}

        basic_rgba& operator= (basic_rgba const &v) {
                r = v.r;
                g = v.g;
                b = v.b;
                a = v.a;
                return *this;
        }

#if PUFFIN_HAS_MOVE_SEMANTICS
        basic_rgba(basic_rgba &&v) :
                r(std::move(v.r)),
                g(std::move(v.g)),
                b(std::move(v.b)),
                b(std::move(v.b))
        {}

        basic_rgba& operator= (basic_rgba &&v) {
                r = std::move(v.r);
                g = std::move(v.g);
                b = std::move(v.b);
                a = std::move(v.a);
                return *this;
        }
#endif

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // basic_rgba() -> {default, default, default, default}
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        basic_rgba() :
                r(red_traits_type::color_default()),
                g(green_traits_type::color_default()),
                b(blue_traits_type::color_default()),
                a(alpha_traits_type::alpha_default())
        {}

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // basic_rgba(gray, alpha = opage) -> {gray, gray, gray, alpha}
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // remarks:
        //   - only enabled if r, g and b have same type
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        template< typename =
            typename impl::enable_if<basic_rgba::rgb_have_common_type>::type >
        explicit basic_rgba(
                red_type   v,
                alpha_type a = alpha_traits_type::alpha_default()
        ) :
                r(v), g(v), b(v), a(a)
        {}

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // basic_rgba(r, g, b, alpha = opague) -> {r, g, b, alpha}
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        basic_rgba(
                red_type   r,
                green_type g,
                blue_type  b,
                alpha_type a = alpha_traits_type::alpha_default()
        ) :
                r(r), g(g), b(b), a(a)
        {}
};

}

#endif // COLOR_HH_INCLUDED_20181221
