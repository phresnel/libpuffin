#ifndef SIZE_HH_INCLUDED_20190102
#define SIZE_HH_INCLUDED_20190102

#include <iostream>

namespace puffin {

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

}

#endif //SIZE_HH_INCLUDED_20190102
