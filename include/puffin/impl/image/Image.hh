#ifndef CANVAS_HH_INCLUDED_20181025
#define CANVAS_HH_INCLUDED_20181025

#include "coords.hh"
#include "Rgb.hh"
#include <vector>

namespace puffin { namespace image {

template <typename T>
class Image final {
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
        Image(int width, int height);
        Image(int width, int height, value_type const &init);

        Image(Image const &) = default;
        Image& operator= (Image const &) = default;

        Image(Image &&) noexcept = default;
        Image& operator= (Image &&) noexcept = default;

        ~Image() = default;

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

        // TODO: can a min()/max() be meaningful? what to take? smallest of r g b? intensity?
        // value_type min() const;
        // value_type max() const;

private:
        int width_ = 0, height_ = 0;
        container_type pixels_;

        void ensureBoundsContract(int x, int y) const {
                namespace cont = support::contract;
                cont::positive(x);
                cont::less_than(x, width_);
                cont::positive(y);
                cont::less_than(y, height_);
        }
};

template <typename T> inline Image<T> operator* (Image<T> canvas, math::Real f);
template <typename T> inline Image<T> operator* (math::Real f, Image<T> canvas);
template <typename T> inline Image<T> operator/ (Image<T> canvas, math::Real f);
template <typename T> inline Image<T> operator/ (math::Real f, Image<T> canvas);
template <typename T> inline Image<T> operator*= (Image<T> &canvas, math::Real f);
template <typename T> inline Image<T> operator/= (Image<T> &canvas, math::Real f);
template <typename T> inline Image<T> min(Image<T> canvas, math::Real f);
template <typename T> inline Image<T> min(math::Real f, Image<T> canvas);
template <typename T> inline Image<T> max(Image<T> canvas, math::Real f);
template <typename T> inline Image<T> max(math::Real f, Image<T> canvas);

template <typename T> inline int width(Image<T> const &canvas);
template <typename T> inline int height(Image<T> const &canvas);

template <typename T> inline Image<T> mirror_x(Image<T> const &canvas);
template <typename T> inline Image<T> mirror_y(Image<T> const &canvas);
template <typename T> inline Image<T> transpose(Image<T> const &canvas);
template <typename T> inline Image<T> rotate90cw(Image<T> const &canvas);
template <typename T> inline Image<T> rotate180cw(Image<T> const &canvas);
template <typename T> inline Image<T> rotate270cw(Image<T> const &canvas);
template <typename T> inline Image<T> rotate90ccw(Image<T> const &canvas);
template <typename T> inline Image<T> rotate180ccw(Image<T> const &canvas);
template <typename T> inline Image<T> rotate270ccw(Image<T> const &canvas);
template <typename T> inline Image<T> copy(Image<T> const &, Rect const &);

} }

