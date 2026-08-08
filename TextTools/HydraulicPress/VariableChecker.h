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

#include "../Common/Common.h"

namespace VariableChecker {
	struct Position {
		std::string Filename{};
		std::string ActualString{};
		std::uint64_t Line{};

		explicit Position(std::string const& fn, std::string const& as, std::uint64_t const& ln)
			: Filename(fn), ActualString(as), Line(ln) {

		}
	};

	struct UV {
		std::string UntranslatedVariable{};
		std::vector<Position> WhereIsIt{};
	};

	void CheckVariables(std::vector<VarEntry> const& variables, std::vector<std::string> const& files_to_search);
}