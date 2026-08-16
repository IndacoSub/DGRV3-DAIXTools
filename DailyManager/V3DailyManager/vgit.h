// Team DAIX, 2026
// VGIT

// The majority of this code was written between 2020 and 2022

// Core Git command wrappers and system utilities used by VGitUtils.
// Provides thin abstractions over shell-based Git operations.

#pragma once

#include <string>
#include <iostream>
#include "common.h"

// Enables verbose logging for all Git command executions.
// This is a custom flag, not the compiler's built-in DEBUG macro.

#define _DEBUG

// Alias 'fsys' to std::filesystem unless already defined elsewhere.
// Allows platform-specific overrides in other headers.

#ifndef fsys
#define fsys std::filesystem
#endif

#define EncryptString(x) std::string{x}

#define MAIN_BRANCH EncryptString("master")
#define MAIN_BRANCH_2 EncryptString("main")

// Predefined Git command strings used to build shell commands.
// These are concatenated with repository paths, branch names, or arguments.

#define GIT_COMMAND_ADD EncryptString("git add ")
#define GIT_COMMAND_CHECKOUT EncryptString("git checkout ")
#define GIT_COMMAND_CLONE EncryptString("git clone --recursive ")
#define GIT_COMMAND_COMMIT EncryptString("git commit ")
#define GIT_COMMAND_FETCH EncryptString("git fetch ")
#define GIT_COMMAND_INIT EncryptString("git init")
#define GIT_COMMAND_LIST_BRANCHES EncryptString("git branch -r")
#define GIT_COMMAND_MERGE EncryptString("git merge ")
#define GIT_COMMAND_PULL EncryptString("git pull ")
#define GIT_COMMAND_PUSH EncryptString("git push ")
#define GIT_COMMAND_PUSH_TAG EncryptString("git push origin ")
#define GIT_COMMAND_REMOTE_ADD EncryptString("git remote add origin ")
#define GIT_COMMAND_STATUS EncryptString("git status")
#define GIT_COMMAND_TAG EncryptString("git tag -a ")
#define GIT_COMMAND_VER EncryptString("git --version")

namespace System {
	inline void Execute(std::string const& command, bool const& silent = false);
	inline void ExecuteCD(std::string const& repo, std::string const& command, bool const& silent = false);
}


namespace System {

	// Executes a shell command directly. If 'silent' is true, suppresses echo.
	// Used for commands that do not require changing directories.

	inline void Execute(std::string const& command, bool const& silent) {

		std::string const com = silent ? "@echo off && " + command : command;
#ifdef _DEBUG
		std::cout << "[DEBUG] Executing command (normal): " << com << std::endl;
#endif
		// Could maybe break if com contains spaces
		if (system(com.c_str()) != 0) {
			std::cout << "Execute failed (command: " + com + ")" << std::endl;
		}
	}

	// Executes a shell command after switching to the specified repository folder.
	// Useful for Git commands that must run inside the repo directory.


	inline void ExecuteCD(std::string const& repo, std::string const& command, bool const& silent) {

		std::string const before = (silent ? "@echo off && " : "");
		std::string const c = before + "cd " + repo + " && " + command;

#ifdef _DEBUG
		std::cout << "[DEBUG] Executing command (CD): " << c << std::endl;
#endif

		// Could maybe break if c contains spaces
		if (system(c.c_str()) != 0) {
			std::cout << "ExecuteCD failed (command: " + c + ")" << std::endl;
		}
	}
}

namespace VGit {
	inline void AddAll(std::string const& repo);
	inline void AddEverything(std::string const& repo);
	inline void Checkout(std::string const& repo, std::string const& branch);
	inline void CloneRepositoryBranch(std::string const& repository, std::string const& branch_name);
	inline void CloneRepositoryMain(std::string const& repository);
	inline void Commit(std::string const& repo, bool const& silent = false);
	inline void CommitWithMessage(std::string const& repo, std::string const& message, bool const& silent = false);
	inline void Fetch(std::string const& repo, std::string const& branch);
	inline void GetGitStatus(void);
	inline void GetGitVersion(void);
	inline void Init(std::string const& repo);
	inline void Merge(std::string const& repo, std::string const& branch, std::string const& result_file);
	inline void Pull(std::string const& repo, std::string const& branch, std::string const& result_file);
	inline void Push(std::string const& repo, std::string const& remote, std::string const& branch);
	inline void PushTag(std::string const& repo, std::string const& tag_name);
	inline void RemoteAdd(std::string const& repository);
	inline void Tag(std::string const& repo, std::string const& version, std::string const& message);
}

