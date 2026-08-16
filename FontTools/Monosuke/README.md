# FONTTOOLS — MONOSUKE  
## Team DAIX, 2026  
---

Monosuke is the **final stage** of the FontTools pipeline.  
If Monotaro clones the repo, Monophanie prepares the fonts, Monodam compiles them, and Monokid filters them…  
**Monosuke is the one who actually produces the distributable UPS patches.**

This README explains **what Monosuke does**, **why it exists**, **how it works**, and **what its output means**.

If you cannot read the code, this repository is not for you.

---

## ⚠️ IMPORTANT WARNING  
Monosuke **does not** distribute SPC files.  
It only distributes **UPS patches**, which contain *differences* between the original SPC and the modified SPC.

This is intentional:

- SPC archives contain copyrighted assets.  
- UPS patches contain only binary diffs.  
- UPS patches are safe to distribute publicly.

Monosuke ensures **no copyrighted SPC data ever leaves your machine**.

---

## What Monosuke Does (High-Level)

Monosuke takes the output of previous FontTools stages:

- **base_fonts_copy/** → untouched SPCs  
- **ModifiedFiles-Font/** → modified SPCs (produced by Monodam → filtered by Monokid)

Then it:

1. **Copies base_fonts_copy into Distribute-Font/**
2. **Renames base SPCs to `_normal.spc`**
3. **Copies modified SPCs into Distribute-Font/**
4. **Runs UPS patcher (`ups.exe` / `ups`)**  
   to generate `*_patch.ups` files
5. **Deletes everything except `.ups` files**

The result is a clean folder containing **only UPS patches**, ready for distribution.

---

## Pipeline Context

Monosuke is stage **5** of the canonical FontTools pipeline:

1. **Monotaro** — clone/update DGRV3-Font  
2. **Monophanie** — extract SPCTool/HTFont, prepare base fonts  
3. **Monodam** — compile fonts → STX/SRDV → SPC insertion  
4. **Monokid** — filter SPCs, keep only modified ones  
5. **Monosuke** — generate UPS patches (this tool)

Monosuke assumes all previous stages have completed successfully.

---

## Detailed Behavior

### 1. Validate Required Paths

Monosuke checks for:

- `ModifiedFiles-Font/`  
- `base_spc/`  
- `base_fonts_copy/`  
- `ups.exe` / `ups`

If any are missing, Monosuke aborts.

---

### 2. Copy Base SPCs

Monosuke copies:

DGRV3-Font/base_spc/base_fonts_copy/

into:

Distribute-Font/

These SPCs represent the **original** (unmodified) versions.

---

### 3. Rename Base SPCs → *_normal.spc

UPS patcher requires a strict naming pattern:

base:     something_normal.spc
modified: something.spc
output:   something_patch.ups


Monosuke renames every base SPC accordingly.

Example:

game_font01_4.spc → game_font01_4_normal.spc

---

### 4. Copy Modified SPCs

Monosuke copies all modified SPCs from:

ModifiedFiles-Font/

into:

Distribute-Font/

These SPCs are the ones produced by Monodam.

---

### 5. Generate UPS Patches

For each modified SPC:

- Identify its corresponding `_normal.spc`
- Run UPS patcher:

ups diff --base base_normal.spc --modified modified.spc --output modified_patch.ups


---

### 6. Delete Everything Except UPS Files

After patch generation, Monosuke removes:

- All `.spc` files  
- All temporary files  
- All non-UPS artifacts  

Leaving only:

Distribute-Font/
game_font01_1_patch.ups
game_font01_2_patch.ups
game_font01_3_patch.ups
...

This folder is safe to upload or distribute.

---

## Output Structure

After running Monosuke, you will have:

Distribute-Font/
game_font00_patch.ups
game_font01_1_patch.ups
game_font01_2_patch.ups
...

This is the **final output** of FontTools.

---

## Why UPS?

UPS is a binary diff format originally used for ROM hacking.  
It is ideal for distributing modifications without shipping copyrighted data.

Advantages:

- Only differences are stored  
- No copyrighted SPC data included  
- Small file sizes  
- Easy to apply using standard UPS patchers  
- Fully reversible (UPS patches can be undone)

---

## Troubleshooting

### “UPS patcher not found”
Ensure `ups.exe` or `ups` is inside:

DGRV3-Font/base_spc/


### “No patches generated”
Check:

- Monodam actually modified SPCs  
- Monokid filtered SPCs correctly  
- ModifiedFiles-Font is not empty

### “normal SPC missing”
Monosuke requires:

something_normal.spc


If base_fonts_copy was modified or corrupted, regenerate it via Monophanie.

---

## Notes for Developers

- Monosuke does not validate SPC contents; it only pairs files by name.  
- UPS generation is sequential and may take time for large SPCs.  
- Monosuke does not support multithreading.  
- Monosuke assumes folder structure created by Monotaro + Monophanie + Monodam + Monokid.

---

## Summary

Monosuke is the final packaging stage of FontTools.  
It ensures that:

- Only modified SPCs produce patches  
- Only UPS patches are distributed  
- No copyrighted SPC data leaks  
- The output is clean, reproducible, and safe

If you want the easy path, use Harmony-Tools.  
If you want the industrial-strength automated pipeline, Monosuke is part of it.