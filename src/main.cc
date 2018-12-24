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
                // image::Canvas canvas(512, 512);
                //puffin::read_bmp("dev-assets/bmp/red256_green128_blue64_4x2x8bit.bmp");
                //puffin::read_bmp("dev-assets/bmp/rg_is_xy_2x2x8bit.bmp");

                /*
                typedef puffin::basic_rgba<unsigned short, unsigned int, unsigned int, float> rgba_t;
                rgba_t x(1, 1, 1, 0.5f);
                std::cout << "rgb_have_common_type:" << rgba_t::rgb_have_common_type << std::endl;
                std::cout << "rgba_have_common_type:" << rgba_t::rgba_have_common_type << std::endl;
                */

                //typedef puffin::basic_rgba<unsigned char, unsigned char, unsigned char, unsigned char> rgb565_t;
                //rgb565_t x(1, 1, 1, 0.5);
                return 0;

                puffin::impl::Sdl sdl;
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
