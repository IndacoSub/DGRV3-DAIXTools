// Team DAIX, 2026
// V3DAILYMANAGER — PROCESS EXECUTION HELPERS
//
// The majority of this code was written between 2020 and 2022
//
// This file contains helper functions used by V3DailyManager to run external
// toolchains (TextTools, ImageTools, FontTools) for daily automated builds.
//
// DAILY does not compile anything itself — it simply launches the correct
// external executables depending on the selected platform and configuration.
//
// ---------------------------------------------------------------------------
// WHAT IS DGRV3-TOOLS?
// ---------------------------------------------------------------------------
// DGRV3-Tools is a remote GitHub repository cloned by V3DailyManager at
// runtime. It contains ALL toolchain folders and ALL configuration folders
// required to run TextTools, ImageTools, and FontTools.
//
// IMPORTANT STRUCTURE NOTE:
//   • The folders *TextTools/*, *ImageTools/*, and *FontTools/*
//     DO NOT contain configuration files.
//     They only contain:
//         - Tools.7z / Tools_Linux.7z
//         - 7za.exe
//         - optional unpackers (V3TextUnpacker.exe, etc. which DAILY ignores)
//
//   • ALL configuration folders live OUTSIDE those tool folders, e.g.:
//         - ForDaily/
//         - ForDailyPrivate/
//         - TextInjector/
//         - TextInjectorForSwitch/
//         - TextInjectorForXbox/
//         - TextInjectorRelease/
//         - TextCrazy/
//         - TextRandomizer/
//		   - TextRandomizerDumb/
//         - TextRandomizerDumbAll/
//         - ImageInjector/
//         - ImageInjectorForSwitch/
//         - ImageInjectorForXbox/
//         - ImageInjectorNoMultithread/
//
//     These folders contain the actual *.config files used by DAILY.
//
// Process.cpp intentionally avoids searching for configs inside
// TextTools/ImageTools/FontTools because those folders contain ONLY the tool
// archives, not configuration data.
//
// ---------------------------------------------------------------------------
//
// These helpers provide:
//   • Platform → string conversion (GetPlatformName)
//   • Execution wrappers for:
//         - TextTools (StackedBooks)
//         - ImageTools (Pianist)
//         - FontTools (Monokuma)
//
// Each function:
//   • Locates the correct tool folder inside DGRV3-Tools
//   • Extracts Tools.7z into the working directory
//   • Copies the selected configuration folder (never from TextTools/ImageTools/FontTools)
//   • Runs the appropriate executable
//
// DAILY uses these helpers to perform multi‑platform builds cleanly and
// consistently.


#include "process.h"

#include "common.h"

#include <iostream>

namespace Process {

	// GetPlatformName()
	// Converts a Platform enum value into a human‑readable string.
	//
	// DAILY uses this for:
	//   • Logging
	//   • Status messages
	//   • README updates
	//
	// Unsupported platforms return "Unknown".

	std::string GetPlatformName(Platform const& p) {

		switch (p) {
		case Platform::None:
			return "None";
		case Platform::PC:
			// Steam
			return "PC";
		case Platform::Switch:
			return "Switch";
		case Platform::Xbox:
			return "Xbox";
		case Platform::PSVita:
			return "PSVita";
		case Platform::PS4:
			return "PS4";
		case Platform::Android:
			return "Android";
		case Platform::iOS:
			return "iOS";
		case Platform::Num:
		default:
			return "Unknown";
		}
	}

	// RunTextTools()
	// Launches the TextTools pipeline (StackedBooks) for the selected platform.
	//
	// Steps:
	//   1. Determine which TextTools config folder to use
	//      - PC: ForDaily / ForDailyPrivate / TextInjector / TextRandomizer
	//      - Switch: TextInjectorForSwitch
	//      - Xbox: unsupported (returns false)
	//
	//   2. Extract Tools.7z (or Tools_Linux.7z) into the working directory
	//   3. Copy the selected config folder into the working directory
	//   4. Run StackedBooks.exe (or ./StackedBooks)
	//
	// DAILY uses this to generate:
	//   • Baked text files
	//   • Logs
	//   • Translation output

