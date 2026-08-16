/// Team DAIX, 2026
// MONOKUMA

// The majority of this code was written between 2020 and 2022

// This tool's purpose is:
// Monokuma is the master controller for the entire FontTools pipeline. It runs
// every FontTools component in the correct canonical order, handles dependency
// checks, and performs final packaging steps for game_resident fonts.
//
// It performs:
//   • Dependency checks (Git, .NET, 7za)
//   • Sequential execution of:
//         Monotaro   — clone/update DGRV3-Font repository
//         Monophanie — extract SPCTool/HTFont archives, prepare base fonts
//         Monodam    — compile fonts (HTFont → STX/SRDV → SPC insertion)
//         Monokid    — filter SPCs, keep only modified ones
//         Monosuke   — generate UPS patches (if DISTRIBUTE enabled)
//   • Final packaging of v3_font00.stx and v3_font00.srdv into gr_font.7z
//     inside DGRV3/base_spc
//
// Monokuma is the FontTools equivalent of Pianist from ImageTools. It ensures
// that all font compilation steps run in the correct order and that the final
// distributable artifacts are produced cleanly and reproducibly.

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cstdlib>
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
	std::string const monotaro = EncryptString("Monotaro");

	// Unzipper
	std::string const monophanie = EncryptString("Monophanie");

	// Mod the game files
	std::string const monodam = EncryptString("Monodam");

	// Copy the modified files to another folder
	std::string const monokid = EncryptString("Monokid");

#ifdef DISTRIBUTE

	// Create patch
	std::string const monosuke = EncryptString("Monosuke");

#endif

#ifndef _WIN32
	Common::executeBatch("chmod -R +x *");
#endif

	DisplayTime();

	LOG("Checking dependencies:", HERE, "Monokuma");
	// Get info about Git (Ropeway needs it)
	Common::executeBatch("git --version");
	// Get info about .NET (Electrohammer needs it because SPCTool/((old) STXTool) need it)
	Common::executeBatch("dotnet --info");
	LOG("\n", HERE, "Monokuma");

	commands.push_back({ monotaro, {report_string} });
	commands.push_back({ monophanie, {report_string} });
	commands.push_back({ monodam, {all_string, report_string} });
	commands.push_back({ monokid, {all_string, report_string} });

#ifdef DISTRIBUTE

	commands.push_back({ monosuke, {report_string} });

#endif

	for (auto c : commands) {
#ifdef _WIN32
		c.Filename += ".exe";
#else
		c.Filename = "./" + c.Filename;
#endif
		if (!std::filesystem::exists(c.Filename)) {
			LOG("ERROR: \"" + c.Filename + "\" could not be found!", HERE, "Monokuma");
			LOG("The execution of the program cannot continue.", HERE, "Monokuma");
			return -1;
		}

		if (std::filesystem::exists("ropeway_failed.txt") || std::filesystem::exists("vgit_failed.txt")) {
			LOG("ERROR: Interrupting compilation due to cloning error...", HERE, "Monokuma");
			break;
		}

		Common::executeBatch(c.Filename.c_str(), c.Arguments);
	}

	if (commands.empty()) {
		LOG("ERROR: Couldn't find any program to run!", HERE, "Monokuma");
		return EXIT_FAILURE;
	}

	std::filesystem::path const cur = std::filesystem::current_path();

#ifdef _WIN32
	std::filesystem::path const _7za = cur / "7za.exe";

	if (!std::filesystem::exists(_7za)) {
		LOG("ERROR: 7za could not be found!", HERE, "Monokuma");
		return EXIT_FAILURE;
	}
#else
	std::filesystem::path const _7za = "7za";
#endif

	// Locate the main DGRV3 repository from TextTools (NOT DGRV3-Font)
	std::filesystem::path const dgrv3 = cur / "DGRV3";
	if (!std::filesystem::exists(dgrv3)) {
		LOG("WARNING: " + dgrv3.string() + " could not be found (it's fine if it's not being ran by Daily)!", HERE, "Monokuma");
		return EXIT_FAILURE;
	}

	// Locate base_spc and the existing gr_font.7z inside the main repo
	std::filesystem::path const v3_base_spc = dgrv3 / "base_spc";
	std::filesystem::path const gr_font = v3_base_spc / "gr_font.7z";

	// Locate the newly compiled font00 files inside DGRV3-Font
	std::filesystem::path const cur_font = cur / "DGRV3-Font";
	if (!std::filesystem::exists(cur_font)) {
		LOG("ERROR:" + cur_font.string() + " could not be found!", HERE, "Monokuma");
		return EXIT_FAILURE;
	}

	std::filesystem::path const cur_gr = cur_font / "game_resident";
	std::filesystem::path const cur_gr_font = cur_gr / "game_resident_US_DEC";
	std::filesystem::path const new_stx = cur_gr_font / "v3_font00.stx";
	std::filesystem::path const new_srdv = cur_gr_font / "v3_font00.srdv";

	// Delete the old gr_font.7z
	std::string const remove_command = "rm " + gr_font.string();
	Common::executeBatch(remove_command.c_str());

	// Create a new gr_font.7z containing the updated font00 files
#ifdef _WIN32
	std::string const create_7z = "\"" + _7za.string() + "\" a " + gr_font.string() + " " + new_stx.string() + " " + new_srdv.string();
#else
	std::string const create_7z = "7za a " + gr_font.string() + " " + new_stx.string() + " " + new_srdv.string();
#endif

	Common::executeBatch(create_7z.c_str());

	LOG("All done!", HERE, "Monokuma");
	LOG("\n", HERE, "Monokuma");

	DisplayTime();
}

void DisplayTime(void) {
#ifdef CPP20
	std::chrono::zoned_time const zt{ "Europe/Amsterdam", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()) };
	auto const lt = zt.get_local_time();
	auto const tod = lt - std::chrono::floor<std::chrono::days>(lt);
	auto const tm = std::chrono::hh_mm_ss{ tod };
	std::stringstream time_ss{};
	time_ss << tm;
	LOG("\n", HERE, "Monokuma");
	LOG(time_ss.str(), HERE, "Monokuma");
	LOG("\n", HERE, "Monokuma");
#endif
}