// Team DAIX, 2026
// MAID — PC COMPILATION DEFINITIONS

// The majority of this code was written between 2020 and 2022

// This file's purpose is:
// compile_pc.h declares all data structures, lookup tables, and function
// prototypes used by Maid’s PC/Xbox image compilation pipeline. It
// defines:
//
//   • Entry and Alias types for SRD/SPC mapping
//   • Minigame-specific SPC insertion rules
//   • Blacklisted SPCs that cannot be compiled
//   • Special-case SRD filenames and alias resolution tables
//   • Matryoshka SPC mappings (nested SPC → parent SPC)
//   • Atomic counters for multithreaded SRD/SPC compilation
//   • Function declarations for ALT handling, SRD/SPC generation,
//     preprocessing, sorting, downloading, and cleanup
//
// This header provides the structural backbone for compile_pc.cpp, allowing
// Maid to correctly identify, preprocess, sort, and rebuild all SRD/SPC assets
// required for the PC pipeline.

#pragma once

#include <vector>
#include <string>
#include <atomic>
#include <filesystem>
#include <array>

#include "Maid.h"

#define EN(x, y) std::make_pair(x, y)
#define AL(x, y, z) std::make_tuple(x, y, z)

namespace PC {

	inline const std::filesystem::path nil("");

	using Entry = std::pair<std::string, std::string>;
	using Alias = std::tuple<std::string, std::string, std::string>;

	// These entries define which PNG/SPC assets belong to minigame data files
	// that require special handling during compilation.

	static const std::array<Entry, 1> minigame_entries{

		EN("anagram_US", "minigame/anagram/answer000.dat"),
	};

	// SPC files that should never be compiled on PC.
	// These assets either crash SPCTool, produce invalid output,
	// or require manual handling outside Maid.

	static const std::array<std::string, 1> spc_blacklist{
		// Not ready, pain in the ....
		"adv_reaction_voice_US",
	};

	// TODO: Deprecate SPECIAL and ALIASES
	// SRD files whose internal PNG → SRD mapping does NOT follow the usual
	// "texture.srd" convention. These entries override the default SRD target.

	static const std::vector<Entry> srd_special {
		// Entries are useful when the .srd file is not "texture.srd"
		// X should go in Y
		EN("3B48A96E.png", "caution_download_US.srd"),
		EN("7AEB69E6.png", "caution_gro_US.srd"),
	};

	// TODO: Deprecate SPECIAL and ALIASES
	// SRD alias table: resolves naming conflicts where multiple PNGs share the same
	// CRC-32 hash or temporary name. Maps (temp → real → target SRD).

	static const std::vector<Alias> srd_aliases {
		// Aliases are useful when two or more names are in conflict
		// CRC-32 using 7Zip
		// Temporary name, Real name, .srd file to go inside
		// X, which is really Y "in disguise", should go in Z
		AL("3B48A96E.png", "kanban_US.png", "caution_download_US.srd"),
		AL("7AEB69E6.png", "kanban_US.png", "caution_gro_US.srd"),
	};

	// "Matryoshka SPCs": SPC files that contain OTHER SPC files inside them.
	// Maid cannot compile these normally, so they must be pre-repacked manually.
	// This table maps (inner SPC → outer SPC) for correct insertion.

