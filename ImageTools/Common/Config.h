// Team DAIX, 2026
// CONFIG

// The majority of this code was written between 2020 and 2022

#pragma once

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <filesystem>

#include "Common.h"

#define MP(x, y) std::make_pair(x, y)

namespace Configuration {

	template<typename T1, typename T2>
	using MapType = std::map<T1, T2>;

	// String, Default
	// Global configuration flags shared across all ImageTools programs.
	// Values are loaded from *.config files and override these defaults.

	inline MapType<std::string, bool> ConfigMap{

		MP("FileOnDemand",				true),
		MP("UseSwitchConfiguration",	false),
		MP("UseXboxConfiguration",		false),

		/* MAID */

		MP("UseALTs", false),
		MP("MultithreadedCompilation", true),
	};

	inline void ReadConfig(std::filesystem::path const& config_file) {

		LOG("Reading configuration file: " + config_file.string(), HERE, "Common");
		// Read the configuration file
		if (std::filesystem::exists(config_file)) {
			std::ifstream inconfig(config_file.string(), std::ios::in);
			std::string temp{};
			std::uint64_t linecont = 0;
			while (std::getline(inconfig, temp)) {
				linecont++;
				if (temp.empty()) {
					continue;
				}

				if (temp.front() == '{') {
					continue;
				}

				if (temp.front() == '[') {
					continue;
				}

				if (temp.front() == '/') {
					continue;
				}

				std::size_t const find_equal = temp.find("=");
				if (find_equal == std::string::npos) {
					LOG("WARNING: No assignment found in :" + std::to_string(linecont), HERE, "Common");
					continue;
				}

				std::string parameter = temp.substr(0, find_equal - 1);
				std::string static const allowed_characters = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789";
				std::size_t const find_last_letter = parameter.find_last_of(allowed_characters);
				if (find_last_letter == std::string::npos) {
					LOG("WARNING: Couldn't read last letter in line: " + std::to_string(linecont) + " in parameter: \"" + parameter + "\"", HERE, "Common");
					for (auto const& ch : parameter) {
						if (allowed_characters.find(ch) == std::string::npos) {
							LOG("WARNING: Unrecognized character: \"" + std::to_string(ch) + "\"", HERE, "Common");
						}
					}
					continue;
				}

				parameter = parameter.substr(0, find_last_letter + 1);
				auto const cmit = ConfigMap.find(parameter);
				if (cmit != ConfigMap.end()) {
					std::string const value = temp.substr(find_equal + 1 + 1);
					ConfigMap[parameter] = value != "FALSE";
				}
				else {
					LOG("WARNING: Setting not recognised in the ConfigMap (found in file but not in map): \"" + parameter + "\"", HERE, "Common");
				}
			}

			LOG("Configuration file red!", HERE, "Common");
		}
		else {
			LOG("WARNING: The configuration file was not found.", HERE, "Common");
		}
	}

	inline void ViewDebugCurrentConfig() {

		auto const bts = [&](bool const& b) -> std::string {return b ? "true" : "false"; };

		LOG("\n", HERE, "Common");
		LOG("------------ CURRENT CONFIGURATION ------------", HERE, "Common");
		LOG("\n", HERE, "Common");
		for (auto const& config : ConfigMap) {
			LOG(config.first + ": " + bts(config.second), HERE, "Common");
		}
		LOG("\n", HERE, "Common");
		LOG("-----------------------------------------------", HERE, "Common");
		LOG("\n", HERE, "Common");
	}
}