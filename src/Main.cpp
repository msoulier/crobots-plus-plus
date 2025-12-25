#include <CLI/CLI.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <string>
#include <fstream>

#include "Api.hpp"
#include "App.hpp"

// Verbose logging.
static bool verbose = false;
static bool debug = false;
// Damage enabled?
static bool damage = true;
static bool pause_on_scan = false;

static uint32_t arenaX = 100;
static uint32_t arenaY = 100;
// FIXME: make logpath configurable
static std::string logFile{"crobots++.log"};
static std::ofstream logStream;

/* TODO(Michael): We should have enums "Robot1", "Robot2", etc, etc and an array */
static std::string robot1_path;
static std::string robot2_path;
static std::string robot3_path;
static std::string robot4_path;

static void LogCallback(void* data, int category, SDL_LogPriority priority, const char* string)
{
    SDL_GetDefaultLogOutputFunction()(data, category, priority, string);
    if (logStream.is_open())
    {
        logStream << string << '\n';
        logStream.flush();
    }
}

// https://github.com/CLIUtils/CLI11 for CLI
static bool ParseOptions(int argc, char** argv, Crobots::AppInfo& info)
{
    /* TODO: add args to AppInfo */

    CLI::App parser{"Crobots++: <arguments>"};
    argv = parser.ensure_utf8(argv);

    parser.add_flag("-v,--verbose", verbose, "Verbose logging");
    parser.add_flag("-d,--debug", debug, "Enable debug features");
    parser.add_flag("-p,--pause-on-scan", pause_on_scan, "Pause on each scan hit");
    parser.add_flag("!-D,!--no-damage", damage, "Disable damage for debugging");
    parser.add_option("-x,--arena-x", arenaX, "Arena X dimension (default 1000)")->check(CLI::Number);
    parser.add_option("-y,--arena-y", arenaY, "Arena Y dimension (default 1000)")->check(CLI::Number);
    parser.add_option("-l,--logfile", logFile, "Path to logfile (default crobots++.log)");
	parser.add_option("robot1", robot1_path, "First robot")->required();
	parser.add_option("robot2", robot2_path, "Second robot");
	parser.add_option("robot3", robot3_path, "Third robot");
	parser.add_option("robot4", robot4_path, "Fourth robot");

    try
    {
        parser.parse(argc, argv);
    }
    catch (const CLI::ParseError& e)
    {
        parser.exit(e);
        return false;
    }

    info.arenaX = arenaX;
    info.arenaY = arenaY;
	info.nrobots = 0;
	info.robot1_path = "";
	info.robot2_path = "";
	info.robot3_path = "";
	info.robot4_path = "";
    info.debug = debug;
    info.damage = damage;
    info.pause_on_scan = pause_on_scan;
    info.verbose = verbose;
	if (! robot1_path.empty())
	{
		info.nrobots++;
        info.robot1_path = robot1_path;
	}
	if (! robot2_path.empty())
	{
		info.nrobots++;
        info.robot2_path = robot2_path;
	}
	if (! robot3_path.empty())
	{
		info.nrobots++;
        info.robot3_path = robot3_path;
	}
	if (! robot4_path.empty())
	{
		info.nrobots++;
        info.robot4_path = robot4_path;
	}

    return true;
}

/* TODO: switch to callbacks when resize slowdowns on Vulkan get fixed */
int main(int argc, char** argv)
{
    Crobots::AppInfo info{};
    info.title = "Crobots++";
    if (!ParseOptions(argc, argv, info))
    {
        return 1;
    }
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    logStream.open(logFile, std::ios::out);
    if (!logStream.is_open())
    {
        CROBOTS_LOG("Failed to open log stream: %s", SDL_GetError());
    }
    SDL_SetLogOutputFunction(LogCallback, nullptr);
    Crobots::App app{};
    if (!app.Init(info))
    {
        return 1;
    }
    while (!app.ShouldQuit())
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            app.Event(&event);
        }
        app.Iterate();
    }
    app.Quit();
    SDL_ResetLogPriorities();
    SDL_SetLogOutputFunction(SDL_GetDefaultLogOutputFunction(), nullptr);
    logStream.close();
    return 0;
}
