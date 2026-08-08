#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <fstream>
#include <iostream>
#include "../Common/Common.h"

namespace FileUtils {
	bool IsFileGood(std::string const& file);
	std::string GetInputFile(std::filesystem::path const& backup_path);
	std::vector<std::string> ReadDifferentHashesFile(std::string const& filename);
	std::vector<VarEntry> ReadInputFile(std::string const& input_file);
	std::vector<std::string> GetFilesToSearch(std::filesystem::path const& search_path);
	std::string GetContainingFolder(std::string const& full_path);
}