	bool RunTextTools(fsys::path const& where, fsys::path const& current_dir, Platform const& p) {

		// Randomizer
		constexpr static bool randomizer = false;

		constexpr static bool debug = true;

		// If the user did not specify --text-config, DAILY selects a default config
		// based on platform and private_repo/debug/randomizer flags.

		std::string ConfigPath = specific_text_config;
		if (ConfigPath.empty()) {
			std::cout << "ConfigPath is empty, replacing ConfigPath" << std::endl;
			switch (p) {
			case Platform::PC:
				ConfigPath = debug ? (randomizer ? "TextRandomizer" : (private_repo ? "ForDailyPrivate" : "ForDaily")) : "TextInjector";
				break;
			case Platform::Switch:
				ConfigPath = "TextInjectorForSwitch";
				break;
			case Platform::Xbox:
				ConfigPath = "TextInjectorForXbox";
				std::cout << "Xbox text not supported" << std::endl;
				return false;	// Xbox not supported, for now
				break;
			default:
				return false;
			}
		}

		std::cout << "Selected text config: " << (ConfigPath.length() > 0 ? ConfigPath : "Invalid Config") << std::endl;
		std::cout << std::endl;

		// Extract Tools.7z using 7za.exe (Windows) or /usr/bin/7za (Linux).

		fsys::path const _7zipa = where / EncryptString("TextTools") / EncryptString("7za.exe");
#ifdef _WIN32
		if (!fsys::exists(_7zipa)) {
			std::cout << "7za not found (" + _7zipa.string() + ") !" << std::endl;
			return false;
		}
#endif

#ifdef _WIN32
		std::string const tools_name = "Tools.7z";
#else
		std::string const tools_name = "Tools_Linux.7z";
#endif

		std::string const _7zfile = (where / EncryptString("TextTools") / tools_name).string();

#ifdef _WIN32
		std::string const _7zacall = "\"" + _7zipa.string() + "\"";
		std::string const _7zcommand = _7zacall + std::string{ " x " } + std::string{ "-mtc=off " } + "\"" + _7zfile + "\" -o\"" + current_dir.string() + "\" -aoa -y";
#else
		std::string const _7zacall = "7za";
		std::string const _7zcommand = "/usr/bin/7za " + std::string{ " x " } + std::string{ "-mtc=off " } + "\"" + _7zfile + "\" -aoa -y";
#endif
		std::cout << "7z command: " << _7zcommand << std::endl;
		Common::executeBatch(_7zcommand.c_str());

		std::cout << "7z operation completed successfully!" << std::endl;

		// Copy the selected configuration folder into the working directory.

		if (ConfigPath.length() > 0) {
			fsys::copy(where / ConfigPath, current_dir, fsys::copy_options::recursive | fsys::copy_options::overwrite_existing);
		}

		std::cout << "Configuration copied!" << std::endl;

		// Run StackedBooks (TextTools main executable).

#ifdef _WIN32
		std::string const ext = ".exe";
		std::string const before = "";
#else
		Common::executeBatch("chmod -R +x *");

		std::string const ext = "";
		std::string const before = "./";
#endif

		std::string const command = before + EncryptString("StackedBooks") + ext;

		fsys::path commandfs((current_dir / command));
		if (!fsys::exists(commandfs)) {
			std::cout << "Could not find any program to launch: " << commandfs.string() << std::endl;
			return false;
		}
		std::cout << "Running TextTools..." << std::endl;

		Common::executeBatch(command.c_str());

		std::cout << "Text OK" << std::endl;
		return true;
	}

	// RunImageTools()
	// Launches the ImageTools pipeline (Pianist) for the selected platform.
	//
	// Steps:
	//   1. Determine which ImageTools config folder to use
	//      - PC: ImageInjector
	//      - Switch: ImageInjectorForSwitch
	//      - Xbox: unsupported (returns false)
	//
	//   2. Extract Tools.7z (or Tools_Linux.7z)
	//   3. Copy the selected config folder
	//   4. Run Pianist.exe (or ./Pianist)
	//
	// DAILY uses this to generate:
	//   • ModifiedFiles-GFX
	//   • Distribute-GFX
	//   • SPC/AB patches for graphics


