## DGRV3-DAIXTOOLS TOOLCHAIN — README (TextTools)
## Team DAIX, 2026
----

This README is the high-level overview for Team DAIX's TextTools from DGRV3-DAIXTOOLS. 

Individual tools (HydraulicPress, Electrohammer, Necronomicon, WhiteSheet, Crossbow, etc.) have their own READMEs and source comments for implementation details (COMING SOON).

PLEASE **read the code** before asking any questions.

If you can't read the code, this repository is NOT for you.

**Generative AI** was used in the creation of this project’s code, comments, documentation, and miscellaneous supporting files.

Our translation was not generated or supported by AI.
  
-------------------------------------------------------------------------------
IMPORTANT WARNING
-------------------------------------------------------------------------------
**THIS TOOLCHAIN IS EXTREMELY COMPLEX AND ABSOLUTELY NOT FOR THE AVERAGE USER.**

If you just want to make a *meme mod*, or edit a handful of files manually, use **Harmony-Tools** instead. Harmony-Tools is simple, safe, and designed for casual modding.

This repository is for:
  
  • people editing **thousands** of files,
  
  • people who need **automated checks**, **variable replacement**, **SPC rebuilds**,
  
  • people who want **full automation**, **full reproducibility**, and **full control**,
  
  • and, frankly, **masochists**.

If you want the easy path, Harmony-Tools exists.

If you want the *complete, automated, industrial-strength pipeline*, the code is here.

-------------------------------------------------------------------------------
TEXTTOOLS PIPELINE (canonical order)
-------------------------------------------------------------------------------
StackedBooks runs HiddenDoor; HiddenDoor runs the tools in this order by default.

You can run tools individually for debugging; HiddenDoor executes them automatically in sequence.

  0) **StackedBooks**       — run HiddenDoor
  1) **HiddenDoor**         — run everything
  2) **Ropeway**            — clone/update repositories (git)
  3) **CrammedPiranhas**    — unpack archives / prepare files
  4) **MarkerStone**        — line counting and sanity checks
  5) **Necronomicon**       — compute file hashes and produce different_* lists
  6) **Bugvac**             — name/surname replacements (C#)
  7) **HydraulicPress**     — variable replacement, platform filtering, baking
  8) **PoolRules**          — translation percentage & charcount analysis
  9) **Electrohammer**      — STX generation and SPC injection
 10) **Crossbow**           — collect modified SPC/PB files (part1)
 11) **CrammedPiranhas (part2)** — second-stage unpack
 12) **WhiteSheet**         — generate UPS patches (part1)
 13) **Crossbow (part2)**   — distribution copy for part2 artifacts
 14) **Necronomicon (part2)** — hashing/verification for part2 artifacts
 15) **WhiteSheet (part2)** — generate UPS patches for part2 artifacts

Several tools are split into "part2" stages (Crossbow part2, WhiteSheet part2, etc.).

HiddenDoor runs those part2 stages in the canonical sequence above when DISTRIBUTE mode is enabled.

-------------------------------------------------------------------------------
EXPECTED DGRV3 REPO LAYOUT (required)
-------------------------------------------------------------------------------
Your DGRV3 repo must follow this tree.

C:\path\to\DGRV3\

	├───ainori

	├───base_spc

	├───chapter1

	├───chapter2

	├───chapter3

	├───chapter4

	├───chapter5

	├───chapter6

	├───epilogue

	├───gallery

	├───game_resident

	├───i18n

	├───MapObjName

	├───prologue

	├───subroutine

	└───test

Notes:
  - Except for `base_spc`, the folders above should contain only .txt or .pb script files (the pipeline expects text assets in these directories).
  - `vars_bak.txt` must be present in the repo root (HydraulicPress uses `vars.txt` first, then falls back to `vars_bak.txt`).
  
`vars_bak.txt` is NOT provided, at least for now.

-------------------------------------------------------------------------------
REQUIRED .7z FILES & HELPERS (place in DGRV3/base_spc/)
-------------------------------------------------------------------------------
You must provide the following archives and helper executables in DGRV3/base_spc/ (names shown as examples; case may matter on some platforms):

  - `7za.exe` (or ensure 7za is on PATH, if on Linux)
  - `danganronpa_spc_legacy.7z`
  - `danganronpa_spc_switch.7z`
  - `danganronpa_spc_xbox.7z`
  - `gr_font.7z`
  - `i18n_switch.7z`
  - `i18n_xbox.7z`
  - `NewSTXTool.7z`
  - `NewSTXTool_Linux.7z`
  - `SPCTool.7z`
  - `SPCTool_Linux.7z`
  - `STXTool.7z`
  - `STXTool_Linux.7z`
  - `trial_english.7z`
  - `trial_french.7z`
  - `ups` (Linux binary)
  - `ups.exe` (Windows binary)