//==============================================================================
// Implementation.
//==============================================================================
namespace puffin { namespace image {

template <typename T>
inline Image<T>::Image (int width, int height) :
        width_{support::contract::positive(width)},
        height_{support::contract::positive(height)},
        pixels_{width*height}
{
}

template <typename T>
inline Image<T>::Image (
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
inline auto Image<T>::operator() (int x, int y) -> value_type& {
        ensureBoundsContract(x, y);
        return pixels_[y*width_ + x];
}

template <typename T>
inline auto Image<T>::operator() (int x, int y) const -> value_type {
        ensureBoundsContract(x, y);
        return pixels_[y*width_ + x];
}

template <typename T>
inline auto Image<T>::operator() (Coords const &coords) -> value_type& {
        return (*this)(coords.x, coords.y);
}

template <typename T>
inline auto Image<T>::operator() (Coords const &coords) const -> value_type {
        return (*this)(coords.x, coords.y);
}

template <typename T>
inline auto Image<T>::at(int x, int y) -> value_type& {
        ensureBoundsContract(x, y);
        return pixels_[y*width_ + x];
}

template <typename T>
inline auto Image<T>::at(int x, int y) const -> value_type {
        ensureBoundsContract(x, y);
        return pixels_[y*width_ + x];
}

template <typename T>
inline auto Image<T>::at(Coords const &coords) -> value_type& {
        return at(coords.x, coords.y);
}

template <typename T>
inline auto Image<T>::at(Coords const &coords) const -> value_type {
        return at(coords.x, coords.y);
}

// -- iterators --------------------------------------------------------

template <typename T>
inline auto Image<T>::begin() -> iterator {
        return pixels_.begin();
}

template <typename T>
inline auto Image<T>::begin() const -> const_iterator {
        return pixels_.begin();
}

template <typename T>
inline auto Image<T>::cbegin() const -> const_iterator{
        return pixels_.cbegin();
}

template <typename T>
inline auto Image<T>::end() -> iterator {
        return pixels_.end();
}

template <typename T>
inline auto Image<T>::end() const -> const_iterator {
        return pixels_.end();
}

template <typename T>
inline auto Image<T>::cend() const -> const_iterator {
        return pixels_.cend();
}

// -- capacity ---------------------------------------------------------

template <typename T>
inline auto Image<T>::empty() const -> bool {
        return pixels_.empty();
}

template <typename T>
inline auto Image<T>::size() const -> size_type {
        return pixels_.size();
}

template <typename T>
inline auto Image<T>::max_size() const -> size_type {
        return pixels_.max_size();
}

// -- dimensions -------------------------------------------------------

template <typename T>
inline auto Image<T>::width() const -> int {
        return width_;
}

template <typename T>
inline auto Image<T>::height() const -> int {
        return height_;
}

template <typename T>
inline auto Image<T>::stride() const -> size_type {
        return width();
}

// -- algorithms -------------------------------------------------------

template <typename T>
template <typename RgbFunction>
inline auto Image<T>::for_each (RgbFunction f) -> void {
        std::for_each(begin(), end(), f);
}

template <typename T>
template <typename RgbFunction>
inline auto Image<T>::for_each_2di (RgbFunction f) -> void {
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
inline auto Image<T>::for_each_2dr (RgbFunction f) -> void {
        using math::Real;
        const auto yStep = Real{1} / Real{height()};
        const auto xStep = Real{1} / Real{width()};
        auto fy = Real(0);
        for (int y=0; y!=height_; ++y) {
                auto fx = Real(0);
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
inline auto Image<T>::replace_if(
        UnaryPredicate pred,
        const_reference newValue
) -> void {
        std::replace_if(begin(), end(), pred, newValue);
}

template <typename T>
inline auto Image<T>::fill(const_reference val) -> void {
        for_each([=] (reference pixel) {
                pixel = val;
        });
}

template <typename T>
inline auto operator* (Image<T> canvas, math::Real f) -> Image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = rgb * f;
        });
        return canvas;
}

template <typename T>
inline auto operator* (math::Real f, Image<T> canvas) -> Image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = f * rgb;
        });
        return canvas;
}

template <typename T>
inline auto operator/ (Image<T> canvas, math::Real f) -> Image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = rgb / f;
        });
        return canvas;
}

template <typename T>
inline auto operator/ (math::Real f, Image<T> canvas) -> Image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = f / rgb;
        });
        return canvas;
}

template <typename T>
inline auto operator*= (Image<T> &canvas, math::Real f) -> Image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = rgb * f;
        });
        return canvas;
}

template <typename T>
inline auto operator/= (Image<T> &canvas, math::Real f) -> Image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = rgb / f;
        });
        return canvas;
}

template <typename T>
inline auto min (Image<T> canvas, math::Real f) -> Image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = min(rgb, f);
        });
        return canvas;
}

template <typename T>
inline auto min (math::Real f, Image<T> canvas) -> Image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = min(f, rgb);
        });
        return canvas;
}

template <typename T>
inline auto max (Image<T> canvas, math::Real f) -> Image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = max(rgb, f);
        });
        return canvas;
}