	bool RunImageTools(fsys::path const& where, fsys::path const& current_dir, Platform const& p) {

		std::string ConfigPath = specific_gfx_config;

		if (ConfigPath.empty()) {
			switch (p) {
			case Platform::PC:
				ConfigPath = "ImageInjector";
				break;
			case Platform::Switch:
				ConfigPath = "ImageInjectorForSwitch";
				break;
			case Platform::Xbox:
				ConfigPath = "ImageInjectorForXbox";
				std::cout << "Xbox images not supported" << std::endl;
				return false;	// Xbox not supported, for now
				break;
			default:
				return false;
			}
		}

		std::cout << "Selected config: " << (ConfigPath.length() > 0 ? ConfigPath : "Invalid Config") << std::endl;

		// Extract Tools.7z for ImageTools.

		fsys::path const _7zipa = where / EncryptString("ImageTools") / EncryptString("7za.exe");
#ifdef _WIN32
		if (!fsys::exists(_7zipa)) {
			std::cout << "7za not found!" << std::endl;
			return false;
		}
#endif

#ifdef _WIN32
		std::string const tools_name = "Tools.7z";
#else
		std::string const tools_name = "Tools_Linux.7z";
#endif

		std::string const _7zfile = (where / EncryptString("ImageTools") / tools_name).string();

#ifdef _WIN32
		std::string const _7zacall = "\"" + _7zipa.string() + "\"";
		std::string const _7zcommand = _7zacall + std::string{ " x " } + std::string{ "-mtc=off " } + "\"" + _7zfile + "\" -o\"" + current_dir.string() + "\" -aoa -y";
#else
		std::string const _7zacall = "7za";
		std::string const _7zcommand = "/usr/bin/7za " + std::string{ " x " } + std::string{ "-mtc=off " } + "\"" + _7zfile + "\" -aoa -y";
#endif

		std::cout << "7z command: " << _7zcommand << std::endl;
		Common::executeBatch(_7zcommand.c_str());

		// Copy the selected ImageTools configuration.

		if (ConfigPath.length() > 0) {
			fsys::copy(where / ConfigPath, current_dir, fsys::copy_options::recursive | fsys::copy_options::overwrite_existing);
		}

		// Run Pianist (ImageTools orchestrator).

#ifdef _WIN32
		std::string const ext = ".exe";
		std::string const before = "";
#else
		Common::executeBatch("chmod -R +x *");

		std::string const ext = "";
		std::string const before = "./";
#endif

		std::string const command = before + EncryptString("Pianist") + ext;

		if (!fsys::exists((current_dir / command))) {
			std::cout << "Could not find any program to launch: " << command << std::endl;
			return false;
		}

		Common::executeBatch(command.c_str());

		std::cout << "Images OK" << std::endl;

		return true;
	}

	// RunFontTools()
	// Launches the FontTools pipeline (Monokuma) for the selected platform.
	//
	// Notes:
	//   • Only PC is supported.
	//   • Switch and Xbox are explicitly unsupported.
	//   • ConfigPath is optional; DAILY normally runs FontTools without a config.
	//
	// Steps:
	//   1. Extract Tools.7z (or Tools_Linux.7z)
	//   2. Copy the selected config folder (if any)
	//   3. Run Monokuma.exe (or ./Monokuma)
	//
	// DAILY uses this to generate:
	//   • ModifiedFiles-Font
	//   • Distribute-Font
	//   • Updated gr_font.7z inside DGRV3/base_spc

	bool RunFontTools(fsys::path const& where, fsys::path const& current_dir, Platform const& p) {

		std::string ConfigPath = specific_font_config;

		if (ConfigPath.empty()) {
			switch (p) {
			case Platform::PC:
				//ConfigPath = "FontTools";
				break;
			case Platform::Xbox:
				std::cout << "Xbox version unsupported" << std::endl;
				// Xbox version unsupported, for now
				return false;
			case Platform::Switch:
			default:
				std::cout << "Font tools incompatible" << std::endl;
				// Font tools incompatible
				return false;
			}
		}

		std::cout << "Selected config: " << (ConfigPath.length() > 0 ? ConfigPath : "Invalid Config") << std::endl;

		// Extract Tools.7z for FontTools.

		fsys::path const _7zipa = where / EncryptString("FontTools") / EncryptString("7za.exe");
#ifdef _WIN32
		if (!fsys::exists(_7zipa)) {
			std::cout << "7za not found!" << std::endl;
			return false;
		}
#endif

#ifdef _WIN32
		std::string const tools_name = "Tools.7z";
#else
		std::string const tools_name = "Tools_Linux.7z";
#endif

		std::string const _7zfile = (where / EncryptString("FontTools") / tools_name).string();

#ifdef _WIN32
		std::string const _7zacall = "\"" + _7zipa.string() + "\"";
		std::string const _7zcommand = _7zacall + std::string{ " x " } + std::string{ "-mtc=off " } + "\"" + _7zfile + "\" -o\"" + current_dir.string() + "\" -aoa -y";
#else
		std::string const _7zacall = "7za";
		std::string const _7zcommand = "/usr/bin/7za " + std::string{ " x " } + std::string{ "-mtc=off " } + "\"" + _7zfile + "\" -aoa -y";
#endif

		std::cout << "7z command: " << _7zcommand << std::endl;
		Common::executeBatch(_7zcommand.c_str());

		// Copy risky config folder (if provided).

		std::cout << "Attempting risky copy" << std::endl;

		if (ConfigPath.length() > 0) {
			fsys::copy(where / ConfigPath, current_dir, fsys::copy_options::recursive | fsys::copy_options::overwrite_existing);
		}

		// Run Monokuma (FontTools orchestrator).

#ifdef _WIN32
		std::string const ext = ".exe";
		std::string const before = "";
#else
		Common::executeBatch("chmod -R +x *");

		std::string const ext = "";
		std::string const before = "./";
#endif

		std::string const command = before + EncryptString("Monokuma") + ext;

		if (!fsys::exists((current_dir / command))) {
			std::cout << "Could not find any program to launch!" << std::endl;
			return false;
		}

		Common::executeBatch(command.c_str());

		std::cout << "Font OK" << std::endl;

		return true;
	}
}