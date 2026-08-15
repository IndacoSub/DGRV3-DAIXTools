// Team DAIX, 2026
// MAID — CONSOLE COMPILATION DEFINITIONS

// The majority of this code was written between 2020 and 2022

// This file's purpose is:
// compile_console.h declares the function prototypes and shared state used by
// Maid’s Nintendo Switch / Unity image compilation pipeline. It defines:
//
//   • The ABOrAssetCompiled atomic counter for multithreaded progress tracking
//   • PreprocessPNG() for scanning PNG/TGA files and resolving their target
//     Unity AssetBundle/sharedassets containers
//   • CompileImages() for batching and compiling Unity assets
//   • DownloadABOrAsset() for FileOnDemand cloud retrieval of .ab/.assets files
//   • SortEntriesByABOrAsset() for grouping PNGs by their target container
//   • CalculateAllEntriesABOrAsset() and CalculateABOrAsset() for invoking
//     UAFGJ and performing actual Unity asset injection
//
// This header provides the structural interface for compile_console.cpp,
// enabling Maid to perform Unity‑based texture injection for console builds.

#pragma once

#include "Maid.h"

namespace Console {

	inline std::atomic<std::uint64_t> ABOrAssetCompiled = 0;

	bool CompileConsole(std::filesystem::path const& current_dir, std::string const& repo, std::string const& basefolder);
	std::vector<Common::FileStructure> PreprocessPNG(std::filesystem::path const& dgrv3path, std::filesystem::path const& where, std::string const& repo, std::vector<std::string>& seen);
	void CompileImages(std::vector<Common::FileStructure> const& fs_vec, std::vector<std::string> const& seen);
	bool DownloadABOrAsset(std::string const& output_file, std::string const& dl_repo_path, int const& tryno);
	Common::SplitType SortEntriesByABOrAsset(std::vector<Common::FileStructure> const& entries, std::vector<std::string> const& container_list);
	void CalculateAllEntriesABOrAsset(std::vector<Common::FileStructure> const& entries);
	void CalculateABOrAsset(std::filesystem::path const& uafgj_loc, std::string const& input_file, std::string const& container);
}