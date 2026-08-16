// Team DAIX, 2026
// VGITUTILS

// The majority of this code was written between 2020 and 2022

// Utility functions for interacting with Git through external commands.
// Provides cloning, branch listing, merging, and filesystem helpers.

#pragma once

#include "vgit.h"
#include "../Common/common.h"

#ifdef _WIN32
#include <Windows.h>
#else
// Define fsys as filesystem?
// Non‑Windows platforms rely on std::filesystem (aliased as fsys in vgit.h).

#endif

#include <string>
#include <fstream>

namespace VGitUtils {
	inline bool CloneBeta(bool const& merge_branches, std::string const& repo, std::string const& repo_url, std::vector<std::string> const& blacklist = {});
	inline std::string CalculateRepoURL(std::string const& username, std::string const& token, std::string const& password, std::string const& repo_account_name, std::string const& repo_name);
	inline std::string GetActualRepoURL(std::string const& username, std::string const& repo_name);
	inline std::vector<std::string> CalculateBranches(std::string const& repo);
	inline void CopyFolder(std::string const& old_name, std::string const& new_name);
	inline void DeleteDotGitFolder(void);
	inline void DeleteFolder(std::string const& folder_name);
	inline void DoDeleteFile(std::string const& file_name);
	inline void DoMergeBranches(std::vector<std::string> const& beta_branches, std::string const& repo, std::string const& beta_branch, std::string const& repo_url);
	inline void ListBranches(std::string const& repo, bool save_results, std::string const& filename = "");
	inline void RenameFolder(std::string const& old_name, std::string const& new_name);
	inline void SaveToFile(std::string const& file_name, std::string const& content);
}

namespace VGitUtils {
	// Runs 'git branch' and optionally redirects output to a file.
	// Used to enumerate all branches in a repository.

	inline void ListBranches(std::string const& repo, bool save_results, std::string const& filename) {

		std::string command = GIT_COMMAND_LIST_BRANCHES;
		if (save_results) {
			command += " > ";
			command += filename;
			command += " 2>&1"; // Redirect stderr to stdout so branch output and errors go into the same file.

		}

		System::ExecuteCD(repo, command);
	}

	inline std::string GetActualRepoURL(std::string const& username, std::string const& repo_name) {

		std::string ret = "https://github.com/";
		ret += username;
		ret += "/";
		ret += repo_name;
		return ret;
	}

	// Builds an authenticated HTTPS GitHub URL using a personal access token.
	// Password-based authentication is deprecated, so token is used instead.

	inline std::string CalculateRepoURL(std::string const& username, std::string const& token, std::string const& password, std::string const& repo_account_name, std::string const& repo_name) {

		if (username.empty()) {
			std::cout << "Empty username." << std::endl;
		}

		if (token.empty()) {
			std::cout << "Empty token." << std::endl;
		}

		/*
		std::string ret = "https://";
		ret += username;
		ret += ":";
		ret += password;
		ret += "@github.com/";
		ret += repo_account_name;
		ret += "/";
		ret += repo_name;
		return ret;
		*/

		// New: Github password authentication was removed on August 13, 2021.
		// This now requires to use a token instead of a password
		// but if they change their mind later,
		// I'm still keeping the old code here

		std::string ret = "https://";
		ret += token;
		ret += ":";
		ret += "@github.com/";
		ret += username;
		ret += "/";
		ret += repo_name;
		return ret;
	}

	// Renames a folder using platform-specific APIs.
	// Windows uses MoveFileA; other platforms use filesystem::rename.


	inline void RenameFolder(std::string const& old_name, std::string const& new_name) {

		std::cout << std::endl;
		std::cout << "Renaming folder..." << std::endl;
		std::cout << std::endl;

#ifdef _WIN32
		MoveFileA(LPCSTR(old_name.c_str()), LPCSTR(new_name.c_str()));
#else
		fsys::rename(old_name, new_name);
#endif
	}

	// Recursively copies a folder, creating the destination directory first.

	inline void CopyFolder(std::string const& old_name, std::string const& new_name) {

		std::cout << std::endl;
		std::cout << "Copying folder..." << std::endl;
		std::cout << std::endl;

		fsys::create_directories(new_name);

		fsys::copy(old_name, new_name, fsys::copy_options::recursive | fsys::copy_options::overwrite_existing);
	}

	// Parses branches.txt produced by ListBranches().
	// Each line contains a prefix before the actual branch name.

	inline std::vector<std::string> CalculateBranches(std::string const& repo) {

		std::vector<std::string> branches{};
		std::ifstream in(repo + "/branches.txt", std::ios::in);
		std::string line{};
		while (std::getline(in, line)) {
			std::string s = line;
			// Why 9? If somebody knows, please make a PR
			s = s.substr(9);
			branches.push_back(s);
		}
		in.close();

		return branches;
	}

	inline void DeleteFolder(std::string const& folder_name) {

#ifdef _WIN32
		std::string const command = "rmdir /s /q " + folder_name;
#else
		std::string const command = "rm -rf " + folder_name;
#endif

		System::Execute(command);
	}

	inline void DeleteDotGitFolder(void) {
		VGitUtils::DeleteFolder(".git");
	}

