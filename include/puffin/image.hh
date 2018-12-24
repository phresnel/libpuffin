#ifndef CANVAS_HH_INCLUDED_20181221
#define CANVAS_HH_INCLUDED_20181221

#include "coords.hh"
#include "color.hh"
#include "impl/contract.hh"
#include <vector>

namespace puffin {

class ColorSpace {};

template <typename T>
class base_image final {
public:
        // -- types ------------------------------------------------------------
        using value_type = T;
        using container_type = std::vector<value_type>;

        using size_type = typename container_type::size_type;
        using difference_type = typename container_type::difference_type;

        using reference = typename container_type::reference;
        using const_reference = typename container_type::const_reference;

        using pointer = typename container_type::pointer;
        using const_pointer = typename container_type::const_pointer;

        using iterator = typename container_type::iterator;
        using const_iterator = typename container_type::const_iterator;

        // -- constructors -----------------------------------------------------
        base_image(int width, int height);
        base_image(int width, int height, value_type const &init);

        base_image(base_image const &) = default;
        base_image& operator= (base_image const &) = default;

        base_image(base_image &&) noexcept = default;
        base_image& operator= (base_image &&) noexcept = default;

        ~base_image() = default;

        // -- element access ---------------------------------------------------
        value_type& operator() (int x, int y);
        value_type operator() (int x, int y) const;

        value_type& operator() (Coords const &);
        value_type operator() (Coords const &) const;

        value_type& at(int x, int y);
        value_type at(int x, int y) const;

        value_type& at(Coords const &);
        value_type at(Coords const &) const;

        // -- iterators --------------------------------------------------------
        iterator begin();
        const_iterator begin() const;
        const_iterator cbegin() const;

        iterator end();
        const_iterator end() const;
        const_iterator cend() const;

        // -- capacity ---------------------------------------------------------
        bool empty() const;
        size_type size() const;
        size_type max_size() const;

        // -- dimensions -------------------------------------------------------
        int width() const;
        int height() const;
        size_type stride() const;

        // -- algorithms -------------------------------------------------------
        template <typename RgbFunction>
        void for_each (RgbFunction f);

        template <typename RgbFunction>
        void for_each_2di (RgbFunction f);

        template <typename RgbFunction>
        void for_each_2dr (RgbFunction f);

        template <typename UnaryPredicate>
        void replace_if(UnaryPredicate pred, const_reference newValue);

        void fill(const_reference val);

private:
        int width_ = 0, height_ = 0;
        container_type pixels_;

        void ensureBoundsContract(int x, int y) const {
                namespace cont = impl::contract;
                cont::positive(x);
                cont::less_than(x, width_);
                cont::positive(y);
                cont::less_than(y, height_);
        }
};

template <typename T> inline base_image<T> operator* (base_image<T> canvas, double f);
template <typename T> inline base_image<T> operator* (double f, base_image<T> canvas);
template <typename T> inline base_image<T> operator/ (base_image<T> canvas, double f);
template <typename T> inline base_image<T> operator/ (double f, base_image<T> canvas);
template <typename T> inline base_image<T> operator*= (base_image<T> &canvas, double f);
template <typename T> inline base_image<T> operator/= (base_image<T> &canvas, double f);
template <typename T> inline base_image<T> min(base_image<T> canvas, double f);
template <typename T> inline base_image<T> min(double f, base_image<T> canvas);
template <typename T> inline base_image<T> max(base_image<T> canvas, double f);
template <typename T> inline base_image<T> max(double f, base_image<T> canvas);

template <typename T> inline int width(base_image<T> const &canvas);
template <typename T> inline int height(base_image<T> const &canvas);

template <typename T> inline base_image<T> mirror_x(base_image<T> const &canvas);
template <typename T> inline base_image<T> mirror_y(base_image<T> const &canvas);
template <typename T> inline base_image<T> transpose(base_image<T> const &canvas);
template <typename T> inline base_image<T> rotate90cw(base_image<T> const &canvas);
template <typename T> inline base_image<T> rotate180cw(base_image<T> const &canvas);
template <typename T> inline base_image<T> rotate270cw(base_image<T> const &canvas);
template <typename T> inline base_image<T> rotate90ccw(base_image<T> const &canvas);
template <typename T> inline base_image<T> rotate180ccw(base_image<T> const &canvas);
template <typename T> inline base_image<T> rotate270ccw(base_image<T> const &canvas);
template <typename T> inline base_image<T> copy(base_image<T> const &, Rect const &);

}

