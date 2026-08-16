# IMAGETOOLS - DETECTIVE
## Team DAIX, 2026 
---

## 📌 Overview

**Detective** is the final stage of the **ImageTools** pipeline.  
Its job is simple but extremely important:

> **Detective generates UPS patches for all modified graphics assets**, ensuring that only *differences* are distributed — never full copyrighted SPC/AB files.

It compares the **base** graphics files (from the cloned DGRV3‑GFX / DGRV3‑AB‑GFX repository) with the **modified** ones produced by **Maid** and **TennisPro**, then produces clean, distributable `*_patch.ups` files.

Detective is the ImageTools equivalent of **Monosuke** from FontTools.

---

## 🧩 Responsibilities

Detective performs the following tasks:

### 1. **Load configuration**
Reads `ImageConfig.config` to determine:
- Platform (PC / Xbox / Switch)
- Base folder (`base_spc` or `base_ab`)
- Repository name (`DGRV3-GFX` or `DGRV3-AB-GFX`)

### 2. **Locate required folders**
Detective expects:
- `ModifiedFiles-GFX/` — produced by **TennisPro**
- `danganronpa_files_copy/` — base files extracted by **Inventor**
- `ups(.exe)` — UPS patcher inside the base folder

If any of these are missing, Detective aborts.

### 3. **Prepare distribution folder**
Creates:

Distribute-GFX/

Then copies the entire `danganronpa_files_copy/` folder into it.  
These files represent the **base** versions of SPC/AB assets.

### 4. **Rename base files → *_normal**
Every file inside `Distribute-GFX/` is renamed:

example.spc → example_normal.spc
example.ab  → example_normal.ab
example.assets → example_normal.assets

This naming scheme is required by the UPS patcher.

### 5. **Copy modified files**
Detective copies all modified files from:

ModifiedFiles-GFX/

into the correct subfolders inside:

Distribute-GFX/

Folder structure is preserved.

### 6. **Generate UPS patches**
For each modified file:

modified.spc
modified.ab
modified.assets


Detective pairs it with its base version:

modified_normal.spc
modified_normal.ab
modified_normal.assets

Then runs:

ups diff --base modified_normal.spc --modified modified.spc --output modified_patch.ups


This produces a clean UPS patch containing only the differences.

### 7. **Cleanup**
After patch generation:
- All non‑UPS files are deleted from `Distribute-GFX/`
- Only `*_patch.ups` files remain

This ensures the final output is safe for distribution.

---

## 📁 Folder Structure (Before & After)

### Input folders:

DGRV3-GFX/
base_spc/
danganronpa_files_copy/
ModifiedFiles-GFX/

### Output folder:

Distribute-GFX/
<mirrored folder structure>
*_normal.spc / *_normal.ab / *_normal.assets
*_patch.ups


---

## ⚙️ Platform Behavior

Detective supports both pipelines:

### **PC / Xbox**
- Works with `.spc` files
- Generates `*_patch.ups` for SPC archives

### **Nintendo Switch / Unity**
- Works with:
  - `.ab` (AssetBundles)
  - `.assets` (sharedassets files)
- Generates UPS patches for Unity assets

Detective automatically detects the platform via `ImageConfig.config`.

---

## 🔧 Dependencies

Detective requires:

- `ups(.exe)` — UPS patcher  
  Located inside:

DGRV3-GFX/base_spc/ups.exe
DGRV3-AB-GFX/base_ab/ups.exe

- `ModifiedFiles-GFX/` — produced by **TennisPro**
- `danganronpa_files_copy/` — produced by **Inventor**

If any are missing, Detective stops immediately.

---

## 🧠 Internal Logic Summary

1. Read config  
2. Determine platform  
3. Create `Distribute-GFX/`  
4. Copy base files  
5. Rename base files → `_normal`  
6. Copy modified files  
7. Generate UPS patches  
8. Delete everything except `.ups`  
9. Print “Patch files generated!”

---

## 📝 Example Output

After running Detective, you will see:

Distribute-GFX/
flash/adv/spcpack/example_patch.ups
flash/trial/spcpack/t_example_patch.ups
...


These UPS patches can be safely distributed without containing copyrighted SPC/AB data.

---

## 🐾 Notes

- Detective **never** modifies SPC/AB files directly — it only compares them.
- UPS patches are extremely small compared to SPC/AB archives.
- This stage ensures ImageTools remains legally safe for mod distribution.
- Detective is always run last by **Pianist**.

---

## 🏁 Conclusion

Detective is the final “forensic” step of ImageTools.  
It ensures that all modifications made by Maid and TennisPro are converted into clean, distributable UPS patches — preserving legality, safety, and reproducibility.