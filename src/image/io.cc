// TODO: Error handling
// TODO: Monochrome formats
// TODO: Alpha channel formats

#include "io.hh"
#include "compat/optional.hh"
#include "compat/str_algo.hh"
#include "log.hh"

#include "stb/stb_image.h"

#if 0
#include <boost/gil/image.hpp>
#include <boost/gil/extension/io/jpeg_io.hpp>
#endif

namespace puffin { namespace image {

RgbImage loadRgbImageFromFile (std::string const &filename) {
        log.debug("loadRgbImageFromFile(", filename, ")");
        int width, height, channels;
        unsigned short *data = stbi_load_16(
                filename.c_str(),
                &width, &height,
                &channels, // number of channels as per actual file
                0 // desired number of channels
        );
        if (data == 0) {
                throw std::runtime_error("Failed loading '" + filename + "'");
        }

        // [std_image.h:143]
        // An output image with N components has the following components interleaved
        // in this order in each pixel:
        //
        //     N=#comp     components
        //       1           grey
        //       2           grey, alpha
        //       3           red, green, blue
        //       4           red, green, blue, alpha
        //
        RgbImage ret{width, height};

        log.debug("- image loaded");
        log.debug("- channels:", channels);
        switch (channels) {
        case 1:
                ret.for_each_2di([&](int x, int y, auto &out) {
                        const unsigned short *p = data + x + y*width;
                        out.r = Real(p[0]) / 65535_R;
                        out.g = Real(p[0]) / 65535_R;
                        out.b = Real(p[0]) / 65535_R;
                });
                break;
        case 2:
                ret.for_each_2di([&](int x, int y, auto &out) {
                        const unsigned short *p = data + x*2 + y*2*width;
                        out.r = Real(p[0]) / 65535_R;
                        out.g = Real(p[0]) / 65535_R;
                        out.b = Real(p[0]) / 65535_R;
                });
                break;
        case 3:
                ret.for_each_2di([&](int x, int y, auto &out) {
                        const unsigned short *p = data + x*3 + y*3*width;
                        out.r = Real(p[0]) / 65535_R;
                        out.g = Real(p[1]) / 65535_R;
                        out.b = Real(p[2]) / 65535_R;
                });
                break;
        case 4:
                ret.for_each_2di([&](int x, int y, auto &out) {
                        const unsigned short *p = data + x*4 + y*4*width;
                        out.r = Real(p[0]) / 65535_R;
                        out.g = Real(p[1]) / 65535_R;
                        out.b = Real(p[2]) / 65535_R;
                });
                break;
        default:
                throw std::logic_error(
                        "n-channel image loading not implemented with n=" + std::to_string(channels));
        }

        return ret;
}



RgbaImage loadRgbaImageFromFile (std::string const &filename) {
        log.debug("loadRgbaImageFromFile(", filename, ")");
        int width, height, channels;
        unsigned short *data = stbi_load_16(
                filename.c_str(),
                &width, &height,
                &channels, // number of channels as per actual file
                0 // desired number of channels
        );
        if (data == 0) {
                throw std::runtime_error("Failed loading '" + filename + "'");
        }

        // [std_image.h:143]
        // An output image with N components has the following components interleaved
        // in this order in each pixel:
        //
        //     N=#comp     components
        //       1           grey
        //       2           grey, alpha
        //       3           red, green, blue
        //       4           red, green, blue, alpha
        //
        RgbaImage ret{width, height};

        log.debug("- image loaded");
        log.debug("- channels:", channels);
        switch (channels) {
                case 1:
                        ret.for_each_2di([&](int x, int y, auto &out) {
                                const unsigned short *p = data + x + y*width;
                                out.r = Real(p[0]) / 65535_R;
                                out.g = Real(p[0]) / 65535_R;
                                out.b = Real(p[0]) / 65535_R;
                                out.a = Real(1);
                        });
                        break;
                case 2:
                        ret.for_each_2di([&](int x, int y, auto &out) {
                                const unsigned short *p = data + x*2 + y*2*width;
                                out.r = Real(p[0]) / 65535_R;
                                out.g = Real(p[0]) / 65535_R;
                                out.b = Real(p[0]) / 65535_R;
                                out.a = Real(p[1]) / 65535_R;
                        });
                        break;
                case 3:
                        ret.for_each_2di([&](int x, int y, auto &out) {
                                const unsigned short *p = data + x*3 + y*3*width;
                                out.r = Real(p[0]) / 65535_R;
                                out.g = Real(p[1]) / 65535_R;
                                out.b = Real(p[2]) / 65535_R;
                                out.a = Real(1);
                        });
                        break;
                case 4:
                        ret.for_each_2di([&](int x, int y, auto &out) {
                                const unsigned short *p = data + x*4 + y*4*width;
                                out.r = Real(p[0]) / 65535_R;
                                out.g = Real(p[1]) / 65535_R;
                                out.b = Real(p[2]) / 65535_R;
                                out.a = Real(p[3]) / 65535_R;
                        });
                        break;
                default:
                        throw std::logic_error(
                                "n-channel image loading not implemented with n=" + std::to_string(channels));
        }

        return ret;
}

} }
