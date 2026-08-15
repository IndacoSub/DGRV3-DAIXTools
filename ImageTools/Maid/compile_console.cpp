// Team DAIX, 2026
// MAID — CONSOLE COMPILATION PIPELINE

// The majority of this code was written between 2020 and 2022

// This tool's purpose is:
// Maid’s console pipeline performs PNG/TGA → AssetBundle/sharedassets injection
// for the Nintendo Switch / Unity-based versions of Danganronpa V3. It handles:
//   • PNG/TGA preprocessing (folder normalization, asset/bundle detection)
//   • FileOnDemand cloud fallback for missing .ab/.assets files
//   • Unity asset injection via UAFGJ
//   • Multithreaded AssetBundle/sharedassets compilation
//   • Automatic grouping of PNGs by their target container
//   • Manual injection of SDF font files into sharedassets0.assets
//
// Unlike the PC pipeline, console builds do not use SRD/SPC archives. Instead,
// all textures are inserted directly into Unity AssetBundles (.ab) or engine
// sharedassets files (.assets). This pipeline mirrors the structure of the PC
// workflow but adapts it to Unity’s asset system.

#include <regex>
#include <thread>

#include "compile_console.h"

#include "../Common/Cloud.h"
#include "../Common/Common.h"

namespace Console {

	// Main entry point for console (Switch/Unity) image compilation.
	// Console builds use Unity AssetBundles (.ab) and sharedassets files instead of SPC/SRD.

	bool CompileConsole(std::filesystem::path const& current_dir, std::string const& repo, std::string const& basefolder) {

		std::filesystem::path const dgrv3path = current_dir / repo;

		std::vector<std::string> seen{};

		// List of files to compile
		// Scan the graphics repository for PNG/TGA files and determine which AssetBundle
		// or sharedassets file each one belongs to. Builds the FileStructure list.

		std::vector<Common::FileStructure> const fs_vec = PreprocessPNG(dgrv3path, dgrv3path / basefolder / Cloud::dl_repo_name, repo, std::ref(seen));

		// Compile all modified PNGs into their corresponding AssetBundles or sharedassets files.
		CompileImages(fs_vec, seen);

		auto assets = dgrv3path / basefolder / Cloud::dl_repo_name / "Data" / "sharedassets0.assets";

		// List of font files to inject
		// Unity fonts must be manually injected into sharedassets0.assets.
		// These text files contain SDF font data used by the engine.

		std::vector<std::filesystem::path> fonts = {
			dgrv3path / "Data" / "sharedassets0" / "FOT-HummingStd-D.txt",
			dgrv3path / "Data" / "sharedassets0" / "FOT-NewRodinPro-DB SDF.txt"
		};

		// Inject each font into sharedassets0.assets using UAFGJ.
		// UAFGJ is Team DAIX’s Unity asset injector, built on top of UABEA’s
		// asset/bundle manipulation APIs. It can replace textures and text
		// inside .assets and .ab files by rewriting Unity’s serialized data.
		// Skip if either the font file or the target asset is missing.
		// UAFGJ: https://github.com/IndacoSub/UABEA/tree/custom

		for (const auto& font : fonts) {
			if (std::filesystem::exists(font) && std::filesystem::exists(assets)) {
				CalculateABOrAsset(current_dir, font.string(), assets.string());
			}
			else {
				LOG("Skipping font injection: missing " + font.string() + " or " + assets.string(), HERE, "Maid");
			}
		}

		return true;
	}

	// Compile all AssetBundles / sharedassets files referenced by PNGs.
	// Handles downloading missing files (FileOnDemand), sorting, and multithreaded processing.

