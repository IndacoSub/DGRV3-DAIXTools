// Team DAIX, 2026
// HYDRAULICPRESS — platform.h
//
// The majority of this code was written between 2020 and 2022
//
// This module's purpose is to:
// 1. Detect <PLATFORM_X> tags inside script lines
// 2. Compare them against the currently selected build platform
// 3. Remove platform tags when they match
// 4. Skip the entire line when they do not match
//
// Used to filter platform‑specific script content during compilation.

#pragma once

#include <string>
#include "../Common/Common.h"

namespace Distribution {
	// The active build platform (e.g., "PC", "SWITCH", "XBOX").
	// Set externally before calling CheckPlatforms().

	inline std::string Platform{};

	// Parses a line containing <PLATFORM_X> tags and returns:
	// - the cleaned line if X matches the active platform
	// - "SKIPTHISLINE" if it does not match
	// Supports multiple tags in a row (e.g., <PLATFORM_PC><PLATFORM_SWITCH>Text).

	inline std::string CheckPlatforms(std::string const& x, bool const forbaked) {
		std::string ret = x;

		// The objective of this function is to determine if the build platform is the same as the platform in the string
		// If it's not, we return "SKIPTHISLINE"
		// If it is, we're good and we just return the string without the <platform>

		// Check platforms
		// Prefix used to detect platform markers inside script lines.
		std::string const platform_prefix = "<PLATFORM_";
		// The exact tag for the active platform.
		// Example: "<PLATFORM_PC>"
		std::string const current_platform_str = platform_prefix + Distribution::Platform + ">";
		if (!ret.empty() && Common::StringContains(ret, platform_prefix)) {
			// Is it the platform we're trying to compile?
			std::size_t const platform_we_want = ret.find(current_platform_str);
			if (platform_we_want != std::string::npos) {
				std::size_t const just_one_platform = ret.find(platform_prefix, platform_we_want) == std::string::npos;
				if (just_one_platform) {
					// Case 1: Only one platform tag exists.
					// Example: "<PLATFORM_PC>MyString" → "MyString"
					ret = ret.substr(platform_we_want + current_platform_str.length());
				}
				else {
					// Case 2: Multiple platform tags exist.
					// Example:
					//   "<PLATFORM_PC><PLATFORM_SWITCH><PLATFORM_ANDROID>MyString"
					// If active platform = PC → remove all other tags.
					std::vector<std::size_t> platform_positions{};
					std::size_t last = 0;
					std::string const bak = ret;
					// Collect positions of all platform tags except the active one.
					// These will be removed later.

					for (std::uint64_t j = 0; j < (bak.length() / platform_prefix.length()) + 1; ++j) {
						last = bak.find(platform_prefix, last);
						if (last != platform_we_want && last != std::string::npos &&
							std::find(platform_positions.begin(), platform_positions.end(), last) == platform_positions.end()) {
							platform_positions.push_back(last);
						}
					}

					if (!platform_positions.empty()) {
						// Fixed: missing while

						// Strip platform tags from the left until only the active one remains.
						// Example:
						//   "<PLATFORM_PC><PLATFORM_SWITCH>MyString"
						// → "<PLATFORM_PC>MyString"
						// → "MyString"

						do {
							std::size_t const rightmost_end = bak.find(">");
							if (rightmost_end != std::string::npos) {
								// Ex. <PLATFORM_ANDROID>MyString
								// becomes MyString
								ret = bak.substr(rightmost_end + 1);
							}
							else {
								// Cannot make example
								ret = bak.substr(platform_we_want + current_platform_str.length());
								break;
							}
						} while (ret.starts_with("<PLATFORM_"));
					}
					else {
						// ??? (Cannot make example)
						// Fallback: if tag parsing fails, remove the active tag and return the remainder.

						ret = bak.substr(platform_we_want + current_platform_str.length());
					}
				}
			}
			else {
				// No matching platform tag found → this line does not belong to the active platform.

				ret = "SKIPTHISLINE";

			}

			if (!forbaked) {
				// Optional debug logging (disabled for baked builds).

				LOG("X: " + x + " - Found platform " + Distribution::Platform + ": returning " + ret, HERE, "Platform");
			}
		}

		return ret;
	}
}