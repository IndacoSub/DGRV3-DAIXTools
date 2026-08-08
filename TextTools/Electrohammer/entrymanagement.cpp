// Team DAIX, 2026
// ELECTROHAMMER — Entry Management
//
// The majority of this code was written between 2020 and 2022
//
// This module's purpose is to:
// 1. Scan the DGRV3 folder and identify which .txt files must be compiled
// 2. Filter out irrelevant files (README, LICENSE, vars_bak, SHA files, PB files, etc.)
// 3. Determine the correct .spc archive each file belongs to (chapter1 → chap1_text_US.spc, etc.)
// 4. Use Necronomicon’s output (different_hashes.txt + different_hash_folders.txt) to compile only changed files
// 5. Apply blacklist rules to skip specific files
// 6. Group entries by SPC target for multithreaded compilation
// 7. Produce a final list of Entry objects used by Electrohammer’s compiler
//
// EntryManagement is the “front door” of Electrohammer: it decides *what* gets compiled and *where* it goes.

#include "entrymanagement.h"

#include <iostream>
#include <thread>
#include <regex>
#include <fstream>
#include "../Common/Config.h"

namespace EntryMG {

	// Determines whether a file should be compiled into an SPC archive.
	// Applies folder checks, file-type checks, blacklist checks, and “changed file” logic.
	// If valid, sets file_to_insert to the correct SPC target.

	bool CheckEntry(
		std::filesystem::directory_entry const& file,
		std::string* file_to_insert,
		std::vector<std::uint64_t> const& different_indexes,
		std::vector<std::string> const& different_files,
		std::vector<std::string>& skipped
	) {

		std::string const& file_str = file.path().string();

		// Skip Switch PB files — they are binary and not compiled here.

		if (file.path().extension().string().contains(".pb")) {
			return false;
		}

		std::string const& filename = file.path().filename().string();

		// Figure out which folder this file is in (es. "prologue", "chapter1" etc.)
		// Example: "DGRV3/chapter3/scene_12.txt" → index of "chapter3".

		std::uint64_t const find = GetIndex(file_str);

		// If it isn't part of one of the folders in the 'folders' vector
		if (find == UINT64_MAX) {
			return false;
		}

		// If the "file" is actually a directory (which probably means that file_different is true?)
		// 
		// If the entry is a directory, determine which SPC archive it corresponds to.
		// Directories themselves are not compiled, but they define the SPC target.

		if (file.is_directory()) {

			// Get which is the .spc file that we should insert this file in
			// We can detect that by using a vector and seeing the result index of the function
			// ex. /chapter1/c00_000_001.txt should go in chap1_text_US.spc

			*file_to_insert = GetFileToInsert(find);

			// It's a directory and we did everything
			// we needed to do
			return false;
		}

		// Since the program arrived here, the "file" is *NOT* a directory
		// but a "normal" file

		// If the folder the file is in, is in the list of "changed folders/files", or we just want to compile everything
		// 
		// Decide whether this file should be compiled:
		// - If CheckAllFiles = true → always compile
		// - Otherwise → compile only if both:
		// • its folder index is in different_indexes
		// • its filename is in different_files (unless optimize_for_size = true)

		bool file_different = false;
		// Size or compilation speed?
		constexpr static bool optimize_for_size = false;

		if constexpr (optimize_for_size) {
			file_different = Configuration::ConfigMap["CheckAllFiles"] || std::find(different_indexes.begin(), different_indexes.end(), find) != different_indexes.end();
		}
		else {
			file_different = Configuration::ConfigMap["CheckAllFiles"] ||
				(std::find(different_indexes.begin(), different_indexes.end(), find) != different_indexes.end() &&
					std::find(different_files.begin(), different_files.end(), filename) != different_files.end());
		}

		// If the folder the file is in, is *NOT* in the list of "changed folders", and we *DO NOT* want to compile everything
		if (!file_different) {
			return false;
		}

		// Reject files that are not real script files:
		// - SHA files
		// - line-count files
		// - README/LICENSE
		// - .git metadata
		// - Baked output
		// - vars_bak.txt (variable definition file)


		// If it's not a .txt, continue
		// This was below, before, but this should be more efficient
		if (!Common::StringContains(file_str, ".txt")) {
			return false;
		}

		// We've done some basic checks now, but we also need to check that the file
		// doesn't contain these strings

		if (Common::StringContains(file_str, "_sha")) {
			return false;
		}
		if (Common::StringContains(file_str, "_lines")) {
			return false;
		}
		if (Common::StringContains(file_str, "README")) {
			return false;
		}
		if (Common::StringContains(file_str, "LICENSE")) {
			return false;
		}
		if (Common::StringContains(file_str, ".git")) {
			return false;
		}
		if (Common::StringContains(file_str, "Baked")) {
			return false;
		}

		// It can't be the file with the variables
		if (Common::StringContains(file_str, EncryptString("vars_bak"))) {
			return false;
		}

		// Since the program arrived here, it *IS* a txt
		// so we can continue

		// Try to see if this file is in the blacklist
		// (if the blacklist isn't empty)

		// Skip files matching any blacklist pattern.
		// Blacklist is user-defined and optional.

		if (!blacklist.empty()) {
			// Try to see if this file is in the blacklist
			auto const find_in_blacklist = [&](std::string const& str) -> bool {
#ifdef CPP20
				return std::any_of(blacklist.begin(), blacklist.end(), [str](std::string const& j) -> bool {return Common::StringContains(str, j); });
#else
				for (auto const& j : blacklist) {
					if (Common::StringContains(str, j)) {
						return true;
					}
				}
#endif
				return false;
			};

			// If it *is* in the blacklist
			if (find_in_blacklist(file_str)) {
				// Then add it to the list of files that
				// we won't compile
				skipped.push_back(file_str);
				// Actually skip compiling this file,
				// moving on to the next one
				return false;
			}
		}

		// If the right .spc for this file wasn't found
		// which means that the function returned ""
		// then we can't insert it anywhere...
		// If we can't insert it anywhere,
		// we don't know what to do!
		if (file_to_insert == nullptr || file_to_insert->empty()) {
			// If no SPC target was determined, this file cannot be compiled.
			// Example: unexpected folder or missing mapping.

			return false;
		}

		return true;
	}

