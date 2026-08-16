// Te// Team DAIX, 2026
// V3DAILYMANAGER — FILE OPERATIONS
//
// The majority of this code was written between 2020 and 2022
//
// This file contains all filesystem‑related utilities used by the DAILY
// automation pipeline. DAILY is completely separate from DAIXTOOLS and acts
// as a CI/CD system for Danganronpa V3 modding.
//
// These functions handle:
//   • Reading translation progress percentages
//   • Comparing files (.ups) between Daily and local builds
//   • Copying files with filters and preserving relative paths
//   • Cleaning directories before a new DAILY run
//   • Deleting specific Daily artifacts (UPS normals, Raw folders, etc.)
//   • Counting files for safe‑mode checks
//
// *** EXTREMELY IMPORTANT WARNING ***
//
// This file contains two functions — DeleteEverything() and DeleteEverything2() —
// which are **high‑risk destructive operations**. They will delete **EVERY FILE
// AND FOLDER THEY FIND**, except for a small whitelist of executables and source
// files.
//
// These functions MUST ONLY be executed inside:
//     • an EMPTY folder,
//     • OR a folder containing ONLY the DAILY executables,
//     • unless the user explicitly provides the --unsafe flag.
//
// Running DAILY in a non‑empty directory WITHOUT --unsafe is safe (safe‑mode
// prevents execution).
//
// Running DAILY in a non‑empty directory WITH --unsafe WILL DELETE FILES.
//
// These functions are intentionally dangerous because DAILY must guarantee a
// clean environment before cloning toolchains and generating builds. Use with
// extreme caution.


#include "files.h"

#include <array>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <Windows.h>
#endif
#include <unordered_set>

namespace Files {
	// GetPercentages()
	// Reads percentage_res.txt and maps each line to a predefined category
	// (Chapter 1, Chapter 2, Prologue, etc.).
	//
	// DAILY uses this to update README.md and README_EN.md with translation
	// progress percentages.
	//
	// Steps:
	//   1. Check if percentage_res.txt exists
	//   2. Read each line and pair it with a category name
	//   3. Swap Prologue with "Extra Mode(s)" (historical quirk)
	//   4. Return formatted strings

	std::vector<std::string> GetPercentages(fsys::path const& current_dir) {

		std::cout << "Checking percentages..." << std::endl;

		std::array<std::string, 15> static const possible_folders{
		"Extra Mode(s)",
		"Chapter 1",
		"Chapter 2",
		"Chapter 3",
		"Chapter 4",
		"Chapter 5",
		"Chapter 6",
		"Epilogue",
		"Gallery",
		"Generic Text",
		"Map Object Names",
		"Prologue",
		"Subroutine",
		"Test",
		"Total"
		};

		std::string const percentage_file = (current_dir / "percentage_res.txt").string();
		if (!fsys::exists(percentage_file)) {
			std::cout << "Couldn't find percentage file!" << std::endl;
			return {};
		}

		std::vector<std::string> ret{};
		std::string temp{};
		std::ifstream percentage_in(percentage_file, std::ios::in);
		std::uint64_t cont = 0;
		while (std::getline(percentage_in, temp)) {
			if (!temp.empty()) {
				std::string const str = possible_folders[cont] + ": " + temp + "%";
				ret.push_back(str);
			}
			cont++;
			if (cont >= possible_folders.size()) {
				break;
			}
		}
		percentage_in.close();

		std::cout << "Loaded percentage file" << std::endl;

		// Invert Prologue with ainori
		temp = ret[0];
		ret[0] = ret[11];
		ret[11] = temp;

		return ret;
	}

	// IsSTDEqualSame()
	// Compares two files using std::equal.
	//
	// Returns:
	//   -1  → file open error
	//   -2  → size mismatch
	//    0  → equal
	//    1  → different
	//
	// Used by DAILY to detect whether .ups files have changed.

