// Team DAIX, 2026
// MARKERSTONE
//
// The majority of this code was written between 2020 and 2022
//
// This tool's purpose is to:
// 1. Count the number of lines in every .txt file in the English branch
// 2. Count the number of lines in the corresponding translated files
// 3. Compare both counts to detect files where the translation added or removed lines
// 4. Save a report listing all files with mismatched line counts
//
// Used to ensure script integrity and prevent accidental line shifts between branches.

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>
#include <algorithm>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../Common/Common.h"

bool PerformChecks(std::filesystem::directory_entry const& file);
// Saves lines(?)
bool SaveLines(std::filesystem::directory_entry const& file);

// Folders containing .txt files
// 
// Ordered list of script folders used to categorize mismatched files.
// The index of each folder is saved when line counts differ.

std::vector<std::string> const folders {

	EncryptString("test"),
	EncryptString("subroutine"),
	EncryptString("prologue"),
	EncryptString("MapObjName"),
	EncryptString("game_resident"),
	EncryptString("gallery"),
	EncryptString("epilogue"),
	EncryptString("chapter6"),
	EncryptString("chapter5"),
	EncryptString("chapter4"),
	EncryptString("chapter3"),
	EncryptString("chapter2"),
	EncryptString("chapter1"),
	EncryptString("ainori"),
};

