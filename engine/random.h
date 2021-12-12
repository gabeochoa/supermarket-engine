
#pragma once

#include <random>

#include "pch.hpp"

inline int randIn(int a, int b) { return a + (std::rand() % (b - a + 1)); }
