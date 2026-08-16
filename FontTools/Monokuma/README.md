# FONTTOOLS — MONOKUMA  
## Team DAIX, 2026  

---

Monokuma is the **central controller** of the FontTools suite.  
If FontTools were a factory, **Monokuma is the conveyor belt** that moves each stage forward in the correct order, checks that every machine is working, and finally packages the finished product.

It does **not** compile fonts itself.  
It does **not** extract archives itself.  
It does **not** generate UPS patches itself.

Instead, Monokuma:

- **Runs every FontTools component in the canonical order**
- **Verifies dependencies before starting**
- **Stops the pipeline if any stage reports failure**
- **Packages the final game_resident font (v3_font00) into gr_font.7z**
- **Ensures reproducibility and consistency across all runs**

Monokuma is the FontTools equivalent of **Pianist** from ImageTools and **StackedBooks** from TextTools — the “master orchestrator.”

---

## ⚠️ IMPORTANT WARNING  
Monokuma executes **multiple destructive tools** (Monodam, Monokid, Monosuke).  
These tools modify SPC archives, delete temporary files, and overwrite existing font assets.

If your DGRV3-Font repository is not clean, **you WILL lose data**.

Monokuma assumes:

- The repository was cloned by **Monotaro**
- The base fonts were extracted by **Monophanie**
- The working directory contains **no unrelated files**
- You understand exactly what FontTools does

If you do not understand the pipeline, **stop here**.

---

## 📦 FULL PIPELINE (canonical order)

Monokuma runs the following tools **in this exact sequence**:

### 1. **Monotaro** — Repository Manager  
- Clones DGRV3-Font  
- Optionally clones beta branches  
- Filters out unwanted branches  
- Writes `vgit_failed.txt` on error  

If Monotaro fails, Monokuma **aborts immediately**.

---

### 2. **Monophanie** — Extractor & Font Duplicator  
- Extracts SPCTool and HTFont archives  
- Copies base font assets  
- Creates trial_font variants  
- Renames STX/SRDV files  
- Prepares the entire working directory  

If Monophanie fails, Monokuma **aborts immediately**.

---

### 3. **Monodam** — Font Compiler  
- Runs HTFont to generate `.srd` and `.srdv`  
- Converts `.srd → .stx`  
- Inserts STX/SRDV into SPC archives  
- Logs modified SPCs into `list_changed.txt`  

If Monodam fails, Monokuma **aborts immediately**.

---

### 4. **Monokid** — SPC Filter  
- Removes SPCs that were not modified  
- Keeps only the SPCs listed in `list_changed.txt`  
- Prepares the distribution folder  

If Monokid fails, Monokuma **aborts immediately**.

---

### 5. **Monosuke** — UPS Patch Generator *(only if DISTRIBUTE is enabled)*  
- Generates UPS patches for modified SPCs  
- Places them into `Distribute-Font/`  

If Monosuke fails, Monokuma **aborts immediately**.

---

## 🧩 FINAL PACKAGING (gr_font.7z)

After all tools finish successfully, Monokuma:

1. Locates the main DGRV3 repository  
2. Deletes the existing `gr_font.7z`  
3. Creates a new `gr_font.7z` containing:
   - `v3_font00.stx`
   - `v3_font00.srdv`

These files come from:

DGRV3-Font/game_resident/game_resident_US_DEC/


This ensures the main game repository always contains the **latest compiled font00**.

---

## 🔧 DEPENDENCY CHECKS

Before running anything, Monokuma verifies:

- **Git** (`git --version`)
- **.NET** (`dotnet --info`)
- **7za** (Windows: `7za.exe`, Linux: `7za`)

If any dependency is missing, Monokuma **refuses to continue**.

---

## 🧠 DESIGN PHILOSOPHY

Monokuma is intentionally:

- **Sequential** — no parallel execution  
- **Strict** — aborts on any error  
- **Deterministic** — same input → same output  
- **Verbose** — logs every step  
- **Safe-ish** — stops if Monotaro or Ropeway fail  

FontTools is not designed for casual modding.  
Monokuma ensures that **every run is clean, reproducible, and complete**.

---

## 📁 EXPECTED DIRECTORY STRUCTURE

Monokuma expects the following layout:

DGRV3-Font/
├───base_spc/
├───└───base_fonts/
├───└───HTFont.7z
├───└───HTFont_Linux.7z
├───└───SpcTool.7z
├───└───SpcTool_Linux.7z
├───└───7za.exe
├───└───assimp.dll / libassimp.so
├───trial_font/*_DEC/v3_fontXX/
├───game_resident/*_DEC/v3_fontXX/


If any folder is missing, Monokuma will fail.

---

## 📝 OUTPUTS

Monokuma produces:

- `list_changed.txt` — SPCs modified by Monodam  
- `Distribute-Font/` — UPS patches (Monosuke)  
- `gr_font.7z` — final packaged font00  
- `report.txt` — optional combined logs  
- `vgit_failed.txt` — cloning error  
- `ropeway_failed.txt` — TextTools clone error (if running inside Daily)  

---

## 🧨 TROUBLESHOOTING

### “ERROR: 7za could not be found!”
Place `7za.exe` or `7za` inside `DGRV3-Font/base_spc`.

### “SPC does not exist”
Your base_fonts folder is corrupted or incomplete.

### “HTFont fails to pack”
Ensure `assimp.dll` / `libassimp.so` is present.

### “Monokid filtered out everything”
Monodam did not modify any SPCs.

### “gr_font.7z missing”
Monokuma could not locate `v3_font00.stx` or `v3_font00.srdv`.

---

## 🐻 FINAL NOTES

Monokuma is the **brain** of FontTools.  
It does not care about convenience.  
It does not care about safety.  
It cares about **correctness** and **reproducibility**.

If you want something simple, use Harmony-Tools.  
If you want the full industrial-strength font pipeline, Monokuma is your overseer.