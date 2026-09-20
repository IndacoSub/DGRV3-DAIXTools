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
// Used to filter platform-specific script content during compilation.

#pragma once

#include <string>
#include <vector>
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
			std::size_t const first_platform_pos = ret.find(platform_prefix);

			if (first_platform_pos == std::string::npos) {
				return ret;
			}

			// Find and process all consecutive platform tags.
			// Example:
			//   "<PLATFORM_PC><PLATFORM_SWITCH>Text"
			//
			// The entire consecutive platform-tag group belongs to the same line.
			// The line is valid if at least one of those platforms matches the
			// currently selected build platform.

			std::size_t cursor = first_platform_pos;
			std::size_t content_start = std::string::npos;
			bool found_current_platform = false;
			bool malformed_platform_tag = false;

			while (cursor != std::string::npos && cursor < ret.length()) {

				// Make sure the current cursor actually points at a platform tag.
				if (ret.compare(cursor, platform_prefix.length(), platform_prefix) != 0) {
					break;
				}

				std::size_t const tag_end = ret.find(">", cursor);

				if (tag_end == std::string::npos) {
					malformed_platform_tag = true;
					break;
				}

				std::string const platform_tag = ret.substr(
					cursor,
					tag_end - cursor + 1
				);

				if (platform_tag == current_platform_str) {
					found_current_platform = true;
				}

				cursor = tag_end + 1;

				// The next character is not another platform tag.
				if (ret.compare(cursor, platform_prefix.length(), platform_prefix) != 0) {
					content_start = cursor;
					break;
				}
			}

			if (malformed_platform_tag) {
				// Malformed platform tag: safest behavior is to skip the line.
				ret = "SKIPTHISLINE";
			}
			else if (found_current_platform) {
				// Remove all consecutive platform tags while preserving any text
				// before the first platform marker.
				//
				// Example:
				//   "<PLATFORM_PC><PLATFORM_XBOX>V3_BGM_037"
				// → "V3_BGM_037"

				if (content_start != std::string::npos) {
					ret = ret.substr(0, first_platform_pos) + ret.substr(content_start);
				}
				else {
					// Fallback: if tag parsing fails, remove the active tag and return the remainder.
					ret = ret.substr(first_platform_pos + current_platform_str.length());
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