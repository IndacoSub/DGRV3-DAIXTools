// Team DAIX, 2026
// POOLRULES
//
// The majority of this code was written between 2020 and 2022
//
// This tool's purpose is to:
// 1. Compare every translated .txt file against its English equivalent
// 2. Count translated vs untranslated lines
// 3. Count translated vs untranslated characters
// 4. Compute per‑file, per‑chapter, and whole‑game translation percentages
// 5. Detect line mismatches between TR and EN versions
// 6. Produce detailed reports (percentage_res.txt, charcount_res.txt, detailed_charcount_rex.txt)
//
// How PoolRules differs from MarkerStone:
// - MarkerStone only checks **line counts** (TR vs EN) and reports mismatches.
// - PoolRules performs **full translation analysis**, including:
//      • per‑line translation detection  
//      • per‑character translation ratio  
//      • per‑chapter aggregation  
//      • whole‑game completion percentage  
//      • detailed breakdowns for every file  
// - MarkerStone is a *sanity checker*.
// - PoolRules is a *translation progress analyzer*.


#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <regex>
#include <filesystem>
#include <random>
#include <map>

#include "../Common/Common.h"

// No header file?

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

using RatioType = long double;
using ConvertedType = RatioType;

// Represents translation statistics for a single file.
// Includes:
// - translated vs untranslated lines
// - translated vs untranslated characters
// - total lines
// - per‑file translation percentage
// - per‑file character ratio
// - line‑count consistency check

class FileEntry {
public:
	std::string						f_name{};					// Filename
	std::uint64_t					f_total_tr{};				// Total lines translated
	std::uint64_t					f_total_utr{};				// Total lines untranslated
	std::uint64_t					f_total_characters_tr{};	// Total characters translated
	std::uint64_t					f_total_characters_utr{};	// Total characters untranslated
	std::uint64_t					f_total_lines{};			// Total lines in the file
	RatioType						f_characters_percentage{};	// 100% being 1:1 character ratio (translated/untranslated)
	RatioType						f_translation_percentage{};	// 100% being 1:1 translated lines ratio
	bool							f_lines_match = false;		// Same number of lines?

	FileEntry() = default;
	virtual compl FileEntry() = default;
};

// Aggregates FileEntry data for an entire chapter/folder.
// Computes chapter‑level totals and percentages.

class ChapterEntry {
public:
	std::vector<FileEntry>			F_Entries;					// Vector representing each file in a chapter
	std::uint64_t					c_total_tr{};				// Total files translated in the whole chapter
	std::uint64_t					c_total_utr{};				// Total files untranslated in the whole chapter
	std::uint64_t					c_total_lines{};			// Total lines in the whole chapter
	std::uint64_t					c_total_characters_tr{};	// Total characters translated in the whole chapter
	std::uint64_t					c_total_characters_utr{};	// Total characters untranslated in the whole chapter
	RatioType						c_characters_percentage{};	// 100% being 1:1 character ratio (translated/untranslated)
	RatioType						c_translation_percentage{};	// 100% being 1:1 translated lines ratio
	bool							c_lines_match = true;		// Same number of lines?

	ChapterEntry() = default;
	virtual compl ChapterEntry() = default;
};

// Aggregates ChapterEntry data for the entire game.
// Computes global translation progress and character ratios.

class GameEntry {
public:
	std::array<ChapterEntry, 14>	C_Entries;					// Array representing all chapters; 14: number of chapters/folders
	std::uint64_t					g_total_tr{};				// Total files translated in the whole game
	std::uint64_t					g_total_utr{};				// Total files untranslated in the whole game
	std::uint64_t					g_total_lines{};			// Total lines in the whole game
	std::uint64_t					g_total_characters_tr{};	// Total characters translated in the whole game
	std::uint64_t					g_total_characters_utr{};	// Total characters untranslated in the whole game
	RatioType						g_characters_percentage{};	// 100% being 1:1 character ratio (translated/untranslated)
	RatioType						g_translation_percentage{};	// 100% being 1:1 translated lines ratio
	bool							g_lines_match = true;		// Same number of lines?

