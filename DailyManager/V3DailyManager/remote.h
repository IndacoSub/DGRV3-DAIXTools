#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include "common.h"

namespace Remote {
	bool Clone(std::string const& repo, std::string const& repo_url, fsys::path const& current_dir,
		bool const& beta, std::vector<std::string> const& taken_branches);

	void UploadToCloud(fsys::path const& current_dir, std::string const& upload_repo, std::string const& info);
	void UpdateGitignore(fsys::path const& file_location, bool const& allow_txt, bool const& allow_patches, bool const& allow_spc);
	void UpdateReadme(fsys::path const& readme_location, fsys::path const& current_dir, std::string const& time, bool is_private);
	void UpdateReadmeEN(std::vector<std::string> const& percentages, fsys::path const& readme_file_en, std::string const& time, bool is_private);
	void UpdateReadmeIT(std::vector<std::string> const& percentages, fsys::path const& readme_file_it, std::string const& time, bool is_private);
}