template <typename T>
inline auto max (math::Real f, Image<T> canvas) -> Image<T> {
        canvas.for_each([f] (Rgb &rgb) {
                rgb = max(f, rgb);
        });
        return canvas;
}

template <typename T>
inline auto width (Image<T> const &canvas) -> int {
        return canvas.width();
}

template <typename T>
inline auto height (Image<T> const &canvas) -> int {
        return canvas.height();
}

template <typename T>
inline auto mirror_x(Image<T> const &canvas) -> Image<T> {
        Image<T> ret {canvas.width(), canvas.height()};
        const auto m = canvas.width() - 1;
        ret.for_each_2di([&] (auto x, auto y, T &p) {
                p = canvas(m - x, y);
        });
        return ret;
}

template <typename T>
inline auto mirror_y(Image<T> const &canvas) -> Image<T> {
        Image<T> ret {canvas.width(), canvas.height()};
        const auto m = canvas.height() - 1;
        ret.for_each_2di([&] (auto x, auto y, T &p) {
                p = canvas(x, m - y);
        });
        return ret;
}

template <typename T>
inline auto transpose(Image<T> const &canvas) -> Image<T> {
        Image<T> ret {canvas.height(), canvas.width()};
        ret.for_each_2di([&] (auto x, auto y, T &p) {
                p = canvas(y, x);
        });
        return ret;
}

template <typename T>
inline auto rotate90cw(Image<T> const &canvas) -> Image<T> {
        // [00 10 20]             [00 01 02]            [02 01 00]
        // [01 11 21], transpose: [10 11 12], mirror_x: [12 11 10]
        // [02 12 22]             [20 21 22]            [22 21 20]
        Image<T> ret {canvas.height(), canvas.width()};
        const auto m = canvas.height() - 1;
        ret.for_each_2di([&] (auto x, auto y, T &p) {
                p = canvas(y, m - x);
        });
        return ret;
}

template <typename T>
inline auto rotate180cw(Image<T> const &canvas) -> Image<T> {
        Image<T> ret {canvas.width(), canvas.height()};
        const auto mw = canvas.width() - 1;
        const auto mh = canvas.height() - 1;
        ret.for_each_2di([&] (auto x, auto y, T &p) {
                p = canvas(mw - x, mh - y);
        });
        return ret;
}

template <typename T>
inline auto rotate270cw(Image<T> const &canvas) -> Image<T> {
        Image<T> ret {canvas.height(), canvas.width()};
        const auto m = canvas.width() - 1;
        ret.for_each_2di([&] (auto x, auto y, T &p) {
                p = canvas(m - y, x);
        });
        return ret;
}

template <typename T>
inline auto rotate90ccw(Image<T> const &canvas) -> Image<T> {
        return rotate270cw(canvas);
}

template <typename T>
inline auto rotate180ccw(Image<T> const &canvas) -> Image<T> {
        return rotate180cw(canvas);
}

template <typename T>
inline auto rotate270ccw(Image<T> const &canvas) -> Image<T> {
        return rotate90cw(canvas);
}

template <typename T>
inline Image<T> copy(Image<T> const &canvas, Rect const &rect) {
        Image<T> ret {rect.width(), rect.height()};
        for (int y=rect.top(); y!=rect.bottom(); y++) {
                for (int x=rect.left(); x!=rect.right(); x++) {
                        ret(x-rect.left(), y-rect.top()) = canvas(x, y);
                }
        }
        return ret;
}


} }

//==============================================================================
// Standard instantiations.
//==============================================================================
namespace puffin { namespace image {
using Canvas = Image<Rgb>;
using RgbImage  = Image<Rgb>;
using RgbaImage = Image<Rgba>;
} }
namespace puffin {
using image::Canvas;
using image::RgbImage;
using image::RgbaImage;
}

#endif // CANVAS_HH_INCLUDED_20181025
