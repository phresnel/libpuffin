#ifndef BITMASK_HH_INCLUDED_20190102
#define BITMASK_HH_INCLUDED_20190102

#include <iostream>
#include <vector>

namespace puffin {

class bitmask {
        typedef std::vector<bool> container_type;

        // some optimization ideas:
        //  - step 1: instead of explicitly storing all zero and non-zero bits,
        //            store the positions of non-zero bits and nothing more.
        //  - step 2: instead of storing all non-zero-bit positions, store
        //            start:width per 1-sequence (a runtime length encoding)
        //  - a mask of contiguous bits (e.g. 0011110, but not 0100111) could be
        //    represented by just two numbers (start:end, or start:length)
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
        T unpack(T const &bits) const {
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
                for (size_type i=0, c=count(); i!=c; ++i) {
                        ret *= 2;
                }
                return ret;
        }
private:
        std::vector<bool> bits_;
};

inline
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

}


// bitmask_vector
namespace puffin {

template <typename T> class from_proxy;

class bitmask_vector {
        typedef std::vector<bitmask> container_type;
public:
        // -- types ------------------------------------------------------------
        typedef container_type::size_type size_type;
        typedef container_type::value_type value_type;

        // -- ctor/dtor/assgnmt ------------------------------------------------
        bitmask_vector() {}
        bitmask_vector(bitmask_vector const &v) : masks_(v.masks_) {}

        // -- element access ---------------------------------------------------
        value_type& operator[] (size_type i) { return masks_[i]; }
        value_type operator[] (size_type i) const { return masks_[i]; }

        value_type& at (size_type i) { return masks_.at(i); }
        value_type at (size_type i) const { return masks_.at(i); }

        // -- data extraction --------------------------------------------------
        template <typename T>
        from_proxy<T> unpack(T const &bits) const {
                return from_proxy<T>(*this, bits);
        }

        template <typename T>
        T unpack(T const &bits, size_type n) const {
                if (n >= size())
                        return T();
                return masks_[n].unpack(bits);
        }

        template <typename T>
        void unpack_to(T const &bits, T &o) const {
                o = unpack(bits, 0);
        }

        template <typename T>
        void unpack_to(T const &bits, T &o0, T &o1) const {
                unpack_to(bits, o0);
                o1 = unpack(bits, 1);
        }

        template <typename T>
        void unpack_to(T const &bits, T &o0, T &o1, T &o2) const {
                unpack_to(bits, o0, o1);
                o2 = unpack(bits, 2);
        }

        template <typename T>
        void unpack_to(
                T const &bits, T &o0, T &o1, T &o2, T &o3
        ) const {
                unpack_to(bits, o0, o1, o2);
                o3 = unpack(bits, 3);
        }

        template <typename T>
        void unpack_to(
                T const &bits, T &o0, T &o1, T &o2, T &o3, T &o4
        ) const {
                unpack_to(bits, o0, o1, o2, o3);
                o4 = unpack(bits, 4);
        }

        template <typename T>
        void unpack_to(
                T const &bits, T &o0, T &o1, T &o2, T &o3, T &o4, T &o5
        ) const {
                unpack_to(bits, o0, o1, o2, o3, o4);
                o5 = unpack(bits, 5);
        }

        // -- capacity ---------------------------------------------------------
        bool empty() const { return masks_.empty(); }
        size_type size() const { return masks_.size(); }

        // -- modifiers --------------------------------------------------------
        void push_back(bitmask const &v) { masks_.push_back(v); }
        void clear() { masks_.clear(); }
        void resize(size_type count) { masks_.resize(count); }

private:
        std::vector<bitmask> masks_;
};

template <typename T>
class from_proxy {
public:
        from_proxy(bitmask_vector const &vec, T const &bits) :
                vec_(vec), bits_(bits)
        { }

        T operator[] (bitmask_vector::size_type i) const {
                return vec_[i].unpack(bits_);
        }

        T at (bitmask_vector::size_type i) const {
                return vec_.at(i).unpack(bits_);
        }

        void to(T &o) const {
                o = unpack(0);
        }

        void to(T &o0, T &o1) const {
                to(o0);
                o1 = unpack(1);
        }

        void to(T &o0, T &o1, T &o2) const {
                to(o0, o1);
                o2 = unpack(2);
        }

        void to(T &o0, T &o1, T &o2, T &o3) const {
                to(o0, o1, o2);
                o3 = unpack(3);
        }

        void to(T &o0, T &o1, T &o2, T &o3, T &o4) const {
                to(o0, o1, o2, o3);
                o4 = unpack(4);
        }

        void to(T &o0, T &o1, T &o2, T &o3, T &o4, T &o5) const {
                to(o0, o1, o2, o3, o4);
                o5 = unpack(5);
        }
private:
        bitmask_vector const &vec_;
        T const &bits_;

        T unpack(bitmask_vector::size_type n) const {
                return vec_.unpack(bits_, n);
        }
};

}

// puffin::bitmask_vector fmt;
// fmt.push_back(puffin::bitmask::Bitmask32(0xff0000));
// fmt.push_back(puffin::bitmask::Bitmask32(0x00ff00));
// fmt.push_back(puffin::bitmask::Bitmask32(0x0000ff));
// std::cout << "red_mask:  " << fmt[0] << std::endl;
// std::cout << "green_mask:" << fmt[1] << std::endl;
// std::cout << "blue_mask: " << fmt[2] << std::endl;
// uint32_t rgb = (11<<16) | (10<<8) | (9);
// std::cout << "red  :" << (fmt.unpack(rgb, 0)) << std::endl;
// std::cout << "green:" << (fmt.unpack(rgb, 1)) << std::endl;
// std::cout << "blue :" << (fmt.unpack(rgb, 2)) << std::endl;
//
// uint32_t r, g, b;
// fmt.unpack(rgb).to(r,g,b);
// std::cout << r << ":" << g << ":" << b << std::endl;
// std::cout << fmt.unpack(rgb)[0] << ":" << fmt.unpack(rgb).at(1) << ":" << fmt.unpack(rgb).at(2) << std::endl;

#endif //BITMASK_HH_INCLUDED_20190102
