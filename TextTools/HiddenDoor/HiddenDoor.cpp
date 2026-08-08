// Team DAIX, 2026
// HIDDENDOOR

// The majority of this code was written between 2020 and 2022

// This tool's purpose is to:
// Run every other program except for StackedBooks, in the designated order

#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <numeric>

#include "../Common/Common.h"

#define DISTRIBUTE

void DisplayTime(void);

struct Command {
	std::string Filename = "";
	std::vector<std::string> Arguments{};

	Command() = default;
	explicit Command(std::string const& name, std::vector<std::string> const& arg = {}) {
		this->Filename = name;
		this->Arguments = arg;
	}
	virtual compl Command() = default;
};

int main(int argc, char* argv[]) {

	bool all = false; // Do everything?
	bool skip_ropeway = false; // Assuming you already set up everything offline
	bool report = false; // Create a report file I guess?

	std::vector<std::string> args(argv, argv + argc);

	// Find args

	if (argc > 0) {
		if (std::any_of(args.begin(), args.end(), [](std::string const& arg) -> bool {return Common::StringContains(arg, "--all"); })) {
			all = true;
		}
		if (std::any_of(args.begin(), args.end(), [](std::string const& arg) -> bool {return Common::StringContains(arg, "--report"); })) {
			report = true;
		}
		// Skipping Ropeway? Make sure you already have everything in the folder
		if (std::any_of(args.begin(), args.end(), [](std::string const& arg) -> bool {return Common::StringContains(arg, "--skip-ropeway"); })) {
			skip_ropeway = true;
		}
	}

	std::vector<Command> commands{};

	std::string const all_string = all ? EncryptString(" --all") : "";
	std::string const report_string = report ? EncryptString(" >> report.txt") : "";
	std::string const second_part = " part2"; // Some programs are split into two "tasks", which is a terrible way to go about it, I know
	std::string const force_platf = " --pc"; // This is probably deprecated but sure
	std::string const force_beta = " --beta";

	// GitHub clone
	std::string const ropeway = EncryptString("Ropeway");

	// Unzipper
	std::string const crammedpiranhas = EncryptString("CrammedPiranhas");

	// Line calculation
	std::string const markerstone = EncryptString("MarkerStone");

	// Hash calculation
	std::string const necronomicon = EncryptString("Necronomicon");

	// Replaces names (C#)
	std::string const bugvac = EncryptString("Bugvac");

	// Replace variables with strings
	std::string const hydraulicpress = EncryptString("HydraulicPress");

	// Calculate percentages
	std::string const poolrules = EncryptString("PoolRules");

	// Mod the game files
	std::string const electrohammer = EncryptString("Electrohammer");

	// Copy the modified files to another folder
	std::string const crossbow = EncryptString("Crossbow");

#ifdef DISTRIBUTE

	// Create patch
	std::string const whitesheet = EncryptString("WhiteSheet") + report_string;

#endif

#ifndef _WIN32
	Common::executeBatch("chmod -R +x *");
#endif

	DisplayTime();

	LOG("Checking dependencies:", HERE, "HiddenDoor");
	// Get info about Git (Ropeway needs it)
	Common::executeBatch("git --version");
	// Get info about .NET (Electrohammer needs it because SPCTool/((old) STXTool) need it)
	Common::executeBatch("dotnet --info");
	LOG("\n", HERE, "HiddenDoor");

	// Check which programs actually need to run

	if (!skip_ropeway) {
		commands.push_back(Command{ ropeway,			{report_string
			//, force_beta
			}});
	}
	commands.push_back(Command{ crammedpiranhas,		{report_string}					});
	commands.push_back(Command{ markerstone,			{all_string, report_string}		});
	commands.push_back(Command{ necronomicon,			{all_string, report_string}		});
	commands.push_back(Command{ bugvac,					{report_string}					});
	commands.push_back(Command{ hydraulicpress,			{report_string
		//, force_platf
		}});
	commands.push_back(Command{ poolrules,				{report_string}					});
	commands.push_back(Command{ electrohammer,			{all_string, report_string}		});
	commands.push_back(Command{ crossbow,				{report_string}					});

#ifdef DISTRIBUTE

	commands.push_back(Command{ crammedpiranhas,		{second_part, report_string}				});

	commands.push_back(Command{ whitesheet,				{report_string}								});

	commands.push_back(Command{ crossbow,				{second_part, report_string} });

	commands.push_back(Command{ necronomicon,			{all_string, second_part, report_string}	});

	commands.push_back(Command{ whitesheet,				{second_part, report_string}				});
#endif

	// Run the programs

	for (auto& c : commands) {
#ifdef _WIN32
		c.Filename += ".exe";
#else
		c.Filename = "./" + c.Filename;
#endif
		if (!std::filesystem::exists(c.Filename)) {
			LOG("ERROR: \"" + c.Filename + "\" could not be found!", HERE, "HiddenDoor");
			LOG("The execution of the program cannot continue.", HERE, "HiddenDoor");
			return -1;
		}

		if (std::filesystem::exists("ropeway_failed.txt") || std::filesystem::exists("vgit_failed.txt")) {
			LOG("ERROR: Interrupting compilation due to cloning error...", HERE, "HiddenDoor");
			break;
		}

		int ret = Common::executeBatch(c.Filename.c_str(), c.Arguments);
		if (ret != 0) {
			LOG("ERROR: \"" + c.Filename + "\" failed with exit code " + std::to_string(ret),
				HERE, "HiddenDoor");
			break; // stop on first failure
		}
	}

	if (commands.empty()) {
		LOG("ERROR: Couldn't find any program to run!", HERE, "HiddenDoor");
	}

	LOG("All done!", HERE, "HiddenDoor");
	LOG("\n", HERE, "HiddenDoor");

	DisplayTime();
}

void DisplayTime(void) {
#ifdef CPP20
	std::chrono::zoned_time const zt{ "Europe/Amsterdam", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()) };
	auto const lt = zt.get_local_time();
	auto const tod = lt - std::chrono::floor<std::chrono::days>(lt);
	auto const tm = std::chrono::hh_mm_ss{ tod };
	std::stringstream ss{};
	ss << tm;
	LOG("\n", HERE, "HiddenDoor");
	LOG(ss.str(), HERE, "HiddenDoor");
	LOG("\n", HERE, "HiddenDoor");
#endif
}