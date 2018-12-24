#ifndef ALGORITHM_HH_INCLUDED_20181224
#define ALGORITHM_HH_INCLUDED_20181224

namespace puffin { namespace impl {

constexpr int clamp(int i, int lo, int hi) noexcept {
        return i<lo ? 0 : i>hi ? hi : i;
}

constexpr int clamp(int i, int m) noexcept {
        return clamp(i, 0, m);
}

constexpr int wrap (int i, int m) noexcept {
        const int m_ = m + 1; // make 0..m inclusive
        return ((i % m_) + m_) % m_;
#if 0
        // This is a variant I developed by trial and error.
        if (i < 0) {
                // Heureka?!
                // What this does:
                //   a) given -7 % 10, we want 3 as the answer.
                //   b) simply removing the '-' using abs()
                //      transforms -7 to 7. not yet what we want.
                //   c) but if we subtract 10 from 7, we reap -3.
                //      almost what we want!
                //   d) now remove the sign and be done.
                //
                // Tests:
                //   wrap(-100, 10): expected: 10
                //     abs(-100) = 100
                //     100-10 = 90
                //     abs(90) = 90
                //     90%10 = 0
                //
                //   wrap(-1007, 10): expected: 3
                //     abs(-1007) = 1007
                //     1007-10 = 997
                //     abs(997) = 997
                //     997%10 = 7
                //
                // Crap. But the basic idea is not that bad.
                // Slight modification to initial variant:
                //
                //   a) given -1007 % 10, we want 3 as the answer.
                //   b) simply removing the '-' using abs()
                //      transforms -1007 to 1007. not yet what we want.
                //   c) modulus once: 1007 % 10 = 7
                //   d) but if we subtract 10 from 7, we reap -3.
                //      almost what we want!
                //   e) now remove the sign and be done.
                //
                using std::abs;
                i = abs((abs(i)%m) - m);
        }
        return i - (i/m) * m;
#endif
}

constexpr int wrap (int i, int lo, int hi) noexcept {
        // TODO: implement case for hi<lo
        return lo + wrap(i - lo, hi - lo);
}

constexpr int mirror(int i, int m) noexcept {
        // assert: m > 0
        const bool odd = (i / m) % 2;
        const bool mirr = (i>=0) ? odd : !odd;
        const auto wrapped = wrap(i, m-1);
        return mirr ? m-wrapped : wrapped;
}

constexpr int mirror(int i, int lo, int hi) noexcept {
        return lo + mirror(i-lo, hi-lo);
}

} }

#endif //ALGORITHM_HH_INCLUDED_20181224
