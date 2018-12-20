#ifndef SDL_UTIL_HH_20181220
#define SDL_UTIL_HH_20181220

#include "SDL2/SDL.h"
#include <memory>
#include <functional>
#include <algorithm>
#include <stdexcept>

namespace puffin { namespace impl {

class Sdl;

class SdlRenderer final {
public:
        SdlRenderer() = delete;

        SdlRenderer(SdlRenderer const&) = delete;
        SdlRenderer& operator= (SdlRenderer const&) = delete;

        SdlRenderer (SdlRenderer &&) = default;
        SdlRenderer& operator= (SdlRenderer &&) = default;

        int width() const { return width_; }
        int height() const { return height_; }

        void clear(int r, int g, int b) {
                SDL_SetRenderDrawColor(
                        renderer_.get(),
                        static_cast<Uint8>(r),
                        static_cast<Uint8>(g),
                        static_cast<Uint8>(b),
                        255);
                SDL_RenderClear(renderer_.get());
        }

        void setPixel(int x, int y, int r, int g, int b) {
                ensureBoundsContract(x, y);
                ensureRgbContract(r, g, b);

                SDL_SetRenderDrawColor(
                        renderer_.get(),
                        static_cast<Uint8>(r),
                        static_cast<Uint8>(g),
                        static_cast<Uint8>(b),
                        255U);
                SDL_RenderDrawPoint(renderer_.get(), x, y);
        }

        // The draw method assumes that the canvas has all colors scaled and
        // maximized already. All it does is a bounds-safe 1-to-1 copy and
        // conversion to integers, starting at P(0|0) w.r.t. both surfaces.
        template <typename CanvasT>
        void copy(
                CanvasT const &src,
                std::function<void(double&,double&,double&)> colTran =
                        [] (double &r, double &g, double &b) {
                                using std::min;
                                using std::max;
                                r = max(0.0, min(r, 1.0)) * 255.0;
                                g = max(0.0, min(g, 1.0)) * 255.0;
                                b = max(0.0, min(b, 1.0)) * 255.0;
                        }
        ) {
                using std::max;
                using std::min;

                using OwnHeightT = decltype(height());
                using OwnWidthT = decltype(width());

                const auto srcHeight = static_cast<OwnHeightT>(src.height());
                const auto srcWidth = static_cast<OwnWidthT>(src.width());
                const auto maxY = min(height(), srcHeight);
                const auto maxX = min(width(), srcWidth);

                for (OwnHeightT y=0; y!=maxY; ++y) {
                        for (OwnWidthT x = 0; x != maxX; ++x) {
                                const auto &rgb = src(x, y);
                                double fr = static_cast<double>(rgb.r);
                                double fg = static_cast<double>(rgb.g);
                                double fb = static_cast<double>(rgb.b);
                                colTran(fr, fg, fb);
                                const int r = static_cast<int>(fr);
                                const int g = static_cast<int>(fg);
                                const int b = static_cast<int>(fb);
                                setPixel(x, y, r, g, b);
                        }
                }
        }

        void present() {
                SDL_RenderPresent(renderer_.get());
        }

private:
        SdlRenderer(int width, int height) :
                width_(width),
                height_(height) {
                SDL_Renderer *renderer;
                SDL_Window *window;
                if (0 != SDL_CreateWindowAndRenderer(width, height,
                                                     0, &window, &renderer)
                ) {
                        throw std::runtime_error("Could not create SDL window "
                                                 "and/or renderer");
                }
                window_ = {window, SDL_DestroyWindow};
                renderer_ = {renderer, SDL_DestroyRenderer};
        }

        void ensureBoundsContract(int x, int y) const {
                /*
                contract::positive(x);
                contract::less_than(x, width_);
                contract::positive(y);
                contract::less_than(y, height_);
                */
        }

        void ensureRgbContract(int r, int g, int b) const {
                /*
                namespace cont = oneiric::support::contract;
                cont::less_or_equal(r, 255);
                cont::positive(g);
                cont::less_or_equal(g, 255);
                cont::positive(b);
                cont::less_or_equal(b, 255);
                */
        }

        friend class Sdl;

        int width_, height_;

        using RendererDeleter = std::function<void(SDL_Renderer *)>;
        std::unique_ptr<SDL_Renderer, RendererDeleter> renderer_;

        using WindowDeleter   = std::function<void(SDL_Window *)>;
        std::unique_ptr<SDL_Window, WindowDeleter> window_;
};


class Sdl final {
public:
        Sdl(Sdl const &) = delete;

        Sdl &operator=(Sdl const &) = delete;

        Sdl() {
                if (0 != SDL_Init(0))
                        throw std::runtime_error("Could not init SDL.");
        }

        SdlRenderer createRenderer(int width, int height) const {
                return {width, height};
        }

        ~Sdl() {
                SDL_Quit();
        }

        void pollTilQuit() {
                while (!quitRequested()) {
                        poll();
                }
        }

        void poll() {
                SDL_Event event;
                while(SDL_PollEvent(&event)) {
                        quitRequested_ = quitRequested_ || shouldQuit(event);
                }
        }

        bool quitRequested() const {
                return quitRequested_;
        }

private:
        bool shouldQuit(SDL_Event const &event) const {
                const bool otherQuit = event.type == SDL_QUIT;
                const bool escapeKeyUp = (event.type == SDL_KEYUP) &&
                                         (event.key.keysym.sym == SDLK_ESCAPE);
                return escapeKeyUp || otherQuit;
        }

        bool quitRequested_ = false;
};

} }

#endif //SDL_UTIL_HH_20181220
