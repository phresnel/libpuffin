#ifndef PIXEL_FORMAT_HH_INCLUDED_20190102
#define PIXEL_FORMAT_HH_INCLUDED_20190102

namespace {
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

#endif //PIXEL_FORMAT_HH_INCLUDED_20190102
