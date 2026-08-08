// Team DAIX, 2026
// STACKEDBOOKS

// The majority of this code was written between 2020 and 2022

// This tool's purpose is to:
// Literally just run HiddenDoor

#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>

#include "../Common/Common.h"

int main(int argc, char* argv[]) {

	bool all = false;

	std::vector<std::string> args(argv, argv + argc);

	if (argc > 0) {
		if (std::any_of(args.begin(), args.end(), [](std::string const& arg) -> bool {return Common::StringContains(arg, "--all"); })) {
			all = true;
		}
	}

	std::string args_str = std::accumulate(args.begin(), args.end(), std::string{" "});

	std::filesystem::path const current_dir = std::filesystem::current_path();

	Common::DoLog("Starting", HERE, "StackedBooks");

#ifdef _WIN32
	std::string const hiddendoor = EncryptString("HiddenDoor.exe");
#else
	std::string const hiddendoor = EncryptString("./HiddenDoor");
#endif
	if (!std::filesystem::exists(current_dir / hiddendoor)) {
		LOG("ERROR: \"" + hiddendoor + "\" could not be found!", HERE, "HiddenDoor");
		LOG("The execution of the program cannot continue.", HERE, "HiddenDoor");
		return -1;
	}
	std::string const command = hiddendoor + " " + args_str;
	//std::cout << "HiddenDoor executable: " << hiddendoor << std::endl;
	Common::executeBatch(command.c_str());
}