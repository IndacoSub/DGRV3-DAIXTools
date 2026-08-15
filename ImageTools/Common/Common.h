// Team DAIX, 2026
// COMMON

// The majority of this code was written between 2020 and 2022

#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <cstring>
#include <numeric>

#ifdef _WIN32
#define CPP20
#endif

#ifdef CPP20
#include <source_location>
#define HERE std::source_location::current()
#else
#define HERE std::string{__FILE__}
#endif

#define MV(x, y) std::make_pair(x, y)
#define EncryptString(x) x

#define LOG(x, y, z) Common::DoLog(x, y ,z)

namespace Common {

	// Unified logging function used by all tools. Writes to imagetools_log.txt
	// and prints to stdout. Includes file/line/function info when C++20 is available.

#ifdef CPP20
	inline void DoLog(std::string const& Message, const std::source_location Where, std::string const& Project) {

		std::stringstream ss{};
		if (Message == "\n" || Message.length() == 0) {
			ss << Message << std::endl;
		}
		else {
			ss << Project
				<< " | "
				<< Where.file_name() << " | "
				<< Where.line() << " | "
				<< Where.column() << " | "
				<< Where.function_name() << " | "
				<< Message << std::endl;
		}
		std::cout << Message << std::endl;
		std::ofstream logout("imagetools_log.txt", std::ios::out | std::ios::app);
		logout << ss.str();
		logout.close();
	}
#else
	inline void DoLog(std::string const& Message, std::string const& Where, std::string const& Project) {
		std::stringstream ss{};
		if (Message == "\n" || Message.length() == 0) {
			ss << Message << std::endl;
		}
		else {
			ss << Project << " | " << Where << " | " << Message << std::endl;
		}
		std::cout << Message << std::endl;
		std::ofstream logout("imagetools_log.txt", std::ios::out | std::ios::app);
		logout << ss.str();
		logout.close();
	}
#endif

	inline int executeBatch(const char* fullBatchFileName, std::vector<std::string> const& arguments = {}, bool silent = false) {

		std::ostringstream oss{};

#ifdef _WIN32
		std::string batch = '\"' + std::string{ fullBatchFileName } + '\"';
#else
		std::string batch = std::string{ fullBatchFileName };
#endif
		if (!arguments.empty()) {
#ifdef CPP20
			std::string arg_s = std::accumulate(arguments.begin(), arguments.end(), std::string{});
			batch += arg_s;
#else
			for (auto const& j : arguments) {
				batch += j;
			}
#endif
		}

		if (!silent) {
			LOG("\n", HERE, "Common");
			LOG("[ImageTools] Executing command: " + batch, HERE, "Common");
			LOG("\n", HERE, "Common");
		}

		oss << batch;

		// "system" is synchronous, not asynchronous
		return system(oss.str().c_str());
	}

	inline void WaitExit(void) {

		static constexpr int WaitSeconds = 3;

		LOG("\n", HERE, "Common");
		LOG("Exiting in " + std::to_string(WaitSeconds) + " seconds...", HERE, "Common");
		LOG("\n", HERE, "Common");
		std::this_thread::sleep_for(std::chrono::seconds(WaitSeconds));
	}

	// Utility for shortening long paths in logs by keeping only the last N components.

	inline std::string ShortenFilename(std::string const& filename, int slashes) {

		std::string ret{};
		std::vector<std::string> paths{};
		std::filesystem::path p(filename);
		paths.push_back(p.filename().string());
		for (int i = 0; i < slashes; i++) {
			p = p.parent_path();
			paths.push_back(p.filename().string());
		}
		std::reverse(paths.begin(), paths.end());
		for (auto const& str : paths) {
			ret += str;
			ret += "/";
		}
		ret.pop_back();

		return ret;
	}

	// Cross-platform substring check. Uses std::string::contains on Windows,
	// fallback to find() on other platforms.

	inline bool StringContains(std::string const& str, std::string const& substr) {

#ifdef _WIN32
		return str.contains(substr);
#else
		return str.find(substr) != std::string::npos;
#endif
	}
}