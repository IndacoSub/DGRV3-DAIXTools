// Team DAIX, 2026
// PIANIST

// The majority of this code was written between 2020 and 2022

// This tool's purpose is to:
// Run every other program in the designated order

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cstring>

#include "../Common/Common.h"

#ifdef _WIN32
#define CPP20
#endif

#define DISTRIBUTE

void DisplayTime(void);

struct Command {
	std::string Filename = "";
	std::vector<std::string> Arguments = { {} };

	Command() = default;
	Command(std::string const& name, std::vector<std::string> const& arg = {}) {
		this->Filename = name;
		this->Arguments = arg;
	}
	virtual compl Command() = default;
};

int main(int argc, char* argv[]) {

	bool all = false;
	bool report = false;

	if (argc > 0) {
		if (argv[1] != nullptr) {
			if (strcmp(argv[1], EncryptString("--all")) == 0) {
				all = true;
			}
		}
	}

	std::vector<Command> commands{};

	std::string const all_string = all ? EncryptString(" --all") : "";
	std::string const report_string = report ? EncryptString(" >> report.txt") : "";
	std::string const second_part = " part2";

	// GitHub clone
	std::string const adventurer = EncryptString("Adventurer");

	// Extract archives
	std::string const inventor = EncryptString("Inventor");

	// Compiles SPC
	std::string const maid = EncryptString("Maid");

	// Copies files
	std::string const tennispro = EncryptString("TennisPro");

#ifdef DISTRIBUTE
	// Create patches
	std::string const detective = EncryptString("Detective");
#endif

#ifndef _WIN32
	Common::executeBatch("chmod -R +x *");
#endif

	DisplayTime();

	LOG("Checking dependencies:", HERE, "Pianist");
	// Get info about Git (Adventurer needs it)
	Common::executeBatch("git --version");
	// Get info about .NET (Maid needs it because SPCTool/STXTool need it)
	Common::executeBatch("dotnet --info");
	LOG("\n", HERE, "Pianist");

	commands.push_back({ adventurer, {report_string} });
	commands.push_back({ inventor, {report_string} });
	commands.push_back({ maid, {report_string} });
	commands.push_back({ tennispro, {report_string} });

#ifdef DISTRIBUTE

	commands.push_back({ detective, {report_string} });
#endif

	for (auto c : commands) {
#ifdef _WIN32
		c.Filename += ".exe";
#else
		c.Filename = "./" + c.Filename;
#endif
		if (!std::filesystem::exists(c.Filename)) {
			LOG("ERROR: \"" + c.Filename + "\" could not be found in " + std::filesystem::current_path().string() + "!", HERE, "Pianist");
			LOG("The execution of the program cannot continue.", HERE, "Pianist");
			return -1;
		}

		if (std::filesystem::exists("adventurer_failed.txt") || std::filesystem::exists("vgit_failed.txt")) {
			LOG("ERROR: Interrupting compilation due to cloning error...", HERE, "Pianist");
			break;
		}

		Common::executeBatch(c.Filename.c_str(), c.Arguments);
	}

	if (commands.empty()) {
		LOG("ERROR: Couldn't find any program to run!", HERE, "Pianist");
	}

	LOG("All done!", HERE, "Pianist");
	LOG("\n", HERE, "Pianist");

	DisplayTime();
}

void DisplayTime(void) {
#ifdef CPP20
	std::chrono::zoned_time const zt{ "Europe/Amsterdam", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()) };
	auto const lt = zt.get_local_time();
	auto const tod = lt - std::chrono::floor<std::chrono::days>(lt);
	auto const tm = std::chrono::hh_mm_ss{ tod };
	std::stringstream timess{};
	timess << tm;
	LOG("\n", HERE, "Pianist");
	LOG(timess.str(), HERE, "Pianist");
	LOG("\n", HERE, "Pianist");
#endif
}