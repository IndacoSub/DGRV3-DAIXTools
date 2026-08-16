// Team DAIX, 2026
// MONODAM — FONTTOOLS FONT COMPILER

// The majority of this code was written between 2020 and 2022

// This tool's purpose is:
// Monodam compiles all font variants inside DGRV3-Font by invoking HTFont
// (from the older Harmony-Tools suite) and then inserting the resulting
// STX/SRDV files into the correct SPC archives using SpcTool.
//
// It performs:
//   • Cleanup of leftover .bat and charset.txt files
//   • Detection of all *_DEC/v3_fontXX/ folders
//   • Copying HTFont and its dependency (assimp.dll / libassimp.so)
//     into each font folder
//   • Running HTFont --pack to generate .srd and .srdv files
//   • Converting .srd → .stx (HTFont produces .srd, but the game expects .stx)
//   • Inserting .stx and .srdv into the correct SPC using SpcTool
//   • Logging all modified SPCs into list_changed.txt
//
// NOTE ABOUT HTFont:
// HTFont.exe is part of an **older version of Harmony-Tools** (by Redssu, https://github.com/redssu/Harmony-Tools).
// Harmony-Tools used to be split into multiple executables (HTFont, HTSrd,
// HTSpc, etc.). Modern Harmony-Tools is now a single unified executable,
// but FontTools still uses the older HTFont component because:
//
//   • It provides stable STX/SRDV packing routines
//   • It matches the expected behavior of the DAIXTOOLS font pipeline
//   • It avoids breaking compatibility with existing SPC workflows
//
// Monodam is the “font compiler” of FontTools. It transforms prepared font
// directories (created by Monophanie) into final STX/SRDV assets and inserts
// them into the correct SPC archives.

#include <iostream>
#include <filesystem>
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "../Common/Common.h"

