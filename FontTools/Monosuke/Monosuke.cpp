// Team DAIX, 2026
// MONOSUKE — FONTTOOLS UPS PATCH GENERATOR

// The majority of this code was written between 2020 and 2022

// This tool's purpose is:
// Monosuke produces UPS patches for all modified font SPC archives. It takes
// the filtered SPCs produced by Monokid, pairs each modified SPC with its
// corresponding “normal” (unmodified) base SPC, and invokes the UPS patcher to
// generate .ups patch files.
//
// It performs:
//   • Copying base_fonts_copy into Distribute-Font (base SPCs)
//   • Renaming base SPCs to *_normal.spc for patch pairing
//   • Copying modified SPCs from ModifiedFiles-Font into Distribute-Font
//   • Running the UPS patcher (ups.exe / ups) to generate .ups patches
//   • Deleting all non-.ups files from Distribute-Font
//
// Monosuke is the final stage of FontTools. It converts modified SPC archives
// into distributable UPS patches, ensuring that only differences are shipped
// and that no copyrighted SPC data is redistributed directly.


#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <fstream>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../Common/Common.h"

int main(int argc, char* argv[]) {

#ifdef _WIN32
	std::string ext = ".exe";
#else
	std::string ext = "";
#endif

	std::string const repo = EncryptString("DGRV3-Font");
	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const spcloc = current_dir / EncryptString("ModifiedFiles-Font");
	std::filesystem::path const distr = current_dir / EncryptString("Distribute-Font");
	std::filesystem::path const dgrv3 = current_dir / repo;
	std::filesystem::path const bases = dgrv3 / EncryptString("base_spc");
	std::filesystem::path const ups = bases / EncryptString(("ups" + ext));
	std::filesystem::path const where = bases;
	std::filesystem::path const copydir = where / EncryptString("base_fonts_copy");

	// UPS patcher must exist. Monosuke uses it to generate .ups patches
	// by comparing base SPCs with modified SPCs.

	if (!std::filesystem::exists(ups)) {
		LOG("ERROR: UPS patcher not found!", HERE, "Monosuke");
		return -1;
	}

	if (!std::filesystem::exists(where)) {
		LOG("ERROR: base_spc folder not found!", HERE, "Monosuke");
		return -1;
	}

	// Extract base SPCs in the distribution folder
	if (!std::filesystem::exists(distr)) {
		std::filesystem::create_directory(distr);
	}

	// Copy base_fonts_copy into Distribute-Font.
	// These SPCs represent the “normal” (unmodified) versions used as patch bases.

	std::filesystem::copy(copydir, distr, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

	std::vector<std::string> filelist{};
	std::vector<std::string> modifiedfilename{};

	// For every file in Distribute-Font
	for (auto const& file : std::filesystem::recursive_directory_iterator(distr)) {
		std::string const& oldname = file.path().string();

		if (file.is_directory()) {
			continue;
		}

		// If "normal" is not found
		if (!Common::StringContains(oldname, "normal")) {

			// Add (oldname)_normal.spc to a vector
			filelist.push_back(oldname);
		}
	}

	// Rename base SPCs to *_normal.spc.
	// UPS patcher requires base and modified files to follow a strict naming pattern:
	//   base:     something_normal.spc
	//   modified: something.spc

	for (auto const& j : filelist) {
		if (!Common::StringContains(j, "normal")) {
			std::filesystem::path const p(j);
			// Rename to (oldname)_normal.spc
			std::string const& newname = (j.substr(0, j.length() - p.extension().string().length())) + "_normal" + p.extension().string();
			std::filesystem::rename(j, newname);
		}
	}

	// Add the .spc files from ModifiedFiles-Font to a vector
	for (auto const& file : std::filesystem::recursive_directory_iterator(spcloc)) {
		if (file.is_directory()) {
			continue;
		}
		modifiedfilename.push_back(file.path().string());
	}

	// For every modified SPC
	// Copy modified SPCs from ModifiedFiles-Font into Distribute-Font.
	// These will be paired with *_normal.spc to generate UPS patches.

	for (auto const& j : modifiedfilename) {
		// Copy from ModifiedFiles-Font to Distribute-Font
		// (without changing names)

		std::string const tofind = "ModifiedFiles-Font";
		std::string rename_later = std::filesystem::path(j).parent_path().string();
		std::size_t const find_modified = rename_later.find(tofind);
		if (find_modified == std::string::npos) {
			LOG("ERROR: Couldn't find " + tofind, HERE, "Monosuke");
			return -1;
		}
		rename_later = rename_later.substr(find_modified);
		rename_later = rename_later.substr(tofind.length() + 1);

		std::string const filename = std::filesystem::path(j).filename().string();

		std::string const before = (spcloc / rename_later / filename).string();
		std::string const after = (distr / rename_later / filename).string();

		std::filesystem::copy(before, after);
	}

	LOG("Generating UPS patches...", HERE, "Monosuke");

	// Iterate through Distribute-GFX
	// For each modified SPC:
	//   • Identify its corresponding *_normal.spc
	//   • Run UPS patcher to generate something_patch.ups
	// Only modified SPCs produce patches.

	for (auto const& j : std::filesystem::recursive_directory_iterator(distr)) {

		std::string const file_str = j.path().string();

		// We don't want file_str to contain "normal"
		// (we want the "edited" spc files)
		if (Common::StringContains(file_str, "normal")) {
			continue;
		}

		// We don't want .ups files, only .spc
		if (Common::StringContains(file_str, ".ups")) {
			continue;
		}

		// If somehow base_spc was copied, ignore it
		if (Common::StringContains(file_str, "base_spc")) {
			continue;
		}

		// If it's not a .spc, continue
		if (!Common::StringContains(file_str, ".spc") && !Common::StringContains(file_str, ".SPC")) {
			continue;
		}

		std::string const& modified = file_str;
		std::string const& base_format = j.path().extension().string();
		std::string const& base = modified.substr(0, modified.length() - base_format.length()) + "_normal" + base_format;

		if (!std::filesystem::exists(base)) {
			continue;
		}

		if (!std::filesystem::exists(modified)) {
			continue;
		}

		std::string const& out_format = ".ups";
		std::string const& outname = (modified.substr(0, modified.length() - out_format.length())) + "_patch" + out_format;
#ifdef _WIN32
		std::string const& command = "\"" + ups.string() + "\" diff --base \"" + base + "\" --modified \"" + modified + "\" --output " + outname;
#else
		std::string const& command = ups.string() + " diff --base \"" + base + "\" --modified \"" + modified + "\" --output " + outname;
#endif
		Common::executeBatch(command.c_str());
	}

	LOG("Deleting leftover files...", HERE, "Monosuke");

	// Delete all files in Distribute-Font which aren't .ups files
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
		LOG("Deleting: " + j, HERE, "Monosuke");
	}

	LOG("Patch files generated!", HERE, "Monosuke");
}