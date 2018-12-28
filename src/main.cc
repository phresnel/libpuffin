#include "puffin/impl/sdl_util.hh"
#include "puffin/bmp.hh"
#include "puffin/color.hh"
#include "puffin/image.hh"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>

#include <bitset>
#include <sstream>

int main() {
        /*
        typedef puffin::bitfield4<8,8,8,8> bf;
        std::cout << std::bitset<bf::storage_bits>(bf::mask1) << std::endl;
        std::cout << std::bitset<bf::storage_bits>(bf::mask2) << std::endl;
        std::cout << std::bitset<bf::storage_bits>(bf::mask3) << std::endl;
        std::cout << std::bitset<bf::storage_bits>(bf::mask4) << std::endl;
        std::cout << "bits:" << bf::bits << std::endl;
        std::cout << "typeid:" << typeid(bf::storage_type).name() << std::endl;
        std::cout << "storage bits:" << bf::storage_bits << std::endl;

        bf a;
        a.values(44,55,66,77);
        std::cout << std::dec << a;
         */

        try {
                //puffin::Bitmap32 *p = puffin::read_bmp32("dev-assets/bmp/rg_is_xy_2x2x24bit.bmp");
                puffin::Bitmap32 *p = puffin::read_bmp32("dev-assets/bmp/puffin_8bit.bmp");
                puffin::impl::Sdl sdl;
                puffin::impl::SdlRenderer sdlRenderer = sdl.createRenderer(p->width(), p->height());
                for (int y=0; y!=p->height(); ++y) {
                        for (int x = 0; x!=p->width(); ++x) {
                                const puffin::Color32 col = (*p).at(x, y);
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
