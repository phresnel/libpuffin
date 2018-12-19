#include <iostream>
#include <fstream>
#include <stdexcept>
#include "sdlxx/Sdl.hh"

#include "puffin/bmp/read_bmp.hh"

#include <bitset>
#include <sstream>

int main() {
        try {
                // image::Canvas canvas(512, 512);
                //puffin::read_bmp("dev-assets/bmp/red256_green128_blue64_4x2x8bit.bmp");
                puffin::read_bmp("dev-assets/bmp/rg_is_xy_2x2x8bit.bmp");
                return 0;

                puffin::sdlxx::Sdl sdl;
                auto renderer = sdl.createRenderer(512, 512);

                // dev-assets/bmp/puffin_1bit.bmp
                // dev-assets/bmp/puffin_4bit.bmp
                // dev-assets/bmp/puffin_8bit.bmp
                // dev-assets/bmp/puffin_24bit.bmp

                // renderer.copy(canvas);
                // renderer.present();

                sdl.pollTilQuit();
                return 0;
        } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                return 1;
        }
}