//==============================================================================
// Implementation.
//==============================================================================
namespace puffin {

template <typename T>
inline base_image<T>::base_image (int width, int height) :
        width_{support::contract::positive(width)},
        height_{support::contract::positive(height)},
        pixels_{width*height}
{
}

template <typename T>
inline base_image<T>::base_image (
        int width,
        int height,
        value_type const &init
) :
        width_{support::contract::positive(width)},
        height_{support::contract::positive(height)},
        pixels_{width*height, init}
{
}

// -- element access ---------------------------------------------------

template <typename T>
inline auto base_image<T>::operator() (int x, int y) -> value_type& {
        ensureBoundsContract(x, y);
        return pixels_[y*width_ + x];
}

template <typename T>
inline auto base_image<T>::operator() (int x, int y) const -> value_type {
        ensureBoundsContract(x, y);
        return pixels_[y*width_ + x];
}

template <typename T>
inline auto base_image<T>::operator() (Coords const &coords) -> value_type& {
        return (*this)(coords.x, coords.y);
}

template <typename T>
inline auto base_image<T>::operator() (Coords const &coords) const -> value_type {
        return (*this)(coords.x, coords.y);
}

template <typename T>
inline auto base_image<T>::at(int x, int y) -> value_type& {
        ensureBoundsContract(x, y);
        return pixels_[y*width_ + x];
}

template <typename T>
inline auto base_image<T>::at(int x, int y) const -> value_type {
        ensureBoundsContract(x, y);
        return pixels_[y*width_ + x];
}

template <typename T>
inline auto base_image<T>::at(Coords const &coords) -> value_type& {
        return at(coords.x, coords.y);
}

template <typename T>
inline auto base_image<T>::at(Coords const &coords) const -> value_type {
        return at(coords.x, coords.y);
}

// -- iterators --------------------------------------------------------

template <typename T>
inline auto base_image<T>::begin() -> iterator {
        return pixels_.begin();
}

template <typename T>
inline auto base_image<T>::begin() const -> const_iterator {
        return pixels_.begin();
}

template <typename T>
inline auto base_image<T>::cbegin() const -> const_iterator{
        return pixels_.cbegin();
}

template <typename T>
inline auto base_image<T>::end() -> iterator {
        return pixels_.end();
}

template <typename T>
inline auto base_image<T>::end() const -> const_iterator {
        return pixels_.end();
}

template <typename T>
inline auto base_image<T>::cend() const -> const_iterator {
        return pixels_.cend();
}

// -- capacity ---------------------------------------------------------

template <typename T>
inline auto base_image<T>::empty() const -> bool {
        return pixels_.empty();
}

template <typename T>
inline auto base_image<T>::size() const -> size_type {
        return pixels_.size();
}

template <typename T>
inline auto base_image<T>::max_size() const -> size_type {
        return pixels_.max_size();
}

// -- dimensions -------------------------------------------------------

template <typename T>
inline auto base_image<T>::width() const -> int {
        return width_;
}

template <typename T>
inline auto base_image<T>::height() const -> int {
        return height_;
}

template <typename T>
inline auto base_image<T>::stride() const -> size_type {
        return width();
}

// -- algorithms -------------------------------------------------------

template <typename T>
template <typename RgbFunction>
inline auto base_image<T>::for_each (RgbFunction f) -> void {
        std::for_each(begin(), end(), f);
}

template <typename T>
template <typename RgbFunction>
inline auto base_image<T>::for_each_2di (RgbFunction f) -> void {
        for (int y=0; y!=height_; ++y) {
                pointer pixel = pixels_.data() + y*stride();
                for (int x=0; x!=width_; ++x) {
                        f(x, y, *pixel);
                        ++pixel;
                }
        }
}

template <typename T>
template <typename RgbFunction>
inline auto base_image<T>::for_each_2dr (RgbFunction f) -> void {
        using double;
        const auto yStep = double{1} / double{height()};
        const auto xStep = double{1} / double{width()};
        auto fy = double(0);
        for (int y=0; y!=height_; ++y) {
                auto fx = double(0);
                pointer pixel = pixels_.data() + y*stride();
                for (int x=0; x!=width_; ++x) {
                        f(fx, fy, *pixel);
                        ++pixel;
                        fx += xStep;
                }
                fy += yStep;
        }
}

template <typename T>
template <typename UnaryPredicate>
inline auto base_image<T>::replace_if(
        UnaryPredicate pred,
        const_reference newValue
) -> void {
        std::replace_if(begin(), end(), pred, newValue);
}

template <typename T>
inline auto base_image<T>::fill(const_reference val) -> void {
        for_each([=] (reference pixel) {
                pixel = val;
        });
}

template <typename T>
inline auto operator* (base_image<T> canvas, double f) -> base_image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = rgb * f;
        });
        return canvas;
}

