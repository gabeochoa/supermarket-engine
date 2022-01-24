
#pragma once

#include <random>

inline int randIn(int a, int b) { return a + (std::rand() % (b - a + 1)); }

template <size_t size>
struct RandomIndex {
    int index = 0;
    std::array<size_t, size> indexes;

    RandomIndex() {
        for (size_t i = 0; i < size; i++) {
            indexes[i] = i;
        }
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(std::begin(indexes), std::end(indexes), g);
    }
    size_t next() { return indexes[index++]; }
};
