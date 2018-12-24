#ifndef COORD_HH_INCLUDED_20181211
#define COORD_HH_INCLUDED_20181211

#include "impl/algorithm.hh"

namespace puffin {

// -- Coords -------------------------------------------------------------------
struct Coords {
        int x, y;
};
constexpr Coords min(Coords const &a, Coords const &b) noexcept {
        return {
                a.x < b.x ? a.x : b.x,
                a.y < b.y ? a.y : b.y,
        };
}
constexpr Coords max(Coords const &a, Coords const &b) noexcept {
        return {
                a.x > b.x ? a.x : b.x,
                a.y > b.y ? a.y : b.y,
        };
}


// -- Screen -------------------------------------------------------------------
struct Screen {
        int width, height;
};
constexpr int width(Screen const &r) noexcept { return r.width; }
constexpr int height(Screen const &r) noexcept { return r.height; }


// -- Rect ---------------------------------------------------------------------
class Rect {
public:
        constexpr Rect(Coords const &a, Coords const &b) noexcept
        : min_{puffin::min(a,b)}, max_{puffin::max(a,b)}
        {}

        constexpr int left() const noexcept { return min_.x; }
        constexpr int top() const noexcept { return min_.y; }
        constexpr int right() const noexcept { return max_.x; }
        constexpr int bottom() const noexcept { return max_.y; }

        constexpr int width() const noexcept { return right() - left(); }
        constexpr int height() const noexcept { return bottom() - top(); }

        constexpr Coords min() const noexcept { return min_; }
        constexpr Coords max() const noexcept { return max_; }
private:
        Coords min_, max_;
};
constexpr int width(Rect const &r) noexcept { return r.width(); }
constexpr int height(Rect const &r) noexcept { return r.height(); }


// -- wrapping algorithms ------------------------------------------------------
// wrap
template <typename ScreenT>
constexpr int wrap_x(ScreenT const &screen, int x) noexcept {
        using puffin::impl::wrap;
        return wrap(x, width(screen)-1);
}
template <typename ScreenT>
constexpr int wrap_y(ScreenT const &screen, int y) noexcept {
        using puffin::impl;
        return wrap(y, height(screen)-1);
}
template <typename ScreenT>
constexpr Coords wrap(ScreenT const &screen, Coords const &coords) noexcept {
        return {wrap_x(coords.x, screen),
                wrap_y(coords.y, screen)};
}
// clamp
template <typename ScreenT>
constexpr int clamp_x(ScreenT const &screen, int x) noexcept {
        using puffin::impl::clamp;
        return clamp(x, width(screen)-1);
}
template <typename ScreenT>
constexpr int clamp_y(ScreenT const &screen, int y) noexcept {
        using puffin::impl::clamp;
        return clamp(y, height(screen)-1);
}
template <typename ScreenT>
constexpr Coords clamp(ScreenT const &screen, Coords const &coords) noexcept {
        return {clamp_x(coords.x, screen),
                clamp_y(coords.y, screen)};
}
// mirror
template <typename ScreenT>
constexpr int mirror_x(ScreenT const &screen, int x) noexcept {
        using puffin::impl::mirror;
        return mirror(x, width(screen)-1);
}
template <typename ScreenT>
constexpr int mirror_y(ScreenT const &screen, int y) noexcept {
        using puffin::impl::mirror;
        return mirror(y, height(screen)-1);
}
template <typename ScreenT>
constexpr Coords mirror(ScreenT const &screen, Coords const &coords) noexcept {
        return {mirror_x(coords.x, screen),
                mirror_y(coords.y, screen)};
}
enum class Wrapping {
        Wrap,
        Mirror,
        Clamp
};
struct CoordWrapper {
        Wrapping wrapping_x = Wrapping::Wrap;
        Wrapping wrapping_y = Wrapping::Wrap;

        CoordWrapper() = default;

        CoordWrapper(Wrapping wx, Wrapping wy) :
                wrapping_x{wx}, wrapping_y{wy}
        {}

        CoordWrapper(Wrapping w) :
                CoordWrapper{w, w}
        {}

        template <typename ScreenT>
        constexpr Coords operator() (
                ScreenT const &screen,
                Coords const &coords
        ) const noexcept {
                int x = 0;
                switch (wrapping_x) {
                case Wrapping::Mirror:
                        x = mirror_x(screen, coords.x);
                        break;
                case Wrapping::Clamp:
                        x = clamp_x(screen, coords.x);
                        break;
                case Wrapping::Wrap:
                default:
                        x = wrap_x(screen, coords.x);
                        break;
                }

                int y = 0;
                switch (wrapping_y) {
                case Wrapping::Mirror:
                        y = mirror_y(screen, coords.y);
                        break;
                case Wrapping::Clamp:
                        y = clamp_y(screen, coords.y);
                        break;
                case Wrapping::Wrap:
                default:
                        y = wrap_y(screen, coords.y);
                        break;
                }

                return Coords{x, y};
        }
};

}

#endif //COORD_HH_INCLUDED_20181211