	int IsSTDEqualSame(const std::string& p1, const std::string& p2) {
		std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
		std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

		if (f1.fail() || f2.fail()) {
			return -1; //file problem
		}

		if (f1.tellg() != f2.tellg()) {
			return -2; //size mismatch
		}

		//seek back to beginning and use std::equal to compare contents
		f1.seekg(0, std::ifstream::beg);
		f2.seekg(0, std::ifstream::beg);
		return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
			std::istreambuf_iterator<char>(),
			std::istreambuf_iterator<char>(f2.rdbuf()));
	}

	// IsInternallySame()
	// Loads both files fully into memory and compares their contents.
	//
	// Used as a fallback when IsSTDEqualSame() is inconclusive.

	bool IsInternallySame(const std::string& p1, const std::string& p2) {

		std::ifstream t(p1, std::ios::in | std::ios::binary);
		if (!t.good()) {
			return false;
		}
		std::string const str((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());
		t.close();

		std::ifstream t2(p2, std::ios::in | std::ios::binary);
		if (!t2.good()) {
			return false;
		}
		std::string const str2((std::istreambuf_iterator<char>(t2)),
			std::istreambuf_iterator<char>());
		t2.close();

		return str.length() == str2.length() && str == str2;
	}

	// Recursively copies all files and folders from src to target and overwrites existing files in target.
	//
	// DAILY uses this for copying Baked text files and other build artifacts.

	void CopyRecursive(const fsys::path& src, const fsys::path& target) noexcept {

		std::cout << "Copying Baked..." << std::endl;
		try
		{
			fsys::create_directories(src);
			fsys::create_directories(target);
			fsys::copy(src, target, fsys::copy_options::overwrite_existing | fsys::copy_options::recursive);
		}
		catch (std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
		std::cout << "Done copying." << std::endl;
	}

	// CopyFilesWithFilter()
	// Copies only files matching a specific extension (e.g., ".ups", ".txt").
	//
	// Features:
	//   • Supports recursive and non‑recursive modes
	//   • Preserves relative folder structure
	//   • Converts extension to uppercase for case‑insensitive matching
	//
	// DAILY uses this to copy:
	//   • ModifiedFiles → Daily SPC repo
	//   • Baked → Daily Text repo
	//   • Logs → Daily Log repo

	void CopyFilesWithFilter(const fsys::path& cur,
		const fsys::path& src,
		const fsys::path& target,
		std::string const& filetype,
		bool recursive) noexcept
	{
		std::vector<std::string> list{};

		fsys::create_directories(src);
		fsys::create_directories(target);

		auto uppercase_ext = filetype;
		std::transform(uppercase_ext.begin(), uppercase_ext.end(), uppercase_ext.begin(),
			[](char s) { return std::toupper(static_cast<unsigned char>(s)); });

		if (recursive) {
			for (auto const& it : fsys::recursive_directory_iterator(src)) {
				if (it.path().extension() == filetype || it.path().extension() == uppercase_ext) {
					list.push_back(it.path().string());
				}
			}
		}
		else {
			for (auto const& it : fsys::directory_iterator(src)) {
				if (it.path().extension() == filetype || it.path().extension() == uppercase_ext) {
					list.push_back(it.path().string());
				}
			}
		}

		for (auto const& file : list) {
			fsys::path const p(file);

			std::string name = "DefaultEmptyName";
			if (p.parent_path().filename() == cur.filename()) {
				// Same as before: just the filename
				name = p.filename().string();
			}
			else {
				// Get relative path from cur
				fsys::path rel = fsys::relative(p, cur);

				// Drop the first component (e.g., "ModifiedFiles")
				fsys::path trimmed;
				bool first = true;
				for (auto& part : rel) {
					if (first) { first = false; continue; }
					trimmed /= part;
				}
				name = trimmed.string();
			}

			std::cout << "Name for " << file << ": " << name << std::endl;

			std::string const newsrc = file;
			std::string const newdst = (target / name).string();

			if (!fsys::exists((target / name).parent_path())) {
				try {
					fsys::create_directories((target / name).parent_path());
				}
				catch (fsys::filesystem_error const& ec) {
					std::cout << "Attempt at creating folder(s) failed: " << ec.what() << std::endl;
				}
			}

			std::cout << "Copying " << newsrc << " to " << newdst << std::endl;
			try {
				fsys::copy(newsrc, newdst,
					fsys::copy_options::overwrite_existing |
					fsys::copy_options::recursive);

				if (!std::filesystem::exists(newdst)) {
					std::cout << "Something went wrong went copying, file does not actually exist" << std::endl;
				}
			}
			catch (fsys::filesystem_error const& ec) {
				std::cout << "Error while copying: " << ec.what() << std::endl;
				if (!fsys::exists(newsrc)) {
					std::cout << "newsrc's fault" << std::endl;
				}
				if (!fsys::exists(newdst)) {
					std::cout << "newdst's fault" << std::endl;
				}
			}
		}

		if (list.empty()) {
			std::cout << "CopyFilesWithFilter: list empty (" << filetype << "): " << target.string() << std::endl;
		}
		else {
			std::cout << "CopyFilesWithFilter: ok" << std::endl;
		}
	}

	// AppendFolder()
	// Ensures that all parent folders exist before creating a new folder.
	//
	// DAILY uses this when preparing the "Raw" folder inside LatestAutomaticBuild.

	fsys::path AppendFolder(fsys::path const& mypath, std::string const& str) {

		try {
			fsys::path temp = mypath;
			std::vector<fsys::path> paths{};
			bool ok = false;
			while (!ok) {

				paths.push_back(temp);
				temp = temp.parent_path();

				ok |= temp.parent_path().empty();
				ok |= !Common::StringContains(temp.parent_path().string(), "V3DailyManager");
			}
			for (int i = paths.size() - 1; i >= 0; i--) {
				fsys::path const it_path = paths[i];
				if (!fsys::exists(it_path)) {
					fsys::create_directory(it_path);
				}
			}
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
		}

		try {
			fsys::path ret = mypath / str;
			if (!fsys::exists(ret)) {
				fsys::create_directory(ret);
			}
			return ret;
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
			return mypath;
		}
	}

	// DeleteFolder()
	// Simple wrapper around remove_all() with exception handling.

	void DeleteFolder(std::string const& folder) {

		try {
			fsys::remove_all(folder);
		}
		catch (std::exception& ex) {
			std::cout << ex.what() << std::endl;
		}
	}

	// CheckDifferent()
	// Compares local .ups files against the Daily repository's Raw folder.
	//
	// Logic:
	//   • Raw folder = build_fd / "Raw"
	//   • For each .ups file:
	//       - Check if Raw version exists
	//       - Compare using IsSTDEqualSame() and IsInternallySame()
	//       - If ANY file differs → DAILY must rebuild
	//   • Always overwrite Raw with the new file
	//
	// Returns:
	//   true  → at least one file is different (build required)
	//   false → all files identical (no build needed)

	bool CheckDifferent(fsys::path const& build_fd, std::vector<std::string> const& files_vec) {

		bool new_exists = true;
		bool are_std_different = false;
		bool are_internally_different = false;
		bool comparison_good = false;

		// where_to_find becomes Daily's "Raw" inside LatestAutomaticBuild
		// NOTE: the "Raw" folder, despite the name, contains .UPS files
		fsys::path where_to_find = build_fd;
		where_to_find = Files::AppendFolder(where_to_find, "Raw");

		// Iterate through the second parameter (containing filenames, not paths)
		for (auto const& file : files_vec) {

			// Basically newf = Raw + file
			std::string const newf = (where_to_find / fsys::path(file).filename()).string();

			// Does the file (newf) exist?
			new_exists &= fsys::exists(newf);

			// Are they the same?
			are_std_different = Files::IsSTDEqualSame(file, newf) == false;
			are_internally_different = !Files::IsInternallySame(file, newf);

			// Even if just one file is different, then this is always going to be true (intentionally)
			comparison_good |= !new_exists || (are_std_different || are_internally_different);

			// Overwrite the file anyway
			fsys::copy_file(file, newf, fsys::copy_options::overwrite_existing);
		}

		// Return "was there any different file?"
		return comparison_good;
	}

	// DeleteDaily()
	// Deletes an entire Daily repository folder using rmdir /s /q.
	//
	// Used when DAILY needs to reset a repository before cloning.

	void DeleteDaily(std::string const repo_name) {

		std::string const command = "rmdir /s /q \"" + repo_name + "\"";
		Common::executeBatch(command.c_str());
	}

	// CountFilesInPath()
	// Counts all non‑directory entries.
	//
	// DAILY uses this for safe‑mode checks to prevent running in a dirty folder.

	std::uint64_t CountFilesInPath(fsys::path const& where) {

		std::uint64_t count = 0;
		for (auto const& file : fsys::recursive_directory_iterator(where)) {
			if (file.is_directory()) {
				continue;
			}

			count++;
		}

		return count;
	}

	// DeleteEverything()
	// !!! DANGEROUS FUNCTION — READ CAREFULLY !!!
	//
	// This is the legacy cleanup function used by DAILY.
	//
	// It deletes **EVERY FILE AND FOLDER** in the working directory EXCEPT:
	//   • DAILY executables (V3DailyManager, ups, unpackers)
	//   • autov3.txt
	//   • source files (.h/.cpp/.vcxproj/.sln/.filters/.user)
	//
	// EVERYTHING ELSE WILL BE DELETED.
	//
	// This function MUST ONLY be run inside:
	//   • an empty folder,
	//   • OR a folder containing ONLY the DAILY executables.
	//
	// Running DAILY without --unsafe prevents this function from running in
	// non‑empty folders.
	//
	// Running DAILY with --unsafe WILL DELETE FILES. Use with extreme caution.

	void DeleteEverything(fsys::path const& where, fsys::path const& current_dir) {

		std::vector<std::string> delete_vec{};

		if (fsys::exists(where)) {
			std::string const command = EncryptString("rmdir /s /q ") + where.string();
			system(command.c_str());
		}
		std::cout << "Removed old folder!" << std::endl;

		for (auto const& file : fsys::recursive_directory_iterator(current_dir)) {
			std::string const file_str = file.path().filename().string();
			std::string const file_ext = file.path().extension().string();
#ifdef _WIN32
			std::string const exe_ext = ".exe";
#else
			std::string const exe_ext = "";
#endif
			if (file_str != "7za" + exe_ext &&
				file_str != "V3DailyManager" + exe_ext &&
				file_str != "_V3DailyManager" + exe_ext &&
				file_str != "ups" + exe_ext &&
				file_str != "V3ImageUnpacker" + exe_ext &&
				file_str != "V3FontUnpacker" + exe_ext &&
				file_str != "V3DMGUI.exe" &&
				file_str != "D3DCompiler_47_cor3.dll" &&
				file_str != "PenImc_cor3.dll" &&
				file_str != "PresentationNative_cor3.dll" &&
				file_str != "vcruntime140_cor3.dll" &&
				file_str != "wpfgfx_cor3.dll" &&
				file_str != "autov3.txt"
				&&
				file_ext != ".h" &&
				file_ext != ".cpp" &&
				file_ext != ".vcxproj" &&
				file_ext != ".sln" &&
				file_ext != ".user" &&
				file_ext != ".filters"
				) {
				delete_vec.push_back(file.path().string());
			}
		}

		for (auto const& file : delete_vec) {
			if (fsys::is_directory(file)) {
#ifdef _WIN32
				std::string const command = "rmdir /s /q \"" + file + "\"";
#else
				std::string const command = "rm -r \"" + file + "\"";
#endif
				system(command.c_str());
			}
			else {
#ifdef _WIN32
				DeleteFileA(file.c_str());
#else
				fsys::remove(file);
#endif
			}
		}
	}

	// shouldDelete()
	// Determines whether a file/folder should be deleted.
	//
	// Whitelist includes:
	//   • DAILY executables
	//   • unpackers
	//   • autov3.txt
	//   • source code (.h/.cpp/etc)
	//   • anything inside DGRV3-Tools or DGRV3-Daily
	//
	// Everything else is considered safe to delete.
	// This function is !!DANGEROUS!! as it deletes ANYTHING IT FINDS with exceptions

	bool shouldDelete(const fsys::path& p) {

#ifdef _WIN32
		static const std::string exe_ext = ".exe";
#else
		static const std::string exe_ext = "";
#endif

		static const std::unordered_set<std::string> protectedNames = {
			"7za" + exe_ext, "V3DailyManager" + exe_ext, "_V3DailyManager" + exe_ext, "ups" + exe_ext,
			"V3ImageUnpacker" + exe_ext, "V3FontUnpacker" + exe_ext, "V3DMGUI" + exe_ext,
			"D3DCompiler_47_cor3.dll", "PenImc_cor3.dll", "PresentationNative_cor3.dll",
			"vcruntime140_cor3.dll", "wpfgfx_cor3.dll", "autov3.txt"
		};

		static const std::unordered_set<std::string> protectedExts = {
			".h", ".cpp", ".vcxproj", ".sln", ".user", ".filters"
		};

		std::string name = p.filename().string();
		std::string ext = p.extension().string();
		std::string pathStr = p.string();

		// Whitelist check
		if (protectedNames.count(name) || protectedExts.count(ext))
			return false;

		// Path-based exclusions
		if (pathStr.contains("DGRV3-Tools") || pathStr.contains("DGRV3-Daily"))
			return false;

		return true;
}

	// deletePath()
	// Wrapper around remove_all() with error reporting.

	void deletePath(const fsys::path& p) {
		std::error_code ec;
		fsys::remove_all(p, ec);
		if (ec) {
			std::cerr << "Failed to delete: " << p << " (" << ec.message() << ")\n";
		}
		else {
			std::cout << "Deleted: " << p << "\n";
		}
	}

	// DeleteEverything2()
	// !!! EXTREMELY DANGEROUS FUNCTION — READ CAREFULLY !!!
	//
	// This is the newer cleanup function. It is MORE explicit, but JUST AS
	// destructive as DeleteEverything().
	//
	// It deletes:
	//   • ALL files and folders not in the whitelist (executables, source files)
	//   • ALL Daily artifacts (Baked, DGRV3, EXTRACTED_FILES, logs, etc.)
	//   • ANYTHING that shouldDelete() considers safe to delete
	//
	// In practice, this function will delete **NEARLY EVERYTHING** in the working
	// directory.
	//
	// This function MUST ONLY be run inside:
	//   • an empty folder,
	//   • OR a folder containing ONLY the DAILY executables,
	//   • unless the user explicitly provides --unsafe.
	//
	// Running DAILY without --unsafe prevents accidental destruction.
	// Running DAILY with --unsafe WILL DELETE FILES. Use with extreme caution.

	void DeleteEverything2(const fsys::path& current_dir) {
		std::vector<fsys::path> delete_vec;

		// 1. Collect files/folders to delete based on whitelist rules
		for (auto const& file : fsys::recursive_directory_iterator(current_dir)) {
			if (shouldDelete(file.path())) {
				delete_vec.push_back(file.path());
			}
		}

		// 2. Add explicit targets for deletion
		static const std::vector<std::string> explicitTargets = {
			"Baked",
			"DGRV3",
			"DGRV3_EN",
			"EXTRACTED_FILES",
			"REPACKED_FILES",
			"stx_folder",
			"wrd_folder",
			"arrow_report.txt",
			"charcount_res.txt",
			"conflicts.txt",
			"detailed_charcount_rex.txt",
			"different_hashes.txt",
			"different_lines.txt",
			"different_part2.txt",
			"dolog.txt",
			"eh_report.txt",
			"file_copied.txt",
			"file_copied.txt",
			"fonttools_log.txt",
			"imagetools_log.txt",
			"list_changed.txt",
			"percentage_res.txt",
			"randomizer_report.txt",
			"sha_diff.txt",
			"texttols_log.txt",
			"var_replace_map.txt",
			"variablechecker.txt",
			"variablechecker_short.txt",
			"words_counted.txt",
			"classdatabase_license.txt",
		};

		for (auto const& target : explicitTargets) {
			fsys::path fullPath = current_dir / target;
			if (fsys::exists(fullPath)) {
				delete_vec.push_back(fullPath);
			}
		}

		// 3. Delete everything in the list
		for (auto const& file : delete_vec) {
			deletePath(file);
		}
	}

	// DeleteUPSNormals()
	// Removes *_patch_normal.ups files.
	//
	// These are temporary artifacts created during UPS generation and should not
	// be uploaded to Daily repositories.

	void DeleteUPSNormals(fsys::path const& where) {
		std::vector<std::string> delete_vec{};

		for (auto const& file : fsys::recursive_directory_iterator(where)) {
			if (file.is_directory()) {
				continue;
			}
			if (Common::StringContains(file.path().string(), "_patch_normal.ups")) {
				delete_vec.push_back(file.path().string());
			}
		}

		for (auto const& del : delete_vec) {
			std::cout << "Deleting UPS normal: " << del << std::endl;
			fsys::remove_all(del);
		}
	}
}