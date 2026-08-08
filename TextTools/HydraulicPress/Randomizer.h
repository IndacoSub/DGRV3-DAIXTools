#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <random>
#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Randomizer {
	struct Special {
		std::string Filename;
		std::uint32_t Line;
		bool HasWeak = false;
		bool HasAgree = false;
	};

	void Randomize(std::vector<std::string> const& files_to_search);
	std::string GetRandomString(void);
	std::uint64_t GetRand(std::uint64_t const& min, std::uint64_t const& max);
	std::string HandleSpecials(std::string const& my_line, std::vector<Special> const& specials, std::string const& filename, int const& line);
}