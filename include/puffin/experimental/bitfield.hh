#ifndef BITFIELD_HH_INCLUDED_20181224
#define BITFIELD_HH_INCLUDED_20181224

#include <ostream>
#include <cstdint>
#include <bitset>

// Simple, generic bitfield implementation.
// Unfortunately, we can not get it to have a guaranteed size of 64 bit
// or more in C++03 (in C++11, it's as simple as enum:uint64_t).

namespace puffin {

// -- set_bits -----------------------------------------------------------------
template <int v> struct set_bits {
        enum { value = (set_bits<v-1>::value<<1) | 1 };
};
template <> struct set_bits<0> {
        enum { value = 0 };
};
template <> struct set_bits<1> {
        enum { value = 1 };
};

// -- fit_to_uint --------------------------------------------------------------
template <int n> struct fit_to_uint {
        typedef typename fit_to_uint<n+1>::type type;
        enum {
                bits = fit_to_uint<n+1>::bits
        };
};
template <> struct fit_to_uint<8>  { typedef uint8_t  type; enum{ bits=8 }; };
template <> struct fit_to_uint<16> { typedef uint16_t type; enum{ bits=16 }; };
template <> struct fit_to_uint<32> { typedef uint32_t type; enum{ bits=32 }; };
template <> struct fit_to_uint<64> { typedef uint64_t type; enum{ bits=64 }; };

// -- bitfield4 ----------------------------------------------------------------
template <int Bits1, int Bits2, int Bits3, int Bits4>
struct bitfield4 {
        enum {
                bits1 = Bits1,
                bits2 = Bits2,
                bits3 = Bits3,
                bits4 = Bits4,
                bits = Bits1 + Bits2 + Bits3 + Bits4
        };
        enum {
                shift1 = bits2 + bits3 + bits4,
                shift2 = bits3 + bits4,
                shift3 = bits4,
                shift4 = 0
        };
        enum {
                mask1 = set_bits<Bits1>::value << shift1,
                mask2 = set_bits<Bits2>::value << shift2,
                mask3 = set_bits<Bits3>::value << shift3,
                mask4 = set_bits<Bits4>::value << shift4
        };
        typedef typename fit_to_uint<bits>::type storage_type;
        enum { storage_bits = fit_to_uint<bits>::bits };

        storage_type storage;

        bitfield4() {}

        bitfield4(
                storage_type a, storage_type b, storage_type c, storage_type d
        ) {
                values(a,b,c,d);
        }

        bitfield4(storage_type raw) : storage(raw) { }

        storage_type value1() const { return (storage & mask1) >> shift1; }
        storage_type value2() const { return (storage & mask2) >> shift2; }
        storage_type value3() const { return (storage & mask3) >> shift3; }
        storage_type value4() const { return (storage & mask4) >> shift4; }

        void value1(storage_type v) {
                storage = (storage & ~mask1) | ((v << shift1) & mask1);
        }
        void value2(storage_type v) {
                storage = (storage & ~mask2) | ((v << shift2) & mask2);
        }
        void value3(storage_type v) {
                storage = (storage & ~mask3) | ((v << shift3) & mask3);
        }
        void value4(storage_type v) {
                storage = (storage & ~mask4) | ((v << shift4) & mask4);
        }
        void values(storage_type a,
                    storage_type b,
                    storage_type c,
                    storage_type d)
        {
                storage = ((a << shift1) & mask1) |
                          ((b << shift2) & mask2) |
                          ((c << shift3) & mask3) |
                          ((d << shift4) & mask4) ;
        }
};

template <int a, int b, int c, int d>
inline
std::ostream& operator<< (std::ostream &os, bitfield4<a,b,c,d> const &bf) {
        os << "("
           << bf.value1() << ","
           << bf.value2() << ","
           << bf.value3() << ","
           << bf.value4()
           << ")";
        os << "==("
           << std::bitset<bitfield4<a,b,c,d>::storage_bits>(bf.storage)
           << ")";
        return os;
}

template <int bits1, int bits2, int bits3, int bits4>
inline
std::string type_str(bitfield4<bits1, bits2, bits3, bits4> const &) {
        std::stringstream ss;
        ss << "bitfield4<"
           << "Bits1=" << bits1 << ", "
           << "Bits2=" << bits2 << ", "
           << "Bits3=" << bits3 << ", "
           << "Bits4=" << bits4 << ">";
        return ss.str();
}

}

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

#endif //BITFIELD_HH_INCLUDED_20181224
