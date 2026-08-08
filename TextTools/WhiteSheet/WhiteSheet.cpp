// Team DAIX, 2026
// WHITESHEET — UPS Patch Generator
//
// The majority of this code was written between 2020 and 2022
//
// This tool's purpose is to:
// 1. Prepare SPC files for UPS patch generation (stage 1)
//      - Rename original SPCs to *_normal.spc
//      - Copy modified SPCs into Distribute/
//      - Copy i18n files for Switch builds
// 2. Generate UPS patches (stage 2)
//      - Compare *_normal.spc (base) with modified SPCs
//      - Produce *_patch.ups files using the UPS diff tool
// 3. Handle special trial SPCs (hanron, nonstop, panic, kokoronpa, nico)
//      - Generate English → French UPS patches for better debate text positioning
// 4. Clean up invalid UPS files and leftover SPCs
//
// WhiteSheet is the final packaging stage of the DAIX pipeline:
// it produces UPS patches for distribution, mod releases, or patch managers.

#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <thread>
#include <map>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../Common/Common.h"
#include "../Common/Config.h"
#include <set>

// Remove quotes (why?)
// Remove leading/trailing quotes from a string.
// Used to sanitize paths before passing them to external tools.

inline auto strip_quotes = [](std::string s) -> std::string {
	if (!s.empty() && s.front() == '"' && s.back() == '"' && s.size() >= 2) {
		s = s.substr(1, s.size() - 2);
	}
	while (s.length() > 1 && s.starts_with("\"")) {
		s = s.substr(1);
	}
	while (s.length() > 1 && s.ends_with("\"")) {
		s.pop_back();
	}
	return s;
};

// Wrap in quotes
// Wrap a string in quotes exactly once.
// Required for safe command-line arguments.


inline auto quote_wrap = [](std::string s) -> std::string {
	if (!s.empty() && s.front() == '"' && s.back() == '"') {
		s = s.substr(1, s.size() - 2); // remove existing quotes
	}
	return '"' + s + '"';
};

inline bool CreateUPSPatch(
	std::filesystem::path const& ups,
	std::filesystem::path const& base, std::filesystem::path const& modified, std::filesystem::path const& outname);


