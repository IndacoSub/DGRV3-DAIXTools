// Team DAIX, 2026
// HYDRAULICPRESS — Randomizer
//
// The majority of this code was written between 2020 and 2022
//
// This module's purpose is to:
// 1. Read all script lines from selected .txt files
// 2. Randomize their order or replace them with random strings
// 3. Preserve special CLT markers (weak/agree) and reapply them after shuffling
// 4. Rebuild each file with randomized content
// 5. Generate a report showing where each original line ended up
//
// Used for testing, debugging, and stress‑testing the script pipeline... and for having fun, obviously :)

#include "Randomizer.h"
#include "StringUtils.h"

#include <regex>
#include <map>

#include "../Common/Common.h"
#include "../Common/Config.h"

void Randomizer::Randomize(std::vector<std::string> const& files_to_search) {

	LOG("Randomizing lines...", HERE, "HydraulicPress");

	// Stores metadata for a single file:
	// - Filename
	// - Number of non-bracket lines
	// - Whether the file begins/ends with { }
	// Example: "{" / "}" are preserved.

	struct FileLine {
		std::string Filename{};
		std::uint64_t Lines{};
		bool HasBrackets{};
	};

	// Records where a specific line originally appeared.
	// Used later to generate the randomizer report.
	// Example: ("scene_12.txt", 45)

	struct RandomEntry {
		std::string Filename{};
		std::uint64_t Line{};
	};

	// A list of all occurrences of a given line across files.
	// randmap maps: line_text → REVec
	// Example: "Hello world" → appears in 3 files at different line numbers.


	struct REVec {
		std::vector<RandomEntry> Entries{};
	};

	// Maps each unique line to all its original positions.
	// Used to track how lines move after randomization.


	std::map<std::string, REVec> randmap;

	// Read Text
	// Read all files and collect:
	// - total_text: every non-bracket line
	// - specials: lines containing <CLT=cltWEAK> or <CLT=cltAGREE>
	// - lines: per-file metadata
	// - randmap: original line positions


	std::vector<Randomizer::Special> specials{};
	std::vector<FileLine> lines{};
	std::vector<std::string> total_text{};
	for (auto const& j : files_to_search) {

		if (!std::filesystem::exists(j)) {
			LOG("[1] File does not exist: " + j, HERE, "HydraulicPress");
			continue;
		}

		FileLine fl;
		fl.Filename = j;

		std::ifstream in(j, std::ios::in);
		std::string temp{};
		std::uint64_t count = 0;
		while (std::getline(in, temp)) {

			temp = std::regex_replace(temp, std::regex("\n"), "");
			temp = std::regex_replace(temp, std::regex("\r"), "");

			// Detect { }.
			// These brackets must be preserved exactly in output.

			fl.HasBrackets = (fl.Lines == 0 && temp == "{") || (fl.Lines > 0 && temp == "}");

			if (!fl.HasBrackets) [[likely]] {
				total_text.push_back(temp);
				fl.Lines++;
				std::filesystem::path const p(j);
				if (!temp.empty()) {
					REVec revec{};
					revec.Entries.push_back({ p.filename().string(), count });
					randmap[temp] = revec;
				}
			}

			// Record special CLT markers so they can be re-applied after shuffling.
			// Example: "<CLT=cltWEAK>" on line 12 of scene_05.txt


			if (Common::StringContains(temp, "<CLT=cltWEAK>")) {
				Special s{};
				s.Filename = fl.Filename;
				s.Line = fl.Lines;
				s.HasWeak = true;
				specials.push_back(s);
			}

			if (Common::StringContains(temp, "<CLT=cltAGREE>")) {
				Special s{};
				s.Filename = fl.Filename;
				s.Line = fl.Lines;
				s.HasAgree = true;
				specials.push_back(s);
			}

			count++;
		}
		in.close();

		lines.push_back(fl);
	}

	if (total_text.empty()) {
		return;
	}

	if (lines.empty()) {
		return;
	}

	// Count total number of non-bracket lines across all files.
	// Used to ensure we don't read past total_text during reconstruction.

	std::uint64_t total_lines = std::accumulate(lines.begin(), lines.end(), 0ULL,
		[](std::uint64_t accumulator, const auto& j) {
		return accumulator + j.Lines;
	});

	// Shuffle Text
	// SmartRandomization: shuffle all lines globally.
	// Otherwise: replace each line with a random alphanumeric string.


	if (Configuration::ConfigMap["SmartRandomization"]) {
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(total_text.begin(), total_text.end(), g);
	}

	// Rebuild each file:
	// - Delete original
	// - Recreate with randomized lines
	// - Preserve brackets
	// - Track new line positions in randmap


	std::uint64_t totlinecont = 0;
	for (std::uint64_t file = 0; file < files_to_search.size(); file++) {
		std::string const filename = files_to_search[file];

		// Delete the file, then re-create it
		if (std::filesystem::exists(filename)) {
			std::filesystem::remove(filename);
		}
		else {
			LOG("[2] File does not exist: " + filename, HERE, "HydraulicPress");
			continue;
		}

		if (file >= lines.size()) {
			LOG("Too few lines (" + std::to_string(file) + ")", HERE, "HydraulicPress");
			// Otherwise it'd crash, right?
			continue;
		}

		if (filename != lines[file].Filename) {
			LOG("WARNING: Different filenames: " + filename + " and " + lines[file].Filename + "!", HERE, "HydraulicPress");
		}

		std::vector<std::string> my_recreated_file{};
		bool const has_brackets = lines[file].HasBrackets;
		if (has_brackets) {
			my_recreated_file.push_back("{");
		}

		std::uint64_t const my_lines = lines[file].Lines;
		if (my_lines > 0) [[likely]] {
			for (std::uint64_t line = 0; line < my_lines; line++) {
				std::string my_line{};
				if (Configuration::ConfigMap["SmartRandomization"]) {
					my_line = total_text[totlinecont];
				}
				else {
					my_line = GetRandomString();
					//Common::DoLog("Randomizing string @ " + filename + ", L" + std::to_string(line) + ", replaced with: " + my_line, HERE, "HydraulicPress");
				}
				my_recreated_file.push_back(my_line);
				totlinecont++;
			}
		}

		if (has_brackets) {
			my_recreated_file.push_back("}");
		}

		std::uint64_t nline = 0;

		std::ofstream out_file(filename, std::ios::out | std::ios::app | std::ios::binary);
		for (auto const& new_lines : my_recreated_file) {
			/*
			std::string const my_str = Common::CheckPlatforms(new_lines);
			if (my_str == "SKIPTHISLINE") {
				// todo
				out_file << "PLATFORM_SKIPPED" << std::endl;
			}
			*/
			std::string const my_str = new_lines;
			std::string out_str{};

			if (my_str != "{" && my_str != "}" && !my_str.empty()) {
				std::filesystem::path pfn(filename);
				randmap[my_str].Entries.push_back({ pfn.filename().string(), nline});
			}

			// Replace lines containing DIG or PAD markers with placeholders.
			// DIG → "DIG_SKIPPED"
			// PAD → "PAD_SKIPPED"
			// Empty/space-only lines → "EMPTY_LINE"


			if (!Common::StringContains(my_str, "DIG")) [[likely]] {
				if (!Common::StringContains(my_str, "PAD")) [[likely]] {
					bool const only_spaces = my_str.find_first_not_of(" ") == std::string::npos;
					if (!my_str.empty() && !only_spaces) {
						out_str = my_str;
					}
					else {
						out_str = "EMPTY_LINE";
					}
				}
				else {
					out_str = "PAD_SKIPPED";
				}
			}
			else {
				out_str = "DIG_SKIPPED";
			}

			// Remove CLT markers before applying specials.
			// They will be reinserted by HandleSpecials().

			out_str = std::regex_replace(out_str, std::regex("<CLT=cltWEAK>"), "");
			out_str = std::regex_replace(out_str, std::regex("<CLT=cltAGREE>"), "");

			if (!new_lines.starts_with('{') && !new_lines.starts_with('}')) {
				// Reapply CLT weak/agree markers to the correct randomized line.
				// Why +1? Because original CLT markers were recorded using 1-based indexing.

				out_str = HandleSpecials(out_str, specials, filename, nline + 1);
				nline++;
			}

			out_file << out_str << std::endl;
		}
		out_file.close();
	}

	LOG("Randomizing done!", HERE, "HydraulicPress");
	LOG("Saving randomizer report...", HERE, "HydraulicPress");

	// Save a report showing where each original line moved.
	// Example:
	// "Hello world" (from scene_12.txt L45) is now here: { scene_03.txt L12 | scene_07.txt L88 }


	std::ofstream out_report("randomizer_report.txt", std::ios::out | std::ios::app);
	for (auto const& line : randmap) {
		out_report << "\"" << line.first << "\" (from ";
		out_report << randmap[line.first].Entries[0].Filename << " L" << randmap[line.first].Entries[0].Line;
		out_report << ") is now here : ";
		out_report << "{ ";
		for (int i = 1; i < randmap[line.first].Entries.size(); i++) {
			out_report << randmap[line.first].Entries[i].Filename << " L" << randmap[line.first].Entries[i].Line;
			if (i < randmap[line.first].Entries.size() - 1) {
				out_report << " | ";
			}
		}
		out_report << " }";
		out_report << std::endl;
	}
	out_report.close();

	LOG("Done saving randomizer report", HERE, "HydraulicPress");
}

