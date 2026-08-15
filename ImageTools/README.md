# DGRV3‑DAIXTOOLS TOOLCHAIN — README (ImageTools)  
## Team DAIX, 2026
----

This README is the high-level overview for Team DAIX's ImageTools from DGRV3-DAIXTOOLS. 

Individual tools have their own READMEs and source comments for implementation details (COMING SOON).

PLEASE **read the code** before asking any questions.

If you can't read the code, this repository is NOT for you.

**Generative AI** was used in the creation of this project’s code, comments, documentation, and miscellaneous supporting files.

Our translation was not generated or supported by AI.

-------------------------------------------------------------------------------
## IMPORTANT WARNING
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
## IMAGETOOLS PIPELINE (canonical order)
-------------------------------------------------------------------------------

This pipeline is executed automatically by **Pianist**, the master orchestrator.

You can run tools individually for debugging, but Pianist executes them in this order:

0. Pianist        — run everything

1. Adventurer     — clone/update graphics repositories - DGRV3-GFX for PC/Xbox, DGRV3-AB-GFX for Switch

2. Inventor       — extract archives, tools, and base assets

3. Maid           — compile images (PC/Xbox: SRD/SPC; Switch: AB/assets)

4. TennisPro      — collect compiled SPC/AB files

5. Detective      — generate UPS patches (distribution mode)

If DISTRIBUTE mode is enabled, Pianist includes Detective automatically.

-------------------------------------------------------------------------------
## EXPECTED DGRV3-GFX REPO LAYOUT
-------------------------------------------------------------------------------

**(PC/Xbox only)**

Your graphics repo must follow this structure (read the code - especially Maid - for more details):

C:\path\to\DGRV3-GFX\

	├───

	base_spc/
	
	├───
	
	boot/
	
	├───
	
	flash/
	
	├───
	
	MANUAL/
	
	├───
	
	minigame/
	
	├───
	
	noimages/
	
	├───
	
	...

Notes:

- **base_spc** contains tool .7z files.  
- Adventurer clones the repo; Inventor extracts archives into base_spc.  
- Maid expects the repo to be **clean**, **untouched**, and **unmodified** before compilation.

-------------------------------------------------------------------------------
## EXPECTED DGRV3-AB-GFX REPO LAYOUT
-------------------------------------------------------------------------------

**(Switch only)**

Your graphics repo must follow this structure (read the code - especially Maid - for more details):

C:\path\to\DGRV3-AB-GFX\

	├───

	base_ab/
	
	├───
	
	Data/
	
	├───
	
	...

Notes:

- **base_ab** contains UAFGJ as well as the other tools such as SRDTool and SPCTool.  
- Adventurer clones the repo; Inventor extracts archives into base_ab.  
- Maid expects the repo to be **clean**, **untouched**, and **unmodified** before compilation.

-------------------------------------------------------------------------------
## REQUIRED .7z FILES & HELPERS (place in base_spc or base_ab)
-------------------------------------------------------------------------------

Assuming you're going to be using FileOnDemand (explained below),

You must provide the following archives and helper executables:

- `7za.exe` / `7za`
- `SPCTool.7z`
- `SPCTool_Linux.7z`
- `SRDTool.7z`
- `SRDTool_Linux.7z`
- `NewSTXTool.7z`
- `NewSTXTool_Linux.7z`
- `ups.exe` / `ups`

### Switch
- Everything above, plus:
- `UAFGJ.7z` (Team DAIX’s Unity injector)

NOTICE: We will **absolutely not** provide any of these files.

-------------------------------------------------------------------------------
## CONFIGURATION (ImageConfig.config)
-------------------------------------------------------------------------------

ImageConfig.config is the central configuration file used by ALL ImageTools.
It **must be placed in the same directory as the tool executables**.

**A sample ImageConfig.config is provided** in the Release folder. It is up to you to copy to the right location.

The ImageTools configuration file is divided into sections:

### [PLATFORM]
- `UseSwitchConfiguration` — Switch/Unity pipeline  
- `UseXboxConfiguration` — Xbox pipeline  
- Otherwise PC pipeline is used.

### [FILES]
- `FileOnDemand` — "download" missing SPC/AB files individually (see Common/Cloud.h and Maid)
- `UseALTs` — enable ALT_ PNG replacement

### [COMPILATION]
- `MultithreadedCompilation` — parallel SRD/SPC/AB compilation

If the file is missing or placed elsewhere, tools will either fail, fall back to defaults, or behave unpredictably.

-------------------------------------------------------------------------------
## COMMON OUTPUTS
-------------------------------------------------------------------------------

ImageTools produces:

- `ModifiedFiles-GFX/` — compiled SPC/AB files  
- `Distribute-GFX/` — UPS patches (Detective)  
- `arrow_report.txt` — PNG → SRD → SPC or PNG → AB mapping  
- `adventurer_failed.txt` — cloning error  
- `vgit_failed.txt` — Git error  

-------------------------------------------------------------------------------
## MEMORY & PERFORMANCE NOTES
-------------------------------------------------------------------------------

- SRD/SPC compilation is CPU-heavy.  
- Unity asset injection (UAFGJ) is I/O-heavy.  
- Multithreading significantly improves performance.  
- FileOnDemand reduces disk usage but increases network usage.

-------------------------------------------------------------------------------
## TROUBLESHOOTING
-------------------------------------------------------------------------------

### “Tool not found”
Ensure SPCTool, SRDTool, UAFGJ, and UPS are placed in base_spc or base_ab.

### “Invalid SPC/AB”
Check that your base archives are **untouched** originals.

### “UPS too small”
Detective deletes UPS files smaller than 0x20 bytes.

### “Compilation hangs”
Multithreaded mode has a 5-minute timeout per batch.

### “ALT not applied”
Ensure `UseALTs = true` in ImageConfig.config.