int main(int argc, char* argv[]) {

	// Check CLI arguments to determine whether to run stage 1 or stage 2.
	// "part2" triggers UPS patch generation instead of file preparation.

	bool part2 = false;

	if (argc > 1 && argv[1] && strcmp(argv[1], EncryptString("part2")) == 0) {
		part2 = true;
	}
	if (argc > 2 && argv[2] && strcmp(argv[2], EncryptString("part2")) == 0) {
		part2 = true;
	}

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const configfile = current_dir / "TextConfig.config";

	LOG("Reading config", HERE, "WhiteSheet");
	Configuration::ReadConfig(configfile);
	Configuration::ViewDebugCurrentConfig();
	LOG("Reading config", HERE, "WhiteSheet");

#ifdef _WIN32
	std::string ext = ".exe";
#else
	std::string ext = "";
#endif

	std::filesystem::path const spcloc = current_dir / EncryptString("ModifiedFiles");
	std::filesystem::path const distr = current_dir / EncryptString("Distribute");
	std::filesystem::path const dgrv3 = current_dir / EncryptString("DGRV3");
	std::filesystem::path const bases = dgrv3 / EncryptString("base_spc");
	std::filesystem::path const where = bases;
	std::filesystem::path const ups = bases / EncryptString(("ups" + ext));

	// spcloc = ModifiedFiles/
	// distr  = Distribute/
	// bases  = DGRV3/base_spc/
	// ups    = UPS patcher executable

	// The UPS binary must exist; it is used to generate diff patches.
	if (!std::filesystem::exists(ups)) {
		LOG("ERROR: UPS patcher not found!", HERE, "WhiteSheet");
		return -1;
	}

	if (!std::filesystem::exists(distr)) {
		std::filesystem::create_directory(current_dir / EncryptString("Distribute"));
	}

	// Part-1-only
	if (!part2) {
		// Do this the first time it is run (no --part2 argument)

		// Stage 1: Prepare SPC files for UPS patching.
		// - Rename originals to *_normal.spc
		// - Copy modified SPCs into Distribute/
		// - Copy i18n files for Switch builds

		std::vector<std::pair<std::string, std::string>> filelist{};

		std::vector<std::string> modifiedfilename{};

		// Iterate through the Distribute folder (the ones with the .ups files)
		// Scan existing SPC files in Distribute and prepare them by renaming
		// original files to *_normal.spc for later patch generation.
		// (WHY ARE THEY IN DISTRIBUTE IN THE FIRST PLACE????)

		// Stage 1: Scan the Distribute folder for SPC files.
		// Each original SPC is renamed to *_normal.spc so modified versions can be diffed later.

		for (auto const& file : std::filesystem::recursive_directory_iterator(distr)) {
			if (file.path().string().contains("_extracted")) {
				continue;
			}
			// No directories, right???
			/*
			if (file.is_directory()) {
				continue;
			}
			*/
			auto j = file.path().string();

			bool const is_trial_file =
				(j.contains(".spc") || j.contains(".SPC")) &&
				(j.contains("hanron_") || j.contains("kokoronpa_08_") || j.contains("nico_06_") || j.contains("nonstop_") || j.contains("panic_"));

			// Why are we... ignoring trial files? I don't know 
			// Perhaps because they're too small, and we have issues checking whether or not the UPS patching failed
			// so we resort to checking the UPS file's size?

			// Trial debate SPCs require special handling: the French versions have better text
			// positioning/layout, so patches must be generated by diffing English → French.
			// These files do not appear in different_part2.txt and cannot be auto-detected,
			// so they are hardcoded here.

			if (is_trial_file) {
				continue;
			}
			std::string output_format = file.path().extension().string();
			std::transform(output_format.begin(), output_format.end(), output_format.begin(),
				[](unsigned char c) { return std::tolower(c); });
			std::string const& oldname = file.path().string();
			std::string const& newname = file.path().string().substr(0, file.path().string().length() - output_format.length()) + "_normal" + output_format;
			filelist.push_back(std::make_pair(oldname, newname));
		}

		// Iterate through the list of "good" SPC files (the ones added to the list in the loop above)
		for (auto const& j : filelist) {
			if (!Common::StringContains(j.first, "normal")) {
				try {
					LOG("Renaming " + j.first + " to " + j.second, HERE, "WhiteSheet");
					std::filesystem::rename(j.first, j.second);
				}
				catch (std::filesystem::filesystem_error ec) {
					LOG("Error while renaming " + j.first + " to " + j.second + ": " + ec.what(), HERE, "WhiteSheet");
				}
			}
		}

		// SPCLOC: ModifiedFiles folder
		// Collect all modified SPC filenames that should overwrite or patch originals.

		for (auto const& file : std::filesystem::recursive_directory_iterator(spcloc)) {

			// Uhhh, ignore stuff from Harmony-Tools?
			if (file.path().string().contains("_extracted")) {
				continue;
			}

			auto j = file.path().string();

			// Again, we're ignoring the trial files

			// Trial debate SPCs require special handling: the French versions have better text
			// positioning/layout, so patches must be generated by diffing English → French.
			// These files do not appear in different_part2.txt and cannot be auto-detected,
			// so they are hardcoded here.

			bool const is_trial_file =
				(j.contains(".spc") || j.contains(".SPC")) &&
				(j.contains("hanron_") || j.contains("kokoronpa_08_") || j.contains("nico_06_") || j.contains("nonstop_") || j.contains("panic_"));

			if (is_trial_file) {
				continue;
			}

			/*
			if (file.is_directory()) {
				continue;
			}
			*/

			LOG("Seen modified file: " + file.path().string(), HERE, "WhiteSheet");
			modifiedfilename.push_back(file.path().filename().string());
		}

		LOG("Copying...", HERE, "WhiteSheet");

		if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
			// Unity-based port which uses i18n as well
			// Those i18n files contain text, which is... well, something we want to patch
			std::filesystem::path const i18nfiles = dgrv3 / "i18n";
			if (std::filesystem::exists(i18nfiles)) {
				for (auto const& file : std::filesystem::recursive_directory_iterator(i18nfiles)) {
					try {
						LOG("Copying " + file.path().string() + " to " + ((distr / file.path().filename()).string()), HERE, "WhiteSheet");
						std::filesystem::copy(file, distr / file.path().filename(), std::filesystem::copy_options::recursive);
					}
					catch (const std::filesystem::filesystem_error& e) {
						LOG(std::string("ERROR copying file: ") + e.what(), HERE, "WhiteSheet");
					}
				}
			}
		}

		for (auto const& j : modifiedfilename) {
			if (std::filesystem::exists(spcloc / j)) {
				try {
					LOG("Copying " + (spcloc / j).string() + " to " + (distr / j).string(), HERE, "WhiteSheet");
					std::filesystem::copy(spcloc / j, distr / j, std::filesystem::copy_options::recursive);
				}
				catch (const std::filesystem::filesystem_error& e) {
					LOG(std::string("ERROR copying file: ") + e.what(), HERE, "WhiteSheet");
				}
			}
		}

		Common::WaitExit();

		return EXIT_SUCCESS;
	}

	// Part-2 only

	// Stage 2: Generate UPS patches using base and modified SPC files.
	// Requires different_part2.txt to list which SPCs changed.

	LOG("Doing part-2", HERE, "WhiteSheet");

	// Load list of SPCs that differ between base and modified versions.
	// These will be used to generate UPS patches.

	std::ifstream in("different_part2.txt", std::ios::in);
	if (!in.good()) {
		LOG("File does not exist: different_part2.txt", HERE, "WhiteSheet");
	}
	std::set<std::string> different_spcs{};
	std::string line{};
	// Read a specific file that contains a list of different SPCs
	while (std::getline(in, line)) {
		std::filesystem::path const p = std::filesystem::path(line);
		std::error_code ec;
		if (std::filesystem::is_directory(p, ec)) {
			continue;
		}
		different_spcs.insert(p.string());
	}
	in.close();

	if (different_spcs.empty()) {
		LOG("ERROR: No different SPCs were found! (Exiting)", HERE, "WhiteSheet");
		return EXIT_FAILURE;
	}
	else {
		LOG(std::to_string(different_spcs.size()) + " different SPCs!", HERE, "WhiteSheet");

		for (auto const& spc : different_spcs) {
			LOG("\t" + spc, HERE, "WhiteSheet");
		}
	}

	if (Configuration::ConfigMap["UseSwitchConfiguration"]) {

		for (auto const& file : std::filesystem::recursive_directory_iterator(distr)) {

			if (file.is_directory()) {
				continue;
			}

			std::string const file_name = file.path().filename().string();
			if (Common::StringContains(file_name, "_normal")) {
				continue;
			}

			// Limit to .pb files?
			// Switch builds include PB files (binary text).
			// These must also be patched, so they are added to different_spcs.

			if (file.path().extension().string() != ".pb") {
				continue;
			}

			different_spcs.insert(file.path().string());
		}
	}

	// For each modified SPC:
	// - Compute base file (xxx_normal.spc)
	// - Compute output UPS filename (xxx_patch.ups)
	// - Call CreateUPSPatch()


	for (auto const& j : different_spcs) {

		// Directories were already filtered before

		// Some SPCs may use uppercase extensions (.SPC).
		// Check for both lowercase and uppercase base filenames.

		std::string input_format = std::filesystem::path(j).extension().string();
		std::transform(input_format.begin(), input_format.end(), input_format.begin(),
			[](unsigned char c) { return std::tolower(c); });

		if (input_format.contains(".ups") || input_format.contains("_patch") || input_format.contains("_sha")) {
			continue;
		}

		std::string const output_format = ".ups";

		auto const& modified = std::filesystem::path(strip_quotes(j));
		auto mod_ext = modified.extension().string();

		auto const& outname = std::filesystem::path(strip_quotes(
			strip_quotes(modified.string().substr(0, modified.string().length() - modified.extension().string().length())) + "_patch" + output_format
		));
		auto const& base = std::filesystem::path(strip_quotes(
			strip_quotes(modified.string().substr(0, modified.string().length() - modified.extension().string().length())) + "_normal" + input_format
		));

		std::transform(input_format.begin(), input_format.end(), input_format.begin(),
			[](unsigned char c) { return std::toupper(c); });

		if (!std::filesystem::exists(base)) {
			LOG("WARNING: Base does not exist: \"" + base.string() + "\"", HERE, "WhiteSheet");

			auto const& base2 = std::filesystem::path(strip_quotes(
				strip_quotes(modified.string().substr(modified.string().length() - modified.extension().string().length())) + "_normal" + input_format
			));
			if (std::filesystem::exists(base2)) {
				LOG("Base *does* exist with uppercase extension, though", HERE, "WhiteSheet");
			}
		}

		if (!std::filesystem::exists(modified)) {
			LOG("WARNING: Modified does not exist: \"" + modified.string() + "\"", HERE, "WhiteSheet");
		}

		CreateUPSPatch(ups, base, modified, outname);
	}

	// I F*****NG REMEMBER NOW.
	// WE ARE USING THE FRENCH FILES BECAUSE THEY HAVE BETTER TEXT POSITIONING IN THE DEBATES	

	// Generate UPS patches for trial SPCs (English → French).
	// These are special-case files not covered by different_part2.txt.

	// Generate UPS patches for trial SPCs (English → French).
	// These files are not auto-detected and must be hardcoded.

	{
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "hanron_01_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "hanron_01_FR.spc"), std::filesystem::path(distr / "wrd_data" / "hanron_01_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "hanron_02_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "hanron_02_FR.spc"), std::filesystem::path(distr / "wrd_data" / "hanron_02_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "hanron_03_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "hanron_03_FR.spc"), std::filesystem::path(distr / "wrd_data" / "hanron_03_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "hanron_04_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "hanron_04_FR.spc"), std::filesystem::path(distr / "wrd_data" / "hanron_04_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "hanron_05_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "hanron_05_FR.spc"), std::filesystem::path(distr / "wrd_data" / "hanron_05_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "hanron_06_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "hanron_06_FR.spc"), std::filesystem::path(distr / "wrd_data" / "hanron_06_US_patch.ups"));

		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "kokoronpa_08_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "kokoronpa_08_FR.spc"), std::filesystem::path(distr / "wrd_data" / "kokoronpa_08_US_patch.ups"));

		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "nico_06_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "nico_06_FR.spc"), std::filesystem::path(distr / "wrd_data" / "nico_06_US_patch.ups"));

		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "nonstop_01_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "nonstop_01_FR.spc"), std::filesystem::path(distr / "wrd_data" / "nonstop_01_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "nonstop_02_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "nonstop_02_FR.spc"), std::filesystem::path(distr / "wrd_data" / "nonstop_02_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "nonstop_03_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "nonstop_03_FR.spc"), std::filesystem::path(distr / "wrd_data" / "nonstop_03_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "nonstop_04_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "nonstop_04_FR.spc"), std::filesystem::path(distr / "wrd_data" / "nonstop_04_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "nonstop_05_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "nonstop_05_FR.spc"), std::filesystem::path(distr / "wrd_data" / "nonstop_05_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "nonstop_06_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "nonstop_06_FR.spc"), std::filesystem::path(distr / "wrd_data" / "nonstop_06_US_patch.ups"));

		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "panic_01_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "panic_01_FR.spc"), std::filesystem::path(distr / "wrd_data" / "panic_01_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "panic_02_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "panic_02_FR.spc"), std::filesystem::path(distr / "wrd_data" / "panic_02_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "panic_03_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "panic_03_FR.spc"), std::filesystem::path(distr / "wrd_data" / "panic_03_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "panic_04_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "panic_04_FR.spc"), std::filesystem::path(distr / "wrd_data" / "panic_04_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "panic_05_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "panic_05_FR.spc"), std::filesystem::path(distr / "wrd_data" / "panic_05_US_patch.ups"));
		CreateUPSPatch(ups, std::filesystem::path(where / "trial_english_extracted" / "panic_06_US.spc"), std::filesystem::path(where / "trial_french_extracted" / "panic_06_FR.spc"), std::filesystem::path(distr / "wrd_data" / "panic_06_US_patch.ups"));
	}


	std::vector<std::string> todelete{};
	// Delete everything except *valid* UPS
	// Clean up the Distribute folder by removing all non-UPS files
	// or UPS files that are too small to be valid.

	// Remove all non-UPS files and UPS files smaller than 0x20 bytes (likely invalid).

	for (auto const& file : std::filesystem::recursive_directory_iterator(distr)) {

		if (file.is_directory()) {
			continue;
		}

		// If UPS: continue?
		if (Common::StringContains(file.path().string(), ".ups")) {
			// It's an UPS file
			std::stringstream ss{};
			std::ifstream i(file.path().string(), std::ios::in | std::ios::binary);
			if (i.good()) {
				ss << i.rdbuf();
				i.close();
				// Arbitrary 0x20
				if (ss.str().length() > 0x20) {
					// It's a valid UPS file
					continue;
				}
			}
		}
		todelete.push_back(file.path().string());
	}

	for (auto const& j : todelete) {
		try {
			LOG("Deleting file: " + j, HERE, "WhiteSheet");
			std::filesystem::remove(j);
		}
		catch (std::filesystem::filesystem_error ec) {
			LOG("Error when trying to remove file: " + j, HERE, "WhiteSheet");
		}
	}

	LOG("Patch files generated!", HERE, "WhiteSheet");

	// Remove leftover temporary UPS tool executable if present.
	if (std::filesystem::exists(EncryptString("tempfile.exe"))) {
		std::filesystem::remove(EncryptString("tempfile.exe"));
	}

	Common::WaitExit();
}

