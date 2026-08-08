// Team DAIX, 2026
// NECRONOMICON
//
// The majority of this code was written between 2020 and 2022
// 
// This tool's purpose is to:
// 1. Compute SHA-512 hashes for English and translated script files
// 2. Compare both hashes to detect any byte-level differences
// 3. Save lists of mismatched files and mismatched folders
// 4. Provide a "part2" mode to compare SPC/PB files instead of .txt
//
// Used to ensure script integrity and detect accidental edits.


#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>
#include <thread>
#include <unordered_set>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "calc_sha.h"

#include "../Common/Common.h"
#include "../Common/Config.h"

// Stores the SHA results for a single file comparison.
// Used only when a mismatch is found.

struct HashTemp {
	std::string FirstHash{};
	std::string SecondHash{};

	void SaveToFile(std::string const& open_filename) const {
		std::ofstream out("sha_diff.txt", std::ios::out | std::ios::app);
		out << "Filename: " << open_filename << std::endl;
		out << "First SHA: " << this->FirstHash << std::endl;
		out << "Second SHA: " << this->SecondHash << std::endl;
		out << "Same SHA: " << (this->FirstHash == this->SecondHash ? "True " : "False") << std::endl;
		out << std::endl;
		out.close();
	}
};

std::string ReplaceWithEnglish(std::string const& str, std::string const repo, std::string const repo_en);
bool DidSaveSha(std::filesystem::directory_entry const& file, bool const& ispart2);
bool CalculateSome(std::filesystem::path const& dgrv3path, std::filesystem::path const& dgrv3path_en,
	std::string const& repo, std::string const& repo_en,
	std::vector<std::uint64_t>& different_indexes, std::vector<std::string>& different);
bool DoPart2(std::filesystem::path const& current_dir);
bool PerformChecks(std::string const& file_str);

std::vector<std::string> const static folders{

	EncryptString("test"),
	EncryptString("subroutine"),
	EncryptString("prologue"),
	EncryptString("MapObjName"),
	EncryptString("game_resident"),
	EncryptString("gallery"),
	EncryptString("epilogue"),
	EncryptString("chapter6"),
	EncryptString("chapter5"),
	EncryptString("chapter4"),
	EncryptString("chapter3"),
	EncryptString("chapter2"),
	EncryptString("chapter1"),
	EncryptString("ainori"),
};

