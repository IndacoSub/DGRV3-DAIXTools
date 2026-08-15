// Team DAIX, 2026
// TENNISPRO

// The majority of this code was written between 2020 and 2022

// This tool's purpose is:
// TennisPro collects all compiled SPC/AB files produced by Maid and builds the
// final distribution folder (ModifiedFiles-GFX). It removes empty files,
// cleans leftover metadata, and copies the fully‑compiled asset tree into a
// clean folder ready for packaging or mod distribution.

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <map>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../Common/Config.h"
#include "../Common/Common.h"
#include "../Common/Cloud.h"

int main() {

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const configfile = current_dir / "ImageConfig.config";

	Configuration::ReadConfig(configfile);
	Configuration::ViewDebugCurrentConfig();

	// Repository name (and also folder)
	// Select correct graphics repo based on platform
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

	std::filesystem::path const dist = current_dir / EncryptString("ModifiedFiles-GFX");
	std::filesystem::path const dgrv3path = current_dir / repo;
	std::filesystem::path const where = (dgrv3path / basefolder);
	std::filesystem::path const nested_spc = where / Cloud::dl_repo_name;
	std::filesystem::path const copy = where / "danganronpa_files_copy";

	if (!std::filesystem::exists(dgrv3path)) {
		LOG("ERROR: The folder to copy from was not found: " + dgrv3path.string(), HERE, "TennisPro");
		return -1;
	}

	if (!std::filesystem::exists(where)) {
		LOG("ERROR: The folder to copy from was not found: " + where.string(), HERE, "TennisPro");
		return -1;
	}

	// Reference for timestamp
	// The DLL should be older than the files
	// If it's not, then the files haven't been modified correctly or modified at all
	// NOTE: SpcTool.dll does not exist in DGRV3-AB-GFX
	std::filesystem::path comparison_file{};
	if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
		comparison_file = current_dir / EncryptString("classdata.tpk");
	}
	else {
		comparison_file = current_dir / EncryptString("SpcTool.dll");
	}

	// Remove the distribution folder (ModifiedFiles-GFX) if it already exists
	if (std::filesystem::exists(dist)) {
		//std::cout << "Distribution folder already exists, deleting old one..." << std::endl;
		//std::filesystem::remove_all(dist);
	}
	else {
		std::filesystem::create_directories(dist);
	}

	// Remove empty SPC files inside nested_spc (these come from failed downloads?)
	std::vector<std::string> removec{};
	if (std::filesystem::exists(nested_spc)) {
		for (auto const& file : std::filesystem::recursive_directory_iterator(nested_spc)) {
			try {
				if (!file.is_directory() && std::filesystem::exists(file) && std::filesystem::is_empty(file)) {
					removec.push_back(file.path().string());
				}
			}
			catch (const std::filesystem::filesystem_error& e) {
				LOG(std::string("WARNING: Could not check file: ") + file.path().string() + " - " + e.what(), HERE, "TennisPro");
			}
		}
		for (auto const& f : removec) {
			std::filesystem::remove_all(f);
		}
	}

	removec.clear();

	// Remove empty SPCs inside danganronpa_files_copy as well
	if (std::filesystem::exists(copy)) {
		for (auto const& file : std::filesystem::recursive_directory_iterator(copy)) {
			try {
				if (!file.is_directory() && std::filesystem::exists(file) && std::filesystem::is_empty(file)) {
					removec.push_back(file.path().string());
				}
			}
			catch (const std::filesystem::filesystem_error& e) {
				LOG(std::string("WARNING: Could not check file: ") + file.path().string() + " - " + e.what(), HERE, "TennisPro");
			}
		}
		for (auto const& f : removec) {
			std::filesystem::remove_all(f);
		}
	}

	// Copy compiled SPCs into distribution folder
	if (std::filesystem::exists(nested_spc) && std::filesystem::exists(dist)) {
		try {
			std::filesystem::copy(nested_spc, dist,
				std::filesystem::copy_options::recursive |
				std::filesystem::copy_options::overwrite_existing);
		}
		catch (const std::filesystem::filesystem_error& e) {
			LOG(std::string("ERROR copying from ") + nested_spc.string() + " to " + dist.string() + ": " + e.what(), HERE, "TennisPro");
		}
	}

	// Helper lambda: safely remove files/folders while catching errors
	auto safe_remove = [](const std::filesystem::path& p) {
		try {
			if (std::filesystem::exists(p)) {
				if (std::filesystem::is_directory(p)) {
					std::filesystem::remove_all(p);
				}
				else {
					std::filesystem::remove(p);
				}
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			LOG(std::string("WARNING: Could not remove ") + p.string() + " - " + e.what(), HERE, "TennisPro");
		}
	};

	// Remove leftover Git metadata and README files from both folders
	safe_remove(dist / ".git");
	safe_remove(dist / "README.md");
	safe_remove(copy / ".git");
	safe_remove(copy / "README.md");

	LOG("All files copied in the distribution folder!", HERE, "TennisPro");

	Common::WaitExit();
}