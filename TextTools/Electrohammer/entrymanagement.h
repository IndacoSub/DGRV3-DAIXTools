#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <array>

#include "Electrohammer.h"
#include "../Common/Common.h"

namespace EntryMG {

	struct Entry {
		// File name
		std::string Filename = "";
		// Current dir (probably redundant)
		std::filesystem::path CurrentDir{};
		// Program directory?
		std::filesystem::path WhereTo{};
		// .STX file this goes into
		std::string STXFileToInsert = "";
		// .SPC file this goes into
		std::string SPCFileToInsert = "";

		Entry() = default;
		virtual compl Entry() = default;
	};

	constexpr static const std::uint64_t SpcFiles = 14;

	// Has to be down here
	using SplitType = std::array<std::vector<Entry>, SpcFiles>;

	static std::vector<const char*> const blacklist {

		// game_resident's SaveLoad has issues with StxTool (refer to the "game resident repacking" issue on GitHub)
		// Remove this when the issue gets fixed (or if we're not using StxTool anymore):

		//"SaveLoad",

		// game_resident's A-CharacterName has issues with StxTool (refer to the "game resident repacking" issue on GitHub)
		// In particular, character names "in the overworld" are blank when this is enabled
		// Remove this when the issue gets fixed (or if we're not using StxTool anymore):
		
		//"A-CharacterName",
	};

	// Folders
	static std::array<const char*, 14> const folders {

		"test",
		"subroutine",
		"prologue",
		"MapObjName",
		"game_resident",
		"gallery",
		"epilogue",
		"chapter6",
		"chapter5",
		"chapter4",
		"chapter3",
		"chapter2",
		"chapter1",
		"ainori",
	};

	// FTI = files to insert?
	static std::array<const char*, SpcFiles> const fti {
		EncryptString("test_text_US.SPC"),			// 0
		EncryptString("sub_routine_text_US.SPC"),	// 1
		EncryptString("chap0_text_US.SPC"),			// 2
		EncryptString("map_obj_name_text_US.SPC"),	// 3
		EncryptString("game_resident_US.spc"),		// 4
		EncryptString("gallery_text_US.SPC"),		// 5
		EncryptString("chap7_text_US.SPC"),			// 6
		EncryptString("chap6_text_US.SPC"),			// 7
		EncryptString("chap5_text_US.SPC"),			// 8
		EncryptString("chap4_text_US.SPC"),			// 9
		EncryptString("chap3_text_US.SPC"),			// 10
		EncryptString("chap2_text_US.SPC"),			// 11
		EncryptString("chap1_text_US.SPC"),			// 12
		EncryptString("ainori_text_US.SPC"),		// 13
	};

	bool CheckEntry(
		std::filesystem::directory_entry const& file,
		std::string* file_to_insert,
		std::vector<std::string> const& different_files,
		std::vector<std::uint64_t> const& different_indexes,
		std::vector<std::string>& skipped
	);

	void SortEntriesBySPC(std::vector<Entry> const& entries, SplitType& split);
	std::uint64_t GetIndex(std::string const& str);
	std::string GetFileToInsert(std::uint64_t const& index);
	std::vector<std::uint64_t> CalculateFoldersToCompile(std::filesystem::path const& savepath);
	std::vector<std::string> CalculateFilesToCompile(std::filesystem::path const& different_hashes_file);
	std::vector<Entry> CalculateEntries(std::filesystem::path const& folder_index_file, std::filesystem::path const& different_hashes_file,
		std::filesystem::path const& dgrv3path,
		std::filesystem::path const& where, std::filesystem::path const& current_dir, std::vector<std::string>& skipped_files);
}