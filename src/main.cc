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

// Basis for a more flexible approach.
namespace puffin {

enum signedness {
        is_unspecified = 0,
        is_signed = 1,
        is_unsigned = -1
};
typedef signedness signedness_t;

class value_format {
public:
        // -- constructors -----------------------------------------------------
        static value_format raw_bytes(int bits) {
                return value_format()
                      .bits(bits);
        }

        static value_format integer_sign_unspecified(int bits) {
                return raw_bytes(bits)
                      .is_integer(true);
        }

        static value_format integer(signedness s, int bits) {
                return integer_sign_unspecified(bits)
                      .signedness(s);
        }

        static value_format signed_int(int bits) {
                return integer(signedness::is_signed, bits);
        }

        static value_format unsigned_int(int bits) {
                return integer(signedness::is_unsigned, bits);
        }

        static value_format floating_point(int bits) {
                return raw_bytes(bits)
                      .is_float(true);
        }

        // -- constructors reflecting some cstdint-types -----------------------
        static value_format int8_t() {  return signed_int(8); }
        static value_format int16_t() { return signed_int(16); }
        static value_format int32_t() { return signed_int(32); }
        static value_format int64_t() { return signed_int(64); }
        static value_format uint8_t() { return unsigned_int(8); }
        static value_format uint16_t() { return unsigned_int(16); }
        static value_format uint32_t() { return unsigned_int(32); }
        static value_format uint64_t() { return unsigned_int(64); }

        // -- methods ----------------------------------------------------------

        // -- basic-type accessors ---
        bool is_integer() const {
                return is_integer_;
        }
        value_format& is_integer(bool v) {
                is_integer_ = v;
                return *this;
        }

        bool is_float() const {
                return is_float_;
        }
        value_format& is_float(bool v) {
                is_float_ = v;
                return *this;
        }

        // --- signedness accessors ---
        signedness signedness() const {
                return signedness_;
        }
        bool is_signedness_unspecified() const {
                return signedness() == signedness::is_unspecified;
        }
        bool is_signed() const {
                return signedness() == signedness::is_signed;
        }
        bool is_unsigned() const {
                return signedness() == signedness::is_unsigned;
        }
        value_format& signedness(enum signedness v) {
                signedness_ = v;
                return *this;
        }
        value_format& set_signedness_unspecified() {
                return signedness(signedness::is_unspecified);
        }
        value_format& is_signed(bool v) {
                return signedness(v ? signedness::is_signed
                                    : signedness::is_unsigned);
        }
        value_format& is_unsigned(bool v) {
                return is_signed(!v);
        }

        // --- bitwise stuff ---
        int bits() const {
                return bits_;
        }
        value_format& bits(int bits) {
                bits_ = bits;
                return *this;
        }

        // --- numeric limits ---
        int min_value() const {
        }

        int max_value() const {
        }

        // -- open tasks -------------------------------------------------------
        // TODO: does it makes to handle negative size here?

private:
        value_format() :
                bits_(0),
                signedness_(signedness::is_unspecified),
                is_integer_(false),
                is_float_(false)
        {}

        int bits_;
        enum signedness signedness_;
        bool is_integer_;
        bool is_float_;
};

class color_channel_format {
public:
        color_channel_format() : from_(0), to_(0) {}
        color_channel_format(int from, int to) : from_(from), to_(to) {}

        int size() const {
                const int s = to_ - from_;
                return s < 0 ? -s : s;
        }

        int from() const {
                return from_;
        }

        int to() const {
                return to_;
        }

        int min() const {
                return from_ < to_ ? from_ : to_;
        }

        int max() const {
                return from_ > to_ ? from_ : to_;
        }

        int min_value() const {
                return 0;
        }

        int max_value() const {

                size()
        }
private:
        int from_, to_;
};

struct pixel_format {
        int chunk_bits;
        int pixel_bits;



        static pixel_format palleted(int chunk_bits, int pixel_bits) {
                pixel_format ret;
                ret.chunk_bits = chunk_bits;
                ret.pixel_bits = pixel_bits;
                return ret;
        }

        static pixel_format rgb(int r_bits, int g_bits, int b_bits) {
                pixel_format ret;

                return ret;
        }

private:
        pixel_format() :
                chunk_bits(0),
                pixel_bits(0)
        { }
};

}

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
                puffin::Bitmap32 *p = puffin::read_bmp32("dev-assets/bmp/puffin_4bit.bmp");
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
