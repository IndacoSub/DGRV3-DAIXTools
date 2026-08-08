// Team DAIX, 2026
// CRAMMEDPIRANHAS
//
// The majority of this code was written between 2020 and 2022
//
// This tool's purpose is to:
// 1. Extract all SPC-related archives for the selected platform (PC / Switch / Xbox)
// 2. Unpack additional assets such as fonts, STX tools, and SPCTool
// 3. Handle a second-stage extraction mode ("part2") for distributing base SPCs
//
// It relies on 7za for all archive operations.

#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>
#include <thread>
#include <chrono>
#include <sstream>
#include <fstream>
#include <map>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../Common/Common.h"
#include "../Common/Config.h"

bool Extract7z(std::filesystem::path const& file, std::filesystem::path const& where, std::filesystem::path const& _7zadir);

int main(int argc, char* argv[]) {

	bool part2 = false;

	std::vector<std::string> args(argv, argv + argc);

	if (std::any_of(args.begin(), args.end(), [](std::string const& arg) -> bool {return Common::StringContains(arg, "part2"); })) {
		// The program is being used again, this time with the part2 argument
		part2 = true;
	}

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const configfile = current_dir / "TextConfig.config";

	LOG("Reading config", HERE, "CrammedPiranhas");
	Configuration::ReadConfig(configfile);
	Configuration::ViewDebugCurrentConfig();
	LOG("Done reading config", HERE, "CrammedPiranhas");

	bool static constexpr UseNewSTXTool = true;

	std::string const repo = EncryptString("DGRV3");

	std::filesystem::path const dgrv3path = current_dir / repo;
	std::filesystem::path const where = (dgrv3path / EncryptString("base_spc"));

	if (!std::filesystem::exists(dgrv3path)) {
		LOG("ERROR: Repo folder not found: " + dgrv3path.string(), HERE, "CrammedPiranhas");
		return -1;
	}
	if (!std::filesystem::exists(where)) {
		LOG("ERROR: base_spc folder not found: " + where.string(), HERE, "CrammedPiranhas");
		return -1;
	}

	// Part2 is shorter so we can move it up here
	// Stage 2: only extract SPC archives into Distribute and trial folders.
	// No STX/SPC tools are extracted in this mode.

	{
		if (part2) {
			// Extract base SPCs in "distribute" folder
			std::filesystem::create_directory(current_dir / EncryptString("Distribute"));

			// Select the correct SPC archive depending on the target platform (PC / Switch / Xbox).

			if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
				// Extract i18n in "distribute" folder
				Extract7z(where / EncryptString("i18n_switch.7z"), current_dir / EncryptString("Distribute"), where);
				Extract7z(where / EncryptString("danganronpa_spc_switch.7z"), current_dir / EncryptString("Distribute"), where);
			}
			else {
				if (Configuration::ConfigMap["UseXboxConfiguration"]) {
					Extract7z(where / EncryptString("danganronpa_spc_xbox.7z"), current_dir / EncryptString("Distribute"), where);
				}
				else {
					// PC
					Extract7z(where / EncryptString("danganronpa_spc_legacy.7z"), current_dir / EncryptString("Distribute"), where);
				}
			}

			// We want French files because they have better text layout n' stuff in the trial minigames

			Extract7z(where / EncryptString("trial_french.7z"), where / EncryptString("trial_french_extracted"), where);
			Extract7z(where / EncryptString("trial_english.7z"), where / EncryptString("trial_english_extracted"), where);

			return EXIT_SUCCESS;
		}
	}

	// Part 1

	LOG("Extracting archives...", HERE, "CrammedPiranhas");

	std::filesystem::path _7zfile{};

	if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
		_7zfile = where / EncryptString("danganronpa_spc_switch.7z");
	}
	else {
		if (Configuration::ConfigMap["UseXboxConfiguration"]) {
			_7zfile = where / EncryptString("danganronpa_spc_xbox.7z");
		}
		else {
			// PC
			_7zfile = where / EncryptString("danganronpa_spc_legacy.7z");
		}
	}

	if (std::filesystem::exists(_7zfile)) {
		if (!Extract7z(_7zfile, where, where)) {
			return -1;
		}
	}

	_7zfile = where / EncryptString("gr_font.7z");
	if (std::filesystem::exists(_7zfile)) {
		if (!Extract7z(_7zfile, where, where)) {
			return -1;
		}
	}

	// Extract the STX tool used for processing text assets.
	// 'UseNewSTXTool' switches between legacy and updated versions.


	if constexpr (UseNewSTXTool) {
#ifdef _WIN32
		_7zfile = where / EncryptString("NewSTXTool.7z");
#else
		_7zfile = where / EncryptString("NewSTXTool_Linux.7z");
#endif
	}
	else {
#ifdef _WIN32
		_7zfile = where / EncryptString("STXTool.7z");
#else
		_7zfile = where / EncryptString("STXTool_Linux.7z");
#endif
	}
	if (std::filesystem::exists(_7zfile)) {
		if (!Extract7z(_7zfile, current_dir, where)) {
			return -1;
		}
	}

	//std::remove(_7zfile.string().c_str());

	// Extract SPCTool, required for unpacking and repacking SPC files.

#ifdef _WIN32
	_7zfile = where / EncryptString("SPCTool.7z");
#else
	_7zfile = where / EncryptString("SPCTool_Linux.7z");
#endif
	if (std::filesystem::exists(_7zfile)) {
		if (!Extract7z(_7zfile, current_dir, where)) {
			return -1;
		}
	}

	//std::remove(_7zfile.string().c_str());

	if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
		_7zfile = where / EncryptString("i18n_switch.7z");

		if (std::filesystem::exists(_7zfile)) {
			if (!Extract7z(_7zfile, current_dir, where)) {
				return -1;
			}
		}
	}

#ifndef _WIN32
	Common::executeBatch("chmod -R +x *");
#endif

	Common::WaitExit();
}

bool Extract7z(std::filesystem::path const& file, std::filesystem::path const& where, std::filesystem::path const& _7zadir) {

#ifdef _WIN32
	std::filesystem::path const _7zipa = _7zadir / EncryptString("7za.exe");

	if (!std::filesystem::exists(_7zipa)) {
		LOG("ERROR: 7za could not be found!", HERE, "CrammedPiranhas");
		return false;
	}

	// Build the 7za extraction command.
	// -aoa = overwrite all existing files without prompting.


	std::string const _7zcommand = "\"" + _7zipa.string() + "\"" + std::string{ " x \"" } + file.string() + "\" -o\"" + where.string() + "\" -aoa";
#else
	// On Linux, assume '7za' is available in PATH.

	std::filesystem::path const _7zipa = "7za";
	std::string const _7zcommand = "7za x \"" + file.string() + "\" -o\"" + where.string() + "\" -aoa";
#endif

	if (Common::executeBatch(_7zcommand.c_str()) != 0) {
		LOG("ERROR: Extraction failed for " + file.string(), HERE, "CrammedPiranhas");
		return false;
	}
	return true;
}