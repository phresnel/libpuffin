#ifndef CONTRACT_HH_INCLUDED_20181221
#define CONTRACT_HH_INCLUDED_20181221

#include <stdexcept>
#include <string>

namespace puffin { namespace impl {

template <typename T>
inline T positive(T value) {
        if (value >= static_cast<T>(0))
                return value;
        using std::to_string;
        throw std::invalid_argument(
                "value (==" + to_string(value) + ") must be positive");
}

template <typename T>
inline T greater_than(T value, T min) {
        if (value > min)
                return value;
        using std::to_string;
        throw std::invalid_argument(
                "value (==" + to_string(value) + ") " +
                "must be greater than " +  to_string(min));
}

template <typename T>
inline T greater_or_equal(T value, T min) {
        if (value >= min)
                return value;
        using std::to_string;
        throw std::invalid_argument(
                "value (==" + to_string(value) + ") " +
                "must be greater than or equal " + to_string(min));
}

template <typename T>
inline T less_than(T value, T max) {
        if (value < max)
                return value;
        using std::to_string;
        throw std::invalid_argument(
                "value (==" + to_string(value) + ") " +
                "must be less than " + to_string(max));
}

template <typename T>
inline T less_or_equal(T value, T max) {
        if (value <= max)
                return value;
        using std::to_string;
        throw std::invalid_argument(
                "value (==" + to_string(value) + ") " +
                "must be less than or equal " +
                to_string(max));
}

} }

#endif //CONTRACT_HH_INCLUDED_20181221