// Invoke the UPS diff tool to generate a patch:
// ups diff --base <file> --modified <file> --output <file>
// Deletes output if the UPS tool fails.

bool CreateUPSPatch(
	std::filesystem::path const& ups,
	std::filesystem::path const& base, std::filesystem::path const& modified, std::filesystem::path const& outname) {

	if (outname.empty()) {
		LOG("Outname is empty", HERE, "WhiteSheet");
		return false;
	}

	try {
		std::filesystem::create_directories(outname.parent_path());
	}
	catch (std::filesystem::filesystem_error ec) {
		LOG("Error when creating directories: " + std::string{ ec.what() }, HERE, "WhiteSheet");
	}

	// Build the UPS diff command: ups diff --base <file> --modified <file> --output <file>

	std::string const& command =
		quote_wrap(strip_quotes(ups.string())) +
		" diff --base " + quote_wrap(strip_quotes(base.string())) +
		" --modified " + quote_wrap(strip_quotes(modified.string())) +
		" --output " + quote_wrap(strip_quotes(outname.string()));

	if (Common::executeBatch(command.c_str()) != 0) {
		LOG("ERROR: Command failed: " + command, HERE, "WhiteSheet");
		if (std::filesystem::exists(outname)) {
			try {
				// If UPS tool fails, delete the output file to avoid leaving corrupted patches.
				std::filesystem::remove(outname);
			}
			catch (std::filesystem::filesystem_error ec) {
				LOG("Failed at deleting (probably) broken PB: " + outname.string() + " - " + std::string{ ec.what() }, HERE, "WhiteSheet");
			}
		}
		return false;
	}
	return true;
}