// Team DAIX, 2026
// V3DAILYMANAGER — DISTRIBUTION PIPELINE
//
// The majority of this code was written between 2020 and 2022
//
// This file contains the final stage of the DAILY automation pipeline:
// distributing compiled text, graphics, and font patches into the correct
// Daily repositories for each platform.
//
// DAILY is completely separate from DAIXTOOLS. It does not compile anything;
// instead, it *runs* external toolchains (TextTools, ImageTools, FontTools)
// and then organizes their output into platform‑specific folder structures.
//
// This file handles:
//   • Running TextTools / ImageTools / FontTools per platform
//   • Preparing platform‑specific directory layouts (PC, Switch, Xbox)
//   • Copying .ups patches into the correct game folders
//   • Copying baked text, logs, and SPC files into Daily repos
//   • Cleaning unneeded platform folders (Unity builds)
//   • Renaming Distribute/ModifiedFiles folders per platform
//
// This is the most complex part of DAILY — it transforms raw tool output
// into the final Daily build structure ready for upload.


#include "distribution.h"

#include <iostream>
#include <array>

#include "files.h"

#include "process.h"

namespace Distribution {

	// DeleteUnneededPlatforms()
	// Removes platform folders that are not relevant to the current build.
	//
	// Used only for Unity platforms (Switch / Android).
	//
	// Logic:
	//   • Keep only "master" and the folder matching platform_internal
	//   • Delete everything else
	//
	// This prevents Unity builds from containing leftover folders from other
	// platforms (e.g., Switch build containing Android folders).

	void DeleteUnneededPlatforms(fsys::path from_where, std::string const& platform_internal) {

		std::vector<std::string> static const whitelisted = {
			"master",
		};

		std::vector<std::string> to_delete{};

		for (auto const& file : fsys::directory_iterator(from_where)) {
			if (!file.is_directory()) {
				continue;
			}

			std::string const filename = file.path().filename().string();

			auto const whitelist_it = std::find(whitelisted.begin(), whitelisted.end(), filename);
			bool const is_whitelisted = whitelist_it != whitelisted.end();
			if (is_whitelisted) {
				continue;
			}

			if (filename == platform_internal) {
				continue;
			}

			to_delete.push_back(file.path().string());
		}

		for (auto const& fold : to_delete) {
			std::cout << "DeleteUnneededPlatforms - Deleting directory: " << fold << std::endl;
			fsys::remove_all(fold);
		}
	}

	// DistributeFiles()
	// This is the core of DAILY’s distribution logic.
	//
	// DAILY calls this once per platform. It performs:
	//
	//   1. Cleanup of working directory (DeleteEverything2)
	//   2. Run TextTools / ImageTools / FontTools depending on flags
	//   3. Prepare platform‑specific folder structure
	//   4. Copy .ups patches into correct game folders
	//   5. Copy baked text into Daily‑Text repo
	//   6. Copy logs into Daily‑Log repo
	//   7. Copy SPC files into Daily‑SPC repo
	//   8. Rename Distribute/ModifiedFiles folders to include platform name
	//
	// This function is the heart of DAILY — it transforms raw tool output into
	// the final Daily build layout.

