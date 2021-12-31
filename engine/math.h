
#pragma once

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