	void CompileImages(std::vector<Common::FileStructure> const& fs_vec, std::vector<std::string> const& seen) {

		// FileOnDemand: download only the specific AssetBundles needed for compilation
		// instead of cloning the entire graphics repository.

		if (Configuration::ConfigMap["FileOnDemand"]) {
			LOG("Downloading ABs / Assets...", HERE, "Maid");

			// Iterate through files that have been "seen"
			// For each required AssetBundle, download it if missing.
			// DownloadABOrAsset() handles path normalization and retry logic.

			for (auto const& file : seen) {
				if (!std::filesystem::exists(file)) {
					DownloadABOrAsset(file, file, 0);	// Same argument for both...?
				}
			}
		}

		LOG("Sorting AB / Assets...", HERE, "Maid");

		// Group PNG entries by their target AssetBundle/sharedassets file.
		// This allows parallel compilation per container.

		Common::SplitType const split_ab = SortEntriesByABOrAsset(fs_vec, seen);

		LOG("Calculating " + std::to_string(split_ab.size()) + " ABs / Assets...", HERE, "Maid");

		auto const start_time = std::chrono::system_clock::now();

		// Compile each AssetBundle in parallel if enabled.
		// Each thread processes all PNGs belonging to a single container.

		for (std::uint64_t en = 0; en < split_ab.size(); ++en) {

			if (Configuration::ConfigMap["MultithreadedCompilation"]) {
				std::jthread{ &CalculateAllEntriesABOrAsset, split_ab[en] }.detach();
			}
			else {
				CalculateAllEntriesABOrAsset(split_ab[en]);
			}
		}

		// This whole block is "wait for ABs to compile"
		// Wait for all threads to finish. Timeout after 5 minutes to avoid infinite hangs.

		if (Configuration::ConfigMap["MultithreadedCompilation"]) {
			while (ABOrAssetCompiled < split_ab.size()) {
				auto const now = std::chrono::system_clock::now();
				auto const diff = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);
				if (diff.count() > 300) {
					LOG("ERROR: Too much time passed calculating ABs!", HERE, "Maid");
					return;
				}
			}
		}

