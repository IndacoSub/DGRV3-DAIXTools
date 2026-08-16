#pragma once

#include <string>
#include <filesystem>
#include <utility>
#include <vector>

#define fsys std::filesystem

#define EncryptString(x) std::string{x}

//#define ALL_PLATFORMS

// Remember: YOLO = no cloud/remote upload

//#define YOLO

enum class Platform {
	None,
	PC,	// (pc; legacy/steam)
	Switch, // (console; anniversary)
	Xbox, // (pc; anniversary)
	PSVita, // unsupported
	PS4, // unsupported
	Android, // unsupported
	iOS, // unsupported
	Num,
};

inline bool private_repo = true;
inline bool all_platforms = false;
inline bool no_text = false;
inline bool no_gfx = false;
inline bool no_font = false;
inline bool no_cloud = false;
inline bool safe_mode = true;
inline bool has_gui = false;
inline std::string specific_text_config = "";
inline std::string specific_gfx_config = "";
inline std::string specific_font_config = "";
inline std::vector<Platform> build_platforms{};

namespace Common {
	std::pair<std::string, std::string> GetTime(void);
	int executeBatch(const char* fullBatchFileName);
	bool StringContains(std::string const& str, std::string const& substring);
	bool StringStartsWith(std::string const& str, std::string const& prefix);
}