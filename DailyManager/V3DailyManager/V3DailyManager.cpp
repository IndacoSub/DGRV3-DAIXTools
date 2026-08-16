// Team DAIX, 2026
// V3DAILYMANAGER — DAILY BUILD AUTOMATION SYSTEM
//
// The majority of this code was written between 2020 and 2022
//
// This tool's purpose is:
// V3DailyManager is a completely separate automation pipeline from DAIXTOOLS.
// It acts as a CI/CD system for Danganronpa V3 modding, performing DAILY
// automated builds across multiple platforms.
//
// V3DailyManager handles:
//
//   • Argument parsing and build‑mode selection
//   • Safe‑mode checks to prevent accidental execution in dirty folders
//
//   • Automatic GitHub authentication (username/token/password)
//   • Cloning all Daily repositories:
//         - DGRV3-Daily(-Private)
//         - DGRV3-Daily-Text(-Private)
//         - DGRV3-Daily-Log(-Private)
//         - DGRV3-Daily-SPC(-Private)
//         - DGRV3-Tools (toolchain)
//
//   • Running external toolchains (NOT part of Daily):
//         - TextTools (StackedBooks)
//         - ImageTools (Pianist)
//         - FontTools (Monokuma)
//
//   • Multi‑platform distribution:
//         - PC
//         - Switch (Unity)
//         - Xbox
//         - PS4 / PSVita (unsupported)
//         - Android / iOS (unsupported)
//
//     This includes:
//         - Reconstructing platform‑specific game folder structures
//         - Copying .ups patches into correct game_resident / wrd_data / wrd_script folders
//         - Copying baked text into Daily‑Text
//         - Copying logs into Daily‑Log
//         - Copying SPC files into Daily‑SPC
//         - Copying GFX and font output into platform builds
//         - Cleaning unneeded Unity folders
//         - Renaming Distribute/ModifiedFiles folders per platform
//
//   • Cloud upload of patches, text, logs, and SPCs:
//         - Commit
//         - Tag creation (YYYY.MM.DD)
//         - Push to origin/main
//
//   • Build timing, version stamping, and metadata generation
//   • Writing URL files for external automation
//
// ---------------------------------------------------------------------------
// *** EXTREMELY IMPORTANT WARNING ***
//
// V3DailyManager uses two cleanup functions — DeleteEverything() and
// DeleteEverything2() — which are **high‑risk destructive operations**.
// They will delete **EVERY FILE AND FOLDER THEY FIND**, except for a small
// whitelist of executables and source files.
//
// DAILY MUST ONLY be run inside:
//     • an EMPTY folder,
//     • OR a folder containing ONLY the DAILY executables,
//     • unless the user explicitly provides the --unsafe flag.
//
// Running DAILY without --unsafe is safe: safe‑mode prevents execution in
// non‑empty folders.
//
// Running DAILY with --unsafe WILL DELETE FILES.  
// These functions are intentionally dangerous to guarantee a clean build
// environment. Use with extreme caution.
// ---------------------------------------------------------------------------
//
// DAILY does not compile anything itself — it simply *runs* external tools,
// organizes their output, and publishes the results to GitHub.
//
// This file contains the main orchestration logic for the entire DAILY pipeline.

#include <iostream>
#include <filesystem>
#include <string>
#include <iomanip>
#include <chrono>
#include <array>
#include <algorithm>
#include <set>
#include <thread>
#include <unordered_set>

#include "vgit_utils.h"

#include "remote.h"
#include "common.h"
#include "files.h"
#include "process.h"
#include "distribution.h"

inline bool any_arg = false;

std::pair<bool, int> ArgFound(std::vector<std::string> const& arg_list, std::string const& single_arg);
void HandleArgs(std::vector<std::string> const& arg_list);
void Start(int argc, char** argv);