template <typename T>
inline auto operator* (double f, base_image<T> canvas) -> base_image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = f * rgb;
        });
        return canvas;
}

template <typename T>
inline auto operator/ (base_image<T> canvas, double f) -> base_image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = rgb / f;
        });
        return canvas;
}

template <typename T>
inline auto operator/ (double f, base_image<T> canvas) -> base_image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = f / rgb;
        });
        return canvas;
}

template <typename T>
inline auto operator*= (base_image<T> &canvas, double f) -> base_image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = rgb * f;
        });
        return canvas;
}

template <typename T>
inline auto operator/= (base_image<T> &canvas, double f) -> base_image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = rgb / f;
        });
        return canvas;
}

template <typename T>
inline auto min (base_image<T> canvas, double f) -> base_image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = min(rgb, f);
        });
        return canvas;
}

template <typename T>
inline auto min (double f, base_image<T> canvas) -> base_image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = min(f, rgb);
        });
        return canvas;
}

template <typename T>
inline auto max (base_image<T> canvas, double f) -> base_image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = max(rgb, f);
        });
        return canvas;
}

template <typename T>
inline auto max (double f, base_image<T> canvas) -> base_image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = max(f, rgb);
        });
        return canvas;
}

template <typename T>
inline auto width (base_image<T> const &canvas) -> int {
        return canvas.width();
}

template <typename T>
inline auto height (base_image<T> const &canvas) -> int {
        return canvas.height();
}

template <typename T>
inline auto mirror_x(base_image<T> const &canvas) -> base_image<T> {
        base_image<T> ret {canvas.width(), canvas.height()};
        const auto m = canvas.width() - 1;
        ret.for_each_2di([&] (auto x, auto y, T &p) {
                p = canvas(m - x, y);
        });
        return ret;
}

template <typename T>
inline auto mirror_y(base_image<T> const &canvas) -> base_image<T> {
        base_image<T> ret {canvas.width(), canvas.height()};
        const auto m = canvas.height() - 1;
        ret.for_each_2di([&] (auto x, auto y, T &p) {
                p = canvas(x, m - y);
        });
        return ret;
}

template <typename T>
inline auto transpose(base_image<T> const &canvas) -> base_image<T> {
        base_image<T> ret {canvas.height(), canvas.width()};
        ret.for_each_2di([&] (auto x, auto y, T &p) {
                p = canvas(y, x);
        });
        return ret;
}

template <typename T>
inline auto rotate90cw(base_image<T> const &canvas) -> base_image<T> {
        // [00 10 20]             [00 01 02]            [02 01 00]
        // [01 11 21], transpose: [10 11 12], mirror_x: [12 11 10]
        // [02 12 22]             [20 21 22]            [22 21 20]
        base_image<T> ret {canvas.height(), canvas.width()};
        const auto m = canvas.height() - 1;
        ret.for_each_2di([&] (auto x, auto y, T &p) {
                p = canvas(y, m - x);
        });
        return ret;
}