int main() {

	// Repository name (and also folder)
	// Locate the DGRV3-Font repository and its base_spc folder.
	// Monodam expects the repo to have been cloned by Monotaro and prepared by Monophanie.

	const std::string repo = EncryptString("DGRV3-Font");

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const dgrv3path = current_dir / repo;

	std::filesystem::path const where = (dgrv3path / EncryptString("base_spc"));

	if (!std::filesystem::exists(where)) {
		LOG("ERROR: The folder where the files to copy are located was not found!", HERE, "Monodam");
		return -1;
	}

	std::vector<std::string> list_changed{};

#ifdef _WIN32
	std::string const ext = ".exe";
#else
	std::string const ext = "";
#endif

	// HTFont depends on assimp.dll / libassimp.so for mesh/texture operations.
	// We copy this dependency into each font folder before running HTFont.

#ifdef _WIN32
	std::filesystem::path const libassimp_before = (where / "assimp.dll");
#else
	std::filesystem::path const libassimp_before = (where / "libassimp.so");
#endif

	// What is the purpose of HTFont??? Ok, it's a .7z which is then extracted
	// It's Harmony-Tools's Font extract/repack utility
	std::filesystem::path htfont_before = (where / ("HTFont" + ext));
	std::filesystem::path spctool_loc = (current_dir / ("SpcTool" + ext));

	std::filesystem::path cur_folder = {};
	std::filesystem::path program_folder = {};

	std::vector<std::string> todelete{};

	// Delete any .bat or charset.txt

	for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path)) {

		if (file.is_directory()) {
			continue;
		}

		std::string const file_str = file.path().string();
		if (Common::StringContains(file_str, ".bat")) {
			todelete.push_back(file_str);
			continue;
		}
		if (Common::StringContains(file_str, "charset.txt")) {
			todelete.push_back(file_str);
			continue;
		}
	}

	for (auto const& td : todelete) {
		std::filesystem::remove(td);
	}

	// Find all font variant folders matching *_DEC/v3_fontXX/.
	// These folders contain the prepared font data created by Monophanie?

	for (auto const& file : std::filesystem::recursive_directory_iterator(dgrv3path)) {

		std::string const file_str = file.path().string();
		if (Common::StringContains(file_str, "base_spc")) {
			continue;
		}
		if (!Common::StringContains(file_str, "_DEC")) {
			continue;
		}
		if (!Common::StringContains(file_str, "v3_")) {
			continue;
		}
		if (!file.is_directory()) {
			// So we want a directory?
			continue;
		}

		// Should be something_DEC/v3_fontxx/
		cur_folder = file.path();
		program_folder = cur_folder.parent_path();

		std::string const font_name = cur_folder.filename().string();
		std::string const cur_folder_str = cur_folder.string();

		// Copy HTFont and its dependency into the font folder.
		// HTFont must run inside the folder it is packing, otherwise it fails.

		std::filesystem::path const htfont_after = (program_folder / ("HTFont" + ext));

#ifdef _WIN32
		std::filesystem::path const libassimp_after = (program_folder / "assimp.dll");
#else
		std::filesystem::path const libassimp_after = (program_folder / "libassimp.so");
#endif

		// Copy HTFont to program_folder
		// program folder being each individual font, probably (why?)
		std::filesystem::copy_file(htfont_before, htfont_after, std::filesystem::copy_options::overwrite_existing);
		LOG("Copied " + htfont_before.string() + " to " + htfont_after.string(), HERE, "Monodam");
		if (std::filesystem::exists(libassimp_before)) {
			std::filesystem::copy_file(libassimp_before, libassimp_after, std::filesystem::copy_options::overwrite_existing);
			LOG("Copied " + libassimp_before.string() + " to " + libassimp_after.string(), HERE, "Monodam");
		}

#ifdef _WIN32
		std::string const command = "\"" + htfont_after.string() + "\" --pack " + cur_folder.string();
#else
		std::string const command = htfont_after.string() + " --pack " + cur_folder.string();
#endif
		Common::executeBatch(command.c_str());

		// Now .srd and .srdv have been created
		// Spoiler: .srd is .stx

		std::string const stx = (program_folder / (font_name + ".stx")).string();
		std::string const srd = (program_folder / (font_name + ".srd")).string();
		std::string const srdv = (program_folder / (font_name + ".srdv")).string();

		if (std::filesystem::exists(stx)) {
#ifdef _WIN32
			DeleteFileA(stx.c_str());
#else
			Common::executeBatch(("rm " + stx).c_str());
#endif
		}

		// Convert .srd → .stx.
		// HTFont produces .srd, but the game expects .stx.
		// .srdv is kept as-is.


#ifdef _WIN32
		MoveFileA(srd.c_str(), stx.c_str());
		DeleteFileA(htfont_after.string().c_str());
		if (std::filesystem::exists(libassimp_after)) {
			DeleteFileA(libassimp_after.string().c_str());
		}
#else
		Common::executeBatch(("mv " + srd + " " + stx).c_str());
		Common::executeBatch(("rm " + (htfont_after.string())).c_str());
		if (std::filesystem::exists(libassimp_after)) {
			Common::executeBatch(("rm " + (libassimp_after.string())).c_str());
		}
#endif

		if (Common::StringContains(file_str, "game_resident")) {
			continue;
		}

		// Remove everything before "DGRV3-Font"
		// Compute relative path to determine which SPC archive this font belongs to.
		// Each font folder maps to a specific SPC inside base_fonts.

		std::filesystem::path const rel = std::filesystem::relative(program_folder, dgrv3path);

		LOG("Relative path between " + program_folder.string() + " and " + dgrv3path.string() + ": " + rel.string(), HERE, "Monodam");

		std::filesystem::path const fonts_dir = (dgrv3path / "base_spc" / "base_fonts");

		std::string toremove = "_DEC";

		std::filesystem::path const spc = (fonts_dir / rel);
		std::string spc_str = spc.string();
		spc_str = spc_str.substr(0, spc_str.length() - toremove.length());
		if (std::filesystem::exists((spc_str + ".spc"))) {
			spc_str += ".spc";
		}
		else {
			spc_str += ".SPC";
		}

		if (!std::filesystem::exists(spc_str)) {
			LOG("WARNING: SPC does not exist: " + spc_str, HERE, "Monodam");
			continue;
		}

		LOG("In " + spc.string() + ":", HERE, "Monodam");

		// Insert the newly generated .stx and .srdv into the correct SPC archive.
		// SpcTool handles the binary insertion and repacking.

#ifdef _WIN32
		std::string const command2 = "\"" + spctool_loc.string() + "\" " + spc_str + " " + stx;
		std::string const command3 = "\"" + spctool_loc.string() + "\" " + spc_str + " " + srdv;
#else
		std::string const command2 = spctool_loc.string() + " " + spc_str + " insert " + stx;
		std::string const command3 = spctool_loc.string() + " " + spc_str + " insert " + srdv;
#endif
		Common::executeBatch(command2.c_str());
		Common::executeBatch(command3.c_str());
		LOG("\n", HERE, "Monodam");

		LOG("Adding to the list: " + spc_str, HERE, "Monodam");

		list_changed.push_back(spc_str);
	}

	std::ofstream out("list_changed.txt", std::ios::out | std::ios::app);
	for (auto const& entry : list_changed) {
		std::string const newf = Common::ShortenFilename(entry, 3);
		LOG("Saving to file: " + newf, HERE, "Monodam");
		out << newf << std::endl;
	}
	out.close();
}