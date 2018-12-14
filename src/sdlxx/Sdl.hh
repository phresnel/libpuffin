#ifndef ONEIRIC_SDL_HH_20181008
#define ONEIRIC_SDL_HH_20181008

#include "Renderer.hh"
#include <SDL2/SDL.h>

namespace puffin { namespace sdlxx {

class Sdl final {
public:
        Sdl(Sdl const &) = delete;

        Sdl &operator=(Sdl const &) = delete;

        Sdl() {
                if (0 != SDL_Init(0))
                        throw std::runtime_error("Could not init SDL.");
        }

        Renderer createRenderer(int width, int height) const {
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

#endif //ONEIRIC_SDL_HH_20181008
