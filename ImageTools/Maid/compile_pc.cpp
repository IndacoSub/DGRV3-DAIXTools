// Team DAIX, 2026
// MAID — PC COMPILATION PIPELINE

// The majority of this code was written between 2020 and 2022

// This tool's purpose is:
// Maid’s PC pipeline performs the full PNG → SRD → SPC transformation workflow
// for the PC/Xbox/Steam versions of Danganronpa V3. It handles:
//   • ALT file resolution
//   • PNG preprocessing (SRD/SPC detection, aliasing, special SRDs)
//   • SRD generation via SrdTool
//   • SPC repacking via SpcTool
//   • Matryoshka SPC handling (nested SPCs)
//   • Minigame asset injection
//   • FileOnDemand cloud fallback for missing SPCs
//   • Multithreaded SRD/SPC compilation
//   • Cleanup of unchanged SPCs
//
// This is the most complex component of ImageTools and contains numerous
// special‑case rules required to match the game’s asset structure.

#include "compile_pc.h"

#include <regex>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../Common/Cloud.h"
#include "../Common/Common.h"

namespace PC {

	bool CompilePC(std::filesystem::path const& current_dir, std::string const& repo, std::string const& basefolder) {

		std::filesystem::path const dgrv3path = current_dir / repo;
		std::filesystem::path const where = (dgrv3path / basefolder / Cloud::dl_repo_name);

		if (!std::filesystem::exists(dgrv3path)) {
			LOG("ERROR: One of the folders where the files to inject are located was not found!", HERE, "Maid");
			return false;
		}

		if (!std::filesystem::exists(where)) {
			try {
				std::filesystem::create_directories(where);
			}
			catch (const std::filesystem::filesystem_error& e) {
				LOG(std::string("ERROR creating directory: ") + where.string() + " - " + e.what(), HERE, "Maid");
				return false;
			}
		}

		// Resolve ALT files: if UseALTs=true, replace base PNGs with ALT_ versions.
		// If UseALTs=false, delete ALT_ versions instead.

		PC::HandleAltFiles(dgrv3path);

		// Track which SPCs and SRDs are referenced by PNGs.
		// These lists determine which files must be downloaded (FileOnDemand)
		// and which must be compiled.


		std::vector<std::string> seen_spc{};
		std::vector<std::string> seen_spc_raw{};
		std::vector<std::string> seen_srd{};

		// Scan all PNG/TGA files in the repo and determine:
		//   • their SRD file
		//   • their SRDV file
		//   • their target SPC file
		//   • whether they are special, aliased, or matryoshka assets
		// Builds the FileStructure list used for compilation.

		auto const fs_vec = PC::PreprocessPNG(dgrv3path, where, repo, std::ref(seen_spc), std::ref(seen_spc_raw), std::ref(seen_srd));
		if (fs_vec.empty()) {
			LOG("ERROR: Empty file vector...", HERE, "Maid");
			return false;
		}

		// Compile all standard PNG → SRD → SPC transformations.
		// Handles SRD generation, SPC repacking, and multithreaded execution.

		PC::CompileImages(fs_vec, seen_spc_raw, seen_srd, where, dgrv3path);

		// Inject minigame assets manually.
		// These files do not follow normal SRD/SPC structure and must be inserted
		// directly into their target SPCs.


		PC::CompileMinigames(fs_vec, std::ref(seen_spc), where, dgrv3path);

		if (!Configuration::ConfigMap["FileOnDemand"]) {
			// Remove SPC files that were not modified.
			// Keeps output clean and avoids distributing unnecessary files.

			PC::DeleteUnchanged(dgrv3path, seen_spc);
		}

		// Generate a report listing all PNG → SRD → SPC mappings.
		// Useful for debugging and verifying compilation results.

		std::ofstream out_ar("arrow_report.txt", std::ios::out | std::ios::app);
		for (auto const& entry : fs_vec) {
			out_ar << entry.ToArrowReport() << std::endl;
		}
		out_ar.close();

		return true;
	}

