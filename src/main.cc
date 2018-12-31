#include "puffin/impl/sdl_util.hh"
#include "puffin/bmp.hh"
#include "puffin/color.hh"
#include "puffin/image.hh"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>

#include <vector>
#include <bitset>
#include <sstream>

// Basis for a more flexible approach.
namespace puffin {

// -- signedness ---------------------------------------------------------------
enum signedness {
        is_unspecified = 0,
        is_signed = 1,
        is_unsigned = -1
};
typedef signedness signedness_t;

// -- bitmask ------------------------------------------------------------------
class bitmask {
        typedef std::vector<bool> container_type;
public:
        typedef container_type::iterator iterator;
        typedef container_type::const_iterator const_iterator;
        typedef container_type::reverse_iterator reverse_iterator;
        typedef container_type::const_reverse_iterator const_reverse_iterator;
        typedef container_type::size_type size_type;
        typedef container_type::reference reference;
        typedef container_type::const_reference const_reference;

        // -- constructors & assignment ----------------------------------------
        static bitmask Bitmask8(uint8_t mask) {
                return bitmask(8, mask);
        }

        static bitmask Bitmask16(uint16_t mask) {
                return bitmask(16, mask);
        }

        static bitmask Bitmask32(uint32_t mask) {
                return bitmask(32, mask);
        }

        static bitmask Bitmask64(uint64_t mask) {
                return bitmask(64, mask);
        }

        bitmask() {}
        bitmask(bitmask const &v) : bits_(v.bits_) {}
        bitmask& operator= (bitmask v) {
                bits_.swap(v.bits_);
                return *this;
        }

        template <typename MaskT>
        bitmask(size_type size, MaskT mask) :
                bits_(size)
        {
                for (int i=0; i<size; ++i) {
                        const bool b = (i<std::numeric_limits<MaskT>::digits) ?
                                       ((mask >> i) & 1) :
                                       0;
                        bits_[i] = b;
                }
        }

        // ---------------------------------------------------------------------
        size_type size() const {
                return bits_.size();
        }

        bool empty() const {
                return size() == 0;
        }

        void push_back(bool v) {
                bits_.push_back(v);
        }

        reference operator[] (size_type i) {
                return bits_[i];
        }

        const_reference operator[] (size_type i) const {
                return bits_[i];
        }

        reference at (size_type i) {
                return bits_.at(i);
        }

        const_reference at (size_type i) const {
                return bits_.at(i);
        }

        // -- range ------------------------------------------------------------
        iterator begin() { return bits_.begin(); }
        iterator end() { return bits_.end(); }

        const_iterator cbegin() const { return bits_.begin(); }
        const_iterator cend() const { return bits_.end(); }

        const_iterator begin() const { return cbegin(); }
        const_iterator end() const { return cend(); }

        reverse_iterator rbegin() { return bits_.rbegin(); }
        reverse_iterator rend() { return bits_.rend(); }

        const_reverse_iterator crbegin() const { return bits_.rbegin(); }
        const_reverse_iterator crend() const { return bits_.rend(); }

        const_reverse_iterator rbegin() const { return crbegin(); }
        const_reverse_iterator rend() const { return crend(); }

        // ---------------------------------------------------------------------
        void prune() {
                while(!bits_.empty()
                      && !bits_.back())
                        bits_.pop_back();
        }

        bool is_zero() const {
                for (const_iterator it=begin(); it!=end(); ++it) {
                        if (*it)
                                return false;
                }
                return true;
        }

        bool is_non_zero() const {
                for (const_iterator it=begin(); it!=end(); ++it) {
                        if (*it)
                                return true;
                }
                return false;
        }

        size_type count() const {
                size_type ret = 0;
                for (const_iterator it=begin(); it!=end(); ++it) {
                        ret += *it ? 1 : 0;
                }
                return ret;
        }

        size_type highest_bit() const {
                if (empty())
                        return 0;
                size_type i=size();
                do {
                        --i;
                        if ((*this)[i])
                                return i;
                } while(i!=0);
                return 0;
        }

        size_type lowest_bit() const {
                for (size_type i=0; i!=size(); ++i)
                        if ((*this)[i])
                                return i;
                return 0;
        }

        template <typename T>
        T extract_value(T const &bits) const {
                if (is_zero())
                        return 0;

                size_type ret_i = 0;
                T ret = 0;
                for(size_type i=0, s=size();
                    i<s && ret_i<std::numeric_limits<T>::digits;
                    ++i
                ) {
                        if (!(*this)[i])
                                continue;

                        const int val = (bits>>i) & 1;
                        ret |= (val<<ret_i);
                        ++ret_i;
                }
                return ret;
        }

        // ---------------------------------------------------------------------
        size_type min_value() const {
                return 0;
        }

        size_type max_value() const {
                if (is_zero())
                        return 0;
                return num_states()-1;
        }

