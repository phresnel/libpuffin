#include "puffin/impl/sdl_util.hh"
#include "puffin/bitmap.hh"
#include "puffin/image.hh"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>

#include <vector>
#include <bitset>
#include <sstream>

namespace puffin {


}

int main() {
        try {
                //puffin::Bitmap32 *p = puffin::read_bmp32("dev-assets/bmp/rg_is_xy_2x2x24bit.bmp");
                //puffin::Bitmap *p = puffin::read_bmp("dev-assets/bmp/puffin_24bit.bmp");
                puffin::InvalidBitmap p = puffin::read_invalid_bmp("dev-assets/bmpsuite-2.5/g/pal4rle.bmp");
                std::cout << p << std::endl;
                if (!p.valid())
                        return 1;
                puffin::impl::Sdl sdl;
                puffin::impl::SdlRenderer sdlRenderer = sdl.createRenderer(p.width(), p.height());
                for (int y=0; y!=p.height(); ++y) {
                        for (int x = 0; x!=p.width(); ++x) {
                                const puffin::Color32 col = p.at(x, y);
                                sdlRenderer.setPixel(x, y, col.r(), col.g(), col.b());
                        }
                }
                sdlRenderer.present();
                sdl.pollTilQuit();
                return 0;
        } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
                return 1;
        }
}
