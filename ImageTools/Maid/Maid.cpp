// Team DAIX, 2026
// MAID — MASTER IMAGE COMPILATION DISPATCHER

// The majority of this code was written between 2020 and 2022

// This tool's purpose is:
// Maid is the central dispatcher for ImageTools. It reads configuration flags,
// selects the correct graphics repository and base folder, and invokes the
// appropriate compilation pipeline:
//
//   • PC/Xbox → compile_pc.cpp (PNG → SRD → SPC workflow)
//   • Switch/Unity → compile_console.cpp (PNG/TGA → AssetBundle/sharedassets workflow)
//
// Orchestrates platform selection, initializes Cloud::dl_repo_name, and forwards 
// execution to the correct subsystem. It is the entry point for all image compilation
// tasks in ImageTools.

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <array>
#include <algorithm>
#include <filesystem>
#include <utility>
#include <sstream>
#include <tuple>
#include <thread>
#include <regex>
#include <map>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "Maid.h"
#include "compile_pc.h"
#include "compile_console.h"

#include "../Common/Common.h"
#include "../Common/Cloud.h"

int main() {

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const configfile = current_dir / "ImageConfig.config";

	Configuration::ReadConfig(configfile);
	Configuration::ViewDebugCurrentConfig();

	// Repository name (and also folder)
	// Select correct graphics repository based on platform.
	// This also sets Cloud::dl_repo_name for downstream tools.
	std::string repo{};

	if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
		repo = EncryptString("DGRV3-AB-GFX");
		Cloud::dl_repo_name = Cloud::dl_repo_name_switch;
	}
	else {
		repo = EncryptString("DGRV3-GFX");

		// Xbox|PC
		if (Configuration::ConfigMap["UseXboxConfiguration"]) {
			Cloud::dl_repo_name = Cloud::dl_repo_name_xbox;
		}
		else {
			// PC (Steam)
			Cloud::dl_repo_name = Cloud::dl_repo_name_pc;
		}
	}

	// Base files folder
	// Select correct base folder (Switch uses base_ab, PC/Xbox use base_spc)
	std::string basefolder{};
	if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
		basefolder = EncryptString("base_ab");
	}
	else {
		basefolder = EncryptString("base_spc");
	}

	// Dispatch to the correct compilation pipeline.
	if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
		Console::CompileConsole(current_dir, repo, basefolder);
	}
	else {
		// PC *and* Xbox
		PC::CompilePC(current_dir, repo, basefolder);
	}

	LOG("All done!", HERE, "Maid");

	Common::WaitExit();
}