		LOG("Done!", HERE, "Maid");
	}

	// Scan the repo for PNG/TGA files and determine their target AssetBundle or sharedassets file.
	// Builds FileStructure entries and tracks which containers are needed.

	std::vector<Common::FileStructure> PreprocessPNG(std::filesystem::path const& dgrv3path, std::filesystem::path const& where, std::string const& repo, std::vector<std::string>& seen) {

		std::vector<Common::FileStructure> fs_vec{};

		// Iterate through the repository folder (DGRV3-AB-GFX)
		// And look for .png files inside any subdirectory
		for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path)) {

			if (file.is_directory()) {
				// We don't care about folders, we only need to find .png
				continue;
			}

			std::string const file_str = file.path().string();

			// Accept .png, .PNG, .tga, .TGA — console assets may use either format.

			std::size_t find_png = file_str.find(".png");
			if (find_png == std::string::npos || find_png == 0) {
				// Invalid file or name
				find_png = file_str.find(".PNG");
				if (find_png == std::string::npos || find_png == 0) {
					// Try .tga
					find_png = file_str.find(".tga");
					if (find_png == std::string::npos || find_png == 0) {
						find_png = file_str.find(".TGA");
						if (find_png == std::string::npos || find_png == 0) {
							continue;
						}
					}
				}
			}

			// We don't want ALTs (even if the option is enabled)

			if (Common::StringContains(file_str, "ALT_")) {
				continue;
			}

			std::size_t const find_repo = file_str.find(repo);
			if (find_repo == std::string::npos || find_repo == 0) {
				continue;
			}

			Common::FileStructure fs{};
			fs.PNG_RepoName = file_str;
			fs.PNG_ActualName = file_str;

			// Convert repo-relative folder structure into the correct AssetBundle/sharedassets filename.
			// Unity uses "sharedassetsX.assets" for engine assets and ".ab" for bundles.

			std::string const file_str_nofile = std::filesystem::path(file_str).parent_path().string();
			std::string normalized_folder = file_str_nofile.substr(find_repo);
			normalized_folder = normalized_folder.substr(repo.length() + 1);
			std::size_t const contains_shared = normalized_folder.find("sharedassets");
			bool const is_asset = contains_shared != std::string::npos;
			normalized_folder += (is_asset ? ".assets" : ".ab");

			fs.AssetOrBundle_Filename = std::filesystem::path(where / normalized_folder).string();

			std::string const a_or_b = fs.AssetOrBundle_Filename;
			//std::transform(a_or_b.begin(), a_or_b.end(), a_or_b.begin(),
				//[](unsigned char c) { return std::tolower(c); });
			auto const find_in_seen = std::find(seen.begin(), seen.end(), a_or_b);
			if (find_in_seen == seen.end()) {
				seen.push_back(a_or_b);
			}

			fs_vec.push_back(fs);
		}

		return fs_vec;
	}

	// Called in CompileImages() above
	// Download a single AssetBundle or sharedassets file from the cloud repo.
	// Handles path normalization, case-insensitive retries, and copying into the working folder.

	bool DownloadABOrAsset(std::string const& output_file, std::string const& dl_repo_path, int const& tryno) {

		std::string const dl_output = output_file;
		std::string folder = dl_repo_path;
		std::size_t const findrepo = folder.find(Cloud::dl_repo_name);
		if (findrepo == std::string::npos) {
			LOG("ERROR: Invalid download repository: " + Cloud::dl_repo_name + " (couldn't be find inside " + folder + ")!", HERE, "Maid");
			return false;
		}
		folder = folder.substr(findrepo);
		folder = folder.substr(Cloud::dl_repo_name.length() + 1);
		//folder = std::regex_replace(folder, std::regex("\\"), "//");

		std::replace(folder.begin(), folder.end(), '\\', '/');

		std::string const output_ab_noname = std::filesystem::path(output_file).parent_path().string();
		std::filesystem::create_directories(output_ab_noname);

		// Download raw file from GitHub using authenticated curl request.
		// Uses --retry to handle transient network failures.

		std::string const command = "curl -s -H \"Authorization: token " + Cloud::dl_repo_token + "\" -L https://raw.githubusercontent.com/" +
			Cloud::dl_repo_owner + "/" + Cloud::dl_repo_name + "/" + Cloud::dl_branch + "/" + folder +
			" --retry 3 --retry-delay 1 --create-dirs -o " + dl_output;

		// System or Common::ExecuteBatch?
		if (Common::executeBatch(command.c_str(), {}, true) != 0) {
			// Log
		}

		// GitHub raw returns "404: Not Found" as file content, not HTTP status.
		// Detect this to avoid treating invalid downloads as valid files.

		bool const ab_exists = std::filesystem::exists(output_file);
		bool valid_file = true;
		if (ab_exists) {
			std::ifstream inab(output_file, std::ios::in);
			std::stringstream ss{};
			ss << inab.rdbuf();
			inab.close();
			valid_file = ss.str() != "404: Not Found";
		}

		if (!ab_exists || !valid_file) {
			LOG("ERROR: Couldn't download: " + folder, HERE, "Maid");
			if (dl_repo_path.length() > 0) {
				std::string input = dl_repo_path;
				std::filesystem::path p(input);
				std::string const url = p.parent_path().string();
				std::string const filename = p.filename().string();
				std::string out_allcaps = filename;
				std::string out_nocaps = filename;
				std::transform(out_allcaps.begin(), out_allcaps.end(), out_allcaps.begin(), [](char a) {return std::toupper(a); });
				std::transform(out_nocaps.begin(), out_nocaps.end(), out_nocaps.begin(), [](char a) {return std::tolower(a); });
				std::string selected{};
				switch (tryno) {
				case 0:
					selected = out_nocaps;
					break;
				case 1:
					selected = out_allcaps;
					break;
				default:
					break;
				}
				p = p.parent_path();
				std::filesystem::path const ret = p / selected;
				input = ret.string();
				// Retry download using lowercase and uppercase filenames.

				if (input != selected && tryno <= 1) {
					bool const retry = DownloadABOrAsset(input, input, tryno + 1);	// Let's try both
					if (!retry) {
						return false;
					}
				}
				else {
					LOG("ERROR: Complete failure when trying to download " + folder, HERE, "Maid");
				}
			}
		}
		else {
			if (tryno > 0) {
				LOG("Managed to download " + folder + " on the " + std::to_string(tryno) + " try!", HERE, "Maid");
			}
		}

		std::string selected_out = dl_output;
		selected_out = std::regex_replace(selected_out, std::regex(Cloud::dl_repo_name), "danganronpa_files_copy");
		std::string const selout = std::filesystem::path(selected_out).parent_path().string();
		std::filesystem::create_directories(selout);

		try {
			std::filesystem::copy_file(dl_output, selected_out, std::filesystem::copy_options::overwrite_existing);
		}
		catch (const std::filesystem::filesystem_error& e) {
			LOG(std::string("ERROR copying file: ") + dl_output + " to " + selected_out + " - " + e.what(), HERE, "Maid");
		}

		return true;
	}

	// Group FileStructure entries by their target AssetBundle/sharedassets file.
	// Each group corresponds to one compilation thread.

	Common::SplitType SortEntriesByABOrAsset(std::vector<Common::FileStructure> const& entries, std::vector<std::string> const& container_list) {

		Common::SplitType split{ {{}} };
		split.resize(entries.size());

		for (auto const& en : entries) {
			// Multithread by AB_Filename
			// Finds .ab / .assets using std::find and parallelizes work
			// Result in a 30-35% increase in performance

			std::string const name = en.AssetOrBundle_Filename;
			//std::transform(name.begin(), name.end(), name.begin(),
				//[](unsigned char c) { return std::tolower(c); });

			auto const it = std::find(container_list.begin(), container_list.end(), name);
			if (it == container_list.end() || name.empty()) {
				// Couldn't be found...?
				LOG("ERROR: Invalid .ab or .assets file: " + std::to_string(std::distance(container_list.begin(), it)), HERE, "Maid");
				continue;
			}

			std::uint64_t const distance = std::distance(container_list.begin(), it);
			if (distance >= container_list.size()) {
				// Invalid index...?
				LOG("ERROR: Invalid index", HERE, "Maid");
				return {};
			}

			if (distance >= split.size()) {
				// Invalid array...?
				LOG("ERROR: Invalid array (distance: " + std::to_string(distance) + ", array size: " + std::to_string(split.size()) + ")", HERE, "Maid");
				return {};
			}

			// Add to an array sorted by .ab or .assets
			split[distance].push_back(en);
		}

		return split;
	}

	// Compile all PNGs belonging to a single AssetBundle/sharedassets file.
	// Calls UAFGJ for each PNG and increments the global counter.

	void CalculateAllEntriesABOrAsset(std::vector<Common::FileStructure> const& entries) {

		std::filesystem::path const current_dir = std::filesystem::current_path();

		for (auto const& en : entries) {
			CalculateABOrAsset(current_dir, en.PNG_ActualName, en.AssetOrBundle_Filename);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		++ABOrAssetCompiled;
	}

	// Invoke UAFGJ to insert a PNG into an AssetBundle/sharedassets file.
	// UAFGJ handles Unity asset injection for console builds.
	// It uses UABEA’s asset parsing to locate Texture2D objects and replace
	// their serialized image data with a newly encoded PNG (usually DXT5 or NX format).

	void CalculateABOrAsset(std::filesystem::path const& uafgj_loc, std::string const& input_file, std::string const& container) {

		if (container.empty()) {
			return;
		}

#ifdef _WIN32
		std::string const ext = ".exe";
#else
		std::string const ext = "";
#endif

		std::filesystem::path const uafgj_path = (uafgj_loc / ("UAFGJ" + ext));

#ifdef _WIN32
		std::string const uafgj_command = "\"" + uafgj_path.string() + "\" \"" + container + "\" \"" + input_file + "\"";
#else
		std::string const uafgj_command = uafgj_path.string() + " \"" + container + "\" \"" + input_file + "\"";
#endif
		//std::cout << "Command: " << ab_command << std::endl;
		if (Common::executeBatch(uafgj_command.c_str()) != 0) {
			LOG("Command failed: " + uafgj_command, HERE, "Maid");
		}
	}
}