// Team DAIX, 2026
// ADVENTURER

// The majority of this code was written between 2020 and 2022

// This tool's purpose is to:
// Clone and prepare the graphics repository (DGRV3-GFX / DGRV3-AB-GFX)
// for image patching workflows. It mirrors the behavior of Ropeway, but for image assets.

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
#include <cstring>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "vgit.h"
#include "vgit_utils.h"

#include "../Common/Config.h"
#include "../Common/Cloud.h"
#include "../Common/Common.h"

bool Clone(std::string const& repo, std::string const& repo_url, bool const& beta, std::vector<std::string> const& blacklist);

int main(int argc, char* argv[]) {

	//return 1;

	// This program does the same thing that "Ropeway" from the Text Tools does

	bool beta = true;

	if (argc > 1) {
		if (strcmp(argv[1], "--beta") == 0) {
			beta = true;
		}
	}

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const configfile = current_dir / "ImageConfig.config";

	LOG("Reading config", HERE, "Adventurer");
	Configuration::ReadConfig(configfile);
	Configuration::ViewDebugCurrentConfig();
	LOG("Done reading config", HERE, "Adventurer");

	// Repository name (and also folder)
	std::string repo{};

	// Select which graphics repository to clone depending on platform.
	// Switch uses AB-GFX, while PC/Xbox use the standard GFX repo.

	// Cloud::dl_repo_name determines which ZIP archive is downloaded when FileOnDemand is disabled.
	// It must match the platform-specific graphics repo on GitHub.
	if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
		repo = EncryptString("DGRV3-AB-GFX");
		Cloud::dl_repo_name = Cloud::dl_repo_name_switch;
	}
	else {
		repo = EncryptString("DGRV3-GFX");

		// Xbox|PC
		if (Configuration::ConfigMap["UseXboxConfiguration"]) {
			Cloud::dl_repo_name = Cloud::dl_repo_name_xbox;
		}
		else {
			// PC (Steam)
			Cloud::dl_repo_name = Cloud::dl_repo_name_pc;
		}
	}

	// Base files folder
	// Base folder name differs between Switch (base_ab) and other platforms (base_spc).
	// This folder contains the unmodified image assets used for patch generation.
	std::string basefolder{};
	if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
		basefolder = EncryptString("base_ab");
	}
	else {
		basefolder = EncryptString("base_spc");
	}

	std::filesystem::path const dgrv3path = current_dir / repo;

	// Basically login automatically
	// The credentials are stored in Common/Cloud.h
	// Construct authenticated Git URL using stored credentials.
	// VGit/VGitUtils use this URL for all clone operations.
	std::string const& repo_url = VGitUtils::CalculateRepoURL(Cloud::dl_repo_owner, Cloud::dl_repo_token, Cloud::dl_repo_password, Cloud::dl_repo_owner, repo);

	// This is a list of branches that we *don't* want to compile as beta
	// Branches that should never be treated as beta branches.
	// Prevents accidental merging or cloning of protected branches.

	std::vector<std::string> static const taken_branches{
		MAIN_BRANCH_2,
		"HEAD",
	};

	// Clone repositories
	// Clone the graphics repository (main + beta branches if enabled).
	// Beta cloning merges multiple branches into a single working tree.

	bool const& res = Clone(repo, repo_url, beta, taken_branches);
	if (!res) {
		return EXIT_FAILURE;
	}

	// If FileOnDemand is disabled, download the entire graphics repository as a ZIP.
	// This bypasses Git and uses GitHub's archive endpoint instead.

	if (!Configuration::ConfigMap["FileOnDemand"]) {

		LOG("Downloading SPC repository...", HERE, "Adventurer");

		// TODO: Why not clone using GitHub token?
		std::string command = "curl -s -H \"Authorization: token " + Cloud::dl_repo_token + "\" --retry 3 --retry-delay 1 -LO https://github.com/" +
			Cloud::dl_repo_owner + "/" + Cloud::dl_repo_name + "/archive/refs/heads/main.zip";

		// System or Common::ExecuteBatch?
		if (Common::executeBatch(command.c_str()) != 0) {
			LOG("Command failed: " + command, HERE, "Adventurer");
		}
	}

	LOG("\n", HERE, "Adventurer");
	LOG("Repository cloned!", HERE, "Adventurer");

	Common::WaitExit();
}

bool Clone(std::string const& repo, std::string const& repo_url, bool const& beta, std::vector<std::string> const& blacklist) {

	// merge_branches=true means beta cloning will merge multiple branches
	// into a single working directory instead of keeping them separate.

	constexpr static const bool merge_branches = true;

	// Get the current git version
	VGit::GetGitVersion();

	// Clones the main branch
	// Always clone the main branch first. Beta branches (if enabled) are merged on top of it.

	VGit::CloneRepositoryMain(repo_url);

	// If we want to use beta stuff
	// Attempt to clone and merge beta branches. If this fails, the tool writes vgit_failed.txt
	// so other tools in the pipeline can detect the failure and abort early.

	if (beta) {
		LOG("Trying beta cloning...", HERE, "Adventurer");
		// If beta failed
		if (!VGitUtils::CloneBeta(merge_branches, repo, repo_url, blacklist)) {
			LOG("\n", HERE, "Adventurer");
			LOG("ERROR: The repository couldn't be cloned successfully!", HERE, "Adventurer");
			LOG("\n", HERE, "Adventurer");

			// Save to file that it failed, so that we may exit early
			VGitUtils::SaveToFile("vgit_failed.txt", ":(");

			return false;
		}
	}

	// If main clone succeeded (and beta clone if enabled), return success.

	return true;
}