	static const std::vector<Entry> matrioSPCka {
		// SPCs inside SPCs, how cool is that?!
		// Not at all. It's a pain in the ...
		// So we're going to ignore them entirely
		// and MITM our way out of it
		// X should go in Y

		// Those SPCs on the right are stored... where?
		// I assure you there's no chara_name_US.spc in flash/adv/spcpack
		// WHERE THE F DOES IT COME FROM?!

		// UPDATE: YOU DUMB F##K
		// YOU NEED TO PRE-REPACK THE .SPC ON THE LEFT WITH THE DESIRED CHANGES
		// THIS IS SO DUMB
		// WHY PUT THE PNGs AS WELL, THEN?!

		EN((nil / "flash" / "adv" / "spcpack" / "chara_name_US" / "chara_name_000.spc").string(), (nil / "flash" / "adv" / "spcpack" / "chara_name_US.spc").string()), // Shuichi
		EN((nil / "flash" / "adv" / "spcpack" / "chara_name_US" / "chara_name_002.spc").string(), (nil / "flash" / "adv" / "spcpack" / "chara_name_US.spc").string()), // Ryoma
		EN((nil / "flash" / "adv" / "spcpack" / "chara_name_US" / "chara_name_003.spc").string(), (nil / "flash" / "adv" / "spcpack" / "chara_name_US.spc").string()), // Rantaro
		EN((nil / "flash" / "adv" / "spcpack" / "chara_name_US" / "chara_name_005.spc").string(), (nil / "flash" / "adv" / "spcpack" / "chara_name_US.spc").string()), // Kokichi
		EN((nil / "flash" / "adv" / "spcpack" / "chara_name_US" / "chara_name_006.spc").string(), (nil / "flash" / "adv" / "spcpack" / "chara_name_US.spc").string()), // Korekiyo
		EN((nil / "flash" / "adv" / "spcpack" / "chara_name_US" / "chara_name_008.spc").string(), (nil / "flash" / "adv" / "spcpack" / "chara_name_US.spc").string()), // Kirumi
		EN((nil / "flash" / "adv" / "spcpack" / "chara_name_US" / "chara_name_021.spc").string(), (nil / "flash" / "adv" / "spcpack" / "chara_name_US.spc").string()), // Monotaro

		EN((nil / "flash" / "adv" / "spcpack" / "chara_name_US" / "chara_name_027.spc").string(), (nil / "flash" / "adv" / "spcpack" / "chara_name_US.spc").string()), // Monokubs

		// Chapter 1 trial

		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_000.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Shuichi
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_u_000.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Shuichi
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_002.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Ryoma
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_u_002.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Ryoma
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_003.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Rantaro
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_u_003.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Rantaro
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_005.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Kokichi
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_u_005.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Kokichi
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_006.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Korekiyo
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_u_006.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Korekiyo
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_008.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Kirumi
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_u_008.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Kirumi
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_021.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Monotaro
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_u_021.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Monotaro

		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_027.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Monokubs
		EN((nil / "flash" / "trial" / "spcpack" / "t_chara_name_US" / "t_kaiwa_chara_name_u_027.spc").string(), (nil / "flash" / "trial" / "spcpack" / "t_chara_name_US.spc").string()), // Monokubs
	};

	// Atomic counters used for multithreaded compilation progress tracking.
	// Incremented whenever an SPC or SRD file is successfully compiled.

	inline std::atomic<std::uint64_t> SPCCompiled = 0;
	inline std::atomic<std::uint64_t> SRDCompiled = 0;

	bool CompilePC(std::filesystem::path const& current_dir, std::string const& repo, std::string const& basefolder);

	void HandleAltFiles(std::filesystem::path const& dgrv3path);
	void CalculateSrd(std::filesystem::path const& srdtool_loc, std::string const& png, std::string const& srd);
	void CalculateSpc(std::string const& srd, std::filesystem::path const& program, std::string const& file_to_insert);
	void DeleteUnchanged(std::filesystem::path const& dgrv3path, std::vector<std::string> const& changed_files);
	void CompileImages(std::vector<Common::FileStructure> const& fs_vec, std::vector<std::string> const& spc_list, std::vector<std::string> const& srd_list,
		std::filesystem::path const& where, std::filesystem::path const& dgrv3path);
	void CompileMinigames(std::vector<Common::FileStructure> const& fs_vec, std::vector<std::string>& seen_spc,
		std::filesystem::path const& where, std::filesystem::path const& dgrv3path);
	Common::SplitType SortEntriesBySPC(std::vector<Common::FileStructure> const& entries, std::vector<std::string> const& spc_list);
	Common::SplitType SortEntriesBySRD(std::vector<Common::FileStructure> const& entries, std::vector<std::string> const& srd_list);
	std::vector<Common::FileStructure> PreprocessPNG(std::filesystem::path const& dgrv3path, std::filesystem::path const& where, std::string const& repo, std::vector<std::string>& seen_spc, std::vector<std::string>& seen_spc_raw, std::vector<std::string>& seen_srd);
	void CalculateAllEntriesSPC(std::vector<Common::FileStructure> const& entries);
	void DownloadSPC(std::string const& output_spc, std::string const& dl_repo_path);
	std::string ReplaceLastOccurrence(const std::string& str, const std::string& from, const std::string& to);
}