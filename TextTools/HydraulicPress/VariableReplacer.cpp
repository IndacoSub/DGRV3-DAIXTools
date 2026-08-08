// Team DAIX, 2026
// HYDRAULICPRESS — VariableReplacer
//
// The majority of this code was written between 2020 and 2022
//
// This module's purpose is to:
// 1. Replace all VAR_* variables in script files with their values from vars.txt / vars_bak.txt
// 2. Handle MAKE_* variables with arguments (MAKE_XYZ(arg))
// 3. Remove or preserve SIGNAL_* tags depending on configuration
// 4. Apply platform filtering (<PLATFORM_PC>, <PLATFORM_SWITCH>, etc.)
// 5. Produce both normal and "baked" output files
// 6. Generate a detailed replacement map showing every substitution performed
//
// This is the core text‑compilation engine of HydraulicPress.


#include "VariableReplacer.h"
#include "FileUtils.h"
#include "Platform.h"
#include "StringUtils.h"

#include <tuple>

#include "../Common/Config.h"

// WHY IS THIS NOT A STRUCT/CLASS?
// Replacement log entry:
// [filename|line], original text, variable name, variable value, final text.
// Used to generate var_replace_map.txt.

using SuperVarEntry = std::tuple<std::string, std::string, std::string, std::string, std::string>;

// Removes SIGNAL_* tags from a line.
// Special rule: SIGNAL_OE is non‑terminal (only the tag is removed).
// All other SIGNAL_* tags are terminal: remove tag + everything after it.

std::string VariableUtils::RemoveSignal(std::string const& full_string) {

	if (!Common::StringContains(full_string, EncryptString("<SIGNAL"))) {
		return full_string;
	}

	// Signals are *usually* at the end
	// The only exception is SIGNAL_OE
	// So... Are we going to do anything about SIGNAL_OE?
	// Well, actually no!
	// <SIGNAL_OE> stands for "otherwise empty"
	// And it means that the line is, for some reason, empty... when it shouldn't be
	// Also, SIGNAL_OE was already handled before entering this function
	// So it's fine

	// <SIGNAL>s (there can be multiple) are at the end of a sentence.
	// Examples: <SIGNAL_CBL>, <SIGNAL_NDR>, <SIGNAL_ALT>, <SIGNAL_AutoTL>
	// And there can be multiple ones in a single sentence
	// (anything after the first <SIGNAL found, it included, must be removed)
	//
	// Special placement rule:
	// - <SIGNAL_OE> is the only one that's at the start of the sentence
	//   (and what's after it *must* be kept).
	// - All other signals are terminal (end-of-sentence) markers.
	//
	// Strategy:
	// - Strip any <SIGNAL_OE> tags we encounter (remove the tag only).
	// - On the first non-OE signal, remove the tag and everything after it.

	std::string x = full_string;
	std::size_t search_from = 0;

	while (true) {
		std::size_t const pos = x.find("<SIGNAL_", search_from);
		if (pos == std::string::npos) {
			break; // no more signals
		}

		std::size_t const end = x.find('>', pos);
		if (end == std::string::npos) {
			// Malformed tag (no '>'): safest is to treat it as terminal and drop the tail.
			x.erase(pos);
			break;
		}

		// Extract the tag name between "<SIGNAL_" and ">"
		std::string tag_name = x.substr(pos + 8, end - (pos + 8)); // e.g., "OE", "NDR", "NOSWAP", ...
		// Normalize to uppercase for safety
		for (auto& ch : tag_name) {
			ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
		}

		if (tag_name == "OE") {
			// Remove only the OE tag and keep scanning, because OE is the only non-terminal signal
			x.erase(pos, end - pos + 1);
			// Continue from the same index (in case tags are adjacent like "<SIGNAL_OE><SIGNAL_NDR>")
			search_from = pos;
		}
		else {
			// Any other signal is terminal: remove the tag and everything after it
			x.erase(pos);
			break;
		}
	}

	return x;
}

// Returns true if the variable name exists in the loaded variable list.

bool VariableUtils::VarExists(std::vector<VarEntry> const& variable_list, std::string const& single) {

	auto const it = std::find_if(variable_list.begin(), variable_list.end(), [=](VarEntry const& a) -> bool {return a.first == single; });
	return it != variable_list.end();
}