template <typename T>
inline auto rotate180cw(base_image<T> const &canvas) -> base_image<T> {
        base_image<T> ret {canvas.width(), canvas.height()};
        const auto mw = canvas.width() - 1;
        const auto mh = canvas.height() - 1;
        ret.for_each_2di([&] (auto x, auto y, T &p) {
                p = canvas(mw - x, mh - y);
        });
        return ret;
}

template <typename T>
inline auto rotate270cw(base_image<T> const &canvas) -> base_image<T> {
        base_image<T> ret {canvas.height(), canvas.width()};
        const auto m = canvas.width() - 1;
        ret.for_each_2di([&] (auto x, auto y, T &p) {
                p = canvas(m - y, x);
        });
        return ret;
}

template <typename T>
inline auto rotate90ccw(base_image<T> const &canvas) -> base_image<T> {
        return rotate270cw(canvas);
}

template <typename T>
inline auto rotate180ccw(base_image<T> const &canvas) -> base_image<T> {
        return rotate180cw(canvas);
}

template <typename T>
inline auto rotate270ccw(base_image<T> const &canvas) -> base_image<T> {
        return rotate90cw(canvas);
}

template <typename T>
inline base_image<T> copy(base_image<T> const &canvas, Rect const &rect) {
        base_image<T> ret {rect.width(), rect.height()};
        for (int y=rect.top(); y!=rect.bottom(); y++) {
                for (int x=rect.left(); x!=rect.right(); x++) {
                        ret(x-rect.left(), y-rect.top()) = canvas(x, y);
                }
        }
        return ret;
}
}


// ImageFilter
namespace puffin {

enum class Filtering {
        Nearest,
        Bilinear,
        // TODO: Anisotropic
};

struct ImageFilter {

        ImageFilter() = default;
        ImageFilter(ImageFilter const &) = default;
        ImageFilter &operator=(ImageFilter const &) = default;
        ImageFilter(ImageFilter &&) = default;
        ImageFilter &operator=(ImageFilter &&) = default;

        ImageFilter(Filtering filt, Wrapping wrap_x, Wrapping wrap_y)
                : filtering_{filt}, wrap_{wrap_x, wrap_y} {}

        ImageFilter(Filtering filt, Wrapping wrap)
                : ImageFilter{filt, wrap, wrap} {}

        template<typename ImageT>
        auto operator()(
                ImageT const &img,
                double u,
                double v
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
        template<typename ImageT>
        auto nearest(
                ImageT const &img,
                double u, double v
        ) const noexcept -> typename ImageT::value_type {
                const auto coords = wrap_(img, {
                        static_cast<int>(round(u * double{img.width()})),
                        static_cast<int>(round(v * double{img.height()}))
                });
                return img(coords.x, coords.y);
        }

        template<typename ImageT>
        auto bilinear(
                ImageT const &img,
                double u, double v
        ) const noexcept -> typename ImageT::value_type {
                const auto fx = u * double{img.width()};
                const auto fy = v * double{img.height()};
                const auto ix = static_cast<int>(floor(fx));
                const auto iy = static_cast<int>(floor(fy));

                const double frac_x = fx - double{ix};
                const double frac_y = fy - double{iy};

                const Coords
                        w00{wrap_(img, Coords{ix, iy})},
                        w10{wrap_(img, Coords{ix + 1, iy})},
                        w01{wrap_(img, Coords{ix, iy + 1})},
                        w11{wrap_(img, Coords{ix + 1, iy + 1})};

                const auto
                        C00 = img(w00),
                        C10 = img(w10),
                        C01 = img(w01),
                        C11 = img(w11);

                const auto A = (1_R - frac_x) * C00 + frac_x * C10;
                const auto B = (1_R - frac_x) * C01 + frac_x * C11;
                const auto C = (1_R - frac_y) * A + frac_y * B;
                return C;
        }

private:
        Filtering filtering_{Filtering::Nearest};
        CoordWrapper wrap_{Wrapping::Wrap};
};

typedef basic_rgba<uint16_t, uint16_t, uint16_t, uint16_t> Rgba64;
typedef base_image<Rgba64> Image64;
}
#endif //CANVAS_HH_INCLUDED_20181221
