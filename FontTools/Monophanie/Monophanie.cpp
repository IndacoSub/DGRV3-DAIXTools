// Team DAIX, 2026
// MONOPHANIE — FONTTOOLS EXTRACTOR & FONT DUPLICATOR

// The majority of this code was written between 2020 and 2022

// This tool's purpose is:
// Monophanie extracts the font-related .7z archives inside DGRV3-Font/base_spc
// and prepares the working font directory for FontTools. It performs:
//
//   • Extraction of SPCTool and HTFont archives via 7za
//   • Copying base font files into the working directory
//   • Preparing trial_font variants by cloning and renaming font folders
//   • Renaming .stx and .srdv files to match the expected game_font01_* naming
//
// NOTE ABOUT HTFont:
// HTFont.exe is part of an **older version of Harmony-Tools** (by Redssu, https://github.com/redssu/Harmony-Tools).
// Harmony-Tools used to be split into multiple executables (HTFont, HTSrd,
// HTSpc, etc.). Modern Harmony-Tools is distributed as a single unified
// executable, but FontTools still uses the older HTFont component because:
//
//   • It provides stable STX/SRDV font manipulation routines
//   • It matches the expected behavior of the DAIXTOOLS font pipeline
//   • It avoids breaking compatibility with existing SPC/STX workflows
//
// Monophanie is the “font bootstrapper” of FontTools. It ensures that all
// required font assets (SPC fonts, STX/SRDV files, trial fonts) are extracted,
// duplicated, and renamed correctly before any other FontTools component runs.

#include <iostream>
#include <filesystem>
#include <array>
#include <string>
#include <errno.h>

#include "../Common/Common.h"

