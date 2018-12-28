#ifndef TYPE_STR_HH_INCLUDED_20181228
#define TYPE_STR_HH_INCLUDED_20181228

#include <string>

namespace puffin { namespace impl {

inline
std::string type_str(int const &) {
        return "int";
}

} }

#endif //TYPE_STR_HH_INCLUDED_20181228
