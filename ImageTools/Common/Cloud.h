// Team DAIX, 2026
// CLOUD

// The majority of this code was written between 2020 and 2022

#pragma once

#include <string>

// Cloud configuration: credentials and repository names used for authenticated
// GitHub downloads and cloning. These values are user-specific and must be set
// manually before running ImageTools.

namespace Cloud {
	inline std::string const dl_repo_owner = "";				// CONFIDENTIAL, put your own
	inline std::string const dl_repo_password = "";				// CONFIDENTIAL, put your own
	inline std::string const dl_repo_token = "";				// CONFIDENTIAL, put your own
	inline std::string const dl_repo_name_pc = "";				// CONFIDENTIAL, put your own
	inline std::string const dl_repo_name_switch = "";			// CONFIDENTIAL, put your own
	inline std::string const dl_repo_name_xbox = "";			// CONFIDENTIAL, put your own
	inline std::string const dl_branch = "main";					// CONFIDENTIAL, put your own (master/main/etc.)
	inline std::string dl_repo_name{};
};