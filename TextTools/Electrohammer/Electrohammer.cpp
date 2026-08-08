#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <thread>
#include <atomic>

// Remove this when <filesystem> actually works on VS2019
#ifdef _WIN32
#include <Windows.h>
#endif

#include "Electrohammer.h"
#include "entrymanagement.h"
#include "compile.h"
#include "../Common/Config.h"

int main(int argc, char* argv[]) {

	std::vector<std::string> args(argv, argv + argc);

	const std::string repo = EncryptString("DGRV3");

	std::filesystem::path const current_dir = std::filesystem::current_path();
	std::filesystem::path const dgrv3path = current_dir / repo;
	std::filesystem::path const where = (dgrv3path / EncryptString("base_spc"));
	std::filesystem::path const configfile = current_dir / "TextConfig.config";

	LOG("Reading config", HERE, "Electrohammer");
	Configuration::ReadConfig(configfile);
	Configuration::ViewDebugCurrentConfig();
	LOG("Done reading config", HERE, "Electrohammer");

	if (argc > 0) {
		if (std::any_of(args.begin(), args.end(), [](std::string const& arg) -> bool {return Common::StringContains(arg, "--all"); })) {
			Configuration::ConfigMap["CheckAllFiles"] = true;
		}
	}

	if (!std::filesystem::exists(dgrv3path)) {
		LOG("ERROR: One of the folders where the files to inject are located was not found!", HERE, "Electrohammer");
		return -1;
	}

	if (!std::filesystem::exists(where)) {
		LOG("ERROR: The folder where the files to inject are located was not found!", HERE, "Electrohammer");
		return -1;
	}

	std::string const different_hash_folders = EncryptString("different_hash_folders.txt");

	std::string const different_hashes = EncryptString("different_hashes.txt");

#ifndef DEBUG
	if (!std::filesystem::exists(dgrv3path)) {
		return -1;
	}
#endif

	// Timer benchmark
	std::chrono::time_point<std::chrono::system_clock> const start = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock> end;

	std::filesystem::path const folder_index_file = current_dir / different_hash_folders;
	std::filesystem::path const different_files_hashes = current_dir / different_hashes;

	std::vector<std::string> skipped_files{};
	std::vector<EntryMG::Entry> entries = EntryMG::CalculateEntries(folder_index_file, different_files_hashes, dgrv3path, where, current_dir, skipped_files);

	// Compile all the entries found
	Compiler::Compile(where, entries);

	LOG("Creating report..", HERE, "Electrohammer");

	// Create and save a report
	std::string const report_file = (current_dir / "eh_report.txt").string();
	if (std::filesystem::exists(report_file)) {
		std::filesystem::remove(report_file);
	}
	std::ofstream eh_report(report_file, std::ios::out | std::ios::app);
	eh_report << "";
	EntryMG::SplitType split{ {{}} };
	EntryMG::SortEntriesBySPC(entries, split);
	for (auto const& folder : split) {
		for (auto const& entry : folder) {
			std::string const new_filename = Compiler::MoveTXTForSTXTool(entry.Filename, entry.CurrentDir, false);
			auto const filepath = std::filesystem::path(entry.Filename);
			std::string const filename = filepath.parent_path().filename().string() + "/" + filepath.filename().string();
			auto const stxpath = std::filesystem::path(entry.STXFileToInsert);
			std::string const stxname = stxpath.parent_path().filename().string() + "/" + stxpath.filename().string();
			auto const spcpath = std::filesystem::path(entry.WhereTo / entry.SPCFileToInsert);
			std::string const spcname = spcpath.parent_path().filename().string() + "/" + spcpath.filename().string();
			std::string report = filename;
			while (report.length() < 80) {
				report += " ";
			}
			report += " --> " + stxname;
			while (report.length() < 160) {
				report += " ";
			}
			report += " --> " + spcname;
			eh_report << report << std::endl;
		}
	}
	eh_report.close();

	// End the benchmark timer
	end = std::chrono::system_clock::now();

	// Tell the user how many seconds elapsed

	LOG("\n", HERE, "Electrohammer");
	LOG("All done! Creating STX && Inserting SPC took: " + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(end - start).count()) + " seconds!", HERE, "Electrohammer");

	// Tell the user which files were skipped

	if (!skipped_files.empty()) {
		LOG("\n", HERE, "Electrohammer");
		LOG("Were skipped:", HERE, "Electrohammer");
		for (auto const& j : skipped_files) {
			LOG(" - " + j, HERE, "Electrohammer");
		}
	}

	Common::WaitExit();
}