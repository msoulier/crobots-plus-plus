#pragma once

#include <Crobots++/Crobots++.hpp>

namespace Crobots
{

struct AppInfo
{
    std::string_view title;
    uint32_t arenaX;
    uint32_t arenaY;
	uint32_t nrobots;
	std::string robot1_path;
	std::string robot2_path;
	std::string robot3_path;
	std::string robot4_path;
    bool debug;
    bool verbose;
    bool damage;
};

}
