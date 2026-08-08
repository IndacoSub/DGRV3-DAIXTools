// Team DAIX, 2026
// HYDRAULICPRESS — VariableChecker
//
// The majority of this code was written between 2020 and 2022
//
// This module's purpose is to:
// 1. Scan all script files for occurrences of VAR_* variables
// 2. Compare each found variable against the known variable list (from vars.txt / vars_bak.txt)
// 3. Identify variables used in the translation but missing from the variable list
// 4. Record every location where each missing variable appears
// 5. Output a detailed report listing all untranslated variables and their occurrences
//
// Used to detect forgotten or newly introduced variables that must be added to the variable list.

#include "VariableChecker.h"
#include "StringUtils.h"
#include "Platform.h"

#include <regex>
#include <tuple>

namespace VariableChecker {
	void CheckVariables(std::vector<VarEntry> const& variables, std::vector<std::string> const& files_to_search) {

		// The purpose of this function is to identify any and all
		// variables in the translation that have not been added
		// to the list of variables (represented as the "variables" 
		// argument)... yet, and save a report

		// Stores variables found in script files that do NOT exist in the known variable list.
		// Each UV contains:
		//   - UntranslatedVariable: the missing VAR_* name
		//   - WhereIsIt: list of (filename, raw line, line number)


		std::vector<UV> untranslated_variables{};
		// Only look through a small number of files, if possible

		// Iterate through all files to search for VAR_* occurrences.
		// Each file is processed line-by-line.

		for (auto const& j : files_to_search) {
			// Open each single file individually
			std::ifstream in(j, std::ios::in);
			std::string temp{};
			std::uint64_t cont = 0;
			// Iterate through strings in that file
			while (std::getline(in, temp)) {
				++cont;
				// No VAR_, we don't care
				// Skip lines that do not contain any variable.
				// Only lines containing VAR_* are relevant.

				if (!Common::StringContains(temp, "VAR_")) {
					continue;
				}
				// There is a VAR somewhere
				std::string const my_raw_str = temp;
				// Check if it's an appropriate platform (if not, then it's SKIPTHISLINE)
				// Apply platform filtering.
				// If the line belongs to a different platform, CheckPlatforms() returns "SKIPTHISLINE".

				temp = Distribution::CheckPlatforms(temp, true);
				// Replace characters
				// Normalize the line by removing escape sequences, punctuation, and symbols.
				// This helps isolate VAR_* tokens cleanly.

				temp = StringUtils::ReplaceSubstring(temp, "\\n", "\r\n");
				temp = std::regex_replace(temp, std::regex("\r\n"), " ");
				temp = std::regex_replace(temp, std::regex("\r"), " ");
				temp = std::regex_replace(temp, std::regex("/[!@#$%^&*:,—()]/g"), " ");
				temp = StringUtils::ReplaceSubstring(temp, "<", " ");
				temp = StringUtils::ReplaceSubstring(temp, ">", " ");
				temp = StringUtils::ReplaceSubstring(temp, "\"", " ");
				temp = StringUtils::ReplaceSubstring(temp, "'", " ");
				// Did the checks above actually filter this line? Let's see
				if (temp.empty() || temp == "SKIPTHISLINE" || temp.find_first_not_of(" ") == std::string::npos) {
					// Yup, invalid line because it's either empty or for another platform
					// If the line is empty or filtered out by platform rules, skip it.

					continue;
				}
				// Still a valid line, apparently
				// Get all variables in this line, individually
				// Will contain all VAR_* tokens found in this line.
				// Example: "Hello VAR_CAT and VAR_DOG" → ["VAR_CAT", "VAR_DOG"]
				std::vector<std::string> variables_in_this_line{};
				// Count hor many they are
				std::uint64_t const how_many_in_this_line = StringUtils::CountOccurrences(temp, "VAR_");
				if (how_many_in_this_line <= 0) {
					// None found (probably got filtered by all ReplaceSubstring-s above)
					continue;
				}

				// It's likely there's only one variable for each line
				// I mean, unless it's for the investigation, at least

				if (how_many_in_this_line == 1) [[likely]] {
					// Just one
					// 
					// Case 1: Only one VAR_* in the line.
					// Extract it by finding VAR_ and reading until the next punctuation/symbol.
					// 
					// What's its position?
					std::size_t const find_var = temp.find("VAR_");
					// We don't care about anything that's before the variable
					temp = temp.substr(find_var);
					// There isn't really a way to know when a variable is done
					// So just... check every character that could "realistically" be after a variable?
					std::size_t const end_of_first_one = temp.find_first_of("\"\'.,—!<>()?:;- \n\r");
					// We don't care about anything that's after the variable
					temp = temp.substr(0, end_of_first_one);
					// Add the variable to the vector
					variables_in_this_line.push_back(temp);
				}
				else {
					// More than 1
					// We're going to add them to a vector, one by one

					// Case 2: Multiple VAR_* occurrences.
					// Loop through the line, extracting each VAR_* token individually.
					// Ensure duplicates in the same line are not added twice.

					std::size_t last = 0;// std::string::npos;
					std::size_t debug_count = 0;
					std::size_t this_one_begin = 0;
					bool ok = true;
					while (ok) {
						// Is last 0 or less?
						if (last <= 0) {
							// This can only happen the first time, I'm assuming
							// If it can find VAR_, which it should because it IS the first time, then good
							this_one_begin = temp.find("VAR_");
							ok = this_one_begin != std::string::npos;
						}
						else {
							// Look for VAR_ after the last position
							this_one_begin = temp.find("VAR_", last);
							ok = this_one_begin != std::string::npos;
						}

						// If there isn't any VAR_ left, we're done
						if (!ok) {
							break;
						}
						// Go to VAR_, we don't care about anything before that
						std::string this_one = temp.substr(this_one_begin);
						// Look for anything that isn't text or _ that could "realistically" be after a variable

						// Determine where the variable ends by scanning for punctuation or whitespace.
						// Example: "VAR_CAT)" → end at ')'

						std::size_t const this_one_end = this_one.find_first_of("\"\'.,—!<>():?;- \n\r");

						// We don't care about anything after this one variable
						// as we're still going to find the next one in the upcoming cycle
						// so let's do this one by one
						this_one = this_one.substr(0, this_one_end);

						// Set the last position to the current position of this very VAR_, but +1
						// so it doesn't get picked up twice
						last = this_one_begin + 1;

						// If the variable is already present in the vector, don't add it
						// Otherwise, add it (only add unique ones)
						if (std::find(variables_in_this_line.begin(), variables_in_this_line.end(), this_one) == variables_in_this_line.end()) {
							variables_in_this_line.push_back(this_one);
						}
						else {
							//std::cout << this_one << " already present!" << std::endl;
						}
						debug_count++;
						//std::cout << debug_count << "/" << how_many_in_this_line << " done: " << this_one << std::endl;
					}
				}

				// Hooray, now we have a list of every single variable in the line

				for (auto const& found_variable : variables_in_this_line) {
					// Look for it in the list of variables
					// Could be an idea to implement a cache of "good" variables
					// TODO: Implement a cache of "good" variables (what does that even mean?)

					// Remember: we're iterating through a vector of variables (string definition/name)

					// Skip special tokens that look like variables but are not real VAR_* entries.
					// Example: "<SIGNAL_OE>" or "MAKE_SOMETHING" → ignored.

					if (Common::StringContains(found_variable, "CLT")) {
						continue;
					}

					if (Common::StringContains(found_variable, "SIGNAL")) {
						continue;
					}

					if (Common::StringContains(found_variable, "MAKE_")) {
						continue;
					}

					if (Common::StringContains(found_variable, "PLATFORM")) {
						continue;
					}

					if (Common::StringContains(found_variable, "PAD")) {
						continue;
					}

					if (Common::StringContains(found_variable, "DIG")) {
						continue;
					}
					
					bool is_found = false;

					// "variables" is an argument of this function
					// It is NOT the vector we're iterating
					// The vector we're iterating is "variables_in_this_line"
					// and the single item iterated is found_variable
					// We want to know if found_variable, a variable from the array, actually exists... I think?
					// Only those that DO NOT exist in the "variables" are the one we actually care about

					// Check if the found variable exists in the known variable list.
					// If found → ignore it.
					// If NOT found → it is an untranslated variable.

					for (auto const& variable_from_list : variables) {
						std::string const raw_variable = variable_from_list.first;

						is_found |= found_variable == raw_variable;
						// It's an OR, so if it's true, it stays true
						if (is_found) {
							break;
						}
					}

					if (!is_found) {
						// Is it already inside the untranslated vector?
						bool is_inside_vec = false;
						// Remember! We're still iterating
						// This is the position of the variable in the array
						std::uint64_t pos = 0;

						// Add the variable to the untranslated list.
						// If it already exists, append a new occurrence.
						// Otherwise, create a new UV entry.


						for (auto const& untranslated_ones : untranslated_variables) {
							is_inside_vec |= found_variable == untranslated_ones.UntranslatedVariable;
							// Once again, it's an OR
							if (is_inside_vec) {
								break;
							}
							pos++;
						}

						if (is_inside_vec) {
							VariableChecker::Position const position = VariableChecker::Position(j, my_raw_str, cont);
							// If it's already in the array, add it to the instances of the same variable
							untranslated_variables[pos].WhereIsIt.push_back(position);
						}
						else {
							// If it's not in the array, add a new variable to the array with one single instance so far (this one)
							VariableChecker::Position const position = VariableChecker::Position(j, my_raw_str, cont);
							UV new_one{};
							new_one.UntranslatedVariable = found_variable;
							new_one.WhereIsIt.push_back(position);
							untranslated_variables.push_back(new_one);
						}
					}
					else {
						//std::cout << found_variable << " already known!" << std::endl;
					}
				}
			}
		}

		LOG("Saving to file...", HERE, "HydraulicPress");

		// Generate a detailed report listing each missing variable and all its occurrences.
		// variablechecker.txt → full report
		// variablechecker_short.txt → only variable names


		std::stringstream ss{};
		for (auto const& uvar : untranslated_variables) {
			ss << "\"" << uvar.UntranslatedVariable << "\" NOT FOUND in the list! It can be found here:" << std::endl;
			for (auto const& places : uvar.WhereIsIt) {
				ss << "\t\t\t" << "Line " << places.Line << " of " << Common::ShortenFilename(places.Filename, 2) << std::endl;
				if (uvar.UntranslatedVariable != places.ActualString) {
					// HUH?!
					ss << "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t" << places.ActualString << std::endl;
				}
			}

			// TODO: why 23?
			// Yeah, why 23?

			// Separator line between variable entries.
			// (Historical magic number: 23 repetitions of "----------")

			for (int j = 0; j < 23; j++) {
				ss << "----------";
			}
			ss << std::endl;
		}

		std::ofstream ofs_out("variablechecker.txt", std::ios::out);
		ofs_out << ss.str() << std::endl;
		ofs_out.close();

		std::ofstream ofs2_out("variablechecker_short.txt", std::ios::out);
		for (auto const& uvar : untranslated_variables) {
			ofs2_out << uvar.UntranslatedVariable << std::endl;
		}
		ofs2_out.close();
	}
}