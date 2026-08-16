# IMAGETOOLS - PIANIST
## Team DAIX, 2026
----

This README is the high‑level overview for **Team DAIX’s ImageTools orchestrator: Pianist**.

If you want to understand Pianist, **read the code**.  
If you can’t read the code, this repository is not for you.

**Generative AI** was used in the creation of this project’s code, comments, documentation, and miscellaneous supporting files.

Our translation was not generated or supported by AI.

-------------------------------------------------------------------------------
## IMPORTANT WARNING
-------------------------------------------------------------------------------

**IMAGETOOLS IS NOT FOR CASUAL USERS.**

If you only want to replace a few PNGs manually, use **Harmony‑Tools** or **UABEA**.

ImageTools is for:

- people modifying **every texture** in the game,  
- people who need **automated AB/SPC rebuilds**,  
- people who need **full reproducibility**,  
- people who want **daily automated builds**,  
- people who understand that this pipeline is *industrial‑strength* and not a toy.

If you want the easy path, Harmony‑Tools exists.

If you want the *complete, automated, multi‑stage graphics pipeline*, Pianist is the conductor.

-------------------------------------------------------------------------------
## WHAT PIANIST ACTUALLY DOES
-------------------------------------------------------------------------------

Pianist is the **master orchestrator** of ImageTools.  
It runs every ImageTools component in the correct canonical order:

1. **Adventurer**  
   - Clone/update the graphics repository (DGRV3‑GFX or DGRV3‑AB‑GFX).  
   - Handle beta branches.  
   - Prepare base folders (`base_spc` or `base_ab`).

2. **Inventor**  
   - Extract SPCTool / UAFGJ archives.  
   - Prepare working directories.  
   - Copy base assets into the workspace.

3. **Maid**  
   - Compile graphics (SPC/AB rebuild).  
   - Run SPCTool / UAFGJ to repack modified textures.

4. **TennisPro**  
   - Copy modified files into distribution folders.  
   - Clean up temporary files.

5. **Detective** *(DISTRIBUTE mode only)*  
   - Generate UPS patches for modified SPC/AB archives.  
   - Ensure only differences are shipped.

Pianist does **not** modify graphics itself.  
It simply **runs the entire pipeline**, checks dependencies, and ensures everything happens in the correct order.

-------------------------------------------------------------------------------
## EXPECTED REPOSITORY LAYOUT
-------------------------------------------------------------------------------

Pianist expects the graphics repository to follow this structure:

(PC/Xbox)

DGRV3-GFX/
├───base_spc/
├───boot/
├───flash/
├───MANUAL/
├───minigame/
├───noimages/

(Switch)

DGRV3-AB-GFX/
├───base_ab/
├───Data


Adventurer clones the repo and prepares the base folders.  
Inventor extracts tools into `base_spc` / `base_ab`.  
Maid rebuilds SPC/AB archives.  
TennisPro copies modified files.  
Detective generates UPS patches.

-------------------------------------------------------------------------------
## OUTPUTS
-------------------------------------------------------------------------------

ImageTools produces:

- `ModifiedFiles-GFX/` — rebuilt SPC/AB archives  
- `Distribute-GFX/` — UPS patches (Detective)  
- `imagetools_log.txt` — combined log output  
- `vgit_failed.txt` — cloning error (Adventurer)  
- `adventurer_failed.txt` — graphics repo failure

-------------------------------------------------------------------------------
## PERFORMANCE NOTES
-------------------------------------------------------------------------------

- SPC/AB rebuilds are **CPU‑heavy**.  
- Texture extraction is **I/O‑heavy**.  
- UPS patch generation is fast.  
- ImageTools does **not** use multithreading.

-------------------------------------------------------------------------------
## TROUBLESHOOTING
-------------------------------------------------------------------------------

### “adventurer_failed.txt”
Graphics repo cloning failed.  
Check your GitHub token or network connection.

### “SPCTool not found”
Inventor failed to extract archives.  
Ensure SPCTool.7z / SPCTool_Linux.7z exist in `base_spc`.

### “Textures not updated”
Maid may have failed to rebuild SPC/AB.  
Check `imagetools_log.txt`.

### “Distribution folder empty”
Detective filtered out all files because none were modified.

-------------------------------------------------------------------------------
## SUMMARY
-------------------------------------------------------------------------------

Pianist is the **conductor** of ImageTools:

- Adventurer clones  
- Inventor extracts  
- Maid compiles  
- TennisPro distributes  
- Detective patches  

If you want daily automated graphics builds for Danganronpa V3, Pianist is the tool that makes it happen.