int main(int argc, char* argv[]) {

	std::vector<std::string> different{};
	std::vector<std::uint64_t> different_indexes{};

	bool all = false;
	bool part2 = false;

	if (argc > 0) {
		if (argv[1] != nullptr) {
			if (strcmp(argv[1], EncryptString("--all")) == 0) {
				all = true;
			}

			if (strcmp(argv[1], EncryptString("part2")) == 0) {
				part2 = true;
			}

			if (argv[2] != nullptr) {
				if (strcmp(argv[2], EncryptString("--all")) == 0) {
					all = true;
				}

				if (strcmp(argv[2], EncryptString("part2")) == 0) {
					part2 = true;
				}
			}
		}
	}

	const std::string repo = EncryptString("DGRV3");
	const std::string repo_en = EncryptString("DGRV3_EN");

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const dgrv3path = current_dir / repo;
	std::filesystem::path const dgrv3path_en = current_dir / repo_en;
	std::filesystem::path const configfile = current_dir / "TextConfig.config";

	LOG("Reading config", HERE, "Necronomicon");
	Configuration::ReadConfig(configfile);
	Configuration::ViewDebugCurrentConfig();
	LOG("Done reading config", HERE, "Necronomicon");

	if (part2) {
		return DoPart2(current_dir);
	}

	if (!std::filesystem::exists(dgrv3path)) {
		LOG("ERROR: One of the folders where hashes were going to be calculated was not found!", HERE, "Necronomicon");
		return -1;
	}

	if (!std::filesystem::exists(dgrv3path_en)) {
		LOG("ERROR: One of the folders where hashes were going to be calculated was not found!", HERE, "Necronomicon");
		return -1;
	}

	// Yes, Necronomicon can delete them because Necronomicon is the one who *creates* them
	std::string const different_hash_folders_file = EncryptString("different_hash_folders.txt");

	if (std::filesystem::exists(current_dir / different_hash_folders_file)) {
		std::filesystem::remove(current_dir / different_hash_folders_file);
	}

	std::string const different_hashes_file = EncryptString("different_hashes.txt");

	if (std::filesystem::exists(current_dir / different_hashes_file)) {
		std::filesystem::remove(current_dir / different_hashes_file);
	}

	bool ok = true;

	if (!all) {
		LOG("Calculating a selection of files...", HERE, "Necronomicon");
		ok = CalculateSome(dgrv3path, dgrv3path_en, repo, repo_en, different_indexes, different);
	}
	else {
		LOG("Calculating all files...", HERE, "Necronomicon");
		for (std::uint64_t i = 0; i < folders.size(); i++) {
			different_indexes.push_back(i);
		}
	}

	if (ok) {
		if (!different_indexes.empty()) {
			LOG("\n", HERE, "Necronomicon");
			LOG("Will be saved:", HERE, "Necronomicon");
			for (auto const& j : different_indexes) {
				LOG(" - " + folders[j], HERE, "Necronomicon");
			}

			LOG("\n", HERE, "Necronomicon");
			LOG("Saving...", HERE, "Necronomicon");

			std::ofstream out(different_hash_folders_file, std::ios::out | std::ios::app);
			for (auto const& j : different_indexes) {
				out << j << std::endl;
			}
			out.close();

			std::sort(different.begin(), different.end());

			std::ofstream out2(different_hashes_file, std::ios::out | std::ios::app);
			for (auto const& j : different) {
				out2 << Common::ShortenFilename(j, 2) << std::endl;
			}
			out2.close();

			LOG("\n", HERE, "Necronomicon");
			LOG("Saving done!", HERE, "Necronomicon");
		}
		else {
			LOG("WARNING: No different indexes found!", HERE, "Necronomicon");
		}
	}
	else {
		LOG("ERROR: Something went wrong...", HERE, "Necronomicon");
	}

	Common::WaitExit();
}

// Computes SHA-512 for a file and writes it to "<filename>_sha".
// Part1: only .txt (or .pb on Switch)
// Part2: only .spc (or .pb on Switch)

bool DidSaveSha(std::filesystem::directory_entry const& file, bool const& ispart2) {

	if (file.is_directory()) {
		return false;
	}

	const std::filesystem::path path = file.path();
	const std::string file_str = path.string();

	// Extension checks (case-insensitive, using path::extension)
	auto ext = path.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

	// Part1: compare text files (.txt) or Switch binary text (.pb)
	// Part2: compare SPC files (.spc) or Switch PB files (.pb)

	if (!ispart2) {
		// Part 1: only .txt
		if (ext != ".txt") {
			if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
				if (ext != ".pb") {
					return false;
				}
			}
			else {
				return false;
			}
		}
	}
	else {
		// Part 2: only .spc
		if (ext != ".spc") {
			if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
				if (ext != ".pb") {
					return false;
				}
			}
			else {
				return false;
			}
		}
	}

	if (!PerformChecks(file_str)) {
		return false;
	}

	// Ok, if the program is here
	// then obviously the iterated
	// file has succeeded in all
	// these checks above
	// so we can be sure that
	// the file is "good"

	try {
		// Read the entire file as-is (binary), preserving all bytes and newline styles
		std::ifstream in;
		in.exceptions(std::ios::failbit | std::ios::badbit);
		in.open(path, std::ios::binary);

		std::ostringstream buffer;
		buffer << in.rdbuf();  // exact bytes, no line ending normalization
		in.close();

		using namespace CoolNameLibrary;
		const auto hash = HashCalc::CalculateHash<SHA512>(buffer.str());

		// Output file: append "_sha" to the filename (keeps the existing convention)
		// Append "_sha" to the filename to store the computed hash.
		// Example: "scene_12.txt" → "scene_12.txt_sha"

		std::filesystem::path out_path = path;
		out_path += EncryptString("_sha");

		std::ofstream out;
		out.exceptions(std::ios::failbit | std::ios::badbit);
		out.open(out_path, std::ios::binary | std::ios::trunc);
		out << hash << '\n';
		out.close();

		return true;
	}
	catch (std::exception const& ex) {
		LOG(std::string("ERROR: Failed to compute or write SHA for ") + file_str + " — " + ex.what(), HERE, "Necronomicon");
		return false;
	}
	catch (...) {
		LOG(std::string("ERROR: Unknown failure while processing ") + file_str, HERE, "Necronomicon");
		return false;
	}
}

