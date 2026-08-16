// Team DAIX, 2026
// MONOTARO — FONTTOOLS REPOSITORY MANAGER

// The majority of this code was written between 2020 and 2022

// This tool's purpose is:
// Monotaro is the FontTools equivalent of Ropeway from TextTools. It orchestrates
// cloning, updating, and preparing multiple branches of the DGRV3-Font repository.
// It uses the embedded VGit and VGitUtils libraries (also by Team DAIX) to perform
// all Git operations through shell commands, including:
//
//   • Authentication via username/password/token
//   • Cloning the main branch of DGRV3-Font
//   • Optionally cloning beta branches
//   • Filtering out unwanted branches via a blacklist
//   • Writing vgit_failed.txt on cloning errors
//
// Monotaro is the first stage of the FontTools pipeline. All other FontTools
// components depend on the repository layout produced by Monotaro.

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <cstring>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "vgit.h"
#include "vgit_utils.h"

#include "../Common/Common.h"

bool Clone(std::string const& repo, std::string const& repo_url, bool const& beta, std::vector<std::string> const& blacklist);

int main(int argc, char* argv[]) {

	//return 1;

	// If the user passes "--beta", enable cloning of beta branches.
	// Otherwise, only the main branch is cloned.


	bool beta = true;

	if (argc > 1) {
		if (strcmp(argv[1], "--beta") == 0) {
			beta = true;
		}
	}

	// GitHub credentials for accessing private DGRV3-Font repositories.
	// These must be supplied by the user; Monotaro does not ship with any.
	// VGitUtils uses these to construct authenticated HTTPS URLs.

	// GitHub Username
	static const std::string gh_username = EncryptString(""); // Provide your own!

	// GitHub Password
	static const std::string gh_password = EncryptString(""); // Provide your own!

	// GitHub Token
	static const std::string gh_token = EncryptString(""); // Provide your own!

	// GitHub Repo Owner
	static const std::string gh_repo_owner = EncryptString(""); // Provide your own!

	// Repository name (and also folder)
	const std::string repo = EncryptString("DGRV3-Font");

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const dgrv3path = current_dir / repo;

	LOG("Monotaro v2.0", HERE, "Monotaro");

	// Basically login automatically
	// Construct authenticated GitHub URL using username/token/password.
	// This allows VGit to perform clone operations without prompting.

	std::string const& repo_url = VGitUtils::CalculateRepoURL(gh_username, gh_token, gh_password, gh_repo_owner, repo);

	// This is a list of branches that we *don't* want to compile as beta
	std::vector<std::string> static const taken_branches{
		MAIN_BRANCH_2,
		"HEAD",
	};

	// Clone repositories
	// Clone the main branch, and optionally beta branches.
	// Writes vgit_failed.txt on error so Monokuma can abort early.

	bool const& res = Clone(repo, repo_url, beta, taken_branches);
	if (!res) {
		return EXIT_FAILURE;
	}

	LOG("\n", HERE, "Monotaro");
	LOG("Repository cloned!", HERE, "Monotaro");

	Common::WaitExit();
}

bool Clone(std::string const& repo, std::string const& repo_url, bool const& beta, std::vector<std::string> const& blacklist) {

	// Get the current git version
	VGit::GetGitVersion();

	// Clones the main branch
	VGit::CloneRepositoryMain(repo_url);

	// If we want to use beta stuff
	if (beta) {
		// Clone beta branches if enabled.
		// CloneBeta() handles branch enumeration, filtering, and cloning.
		// Returns false if any beta branch fails to clone.

		if (!VGitUtils::CloneBeta(true, repo, repo_url, blacklist)) {
			// If beta failed
			LOG("\n", HERE, "Monotaro");
			LOG("ERROR: The repository couldn't be cloned successfully!", HERE, "Monotaro");
			LOG("\n", HERE, "Monotaro");

			// Save to file that it failed, so that we may exit early
			VGitUtils::SaveToFile("vgit_failed.txt", ":(");

			return false;
		}
	}

	return true;
}