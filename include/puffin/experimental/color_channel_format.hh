#ifndef COLOR_CHANNEL_FORMAT_HH_INCLUDED_20190102
#define COLOR_CHANNEL_FORMAT_HH_INCLUDED_20190102

namespace puffin {
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
}

#endif //COLOR_CHANNEL_FORMAT_HH_INCLUDED_20190102