// Checks whether *all* VAR_* tokens in a sentence exist in the variable list.
// Used for recursive replacement: continue replacing until no unknown VAR_* remain.

bool VariableUtils::AllVarsExist(std::vector<VarEntry> const& variable_list, std::string const& sentence){

	if (!Common::StringContains(sentence, "VAR_")) {
		return true;
	}

	bool exists = true;
	std::string failure = "(unknown)";
	std::string sentence2 = sentence;
	sentence2 = StringUtils::ReplaceSubstring(sentence2, ">", " ");
	sentence2 = StringUtils::ReplaceSubstring(sentence2, "<", " ");

	auto const& words = StringUtils::SplitByCharacter(sentence2, ' ', true);
	for (auto const& word : words) {
		if (!exists) {
			break;
		}
		if (!Common::StringContains(word, "VAR_")) {
			continue;
		}

		// Oh my god

		std::string word2 = word;
		word2 = StringUtils::ReplaceSubstring(word2, ".", " ");
		word2 = StringUtils::ReplaceSubstring(word2, ",", " ");
		word2 = StringUtils::ReplaceSubstring(word2, ";", " ");
		word2 = StringUtils::ReplaceSubstring(word2, ":", " ");
		word2 = StringUtils::ReplaceSubstring(word2, "!", " ");
		word2 = StringUtils::ReplaceSubstring(word2, "?", " ");
		word2 = StringUtils::ReplaceSubstring(word2, "—", " ");
		word2 = StringUtils::ReplaceSubstring(word2, "-", " ");
		word2 = StringUtils::ReplaceSubstring(word2, "*", " ");
		word2 = StringUtils::ReplaceSubstring(word2, "(", " ");
		word2 = StringUtils::ReplaceSubstring(word2, ")", " ");
		word2 = StringUtils::ReplaceSubstring(word2, "\'", " ");
		word2 = StringUtils::ReplaceSubstring(word2, "\"", " ");
		word2 = StringUtils::ReplaceSubstring(word2, "\\n", " ");
		word2 = StringUtils::ReplaceSubstring(word2, "\\r", " ");
		auto const& w2 = StringUtils::SplitByCharacter(word2, ' ', true);
		for (auto const& word3 : w2) {
			if (!Common::StringContains(word3, "VAR_")) {
				continue;
			}
			exists &= VarExists(variable_list, word3);
			if (!exists) {
				failure = word3;
				break;
			}
		}
	}

	if (!exists) {
		//std::cout << "(At least) one var not found in: " << sentence << ": \"" << failure << "\"" << std::endl;
	}

	return exists;
}

// Main replacement routine:
// - Reads each file line-by-line
// - Applies variable replacement (including MAKE_* arguments)
// - Removes SIGNAL_* tags if configured
// - Applies platform filtering
// - Writes normal output and optional baked output
// - Logs every replacement into var_replace_map.txt


