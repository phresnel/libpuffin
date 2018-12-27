#ifndef EXCEPTIONS_HH_INCLUDED_20181217
#define EXCEPTIONS_HH_INCLUDED_20181217

#include <stdexcept>
#include <sstream>

namespace puffin { namespace exceptions {

// -- load_image ---------------------------------------------------------------
class load_image : public std::runtime_error {
public:
        load_image() : std::runtime_error("error while loading image file") { }
        load_image(std::string const &msg) : std::runtime_error(msg) { }
};


// -- file_not_found -----------------------------------------------------------
class file_not_found : public std::runtime_error {
public:
        file_not_found() :
                filename(""),
                std::runtime_error("file not found")
        { }

        file_not_found(std::string const &filename) :
                filename(filename),
                std::runtime_error("file not found: " + filename)
        { }

        std::string filename;
};


// -- palette_index_out_of_range -----------------------------------------------
class palette_index_out_of_range : public load_image {
public:
        palette_index_out_of_range(int i, int max) :
                load_image(fmt_msg(i, max)),
                index(i),
                min(0),
                max(max)
        { }

        int index;
        int min;
        int max;
private:
        palette_index_out_of_range(); // delete

        static
        std::string fmt_msg(int index, int max) {
                std::stringstream ss;
                ss << "palette index " << index
                   << " out of range [" << 0 << ".." << max << ")";
                return ss.str();
        }
};

} }

#endif //EXCEPTIONS_HH_INCLUDED_20181217
