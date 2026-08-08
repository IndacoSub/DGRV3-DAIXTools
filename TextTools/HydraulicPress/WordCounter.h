#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

using CountType = std::pair<std::string, std::uint64_t>;
using CountList = std::vector<CountType>;

namespace CountUtils {
	void CountWords(std::vector<std::string> const& files_to_search);
	unsigned long long GetAvailableRAM(void);
	double GetAvailableRAMInGB(void);
	long double CalculatePercentage(std::uint64_t const& number, std::uint64_t const& total);
}