**The repository does not include these files; you must obtain them, sometimes 7zip them, and then place them in base_spc/**.

-------------------------------------------------------------------------------
BASE_SPC FOLDER CONTENT EXAMPLE
-------------------------------------------------------------------------------

(ONLY use new, "untouched", files, otherwise UPS patching will 100% fail)

- **danganronpa_spc_legacy.7z** (and similar) should contain files like `ainori_text_US.SPC`, `chap0_text_US.SPC` etc. from `wrd_script/007`

- **gr_font.7z** contains the font `.stx` and `.srdv` files contained inside `game_resident_US`.

- **trial_english** and **trial_french** are just the trial minigame SPC files from the english and french versions of the game respectively.

- **i18n files** are simply .pb files taken from the **Nintendo Switch** version of the game.

NOTICE: We will **absolutely not** provide any of these files.

-------------------------------------------------------------------------------
CONFIGURATION (TextConfig.config)
-------------------------------------------------------------------------------
TextConfig.config is the central configuration file used by ALL TextTools.
It **must be placed in the same directory as the tool executables**.

**A sample TextConfig.config is provided** in the Release folder. It is up to you to copy to the right location.

Every tool (HydraulicPress, Electrohammer, etc.) loads TextConfig.config from the same folder as the executable.

If the file is missing or placed elsewhere, tools will either fail, fall back to defaults, or behave unpredictably.

The TextTools configuration file is divided into sections:

[PLATFORM]

  **UseSwitchConfiguration**   : Enables Switch-specific behavior (accent replacement, i18n files, etc.). If FALSE, PC/Xbox behavior is used.

[CLONING]

  **CloneBetas**               : Whether Ropeway should also clone beta branches.

[CHARACTERS]

  **ReplaceBlacklistedChars**  : If TRUE, runs LooseFloorboard to replace accented characters. Required for platforms that do not support accents.

[VARIABLES]

  **UseIceConvention**         : (DEPRECATED, DOES NOTHING) Enables "Ice-style" variable naming conventions.
  
  **DoCheckVariables**         : Checks for missing VAR_* variables (requires ~7GB RAM).
  
  **DoSwapNames**              : Optional name-swapping behavior.
  
  **DoRemoveSignals**          : Removes SIGNAL_* tags from text.
  
  **DoReplaceEmpty**           : Replaces empty lines with EMPTY_<filename>L<line> markers.

[FILES]
  
  **CheckAllFiles**            : If TRUE, ignore different_hashes.txt and process ALL files. Otherwise, only changed files (Necronomicon output) are processed.

[RANDOMIZATION]
  
  **DoRandomize**              : Enables the Randomizer (requires ~7GB RAM).
  
  **SmartRandomization**       : Enables smarter randomization rules.

[UTILITY]
  
  **DoCountWords**             : Enables word counting (requires ~15GB RAM).

[BAKED]
  
  **NoCLTInBaked**             : Removes CLT_* markers from baked output.
  
  **NoPlatformInBaked**        : Removes <PLATFORM_*> tags from baked output.

-------------------------------------------------------------------------------
MEMORY & PERFORMANCE NOTES
-------------------------------------------------------------------------------
  - Variable checking and randomization are memory-intensive:
      * Variable checking / Randomizer: ~7 GB recommended minimum
      * Word counting: ~15 GB recommended
  - Electrohammer can run multithreaded; it uses hardware_concurrency to determine parallelism. If you have many cores, compilation will be faster.
  - If you run on low-memory machines, disable DoCheckVariables, DoRandomize, and DoCountWords in TextConfig.config.

-------------------------------------------------------------------------------
COMMON OUTPUTS (files & folders produced)
-------------------------------------------------------------------------------
  - ModifiedFiles/             : collected modified SPC/PB files
  - Distribute/                : distribution folder (output)
  - Baked/                     : baked .txt outputs (if baked mode enabled)
  - var_replace_map.txt        : log of variable replacements
  - randomizer_report.txt      : randomizer mapping report
  - words_counted.txt          : top word frequency list
  - variablechecker.txt        : detailed missing-variable report
  - variablechecker_short.txt  : list of missing variable names
  - different_hashes.txt       : list of files with changed hashes (Necronomicon)
  - different_part2.txt        : list of SPCs for part2 UPS generation
  - file_copied.txt            : list of files copied by Crossbow
  - percentage_res.txt         : per-chapter completion ratios (PoolRules)
  - charcount_res.txt          : per-chapter character counts
  - detailed_charcount_rex.txt : detailed per-file breakdown

-------------------------------------------------------------------------------
TROUBLESHOOTING & NOTES
-------------------------------------------------------------------------------
  • "**Tool not found**" errors:
      Ensure the expected executables are present in the repo root or tool directories.

  • "**UPS patcher not found**":
      Place the UPS binary in DGRV3/base_spc/ or update WhiteSheet to point to the correct path.

  • "**Insufficient RAM**" warnings:
      Disable heavy features in TextConfig.config or run on a machine with more memory.

  • "**Line mismatch**" errors (PoolRules / MarkerStone):
      Check for stray CR/LF differences, bracketed lines ("{" / "}"), or platform tags that may cause line offsets. Use file_line_test.txt to inspect raw lines and ASCII codes.

  • "**Variable not found**" (VariableChecker):
      Ensure vars_bak.txt / vars.txt contains the variable definitions. Variable names are matched literally; check for stray CLT or SIGNAL tags.

  • "**STX/SPC tool failures**":
      Confirm dotnet runtime is installed and the STX/SPC tools are the correct versions for your platform.

  • "**UPS files too small**" after WhiteSheet:
      WhiteSheet deletes UPS files smaller than an arbitrary threshold (0x20 bytes). If your UPS tool produces small files, verify the UPS tool invocation and base/modified inputs.

-------------------------------------------------------------------------------
END
===============================================================================
