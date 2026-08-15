// Team DAIX, 2026
// MAID — CORE IMAGE STRUCTURES

// The majority of this code was written between 2020 and 2022

// This file's purpose is:
// Maid.h defines the FileStructure class, the central data model used by all
// ImageTools components (Inventor, Maid, TennisPro, Detective, compile_pc,
// compile_console). FileStructure represents a single PNG/TGA asset and tracks:
//
//   • Its original repository path (PNG_RepoName)
//   • Its actual working path after alias/ALT resolution (PNG_ActualName)
//   • Its target Unity AssetBundle/sharedassets file (AssetOrBundle_Filename)
//   • Its target SRD/SRDV files (SRD_Filename / SRDV_Filename)
//   • Its target SPC archive (SPC_Filename)
//
// The ToArrowReport() method generates human‑readable pipeline mappings used
// for debugging and verification. Maid.h also defines SplitType, the container
// used for multithreaded batching of SRD/SPC/AB compilation.
//
// This header provides the foundational structure required for all image
// preprocessing, sorting, compilation, and reporting across the ImageTools
// pipeline.


#pragma once

#include <string>
#include <map>
#include <utility>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "../Common/Config.h"
#include "../Common/Common.h"

namespace Common {
	
	// FileStructure represents a single PNG/TGA asset inside the DGRV3-GFX or
	// DGRV3-AB-GFX repository. Every stage of the pipeline relies on this structure
	// to track where an image comes from and where it must be inserted.
	class FileStructure {
	public:
		// The original path of the PNG inside the graphics repository.
		// This is the “source of truth” before any aliasing or renaming.
		std::string PNG_RepoName{};
		// The actual filename used during compilation. This may differ from
		// PNG_RepoName when alias resolution or ALT file replacement occurs.
		std::string PNG_ActualName{};
		// (Switch/Unity only)
		// The target Unity AssetBundle (.ab) or sharedassets file (.assets)
		// that the PNG must be inserted into.
		std::string AssetOrBundle_Filename{};
		// (PC/Xbox only)
		// The SRD file that will receive the PNG. Usually “texture.srd”, but
		// special cases override this via srd_special or alias tables.
		std::string SRD_Filename{};
		// (PC/Xbox only)
		// The companion SRDV file (SRD + “v”) that must also be inserted into
		// the SPC archive.
		std::string SRDV_Filename{};
		// (PC/Xbox only)
		// The final SPC archive that will receive the SRD/SRDV pair.
		std::string SPC_Filename{};

		FileStructure() = default;
		virtual compl FileStructure() = default;

		std::string ToArrowReport(void) const {

			std::string ret = Common::ShortenFilename(this->PNG_RepoName, 3);
			while (ret.length() < 80) {
				ret += " ";
			}

			if (Configuration::ConfigMap["UseSwitchConfiguration"]) {
				ret += " -> " + Common::ShortenFilename(this->AssetOrBundle_Filename, 2);
			}
			else {
				ret += " -> " + Common::ShortenFilename(this->SRD_Filename, 3);;
				while (ret.length() < 180) {
					ret += " ";
				}
				ret += " -> " + Common::ShortenFilename(this->SPC_Filename, 2);
			}

			return ret;
		}
	};

	// Has to be down here
	// SplitType groups FileStructure entries by their target container.
	// Used for multithreaded SRD/SPC/AB compilation.
	using SplitType = std::vector<std::vector<FileStructure>>;
}