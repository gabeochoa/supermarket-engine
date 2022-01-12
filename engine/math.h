
#pragma once

#include <cstdlib>
#include <limits>

template <typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

// This rounds to the Larger number maintaining the sign
// for negative this means lower, for pos this means larger
template <typename T>
T round_higher(T val) {
    int sign = sgn<T>(val);
    if (sign < 0) {
        return floor(val);
    }
    return ceil(val);
}

// This rounds to the lower number maintaining the sign
// for negative this means higher, for pos this means lower
template <typename T>
T round_lower(T val) {
    int sign = sgn<T>(val);
    if (sign < 0) {
        return ceil(val);
    }
    return floor(val);
}

inline bool close(float a, float b) {
    return abs(a - b) < std::numeric_limits<float>::epsilon();
}
