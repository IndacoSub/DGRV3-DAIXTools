// Team DAIX, 2026
// INVENTOR

// The majority of this code was written between 2020 and 2022

// This tool's purpose is:
// Inventor extracts graphics/tool archives from the cloned DGRV3-GFX repository.
// It mirrors the behavior of CrammedPiranhas (Text Tools) but for image assets.
// Inventor unpacks main.zip (if FileOnDemand=false), renames the extracted repo,
// and extracts platform-specific tools (SPCTool, SRDTool, or UAFGJ). It also
// prepares a working folder (danganronpa_files_copy) containing base SPC/AB
// files so other ImageTools components can operate without modifying originals.

#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <sstream>
#include <fstream>
#include <map>
#include <cstring>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../Common/Config.h"
#include "../Common/Cloud.h"
#include "../Common/Common.h"

bool Extract7z(std::filesystem::path const& file, std::filesystem::path const& where, std::filesystem::path const& _7zadir);
bool Extract(std::string const& name, std::filesystem::path const& where, std::filesystem::path const& _7zadir);

int main(int argc, char* argv[]) {

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const configfile = current_dir / "ImageConfig.config";

	Configuration::ReadConfig(configfile);
	Configuration::ViewDebugCurrentConfig();

	// Repository name (and also folder)
	std::string repo{};

	// Select the correct graphics repository depending on platform.
	// Switch uses AB-GFX, while PC/Xbox use the standard GFX repo.

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
	// Base folder contains the unmodified image assets. Switch uses base_ab,
	// while PC/Xbox use base_spc. Inventor extracts tools and files into this folder.

	std::string basefolder{};
	if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
		basefolder = EncryptString("base_ab");
	}
	else {
		basefolder = EncryptString("base_spc");
	}

	std::string const location = basefolder;

	std::filesystem::path const dgrv3path = current_dir / repo;
	std::filesystem::path const where = (dgrv3path / location);

	// Check if the DGRV3-GFX folder exists
	// Ensure the graphics repository has already been cloned by Adventurer.
	// Inventor only extracts archives; it does not clone the repo itself.

	if (!std::filesystem::exists(dgrv3path)) {
		LOG("ERROR: The folder where the files to extract are located was not found!", HERE, "Inventor");
		return -1;
	}

	// Check if the "base_spc" / "base_ab" folder exists

	if (!std::filesystem::exists(where)) {
		LOG("ERROR: The folder where the files to extract are located was not found!", HERE, "Inventor");
		return -1;
	}

	LOG("Extracting archives...", HERE, "Inventor");

	// If FileOnDemand is disabled, extract the downloaded main.zip archive.
	// This ZIP contains the entire graphics repository as downloaded by Adventurer.

	if (!Configuration::ConfigMap["FileOnDemand"]) {
		if (!Extract((current_dir / "main.zip").string(), current_dir, where)) {
			LOG("ERROR: Couldn't extract the main.zip file", HERE, "Inventor");
			return -1;
		}

		// GitHub ZIP archives extract into "<repo>-main". Rename it to the expected folder
		// name so the rest of the pipeline can locate files consistently.

		std::string const before = std::filesystem::path(current_dir / (Cloud::dl_repo_name + "-main")).string();
		if (!std::filesystem::exists(before)) {
			LOG("WARNING: SPC Repository folder not found at: " + before, HERE, "Inventor");
		}
		else {
			std::string const after = std::filesystem::path(current_dir / Cloud::dl_repo_name).string();
			try {
				if (std::filesystem::exists(after)) {
					LOG("Destination already exists, removing: " + after, HERE, "Inventor");
					std::filesystem::remove_all(after);
				}
				std::filesystem::rename(before, after);
				LOG("Renamed " + before + " to " + after, HERE, "Inventor");
			}
			catch (const std::filesystem::filesystem_error& e) {
				LOG(std::string("ERROR renaming '") + before + "' to '" + after + "': " + e.what(), HERE, "Inventor");
			}

			std::string const after2 = std::filesystem::path(where / Cloud::dl_repo_name).string();
			try {
				if (std::filesystem::exists(after2)) {
					LOG("Destination already exists, removing: " + after2, HERE, "Inventor");
					std::filesystem::remove_all(after2);
				}
				if (std::filesystem::exists(after)) {
					std::filesystem::rename(after, after2);
					LOG("Renamed " + after + " to " + after2, HERE, "Inventor");
				}
				else {
					LOG("ERROR: Cannot rename to " + after2 + " because source does not exist: " + after, HERE, "Inventor");
				}
			}
			catch (const std::filesystem::filesystem_error& e) {
				LOG(std::string("ERROR renaming '") + after + "' to '" + after2 + "': " + e.what(), HERE, "Inventor");
			}
		}
	}

