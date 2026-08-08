// Team DAIX, 2026
// HYDRAULICPRESS
//
// The majority of this code was written between 2020 and 2022
//
// This tool's purpose is to:
// 1. Load configuration from TextConfig.config
// 2. Detect the target platform (PC / Switch / Xbox)
// 3. Load the variable definition file (vars.txt / vars_bak.txt)
// 4. Identify which script files need processing
// 5. Run the selected compilation steps:
//      - Variable checking
//      - Variable replacement
//      - Accent replacement (LooseFloorboard)
//      - Text randomization
//      - Word counting
//
// HydraulicPress coordinates all sub‑modules and produces the final compiled script.


#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <thread>
#include <regex>
#include <fstream>
#include <random>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../Common/Common.h"
#include "../Common/Config.h"

#include "StringUtils.h"
#include "FileUtils.h"
#include "Randomizer.h"
#include "WordCounter.h"
#include "VariableReplacer.h"
#include "VariableChecker.h"
#include "Platform.h"

int main(int argc, char* argv[]) {

	std::vector<std::string> args(argv, argv + argc);

	// Why?
	// Set locale for Italian UTF‑8 handling (affects string operations on Windows).

	setlocale(LC_ALL, "it_IT.UTF-8");

	const std::string repo = EncryptString("DGRV3");

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const dgrv3path = current_dir / repo;
	std::filesystem::path const configfile = current_dir / "TextConfig.config";

	LOG("HydraulicPress v1.2", HERE, "HydraulicPress");
	LOG("Changelog: Migrate to TextConfig.config", HERE, "HydraulicPress");
	LOG("\n", HERE, "HydraulicPress");

	// Command‑line overrides:
	// --switch → force Switch configuration
	// --pc     → force PC configuration


	bool force_switch_config = false;
	bool force_pc_config = false;

	if (argc > 0) {
		if (std::any_of(args.begin(), args.end(), [](std::string const& arg) -> bool {return Common::StringContains(arg, "--switch"); })) {
			force_switch_config = true;
		}
		if (std::any_of(args.begin(), args.end(), [](std::string const& arg) -> bool {return Common::StringContains(arg, "--pc"); })) {
			force_pc_config = true;
		}
	}

	// In case there is HydraulicPress.txt, delete it
	// as it's just a leftover from before
	// and we don't want to append text into it
	// but we want to create a new file instead

	if (std::filesystem::exists(EncryptString("HydraulicPress.txt"))) {
		// Remove old HydraulicPress.txt (legacy output file).
		// Prevents accidental appending from older versions.

		std::filesystem::remove(EncryptString("HydraulicPress.txt"));
	}

	LOG("Reading config", HERE, "HydraulicPress");
	Configuration::ReadConfig(configfile);
	Configuration::ViewDebugCurrentConfig();
	LOG("Done reading config", HERE, "HydraulicPress");

	// Get the name of the file
	// which contains the variables
	// to be replaced later
	LOG("Current directory: " + current_dir.string(), HERE, "HydraulicPress");
	LOG("Getting the files with variables...", HERE, "HydraulicPress");
	// Determine which variable file to use:
	// vars.txt (if present)
	// otherwise vars_bak.txt from the repo.

	std::string const vars_txt = FileUtils::GetInputFile(dgrv3path);

	// If the input (variable) file is empty then
	// something went wrong
	if (vars_txt.empty()) {
		Common::WaitExit();
		return EXIT_FAILURE;
	}

	// Does it exist? Can it be red correctly?
	std::ifstream in_vars(vars_txt, std::ios::in);
	if (!in_vars.good()) {
		Common::WaitExit();
		return EXIT_FAILURE;
	}
	in_vars.close();

	// Read input and get variables
	// (this time it's true)
	// Parse the variable file into a list of (name, value) pairs.
	// These will be used for replacement and validation.

	auto const variables = FileUtils::ReadInputFile(vars_txt);
	LOG("Input file (" + StringUtils::TrimDirectories(vars_txt) + ") red!", HERE, "HydraulicPress");

	if (variables.empty()) {
		LOG("WARNING: Variable list empty! (Exiting)", HERE, "HydraulicPress");
		return EXIT_FAILURE;
	}
	else {
		/*
		for (auto const& v : variables) {
			std::cout << "[" << v.first << "] <-> [" + v.second + "]" << std::endl;
		}
		*/
	}

	// Get the list of files to search for variables
	// fts = files to search (vector of strings which contains filenames)
	LOG("Calculating the files with variables to replace...", HERE, "HydraulicPress");
	// Determine which script files need processing.
	// If CheckAllFiles = true → scan entire repo.
	// Otherwise → use different_hashes.txt from Necronomicon.

	auto const fts = FileUtils::GetFilesToSearch(dgrv3path);
	LOG("Files calculated!", HERE, "HydraulicPress");

	if (fts.empty()) {
		LOG("WARNING: No files were found... (Exiting)", HERE, "HydraulicPress");
		Common::WaitExit();
		return EXIT_FAILURE;
	}

	// Select the active platform based on:
	// - command‑line flags
	// - TextConfig.config settings
	// Sets both Configuration::CurrentPlatform and Distribution::Platform.

	if ((!force_pc_config && force_switch_config) || Configuration::ConfigMap["UseSwitchConfiguration"]) {
		Configuration::CurrentPlatform = Configuration::Platform::SWITCH;
		Distribution::Platform = "SWITCH";
		LOG("Automatically detected Switch platform", HERE, "HydraulicPress");
	}
	else {
		if(!force_pc_config && Configuration::ConfigMap["UseXboxConfiguration"]) {
			Configuration::CurrentPlatform = Configuration::Platform::XBOX;
			Distribution::Platform = "XBOX"; // The Xbox version uses the PC version, thank god (not so much for .arc files)
			LOG("Automatically detected Xbox platform", HERE, "HydraulicPress");
		}
		else {
			Configuration::CurrentPlatform = Configuration::Platform::PC;
			Distribution::Platform = "PC";
			LOG("Manually selected PC platform", HERE, "HydraulicPress");
		}
	}

	// Check available RAM to decide whether heavy operations are allowed.
	// Variable checking and randomization require ~7GB.
	// Word counting requires ~15GB.

	auto const gb = CountUtils::GetAvailableRAMInGB();

	LOG("Compiling for platform: " + Distribution::Platform + " with " + std::to_string(gb) + "GB of RAM", HERE, "HydraulicPress");

	// Identify variables used in the script but missing from the variable list.
	// Produces variablechecker.txt and variablechecker_short.txt.

	if (Configuration::ConfigMap["DoCheckVariables"]) {
		if (gb < 7.0f) {
			LOG("You have about " + std::to_string(gb) + "GB of RAM, which is not enough to check variables...", HERE, "HydraulicPress");
		}
		else {
			LOG("You have about " + std::to_string(gb) + "GB of RAM, which is enough to check variables!", HERE, "HydraulicPress");
			LOG("Checking variables...", HERE, "HydraulicPress");
			VariableChecker::CheckVariables(variables, fts);
			LOG("Variables checked!", HERE, "HydraulicPress");
		}
	}

	/*
	std::cout << "Files to search: " << std::endl;
	for (auto const& f : fts) {
		std::cout << f << std::endl;
	}
	*/

	LOG("Replacing variables (if present)...", HERE, "HydraulicPress");
	// Replace the variables
	// Replace all VAR_* occurrences in script files using the loaded variable list.
	// Produces the final compiled script.

	VariableUtils::ReplaceVariables(fts, variables, true);
	LOG("Replacing done!", HERE, "HydraulicPress");
	LOG("\n", HERE, "HydraulicPress");

	// Replace accented characters for platforms that do not support them.
	// Uses LooseFloorboard.exe (C# tool).

	if (Configuration::ConfigMap["ReplaceBlacklistedChars"]) {
		LOG("WARNING: The selected platform does not support accented characters, replacing them...", HERE, "HydraulicPress");
		// LooseFloorboard is made in C#
#ifdef _WIN32
		Common::executeBatch("LooseFloorboard.exe");
#else
		Common::executeBatch("./LooseFloorboard");
#endif
		LOG("Accented characters replaced!", HERE, "HydraulicPress");
	}

	// Randomize text for debugging/stress‑testing.
	// Not compatible with DoReplaceEmpty.

	if (Configuration::ConfigMap["DoRandomize"]) {
		if (gb < 7.0f) {
			LOG("You have about " + std::to_string(gb) + "GB of RAM, which is not enough to randomize text...", HERE, "HydraulicPress");
		}
		else {
			if (Configuration::ConfigMap["DoReplaceEmpty"]) {
				LOG("DoReplaceEmpty is not compatible with the randomizer. Sorry!", HERE, "HydraulicPress");
			}
			else {
				LOG("You have about " + std::to_string(gb) + "GB of RAM, which is enough to randomize text!", HERE, "HydraulicPress");
				LOG("Randomizing text...", HERE, "HydraulicPress");
				Randomizer::Randomize(fts);
				LOG("Text randomized!", HERE, "HydraulicPress");
			}
		}
	}

	// Count word frequency across all processed files.
	// Produces words_counted.txt (top 200 words).

	if (Configuration::ConfigMap["DoCountWords"]) {
		if (gb < 15.0f) {
			LOG("You have about " + std::to_string(gb) + "GB of RAM, which is not enough to count words...", HERE, "HydraulicPress");
		}
		else {
			LOG("You have about " + std::to_string(gb) + "GB of RAM, which is enough to count words!", HERE, "HydraulicPress");
			LOG("Counting words...", HERE, "HydraulicPress");
			CountUtils::CountWords(fts);
			LOG("Words counted!", HERE, "HydraulicPress");
		}
	}
	else {
		LOG("Not doing DoCountWords", HERE, "HydraulicPress");
	}

	//Common::BlockExit();
	Common::WaitExit();
}