#ifndef EXCEPTIONS_HH_INCLUDED_20181217
#define EXCEPTIONS_HH_INCLUDED_20181217

#include <stdexcept>

namespace puffin { namespace exceptions {

// -- file_not_found -----------------------------------------------------------
class file_not_found : public std::runtime_error {
public:
        file_not_found(std::string const &filename) :
                std::runtime_error("file \"" + filename + "\" not found")
        { }
};

// -- invalid_or_unsupported ---------------------------------------------------
class invalid_or_unsupported : public std::runtime_error {
public:
        invalid_or_unsupported(std::string const &filename) :
                std::runtime_error(
                        "invalid file or unsupported format in file \"" +
                        filename +
                        "\"")
        { }
};

} }

#endif //EXCEPTIONS_HH_INCLUDED_20181217