#ifdef _WIN32
	std::string const platform = "";
#else
	std::string const platform = "_Linux";
#endif

	// Extract platform-specific tools used for processing image assets.
	// PC/Xbox use SPCTool and SRDTool; Switch uses UAFGJ (Unity asset extractor).

	if (!Configuration::ConfigMap["UseSwitchConfiguration"]) {
		// Not Switch, aka PC|Xbox
		
		// Extract SPCTool

		if (!Extract((where / ("SPCTool" + platform + ".7z")).string(), current_dir, where)) {
			return -1;
		}

		// Extract SRDTool

		if (!Extract((where / ("SRDTool" + platform + ".7z")).string(), current_dir, where)) {
			return -1;
		}
	}
	else {
		// Switch/Unity
		// Extract UAFGJ
		if (!Extract((where / ("UAFGJ" + platform + ".7z")).string(), current_dir, where)) {
			return -1;
		}
	}

#ifndef _WIN32
	// On Linux, ensure extracted tools have executable permissions.

	Common::executeBatch("chmod -R +x *");
#endif

	if (!Configuration::ConfigMap["FileOnDemand"]) {
		LOG("Copying SPCs...", HERE, "Inventor");

		// Copy from directory to danganronpa_spc_copy

		// Copy the extracted graphics files into a working folder ("danganronpa_files_copy")
		// so other tools can modify them without touching the original base assets.


		try {
			std::filesystem::copy(where / Cloud::dl_repo_name, where / EncryptString("danganronpa_files_copy"), std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
		}
		catch (std::exception& e) {
			std::string estr = std::string{ e.what() };
			LOG(estr, HERE, "Inventor");
		}
	}

	Common::WaitExit();
}

bool Extract7z(std::filesystem::path const& file, std::filesystem::path const& where, std::filesystem::path const& _7zadir) {

#ifdef _WIN32
	std::filesystem::path const _7zipa = _7zadir / EncryptString("7za.exe");
#else
	std::filesystem::path const _7zipa = "7za";
#endif

#ifdef _WIN32
	if (!std::filesystem::exists(_7zipa)) {
		LOG("ERROR: 7za.exe could not be found!", HERE, "Inventor");
		return false;
	}
#endif

	// Build the 7zip extraction command. On Windows, use bundled 7za.exe;
	// on Linux, rely on system-installed 7za.

#ifdef _WIN32
	std::string const _7zcommand = "\"" + _7zipa.string() + "\"" + std::string{ " x \"" } + file.string() + "\" -o\"" + where.string() + "\" -aoa";
#else
	std::string const _7zcommand = "7za x \"" + file.string() + "\" -o\"" + where.string() + "\" -aoa";
#endif

	LOG("Performing command: " +  _7zcommand, HERE, "Inventor");
	if (Common::executeBatch(_7zcommand.c_str()) != 0) {
		LOG("7zcommand failed", HERE, "Inventor");
		return false;
	}
	return true;
}

bool Extract(std::string const& name, std::filesystem::path const& where, std::filesystem::path const& _7zadir) {

	std::filesystem::path const& file = name;
	// Wrapper around Extract7z: checks file existence and logs errors consistently.
	if (std::filesystem::exists(file)) {
		bool ok = Extract7z(file, where, _7zadir);
		if (!ok) {
			LOG("ERROR: Extraction failed for file: " + file.string(), HERE, "Inventor");
		}
		return ok;
	}
	LOG("ERROR: File not found for extraction: " + file.string(), HERE, "Inventor");
	return false;
}