        size_type num_states() const {
                int ret = 1;
                for (int i=0, c=count(); i!=c; ++i) {
                        ret *= 2;
                }
                return ret;
        }
private:
        std::vector<bool> bits_;
};

std::ostream& operator<< (std::ostream &os, bitmask const &v) {
        int bb = 0;
        for(bitmask::const_reverse_iterator it=v.rbegin(); it!=v.rend(); ++it) {
                if (bb>0 && bb%8 == 0)
                        os << ".";
                ++bb;
                os << (*it ? "1" : "0");
        }
        os << " (non-zero:" << v.is_non_zero() << ", "
           << "size:" << v.size() << ", "
           << "lo:" << v.lowest_bit() << ", "
           << "hi:" << v.highest_bit() << ", "
           << "min:" << v.min_value() << ", "
           << "max:" << v.max_value() << ", "
           << "num_states:" << v.num_states() << ""
           << ")"
           ;
        return os;
}

// -- size ---------------------------------------------------------------------
class size {
public:
        static size Bits(uint64_t bits) { return size(bits); }
        static size Bytes(uint64_t bytes) { return Bits(bytes*8); }

        size() : bits_(0) {}

        uint64_t bits() const { return bits_; }
        uint64_t bytes() const { return bits() / 8; }
private:
        explicit size(uint64_t s) : bits_(s) {}
        uint64_t bits_;
};

inline bool operator== (size const &lhs, size const &rhs) {
        return lhs.bits() == rhs.bits();
}
inline bool operator< (size const &lhs, size const &rhs) {
        return lhs.bits() < rhs.bits();
}

inline bool operator!= (size const &lhs, size const &rhs) {
        return !(lhs == rhs);
}
inline bool operator> (size const &lhs, size const &rhs) {
        return rhs < lhs;
}

inline bool operator<= (size const &lhs, size const &rhs) {
        return !(rhs < lhs);
}
inline bool operator>= (size const &lhs, size const &rhs) {
        return !(lhs < rhs);
}

inline std::ostream& operator<< (std::ostream &os, size const& v) {
        return os << v.bits() << " bits";
}

// -- value_format -------------------------------------------------------------
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
                if (is_integer()) {
                        if (is_unsigned())
                                return 0;
                        return -num_states()/2;
                }
                return 0;
        }

        int max_value() const {
                if (is_integer()) {
                        if (is_unsigned())
                                return num_states()-1;
                        return num_states()/2;
                }
                return 0;
        }

        int num_states() const {
                int ret = 1;
                for (int i=0; i!=bits(); ++i) {
                        ret *= 2;
                }
                return ret;
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
inline std::ostream& operator<< (std::ostream& os, signedness const &v) {
        switch(v) {
        case signedness::is_unsigned: return os << "unsigned";
        case signedness::is_signed: return os << "signed";
        case signedness::is_unspecified: return os << "unspecified";
        };
}
inline std::ostream& operator<< (std::ostream& os, value_format const &v) {
        return os << v.signedness()
                  << " "
                  << (v.is_float()?"float":
                      v.is_integer()?"int":
                      "????")
                  << " : " << v.bits()
                  << ", min_value: " << v.min_value()
                  << ", max_value: " << v.max_value()
                  << ", num_states: " << v.num_states()
                  ;

}

// -- color_channel_format -----------------------------------------------------
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
                //size()
                return 0;
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

template <typename T>
void print_numeric_limits() {
        typedef std::numeric_limits<T> nl;
        std::cout << "[" << typeid(T).name() << "] "
                  << (nl::is_signed?"signed":"unsigned")
                  << " "
                  << (nl::is_iec559?"float":
                      nl::is_integer?"int":
                      "????")
                  << " : " << (sizeof(T)*8)
                  << ", min_value: " << nl::min()
                  << ", max_value: " << nl::max()
                  //<< ", num_states: " << v.num_states()
                ;

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

        puffin::value_format int16 = puffin::value_format::uint16_t();
        std::cout << int16 << std::endl;
        print_numeric_limits<unsigned short>();
        std::cout << std::endl;

        /*
        std::cout << std::endl;
        puffin::size a = puffin::size::Bytes(4);
        puffin::size b = puffin::size::Bits(4*8+1);
        std::cout << "a:" << a << "\n";
        std::cout << "b:" << b << "\n";
        std::cout << "a==b:" << (a==b) << "\n";
        std::cout << "a!=b:" << (a!=b) << "\n";
        std::cout << "a<b:" << (a<b) << "\n";
        std::cout << "a>b:" << (a>b) << "\n";
        std::cout << "a<=b:" << (a<=b) << "\n";
        std::cout << "a>=b:" << (a>=b) << "\n";
        */

        puffin::bitmask red_mask  (32, 0x0000FF);
        puffin::bitmask green_mask(32, 0x00FF00);
        puffin::bitmask blue_mask (32, 0xFF0000);
        std::cout << "red_mask:  " << red_mask << std::endl;
        std::cout << "green_mask:" << green_mask << std::endl;
        std::cout << "blue_mask: " << blue_mask << std::endl;
        uint32_t rgb = (11<<16) | (10<<8) | (9);
        std::cout << "red  :" << (red_mask.extract_value(rgb)) << std::endl;
        std::cout << "green:" << (green_mask.extract_value(rgb)) << std::endl;
        std::cout << "blue :" << (blue_mask.extract_value(rgb)) << std::endl;

        return 0;

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