	void CompileMinigames(std::vector<Common::FileStructure> const& fs_vec, std::vector<std::string>& seen_spc,
		std::filesystem::path const& where, std::filesystem::path const& dgrv3path) {

		// Files to be injected manually

		std::filesystem::path const current_dir = std::filesystem::current_path();

		auto noimages = (dgrv3path / "noimages");

		if (!std::filesystem::exists(noimages)) {
			LOG("\"noimages\" folder does not exist in repo.", HERE, "Maid");
			return;
		}

		for (auto const& entry : minigame_entries) {
			auto noimagepath = (noimages / entry.second);
			if (!std::filesystem::exists(noimagepath)) {
				LOG("Minigame file does not exist: " + noimagepath.string(), HERE, "Maid");
				continue;
			}
			if (std::filesystem::is_directory(noimagepath)) {
				LOG("Minigame file is actually a directory: " + noimagepath.string(), HERE, "Maid");
				continue;
			}
			auto spc_lc = entry.first + ".spc";
			auto spc_uc = entry.first + ".SPC";
			for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path)) {
				
				if (file.is_directory()) {
					continue;
				}

				if (Common::StringContains(file.path().string(), "noimages")) {
					continue;
				}

				if (Common::StringContains(file.path().string(), "danganronpa_files_copy")) {
					continue;
				}

				auto exists_lc = file.path().string().ends_with(spc_lc);
				auto exists_uc = file.path().string().ends_with(spc_uc);

				if (!(exists_lc || exists_uc)) {
					continue;
				}

				auto actual = file.path().string();

				LOG("Manually inserting minigame file: " + noimagepath.string() + " in " + actual, HERE, "Maid");
				CalculateSpc(noimagepath.string(), current_dir, actual);

				std::string fspc_name = std::filesystem::path(file).filename().string();
				std::transform(fspc_name.begin(), fspc_name.end(), fspc_name.begin(),
					[](unsigned char c) { return std::tolower(c); });
				auto const find_in_seen = std::find(seen_spc.begin(), seen_spc.end(), fspc_name);
				if (find_in_seen == seen_spc.end()) {
					seen_spc.push_back(fspc_name);
				}

				LOG("break; may need to be removed", HERE, "Maid");
				break;
			}
		}
	}

	std::vector<Common::FileStructure> PreprocessPNG(std::filesystem::path const& dgrv3path, std::filesystem::path const& where, std::string const& repo,
		std::vector<std::string>& seen_spc, std::vector<std::string>& seen_spc_raw, std::vector<std::string>& seen_srd) {

		std::vector<Common::FileStructure> fs_vec{};
		std::filesystem::path const current_dir = std::filesystem::current_path();

		// Iterate through the repository folder (DGRV3-GFX)
		// And look for .png files inside any subdirectory
		for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path)) {

			if (file.is_directory()) {
				// We don't care about folders, we only need to find .png, .srd/.srdv and .spc
				continue;
			}

			bool is_srd_matrioska = false;
			int matrioska_levels = 0;

			std::string const file_str = file.path().string();

			// Get the filename of the file we're iterating
			// (cannot be const)
			auto fn = file.path().filename();

			if (!Common::StringContains(file_str, ".png") && !Common::StringContains(file_str, ".PNG")) {
				// If this file is not a .png, continue
				// (we're compiling .png files here)
				// UPDATE (05/04/2025): Check for TGA images as well?
				if (!Common::StringContains(file_str, ".tga") && !Common::StringContains(file_str, ".TGA")) {
					continue;
				}
			}

			// Skip files that are:
			//   • MANUAL (handled separately)
			//   • noimages (minigame assets)
			//   • ALT_ (handled by HandleAltFiles)


			// Files to be injected manually
			if (Common::StringContains(file_str, "MANUAL")) {
				continue;
			}

			// Files from minigames
			if (Common::StringContains(file_str, "noimages")) {
				continue;
			}

			// We don't want ALTs (even if the option is enabled)
			if (Common::StringContains(file_str, "ALT_")) {
				continue;
			}

			// Skip SPCs known to break SPCTool or require manual handling.

			bool blacklisted = false;
			for (auto const& ff : spc_blacklist) {
				std::size_t const find = file_str.find(ff);
				blacklisted |= find != std::string::npos;
			}

			if (blacklisted) {
				continue;
			}

			Common::FileStructure fs{};
			fs.PNG_RepoName = file_str;

			// Detect non-standard SRD filenames.
			// If texture.srd is missing, check srd_special table or search parent folders.
			// Some assets use custom SRD names or matryoshka folder structures.

			bool is_special = false;

			// Resolve alias PNG names (CRC-based).
			// Some PNGs use temporary hashed names; replace them with real names before insertion.

			bool is_alias = false;

			// Get the folder by creating a substring (0, last, // included) without the name of the file
			auto const& fold = file.path().parent_path();

			// Get texture SRD(V) filename which is constant in most cases
			std::string const& default_texture_srd = EncryptString("texture.srd");
			std::string const& default_texture_srdv = EncryptString("texture.srdv");

			// Copy it in a new variable, some textures don't have texture.srd but a custom .srd name
			// which is not the default one
			// (no const)
			auto texture_srd = default_texture_srd;
			auto texture_srdv = default_texture_srdv;

			// If "texture.srd" is not found, it means that
			// A) there is no .srd at all
			// *or*
			// B) the .srd is not called texture.srd
			// If it's B) then we consider it a special case
			// And we treat it as such
			if (!std::filesystem::exists(fold / texture_srd)) {
				// We're actually not sure if the .srd file exists, since it's not "texture.srd"
				// To make sure, we need to search the list of the "special cases" to see if this
				// is one of them
				// We do this by using find_if on special, and seeing if the filename is one of the specials
				auto const find = std::find_if(srd_special.begin(), srd_special.end(), [&](Entry const& e) -> bool {return e.first == fn; });

				// If we find that it's a special case
				if (find != srd_special.end()) {
					// Get the position in the "special" list/vector
					std::uint64_t const pos = std::distance(srd_special.begin(), find);
					// The .srd file name is the one in the special vector at the specific location
					// where we found the file earlier using find_if
					texture_srd = std::string{ srd_special.at(pos).second }; // real filename of the .srd
					texture_srdv = texture_srd + "v"; // (name).srd + v = (name).srdv

					is_special = true;
				}
				else {

					// Handle matryoshka SPCs: assets stored inside nested folders ending with "_DEC".
					// Determine correct SPC folder by climbing parent directories.

					matrioska_levels = 0;

					// Iterate through the subfolder searching for srd and srdv
					auto const fold_2 = fold.parent_path();
					std::string decfolder{};
					auto fold_3 = file.path().parent_path();
					//LOG("FOLD3: " + fold_3.string(), HERE, "Maid");
					while (fold_3.string().length() > 5 && !fold_3.string().ends_with("_DEC")) {
						fold_3 = fold_3.parent_path();
						//LOG("W FOLD3: " + fold_3.string() + " with level " + std::to_string(matrioska_levels), HERE, "Maid");
						matrioska_levels++;
					}
					std::string fold_3_nodec = fold_3.string().substr(0, fold_3.string().length() - 4);
					auto const parent_path_filename = file.path().parent_path().filename();
					auto possible_srd = (fold_2 / (parent_path_filename.string() + ".srd"));
					auto possible_srdv = (fold_2 / (parent_path_filename.string() + ".srdv"));

					LOG("Making attempt at finding special: " + file.path().string() + " within " + parent_path_filename.string() + ", SRD: " + possible_srd.string(), HERE, "Maid");

					if (std::filesystem::exists(possible_srd) && std::filesystem::exists(possible_srdv)) {

						LOG("Found special: " + file.path().string() + " within " + parent_path_filename.string() + ", SRD: " + possible_srd.string(), HERE, "Maid");

						texture_srd = possible_srd.string();
						texture_srdv = possible_srdv.string();
						is_special = true;
						is_srd_matrioska = true;
					}
				}
			}

			// Get the filename of our "custom" .png file, without the .png extension
			// We're not sure if our .png has an alias, so we're going to check that
			// (cannot be const)
			auto png_noext = file_str.substr(0, file.path().string().length() - file.path().extension().string().length());
			std::string real_file = file_str;

			if (is_special) {
				// Try to find if the texture.srd has a file with an alias
				auto const find = std::find_if(srd_aliases.begin(), srd_aliases.end(), [&](Alias const& al) -> bool {return std::get<2>(al) == texture_srd; });

				// If the alias is present
				if (find != srd_aliases.end()) {

					// Get the position in the "alias" list/vector
					std::uint64_t const pos = std::distance(srd_aliases.begin(), find);

					// Get the actual alias in the vector
					auto const alias = srd_aliases[pos];

					// Get the fake name of the .png
					std::string const fake_name = std::get<0>(alias);

					// Get the real name of the .png
					std::string const real_name = std::get<1>(alias);

					// If the real .png already exists, delete it
					if (std::filesystem::exists(fold / real_name)) {
						std::string const p = (fold / real_name).string();
#ifdef _WIN32
						DeleteFileA(p.c_str());
#else
						std::filesystem::remove(p);
#endif
					}

					// Rename the fake .png into the .real png
					// Before inserting it into the .srd
					std::string const f = (fold / fake_name).string();
					std::string const r = (fold / real_name).string();

					if (std::filesystem::exists(f)) {

#ifdef _WIN32
						MoveFileA(f.c_str(), r.c_str());
#else
						std::filesystem::rename(f, r);
#endif
					}
					else {
						LOG("Source does not exist for copy: " + f, HERE, "Maid");
						continue;
					}

					// Remove the extension
					std::filesystem::path const real_name_p(real_name);
					fn = real_name;
					png_noext = real_name.substr(0, real_name.length() - real_name_p.extension().string().length()); // remove ".png" (long 4)
					real_file = (fold / real_name).string();
					is_alias = true;

					//std::cout << "is an alias!!" << std::endl;
				}
			}

			auto const& fold2 = fold;
			// Get the folder containing the SPCs
			auto spc_fold = fold2.parent_path();
			// By calculating where our SPC folder is, we can
			// assume that the .spc file is inside that folder
			// so we can just create a new variable and use it
			// as a "link" to our .spc file

			for (int m = 0; m < matrioska_levels; m++) {
				spc_fold = spc_fold.parent_path();
			}

			// (cannot be const)
			auto relative_path = file.path().parent_path().string();
			std::size_t const find_repo_in_path = relative_path.find(repo);
			if (find_repo_in_path == std::string::npos) {
				LOG("ERROR: Couldn't find " + repo, HERE, "Maid");
				return {};
			}
			relative_path = relative_path.substr(find_repo_in_path);
			relative_path = relative_path.substr(repo.length() + 1);

			//LOG("Relative path: " + relative_path, HERE, "Maid");

			std::string dec = "_DEC";
			std::filesystem::path fpspc((where / relative_path));

			for (int m = 0; m < matrioska_levels; m++) {
				fpspc = fpspc.parent_path();
			}

			auto fspc = fpspc.string();

			LOG("FPSC: " + fspc, HERE, "Maid");

			fspc = ReplaceLastOccurrence(fspc, "us", "US");
			fspc = fspc.substr(0, fspc.length() - dec.length());
			fspc += ".spc";

			LOG("FPSC (2): " + fspc, HERE, "Maid");

			fpspc = std::filesystem::path(fspc);


			if (is_special) {
				// Special assets: insert PNG into SRD, then SRD/SRDV into SPC immediately.
				// These do not go through the normal batching pipeline.

				if (Configuration::ConfigMap["FileOnDemand"]) {
					if (!std::filesystem::exists(fspc)) {
						DownloadSPC(fspc, fspc);
						if (!std::filesystem::exists(fspc)) {
							fspc = fspc.substr(0, fspc.length() - fpspc.extension().string().length());
							fspc += ".SPC";

							//LOG("FPSC (3): " + fspc, HERE, "Maid");
						}
					}
				}

				// Use SrdTool to insert the .png in the .srd
				CalculateSrd(current_dir, real_file, (fold / texture_srd).string());

				// Use SpcTool to insert the .srd in the .spc
				CalculateSpc((fold / texture_srd).string(), current_dir, fspc);

				// Use SpcTool to insert the .srdv in the .spc
				CalculateSpc((fold / texture_srdv).string(), current_dir, fspc);

				if (Configuration::ConfigMap["FileOnDemand"]) {
					continue;
				}
			}

			std::string fspc_name = std::filesystem::path(fspc).filename().string();
			std::transform(fspc_name.begin(), fspc_name.end(), fspc_name.begin(),
				[](unsigned char c) { return std::tolower(c); });
			auto const find_in_seen = std::find(seen_spc.begin(), seen_spc.end(), fspc_name);
			if (find_in_seen == seen_spc.end()) {
				seen_spc.push_back(fspc_name);
			}

			if (is_special) {
				continue;
			}

			fs.PNG_ActualName = real_file;
			fs.SRD_Filename = (fold / texture_srd).string();
			fs.SRDV_Filename = (fold / texture_srdv).string();
			fs.SPC_Filename = fspc;
			fs_vec.push_back(fs);

			std::string const fspc_raw_low = fspc;
			//std::transform(fspc_raw_low.begin(), fspc_raw_low.end(), fspc_raw_low.begin(),
				//[](unsigned char c) { return std::tolower(c); });
			auto const find_in_seen_raw = std::find(seen_spc_raw.begin(), seen_spc_raw.end(), fspc_raw_low);
			if (find_in_seen_raw == seen_spc_raw.end()) {
				seen_spc_raw.push_back(fspc_raw_low);
			}

			std::string fsrd_name = fs.SRD_Filename;
			std::transform(fsrd_name.begin(), fsrd_name.end(), fsrd_name.begin(),
				[](unsigned char c) { return std::tolower(c); });
			auto const find_srd_in_seen = std::find(seen_srd.begin(), seen_srd.end(), fsrd_name);
			if (find_srd_in_seen == seen_srd.end()) {
				seen_srd.push_back(fsrd_name);
			}
		}

		return fs_vec;
	}

	// Group PNG entries by SRD file.
	// Each group is processed by one thread to avoid SRDTool conflicts.

	Common::SplitType SortEntriesBySRD(std::vector<Common::FileStructure> const& entries, std::vector<std::string> const& srd_list) {

		Common::SplitType split{ {{}} };
		split.resize(srd_list.size());
		//split.resize(entries.size());

		for (auto const& en : entries) {
			// Multithread by SRD_Filename
			// Finds .srd using std::find and parallelizes work
			// Result in a 30-35% increase in performance

			std::string name = en.SRD_Filename;
			std::transform(name.begin(), name.end(), name.begin(),
				[](unsigned char c) { return std::tolower(c); });

			auto const it = std::find(srd_list.begin(), srd_list.end(), name);
			if (it == srd_list.end() || name.empty()) {
				// Couldn't be found...?
				LOG("ERROR: Invalid .srd file: " + std::to_string(std::distance(srd_list.begin(), it)), HERE, "Maid");
				continue;
			}

			std::uint64_t const distance = std::distance(srd_list.begin(), it);
			if (distance >= srd_list.size()) {
				// Invalid index...?
				LOG("ERROR: Invalid index", HERE, "Maid");
				return {};
			}

			if (distance >= split.size()) {
				// Invalid array...?
				LOG("ERROR: Invalid array (distance: " + std::to_string(distance) + ", array size: " + std::to_string(split.size()) + ")", HERE, "Maid");
				return {};
			}

			// Add to an array sorted by .srd
			split[distance].push_back(en);
		}

		for (auto& s : split) {
			// Sort by SPC filename, not SRD filename
			std::sort(s.begin(), s.end(), [](Common::FileStructure const& a, Common::FileStructure const& b) -> bool {return a.SPC_Filename < b.SPC_Filename; });
		}

		return split;
	}

	// Group PNG entries by SPC file.
	// Each group is processed by one thread to avoid SpcTool conflicts.

	Common::SplitType SortEntriesBySPC(std::vector<Common::FileStructure> const& entries, std::vector<std::string> const& spc_list) {

		Common::SplitType split{ {{}} };
		split.resize(spc_list.size());
		//split.resize(entries.size());

		for (auto const& en : entries) {
			// Multithread by SPC_Filename
			// Finds .spc using std::find and parallelizes work
			// Result in a 30-35% increase in performance

			std::string const name = en.SPC_Filename;
			//std::transform(name.begin(), name.end(), name.begin(),
				//[](unsigned char c) { return std::tolower(c); });

			auto const it = std::find(spc_list.begin(), spc_list.end(), name);
			if (it == spc_list.end() || name.empty()) {
				// Couldn't be found...?
				LOG("ERROR: Invalid .spc file: " + name, HERE, "Maid");
				continue;
			}

			std::uint64_t const distance = std::distance(spc_list.begin(), it);
			if (distance >= spc_list.size()) {
				// Invalid index...?
				LOG("ERROR: Invalid index", HERE, "Maid");
				return {};
			}

			if (distance >= split.size()) {
				// Invalid array...?
				LOG("ERROR: Invalid array (distance: " + std::to_string(distance) + ", array size: " + std::to_string(split.size()) + "): " + en.SPC_Filename, HERE, "Maid");
				return {};
			}

			// Add to an array sorted by .spc
			split[distance].push_back(en);
		}

		for (auto& s : split) {
			// Sort by SPC filename
			std::sort(s.begin(), s.end(), [](Common::FileStructure const& a, Common::FileStructure const& b) -> bool {return a.SPC_Filename < b.SPC_Filename; });
		}

		return split;
	}

	// Insert all SRDs/SRDVs belonging to a single SPC file.
	// Avoid duplicate SRD insertions using a local cache.
	
	void CalculateAllEntriesSPC(std::vector<Common::FileStructure> const& entries) {

		std::filesystem::path const current_dir = std::filesystem::current_path();

		std::vector<std::string> local_inserted_srds{};

		for (auto const& en : entries) {

			std::string const srdname = en.SRD_Filename;

			// Not present since it's empty, compile it
			bool should_compile_srd = local_inserted_srds.empty();
			if (!should_compile_srd) {
				auto const it = std::find(local_inserted_srds.begin(), local_inserted_srds.end(), srdname);
				should_compile_srd |= it == local_inserted_srds.end(); // If it's not one we've seen, compile it
			}

			if (should_compile_srd) {
				// Use SpcTool to insert the .srd in the .spc
				CalculateSpc(srdname, current_dir, en.SPC_Filename);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				// Use SpcTool to insert the .srdv in the .spc
				CalculateSpc(en.SRDV_Filename, current_dir, en.SPC_Filename);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));

				local_inserted_srds.push_back(srdname);
			}
		}
		//std::cout << SPCCompiled << " SPCs" << std::endl;
		++SPCCompiled;
	}

	// Insert all PNGs belonging to a single SRD file.
	// SRDTool is invoked once per PNG.

	void CalculateAllEntriesSRD(std::vector<Common::FileStructure> const& entries) {

		std::filesystem::path const current_dir = std::filesystem::current_path();

		for (auto const& en : entries) {
			// std::cout << en.ToArrowReport() << std::endl;
			// Use SrdTool to insert the .png in the .srd
			CalculateSrd(current_dir, en.PNG_ActualName, en.SRD_Filename);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		//std::cout << SRDCompiled << " SRDs" << std::endl;
		++SRDCompiled;
	}

	void CompileImages(std::vector<Common::FileStructure> const& fs_vec, std::vector<std::string> const& spc_list, std::vector<std::string> const& srd_list,
		std::filesystem::path const& where, std::filesystem::path const& dgrv3path) {

		std::filesystem::path const current_dir = std::filesystem::current_path();

		// Download missing SPCs instead of requiring the full repo.
		// Critical for private cloud repos where cloning is not possible.

		if (Configuration::ConfigMap["FileOnDemand"]) {
			LOG("Downloading SPCs...", HERE, "Maid");

			for (auto const& file : spc_list) {
				if (!std::filesystem::exists(file)) {
					DownloadSPC(file, file);
				}
			}
		}

		LOG("Sorting SRD...", HERE, "Maid");

		Common::SplitType const split_SRD = SortEntriesBySRD(fs_vec, srd_list);

		LOG("Sorting SPC...", HERE, "Maid");

		Common::SplitType const split_SPC = SortEntriesBySPC(fs_vec, spc_list);

		auto start_time = std::chrono::system_clock::now();

		LOG("Calculating " + std::to_string(split_SRD.size()) + " SRDs...", HERE, "Maid");

		// Compile SRDs first — SPCs depend on SRD output.

		for (std::uint64_t en = 0; en < split_SRD.size(); ++en) {

			if (Configuration::ConfigMap["MultithreadedCompilation"]) {
				std::jthread{ &CalculateAllEntriesSRD, split_SRD[en] }.detach();
			}
			else {
				CalculateAllEntriesSRD(split_SRD[en]);
			}
		}

		// This whole block is just "wait for SRDs to compile"
		if (Configuration::ConfigMap["MultithreadedCompilation"]) {
			while (SRDCompiled < split_SRD.size()) {
				auto const now = std::chrono::system_clock::now();
				auto const diff = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);
				if (diff.count() > 300) {
					LOG("ERROR: Too much time passed calculating SRDs!", HERE, "Maid");
					return;
				}
			}
		}

		start_time = std::chrono::system_clock::now();

		LOG("Calculating " + std::to_string(split_SPC.size()) + " SPCs...", HERE, "Maid");

		// Compile SPCs after SRDs are finished.
		// Each SPC group is processed in parallel if enabled.

		for (std::uint64_t en = 0; en < split_SPC.size(); ++en) {

			if (Configuration::ConfigMap["MultithreadedCompilation"]) {
				std::jthread{ &CalculateAllEntriesSPC, split_SPC[en] }.detach();
			}
			else {
				CalculateAllEntriesSPC(split_SPC[en]);
			}
		}

		// Wait until all SPCs are compiled
		if (Configuration::ConfigMap["MultithreadedCompilation"]) {
			while (SPCCompiled < split_SPC.size()) {
				auto const now = std::chrono::system_clock::now();
				auto const diff = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);
				if (diff.count() > 1000) {
					LOG("Too much time passed calculating SPCs!", HERE, "Maid");
					return;
				}
			}
		}

		LOG("Compiling special SPCs (a.k.a. MatrioSPCka)", HERE, "Maid");

		// Final stage: insert nested SPCs (matryoshka) into their parent SPCs.
		// These files must be pre-repacked manually and stored in MANUAL/ ?

		for (auto const& spc : matrioSPCka) {
			std::string const goes_into = spc.second;
			std::string const cursed_file = spc.first;
			std::filesystem::path const cursed_file_path = (dgrv3path / "MANUAL" / cursed_file);
			// (Should exist)
			if (!std::filesystem::exists(cursed_file_path)) {
				LOG("WARNING: Couldn't find special SPC: (" + cursed_file + ") " + cursed_file_path.string(), HERE, "Maid");
				continue;
			}
			std::string const cursed_file_path_str = cursed_file_path.string();

			std::filesystem::path const goes_into_path = (where / goes_into);
			std::string const goes_into_path_str = goes_into_path.string();
			if (!std::filesystem::exists(goes_into_path)) {
				// How the F do you go from (nil / "flash" / "adv" / "spcpack" / "chara_name_US.spc").string()) to the download URL?
				DownloadSPC(goes_into_path_str, goes_into_path_str);
				if (!std::filesystem::exists(goes_into_path)) {
					LOG("WARNING: Couldn't download the file: (" + goes_into + ") " + goes_into_path.string(), HERE, "Maid");
					continue;
				}
			}

			// Use SpcTool to insert the .spc in the .spc
			// Remove the // later
			CalculateSpc(cursed_file_path_str, current_dir, goes_into_path_str);
			//std::cout << std::endl;
			//std::cout << "Insert " << cursed_file_path_str << " into " << goes_into_path_str << std::endl;
			//std::cout << std::endl;
		}

		LOG("Done!", HERE, "Maid");
	}

	void DeleteUnchanged(std::filesystem::path const& dgrv3path, std::vector<std::string> const& changed_files) {

		std::vector<std::string> to_delete{};

		//std::cout << "Changed files:" << std::endl;
		//for (auto const& f : changed_files) {
			//std::cout << f << std::endl;
		//}
		//std::cout << std::endl;

		for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path)) {
			if (file.is_directory()) {
				continue;
			}

			std::string const file_str = file.path().string();
			if (!Common::StringContains(file_str, ".spc") && !Common::StringContains(file_str, ".SPC")) {
				continue;
			}

			std::string filename = file.path().filename().string();

			std::transform(filename.begin(), filename.end(), filename.begin(),
				[](unsigned char c) { return std::tolower(c); });

			auto const it = std::find(changed_files.begin(), changed_files.end(), filename);
			if (it == changed_files.end()) {
				to_delete.push_back(file.path().string());
			}
		}

		for (auto const& file : to_delete) {
			//std::cout << "Deleting: " << file << std::endl;
			std::filesystem::remove(file);
		}
	}

	void HandleAltFiles(std::filesystem::path const& dgrv3path) {

		std::vector<std::string> alt_vec{};
		std::string const alt_prefix = "ALT_";

		// Look for "ALT" files
		// Scan entire repo for PNG/TGA files that have an ALT_ variant
		for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path)) {
			if (file.is_directory()) {
				// We don't care about folders, we only need to find .png
				continue;
			}

			std::string const file_str = file.path().string();

			if (!Common::StringContains(file_str, ".png") && !Common::StringContains(file_str, ".PNG")) {
				// If this file is not a .png, continue
				// (we're compiling .png files here)
				// UPDATE (05/04/2025): Check for TGA images as well?
				if (!Common::StringContains(file_str, ".tga") && !Common::StringContains(file_str, ".TGA")) {
					continue;
				}
			}

			if (Common::StringContains(file_str, alt_prefix)) {
				continue;
			}

			std::string const cur_filename = file.path().filename().string();
			std::filesystem::path const p = file.path();
			std::filesystem::path const dir = p.parent_path();
			std::string const alt = (dir / (alt_prefix + cur_filename)).string();
			bool const has_an_alt = std::filesystem::exists(alt);
			if (has_an_alt) {
				alt_vec.push_back(file_str);
			}

			//std::cout << "Has an alt: " << (has_an_alt ? "true, " : "false, ") << file_str << std::endl;
			//std::cout << "Possible name: " << alt << std::endl;
		}

		// Delete old file and rename alt file

		for (auto const& file : alt_vec) {

			std::string const old_name = file;
			std::filesystem::path const old = std::filesystem::path(old_name);
			std::filesystem::path const containing_dir = old.parent_path();
			std::string const alt_name = (containing_dir / (alt_prefix + old.filename().string())).string();

			// Which one is the file to delete?
			// Based on USE_ALT

			if (Configuration::ConfigMap["UseALTs"]) {
				// ALT mode ON → delete original file
#ifdef _WIN32
				DeleteFileA(old_name.c_str());
#else
				std::filesystem::remove(old_name);
#endif
			}
			else {
				// ALT mode OFF → delete ALT file
				if (!std::filesystem::exists(alt_name)) {
					LOG("Alt does not exist: " + alt_name, HERE, "Maid");
					continue;
				}

#ifdef _WIN32
				DeleteFileA(alt_name.c_str());
#else
				std::filesystem::remove(alt_name);
#endif
			}

			// If ALT mode ON, rename ALT_<file> → <file>
			if (Configuration::ConfigMap["UseALTs"]) {

				if (!std::filesystem::exists(alt_name)) {
					LOG("Alt does not exist: " + alt_name, HERE, "Maid");
					continue;
				}

#ifdef _WIN32
				MoveFileA(alt_name.c_str(), old_name.c_str());
#else
				Common::executeBatch(("mv " + alt_name + " " + old_name).c_str());
#endif
			}
		}
	}

	void CalculateSrd(std::filesystem::path const& srdtool_loc, std::string const& png, std::string const& srd) {

		if (png.empty()) {
			return;
		}

#ifdef _WIN32
		std::string const ext = ".exe";
#else
		std::string const ext = "";
#endif

		std::filesystem::path const srdtool_path = srdtool_loc / ("SrdTool" + ext);

#ifdef _WIN32
		std::string const srd_command = "\"" + srdtool_path.string()  + "\" \"" + srd + "\" \"" + png + "\"";
#else
		std::string const srd_command = srdtool_path.string() + " \"" + srd + "\" \"" + png + "\"";
#endif
		//std::cout << "SRD Command: " << srd_command << std::endl;
		if (Common::executeBatch(srd_command.c_str()) != 0) {
			LOG("SRD command failed: " + srd_command, HERE, "Maid");
		}
	}

	void CalculateSpc(std::string const& srd, std::filesystem::path const& program, std::string const& file_to_insert) {

		if (srd.empty()) {
			return;
		}

#ifdef _WIN32
		std::string const ext = ".exe";
#else
		std::string const ext = "";
#endif

		std::filesystem::path const spctool_path = program / ("SpcTool" + ext);

		//std::cout << "In " << file_to_insert << ":" << std::endl;
#ifdef _WIN32
		std::string const spc_command = "\"" + spctool_path.string() + "\" \"" + file_to_insert + EncryptString("\" insert \"") + srd + "\"";
#else
		std::string const spc_command = spctool_path.string() + " \"" + file_to_insert + EncryptString("\" insert \"") + srd + "\"";
#endif
		//std::cout << "SPC Command: " << spc_command << std::endl;
		if (Common::executeBatch(spc_command.c_str()) != 0) {
			LOG("SPC command failed: " + spc_command, HERE, "Maid");
		}
		LOG("\n", HERE, "Maid");
	}

	void DownloadSPC(std::string const& output_spc, std::string const& dl_repo_path) {

		std::string dl_output = output_spc;
		std::string folder = dl_repo_path;

		// Extract repo-relative path: find Cloud::dl_repo_name inside full path
		std::size_t const findrepo = folder.find(Cloud::dl_repo_name);
		if (findrepo == std::string::npos) {
			LOG("ERROR: Invalid download repository: " + Cloud::dl_repo_name + " (couldn't be find inside " + folder + ")!", HERE, "Maid");
			return;
		}

		// Trim everything before repo name
		folder = folder.substr(findrepo);
		folder = folder.substr(Cloud::dl_repo_name.length() + 1);
		//folder = std::regex_replace(folder, std::regex("\\"), "//");

		std::replace(folder.begin(), folder.end(), '\\', '/');

		// Fix lowercase "us" → uppercase "US" (game uses uppercase)
		folder = ReplaceLastOccurrence(folder, "us", "US");
		dl_output = ReplaceLastOccurrence(dl_output, "us", "US");

		std::string const output_spc_noname = std::filesystem::path(output_spc).parent_path().string();
		std::filesystem::create_directories(output_spc_noname);

		//std::cout << "Downloading: " << combined << std::endl;

		// First download attempt (authenticated)
		std::string command = "curl -s -H \"Authorization: token " + Cloud::dl_repo_token +"\" -L https://raw.githubusercontent.com/" +
			Cloud::dl_repo_owner + "/" + Cloud::dl_repo_name + "/" + Cloud::dl_branch + "/" + folder +
			" --retry 3 --retry-delay 1 --create-dirs -o " + dl_output;

		if (Common::executeBatch(command.c_str(), {}, true) != 0) {
			//LOG("Command failed: " + command, HERE, "Maid");
		}

		// Validate download: GitHub raw returns "404: Not Found" as file content
		bool spc_exists = std::filesystem::exists(output_spc);
		bool valid_file = true;
		if (spc_exists) {
			std::ifstream inspc(output_spc, std::ios::in);
			std::stringstream ss{};
			ss << inspc.rdbuf();
			inspc.close();
			valid_file = ss.str() != "404: Not Found";
		}

		// If invalid, retry using uppercase ".SPC"
		if (!spc_exists || !valid_file) {
			if (spc_exists) {
#ifdef _WIN32
				DeleteFileA(dl_output.c_str());
#else
				std::filesystem::remove(dl_output);
#endif
			}
			folder = std::regex_replace(folder, std::regex("spc"), "SPC");
			dl_output = std::regex_replace(dl_output, std::regex("spc"), "SPC");

			command = "curl -s https://raw.githubusercontent.com/" +
				Cloud::dl_repo_owner + "/" + Cloud::dl_repo_name + "/" + Cloud::dl_branch + "/" + folder +
				" -o " + dl_output;

			// System or Common::ExecuteBatch?
			Common::executeBatch(command.c_str());

			spc_exists = std::filesystem::exists(output_spc);
			if (spc_exists) {
				std::ifstream inspc(output_spc, std::ios::in);
				std::stringstream ss{};
				ss << inspc.rdbuf();
				inspc.close();
				valid_file = ss.str() != "404: Not Found";
			}
			if (!spc_exists || !valid_file) {
				LOG("WARNING: Couldn't download: " + folder, HERE, "Maid");
			}
		}

		// Mirror downloaded SPC into working folder (danganronpa_files_copy)

		std::string selected_out = dl_output;
		selected_out = std::regex_replace(selected_out, std::regex(Cloud::dl_repo_name), "danganronpa_files_copy");
		std::string const selout = std::filesystem::path(selected_out).parent_path().string();
		std::filesystem::create_directories(selout);
		std::filesystem::copy_file(dl_output, selected_out, std::filesystem::copy_options::overwrite_existing);
	}

	std::string ReplaceLastOccurrence(const std::string& str, const std::string& from, const std::string& to) {

		auto const SplitString = [](std::string const& str, char delimiter) -> std::vector<std::string> {
			std::vector<std::string> tokens;
			std::stringstream ss(str);
			std::string token;
			while (std::getline(ss, token, delimiter)) {
				tokens.push_back(token);
			}
			return tokens;
		};

		std::vector<std::string> parts = SplitString(str, '_');
		int count_from = 0;

		/*
		for (auto it = parts.rbegin(); it != parts.rend(); ++it) {
			if (*it == from) {
				count_from++;
			}
		}
		*/

		for (auto it = parts.rbegin(); it != parts.rend(); ++it) {
			if (*it == to) {
				break;
			}
			if (*it != from) {
				continue;
			}

			// HAS to be from in order to be here

			it->replace(0, from.length(), to);
			break;
		}
		std::string result;
		for (const auto& part : parts) {
			if (!result.empty()) {
				result += '_';
			}
			result += part;
		}
		return result;
	}

	int main() {
		std::string str = "this_is_a_test_string";
		std::string from = "test";
		std::string to = "example";
		std::string result = ReplaceLastOccurrence(str, from, to);
		std::cout << "Result: " << result << std::endl;
		return 0;
	}

}