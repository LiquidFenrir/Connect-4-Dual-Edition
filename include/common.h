#pragma once

#include <memory>
#include <map>
#include <vector>
#include <array>
#include <numeric>
#include <algorithm>
#include <functional>

#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {
#include <switch.h>
#include <SDL2/SDL.h>
#include <math.h>
}

// #define NODEBUG
#ifdef NODEBUG
#define DEBUG(...)
#else
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#endif
