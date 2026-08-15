// Team DAIX, 2026
// DETECTIVE

// The majority of this code was written between 2020 and 2022

// This tool's purpose is:
// Detective generates UPS patches for ImageTools. It compares base SPC/AB files
// against modified ones produced by Maid/TennisPro, renames base files to
// *_normal, copies modified files into a distribution folder, and invokes the
// UPS patcher to create *_patch.ups files for every changed asset.

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <thread>
#include <map>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../Common/Config.h"
#include "../Common/Common.h"

int main(int argc, char* argv[]) {

#ifdef _WIN32
	std::string ext = ".exe";
#else
	std::string ext = "";
#endif

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const configfile = current_dir / "ImageConfig.config";

	LOG("Reading config", HERE, "Detective");
	Configuration::ReadConfig(configfile);
	Configuration::ViewDebugCurrentConfig();

	// Repository name (and also folder)
	// Select correct graphics repo based on platform
	std::string repo{};
	if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
		repo = EncryptString("DGRV3-AB-GFX");
	}
	else {
		repo = EncryptString("DGRV3-GFX");
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

	std::filesystem::path const spcloc = current_dir / EncryptString("ModifiedFiles-GFX");
	std::filesystem::path const distr = current_dir / EncryptString("Distribute-GFX");
	std::filesystem::path const dgrv3 = current_dir / repo;
	std::filesystem::path const bases = dgrv3 / basefolder;
	std::filesystem::path const ups = bases / EncryptString(("ups" + ext));
	std::filesystem::path const where = bases;
	std::filesystem::path const copydir = where / EncryptString("danganronpa_files_copy");

	if (!std::filesystem::exists(copydir)) {
		LOG("ERROR: The directory to copy from (" + copydir.string() + ") does not exist!", HERE, "Detective");
		return -1;
	}

	if (!std::filesystem::exists(ups)) {
		LOG("ERROR: UPS patcher not found in " + ups.string() + "!", HERE, "Detective");
		return -1;
	}

	if (!std::filesystem::exists(where)) {
		LOG("ERROR: " + basefolder + " folder not found!", HERE, "Detective");
		return -1;
	}

	// Extract base SPCs in the distribution folder
	std::filesystem::create_directories(distr);

	// Copy base SPC/AB files into Distribute-GFX
	try {
		std::filesystem::copy(copydir, distr,
			std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
	}
	catch (const std::filesystem::filesystem_error& e) {
		LOG(std::string("ERROR copying directory: ") + e.what(), HERE, "Detective");
	}

	std::vector<std::string> filelist{};
	std::vector<std::string> modifiedfilename{};

	// For every file in Distribute-GFX
	// Collect all files in Distribute-GFX that need *_normal renaming
	for (auto const& file : std::filesystem::recursive_directory_iterator(distr)) {
		std::string const& oldname = file.path().string();

		if (file.is_directory()) {
			continue;
		}

		// Only rename files that do NOT already contain "_normal"

		// If "normal" is not found
		if (!Common::StringContains(oldname, "_normal")) {

			// Add (oldname)_normal.spc to a vector
			filelist.push_back(oldname);
		}
	}

	// Rename base files → *_normal.<ext>
	for (auto const& j : filelist) {
		// If normal is not found
		if (!Common::StringContains(j, "_normal")) {
			// Rename to (oldname)_normal.spc
			std::string const input_format = std::filesystem::path(j).extension().string();
			std::string const& newname = j.substr(0, j.length() - input_format.length()) + "_normal" + input_format;
			std::filesystem::rename(j, newname);
		}
	}

	// Collect modified SPC/AB files from ModifiedFiles-GFX
	// Add the .spc files from ModifiedFiles-GFX to a vector
	for (auto const& file : std::filesystem::recursive_directory_iterator(spcloc)) {
		if (file.is_directory()) {
			continue;
		}
		modifiedfilename.push_back(file.path().string());
	}

	// Copy modified files into Distribute-GFX, preserving folder structure
	// For every modified SPC
	for (auto const& j : modifiedfilename) {
		// Copy from ModifiedFiles-GFX to Distribute-GFX
		// (without changing names)

		std::string const fname = "ModifiedFiles-GFX";

		// Extract relative path inside ModifiedFiles-GFX
		std::string rename_later = std::filesystem::path(j).parent_path().string();
		std::size_t const find_modified = rename_later.find(fname);
		if (find_modified == std::string::npos) {
			// ModifiedFiles-GFX folder not found
			LOG("ERROR: Couldn't find " + fname, HERE, "Detective");
			return -1;
		}
		rename_later = rename_later.substr(find_modified);

		// Skip root folder itself
		if (rename_later.length() >= fname.length() + 1) {
			rename_later = rename_later.substr(fname.length() + 1); // ModifiedFiles-GFX
		}
		else {
			continue;
		}

		std::string const filename = std::filesystem::path(j).filename().string();

		std::string const before = (spcloc / rename_later / filename).string();
		std::string const after = (distr / rename_later / filename).string();

		try {
			std::filesystem::copy(before, after,
				std::filesystem::copy_options::overwrite_existing);
		}
		catch (const std::filesystem::filesystem_error& e) {
			LOG(std::string("ERROR copying file: ") + e.what(), HERE, "Detective");
		}
	}

	LOG("Generating UPS patches...", HERE, "Detective");

	// Iterate through Distribute-GFX and generate UPS patches

	for (auto const& j : std::filesystem::recursive_directory_iterator(distr)) {

		std::string const file_str = j.path().string();

		// We don't want file_str to contain "_normal"
		// (we want the "edited" files)
		if (Common::StringContains(file_str, "_normal")) {
			continue;
		}

		// We don't want .ups files
		if (Common::StringContains(file_str, ".ups")) {
			continue;
		}

		// Skip accidental base_spc/base_ab copies
		if (Common::StringContains(file_str, "base_spc")) {
			continue;
		}

		if (Common::StringContains(file_str, "base_ab")) {
			continue;
		}
		
		// Platform-specific filtering
		if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
			// Switch/Unity
			// If it's not a .ab/.assets, continue
			if (!Common::StringContains(file_str, ".ab") && !Common::StringContains(file_str, ".assets")) {
				continue;
			}
		}
		else {
			// Xbox|PC
			// If it's not a .spc, continue
			if (!Common::StringContains(file_str, ".spc") && !Common::StringContains(file_str, ".SPC")) {
				continue;
			}
		}

		std::string const input_format = j.path().extension().string();

		std::string const& modified = file_str;
		std::string const& base = modified.substr(0, modified.length() - input_format.length()) + "_normal" + input_format;

		std::string const& outname = modified.substr(0, modified.length() - input_format.length()) + "_patch" + ".ups";

#ifdef _WIN32
		std::string const& command = "\"" + ups.string() + "\" diff --base \"" + base + "\" --modified \"" + modified + "\" --output " + outname;
#else
		std::string const& command = ups.string() + " diff --base \"" + base + "\" --modified \"" + modified + "\" --output " + outname;
#endif
		
		if (Common::executeBatch(command.c_str()) != 0) {
			LOG("ERROR: UPS patch creation failed for " + modified, HERE, "Detective");
		}
	}

	LOG("Deleting leftover files...", HERE, "Detective");

	// Delete all files in Distribute-GFX which aren't .ups files
	std::vector<std::string> todelete{};
	for (auto const& file : std::filesystem::recursive_directory_iterator(distr)) {
		if (Common::StringContains(file.path().string(), ".ups")) {
			continue;
		}
		if (file.is_directory()) {
			continue;
		}
		todelete.push_back(file.path().string());
	}

	for (auto const& j : todelete) {
		std::filesystem::remove(j);
	}

	LOG("Patch files generated!", HERE, "Detective");
}