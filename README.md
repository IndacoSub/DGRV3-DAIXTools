## DGRV3-DAIXTOOLS TOOLCHAIN — README
## Team DAIX, 2026
----

This README is the high-level overview for Team DAIX's DGRV3-TOOLS. 

Individual toolchains and tools have their own READMEs and source comments for implementation details.

PLEASE **read the code** before asking any questions.

If you can't read the code, this repository is NOT for you.

**FontTools**, as well as more documentation, are coming soon!

**Generative AI** was used in the creation of this project’s code, comments, documentation, and miscellaneous supporting files.

Our translation was not generated or supported by AI.

-------------------------------------------------------------------------------
TextTools
-------------------------------------------------------------------------------

You can find more details on the TextTools in the dedicated folder.

-------------------------------------------------------------------------------
ImageTools
-------------------------------------------------------------------------------

You can find more details on the ImageTools in the dedicated folder.

-------------------------------------------------------------------------------
FontTools
-------------------------------------------------------------------------------

Coming soon!

-------------------------------------------------------------------------------
IMPORTANT WARNING
-------------------------------------------------------------------------------
**THESE TOOLCHAINS ARE EXTREMELY COMPLEX AND ABSOLUTELY NOT FOR THE AVERAGE USER.**

If you just want to make a *meme mod*, or edit a handful of files manually, use **Harmony-Tools** instead. Harmony-Tools is simple, safe, and designed for casual modding.

This repository is for:
  
  • people editing **thousands** of files,
  
  • people who need **automated checks**, **variable replacement**, **SPC rebuilds**,
  
  • people who want **full automation**, **full reproducibility**, and **full control**,
  
  • and, frankly, **masochists**.

If you want the easy path, Harmony-Tools exists.

If you want the *complete, automated, industrial-strength pipeline*, the code is here.

-------------------------------------------------------------------------------
KEY DEPENDENCIES (summary)
-------------------------------------------------------------------------------
  • **7za** (7-Zip command-line) — required
      
	  Used to extract all .7z archives in your_repo/base_spc/
	  
      Must be provided by the user (7za.exe on Windows, 7za on PATH elsewhere).

  • **SPCTool** — ALWAYS required (SPC injection)
      
	  Even if you decide not to use STXTool, SPCTool is mandatory.
      
	  SPCTool is from: https://github.com/CaptainSwag101/DRV3-Sharp
      
	  NOTE: The version used in our toolchain(s) is an edited OLD version so it doesn't ask for user input — it simply runs the command and exit.
			

  • **SRDTool** — ALWAYS required (SRD(V) injection) for ImageTools
	  
	  SRDTool is from: https://github.com/CaptainSwag101/DRV3-Sharp
      
	  NOTE: The version used in our toolchain(s) is an edited OLD version so it doesn't ask for user input — it simply runs the command and exit.

  • **StxTool or DRV3-STX-TOOL ("NewSTXTool")** — required if you want to generate .stx from .txt
      
	  StxTool is from: https://github.com/CaptainSwag101/DRV3-Sharp
      
	  NewSTXTool is Liquid-S' DRV3-STX-TOOL, but our fork at branch “custom” (no executable will be provided; you must compile it yourself): https://github.com/IndacoSub/DRV3-STX-TOOL/tree/custom
      
	  NOTE: The version of StxTool used in our toolchain(s) is an edited OLD version so it doesn't ask for user input — it simply runs the command and exit.

  • **UAFGJ** — ALWAYS required for ImageTools (if you intend to compile for Switch)
	  
	  UAFGJ is from: https://github.com/IndacoSub/UABEA/tree/custom
  
  • **UPS binary** (ups / ups.exe) — required for WhiteSheet
      
	  Used to generate UPS patches for distribution.
      You must provide your own build of UPS: https://github.com/rameshvarun/ups

  • **dotnet runtime / SDK**
      
	  Needed for:
        - Bugvac (C#)
        - LooseFloorboard (C#)
		- UAFGJ
        - Some STX/SPC tools depending on platform
        - General DGRV3-DAIXTOOLS helper utilities

  • **External SHA library** — required for Necronomicon
      
	  TextTools/Necronomicon/calc_sha.h expects a SHA-512 (or similar) implementation.
      
	  The public repo DOES NOT bundle any SHA library.
      
	  YOU must provide one and include it in calc_sha.h
	  
-------------------------------------------------------------------------------
BUILD & RUN (recommended)
-------------------------------------------------------------------------------
1) Clone the repository: `git clone <repo-url> --recursive`, then `cd <repo-root>`

2) Provide external tools and archives:
   - Place STX/SPC tools, UPS binary, and 7za in your_repo/base_spc/ as described above.
   - Provide a SHA library and adapt Necronomicon/calc_sha.h to include it.

3) Build C++ projects:
   - Use your preferred method (CMake, Visual Studio, or manual).
   - Example (CMake): `mkdir build && cd build`, `cmake .. -DCMAKE_BUILD_TYPE=Release` then `cmake --build . --config Release`
	
	Please use Visual Studio.

4) Configure:
   - Edit TextConfig.config / ImageConfig.config / FontConfig.config to set flags

5) Run (HiddenDoor for TextTools, Pianist for ImageTools, TODO for others)

To run a single tool instead of the whole toolchain, execute its binary directly (examples provided): `HydraulicPress.exe`, `Electrohammer.exe`, `WhiteSheet.exe part2`

-------------------------------------------------------------------------------
LEGAL & LICENSE
-------------------------------------------------------------------------------
  • Do not commit proprietary binaries or game assets to the public repository.
  
  • The toolchain intentionally avoids bundling certain third-party libraries.
    You must obtain and provide those binaries yourself.
  
  • Respect the licenses of any third-party tools you use.
  
  • This repository is licensed under the ISC license, with exemptions for specific companies or groups noted below:
  
	Spike Chunsoft Co., Ltd. may, at their discretion, relicense any code under either the MIT license or the GPLv2 license.
	Too Kyo Games, LLC may, at their discretion, relicense any code under either the MIT license or the GPLv2 license.

-------------------------------------------------------------------------------
CONTRIBUTING
-------------------------------------------------------------------------------
  • Fork the repo, create a feature branch, and open a pull request.
  
  • Run the tools locally to validate the full pipeline before submitting.
  
  • Consider adding tests or sample inputs for any behavioral changes.
  
  • Document any new external dependency in this README.