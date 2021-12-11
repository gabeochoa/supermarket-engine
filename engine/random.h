
#pragma once

#include <random>

#include "pch.hpp"

int randIn(int a, int b) { return a + (std::rand() % (b - a + 1)); }