	// Group entries by SPC target.
	// Each group corresponds to one SPC archive and will be compiled in parallel.

	void SortEntriesBySPC(std::vector<Entry> const& entries, SplitType& split) {
		for (auto const& en : entries) {
			// Multithread by FileToInsert
			// Finds .spc using std::find and parallelizes work
			// Result in a 30-35% increase in performance

			// Find the index of the SPC file in the global fti list.
			// Example: "chap1_text_US.spc" → index 1.

			auto const it = std::find(fti.begin(), fti.end(), en.SPCFileToInsert);
			if (it == fti.end()) {
				// Couldn't be found...?
				LOG("ERROR: Invalid .spc file", HERE, "Electrohammer");
				return;
			}

			// Validate that the SPC index is within bounds and that the split array is correctly sized.

			std::uint64_t const distance = std::distance(fti.begin(), it);
			if (distance >= fti.size()) {
				// Invalid index...?
				LOG("ERROR: Invalid index", HERE, "Electrohammer");
				return;
			}

			if (distance >= split.size()) {
				// Invalid array...?
				LOG("ERROR: Invalid array (distance: " + std::to_string(distance) + ", array size: " + std::to_string(split.size()) + ")", HERE, "Electrohammer");
				return;
			}

			// Add to an array sorted by .spc
			split[distance].push_back(en);
		}

		if (split.size() > SpcFiles) {
			// Invalid array size...?
			LOG("ERROR: Invalid array size", HERE, "Electrohammer");
			return;
		}
	}

	// Returns the index of the folder containing the file.
	// If no folder matches, returns UINT64_MAX.

	std::uint64_t GetIndex(std::string const& str) {

		// Default the value to UINT64_MAX, which means "not found"
		std::uint64_t find = UINT64_MAX;

		// Counter
		std::uint64_t cont = 0;
		// Iterate through a list of folders like "prologue", "subroutine", "chapter1" etc.
		for (auto const& j : folders) {
			// If the input string contains the name of a folder
			if (Common::StringContains(str, j)) {
				// Then assign the folder's index in the vector to "find"
				find = cont;
			}
			// By incrementing the counter, we also increment the index of the not-yet-found folder in the vector
			cont++;
		}

		// Return the folder's index in the vector
		return find;
	}

	// Returns the SPC filename corresponding to a folder index.
	// Example: index of "chapter1" → "chap1_text_US.spc".

	std::string GetFileToInsert(std::uint64_t const& index) {

		// Index can't be > 13 (fti size) or < 0
		// If it is, return ""
		if (index >= fti.size()) {
			return "";
		}

		std::string const& ret = fti.at(index);

		return ret;
	}

	// Reads different_hash_folders.txt to determine which folders contain changed files.
	// If missing or empty → compile all folders.
	// Used to reduce compilation time by skipping unchanged chapters.

