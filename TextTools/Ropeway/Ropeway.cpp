// Team DAIX, 2026
// ROPEWAY

// The majority of this code was written between 2020 and 2022

// This tool's purpose is to:
// 1. Clone a GitHub repository
// 2. Copy the cloned folder into a new DGRV3_EN folder
// 3. "Clone" again, now the master/main branch + other stuff
// That's it.

// It makes use of the embedded "VGit" library (also by Team DAIX)
// Ropeway orchestrates cloning and preparing multiple branches of the DGRV3 repo.
// It relies on VGit/VGitUtils to perform all Git operations via shell commands.

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <map>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../Common/Config.h"
#include "../Common/Common.h"

#include "vgit.h"
#include "vgit_utils.h"

bool Clone(std::string const& repo, std::string const& repo_url, std::string const& repo_en, bool const& beta, std::vector<std::string> const& blacklist);

int main(int argc, char* argv[]) {

	// Force beta: allows you to merge branches that aren't in the backlist
	bool force_beta = false;

	std::vector<std::string> args(argv, argv + argc);

	// '--beta' overrides config and forces branch merging even if normally disabled.
	if (argc > 0) {
		if (std::any_of(args.begin(), args.end(), [](std::string const& arg) -> bool {return Common::StringContains(arg, "--beta"); })) {
			force_beta = true;
		}
	}

	// YOUR GitHub Username
	static const std::string gh_username = EncryptString(""); // Provide your own!

	// YOUR GitHub Password
	static const std::string gh_password = EncryptString(""); // Provide your own!

	// GitHub Token
	// Since 2022?, you can no longer sign in automatically with GitHub using only username and password
	// If you saw the real gh_token in the source code... no you didn't, alright?
	static const std::string gh_token = EncryptString(""); // Provide your own!

	// Cloning repo owner
	static const std::string gh_owner = EncryptString(""); // Provide your own!

	// **PRETTY MUCH THE WHOLE CODEBASE DEPENDS ON THE PRESENCE OF THE DGRV3 AND DGRV3_EN FOLDERS**
	
	// Repository name (and also folder)
	const std::string repo = EncryptString("DGRV3");

	// English branch folder
	const std::string repo_en = EncryptString("DGRV3_EN");

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const dgrv3path = current_dir / repo;
	std::filesystem::path const configfile = current_dir / "TextConfig.config";

	// REMEMBER TO INCREMENT THIS EVERY UPDATE
	LOG("Ropeway v2.6", HERE, "Ropeway");
	LOG("Changelog: Optimizations", HERE, "Ropeway");

	LOG("Reading config", HERE, "Ropeway");
	Configuration::ReadConfig(configfile);
	Configuration::ViewDebugCurrentConfig();
	LOG("Done reading config", HERE, "Ropeway");

	// Abort early if authentication details are missing; cloning cannot proceed.

	if (gh_username.empty() || gh_token.empty()) {
		LOG("ERROR: Empty credentials! Can't clone anything...", HERE, "Ropeway");
		VGitUtils::SaveToFile("vgit_failed.txt", ":(");
		return EXIT_FAILURE;
	}

	// Basically login automatically
	std::string const& repo_url = VGitUtils::CalculateRepoURL(gh_username, gh_token, gh_password, gh_owner, repo);

	// This is a list of branches that we *don't* want to compile as beta (so we completely ignore them)
	// Including MAIN_BRANCH and not MAIN_BRANCH_2 is fine
	// as a branch can only have one main branch
	// and not two, and the program later
	// checks the number of taken branches
	// so don't add the main/master branch
	// if it's already there
	std::vector<std::string> static const taken_branches{
		MAIN_BRANCH,
		"HEAD",
		"english",
		"french",
		"japanese",
		"switch-jp",
		"switch-en",
		"mobile",
		"xbox",
	};

	// Clone repositories
	// Perform the full cloning workflow: English branch → main branch → optional beta merge.
	bool const& res = Clone(repo, repo_url, repo_en, Configuration::ConfigMap["CloneBetas"] || force_beta, taken_branches);
	if (!res) {
		return EXIT_FAILURE;
	}

	LOG("\n", HERE, "Ropeway");
	LOG("Translated repository cloned!", HERE, "Ropeway");
	LOG("\n", HERE, "Ropeway");

	Common::WaitExit();
}

bool Clone(std::string const& repo, std::string const& repo_url, std::string const& repo_en, bool const& beta, std::vector<std::string> const& blacklist) {

	// I have no idea of merge_branches' purpose since we also have the "beta" in the arguments
	constexpr static const bool merge_branches = true;

	// The English branch is cloned first and used as the baseline for comparison.

	static const std::string compare_with = EncryptString("english");

	// Get the current git version
	VGit::GetGitVersion();

	// Clones the english branch first
	VGit::CloneRepositoryBranch(repo_url, compare_with);

	/*
	// Renames the "DGRV3" folder (which is the english now) to repo_en (DGRV3_EN)
	VGitUtils::RenameFolder(repo, repo_en);
	*/
	// Duplicate the English branch into a separate folder for later patching and comparison.


	VGitUtils::CopyFolder(repo, repo_en);

	// Switch the original clone to the main branch and update it before beta merging.

	System::ExecuteCD(repo, GIT_COMMAND_CHECKOUT + MAIN_BRANCH, false);
	System::ExecuteCD(repo, GIT_COMMAND_FETCH + " origin");
	System::ExecuteCD(repo, GIT_COMMAND_PULL);

	// Clones the main branch
	// VGit::CloneRepositoryMain(repo_url);

	// If we want to use beta stuff
	if (beta) {
		LOG("Trying beta cloning...", HERE, "Ropeway");
		// If beta failed
		if (!VGitUtils::CloneBeta(merge_branches, repo, repo_url, blacklist)) {
			LOG("\n", HERE, "Ropeway");
			LOG("The repository couldn't be cloned successfully!", HERE, "Ropeway");
			LOG("\n", HERE, "Ropeway");

			// Save to file that it failed, so that we may exit early
			VGitUtils::SaveToFile("vgit_failed.txt", ":(");

			return false;
		}
	}

	return true;
}