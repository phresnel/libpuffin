#ifndef RGBA_BITMASK_HH_INCLUDED_20190114
#define RGBA_BITMASK_HH_INCLUDED_20190114

#include "puffin/impl/io_util.hh"

namespace puffin {

template <typename UintType>
class Bitmask {
public:
        typedef UintType value_type;

        Bitmask() {
                reset();
        }

        explicit Bitmask(uint32_t v) {
                reset(v);
        }

        void reset(uint32_t v = 0) {
                using impl::first_bit_set;
                using impl::last_bit_set;
                using impl::no_bit_set;

                if (no_bit_set(v)) {
                        shift_ = 0;
                        mask_ = 0;
                        width_ = 0;
                        return;
                }

                shift_ = static_cast<uint8_t>(first_bit_set(v));
                mask_ = static_cast<uint8_t>(v >> shift_);
                width_ = static_cast<uint8_t>((1 + last_bit_set(v)) - shift_);
        }

        uint32_t extract(uint32_t raw) const {
                const uint32_t
                        extracted = static_cast<uint8_t>((raw >> shift_) & mask_),
                        scaled = extracted << (sizeof(value_type)*8-width_); // TODO: unhardcode
                return scaled;
        }

        value_type shift() const { return shift_; }
        value_type mask() const { return mask_; }
        value_type width() const { return width_; }
private:
        value_type shift_, mask_, width_;
};

template <typename ChunkType, typename ChannelType, typename ColorType>
class RgbaBitmask {
public:
        typedef ChunkType             chunk_type;
        typedef ChannelType           channel_type;
        typedef Bitmask<channel_type> bitmask_type;
        typedef ColorType             color_type;


        RgbaBitmask() {}

        RgbaBitmask(
                chunk_type r_mask,
                chunk_type g_mask,
                chunk_type b_mask,
                chunk_type a_mask = 0
        ) {
                reset(r_mask, g_mask, b_mask, a_mask);
        }

        void reset() {
                r_.reset();
                g_.reset();
                b_.reset();
                a_.reset();
        }

        void reset(
                chunk_type r_mask,
                chunk_type g_mask,
                chunk_type b_mask,
                chunk_type a_mask = 0
        ) {
                r_.reset(r_mask);
                g_.reset(g_mask);
                b_.reset(b_mask);
                a_.reset(a_mask);
        }

        bitmask_type r() const { return r_; }
        bitmask_type g() const { return g_; }
        bitmask_type b() const { return b_; }
        bitmask_type a() const { return a_; }

        color_type rawToColor(chunk_type raw) const {
                const color_type ret = color_type(
                        static_cast<channel_type>(r_.extract(raw)),
                        static_cast<channel_type>(g_.extract(raw)),
                        static_cast<channel_type>(b_.extract(raw)),
                        static_cast<channel_type>(a_.extract(raw))
                );
                return ret;
        }


private:
        bitmask_type r_, g_, b_, a_;
};

typedef RgbaBitmask<uint32_t, uint8_t, Color32> RgbaBitmask32;

}

#endif //RGBA_BITMASK_HH_INCLUDED_20190114