	std::vector<std::uint64_t> CalculateFoldersToCompile(std::filesystem::path const& savepath) {

		std::vector<std::uint64_t> different_indexes{};
		// Find out which parts ("prologue", "chapter1" etc. should be compiled)
		// Check if the file (containing the files with different hashes) exists
		if (std::filesystem::exists(savepath)) {
			if (!Configuration::ConfigMap["CheckAllFiles"]) {

				// Which ones should be compiled, then?
				std::ifstream in((savepath), std::ios::in);

				std::string line{};

				while (std::getline(in, line)) {
					if (line.empty() || line == "\n" || line == "\r") {
						continue;
					}
					// Remove newlines
					line = std::regex_replace(line, std::regex("\n"), "");
					line = std::regex_replace(line, std::regex("\r"), "");
					// Convert string to std::uint64_t
					std::uint64_t const index = std::stoull(line);
					// Is "index" present in "different_indexes"?
					auto const it = std::find(different_indexes.begin(), different_indexes.end(), index);
					if (it == different_indexes.end()) {
						different_indexes.push_back(index);
					}
				}

				in.close();

				std::filesystem::remove(savepath);
			}
		}
		else {
			// If the folder index file is missing, warn the user and compile all folders.
			// This is extremely slow but guarantees correctness.

			LOG("WARNING: Couldn't find the file with the indexes (" + savepath.filename().string() + ")!", HERE, "Electrohammer");
			LOG("Compile every single file, then...? (VERY SLOW)", HERE, "Electrohammer");
			LOG("You have 5 seconds to abort...", HERE, "Electrohammer");
			std::this_thread::sleep_for(std::chrono::seconds(5));
			LOG("Proceeding!", HERE, "Electrohammer");
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (different_indexes.empty()) {
			// Every file from the folders ("prologue", "chapter1" etc.) is going to be compiled
			for (std::uint64_t index = 0; index < EntryMG::folders.size(); index++) {
				different_indexes.push_back(index);
			}
		}

		// Print the list of folders that will actually be compiled.
		LOG("Will actually be compiled:", HERE, "Electrohammer");
		for (auto const& ind : different_indexes) {
			std::string istr = std::string{ EntryMG::folders[ind] };
			LOG("- " + istr, HERE, "Electrohammer");
		}
		LOG("\n", HERE, "Electrohammer");

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		return different_indexes;
	}

	// Reads different_hashes.txt to determine which specific files changed.
	// Returns only filenames (not full paths).

	std::vector<std::string> CalculateFilesToCompile(std::filesystem::path const& different_hashes_file) {

		std::vector<std::string> ret{};
		std::string const file = different_hashes_file.string();
		// Warn if a file listed in different_hashes.txt does not exist in the repo.

		if (!std::filesystem::exists(different_hashes_file)) {
			LOG("WARNING: " + file + " does not exist!", HERE, "Electrohammer");
			return ret;
		}
		std::string temp{};
		std::ifstream infile(file, std::ios::in);
		std::filesystem::path const cur = std::filesystem::current_path();
		while (std::getline(infile, temp)) {
			std::filesystem::path const p(cur / temp);
			if (std::filesystem::exists(p)) {
				ret.push_back(p.filename().string());
			}
			else {
				LOG("WARNING: " + p.filename().string() + " from hashes file does not actually exist!", HERE, "Electrohammer");
			}
		}
		infile.close();
		return ret;
	}

	// Build the final list of Entry objects:
	// - Apply folder and file filters
	// - Determine SPC targets
	// - Record paths and directories
	// This list is passed directly to Electrohammer’s compiler.

	std::vector<Entry> CalculateEntries(std::filesystem::path const& folder_index_file, std::filesystem::path const& different_hashes_file, std::filesystem::path const& dgrv3path, std::filesystem::path const& where, std::filesystem::path const& current_dir, std::vector<std::string>& skipped_files) {

		std::vector<Entry> ret{};

		// Get which folders containing files are different (es. "prologue", "chapter1" etc.)
		std::vector<std::uint64_t> const different_folder_indexes = EntryMG::CalculateFoldersToCompile(folder_index_file);
		std::vector<std::string> const different_files = EntryMG::CalculateFilesToCompile(different_hashes_file);

		std::string file_to_insert = "";

		// Iterate through the DGRV3 folder
		for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path)) {
			// This is basically a giant check
			if (!EntryMG::CheckEntry(file, &file_to_insert, different_folder_indexes, different_files, skipped_files)) {
				continue;
			}

			// Create an entry

			// Create an Entry containing:
			// - Filename
			// - SPC target
			// - WhereTo (output directory)
			// - CurrentDir (tool directory)

			EntryMG::Entry en{};
			en.Filename = file.path().string();
			en.SPCFileToInsert = file_to_insert;
			en.WhereTo = where;
			en.CurrentDir = current_dir;

			// Add it to a vector of entries

			ret.push_back(en);
		}

		return ret;
	}
}