namespace VGit {
	inline void GetGitVersion(void) {
		std::cout << std::endl;
		std::cout << "Obtaining Git version..." << std::endl;
		std::cout << std::endl;

		System::Execute(GIT_COMMAND_VER);
	}

	inline void GetGitStatus(void) {

		System::Execute(GIT_COMMAND_STATUS);
	}

	// Clones the repository's default branch (master/main) using --recursive.

	inline void CloneRepositoryMain(std::string const& repository) {

		std::cout << std::endl;
		std::cout << "Cloning main (translated) branch" << std::endl;
		std::cout << std::endl;

		std::string const command = GIT_COMMAND_CLONE + repository + " ";
		System::Execute(command.c_str());
	}

	// Clones a specific branch using 'git clone -b'. Includes submodules.

	inline void CloneRepositoryBranch(std::string const& repository, std::string const& branch_name) {

		std::cout << std::endl;
		std::cout << "Cloning branch: \"" << branch_name << "\"..." << std::endl;
		std::cout << std::endl;

		std::string const command = GIT_COMMAND_CLONE + repository + " -b " + branch_name;
		System::Execute(command.c_str());

		std::cout << std::endl;
		std::cout << "Repository cloned!" << std::endl;
		std::cout << std::endl;
	}

	inline void Init(std::string const& repo) {

		System::ExecuteCD(repo, GIT_COMMAND_INIT);
	}

	// Stages all changes in the repository (equivalent to 'git add .').


	inline void AddEverything(std::string const& repo) {

		std::string const& command = GIT_COMMAND_ADD + ".";

		System::ExecuteCD(repo, command);
	}

	inline void Fetch(std::string const& repo, std::string const& branch) {

		std::string const command = GIT_COMMAND_FETCH + branch;

		System::ExecuteCD(repo, command);
	}

	inline void RemoteAdd(std::string const& repository) {

		std::string const command = GIT_COMMAND_REMOTE_ADD + repository;

		System::ExecuteCD(repository, command);
	}

	inline void Checkout(std::string const& repo, std::string const& branch) {

		std::string const command = GIT_COMMAND_CHECKOUT + branch;

		System::ExecuteCD(repo, command);
	}

	// Merges a remote branch into the current branch.
	// Redirects output to a file so merge conflicts can be detected later.


	inline void Merge(std::string const& repo, std::string const& branch, std::string const& result_file) {

		std::string const command = GIT_COMMAND_MERGE + "origin/" + branch + " > " + result_file + " 2>&1";

		System::ExecuteCD(repo, command);
	}

	// Pulls updates from a remote branch and logs the output to a file.


	inline void Pull(std::string const& repo, std::string const& branch, std::string const& result_file) {

		std::string const command = GIT_COMMAND_PULL + "origin/" + branch + " > " + result_file + " 2>&1";

		System::ExecuteCD(repo, command);
	}

	inline void Commit(std::string const& repo, bool const& silent) {
		System::ExecuteCD(repo, GIT_COMMAND_COMMIT, silent);
	}

	// Creates a commit with a custom message. Supports silent execution.


	inline void CommitWithMessage(std::string const& repo, std::string const& message, bool const& silent) {
		System::ExecuteCD(repo, GIT_COMMAND_COMMIT + "-m \"" + message + "\"", silent);
	}

	inline void Push(std::string const& repo, std::string const& remote, std::string const& branch) {
		System::ExecuteCD(repo, GIT_COMMAND_PUSH + remote + " " + branch);
	}

	inline void AddAll(std::string const& repo) {
		System::ExecuteCD(repo, GIT_COMMAND_ADD + ".");
	}

	// Creates an annotated tag with a message.


	inline void Tag(std::string const& repo, std::string const& version, std::string const& message) {
		System::ExecuteCD(repo, GIT_COMMAND_TAG + version + " -m \"" + message + "\"");
	}

	// Pushes a tag to the remote repository (origin).


	inline void PushTag(std::string const& repo, std::string const& tag_name) {
		System::ExecuteCD(repo, GIT_COMMAND_PUSH_TAG + tag_name);
	}
}