int main(int argc, char* argv[]) {

	// '--all' mode: treat every folder as mismatched, regardless of actual line counts.

	bool all = false;

	if (argc > 1) {
		if (argv[1] != nullptr) {
			if (strcmp(argv[1], EncryptString("--all")) == 0) {
				all = true;
			}
		}
	}

	const std::string repo = EncryptString("DGRV3");
	const std::string repo_en = EncryptString("DGRV3_EN");

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const dgrv3path = current_dir / repo;
	std::filesystem::path const dgrv3path_en = current_dir / repo_en;

	if (!std::filesystem::exists(dgrv3path)) {
		LOG("ERROR: One of the folders where lines were going to be calculated was not found!", HERE, "MarkerStone");
		return -1;
	}

	if (!std::filesystem::exists(dgrv3path_en)) {
		LOG("ERROR: One of the folders where lines were going to be calculated was not found!", HERE, "MarkerStone");
		return -1;
	}

	std::string const savefile = EncryptString("different_lines.txt");

	if (std::filesystem::exists(current_dir / savefile)) {
		std::filesystem::remove(current_dir / savefile);
	}

	// 'different' stores entries like:
	// { "<translated file path>", { "<translated line count>", "<english line count>" } }
	// Example: { "DGRV3/chapter3/scene_12.txt", { "145", "143" } }

	// 'different_indexes' stores the index of the folder containing a mismatched file.
	// Example: "chapter3" → index 10, "prologue" → index 2.

	std::vector<std::pair<std::string, std::pair<std::string, std::string>>> different{};
	std::vector<std::uint64_t> different_indexes{};

	if (!all) {

		std::vector<std::string> check_en{};
		std::vector<std::string> check_it{};

		LOG("Calculating no. of lines for english branch...", HERE, "MarkerStone");

		std::pair<std::uint64_t, std::uint64_t> cont_files = { 0, 0 };
		// Iterate through the english directory
		for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path_en)) {
			// Save lines from the files from the english directory
			// (cont_files.first is going to be incremented if everything succeeds)

			// SaveLines() creates a companion file "<name>_lines" containing the number of lines.
			// This is used later to compare English vs translated versions.

			if (SaveLines(file)) {
				cont_files.first++;
				check_en.push_back(Common::ShortenFilename(file.path().string(), 2));
			}
		}
		LOG("\n", HERE, "MarkerStone");
		LOG("Done calculating lines!", HERE, "MarkerStone");

		// Iterate through the normal (translated) directory
		for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path)) {
			if (!PerformChecks(file)) {
				continue;
			}
			if (Common::StringContains(file.path().string(), "vars_bak")) {
				continue;
			}
			// Increment the number of files checked in
			// the normal (translated) directory
			cont_files.second++;
			check_it.push_back(Common::ShortenFilename(file.path().string(), 2));
		}

		bool const matches = cont_files.first == cont_files.second;

		// If the numbers don't match

		// If the number of processed files differs between branches,
		// the comparison cannot continue reliably — dump diagnostics and abort.

		if (!matches) {
			LOG("ERROR: The number of files checked is not matching (" + std::to_string(cont_files.first) + " VS " + std::to_string(cont_files.second) + ") !", HERE, "MarkerStone");

			std::ofstream f_en("mstone_check_en.txt", std::ios::out | std::ios::app);
			for (auto const& en_str : check_en) {
				f_en << en_str << std::endl;
			}
			f_en.close();

			std::ofstream f_it("mstone_check_it.txt", std::ios::out | std::ios::app);
			for (auto const& it_str : check_it) {
				f_it << it_str << std::endl;
			}
			f_it.close();

			return -1;
		}
		LOG("The number of files matches (" + std::to_string(cont_files.first) + ") !", HERE, "MarkerStone");

		LOG("\n", HERE, "MarkerStone"),
		LOG("Calculating lines once again for translation, and comparing...", HERE, "MarkerStone");

		// Iterate through the normal (translated)
		// folder once again

		// Second pass: compare each translated file with its English counterpart.
		// Count newlines in the translated file and compare with the saved English count.

		for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path)) {

			if (!PerformChecks(file)) {
				continue;
			}

			// TODO: can this be simplified by... just using while(getline(in, temp))?

			std::stringstream ss1{};
			// Imagine that we had two files
			// ...well, we do have to files,
			// but it'easier to imagine it than
			// to explain it properly
			// We have what we call "File 1"
			// and what we call "File 2"
			// f is "File 1"
			std::ifstream f(file.path().string(), std::ios::in | std::ios::binary);
			if (!f) {
				LOG("ERROR: Cannot open file " + file.path().string(), HERE, "MarkerStone");
				continue;
			}
			// Read ALL the content of "File 1"
			// inside a stringstream (ss1)
			// So now, this stringstream (ss1)
			// contains ALL the text inside the file
			ss1 << f.rdbuf();
			// Declare a new variable called str1
			// str1 contains ALL the content
			// of the stringstream from before (ss1)
			// in a SINGLE string
			auto const str1 = ss1.str();
			// Count how many \n (NOT \\n) are
			// inside this huge string
			// so that we know how many newlines
			// the string has
			auto const count1 = std::count(str1.begin(), str1.end(), '\n');
			// Convert that number (how many newlines)
			// into a string (it's important for later)
			auto const i = std::to_string(count1);
			// Insert the result (which is now a string)
			// into a pair of string
			// in particular, the first one
			// (so now compare.first contains a string
			// which is the number of newlines present
			// inside the file that we are currently iterating on)
			std::pair<std::string, std::string> compare{};
			compare.first = i;

			// Declare a new variable called a
			// This variable has no particular
			// purpose other than storing the
			// full path of the file that we
			// are currently iterating on
			auto a = file.path().string();
			// Since the variable above (a) contains the
			// full path, it also contains the directories
			// and the separators
			// What we can do, is simply to replace the name
			// of the folder of the normal(translated) directory
			// (where the file we are iterating is)
			// with the name of the folder of the
			// english directory, so that we can
			// get the full path of the "equivalent"
			// file from the english directory
			{
				// Rebuild the path by replacing the translated repo folder with the English repo folder.
				// This yields the full path of the corresponding English file.

				std::filesystem::path original_path(file.path());
				std::filesystem::path rebuilt_path;
				bool replaced = false;

				for (auto const& segment : original_path) {
					if (!replaced && segment == repo) {
						rebuilt_path /= repo_en; // append replacement segment
						replaced = true;
					}
					else {
						rebuilt_path /= segment; // append original segment
					}
				}

				a = rebuilt_path.string();
			}
			// Remove the extension from the full path, expecting it
			// to be .txt (".txt" has a length of 4)

			// Convert the English file path to its companion "<file>_lines" file,
			// which contains the precomputed line count.

			std::filesystem::path ap(a);
			ap.replace_extension(EncryptString(".txt_lines"));
			a = ap.string();
			// We are expecting this file to already exist
			// because it should have been created in the
			// SaveLines() function that was called before
			// If english lines file, for some reason, doesn't exist, continue
			// (we ignore this file and move on, basically)
			if (!std::filesystem::exists(std::filesystem::path(a))) {
				continue;
			}
			// If the process is here, it means that the file
			// does indeed exist, great!
			// So, now we want to open that file, which is,
			// I remind you, a file that CONTAINS THE NUMBER OF LINES
			// of the same file (a, the english file) but with a ".txt"
			// extension (NOT ".txt_lines", just ".txt")
			// TL;DR contains the number of lines in the english
			// equivalent of the file we're iterating
			// so that we know how many lines are in the english version
			// of that file, to compare it later and see
			// if the english version has more, less, or the same
			// number of lines as the translated file(s)
			// and we didn't accidentally make new lines
			// or delete existing ones
			// TL;DR 2: Open the english file containing the lines (a)
			std::ifstream f2(a, std::ios::in | std::ios::binary);
			if (!f2) {
				LOG("ERROR: Cannot open file " + a, HERE, "MarkerStone");
				continue;
			}
			// (Oh God no)
			// Declare another stringstream just like before
			std::stringstream ss2{};
			// Once again, read a file into the stringstream (ss2)
			// Which file? The english file (a) of course
			// So, now, ss2 contains ALL the text of the
			// english file
			ss2 << f2.rdbuf();
			// (This should be fine as the file should only contain the number of lines)
			// Convert all the stringstream to a single line
			// (it's a single line anyway)
			// and assign it to compare.second
			compare.second = ss2.str();
			// Remove any eventual newline from compare.second (???)
			compare.second = std::regex_replace(compare.second, std::regex("\r"), "");
			compare.second = std::regex_replace(compare.second, std::regex("\n"), "");

			// Strip stray newline characters from both counts to ensure clean numeric comparison.

			while (compare.first.ends_with('\n') || compare.first.ends_with('\r')) {
				compare.first.pop_back();
			}
			while (compare.first.starts_with('\n') || compare.first.starts_with('\r')) {
				compare.first = compare.first.substr(1);
			}
			while (compare.second.ends_with('\n') || compare.second.ends_with('\r')) {
				compare.second.pop_back();
			}
			while (compare.second.starts_with('\n') || compare.second.starts_with('\r')) {
				compare.second = compare.second.substr(1);
			}

			// Compare the number of files from the normal (translated)
			// directory, and the english directory
			// We also don't want compare.second to be empty
			// So, if it's different
			if (compare.first != compare.second && !compare.second.empty()) {
				// If it is, different, add it to a vector which
				// takes:
				// 1. The full path of the file that we are currently iterating
				// 2. The two strings containing the number of lines in the
				// normal (translated) and english branches
				different.push_back({ file.path().string(), {compare.first, compare.second} });
			}
			// Close the files
			f.close();
			f2.close();
		}

		if (different.empty()) {
			LOG("WARNING: No files with different lines were found!", HERE, "MarkerStone");
		}

		// For each different file
		for (auto const& j : different) {
			// Tell the user information about it such as the name and the number of lines in each branch
			LOG("Different lines: " + Common::ShortenFilename(j.first, 2) + " (" + j.second.first + " VS " + j.second.second + ")", HERE, "MarkerStone");
		}

		// For each different file
		for (auto const& j : different) {

			std::filesystem::path const p(j.first);
			std::string const str = p.parent_path().filename().string();

			// Is the "str" one of the folders that are in the "folders" vector?

			// Determine which script folder the mismatched file belongs to,
			// and record its index for later reporting.

			auto const it = std::find(folders.begin(), folders.end(), str);
			if (it != folders.end()) {
				// Get the position of said folder we found in the array
				std::uint64_t const index = std::distance(folders.begin(), it);
				// This is to determine if "prologue" is different or what
				// I don't really know why but ok, maybe to determine
				// which to compile or not
				if (std::find(different_indexes.begin(), different_indexes.end(), index) == different_indexes.end()) {
					different_indexes.push_back(index);
				}
			}
			else {
				LOG("WARNING: Folder not recognised: " + str, HERE, "MarkerStone");
			}
		}
	}
	else {
		for (std::uint64_t i = 0; i < folders.size(); i++) {
			// Add all folders in "folder" to different_indexes
			different_indexes.push_back(i);
		}
	}

	// Save a human-readable report listing all files with mismatched line counts.

	if (!different_indexes.empty()) {
		LOG("\n", HERE, "MarkerStone");
		LOG("Saving...", HERE, "MarkerStone");

		std::ofstream out(savefile, std::ios::out | std::ios::app);
		for (auto const& j : different) {
			out << Common::ShortenFilename(j.first, 2) << " (" << j.second.first << " VS " << j.second.second << ")" << std::endl;
		}
		out.close();

		LOG("\n", HERE, "MarkerStone");
		LOG("Saving done!", HERE, "MarkerStone");
	}

	Common::WaitExit();
}

