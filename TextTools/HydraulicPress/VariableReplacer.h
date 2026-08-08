#pragma once

#include <vector>
#include <string>
#include <regex>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "../Common/Common.h"

namespace VariableUtils {
	void ReplaceVariables(std::vector<std::string> const& files_to_search, std::vector<VarEntry> const& variable_list, bool const& baked);
	std::string RemoveSignal(std::string const& full_string);
	bool VarExists(std::vector<VarEntry> const& variable_list, std::string const& single);
	bool AllVarsExist(std::vector<VarEntry> const& variable_list, std::string const& sentence);
}