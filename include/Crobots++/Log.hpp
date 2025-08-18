#pragma once

#include <format>
#include <string>

#define CROBOTS_LOG(fmt, ...) \
    ::Crobots::Internal::Log(std::format("[{}:{}] " fmt, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__))

namespace Crobots::Internal
{

void Log(const std::string& string);

}