bool PerformChecks(std::filesystem::directory_entry const& file) {

	// Is the current file a directory?
	if (file.is_directory()) {
		// If so, return
		return false;
	}
	// If not, it's a file, and we're going to check which file it is

	auto path_str = file.path().string();

	// If it is NOT a .txt, we don't care about it
	if (!Common::StringContains(path_str, ".txt")) {
		return false;
	}

	// Ignore metadata, backup, and non-script files — only compare actual .txt scripts.

	if (Common::StringContains(path_str, "_sha")) {
		return false;
	}
	if (Common::StringContains(path_str, "_lines")) {
		return false;
	}
	if (Common::StringContains(path_str, "README")) {
		return false;
	}
	if (Common::StringContains(path_str, "LICENSE")) {
		return false;
	}
	if (Common::StringContains(path_str, ".git")) {
		return false;
	}
	if (Common::StringContains(path_str, "Baked")) {
		return false;
	}

	return true;
}

bool SaveLines(std::filesystem::directory_entry const& file) {

	if (!PerformChecks(file)) {
		return false;
	}

	// We're using a stringstream
	// as a container for text
	std::stringstream ss{};

	// Open the file we're iterating on, as input
	std::ifstream f(file.path().string(), std::ios::in | std::ios::binary);

	if (!f) {
		LOG("ERROR: Cannot open file " + file.path().string(), HERE, "MarkerStone");
		return false;
	}

	// Insert ALL the text in the input file (f)
	// (which is the file that we are iterating?)
	// into the stringstream
	// (SS <-- All the text from the file)
	ss << f.rdbuf();

	f.close();

	// Create a new file called (old_file_name)_lines
	// Create a companion file storing only the number of lines in the original file.
	std::ofstream o(std::string(file.path().string() + EncryptString("_lines")), std::ios::out);

	// Convert the stringstream to a single string
	// (oh my fucking God WHY)
	auto const str = ss.str();

	// Count how many \n (NOT \\n) the single string has
	// so that we know how many newlines there are
	auto const count = std::count(str.begin(), str.end(), '\n');

	// Insert the number of lines in the output file (o, (old_file_name)_lines)
	o << count << std::endl;

	o.close();

	return true;
}