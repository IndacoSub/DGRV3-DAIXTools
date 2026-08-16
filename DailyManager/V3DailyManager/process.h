#pragma once

#include <filesystem>

#include "common.h"

namespace Process {

	bool RunTextTools(fsys::path const& where, fsys::path const& current_dir, Platform const& p);
	bool RunImageTools(fsys::path const& where, fsys::path const& current_dir, Platform const& p);
	bool RunFontTools(fsys::path const& where, fsys::path const& current_dir, Platform const& p);
	std::string GetPlatformName(Platform const& p);
}