// Main comparison routine:
// 1. Compute SHA for all English files
// 2. Identify which translated files *should* have SHA files
// 3. Compare translated SHA vs English SHA
// 4. Record mismatches and folder indexes

bool CalculateSome(
	const std::filesystem::path& dgrv3path,
	const std::filesystem::path& dgrv3path_en,
	const std::string& repo,
	const std::string& repo_en,
	std::vector<std::uint64_t>& different_indexes,
	std::vector<std::string>& different
) {
	LOG("Calculating SHA for english branch...", HERE, "Necronomicon");

	// Tracks how many files were processed in each branch.
	// Used to detect mismatched file counts before comparing SHA.

	struct CountManager {
		std::uint64_t TotalEnglishFiles = 0;
		std::uint64_t TotalItalianFiles = 0;
		std::uint64_t TotalSHAEnglishFiles = 0;
		std::uint64_t TheoreticalItalianSHAFiles = 0;
	} cm{};

	std::vector<std::string> en_files;
	std::vector<std::string> it_files;

	auto safe_read_file = [](const std::filesystem::path& p) -> std::string {
		std::ifstream in(p, std::ios::binary);
		if (!in) throw std::runtime_error("Failed to open: " + p.string());
		std::ostringstream buf;
		buf << in.rdbuf();
		return buf.str();
	};

	auto is_valid_txt = [](const std::string& s) {
		return Common::StringContains(s, ".txt") &&
			!Common::StringContains(s, ".bak") &&
			!Common::StringContains(s, "vars_bak");
	};

	// Pass 1: English branch
	for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path_en)) {
		if (file.is_directory()) continue;
		cm.TotalEnglishFiles++;

		try {
			if (DidSaveSha(file, false)) {
				const auto path_str = file.path().string();
				if (is_valid_txt(path_str) || (Configuration::ConfigMap["UseSwitchConfiguration"] && file.path().extension() == ".pb")) {
					cm.TotalSHAEnglishFiles++;
					en_files.push_back(Common::ShortenFilename(path_str, 2));
				}
			}
		}
		catch (const std::exception& e) {
			LOG(std::string("ERROR processing English file: ") + e.what(), HERE, "Necronomicon");
		}
	}

	LOG("\n", HERE, "Necronomicon");
	LOG("Done calculating SHA for the english branch!", HERE, "Necronomicon");

	// Pass 2: Italian branch
	for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path)) {
		if (file.is_directory()) continue;
		cm.TotalItalianFiles++;

		const auto path_str = file.path().string();
		if (!is_valid_txt(path_str)) {
			if (!(Configuration::ConfigMap["UseSwitchConfiguration"] && file.path().extension() == ".pb")) {
				continue;
			}
		}
		if (!PerformChecks(path_str)) continue;

		cm.TheoreticalItalianSHAFiles++;
		it_files.push_back(Common::ShortenFilename(path_str, 2));
	}

	// Sanity check
	// If the number of SHA-able files differs between branches,
	// comparison cannot continue reliably — dump diagnostics and abort.

	if (cm.TheoreticalItalianSHAFiles != cm.TotalSHAEnglishFiles) {
		LOG("ERROR: File count mismatch (" +
			std::to_string(cm.TheoreticalItalianSHAFiles) + " vs " +
			std::to_string(cm.TotalSHAEnglishFiles) + "), aborting", HERE, "Necronomicon");

		{
			std::ofstream f_en("sha_en.txt", std::ios::app);
			for (const auto& en_str : en_files) {
				f_en << en_str << '\n';
			}
		}

		{
			std::ofstream f_it("sha_it.txt", std::ios::app);
			for (const auto& it_str : it_files) {
				f_it << it_str << '\n';
			}
		}

		return false;
	}

	LOG("English SHAs calculated: " +
		std::to_string(cm.TotalSHAEnglishFiles) + "/" +
		std::to_string(cm.TotalEnglishFiles), HERE, "Necronomicon");

	LOG("\n", HERE, "Necronomicon");
	LOG("Calculating SHA once again for translation, and comparing...", HERE, "Necronomicon");

	// Pass 3: Compare
	for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path)) {
		if (file.is_directory()) continue;
		const auto file_str = file.path().string();
		if (!PerformChecks(file_str)) continue;
		if (Common::StringContains(file_str, "base_spc") ||
			//Common::StringContains(file_str, "i18n") ||
			Common::StringContains(file_str, "vars_bak")) continue;

		try {
			using namespace CoolNameLibrary;
			// Compare translated file SHA with English SHA.
			// If different, record the file and save both hashes.

			const auto first_hash = HashCalc::CalculateHash<SHA512>(safe_read_file(file.path()));

			// Build English SHA path
			auto en_path = std::filesystem::path(
				ReplaceWithEnglish(file_str, repo, repo_en)
			);
			en_path.replace_extension(file.path().extension().string() + "_sha");

			if (!std::filesystem::exists(en_path)) {
				LOG("WARNING: Expected file " + en_path.string() + " does not exist!", HERE, "Necronomicon");
				continue;
			}

			auto second_hash = safe_read_file(en_path);
			// Strip CR/LF
			second_hash.erase(std::remove(second_hash.begin(), second_hash.end(), '\r'), second_hash.end());
			second_hash.erase(std::remove(second_hash.begin(), second_hash.end(), '\n'), second_hash.end());

			if (!first_hash.empty() && !second_hash.empty() && first_hash != second_hash) {
				different.push_back(file_str);
				HashTemp{ first_hash, second_hash }.SaveToFile(file.path().filename().string());
			}
		}
		catch (const std::exception& e) {
			LOG(std::string("ERROR comparing file: ") + e.what(), HERE, "Necronomicon");
		}
	}

	// Pass 4: Map differences to folder indexes
	std::unordered_set<std::uint64_t> seen_indexes{};
	for (auto const& diff_file : different) {
		const auto folder_name = std::filesystem::path(diff_file).parent_path().filename().string();
		// Map each mismatched file to its folder index.
		// Example: "chapter3" → index 10
		auto it = std::find(folders.begin(), folders.end(), folder_name);
		if (it != folders.end()) {
			auto idx = static_cast<std::uint64_t>(std::distance(folders.begin(), it));
			if (seen_indexes.insert(idx).second) {
				different_indexes.push_back(idx);
			}
		}
		else {
			LOG("Invalid folder string: " + folder_name, HERE, "Necronomicon");
		}
	}

	return true;
}

