#include <iostream>
#include "sdlxx/Sdl.hh"

int main() {
        // image::Canvas canvas(512, 512);

        puffin::sdlxx::Sdl sdl;
        auto renderer = sdl.createRenderer(512, 512);

        // renderer.copy(canvas);
        // renderer.present();

        sdl.pollTilQuit();
        return 0;
}