	inline void DoDeleteFile(std::string const& file_name) {
#ifdef _WIN32
		DeleteFileA(LPCSTR(file_name.c_str()));
#else
		fsys::remove(file_name);
#endif
	}

	// Appends a line of text to a file, creating it if necessary.

	inline void SaveToFile(std::string const& file_name, std::string const& content) {

		std::ofstream of(file_name, std::ios::out | std::ios::app);
		if (!fsys::exists(file_name)) {
			of << "";
		}
		of << content << std::endl;
		of.close();
	}

	// Attempts to merge multiple branches into the repository.
	// Detects merge conflicts by scanning Git output and falls back to cloning
	// a single conflict-free branch if necessary.

	inline void DoMergeBranches(std::vector<std::string> const& beta_branches, std::string const& repo, std::string const& beta_branch, std::string const& repo_url) {

		// For every beta branch
		for (auto const& bb : beta_branches) {
			// Create a new file called merge_ + the name of the branch
			std::string const resultfile = "merge_" + bb;
			std::cout << "Merging branch \"" << bb << "\"..." << std::endl;
			// Try merging
			VGit::Merge(repo, bb, resultfile);
			// Create an ifstream to read resultfile
			std::ifstream in(fsys::path(repo) / resultfile, std::ios::in);
			std::string line{};
			bool found_conflicts = false;
			// For every line
			while (std::getline(in, line)) {
				// Scan merge output for the word "CONFLICT" to detect failed merges.

				// If 'CONFLICT' is found
				if (Common::StringContains(line, "CONFLICT")) {
					// Save the error to conflicts.txt so that the user
					// knows that something went wrong
					found_conflicts = true;
					std::string const msg = "Merging branch \"" + bb + "\": " + line;
					SaveToFile("conflicts.txt", msg);
					break;
				}
			}
			in.close();
			// Delete the (now useless) resultfile
			VGitUtils::DoDeleteFile((fsys::path(repo) / resultfile).string());
			// If conflicts were found
			if (found_conflicts) {
				std::cout << std::endl;
				std::cout << "Conflicts were found, only cloning the first \"beta\" branch: " << beta_branch << std::endl;
				std::cout << std::endl;

				// Delete the main folder and clone the first beta_branch without conflicts
				VGitUtils::DeleteFolder(repo);
				VGit::CloneRepositoryBranch(repo_url, beta_branch);
				break;
			}
			else {
				std::cout << "Branch \"" << bb << "\" merged successfully!" << std::endl;
			}
			// Add the files
			VGit::AddEverything(repo);

			// Commit
			VGit::CommitWithMessage(repo, bb, true);
		}
	}

	// Clones a repository using a filtered list of branches.
	// Optionally merges all non-blacklisted branches into a single working copy.

	inline bool CloneBeta(bool const& merge_branches, std::string const& repo, std::string const& repo_url, std::vector<std::string> const& blacklist) {

		// Initialize the repo (probably redundant)
		VGit::Init(repo);

		// Get a list of all the branches of the repo and write them to branches.txt
		// by using stdout to a file
		VGitUtils::ListBranches(repo, true, "branches.txt");

		// Copies the content of branches.txt as a vector, reading from the file above
		std::vector<std::string> const branches = CalculateBranches(repo);

		// If the file contains nothing
		if (branches.empty()) {
			return false;
		}
		else {
			for (auto const& j : branches) {
				std::cout << "Branch found: " << j << std::endl;
			}
		}

		// Beware that defining DEBUG also breaks Necronomicon
#ifndef DEBUG
		// Delete the (now useless) branches.txt
		VGitUtils::DoDeleteFile((fsys::path(repo) / "branches.txt").string());
#endif

		std::vector<std::string> beta_branches{};
		std::string beta_branch = MAIN_BRANCH_2;

		if (!blacklist.empty()) {
			// For every branch
			for (auto const& j : branches) {
				// Find if it contains the same words as the blacklist
				auto const find = std::find_if(blacklist.begin(), blacklist.end(), [&](std::string const& str) -> bool {return j.find(str) != std::string::npos; });
				// If it does
				if (find != blacklist.end()) {
					// Continue
					continue;
				}
				// If it doesn't, add it to beta_branches
				beta_branches.push_back(j);
			}
		}

		// If there aren't any branches that we can use, use the main branch
		// If there are, temporarily get the first one of the vector beta_branches
		beta_branch = beta_branches.empty() ? MAIN_BRANCH_2 : beta_branches.front();

		std::size_t const available_branches = branches.size();
		std::uint64_t const difference = available_branches - blacklist.size();

		std::cout << difference << " branches available!" << std::endl;

		// If no usable branches remain, force cloning of the main branch only.

		bool static const force_repo_deletion = false && difference <= 0;

		// If the difference between the number of 
		// available branches and the taken branches
		// is zero (or less?), or if we specifically
		// do not want to merge branches
		if (!merge_branches || force_repo_deletion) {
			// Delete the main folder
			VGitUtils::DeleteFolder(repo);
			// Clone the beta branch instead
			VGit::CloneRepositoryBranch(repo_url, beta_branch);
		}
		else {
			// Merge branches
			VGitUtils::DoMergeBranches(beta_branches, repo, beta_branch, repo_url);
		}

		return true;
	}
}