// Part2 mode: compute SHA for SPC/PB files in Distribute/
// Compare "<file>" vs "<file>_normal.ext_sha"
// Used by WhiteSheet stage 2.


bool DoPart2(std::filesystem::path const& current_dir) {

	LOG("Doing part2", HERE, "Necronomicon");

	std::vector<std::string> different{};

	std::filesystem::path const distr = current_dir / EncryptString("Distribute");

	if (std::filesystem::path(distr).empty()) {
		LOG("Directory is empty: " + distr.string(), HERE, "Necronomicon");
	}

	// Iterate through the distribution folder
	for (auto const& filed : std::filesystem::recursive_directory_iterator(distr)) {
		if (filed.is_directory()) {
			LOG("Ignoring directory: " + filed.path().string(), HERE, "Necronomicon");
			continue;
		}
		
		LOG("(In theory) found file to compare: " + filed.path().string(), HERE, "Necronomicon");

		// We don't need to check if it succeeded
		// since we're not comparing these files to anything else
		DidSaveSha(filed, true);
	}

	for (auto const& file : std::filesystem::recursive_directory_iterator(distr)) {

		if (file.is_directory()) {
			continue;
		}

		if (Common::StringContains(file.path().string(), "_sha")) {
			continue;
		}

		if (Common::StringContains(file.path().string(), "_lines")) {
			continue;
		}

		if (Common::StringContains(file.path().string(), "normal")) {
			continue;
		}

		LOG("(In practice) found file to compare: " + file.path().string(), HERE, "Necronomicon");

		// Compare the SHAs

		std::string const& oldname = file.path().string();
		auto oldname_ext = file.path().extension().string();
		auto oldname_ext_len = oldname_ext.length();

		// Build the expected SHA filename for the base ("_normal") version.
		// Example:
		//   nonstop_01_US.spc
		// → nonstop_01_US_normal.spc_sha

		std::string const& newname = file.path().string().substr(0, 
			file.path().string().length() - oldname_ext_len) + "_normal" + oldname_ext + "_sha";

		std::transform(oldname_ext.begin(), oldname_ext.end(), oldname_ext.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });

		if (!std::filesystem::exists(oldname)) {
			LOG("WARNING:  " + oldname + " (old) does not exist!", HERE, "Necronomicon");
			continue;
		}

		if (!std::filesystem::exists(newname)) {
			LOG("WARNING: " + newname + " (new) does not exist!", HERE, "Necronomicon");
			continue;
		}

		std::string sha1{};
		std::string sha2{};

		std::ifstream i1(oldname, std::ios::in);
		std::getline(i1, sha1);
		i1.close();

		std::ifstream i2(newname, std::ios::in);
		std::getline(i2, sha2);
		i2.close();

		if (sha1 != sha2) {
			std::string noext = oldname.substr(0, oldname.length() - oldname_ext_len);
			std::string const& str_lc = noext + oldname_ext;
			if (std::filesystem::exists(str_lc)) {
				different.push_back(str_lc);
			}
			else {
				std::transform(oldname_ext.begin(), oldname_ext.end(), oldname_ext.begin(),
					[](unsigned char c) { return static_cast<char>(std::toupper(c)); });
				std::string const str_uc = noext + oldname_ext;
				if (std::filesystem::exists(str_uc)) {
					different.push_back(str_uc);
				}
			}
		}
	}

	if (!different.empty()) {

		try {
			if (std::filesystem::exists("different_part2.txt")) {
				std::filesystem::remove("different_part2.txt");
			}

			if (std::filesystem::exists("different_part2_short.txt")) {
				std::filesystem::remove("different_part2_short.txt");
			}
		}
		catch (std::filesystem::filesystem_error ec) {
			LOG("Error while removing old log files: " + std::string{ ec.what() }, HERE, "Necronomicon");
		}

		std::sort(different.begin(), different.end());

		std::ofstream out("different_part2.txt", std::ios::out | std::ios::app);
		for (auto const& j : different) {
			out << std::filesystem::path(j) << std::endl;
		}
		out.close();

		std::ofstream out2("different_part2_short.txt", std::ios::out | std::ios::app);
		for (auto const& j : different) {
			out2 << Common::ShortenFilename(j, 1) << std::endl;
		}
		out2.close();

		LOG("Necronomicon probably succeeded", HERE, "Necronomicon");
	}
	else {
		LOG("different.empty()", HERE, "Necronomicon");
	}

	return EXIT_SUCCESS;
}

