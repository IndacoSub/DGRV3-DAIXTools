// Team DAIX, 2026
// V3DAILYMANAGER — COMMON UTILITIES
//
// The majority of this code was written between 2020 and 2022
//
// These helpers provide:
//   • Cross‑platform command execution (executeBatch)
//   • Basic string utilities (contains / starts_with)
//   • Timestamp generation for logs and version identifiers (GetTime)
//
// DAILY uses these utilities to run external tools, generate build metadata,
// and perform simple text checks during automated build steps.

#include "common.h"

#include <iostream>
#include <sstream>
#include <chrono>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace Common {
	int executeBatch(const char* fullBatchFileName) {

		int ret = -1;

		std::ostringstream oss;

#ifdef _WIN32
		oss << '\"' << fullBatchFileName << '\"';
#else
		oss << fullBatchFileName;
#endif

		std::cout << "[DAILY] Executing command : " << oss.str() << std::endl;

#ifndef _WIN32
		ret = system(oss.str().c_str());
#else
		ret = system(oss.str().c_str());
#endif
		return ret;
	}

	bool StringContains(std::string const& str, std::string const& substring) {
#ifdef _WIN32
		return str.contains(substring);
#else
		return str.find(substring) != std::string::npos;
#endif
	}

	bool StringStartsWith(std::string const& str, std::string const& prefix) {
#ifdef _WIN32
		return str.starts_with(prefix);
#else
		return str.rfind(prefix, 0) == 0; // position 0 means prefix match
#endif
	}

	std::pair<std::string, std::string> GetTime(void) {

		auto const now = std::chrono::system_clock::now();
		auto const in_time_t = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;
		std::stringstream date_as_version;
#ifndef _DEBUG
		auto const local_time = std::localtime(&in_time_t);
		ss << std::put_time(local_time, "%Y/%m/%d %X");
		date_as_version << std::put_time(local_time, "%Y.%m.%d");
#else
		struct tm newtime;
		auto const local_time = localtime_s(&newtime, &in_time_t);
		auto newtime_2 = newtime;
		ss << std::put_time(&newtime, "%Y/%m/%d %X");
		date_as_version << std::put_time(&newtime_2, "%Y.%m.%d");
#endif

		std::pair<std::string, std::string> ret{};

		ret.first = ss.str();
		ret.second = date_as_version.str();

		return ret;
	}
}