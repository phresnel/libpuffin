#ifndef SIGNEDNESS_HH_INCLUDED_20190102
#define SIGNEDNESS_HH_INCLUDED_20190102

namespace puffin {

enum signedness {
        is_unspecified = 0,
        is_signed = 1,
        is_unsigned = -1
};
typedef signedness signedness_t;

}

#endif //SIGNEDNESS_HH_INCLUDED_20190102