// Filters out metadata, backup, and non-script files.
// Only actual script files should be hashed and compared.

bool PerformChecks(std::string const& file_str) {

	if (Common::StringContains(file_str, "_sha")) {
		return false;
	}
	if (Common::StringContains(file_str, "_lines")) {
		return false;
	}
	if (Common::StringContains(file_str, "README")) {
		return false;
	}
	if (Common::StringContains(file_str, "LICENSE")) {
		return false;
	}
	if (Common::StringContains(file_str, ".git")) {
		return false;
	}
	if (Common::StringContains(file_str, "Baked")) {
		return false;
	}

	return true;
}

// Converts a translated file path into the corresponding English file path.
// Example:
//   "DGRV3/chapter3/scene_12.txt"
// → "DGRV3_EN/chapter3/scene_12.txt"

std::string ReplaceWithEnglish(std::string const& str, std::string const repo, std::string const repo_en) {

	if (!Common::StringContains(str, repo)) {
		return str;
	}

	auto const find = str.find(repo);
	std::size_t to_replace = find;
	if (find + 1 < str.length()) {
		auto const find_2 = str.find(repo, find + 1);
		if (find_2 != std::string::npos) {
			to_replace = find_2;
		}
	}

	auto ret = str;
	auto const before = ret.substr(0, to_replace);
	auto const after = ret.substr(to_replace + repo.length());
	ret = before + repo_en + after;
	return ret;
};