void VariableUtils::ReplaceVariables(std::vector<std::string> const& files_to_search, std::vector<VarEntry> const& variable_list, bool const& baked) {

	if (files_to_search.empty() || files_to_search.size() <= 0) {
		LOG("WARNING: The list of files to search was found empty!", HERE, "HydraulicPress");
		return;
	}

	// By this time, the variables have already been read

	// Iterate from 0 through the size of the list of the files to search
	for (std::uint64_t ftp = 0; ftp < files_to_search.size(); ftp++) {

		auto const openfile = std::filesystem::path(files_to_search.at(ftp));
		std::string const openfilename = openfile.filename().string();

		// Skip .pb files (Switch binary text files) — not processed here.
		if (openfilename.contains(".pb")) {
			continue;
		}

		// Read each file to search, one at a time
		std::ifstream in(openfile, std::ios::in | std::ios::binary);

		if (!in.good()) {
			LOG("WARNING: Couldn't manage to read the file: " + openfile.string(), HERE, "HydraulicPress");
			continue;
		}

		// Temporary std::string to read each line from the file
		std::string line;

		// (Unoptimized as hell)
		// Use ss as container for every single
		// line of text
		std::stringstream ss{};

		// (Again, unoptimized as hell)
		// Use this stringstream for
		// "baked" .txt files
		std::stringstream ss2{};

		if (variable_list.empty()) {
			LOG("WARNING: The variable list was found empty!", HERE, "HydraulicPress");
			return;
		}

		std::vector<SuperVarEntry> replace_map{};

		std::uint64_t cont = 0;
		// Read "in" and put the text in "line"
		while (std::getline(in, line)) {
			cont++;
			std::string x = line;
			// Search every single variable in the list in the line

			if (x.empty() || x.front() == '\n' || x.front() == '\r') {
				if (Configuration::ConfigMap["DoReplaceEmpty"]) {
					// Replace empty lines with a unique placeholder if DoReplaceEmpty = true.
					// Example: EMPTY_scene_12.txtL45

					std::string const& E_VALUE = "EMPTY_" + openfilename + "L" + std::to_string(cont);
					x = E_VALUE;
				}
			}
			else {
				// Check if it's SIGNAL_OE
				x = std::regex_replace(x, std::regex("<SIGNAL_OE>"), "");

				if (Configuration::ConfigMap["DoRemoveSignals"]) {
					// Signals are supposed to be only for
					// people who are translating
					// and shouldn't be included
					// in a release

					// Remove SIGNAL_OE (non-terminal) before any other processing.

					x = VariableUtils::RemoveSignal(x);
				}

				std::string last_cout = "";
				std::uint64_t count_same = 0;
				bool cond = false;

				// Recursive variable replacement:
				// Continue replacing until:
				// - No VAR_* remain, OR
				// - All remaining VAR_* exist in the variable list, OR
				// - MAKE_*(...) patterns are fully resolved.
				// Hard limit: 1000 iterations to avoid infinite loops.

				do {
					// Do while is for "recursive variable replacing" (February 2024)
					for (auto i : variable_list) {
						// If it's found

						// Handle MAKE_* variables with arguments.
						// Example:
						// MAKE_HELLO(Mario)
						// vars_bak.txt contains: MAKE_HELLO(MY_ARG) = "Ciao, MY_ARG!"
						// → Replace MY_ARG with "Mario" before substitution.

						if (!Common::StringContains(x, i.first)) {
							if (!Common::StringContains(i.first, "MAKE_")) {
								continue;
							}
						}

						// If it contains <CLT
						// (remember: we're iterating through vars_bak
						// and vars_bak also contains CLTs)
						if (Common::StringContains(i.first, "<CLT")) {
							// then we don't care
							continue;
						}

						std::string bakx = x;

						bool const is_make = Common::StringContains(i.first, "MAKE_") && Common::StringContains(x, "MAKE_") && Common::StringContains(x, "(") && Common::StringContains(x, ")");
						if (is_make) {
							//std::cout << "Is make: " << x << " with i.first being \"" << i.first << "\"" << std::endl;
							std::size_t const find_make = x.find("MAKE_");
							if (find_make != std::string::npos) {
								std::size_t const find_bracket = x.find("(", find_make);
								if (find_bracket != std::string::npos) {
									std::string substr = x.substr(find_bracket + 1);
									std::size_t const end_bracket = substr.find(")"); // No second arg is correct
									if (end_bracket != std::string::npos) {
										substr = substr.substr(0, end_bracket);
										i.first = std::regex_replace(i.first, std::regex("MY_ARG"), substr);
										i.second = std::regex_replace(i.second, std::regex("MY_ARG"), substr);
										//std::cout << x << " --> \"" << i.first << "\" will become \"" << i.second << "\" (substr: " << substr << ")" << std::endl;

										x = StringUtils::ReplaceSubstring(x, i.first, i.second);
									}
								}
							}
						}
						else {
							// Replace it with its proper value
							x = StringUtils::ReplaceSubstring(x, i.first, i.second);
						}

						if (bakx != x) {
							// Maybe?
							bakx = std::regex_replace(bakx, std::regex("\n"), "");
							bakx = std::regex_replace(bakx, std::regex("\r"), "");
							x = std::regex_replace(x, std::regex("\n"), "");
							x = std::regex_replace(x, std::regex("\r"), "");

							// Why isn't this a struct/class????????
							SuperVarEntry sve{};
							std::get<0>(sve) = "[" + openfilename + "|" + std::to_string(cont) + "]";
							std::get<1>(sve) = bakx;
							std::get<2>(sve) = i.first;
							std::get<3>(sve) = i.second;
							std::get<4>(sve) = x;
							replace_map.push_back(sve);
						}
					}
					cond = (Common::StringContains(x, "VAR_") && AllVarsExist(variable_list, x)) || (Common::StringContains(x, "MAKE_") && Common::StringContains(x, "(") && Common::StringContains(x, ")"));
					if (cond) {
						if (x != last_cout) {
							//std::cout << "Still contains VAR_ in " << openfilename << ": " << x << std::endl;
							last_cout = x;
						}
						else {
							count_same++;
							// Hardcoded max recursive iterations
							if (count_same > 1000) {
								//std::cout << "I give up: " << openfilename << ": " << x << std::endl;
								break;
							}
						}
					}
				} while (cond);
			}

			// Maybe?
			x = std::regex_replace(x, std::regex("\n"), "");
			x = std::regex_replace(x, std::regex("\r"), "");

			// The .txt file for the game translation only needs
			// this data, so write it to the
			// stringstream which we're going to
			// use later

			{
				std::string const x2 = Distribution::CheckPlatforms(x, false);

				if (x2 != "SKIPTHISLINE") {
					ss << x2 << std::endl;
				}
			}

			// If we also want "baked" files
			if (baked) {
				x = RemoveSignal(x);
				// Then replace every single variable with its value
				// But this time we also remove the CLTs

				if (Configuration::ConfigMap["NoPlatformInBaked"]) {
					x = Distribution::CheckPlatforms(x, true);
					// Save the changes to the second stringstream for baked .txts
					if (x == "SKIPTHISLINE") {
						// TODO: cont++?
						continue;
					}
				}

				if (Configuration::ConfigMap["NoCLTInBaked"]) {
					for (auto const& i : variable_list) {
						bool const clt_found = Common::StringContains(i.first, "<CLT");
						if (clt_found) {
							x = std::regex_replace(x, std::regex(i.first), "");
						}
					}
				}

				ss2 << x << std::endl;
			}
		}
		in.close();

		// Output the game .txt and overwrite it

		if (ss.str().length() == 0) {
			LOG("ERROR: Something went *very* wrong! ss.str() has length 0!", HERE, "HydraulicPress");
		}

		auto outfile = std::filesystem::path(files_to_search.at(ftp));
		LOG("Saving " + outfile.string(), HERE, "HydraulicPress");
		std::ofstream out(outfile, std::ios::out | std::ios::binary);
		out << ss.str();
		out.flush();
		out.close();

		std::ofstream out_sve("var_replace_map.txt", std::ios::out | std::ios::app);
		for (SuperVarEntry const& sve : replace_map) {
			out_sve << std::get<0>(sve) << " \"" << std::get<1>(sve) << "\" --> (\"" << std::get<2>(sve) << "\" --> \"" << std::get<3>(sve) << "\") --> \""
				<< std::get<4>(sve) << "\"" << std::endl;
		}
		out_sve.flush();
		out_sve.close();

		if (baked) {
			// Output the baked .txt

			// Get the folder containing the txt
			std::string const folder = FileUtils::GetContainingFolder(files_to_search.at(ftp));
			//std::cout << "Folder containing the TXT: " << folder << " with file " << files_to_search.at(ftp) << " and index " << ftp << std::endl;

			std::filesystem::path const baked_path = std::filesystem::current_path() / "Baked" / folder;
			std::filesystem::create_directories(baked_path);
			std::filesystem::path const file_path = baked_path / openfile.filename();
			std::string const file_nodir = file_path.string();
			LOG("Saving baked " + file_nodir, HERE, "HydraulicPress");
			std::ofstream out2(file_nodir, std::ios::out | std::ios::binary);
			out2 << ss2.str();
			out2.flush();
			out2.close();
		}
	}

	LOG("Done replacing variables!", HERE, "HydraulicPress");
}