// Returns a random integer in [min, max] using mt19937_64.


std::uint64_t Randomizer::GetRand(std::uint64_t const& min, std::uint64_t const& max) {

	if (min == max) {
		return min;
	}

	std::random_device rd{};
	std::mt19937_64 mt{ rd() };
	std::uniform_int_distribution<std::uint64_t> dist{ min, max };
	std::uint64_t const ret = dist(mt);
	return ret;
}

// Generates a random uppercase alphanumeric string.
// Length: 5–30 characters.
// First character forced into A–Z range.
// Repeats until the string is alphanumeric.


std::string Randomizer::GetRandomString(void) {

	std::string ret{};
	static constexpr std::uint64_t ASCII_Letter_A = 65;
	static constexpr std::uint64_t ASCII_Letter_Z = 90;
	static constexpr std::uint64_t ASCII_Letter_Diff = ASCII_Letter_Z - ASCII_Letter_A;
	do {
		ret.clear();
		std::uint64_t const max = GetRand(15, 30);
		std::uint64_t const min = GetRand(5, 7);
		std::uint64_t const len = GetRand(min, max);
		for (std::uint64_t index = 0; index < len; index++) {
			ret += static_cast<char>(ASCII_Letter_A + GetRand(0, ASCII_Letter_Diff) + (index == 0 ? ASCII_Letter_Diff : 0));
		}
	} while (!StringUtils::IsAlphanumericString(ret));
	return ret;
}

// Reapply CLT weak/agree markers to the correct randomized line.
// Example:
//   Original: "<CLT=cltWEAK>Hello"
//   Randomized: "XYZABC"
//   Output: CreateRandomWeakAgree("XYZABC", "<CLT=cltWEAK>")


std::string Randomizer::HandleSpecials(std::string const& my_line, std::vector<Randomizer::Special> const& specials, std::string const& filename, int const& line) {

	// What's the logic here?

	std::string ret = my_line;
	// Specials are "weak" and "agree"
	for (auto const& var : specials) {
		// If this isn't the right file(???)
		if (!Common::StringContains(filename, var.Filename)) {
			continue;
		}
		// If this is the line in which the var was encountered(???)
		if (var.Line == line) {
			if (var.HasWeak) {
				ret = StringUtils::CreateRandomWeakAgree(my_line, "<CLT=cltWEAK>");
			}
			else {
				if (var.HasAgree) {
					ret = StringUtils::CreateRandomWeakAgree(my_line, "<CLT=cltAGREE>");
				}
				else {
					LOG("WARNING: Unspecified special at line " + std::to_string(line) + "of" + filename + "!", HERE, "HydraulicPress");
				}
			}
		}
	}

	return ret;
}