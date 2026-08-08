// Team DAIX, 2026
// HYDRAULICPRESS — WordCounter
//
// The majority of this code was written between 2020 and 2022
//
// This module's purpose is to:
// 1. Scan selected .txt script files and extract all valid words
// 2. Filter out common English words and Italian metadata/noise
// 3. Count occurrences of each remaining word
// 4. Track which files each word appears in
// 5. Generate a ranked list of the top 200 most frequent words
//
// Used to analyze vocabulary usage and detect unusual or repeated phrasing.


#include "WordCounter.h"
#include "StringUtils.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#include <regex>
#include <map>

#include "../Common/Common.h"

using PlaceMap = std::map<std::string, std::vector<std::string>>;

void CountUtils::CountWords(std::vector<std::string> const& files_to_search) {

	// List of (word, count) pairs.
	// Example: { "gatto", 42 }

	CountList mcl{ {} };

	// Maps each word → list of filenames where the word appears.
	// Example: pm["gatto"] = { "scene_01.txt", "scene_05.txt" }

	PlaceMap pm{ {{}} };
	constexpr static char delim = ' '; // Divide by space
	constexpr static bool all_lowercase = false;

	// List of most used English words in V3, to exclude from the "most used" words list, sorted alphabetically
	// This is nuts

	// English stop‑word list.
	// These extremely common words are excluded from counting.
	// Example: "the", "and", "is", "to", "you", etc.

	static const std::vector<std::string> en_list{
		"about", "after", "all", "an", "and", "are", "aren't", "as", "at",
		"back", "be", "become", "became", "been",  "bear", "before", "body", "bullet", "bullets", "but", "by",
		"can", "class", "classroom", "close", "conversation", "correct", "could", "courtyard", "cylinder", "crazy", "culprit",
		"damage", "debate", "do", "don't", "door", "down", "dummy", "during",
		"eight", "eighteen", "eleven", "even", "examine", "explain", "explanation",
		"fifteen", "find", "five", "focus", "for", "found", "four", "fourteen", "frame", "from", "front",
		"game", "gauge", "go", "gone", "got", "gotta",
		"had", "has", "have", "he", "he's", "her", "him", "his", "holding", "how", "however",
		"i'm", "i've", "incorrect", "influence", "into", "investigate", "investigation", "if", "is", "isn't", "it", "it's",
		"just",
		"kill", "killing", "key",
		"left", "library", "lie", "lies", "like", "limit", "loaded", "lose",
		"mastermind", "move", "my",
		"need", "needed", "nine", "nineteen", "noise", "non-stop", "not", "now",
		"odd", "of", "off", "on", "or", "one", "only", "open", "other", "out",
		"play", "playing", "press", "pressing", "proper",
		"receive", "right", "room",
		"said", "scene", "school", "select", "seven", "seventeen", "she", "she's", "silencer", "six", "sixteen", "shoot", "someone", "something", "spare", "spot", "spots", "story",
		"ten", "text", "that", "that's", "think", "this", "thirteen", "the", "theater", "then", "there", "these", "they", "they're", "their", "those", "three", "time", "to", "trial", "truth", "twelve", "twenty", "two",
		"ultimate", "up", "use",
		"was", "wasn't", "weak", "we", "we're", "we've", "went", "were", "weren't", "what", "when", "where", "which", "while", "who", "why", "will", "with", "won't", "world", "would", "wrong",
		"you", "your", "yours", "you're", "you've",
	};

	// Ignore these Italian words
	// Italian noise / metadata / punctuation to remove before counting.
	// Includes CLT markers, punctuation, and translator notes.
	// Example: "<CLT=cltWEAK>" → removed entirely.

	static const std::vector<std::string> remove_list{
		"-- Tradotto automaticamente",
		"Alt:",
		"NDR:",
		"È consigliato ricontrollare questa traduzione.",
		"(",
		")",
		"[",
		"]",
		"{",
		"}",
		"?",
		".",
		"!",
		",",
		";",
		":",
		"-",
		"<CLT=cltMIND>",
		"<CLT=cltSTRONG>",
		"<CLT=cltNORMAL>",
		"<CLT=cltSYSTEM>",
		"<CLT=cltWEAK>",
		"<CLT=cltKTDM>",
		"<CLT=cltAGREE>",
		"<CLT=size1>",
		"<CLT=size1.1>",
		"<CLT=size1.2>",
		"<CLT=size1.5>",
		"<CLT=size2>",
		"<CLT=size2.2>",
		"<CLT=size3>",
		"<CLT=size4>",
		"<CLT=size6>",
		"0123456789012345678901234567890123456789",
	};

	std::uint64_t total_words = 0;
	// Search through files_to_search
	// Iterate through all files and extract words line by line.
	// Apply cleanup, filtering, and counting.

	for (auto const& file : files_to_search) {
		if (!std::filesystem::exists(file)) {
			LOG("WARNING: File does not exist: " + file, HERE, "HydraulicPress");
			continue;
		}
		// The file exists, open it
		std::ifstream infs(file, std::ios::in);
		std::string temp{};
		// Iterate through lines
		while (std::getline(infs, temp)) {

			// Remove all noise tokens from the line before splitting.
			// Example: "Ciao <CLT=cltWEAK> mondo!" → "Ciao  mondo"

			for (auto const& signal : remove_list) {
				temp = StringUtils::ReplaceSubstring(temp, signal, "");
			}

			// Remove newlines
			temp = std::regex_replace(temp, std::regex("\n"), "");
			temp = std::regex_replace(temp, std::regex("\r"), "");

			if (temp.empty()) {
				continue;
			}

			// Split the cleaned line into words using space as delimiter.
			// Example: "Il mio gatto" → ["Il", "mio", "gatto"]


			std::vector<std::string> const& out = StringUtils::SplitByCharacter(temp, delim, true);
			if (out.empty()) {
				// The line is empty
				continue;
			}

			// The line is not empty
			for (auto word : out) {
				// Iterate through words ("il", then "mio", then "gatto")
				// Split again (WHY??)

				if (word.find_first_not_of(delim) == std::string::npos) {
					continue;
				}
				
				if (!StringUtils::IsAlphanumericString(word)) {
					continue;
				}

				word = StringUtils::ReplaceSubstring(word, std::string{ delim }, "");

				while (word.starts_with(delim)) {
					//std::cout << "Delim begin in \"" << word << "\"" << std::endl;
					word = word.substr(0);
				}

				while (word.ends_with(delim)) {
					//std::cout << "Delim end in \"" << word << "\"" << std::endl;
					word = word.substr(0, word.length() - 1);
				}

				if (word.starts_with("EMPTY_")) {
					continue;
				}
				if (word.starts_with("<PLATFORM")) {
					continue;
				}
				if (word.starts_with("<SIGNAL")) {
					continue;
				}
				if (word.starts_with("MAKE_")) {
					continue;
				}

				std::string const lower = StringUtils::ToLower(word);

				if (all_lowercase) {
					word = lower;
				}

				auto const bl_it = std::find(en_list.begin(), en_list.end(), lower);
				if (bl_it != en_list.end()) {
					continue;
				}

				// Remove all noise tokens from the line before splitting.
				// Example: "Ciao <CLT=cltWEAK> mondo!" → "Ciao  mondo"
				for (auto const& signal : remove_list) {
					word = StringUtils::ReplaceSubstring(word, signal, "");
				}

				if (word.empty()) {
					continue;
				}

				bool constexpr trim = false;

				if constexpr (trim) {
					// _ added because of randomizer's EMPTY_LINE, PAD_SKIPPED, etc.
					std::size_t const last_alphabetic = word.find_first_not_of("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789'-_");
					if (last_alphabetic != std::string::npos) {
						if (last_alphabetic == 0) {
							// Same as above
							std::size_t const first_alphabetic = word.find_first_of("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789'-_");
							if (first_alphabetic == std::string::npos) {
								continue;
							}
							else {
								if (first_alphabetic != last_alphabetic) {
									word = word.substr(first_alphabetic);
								}
							}
						}
						else {
							word = word.substr(0, last_alphabetic);
						}
					}
				}

				std::filesystem::path const p(file);
				std::string const filename = p.filename().string();
				auto const it_fn = std::find(pm[word].begin(), pm[word].end(), filename);
				if (it_fn == pm[word].end()) {
					pm[word].push_back(filename);
				}

				auto const it = std::find_if(mcl.begin(), mcl.end(), [&](CountType const& ct) -> bool {
					return (
						ct.first == word
						);
				});
				if (it == mcl.end()) {
					CountType const ct_temp = std::make_pair(word, 1);
					mcl.push_back(ct_temp);
				}
				else {
					auto const index = std::distance(mcl.begin(), it);
					mcl[index].second++;
				}
				total_words++;
			}
		}
		infs.close();
	}

	std::sort(mcl.begin(), mcl.end(), [&](CountType const& ct, CountType const& ct2) -> bool {return ct.second > ct2.second; });

	std::string static const words_file = "words_counted.txt";
	std::uint64_t constexpr static HowMany = 200;

	if (std::filesystem::exists(words_file)) {
		std::filesystem::remove(words_file);
	}

	std::stringstream ss{};
	std::string temp{};
	for (std::uint64_t j = 0; j < HowMany; j++) {
		if (j >= mcl.size()) {
			break;
		}
		temp.clear();
		temp = std::to_string(j + 1);
		while (temp.length() <= 12) {
			temp += " ";
		}
		temp += mcl[j].first;
		while (temp.length() <= 47) {
			temp += " ";
		}
		temp += " occurs " + std::to_string(mcl[j].second) + " times (" + std::to_string(CalculatePercentage(mcl[j].second, total_words)) + "%) :";
		while (temp.length() <= 95) {
			temp += " ";
		}
		temp += " { ";
		for (std::uint64_t i = 0; i < pm[mcl[j].first].size(); i++) {
			temp += pm[mcl[j].first][i];
			if (i < pm[mcl[j].first].size() - 1) {
				temp += ", ";
			}
		}
		temp += " }";

		ss << temp << std::endl;
	}

	std::ofstream count_words(words_file, std::ios::out | std::ios::app);
	count_words << "Words:" << std::endl << std::endl << ss.str() << std::endl;
	count_words.close();
}

// Returns (number / total) * 100.
// Example: 42 occurrences out of 1000 → 4.2%

long double CountUtils::CalculatePercentage(std::uint64_t const& number, std::uint64_t const& total) {
	long double percentage = static_cast<long double>(number) / static_cast<long double>(total);
	percentage *= static_cast<long double>(100.0);
	return percentage;
}

unsigned long long CountUtils::GetAvailableRAM(void) {
#ifdef _WIN32
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);
	return status.ullTotalPhys;
#else
	long const pages = sysconf(_SC_PHYS_PAGES);
	long const page_size = sysconf(_SC_PAGE_SIZE);
	return pages * page_size;
#endif
}

// Returns total physical RAM on the system.
// Used for debugging or performance diagnostics.

double CountUtils::GetAvailableRAMInGB(void) {

	auto const av_ram = GetAvailableRAM();
	long double const tot = av_ram / static_cast<long double>((static_cast<std::uint64_t>(static_cast<std::uint64_t>(1024) * 1024 * 1024)));
	double const ret = static_cast<double>(tot);
	return ret;
}