	GameEntry() = default;
	virtual compl GameEntry() = default;
};

bool CalculatePercentages(std::filesystem::path const& translated, std::filesystem::path const& untranslated, std::string const& repo, std::string const& repo_en);
std::string ReplaceWithEnglish(std::string const& str, std::string const& repo, std::string const& repo_en);

int main() {

	const std::string repo = EncryptString("DGRV3");
	const std::string repo_en = EncryptString("DGRV3_EN");

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const dgrv3path = current_dir / repo;
	std::filesystem::path const enpath = current_dir / repo_en;

	if (!std::filesystem::exists(dgrv3path)) {
		LOG("Path does not exist: " + dgrv3path.string() + ", aborting!", HERE, "PoolRules");
		return EXIT_FAILURE;
	}

	// Pretty self-explainatory
	bool const res = CalculatePercentages(dgrv3path, enpath, repo, repo_en);
	if (!res) {
		LOG("Different lines or some error! Couldn't calculate percentage(s).", HERE, "PoolRules");
	}
	else {
		LOG("Done!", HERE, "PoolRules");
	}

	Common::WaitExit();
}

bool CalculatePercentages(std::filesystem::path const& translated, std::filesystem::path const& untranslated, std::string const& repo, std::string const& repo_en) {

	// Chapters/folders
	// Ordered list of all script folders in DGRV3.
	// Used to iterate chapters in a fixed, predictable order.

	std::array<std::string, 14> static const possible_folders{
		"ainori",
		"chapter1",
		"chapter2",
		"chapter3",
		"chapter4",
		"chapter5",
		"chapter6",
		"epilogue",
		"gallery",
		"game_resident",
		"MapObjName",
		"prologue",
		"subroutine",
		"test"
	};

	// Specific files used to verify line‑matching reliability.
	// PoolRules prints detailed line/character dumps for these.

	std::vector<std::string> test_files{};
	// Expected 125
	// WTF is this, test cases?
	test_files.push_back((translated / "subroutine").string());
	test_files.push_back((translated / "prologue" / "c00_999_002.txt").string());
	test_files.push_back((untranslated / "prologue" / "c00_999_002.txt").string());
	test_files.push_back((translated / "game_resident" / "A-MapListNameAscii.txt").string());
	test_files.push_back((untranslated / "game_resident" / "A-MapListNameAscii.txt").string());
	test_files.push_back((translated / "game_resident" / "A-MapName.txt").string());
	test_files.push_back((untranslated / "game_resident" / "A-MapName.txt").string());
	test_files.push_back((translated / "game_resident" / "D-Menu.txt").string());
	test_files.push_back((untranslated / "game_resident" / "D-Menu.txt").string());
	test_files.push_back((translated / "game_resident" / "J-TutorialButton.txt").string());
	test_files.push_back((untranslated / "game_resident" / "J-TutorialButton.txt").string());
	test_files.push_back((translated / "game_resident" / "O-Event.txt").string());
	test_files.push_back((untranslated / "game_resident" / "O-Event.txt").string());
	test_files.push_back((translated / "game_resident" / "S-SaveLoad.txt").string());
	test_files.push_back((untranslated / "game_resident" / "S-SaveLoad.txt").string());
	test_files.push_back((translated / "MapObjName" / "ID132_lab_hoshi_text.txt").string());
	test_files.push_back((untranslated / "MapObjName" / "ID132_lab_hoshi_text.txt").string());

	// Maps filename → whether TR and EN line counts match.
	// Used later in file_line_test.txt.

	std::map<std::string, bool> linemap{};

	// (Current) game entry
	GameEntry my_game_entry{};
	// Chapters done currently
	std::uint64_t cont_chapters{};
	// Iterate through folders (array above)
	// Iterate through each chapter folder and compute translation statistics.

	for (auto const& folder : possible_folders) {
		// Current chapter/folder entry
		ChapterEntry my_chapter_entry{};
		// Open each file in the folder individually, recursively
		for (auto const& file : std::filesystem::recursive_directory_iterator(translated / folder)) {
			// If it's a directory (it shouldn't be), continue
			if (file.is_directory()) {
				continue;
			}

			// We only care about .txt files
			if (file.path().extension() != ".txt") {
				continue;
			}

			// Read translated file line-by-line.
			// Ignore brackets and empty newline-only lines.
			// Count characters and total lines.


			// Current file entry
			FileEntry my_file_entry{};
			std::string const& file_str = file.path().string();
			// Open the file
			std::ifstream my_input_file(file_str, std::ios::in | std::ios::binary);
			std::string temp{};

			// Get the filename of the current file
			my_file_entry.f_name = file.path().filename().string();

			// Vector containing ALL the strings in the file
			std::vector<std::string> definitely_optimized_vec{};
			// Read every single string in the file
			while (std::getline(my_input_file, temp)) {
				if (!temp.empty() && temp.front() == '{' || temp.front() == '}') {
					// Ignore brackets
					continue;
				}
				if (!temp.empty() && temp.front() == '\n' || temp.front() == '\r') {
					continue;
				}
				temp = std::regex_replace(temp, std::regex("\r"), "");
				temp = std::regex_replace(temp, std::regex("\n"), "");
				// Add the string to the file
				definitely_optimized_vec.push_back(temp);
				// Increase the number of translated characters in the file
				// by temp.length() (string length)
				my_file_entry.f_total_characters_tr += temp.length();
				// Increase the number of lines in the file
				my_file_entry.f_total_lines++;
			}

			// Close the file
			my_input_file.close();

			temp.clear();

			// Now do the same with the english version
			// Read the English equivalent file.
			// Compare each EN line with the corresponding TR line:
			// - If identical and >3 chars → untranslated
			// - Otherwise → translated
			// Count EN characters for ratio calculations.

			// Cont is an int we use to access the other vector
			std::uint64_t cont = 0;
			// Get the exact same file but from the english repo (the "english equivalent")
			std::string const english_ver = ReplaceWithEnglish(file_str, repo, repo_en);
			// Open the english equivalent
			std::ifstream my_comparison_file(english_ver, std::ios::in | std::ios::binary);
			// Read every single string in the file
			while (std::getline(my_comparison_file, temp)) {
				if (!temp.empty() && temp.front() == '{' || temp.front() == '}') {
					// Ignore brackets
					continue;
				}
				if (!temp.empty() && temp.front() == '\n' || temp.front() == '\r') {
					continue;
				}
				temp = std::regex_replace(temp, std::regex("\r"), "");
				temp = std::regex_replace(temp, std::regex("\n"), "");

				if (cont >= definitely_optimized_vec.size()) {
					LOG(my_file_entry.f_name + " has more lines in ENG than TR! (otherwise may have crashed)", HERE, "PoolRules");
					break; // or handle mismatch as needed
				}

				// Is this line the same as the one in the translation?
				bool const is_the_same = definitely_optimized_vec[cont] == temp;
				// Is this line kinda short (3 characters or less)?
				bool const short_original = temp.length() <= 3;
				// Same line but NOT short?
				if (is_the_same && !short_original) {
					// Then you didn't translate it
					my_file_entry.f_total_utr++;
				}
				else {
					// Congratulations, you translated the line
					my_file_entry.f_total_tr++;
				}
				// Increase the number of untranslated characters by temp.length()
				// aka the length of the string
				my_file_entry.f_total_characters_utr += temp.length();

				// Increase the int to access the vector
				cont++;
			}

			// Close the file
			my_comparison_file.close();

			// Do the lines match? (untranslated lines + translated lines) should be the total of the lines
			// Just to be sure, let's also check if "cont" is the same

			// Verify that TR and EN have the same number of lines.
			// If mismatched, report it.


			my_file_entry.f_lines_match = (my_file_entry.f_total_tr + my_file_entry.f_total_utr) == my_file_entry.f_total_lines
				&& my_file_entry.f_total_lines == cont;

			// If there's a mismatch, tell it to the user
			if (!my_file_entry.f_lines_match) {
				LOG(my_file_entry.f_name + " has different lines (" + std::to_string(cont) + " - ENG VS " + std::to_string(my_file_entry.f_total_lines) + " - TR) !", HERE, "PoolRules");
			}

			auto const if_it = std::find_if(test_files.begin(), test_files.end(), [=](std::string const& s) -> bool {
				return std::filesystem::path(s).filename() == file.path().filename();
			});

			if (if_it != test_files.end()) {
				linemap[file.path().filename().string()] = my_file_entry.f_lines_match;
			}

			if (my_file_entry.f_total_characters_tr > 0) {
				// Calculate the characters percentage
				if (my_file_entry.f_total_characters_utr == 0 || my_file_entry.f_total_characters_tr == my_file_entry.f_total_characters_utr) {
					my_file_entry.f_characters_percentage = 100.0;
				}
				else {
					my_file_entry.f_characters_percentage = 
						static_cast<RatioType>((static_cast<RatioType>(my_file_entry.f_total_characters_tr)) / (static_cast<RatioType>(my_file_entry.f_total_characters_utr)));
					my_file_entry.f_characters_percentage *= 100.0;
				}
			}

			if (my_file_entry.f_total_tr > 0) {
				// Calculate the translation percentage
				if (my_file_entry.f_total_utr == 0) {
					my_file_entry.f_translation_percentage = 100.0;
				}
				else {
					my_file_entry.f_translation_percentage =
						static_cast<RatioType>(my_file_entry.f_total_tr) /
						static_cast<RatioType>(my_file_entry.f_total_utr + my_file_entry.f_total_tr) * 100.0;
				}
			}

			// Add this file to the entries of the chapter
			my_chapter_entry.F_Entries.push_back(my_file_entry);
		}

		if (my_chapter_entry.F_Entries.size() == 0) {
			// Should be impossible, as long as it's the right directory
			LOG("No file entries found in chapter: " + folder, HERE, "PoolRules");
			LOG("Aborting.", HERE, "PoolRules");
			return false;
		}

		// Accumulate lines and stuff
		for (auto const& j : my_chapter_entry.F_Entries) {
			my_chapter_entry.c_total_tr += j.f_total_tr;
			my_chapter_entry.c_total_utr += j.f_total_utr;
			my_chapter_entry.c_total_characters_tr += j.f_total_characters_tr;
			my_chapter_entry.c_total_characters_utr += j.f_total_characters_utr;
			my_chapter_entry.c_total_lines += j.f_total_lines;
			my_chapter_entry.c_lines_match &= j.f_lines_match;
		}

		// Calculate the characters percentage
		if (my_chapter_entry.c_total_characters_tr > 0) {
			if (my_chapter_entry.c_total_characters_utr == 0 || my_chapter_entry.c_total_characters_tr == my_chapter_entry.c_total_characters_utr) {
				my_chapter_entry.c_characters_percentage = 100.0;
			}
			else {
				my_chapter_entry.c_characters_percentage = static_cast<RatioType>(static_cast<RatioType>(my_chapter_entry.c_total_characters_tr) / static_cast<RatioType>(my_chapter_entry.c_total_characters_utr));
				my_chapter_entry.c_characters_percentage *= 100.0;
			}
		}

		if (my_chapter_entry.c_total_tr > 0) {
			if (my_chapter_entry.c_total_utr == 0 || my_chapter_entry.c_total_tr == my_chapter_entry.c_total_utr) {
				my_chapter_entry.c_translation_percentage = 100.0;
			}
			else {
				// Calculate the translation percentage
				my_chapter_entry.c_translation_percentage = static_cast<RatioType>(static_cast<RatioType>(my_chapter_entry.c_total_tr) / static_cast<RatioType>(my_chapter_entry.c_total_tr + my_chapter_entry.c_total_utr));
				my_chapter_entry.c_translation_percentage *= 100.0;
			}
		}

		if (!my_chapter_entry.c_lines_match) {
			LOG(folder + " has different lines!", HERE, "PoolRules");
		}

		my_game_entry.C_Entries[cont_chapters] = my_chapter_entry;
		cont_chapters++;
	}

	if (my_game_entry.C_Entries.size() == 0) {
		// Should be impossible, as long as it's the right directory
		LOG("ERROR: No chapter entries found in game...", HERE, "PoolRules");
		LOG("Aborting.", HERE, "PoolRules");
		return false;
	}

	// For the whole game, accumulate and stuff
	for (auto const& j : my_game_entry.C_Entries) {
		my_game_entry.g_total_tr += j.c_total_tr;
		my_game_entry.g_total_utr += j.c_total_utr;
		my_game_entry.g_total_characters_tr += j.c_total_characters_tr;
		my_game_entry.g_total_characters_utr += j.c_total_characters_utr;
		my_game_entry.g_total_lines += j.c_total_lines;
		my_game_entry.g_lines_match &= j.c_lines_match;
	}

	if (my_game_entry.g_total_characters_tr > 0) {
		// Calculate the characters percentage
		if (my_game_entry.g_total_characters_utr == 0 || my_game_entry.g_total_characters_tr == my_game_entry.g_total_characters_utr) {
			my_game_entry.g_characters_percentage = 100.0;
		}
		else {
			my_game_entry.g_characters_percentage = static_cast<RatioType>(static_cast<RatioType>(my_game_entry.g_total_characters_tr) / static_cast<RatioType>(my_game_entry.g_total_characters_utr));
			my_game_entry.g_characters_percentage *= 100.0;
		}
	}

	if (my_game_entry.g_total_tr > 0) {
		// Calculate the translation percentage
		if (my_game_entry.g_total_utr == 0 || my_game_entry.g_total_tr == my_game_entry.g_total_utr) {
			my_game_entry.g_translation_percentage = 100.0;
		}
		else {
			my_game_entry.g_translation_percentage = static_cast<RatioType>(static_cast<RatioType>(my_game_entry.g_total_tr) / static_cast<RatioType>(my_game_entry.g_total_tr + my_game_entry.g_total_utr));
			my_game_entry.g_translation_percentage *= 100.0;
		}
	}

	if (!my_game_entry.g_lines_match) {
		LOG("Found different lines!", HERE, "PoolRules");
	}

	LOG("\n", HERE, "PoolRules");

	std::string const percentage_file = "percentage_res.txt";

	if (std::filesystem::exists(percentage_file)) {
		std::filesystem::remove_all(percentage_file);
	}

	std::ofstream out_percentage(percentage_file, std::ios::out | std::ios::app);
	std::uint64_t ratio_cont = 0;
	for (auto const& j : possible_folders) {
		std::string const t = (j == "game_resident" || j == "MapObjName" || j == "subroutine") ? "" : "\t";
		// Can't overtranslate huh? A 101% chapter translation does not exist
		ConvertedType const result = static_cast<ConvertedType>(min(my_game_entry.C_Entries[ratio_cont].c_translation_percentage, 100.0));
		out_percentage << std::setprecision(3) << result << std::endl;
		std::stringstream pfss{};
		pfss << "Completion ratio for " << j << ": " << "\t" << t << std::setprecision(3) << result << " %\t(" <<
			my_game_entry.C_Entries[ratio_cont].c_total_tr << "/" << my_game_entry.C_Entries[ratio_cont].c_total_utr << ")" << std::endl;
		LOG(pfss.str(), HERE, "PoolRules");
		ratio_cont++;
	}
	LOG("\n", HERE, "PoolRules");
	// Can't overtranslate huh? A 101% game translation does not exist
	ConvertedType const total_result = static_cast<ConvertedType>(min(my_game_entry.g_translation_percentage, 100.0));
	out_percentage << std::setprecision(3) << total_result << std::endl;
	std::stringstream tss{};
	tss << "Total completion: \t\t\t" << std::setprecision(3) << total_result << "%\t(" <<
		my_game_entry.g_total_tr << "/" << my_game_entry.g_total_utr << ")" << std::endl;
	LOG(tss.str(), HERE, "PoolRules");
	out_percentage.close();

	LOG("\n", HERE, "PoolRules");

	std::string const charcount_file = "charcount_res.txt";

	if (std::filesystem::exists(charcount_file)) {
		std::filesystem::remove_all(charcount_file);
	}

	std::uint64_t chapcont = 0;
	std::ofstream out_ccount("charcount_res.txt", std::ios::out | std::ios::app);
	for (auto const& j : possible_folders) {
		out_ccount << j << std::endl;
		out_ccount << "[Y] TranslatedC: " << my_game_entry.C_Entries[chapcont].c_total_characters_tr << std::endl;
		out_ccount << "[N] TranslatedC: " << my_game_entry.C_Entries[chapcont].c_total_characters_utr << std::endl;
		out_ccount << "Character Ratio: " << my_game_entry.C_Entries[chapcont].c_characters_percentage << "%" << std::endl;
		out_ccount << "[Y] TranslatedL: " << my_game_entry.C_Entries[chapcont].c_total_tr << std::endl;
		out_ccount << "[N] TranslatedL: " << my_game_entry.C_Entries[chapcont].c_total_utr << std::endl;
		out_ccount << "Translation Ratio: " << my_game_entry.C_Entries[chapcont].c_translation_percentage << "%" << std::endl;
		out_ccount << std::endl;
		chapcont++;
	}
	out_ccount << "Total: " << std::endl;
	out_ccount << "[Y] TranslatedC: " << my_game_entry.g_total_characters_tr << std::endl;
	out_ccount << "[N] TranslatedC: " << my_game_entry.g_total_characters_utr << std::endl;
	out_ccount << "Character Ratio: " << my_game_entry.g_characters_percentage << "%" << std::endl;
	out_ccount << "[Y] TranslatedL: " << my_game_entry.g_total_tr << std::endl;
	out_ccount << "[N] TranslatedL: " << my_game_entry.g_total_utr << std::endl;
	out_ccount << "Translation Ratio: " << my_game_entry.g_translation_percentage << "%" << std::endl;
	out_ccount.close();

	chapcont = 0;
	std::ofstream det_ccount("detailed_charcount_rex.txt", std::ios::out | std::ios::app);
	for (auto const& cc : my_game_entry.C_Entries) {
		det_ccount << possible_folders[chapcont] << std::endl;
		det_ccount << std::endl;
		det_ccount << "\t[" << (cc.c_lines_match ? "Y" : "N") << "] Lines match: " << cc.c_total_lines << std::endl;
		det_ccount << "\t[Y] TranslatedC:  " << cc.c_total_characters_tr << std::endl;
		det_ccount << "\t[N] TranslatedC:  " << cc.c_total_characters_utr << std::endl;
		det_ccount << "\tCharacter Ratio: " << cc.c_characters_percentage << "%" << std::endl;
		det_ccount << "\t[Y] TranslatedL:  " << cc.c_total_tr << std::endl;
		det_ccount << "\t[N] TranslatedL:  " << cc.c_total_utr << std::endl;
		det_ccount << "\tTranslation Ratio: " << cc.c_translation_percentage << "%" << std::endl;
		det_ccount << std::endl;
		if (cc.F_Entries.size() > 0) {
			det_ccount << "\tFiles:" << std::endl;
			det_ccount << std::endl;
			for (auto const& ff : cc.F_Entries) {
				det_ccount << "\t\t" << ff.f_name << std::endl;
				det_ccount << std::endl;
				det_ccount << "\t\t\t[" << (ff.f_lines_match ? "Y" : "N") << "] Lines match: " << ff.f_total_lines << std::endl;
				det_ccount << "\t\t\t[Y] TranslatedC:  " << ff.f_total_characters_tr << std::endl;
				det_ccount << "\t\t\t[N] TranslatedC:  " << ff.f_total_characters_utr << std::endl;
				det_ccount << "\t\t\tCharacter Ratio: " << ff.f_characters_percentage << "%" << std::endl;
				det_ccount << "\t\t\t[Y] TranslatedL:  " << ff.f_total_tr << std::endl;
				det_ccount << "\t\t\t[N] TranslatedL:  " << ff.f_total_utr << std::endl;
				det_ccount << "\t\t\tTranslation Ratio: " << ff.f_translation_percentage << "%" << std::endl;
				det_ccount << std::endl;
			}
		}
		det_ccount << std::endl;
		det_ccount << std::endl;
		chapcont++;
	}
	det_ccount << "Total: " << std::endl;
	det_ccount << "\t[" << (my_game_entry.g_lines_match ? "Y" : "N") << "] Lines match: " << my_game_entry.g_total_lines << std::endl;
	det_ccount << "\t[Y] TranslatedC:  " << my_game_entry.g_total_characters_tr << std::endl;
	det_ccount << "\t[N] TranslatedC:  " << my_game_entry.g_total_characters_utr << std::endl;
	det_ccount << "\tCharacter Ratio: " << my_game_entry.g_characters_percentage << "%" << std::endl;
	det_ccount << "\t[Y] TranslatedL:  " << my_game_entry.g_total_tr << std::endl;
	det_ccount << "\t[N] TranslatedL:  " << my_game_entry.g_total_utr << std::endl;
	det_ccount << "\tTranslation Ratio: " << my_game_entry.g_translation_percentage << "%" << std::endl;
	det_ccount.close();

	// Test to see if the lines numbers can be trusted

	std::ofstream file_line_test("file_line_test.txt", std::ios::out | std::ios::app);
	for (auto const& test_file : test_files) {

		std::vector<std::string> lines{};
		std::string filepath{};
		if (std::filesystem::is_directory(test_file)) {
			for (auto const& file : std::filesystem::recursive_directory_iterator(test_file)) {
				if (file.is_directory()) {
					continue;
				}
				filepath = file.path().string();
				break;
			}
		}
		else {
			filepath = test_file;
		}

		if (filepath.empty()) {
			LOG("WARNING: Skipping file: \"" + test_file + "\" because it's empty!", HERE, "PoolRules");
			continue;
		}

		if (!std::filesystem::exists(filepath)) {
			LOG("WARNING: Skipping file: \"" + test_file + "\" because its filepath doesn't exist!", HERE, "PoolRules");
			continue;
		}

		std::string temp{};
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		while (std::getline(in, temp)) {
			lines.push_back(temp);
		}
		in.close();

		std::filesystem::path const fp(filepath);
		std::string const filename = fp.filename().string();

		std::uint64_t cont_lines = 0;
		file_line_test << "File: " << Common::ShortenFilename(filepath, 2) << std::endl;
		file_line_test << "Lines match (supposedly): " << (linemap[filename] ? "Yes" : "No") << std::endl;
		file_line_test << "Lines No.: " << lines.size() << std::endl;
		for (auto const& line : lines) {
			cont_lines++;
			file_line_test << "[" << cont_lines << "] \"" << line << "\"" << std::endl;
			file_line_test << "\t";
			for (auto const& ch : line) {
				file_line_test << "(" << static_cast<int>(ch) << ") ";
			}
			file_line_test << std::endl;
		}
		file_line_test << std::endl;
	}
	file_line_test.close();

	return true;
}

// Convert a translated file path into its English equivalent.
// Example:
//   "DGRV3/chapter3/scene_12.txt"
// → "DGRV3_EN/chapter3/scene_12.txt"

std::string ReplaceWithEnglish(std::string const& str,
	std::string const& repo,
	std::string const& repo_en)
{
	std::filesystem::path p(str);
	std::filesystem::path new_path;

	bool replaced = false;
	for (auto const& part : p) {
		if (!replaced && part == repo) {
			new_path /= repo_en; // append replacement segment
			replaced = true;
		}
		else {
			new_path /= part;    // append original segment
		}
	}

	return new_path.string();
}