	void DistributeFiles(Platform const& p, fsys::path const& build_fd, fsys::path const& cur,
		fsys::path const& img_dir, fsys::path const& font_dir, fsys::path const& tools_loc,
		fsys::path const& distfolder, fsys::path const& bakedfold, fsys::path const& baked_destination,
		fsys::path const& copy_log_to, fsys::path const& find_spc, fsys::path const& copy_spc_to,
		fsys::path const& find_spc_gfx, fsys::path const& find_spc_font)
	{
		fsys::path build_platform = build_fd;
		fsys::path distribution = distfolder;
		fsys::path dist_base = distribution; // keep original for Switch source paths

		std::string const platform_name = Process::GetPlatformName(p);

		std::cout << "Compiling for platform: " << platform_name << std::endl;

		// Clean the working directory before starting a new platform build.
		// This removes leftover files from previous builds.
		Files::DeleteEverything2(cur);

		// Run TextTools (StackedBooks) unless disabled by --no-text
		std::cout << "Text: " << (no_text ? "false" : "true") << std::endl;
		if (!no_text) {
			Process::RunTextTools(tools_loc, cur, p);
		}

		// Run ImageTools (Pianist) unless disabled by --no-gfx
		std::cout << "GFX: " << (no_gfx ? "false" : "true") << std::endl;
		if (!no_gfx) {
			Process::RunImageTools(tools_loc, cur, p);
		}

		// Run FontTools (Monokuma) unless disabled by --no-font
		std::cout << "Font: " << (no_font ? "false" : "true") << std::endl;
		if (!no_font) {
			Process::RunFontTools(tools_loc, cur, p);
			std::cout << "Font OK again" << std::endl;
		}

		// Ensure the platform build folder exists (LatestAutomaticBuild/<platform>)
		std::cout << "BuildFD: " << build_fd << std::endl;
		if (!fsys::exists(build_fd)) {
			std::cout << "BuildFD does not exist, creating it..." << std::endl;
			try {
				fsys::create_directories(build_fd);
			}
			catch (std::exception& e) {
				std::cout << e.what() << std::endl;
				return;
			}
		}

		std::cout << "Distribution folder: " << distribution << std::endl;
		if (!fsys::exists(distribution)) {
			std::cout << "Distribution folder not found!" << std::endl;
			return;
		}

		std::cout << "Now distributing..." << std::endl;

		// Collect all .ups files from Distribute folder, excluding blacklisted ones.
		// Blacklist contains files that should never be uploaded (debug menus, gallery).

		std::vector<std::filesystem::path> files_vec{};
		std::vector<std::string> const static files_blacklist{
			"Basic_Character",
			"Basic_Character_patch",
			"Basic_Character_patch.ups",
			"DebugMenuInfo_DebugMenuInfo",
			"DebugMenuInfo_DebugMenuInfo_patch",
			"DebugMenuInfo_DebugMenuInfo_patch.ups",
			"Gallery_CharacterGallery",
			"Gallery_CharacterGallery_patch",
			"Gallery_CharacterGallery_patch.ups",
			"Gallery_CharacterGalleryMessage",
			"Gallery_CharacterGalleryMessage_patch",
			"Gallery_CharacterGalleryMessage_patch.ups",
		};

		for (auto const& file : fsys::recursive_directory_iterator(distribution)) {
			if (!file.is_directory() && (file.path().string().find(".ups") != std::string::npos)) {
				auto const it = std::find(files_blacklist.begin(), files_blacklist.end(), fsys::path(file).filename().string());
				if (it == files_blacklist.end()) {
					files_vec.push_back(file);
				}
				else {
					std::cout << "Ignoring file found in blacklist: " << file.path().string() << std::endl;
				}
			}
		}

		std::cout << "Found files from files_vec: " << std::endl;
		for (auto const& ff : files_vec) {
			std::cout << "\t" << ff << std::endl;
		}
		std::cout << std::endl;

		distribution = distribution / platform_name;

		std::cout << "Distributing for platform: " << platform_name << std::endl;

		// DAILY must mimic the real game folder structure for each platform.
		//
		// PC:
		//   <build>/PC/data/win/...
		//
		// Switch (Unity):
		//   <build>/Switch/Data/StreamingAssets/Switch/...
		//
		// Xbox:
		//   <build>/Xbox/Data/WIN/...


		build_platform = Files::AppendFolder(build_platform, platform_name);
		std::string const base_platform = build_platform.string();
		fsys::path before_specific = {};
		bool uses_unity = false;

		switch (p) {
		case Platform::PC:
			build_platform = Files::AppendFolder(build_platform, "data");
			before_specific = build_platform;
			build_platform = Files::AppendFolder(build_platform, "win");
			break;

		case Platform::Switch:
			build_platform = Files::AppendFolder(build_platform, "Data");
			build_platform = Files::AppendFolder(build_platform, "StreamingAssets");
			before_specific = build_platform;
			build_platform = Files::AppendFolder(build_platform, "Switch");
			uses_unity = true;
			break;

		case Platform::Xbox:
			build_platform = Files::AppendFolder(build_platform, "Data");
			before_specific = build_platform;
			build_platform = Files::AppendFolder(build_platform, "WIN");
			break;

		default:
			std::cout << "Unknown platform: " << static_cast<int>(p) << "!" << std::endl;
			return;
		}

		// Unity-only
		// Unity builds require a "master" folder inside StreamingAssets.
		// DAILY creates it automatically for Switch builds.

		fsys::path master = build_fd / platform_name;
		if (uses_unity) {
			master = Files::AppendFolder(master, "Data");
			master = Files::AppendFolder(master, "StreamingAssets");
			master = Files::AppendFolder(master, "master");
		}

		std::cout << "Preparations: done!" << std::endl;

		// For each .ups file:
		//   • Determine its category:
		//       - game_resident
		//       - wrd_data
		//       - wrd_script
		//       - I18n (Unity only)
		//   • Copy it into the correct folder inside the platform build
		//
		// DAILY reconstructs the game’s internal folder structure so patches can be
		// applied correctly by mod loaders or repackers.

		if (!no_text) {
			for (auto const& file : files_vec) {

				std::string file_str = file.string();
				auto j = file_str;
				bool is_game_resident = false;
				bool is_wrd_data = false;
				bool is_pb = false;
				std::string dest{};

				std::string const fn = fsys::path(file).filename().string();
				std::string const ups_fn = fn.substr(0, fn.length() - fsys::path(file).extension().string().length()) + ".ups";
				std::string src{};

				bool const is_trial_file =
					(j.contains(".ups") || j.contains(".UPS")) &&
					(j.contains("hanron_") || j.contains("kokoronpa_08_") || j.contains("nico_06_") || j.contains("nonstop_") || j.contains("panic_"));

				if (file_str.contains("I18n") || file_str.contains("i18n")) {
					if (uses_unity && file_str.contains("I18n") || file_str.contains("i18n")) {
						if (!fsys::exists(master)) {
							fsys::create_directory(master);
						}
						// Wait a sec. src is *always* the file from files_vec...
						src = file_str;
						dest = (master / ups_fn).string();
						is_pb = true;

						if (!fsys::exists(src)) {
							std::cout << "Cannot find file for Switch: " << src << std::endl;
							continue;
						}
					}
					else {
						continue;
					}
				}

				if (src.empty()) {
					src = file_str;
					if (!fsys::exists(src)) {
						std::cout << "Does not exist: " << src << std::endl;
						src = (distribution.parent_path() / ups_fn).string();
						if (!fsys::exists(src)) {
							std::cout << "Cannot find file: " << src << std::endl;
						}
					}
				}

				// TODO: Update
				std::string const latest_chapter = "chap2";

				if (!private_repo && Common::StringContains(file_str, latest_chapter) && false) {
					std::cout << "Skipping latest chapter for public builds" << std::endl;
					continue;
				}

				if (file_str.contains("game_resident")) {
					src = file_str;
					std::cout << "game_resident found: " << file << std::endl;
					fsys::path const gameres = build_platform / "game_resident";
					if (!fsys::exists(gameres)) {
						fsys::create_directory(gameres);
					}

					dest = (gameres / ups_fn).string();
					is_game_resident = true;
				}

				if (file_str.contains("map_obj_name") || is_trial_file) {
					src = file_str;
					std::cout << "map_obj_name found: " << file << std::endl;
					fsys::path const wrddata = build_platform / "wrd_data";
					if (!fsys::exists(wrddata)) {
						fsys::create_directory(wrddata);
					}

					dest = (wrddata / ups_fn).string();
					is_wrd_data = true;
				}

				if (!is_game_resident && !is_wrd_data && !is_pb) {
					src = file_str;
					std::cout << "wrd file found? " << file << std::endl;
					std::string static const last_revision = "007";
					fsys::path wrdscript = build_platform / "wrd_script";
					if (!fsys::exists(wrdscript)) {
						fsys::create_directory(wrdscript);
					}
					wrdscript = wrdscript / last_revision;
					if (!fsys::exists(wrdscript)) {
						fsys::create_directory(wrdscript);
					}

					dest = (wrdscript / ups_fn).string();
				}

				if (src.empty()) {
					std::cout << "Uhhhh... source is empty?" << std::endl;
				}

				if (dest.empty()) {
					std::cout << "Uhhhh... destination is empty?" << std::endl;
				}

				bool copy_res = false;
				try {
					copy_res = fsys::copy_file(src, dest, fsys::copy_options::overwrite_existing);
				}
				catch (fsys::filesystem_error ec) {
					std::cout << "Error while copying " << src << " to " << dest << ": " << ec.what() << std::endl;
					continue;
				}

				if (copy_res) {
					std::cout << "Copied " << src << " to " << dest << std::endl;
				}
				else {
					std::cout << "Did not copy " << src << " to " << dest << std::endl;
				}
			}

			std::cout << "Text: done!" << std::endl;

			// This does NOT copy randomized text from the randomizer
			// because it ONLY COPIES "BAKED"
			// Copy the .txt files from Baked to DGRV3-Daily(-Private)-Text
			// The version is the same as in RunTextTools by default

			// Copy Baked/*.txt into Daily‑Text/<platform>/
			// DAILY only uploads baked text, not randomized text.
			std::string const find_baked = bakedfold.string();
			std::string const copy_baked_to = (baked_destination / platform_name).string();
			Files::CopyRecursive(find_baked, copy_baked_to);
		}

		// Copy Distribute-GFX and Distribute-Font output into the platform build.
		// Unity platforms (Switch/Android) use different folder layouts.

		if (p != Platform::PC && p != Platform::Xbox) {
			// If not PC and Xbox
			if (!no_gfx) {
				if (fsys::exists(img_dir)) {
					std::cout << "(Android/Switch) Copying " << img_dir << " to " << base_platform << std::endl;
					fsys::copy(img_dir, base_platform, fsys::copy_options::recursive | fsys::copy_options::overwrite_existing);
				}
				std::cout << "(Android/Switch) GFX: Done!" << std::endl;
			}

			if (!no_font) {
				// TODO: Support Unity fonts?
			}

			std::string const platform_internal = build_platform.filename().string();
			DeleteUnneededPlatforms(before_specific, platform_internal);
		}
		else {
			// If PC or Xbox
			if (!no_gfx) {
				if (fsys::exists(img_dir)) {
					std::cout << "(PC/Xbox) Copying " << img_dir << " to " << base_platform << std::endl;
					fsys::copy(img_dir, build_platform, fsys::copy_options::recursive | fsys::copy_options::overwrite_existing);
				}

				std::cout << "(PC/Xbox) GFX: Done!" << std::endl;
			}

			if (!no_font) {
				if (fsys::exists(font_dir)) {
					std::cout << "(PC/Xbox) Copying " << font_dir << " to " << base_platform << std::endl;
					fsys::copy(font_dir, build_platform, fsys::copy_options::recursive | fsys::copy_options::overwrite_existing);
				}
				std::cout << "(PC/Xbox) Fonts: Done!" << std::endl;
			}
		}

		// Copy .txt logs into Daily‑Log/<platform>/
		// Copy .spc files into Daily‑SPC/<platform>/
		//
		// DAILY uses CopyFilesWithFilter() to preserve relative paths.

		if (!no_cloud) {
			// Copy the .txt files from the current folder to DGRV3-Daily(-Private)-Log
			Files::CopyFilesWithFilter(cur, cur, copy_log_to / platform_name, ".txt", false);

			// Copy the .txt files from ModifiedFiles to DGRV3-Daily(-Private)-SPC
			Files::CopyFilesWithFilter(cur, find_spc, (copy_spc_to / fsys::relative(build_platform, build_fd)).string(), ".spc", true);
			Files::CopyFilesWithFilter(cur, find_spc_gfx, (copy_spc_to / fsys::relative(build_platform, build_fd)).string(), ".spc", true);
			Files::CopyFilesWithFilter(cur, find_spc_font, (copy_spc_to / fsys::relative(build_platform, build_fd)).string(), ".spc", true);
		}

		std::cout << "Renaming folders" << std::endl;

		// Rename Distribute*, ModifiedFiles* folders to include platform name.
		//
		// Example:
		//   Distribute → Distribute-PC
		//   ModifiedFiles → ModifiedFiles-PC
		//
		// This prevents collisions when building multiple platforms in one run.

		try {
			if (std::filesystem::exists(cur / "Distribute")) std::filesystem::rename(cur / "Distribute", cur / ("Distribute-" + platform_name));
			if (std::filesystem::exists(cur / "Distribute-Font")) std::filesystem::rename(cur / "Distribute-Font", cur / ("Distribute-Font-" + platform_name));
			if (std::filesystem::exists(cur / "Distribute-GFX")) std::filesystem::rename(cur / "Distribute-GFX", cur / ("Distribute-GFX-" + platform_name));
			if (std::filesystem::exists(cur / "ModifiedFiles")) std::filesystem::rename(cur / "ModifiedFiles", cur / ("ModifiedFiles-" + platform_name));
			if (std::filesystem::exists(cur / "ModifiedFiles-Font")) std::filesystem::rename(cur / "ModifiedFiles-Font", cur / ("ModifiedFiles-Font-" + platform_name));
			if (std::filesystem::exists(cur / "ModifiedFiles-GFX")) std::filesystem::rename(cur / "ModifiedFiles-GFX", cur / ("ModifiedFiles-GFX-" + platform_name));
		}
		catch (std::filesystem::filesystem_error ec) {
			std::cout << "Error while renaming folders: " << ec.what() << std::endl;
		}
		
		std::cout << "Done renaming folders" << std::endl;
	}

	// IsUnsupportedPlatform()
	// Returns true for platforms DAILY cannot build:
	//
	//   • Android
	//   • iOS
	//   • PS4
	//   • PSVita
	//   • Num (invalid)
	//
	// DAILY removes unsupported platforms before starting the build.

	bool IsUnsupportedPlatform(Platform const& p) {

		bool const ret = p == Platform::Android || p == Platform::iOS || p == Platform::PS4 || p == Platform::PSVita || p == Platform::Num;
		return ret;
	}
}