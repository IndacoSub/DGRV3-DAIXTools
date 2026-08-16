# IMAGETOOLS - INVENTOR
## Team DAIX, 2026  
---

This document is the **official, detailed README** for **Inventor**, the second stage of the **ImageTools** pipeline inside **DGRV3‑DAIXTOOLS**.

Inventor is the ImageTools equivalent of **CrammedPiranhas** from TextTools:  
it extracts archives, prepares base assets, and sets up the working directory used by all downstream image‑processing tools.

If you cannot read the code, this repository is not for you.

Generative AI was used in the creation of this project’s code, comments, documentation, and miscellaneous supporting files.  
Our translation was not generated or supported by AI.

---

## ⚠️ IMPORTANT WARNING  
Inventor is **not** a casual modding tool.  
It is designed for **full‑automation**, **full‑reproducibility**, and **industrial‑scale** graphics patch generation.

If you only want to edit a few textures manually, use **Harmony‑Tools** or **UAFGJ** directly.  
Inventor is for:

- people generating **hundreds of SPC/AB patches**,  
- people who need **automated extraction**,  
- people who need **consistent folder layouts**,  
- people who want **zero manual steps**,  
- and people who enjoy suffering.

---

# What Inventor Actually Does

Inventor prepares the graphics repository for the rest of ImageTools:

### ✔ 1. Reads `ImageConfig.config`
Determines platform mode:

- **PC (Steam)** → uses `DGRV3-GFX` + `base_spc`
- **Xbox** → uses `DGRV3-GFX` + `base_spc`
- **Switch (Unity)** → uses `DGRV3-AB-GFX` + `base_ab`

This config also controls:

- **FileOnDemand** (GitHub ZIP download vs. Git clone)
- **Switch/Xbox/PC** platform selection

---

### ✔ 2. Extracts `main.zip` (if FileOnDemand=false)
If Adventurer downloaded the repo as a ZIP:

- Extracts `main.zip`
- Renames the extracted folder (`<repo>-main`) to the correct repo name
- Moves it into the correct base folder (`base_spc` or `base_ab`)

This ensures the folder layout matches what the rest of ImageTools expects.

---

### ✔ 3. Extracts platform‑specific tools

Depending on platform:

#### **PC / Xbox**
Inventor extracts:

- `SPCTool.7z`  
- `SRDTool.7z`

These contain:

- SPC unpackers  
- SRD/STX converters  
- Image patching utilities  

#### **Switch**
Inventor extracts:

- `UAFGJ.7z` — used for AB patching

All extraction is done via **7za.exe** (Windows) or system `7za` (Linux).

---

### ✔ 4. Prepares working folder: `danganronpa_files_copy`

If FileOnDemand=false:

- Copies the extracted graphics repository into  
  `base_spc/danganronpa_files_copy`  
  or  
  `base_ab/danganronpa_files_copy`

This folder is where **Maid**, **TennisPro**, and **Detective** operate.

The original base assets remain untouched.

---

### ✔ 5. Ensures correct permissions (Linux)
Runs:

`chmod -R +x *`

so extracted tools are executable.

---

# Expected Repository Layout

After Adventurer + Inventor run, the graphics repo should look like:

(PC/Xbox)

DGRV3-GFX/
├───base_spc/
├──────SPCTool.7z
├──────SRDTool.7z
├──────7za.exe
├──────danganronpa_files_copy/
├───boot/
├───flash/
├───MANUAL/
├───minigame/
├───noimages/

(Switch)

DGRV3-AB-GFX/
├───base_ab/
├──────UAFGJ.7z
├──────7za.exe
├──────danganronpa_files_copy/
├───Data


---

# Common Outputs

Inventor produces:

- Extracted tools (`SPCTool`, `SRDTool`, `UAFGJ`)
- Extracted graphics repository (`DGRV3-GFX` or `DGRV3-AB-GFX`)
- Working folder: `danganronpa_files_copy`
- Renamed ZIP extraction folder (if FileOnDemand=false)

---

# Troubleshooting

### “ERROR: The folder where the files to extract are located was not found!”
You ran Inventor without running **Adventurer** first.

### “7za.exe could not be found!”
Your graphics repo is incomplete or corrupted.

### “ERROR renaming <repo>-main”
GitHub ZIP extraction failed or the ZIP was not downloaded.

### “SPCTool extraction failed”
Your `SPCTool.7z` is missing or damaged.

### “UAFGJ extraction failed”
Switch configuration selected but UAFGJ archive missing.

---

# Summary

Inventor is the **extraction and preparation stage** of ImageTools.  
It:

- extracts archives  
- prepares base folders  
- sets up working directories  
- ensures platform‑correct tools are available  
- prepares everything for Maid → TennisPro → Detective  

It is not optional.  
It is not safe.  
It is not friendly.  
It is essential.