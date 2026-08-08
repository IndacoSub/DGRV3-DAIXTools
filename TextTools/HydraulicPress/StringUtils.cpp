// Team DAIX, 2026
// HYDRAULICPRESS — StringUtils
//
// The majority of this code was written between 2020 and 2022
//
// This module's purpose is to:
// 1. Provide helper functions for splitting, replacing, and analyzing strings
// 2. Support Randomizer.cpp with alphanumeric checks and random CLT marker insertion
// 3. Normalize filenames and directory paths
// 4. Handle WEAK/AGREE CLT marker generation for randomized lines
//
// Used throughout HydraulicPress for text manipulation and CLT processing.


#include "StringUtils.h"
#include <iostream>
#include <algorithm>
#include <regex>
#include <ranges>

#include "Randomizer.h"
#include "../Common/Common.h"

#define MP(x, y) std::make_pair(x, y)

// Splits a string into words using whitespace extraction.
// Removes the delimiter character from each word.
// If remove_newlines = true, normalize all newline/tab escape sequences.
// Example:
//   SplitByCharacter("Hello\nWorld", ' ', true)
// → ["Hello", "World"]

std::vector<std::string> StringUtils::SplitByCharacter(std::string str, char const& delim, bool const& remove_newlines) {

	// Normalize all newline/tab variants to spaces.
	// Handles both real characters (\n) and escaped sequences ("\\n").

	if (remove_newlines) {
		str = StringUtils::ReplaceSubstring(str, "\r\n", " ");
		str = StringUtils::ReplaceSubstring(str, "\r", " ");
		str = StringUtils::ReplaceSubstring(str, "\n", " ");
		str = StringUtils::ReplaceSubstring(str, "\t", " ");
		str = StringUtils::ReplaceSubstring(str, "\\r\\n", " ");
		str = StringUtils::ReplaceSubstring(str, "\\r", " ");
		str = StringUtils::ReplaceSubstring(str, "\\n", " ");

		// Why not \\t at this point?
		//str = StringUtils::ReplaceSubstring(str, "\\t", " ");
	}

	std::vector<std::string> ret;
	// Using std::istringstream to split the string
	std::istringstream iss(str);

	// Extract words from the string
	do {
		std::string word;
		iss >> word;

		word.erase(std::remove(word.begin(), word.end(), delim), word.end());

		if (!word.empty()) {
			ret.push_back(word);
		}
	} while (iss);
	return ret;
}

// Counts how many times 'substring' appears inside 'full_string'.
// Example: CountOccurrences("aaa", "a") → 3

std::uint64_t StringUtils::CountOccurrences(std::string const& full_string, std::string const& substring) {

	// std::count does not support strings, at least for now

	std::uint64_t occurrences = 0;
	std::string::size_type start = 0;

	while ((start = full_string.find(substring, start)) != std::string::npos) {
		++occurrences;
		start += substring.length(); // see the note
	}

	return occurrences;
}

// Replaces all occurrences of 'substring' with 'replace_with'.
// Example: ReplaceSubstring("abcabc", "a", "X") → "XbcXbc"

std::string StringUtils::ReplaceSubstring(std::string const& full_string, std::string const& substring, std::string const& replace_with) {

	std::string str = full_string;

	std::size_t index = 0;
	while (true) {
		index = str.find(substring, index);
		if (index == std::string::npos) break;
		str.replace(index, substring.length(), replace_with);
		index += replace_with.length();
	}

	return str;
}

// Returns true if the string is considered "valid" for randomization.
// Rejects punctuation, CLT markers, SIGNAL markers, PAD/DIG markers,
// and various special tokens.
// Used by Randomizer::GetRandomString() to ensure output looks like text.

bool StringUtils::IsAlphanumericString(std::string const& word) {

	// This is genuinely horrifying

	return
		(
			!word.empty() &&
			word != "\n" &&
			word != "\\n" &&
			word != "\r" &&
			word != "\\r" &&
			word != "\t" &&
			word != "\\t" &&
			word != "{" &&
			word != "}" &&
			word != "<" &&
			word != ">" &&
			word != "..." &&
			word != "." &&
			word != "!" &&
			word != "?" &&
			word != "!?" &&
			word != "?!" &&
			word != "*" &&
			word != "\"" &&
			!Common::StringContains(word, "SIGNAL") &&
			!Common::StringContains(word, "MAKE_") &&
			!Common::StringContains(word, "VAR_") &&
			!Common::StringContains(word, "CLT") &&
			!Common::StringContains(word, "PAD") &&
			!Common::StringContains(word, "DIF") &&
			((int)word.front()) >= 0
			);
}

// Returns only the filename component of a path.
// Example: "DGRV3/chapter3/scene_12.txt" → "scene_12.txt"

std::string StringUtils::TrimDirectories(std::string const& long_filename) {

	std::filesystem::path const p(long_filename);
	std::string const line = p.filename().string();

	return line;
}

// Inserts a CLT WEAK/AGREE marker into a random word of the line.
// Optionally spans multiple words and closes with <CLT=cltNORMAL>.
// Example:
//   Input:  "I totally agree with this"
//   Output: "<CLT=cltAGREE>I totally agree<CLT=cltNORMAL> with this"

std::string StringUtils::CreateRandomWeakAgree(std::string const& my_str, std::string const& toput) {
	// Split into words (space-delimited, keep newlines)
	std::vector<std::string> outvec = StringUtils::SplitByCharacter(my_str, ' ', false);

	// ✅ Guard: if no words, return original string unchanged
	if (outvec.empty()) {
		return my_str;
	}

	// Pick a random starting index
	int const start_index = Randomizer::GetRand(0, static_cast<int>(outvec.size()) - 1);

	// 1-in-3 chance to extend the CLT marker across multiple words.
	// Ensures randomized WEAK/AGREE markers look natural.

	// Decide whether to span multiple words (1 in 3 chance)
	constexpr int MULTIWORD_CHANCE = 3; // 1 in 3
	bool const multiple_string =
		(outvec.size() > 1) &&
		(start_index < static_cast<int>(outvec.size()) - 1) &&
		(Randomizer::GetRand(0, MULTIWORD_CHANCE - 1) == 0);

	// Insert the WEAK/AGREE marker at the start
	outvec[start_index].insert(0, toput);

	// Determine where to end the marker
	int end_index = start_index;
	if (multiple_string) {
		end_index = Randomizer::GetRand(start_index + 1, static_cast<int>(outvec.size()) - 1);
	}

	// Append the closing tag
	// Append <CLT=cltNORMAL> to mark the end of the WEAK/AGREE span.

	outvec[end_index].append("<CLT=cltNORMAL>");

	// Join back into a single string
	std::ostringstream oss;
	for (std::size_t i = 0; i < outvec.size(); ++i) {
		if (i > 0) oss << ' ';
		oss << outvec[i];
	}

	return oss.str();
}

// Converts a string to lowercase using std::tolower.
// Example: "HeLLo" → "hello"

std::string StringUtils::ToLower(std::string const& str) {

	std::string str2 = str;
	std::transform(str2.begin(), str2.end(), str2.begin(),
		[](char c) { return std::tolower(static_cast<unsigned char>(c)); });
	return str2;
}
