#ifndef VALUE_FORMAT_HH_INCLUDED_20190102
#define VALUE_FORMAT_HH_INCLUDED_20190102

#include "signedness.hh"
#include <iostream>

namespace puffin {

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

}

#endif //VALUE_FORMAT_HH_INCLUDED_20190102
