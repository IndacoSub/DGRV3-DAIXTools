#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "common.h"

namespace Files {
	std::vector<std::string> GetPercentages(fsys::path const& current_dir);
	int IsSTDEqualSame(const std::string& p1, const std::string& p2);
	bool IsInternallySame(const std::string& p1, const std::string& p2);
	void CopyRecursive(const fsys::path& src, const fsys::path& target) noexcept;
	void CopyFilesWithFilter(const fsys::path& cur, const fsys::path& src, const fsys::path& target, std::string const& filetype, bool recursive) noexcept;
	fsys::path AppendFolder(fsys::path const& mypath, std::string const& str);
	void DeleteFolder(std::string const& folder);
	bool CheckDifferent(fsys::path const& build_fd, std::vector<std::string> const& files_vec);
	std::uint64_t CountFilesInPath(fsys::path const& where);

	void DeleteDaily(std::string const repo_name);
	void DeleteEverything(fsys::path const& where, fsys::path const& current_dir);
	void DeleteEverything2(fsys::path const& current_dir);
	void DeleteUPSNormals(fsys::path const& where);
}