#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace StringUtils {
	std::string TrimDirectories(std::string const& long_filename);
	std::string TrimStr(std::string const& full_string, std::string const& what_to_cut);
	bool IsAlphanumericString(std::string const& word);
	std::vector<std::string> SplitByCharacter(std::string str, char const& delim, bool const& remove_newlines);
	std::uint64_t CountOccurrences(std::string const& full_string, std::string const& substring);
	std::string ReplaceSubstring(std::string const& full_string, std::string const& substringl, std::string const& replace_with);
	std::string CreateRandomWeakAgree(std::string const& my_str, std::string const& toput);
	std::string ToLower(std::string const& str);
}