int main(int argc, char** argv) {

	Common::executeBatch("cls");

	bool has_args = argc > 1;
	std::vector<std::string> arg_list{};

	if (has_args) {
		std::unordered_set<std::string> seen;

		for (int i = 1; i < argc; i++) {
			std::string s{ argv[i] };
			//std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });

			// Only add if not already seen
			if (seen.insert(s).second) {
				arg_list.push_back(s);
			}
		}

		// HandleArgs processes all flags such as:
		//   --pc, --switch, --xbox, --text-only, --gfx-only, --font-only,
		//   --no-text, --no-gfx, --no-font, --no-cloud,
		//   --text-config, --gfx-config, --font-config,
		//   --public, --unsafe, --gui, --all
		//
		// These flags determine what DAILY will build and upload.

		HandleArgs(arg_list);
	}

	if (!any_arg) {
		std::this_thread::sleep_for(std::chrono::seconds(4));
		std::cout << "Ok then..." << std::endl;
	}

	if (no_text && no_gfx && no_font) {
		std::cout << "Contradicting arguments!" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << std::endl;

	Start(argc, argv);

	return EXIT_SUCCESS;
}

// ArgFound checks whether a specific argument exists in the list.
// It also updates the global any_arg flag to indicate that at least
// one argument was recognized.

std::pair<bool, int> ArgFound(std::vector<std::string> const& arg_list, std::string const& single_arg)
{
	int ret = -1;
	auto const arg_it = std::find(arg_list.begin(), arg_list.end(), single_arg);
	bool const found_arg = arg_it != arg_list.end();

	if (found_arg) {
		std::cout << "Arg recognized: " << single_arg << std::endl;
		ret = std::distance(arg_list.begin(), arg_it);
	}

	any_arg |= found_arg;

	return { found_arg, ret };
}

// HandleArgs interprets all command‑line flags and sets global state:
//
//   • private_repo / public_repo
//   • build_platforms (PC, Switch, Xbox, etc.)
//   • no_text / no_gfx / no_font
//   • no_cloud
//   • specific_*_config overrides
//   • safe_mode / gui mode
//
// DAILY uses these flags to determine which pipelines to run and which
// repositories to upload to.

void HandleArgs(std::vector<std::string> const& arg_list) {
	bool any_arg_found = false;

	for (size_t i = 0; i < arg_list.size(); ++i) {
		const std::string& arg = arg_list[i];

		// next_arg_available retrieves the next argument if it is not another flag.
		// Used for config overrides like --text-config <file>.

		auto next_arg_available = [&](std::string& out) -> bool {
			if (i + 1 < arg_list.size() && !Common::StringStartsWith(arg_list[i + 1], "--")) {
				out = arg_list[++i]; // consume next
				return true;
			}
			return false;
		};

		if (arg == "--public") {
			private_repo = false;
			any_arg_found = true;
		}
		else if (arg == "--pc") {
			build_platforms.push_back(Platform::PC);
			any_arg_found = true;
		}
		else if (arg == "--switch") {
			build_platforms.push_back(Platform::Switch);
			any_arg_found = true;
		}
		else if (arg == "--xbox") {
			build_platforms.push_back(Platform::Xbox);
			any_arg_found = true;
		}
		else if (arg == "--psvita") {
			build_platforms.push_back(Platform::PSVita);
			any_arg_found = true;
		}
		else if (arg == "--ps4") {
			build_platforms.push_back(Platform::PS4);
			any_arg_found = true;
		}
		else if (arg == "--android") {
			build_platforms.push_back(Platform::Android);
			any_arg_found = true;
		}
		else if (arg == "--ios") {
			build_platforms.push_back(Platform::iOS);
			any_arg_found = true;
		}
		else if (arg == "--all") {
			all_platforms = true;
			any_arg_found = true;
		}
		else if (arg == "--text-only") {
			no_gfx = true;
			no_font = true;
			any_arg_found = true;
			std::cout << "Text: " << (no_text ? "false" : "true") << "\n";
			std::cout << "GFX: " << (no_gfx ? "false" : "true") << "\n";
			std::cout << "Font: " << (no_font ? "false" : "true") << "\n";
		}
		else if (arg == "--gfx-only") {
			no_text = true;
			no_font = true;
			any_arg_found = true;
			std::cout << "Text: " << (no_text ? "false" : "true") << "\n";
			std::cout << "GFX: " << (no_gfx ? "false" : "true") << "\n";
			std::cout << "Font: " << (no_font ? "false" : "true") << "\n";
		}
		else if (arg == "--font-only") {
			no_text = true;
			no_gfx = true;
			any_arg_found = true;
			std::cout << "Text: " << (no_text ? "false" : "true") << "\n";
			std::cout << "GFX: " << (no_gfx ? "false" : "true") << "\n";
			std::cout << "Font: " << (no_font ? "false" : "true") << "\n";
		}
		else if (arg == "--no-text") {
			no_text = true;
			any_arg_found = true;
			std::cout << "Text: " << (no_text ? "false" : "true") << "\n";
		}
		else if (arg == "--no-gfx") {
			no_gfx = true;
			any_arg_found = true;
			std::cout << "GFX: " << (no_gfx ? "false" : "true") << "\n";
		}
		else if (arg == "--no-font") {
			no_font = true;
			any_arg_found = true;
			std::cout << "Font: " << (no_font ? "false" : "true") << "\n";
		}
		else if (arg == "--no-cloud") {
			no_cloud = true;
			any_arg_found = true;
			std::cout << "Cloud: " << (no_cloud ? "false" : "true") << "\n";
		}
		else if (arg == "--text-config") {
			std::string val;
			if (next_arg_available(val)) {
				specific_text_config = val;
				std::cout << "Text Config: " << val << "\n";
			}
			else {
				std::cout << "Invalid or missing text config\n";
			}
			any_arg_found = true;
		}
		else if (arg == "--gfx-config") {
			std::string val;
			if (next_arg_available(val)) {
				specific_gfx_config = val;
				std::cout << "GFX Config: " << val << "\n";
			}
			any_arg_found = true;
		}
		else if (arg == "--font-config") {
			std::string val;
			if (next_arg_available(val)) {
				specific_font_config = val;
				std::cout << "Font Config: " << val << "\n";
			}
			any_arg_found = true;
		}
		else if (arg == "--gui") {
			has_gui = true;
			any_arg_found = true;
		}
		else if (arg == "--unsafe") {
			safe_mode = false;
			any_arg_found = true;
		}
	}

	if (!any_arg_found) {
		std::cout << "No args found / recognized? Are you sure?\n";
		any_arg = false;
	}
	else {
		any_arg |= any_arg_found;
	}
}



void Start(int argc, char** argv) {
	// DAILY begins here.
	// Determine current directory and locate DGRV3-Tools (the toolchain repo).
	// See process.cpp header for more info about DGRV3-Tools

	fsys::path const current_dir = fsys::current_path();
	fsys::path const where = current_dir / "DGRV3-Tools";

	// GitHub credentials for automatic login.
	// DAILY uses these to clone and push to private/public Daily repositories.
	// (User must provide their own credentials.)

	// GitHub Username
	static const std::string gh_username = EncryptString(""); // Provide your own!

	// GitHub Password
	static const std::string gh_password = EncryptString(""); // Provide your own!

	// GitHub Token
	static const std::string gh_token = EncryptString(""); // Provide your own!

	// Tools repository name (and also folder)
	const std::string tools_repo_name = EncryptString("DGRV3-Tools");

	// Determine which Daily repositories to use based on private_repo flag:
	//   • DGRV3-Daily(-Private)
	//   • DGRV3-Daily-Text(-Private)
	//   • DGRV3-Daily-Log(-Private)
	//   • DGRV3-Daily-SPC(-Private)

	// Patches repository name (and also folder)
	const std::string upload_patches_repo_name = private_repo ? EncryptString("DGRV3-Daily-Private") : EncryptString("DGRV3-Daily");

	// Text repository name (and also folder)
	const std::string upload_text_repo_name = private_repo ? EncryptString("DGRV3-Daily-Private-Text") : EncryptString("DGRV3-Daily-Text");

	// Log repository name (and also folder)
	const std::string upload_log_repo_name = private_repo ? EncryptString("DGRV3-Daily-Private-Log") : EncryptString("DGRV3-Daily-Log");

	// SPC repository name (and also folder)
	const std::string upload_spc_repo_name = private_repo ? EncryptString("DGRV3-Daily-Private-SPC") : EncryptString("DGRV3-Daily-SPC");

	// This is a list of branches that we *don't* want to compile as beta
	// taken_branches lists branches that DAILY should never treat as beta builds.

	std::vector<std::string> static const taken_branches{
		MAIN_BRANCH_2,
		"HEAD",
	};

	// TODO: settings namespaces
	std::string const version = "V3DailyManager v2.3";

	std::cout << version << std::endl;
	std::cout << std::endl;

	auto const time_start = std::chrono::system_clock::now();

	bool stop_start = false;

	// Safe mode prevents DAILY from running in a directory containing random files.
	// This avoids accidental destructive operations.

	std::cout << "Safe mode: " << (safe_mode ? "on" : "off") << std::endl;

	if (safe_mode) {
		std::uint64_t count = Files::CountFilesInPath(current_dir);
		std::cout << "Files in directory: " << count << std::endl;
		if (count > 1) {
			stop_start = true;
			if (has_gui) {
				if (count <= 2) {
					stop_start = false;
				}
			}
		}
	}

	if (stop_start) {
		std::cout << "Refusing to start because of the files inside this folder." << std::endl;
		return;
	}

	// Basically login automatically
	std::string const& tools_repo_url = VGitUtils::CalculateRepoURL(gh_username, gh_token, gh_password, gh_username, tools_repo_name);
	std::string const& repo_url_upload_patches = VGitUtils::CalculateRepoURL(gh_username, gh_token, gh_password, gh_username, upload_patches_repo_name);
	std::string const& repo_url_upload_text = VGitUtils::CalculateRepoURL(gh_username, gh_token, gh_password, gh_username, upload_text_repo_name);
	std::string const& repo_url_upload_log = VGitUtils::CalculateRepoURL(gh_username, gh_token, gh_password, gh_username, upload_log_repo_name);
	std::string const& repo_url_upload_spc = VGitUtils::CalculateRepoURL(gh_username, gh_token, gh_password, gh_username, upload_spc_repo_name);

	std::cout << "Selected \"Daily\" repositories: " <<
		upload_patches_repo_name << " / " << upload_text_repo_name << " / " << upload_log_repo_name << " / " << upload_spc_repo_name << std::endl;
	std::cout << std::endl;

	// Add new platforms manually if needed

	// If --all was specified, include every supported platform.
	// Otherwise default to PC if none were specified.

	if (all_platforms) {
		build_platforms.push_back(Platform::PC);
		build_platforms.push_back(Platform::Switch);
		build_platforms.push_back(Platform::Xbox);
		build_platforms.push_back(Platform::PSVita);
		build_platforms.push_back(Platform::PS4);
		build_platforms.push_back(Platform::Android);
		build_platforms.push_back(Platform::iOS);
	}
	else {
		if (build_platforms.empty()) {
			build_platforms.push_back(Platform::PC);
		}
	}

	// Deduplicate and sort platform list.
	if (build_platforms.size() > 1) {
		std::sort(build_platforms.begin(), build_platforms.end());

		std::set<Platform> s{};
		for (int i = 0; i < build_platforms.size(); i++) {
			s.insert(build_platforms[i]);
		}
		build_platforms.clear();
		build_platforms.assign(s.begin(), s.end());
	}

	std::cout << "--------------------------------------" << std::endl;

	std::cout << "Text: " << (no_text ? "false" : "true") << "\n";
	std::cout << "GFX: " << (no_gfx ? "false" : "true") << "\n";
	std::cout << "Font: " << (no_font ? "false" : "true") << "\n";

	std::cout << "Attempted build platform(s):" << std::endl;
	for (auto const& platform : build_platforms) {
		std::cout << Process::GetPlatformName(platform) << std::endl;
	}
	std::cout << std::endl;

	for (int i = build_platforms.size() - 1; i >= 0; i--) {
		Platform const p = build_platforms[i];
		bool const is_unsupported = Distribution::IsUnsupportedPlatform(p);
		// Remove unsupported platforms (e.g., ones not implemented yet).

		if (is_unsupported) {
			std::cout << "Removed platform " << Process::GetPlatformName(p) << " as it is not supported." << std::endl;
			build_platforms.erase(build_platforms.begin() + i);
		}
	}

	if (build_platforms.size() <= 0) {
		std::cout << "No supported version(s) found!" << std::endl;
		return;
	}

	std::cout << std::endl;
	std::cout << "Final build platform(s):" << std::endl;
	for (auto const& platform : build_platforms) {
		std::cout << Process::GetPlatformName(platform) << std::endl;
	}
	std::cout << std::endl;

	// Delete everything inside DGRV3-Tools to ensure a clean environment.
	Files::DeleteEverything(where, current_dir);

	// Clone the main toolchain repository (DGRV3-Tools).
	Remote::Clone(tools_repo_name, tools_repo_url, current_dir, true, taken_branches);

	std::string const build_folder = "LatestAutomaticBuild";
	fsys::path const mdis = current_dir / "Distribute";
	fsys::path const build_fd = (current_dir / upload_patches_repo_name) / build_folder;
	fsys::path const autov3 = (current_dir / "autov3.txt");

	bool static constexpr force_update = true;

	// If force_update is false, DAILY checks whether TextTools needs to run.
	// This is a lightweight optimization for PC builds.

	if (!force_update) {
		// PC is default as long as nobody complains
		Platform const default_platform = build_platforms.size() <= 0 ? Platform::PC : build_platforms[0];

		std::cout << std::endl;
		std::cout << "Building platform " << Process::GetPlatformName(default_platform) << " first." << std::endl;
		std::cout << std::endl;

		if (!no_text) {
			// Run TextTools from DGRV3-Tools, config specified in the function?
			// Just check if the default version is different, I guess?
			// Ehh, nobody will complain right?
			Process::RunTextTools(where, current_dir, default_platform);
		}
		else {
			std::cout << "The no-text flag is active, what am I supposed to do?" << std::endl;
			return;
		}
	}

	// Clone DGRV3-Daily(-Private) patches repository.
	Remote::Clone(upload_patches_repo_name, repo_url_upload_patches, current_dir, false, {});

	// If it doesn't exist, create the "LatestAutomaticBuild" folder
	if (!fsys::exists(build_fd)) {
		fsys::create_directory(build_fd);
	}

	// Shouldn't this also be done for the graphics and fonts?
	// AFAIK they use different folders
	// Collect .ups files from the Distribute folder (from DGRV3).
	// DAILY compares these against the Daily repository to detect changes.

	std::vector<std::string> modifiedups_vec{};
	if (fsys::exists(mdis)) {
		for (auto const& file : fsys::recursive_directory_iterator(mdis)) {
			if (!file.is_directory() && (file.path().string().find(".ups") != std::string::npos)) {
				modifiedups_vec.push_back(file.path().string());
			}
		}
	}

	// Check if any .ups files are different between Daily and DGRV3

	bool const comparison_good = force_update ? false : Files::CheckDifferent(build_fd, modifiedups_vec);

	std::stringstream info{};
	std::uint64_t process_sec_n = 0;
	std::uint64_t process_min_n = 0;
	std::chrono::system_clock::time_point process_time_end{};
	std::chrono::seconds process_sec{};

	// If no differences are found and force_update is false,
	// DAILY exits early to avoid unnecessary builds.
	if (comparison_good) {
		std::cout << "Nothing new, nothing to do." << std::endl;
		std::cout << "Returning (exiting)" << std::endl;
		return;
	}

	if (!force_update) {
		std::cout << "Found different files, compiling and distributing!" << std::endl;
	}

	info << " | ";
	info << version;
	info << " | ";
	info << "Platform(s): ";

#ifndef _WIN32
	Common::executeBatch("chmod -R +x *");
#endif

	fsys::path const find_baked = (current_dir / "Baked");
	fsys::path const copy_baked_to = (current_dir / upload_text_repo_name);
	fsys::path const copy_log_to = (current_dir / upload_log_repo_name);
	fsys::path const find_spc = (current_dir / "ModifiedFiles");
	fsys::path const find_spc_font = (current_dir / "ModifiedFiles-Font");
	fsys::path const find_spc_gfx = (current_dir / "ModifiedFiles-GFX");
	fsys::path const copy_spc_to = (current_dir / upload_spc_repo_name);

	// Clone Daily text/log/SPC repositories if cloud upload is enabled.

	if (!no_text) {
		if (!no_cloud) {
			// Clone DGRV3-Daily(-Private)-Text
			Remote::Clone(upload_text_repo_name, repo_url_upload_text, current_dir, false, {});
		}
	}

	if (!no_cloud) {
		// Clone DGRV3-Daily(-Private)-Log
		Remote::Clone(upload_log_repo_name, repo_url_upload_log, current_dir, false, {});

		// Clone DGRV3-Daily(-Private)-SPC
		Remote::Clone(upload_spc_repo_name, repo_url_upload_spc, current_dir, false, {});
	}

	// THIS IS THE PART WHERE IT COMPILES FOR MULTIPLE PLATFORMS

	// For each selected platform, run Distribution::DistributeFiles.
	// This copies baked text, logs, SPCs, GFX, and font patches into the
	// appropriate Daily repositories.
	//
	// DAILY does NOT compile anything itself — it only distributes output
	// produced by external toolchains.

	int i = -1;
	for (auto const& cur_platform : build_platforms) {
		i++;
		Distribution::DistributeFiles(cur_platform, build_fd, current_dir, current_dir / "Distribute-GFX", current_dir / "Distribute-Font", where, mdis, find_baked, copy_baked_to, copy_log_to, find_spc, copy_spc_to, find_spc_gfx, find_spc_font);
		// Avoid saying "All", say each one individually
		auto pname = Process::GetPlatformName(cur_platform);
		std::cout << pname << " DONE" << std::endl;
		info << pname;
		if (i < build_platforms.size() - 1) {
			info << ", ";
		}
	}

	// HERE, IT ALREADY COMPILED FOR ALL PLATFORMS

	std::cout << "Distribution done!" << std::endl;

	// Remove folders that should not be uploaded (x64, V3DailyManager, Release).

	if (fsys::exists(current_dir / upload_patches_repo_name / "x64")) {
		fsys::remove_all(current_dir / upload_patches_repo_name / "x64");
	}

	if (fsys::exists(current_dir / upload_patches_repo_name / "V3DailyManager")) {
		fsys::remove_all(current_dir / upload_patches_repo_name / "V3DailyManager");
	}

	if (fsys::exists(current_dir / upload_patches_repo_name / "Release")) {
		fsys::remove_all(current_dir / upload_patches_repo_name / "Release");
	}

	Files::DeleteUPSNormals(current_dir / upload_patches_repo_name);

#ifndef _WIN32
	Common::executeBatch("chmod -R +x *");
#endif

	process_time_end = std::chrono::system_clock::now();
	process_sec = std::chrono::duration_cast<std::chrono::seconds>(process_time_end - time_start);
	process_sec_n = process_sec.count();
	process_min_n = static_cast<std::uint64_t>(static_cast<float>(process_sec_n) / 60.0f);
	for (int j = 0; j < process_min_n; j++) {
		process_sec_n -= 60;
	}

	// Upload patches, text, logs, and SPCs to their respective Daily repositories.
	// UploadToCloud performs commit + push + tag creation.

	bool specifically_no_upload = false;


	if (!no_cloud && !specifically_no_upload) {
		std::cout << "Uploading patches..." << std::endl;
		Remote::UploadToCloud(current_dir, upload_patches_repo_name, info.str());
		// Remove when public Daily alternatives to SPC, Log and Text exist
		if (private_repo || true) {
			std::cout << "private_repo || b" << std::endl;
			if (!no_text) {
				if (std::filesystem::exists(current_dir / upload_text_repo_name)) {
					// NOT RANDOMIZED, ONLY BAKED
					std::cout << "Uploading text..." << std::endl;
					Remote::UploadToCloud(current_dir, upload_text_repo_name, info.str());
				}
				else {
					std::cout << "Upload text repository does not exist" << std::endl;
				}
			}
			else {
				std::cout << "no_text" << std::endl;
			}
			if (std::filesystem::exists(current_dir / upload_log_repo_name)) {
				std::cout << "Uploading logs..." << std::endl;
				Remote::UploadToCloud(current_dir, upload_log_repo_name, info.str());
			}
			else {
				std::cout << "Upload logs repository does not exist" << std::endl;
			}
			if (std::filesystem::exists(current_dir / upload_spc_repo_name)) {
				std::cout << "Uploading SPCs..." << std::endl;
				Remote::UploadToCloud(current_dir, upload_spc_repo_name, info.str());
			}
			else {
				std::cout << "Upload SPC repository does not exist" << std::endl;
			}
		}
		else {
			std::cout << "!private_repo" << std::endl;
		}
		std::cout << "Uploaded!" << std::endl;
	}
	else {
		std::cout << "No cloud" << std::endl;
	}

	// Write the final GitHub URLs for patches/text/log/SPC into text files.
	// These are used by external automation or user scripts.

	std::ofstream patches_txt_url("patches_url.txt", std::ios::out | std::ios::app);
	patches_txt_url << (no_cloud ? "Unavailable" : VGitUtils::GetActualRepoURL(gh_username, upload_patches_repo_name) + "/tags/") << std::endl;
	patches_txt_url.close();

	std::ofstream text_txt_url("text_url.txt", std::ios::out | std::ios::app);
	text_txt_url << (no_cloud ? "Unavailable" : VGitUtils::GetActualRepoURL(gh_username, upload_text_repo_name) + "/tags/") << std::endl;
	text_txt_url.close();

	std::ofstream log_txt_url("log_url.txt", std::ios::out | std::ios::app);
	log_txt_url << (no_cloud ? "Unavailable" : VGitUtils::GetActualRepoURL(gh_username, upload_log_repo_name) + "/tags/") << std::endl;
	log_txt_url.close();

	std::ofstream spc_txt_url("spc_url.txt", std::ios::out | std::ios::app);
	spc_txt_url << (no_cloud ? "Unavailable" : VGitUtils::GetActualRepoURL(gh_username, upload_spc_repo_name) + "/tags/") << std::endl;
	spc_txt_url.close();

	auto const total_time_end = std::chrono::system_clock::now();
	auto const total_sec = std::chrono::duration_cast<std::chrono::seconds>(total_time_end - time_start);
	std::uint64_t total_sec_n = total_sec.count();
	std::uint64_t const total_min_n = static_cast<std::uint64_t>(static_cast<float>(total_sec_n) / 60.0f);
	for (int j = 0; j < total_min_n; j++) {
		total_sec_n -= 60;
	}

	auto const upload_time_end = (total_time_end - process_time_end);
	auto const upload_sec = std::chrono::duration_cast<std::chrono::seconds>(upload_time_end);
	std::uint64_t upload_sec_n = upload_sec.count();
	std::uint64_t const upload_min_n = static_cast<std::uint64_t>(static_cast<float>(upload_sec_n) / 60.0f);
	for (int j = 0; j < upload_min_n; j++) {
		upload_sec_n -= 60;
	}

	std::cout << std::endl;
	std::cout << "Everything took " << total_min_n << " minutes " << " and " << total_sec_n << " seconds:" << std::endl;
	std::cout << "Processing took " << process_min_n << " minutes " << " and " << process_sec_n << " seconds!" << std::endl;
	std::cout << "Uploading took " << upload_min_n << " minutes " << " and " << upload_sec_n << " seconds!" << std::endl;
	std::cout << std::endl;
	std::cout << "Done :)" << std::endl;No,

	if (fsys::exists(autov3)) {
		fsys::remove(autov3);
	}
}