// Team DAIX, 2026
// ELECTROHAMMER — STX/SPC Compiler
//
// The majority of this code was written between 2020 and 2022
//
// This module's purpose is to:
// 1. Convert .txt files into .stx files using StxTool or DRV3_STX_TOOL
// 2. Insert .stx files into .spc archives using SpcTool
// 3. Support both the legacy (CalculateAllOld) and new (CalculateAllNew) STX pipeline
// 4. Move .txt files into EXTRACTED_FILES / REPACKED_FILES before compilation
// 5. Perform multithreaded compilation when possible
// 6. Group entries by SPC target and compile each group in parallel
// 7. Inject fonts manually after SPC compilation
//
// Electrohammer is the final stage of the build pipeline: it produces the actual game-ready SPC archives.

#include "compile.h"

#include <thread>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace Compiler {

	// Convert a .txt file into a .stx file using StxTool.
	// Example:
	// input:  "scene_12.txt"
	// output: "scene_12.stx"
	// The filename is modified by replacing the last three characters with "stx".

	std::string CalculateStx(std::string const& txt, std::filesystem::path const& where) {

#ifdef _WIN32
		std::string const ext = ".exe";
#else
		std::string const ext = "";
#endif
		std::string const stxtool_loc = std::filesystem::path(where / EncryptString(("StxTool" + ext))).string();

		// Call STXTool
		std::string const stx_command = "\"" + stxtool_loc  + "\" \"" + txt + "\"";
		Common::executeBatch(stx_command.c_str());
		auto const len = txt.length();
		// Why is this needed?
		// Convert "xxx.txt" → "xxx.stx" by rewriting the extension manually.
		// Required because StxTool outputs the .stx file in-place.

		std::string stx = txt;
		stx[len - 1] = 'x';
		stx[len - 2] = 't';
		stx[len - 3] = 's';
		return stx;
	}

	// Insert a .stx file into an .spc archive using SpcTool.
	// Example:
	// SpcTool "game_resident_US.spc" insert "scene_12.stx"

	void CalculateSpc(std::string const& stx, std::filesystem::path const& program, std::filesystem::path const& where, std::string const& file_to_insert) {

#ifdef _WIN32
		std::string const ext = ".exe";
#else
		std::string const ext = "";
#endif

		std::string const spctool_loc = std::filesystem::path(program / EncryptString(("SpcTool" + ext))).string();
		std::string const spcfile_loc = std::filesystem::path(where / file_to_insert).string();
		// Call SPCTool
		std::string const spc_command =
			"\"" + spctool_loc + "\" \"" + spcfile_loc + EncryptString("\" insert \"") + stx + "\"";
		Common::executeBatch(spc_command.c_str());
	}

	// Legacy STX/SPC pipeline:
	// - If COMPILE_STX: generate .stx (if missing)
	// - If COMPILE_SPC: insert .stx into .spc
	// Used when UseNewSTXTool = false.

	template<bool COMPILE_STX, bool COMPILE_SPC>
	void CalculateAllOld(std::string const& filename, std::string const& file_to_insert, std::filesystem::path current_dir, std::filesystem::path whereto) {

		std::string stx = filename;

		if constexpr (COMPILE_STX) {
			// Why is this needed?
			auto const len = stx.length();
			stx[len - 1] = 'x';
			stx[len - 2] = 't';
			stx[len - 3] = 's';

			// If the .stx doesn't exist
			// If the .stx file does not exist, generate it from the .txt file.
			if (!std::filesystem::exists(stx)) {
				// Then we need to create it with our .txt
				stx = CalculateStx(filename, current_dir);
			}
		}

		if constexpr (COMPILE_SPC) {
			// Once we created our .stx, we insert the .stx in the .spc
			// Obviously we have to do this one at a time
			CalculateSpc(stx, current_dir, whereto, file_to_insert);
		}
	}

	// New STX/SPC pipeline:
	// - Move all .txt files into EXTRACTED_FILES
	// - Run DRV3_STX_TOOL once to generate all .stx files
	// - Insert .stx files into .spc archives
	// Used when UseNewSTXTool = true.

	template<bool COMPILE_STX, bool COMPILE_SPC>
	void CalculateAllNew(std::vector<EntryMG::Entry> const& entries) {

		// Is this even multithreaded?

		if (entries.empty()) {
			return;
		}

		if constexpr (COMPILE_STX) {

			auto const cur = entries[0].CurrentDir;

			// TODO: What if the filename or current dir is not valid?
			// Prepare all .txt files for DRV3_STX_TOOL by copying them into EXTRACTED_FILES.
			// DRV3_STX_TOOL processes all files at once.


			for (std::uint64_t index = 0; index < entries.size(); ++index) {

				std::string const newname = MoveTXTForSTXTool(entries[index].Filename, entries[index].CurrentDir);
			}

#ifdef _WIN32
			std::string const ext = ".exe";
#else
			std::string const ext = "";
#endif
			std::string const program_name = EncryptString(("DRV3_STX_TOOL" + ext));
			std::string const drv3stxtool_loc = std::filesystem::path(cur / program_name).string();
			std::string const stx_command = "\"" + drv3stxtool_loc + "\"";
			Common::executeBatch(stx_command.c_str());
		}

		if constexpr (COMPILE_SPC) {
			for (std::uint64_t index = 0; index < entries.size(); ++index) {
				// TODO: What if any of these are not valid?
				CalculateSpc(entries[index].STXFileToInsert, entries[index].CurrentDir, entries[index].WhereTo, entries[index].SPCFileToInsert);
			}
		}
	}

	// Move/copy a .txt file into EXTRACTED_FILES and compute its future .stx path.
	// Example:
	//   EXTRACTED_FILES/scene_12.txt
	//   REPACKED_FILES/scene_12.stx

	std::string MoveTXTForSTXTool(std::string const& filename, std::filesystem::path const& whereto, bool const& actually_copy) {

		std::string const filename_cut = std::filesystem::path(filename).filename().string();

		std::string const newfile = std::filesystem::path(whereto / EncryptString("EXTRACTED_FILES") / filename_cut).string();
		std::string const newfile2 = std::filesystem::path(whereto / EncryptString("REPACKED_FILES") / filename_cut).string();

		if (actually_copy) {
			if (filename != newfile) {
				std::filesystem::copy_file(filename, newfile, std::filesystem::copy_options::overwrite_existing);
			}
			else {
				LOG("WARNING: " + filename + " and " + newfile + " are the same, let's not copy it!", HERE, "Electrohammer");
			}
		}

		std::string stx = newfile2;
		if (!stx.ends_with("stx")) {
			// Why is this needed?
			// Convert the REPACKED_FILES path into a .stx filename by rewriting the extension.
			auto const len = newfile2.length();
			stx[len - 1] = 'x';
			stx[len - 2] = 't';
			stx[len - 3] = 's';
		}
		return stx;
	}

	// Compile a single .stx file using the legacy pipeline.
	// Used in multithreaded STX compilation.

	void CalculateEntrySTX(EntryMG::Entry const& entry) {

		CalculateAllOld<true, false>(entry.Filename, entry.SPCFileToInsert, entry.CurrentDir, entry.WhereTo);
		++STXCompiled;
	}

	// Insert all .stx files into their respective .spc archives.
	// Uses either the new or old pipeline depending on UseNewSTXTool.

	void CalculateAllEntriesSPC(std::vector<EntryMG::Entry> const& entries) {

		for (auto const& en : entries) {
			if constexpr (UseNewSTXTool) {
				CalculateSpc(en.STXFileToInsert, en.CurrentDir, en.WhereTo, en.SPCFileToInsert);
			}
			else {
				CalculateAllOld<true, true>(en.STXFileToInsert, en.SPCFileToInsert, en.CurrentDir, en.WhereTo);
			}
		}
		++SPCCompiled;
	}

	// Main compilation entry point:
	// 1. Prepare STX paths
	// 2. Compile STX files (single-thread or multi-thread)
	// 3. Group entries by SPC target
	// 4. Compile SPC files in parallel
	// 5. Inject fonts manually

	void Compile(std::filesystem::path const& where, std::vector<EntryMG::Entry>& entries) {

		if (entries.empty()) {
			LOG("ERROR: No entries found when compiling!", HERE, "Electrohammer");
			return;
		}

		for (std::uint64_t index = 0; index < entries.size(); ++index) {
			// TODO: what if Filename and CurrentDir are not valid?
			entries[index].STXFileToInsert = MoveTXTForSTXTool(entries[index].Filename, entries[index].CurrentDir, false);
		}

		// Get the number of threads (0 if it fails)
		// If only one CPU thread is available, fall back to single-threaded compilation.

		const auto processor_count = std::thread::hardware_concurrency();

		auto const& e1 = entries[0];

		LOG("Entered SPC compilation function", HERE, "Electrohammer");

		if (processor_count <= 1) {
			LOG("Compiling using single thread", HERE, "Electrohammer");
			if constexpr (!UseNewSTXTool) {
				LOG("WARNING: Using old STX method (1 thread)...", HERE, "Electrohammer");
				for (auto const& e : entries) {
					Compiler::CalculateAllOld<true, true>(e.Filename, e.SPCFileToInsert, e.CurrentDir, e.WhereTo);
				}
			} else {
				LOG("Using new STX method (1 thread)...", HERE, "Electrohammer");
				Compiler::CalculateAllNew<true, true>(entries);
			}
		} else {
			// Launch one thread per entry to compile STX files in parallel.
			// STXCompiled is an atomic counter used to wait for completion.

			LOG("Compiling using \"multithread\" (" + std::to_string(processor_count) + " threads found)", HERE, "Electrohammer");

			// Number of total entries
			std::uint64_t const entry_size = entries.size();

			if constexpr (!UseNewSTXTool) {
				LOG("WARNING: Using old STX method (" + std::to_string(processor_count) + " threads)...", HERE, "Electrohammer");
				// Calculate STXs
				for (std::uint64_t cont_stx = 0; cont_stx < entry_size; ++cont_stx) {
					std::jthread{ &CalculateEntrySTX, entries[cont_stx] }.detach();
				}

				// Wait until all STXs are compiled
				// (STXCompiled is atomic)
				while (STXCompiled < entry_size);
			}
			else {
				LOG("Using new STX method (" + std::to_string(processor_count) + " threads)...", HERE, "Electrohammer");
				// STX: yes, SPC: no
				Compiler::CalculateAllNew<true, false>(entries);
			}

			LOG("All STX compiled!", HERE, "Electrohammer");

			// Just in case there's something still in the background
			// Small delay to ensure all background STX threads have fully finished.

			std::this_thread::sleep_for(std::chrono::milliseconds(500));

			// Group entries by SPC target so each SPC archive is compiled in its own thread.
			// Example:
			// entries for "game_resident_US.spc" → group 0
			// entries for "chapter1_US.spc"      → group 1


			// Sort entries by SPC
			// ex:
			// 0: {entry1, entry2, entry3, etc.}
			// who all have in common that their FileToInsert is "test_text_US.spc"
			// as its index in the "fti" array is 0
			EntryMG::SplitType split{ {{}} };
			EntryMG::SortEntriesBySPC(entries, split);
			// Sorting done, moving on

			// Now that all STX files are compiled, move on to SPC files

			// split.size() is the number of total "filetoinsert" found
			// and split[x] contains all entries from a certain filetoinsert

			// Launch one thread per SPC group.
			// SPCCompiled is an atomic counter used to wait for completion.
			for (std::uint64_t en = 0; en < split.size(); ++en) {
				// All entries from chapter 0, or game_resident, or mapobjname etc.
				// So it will compile chap0, game_resident and mapobjname etc. simultaneously
				std::jthread{ &CalculateAllEntriesSPC, split[en] }.detach();
			}

			// Wait until all SPCs are compiled
			while (SPCCompiled < split.size());
		}

		LOG("All SPC compiled!", HERE, "Electrohammer");
		
		LOG("Injecting font(s) manually...", HERE, "Electrohammer");

		// Inject fonts manually into game_resident_US.spc.
		// These files are not part of the normal entry list and must be inserted explicitly.

		// FONTS IN THE 7Z/ZIP/RAR NEED TO BE UPDATED MANUALLY

		EntryMG::Entry font1_a = e1;
		font1_a.Filename = (where / "v3_font00.stx").string();
		font1_a.SPCFileToInsert = "game_resident_US.spc";

		EntryMG::Entry font1_b = e1;
		font1_b.Filename = (where / "v3_font00.srdv").string();
		font1_b.SPCFileToInsert = "game_resident_US.spc";

		CalculateSpc(font1_a.Filename, font1_a.CurrentDir, font1_a.WhereTo, font1_a.SPCFileToInsert);
		CalculateSpc(font1_b.Filename, font1_b.CurrentDir, font1_b.WhereTo, font1_b.SPCFileToInsert);

		LOG("Font(s) injected!", HERE, "Electrohammer");
	}
}