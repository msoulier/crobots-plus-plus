#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#define CROBOTS_ENTRYPOINT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define CROBOTS_ENTRYPOINT __attribute__((visibility("default"))) extern "C"
#else
#define CROBOTS_ENTRYPOINT extern "C"
#endif

#include <Crobots++/IRobot.hpp>