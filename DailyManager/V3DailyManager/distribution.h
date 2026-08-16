#pragma once

#include "common.h"

#include <string>
#include <vector>

namespace Distribution {
	void DistributeFiles(Platform const& p, fsys::path const& build_fd, fsys::path const& cur,
		fsys::path const& img_dir, fsys::path const& font_dir, fsys::path const& tools_loc,
		fsys::path const& distfolder, fsys::path const& bakedfold, fsys::path const& baked_destination,
		fsys::path const& copy_log_to, fsys::path const& find_spc, fsys::path const& copy_spc_to,
		fsys::path const& find_spc_gfx, fsys::path const& find_spc_font);
	bool IsUnsupportedPlatform(Platform const& p);
}