int main() {

	// Repository name (and also folder)
	const std::string repo = EncryptString("DGRV3-Font");

	// Locate the DGRV3-Font repository and its base_spc folder.
	// Monophanie expects the repo to have been cloned by Monotaro.

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const dgrv3path = current_dir / repo;

	std::filesystem::path const where = (dgrv3path / EncryptString("base_spc"));

	std::filesystem::path const trialfonts = (dgrv3path / "trial_font");
	std::filesystem::path const tf_first = (trialfonts / "game_font01_8_US_DEC");

	if (!std::filesystem::exists(where)) {
		LOG("ERROR: The folder where the files to copy are located was not found!", HERE, "Monophanie");
		return -1;
	}

	LOG("Monophanie v1.4", HERE, "Monophanie");

	LOG("Extracting files...", HERE, "Monophanie");

	// 7za is required to extract SPCTool and HTFont archives.
	// These archives contain font-related SPC/STX/SRDV tools and assets.

#ifdef _WIN32
	std::filesystem::path const _7za = where / "7za.exe";

	if (!std::filesystem::exists(_7za)) {
		LOG("ERROR: 7za could not be found!", HERE, "Monophanie");
		return EXIT_FAILURE;
}
#else
	std::filesystem::path const _7za = "7za";
#endif

	std::filesystem::path const cur_font = current_dir / "DGRV3-Font";
	if (!std::filesystem::exists(cur_font)) {
		LOG("ERROR: " + cur_font.string() + " could not be found!", HERE, "Monophanie");
		return EXIT_FAILURE;
	}

	std::filesystem::path const font_base_spc = cur_font / "base_spc";
	if (!std::filesystem::exists(font_base_spc)) {
		LOG("ERROR: " + font_base_spc.string() + " could not be found!", HERE, "Monophanie");
		return EXIT_FAILURE;
	}

	// Select correct archive names depending on platform.
	// Windows uses SPCTool.7z; Linux uses SPCTool_Linux.7z.

#ifdef _WIN32
	std::string const platform = "";
#else
	std::string const platform = "_Linux";
#endif

	std::filesystem::path const spctool = font_base_spc / ("SPCTool" + platform + ".7z");
	std::filesystem::path const htfont = font_base_spc / ("HTFont" + platform + ".7z");

	// Extract SPCTool archive.
	// SPCTool is used for SPC font manipulation and rebuilding font SPCs.

#ifdef _WIN32
	std::string const extract_spctool = "\"" + _7za.string() + "\"" + std::string{ " x \"" } + spctool.string() + "\" -o\"" + font_base_spc.string() + "\" -aoa";
#else
	std::string const extract_spctool = "7za x \"" + spctool.string() + "\" -o\"" + font_base_spc.string() + "\" -aoa";
#endif

	Common::executeBatch(extract_spctool.c_str());

	// Extract HTFont archive.
	// HTFont.exe comes from an older version of Harmony-Tools (Redssu).
	// It provides STX/SRDV font manipulation routines used by FontTools.

#ifdef _WIN32
	std::string const extract_htfont = "\"" + _7za.string() + "\"" + std::string{ " x \"" } + htfont.string() + "\" -o\"" + font_base_spc.string() + "\" -aoa";
#else
	std::string const extract_htfont = "7za x \"" + htfont.string() + "\" -o\"" + font_base_spc.string() + "\" -aoa";
#endif

	Common::executeBatch(extract_htfont.c_str());

#ifndef _WIN32
	Common::executeBatch("chmod -R +x *");
#endif

	// Copy base font files from base_spc into the working directory.
	// These include STX/SRDV templates and SPC font assets required by FontTools.

	LOG("Copying files...", HERE, "Monophanie");

	std::vector<std::string> files_to_be_copied{};
	for (auto const& file : std::filesystem::directory_iterator(where)) {
		if (file.is_directory()) {
			continue;
		}
		files_to_be_copied.push_back(file.path().filename().string());
	}

	std::uint64_t actually_copied = 0;
	for (auto const& file : files_to_be_copied) {
		LOG("1: Copying " + ((where / file)).string() + " to " + ((current_dir / file)).string(), HERE, "Monophanie");
		std::filesystem::copy((where / file), (current_dir / file), std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
		if (std::filesystem::exists((current_dir / file))) {
			actually_copied++;
		}
	}

	// Duplicate base_fonts into base_fonts_copy.
	// FontTools modifies base_fonts_copy while keeping base_fonts untouched.

	std::filesystem::copy(where / "base_fonts", where / "base_fonts_copy", std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

	LOG(std::to_string(actually_copied) + " files actually copied.", HERE, "Monophanie");

	LOG("Now copying " + tf_first.filename().string() + "...", HERE, "Monophanie");

	// Trial font variants to generate.
	// Each number corresponds to a different game_font01_* folder used by the game.

	std::array<std::string, 7> static const tf_copies{
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"9",
	};

	for (auto const& newtf : tf_copies) {
		std::filesystem::path const newpath = trialfonts / ("game_font01_" + newtf + "_US_DEC");
		std::filesystem::path const oldv3f = newpath / ("v3_font01_8/");
		std::filesystem::path const newv3f = newpath / ("v3_font01_" + newtf + "/");
		std::filesystem::path const oldstx = (newpath / ("v3_font01_8.stx"));
		std::filesystem::path const newstx = (newpath / ("v3_font01_" + newtf + ".stx"));
		std::filesystem::path const oldsrdv = (newpath / ("v3_font01_8.srdv"));
		std::filesystem::path const newsrdv = (newpath / ("v3_font01_" + newtf + ".srdv"));

		// Clone the base trial font folder (game_font01_8_US_DEC)
		// and rename its internal v3_font01_* folders and STX/SRDV files
		// to match the new variant number (1, 2, 3, 4, 5, 6, 9).

		try {
			LOG("2: Copying " + (tf_first).string() + " to " + (newpath).string(), HERE, "Monophanie");
			std::filesystem::copy(tf_first, newpath, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);
			std::filesystem::rename(oldv3f, newv3f);
			std::filesystem::rename(oldstx, newstx);
			std::filesystem::rename(oldsrdv, newsrdv);
		}
		catch (std::exception& e) {
			std::string e_str = std::string{ e.what() };
			LOG(e_str, HERE, "Monophanie");
		}
	}

	LOG(tf_first.string() + " copied!", HERE, "Monophanie");

	Common::WaitExit();
}
