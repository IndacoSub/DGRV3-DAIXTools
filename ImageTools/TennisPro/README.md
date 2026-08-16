# IMAGETOOLS - TENNISPRO
## Team DAIX, 2026  

---

## 📌 Overview

**TennisPro** is the final stage of the **ImageTools** pipeline.  
After **Adventurer** clones the graphics repo, **Inventor** extracts tools and base assets, and **Maid** compiles all modified PNG/TGA files into SPC/AB containers, **TennisPro** steps in to:

- Clean the compiled asset tree  
- Remove empty or invalid files  
- Strip leftover metadata (Git folders, README files, etc.)  
- Copy the final, fully‑compiled SPC/AB files into a clean distribution folder  
- Produce **ModifiedFiles‑GFX**, the folder intended for packaging or mod release

TennisPro ensures that only valid, modified assets are shipped — nothing stale, nothing empty, nothing accidentally left behind.

---

## 🧩 Role in the ImageTools Pipeline

TennisPro is the **fourth stage** of ImageTools:

1. **Adventurer** — Clone graphics repo  
2. **Inventor** — Extract tools & base assets  
3. **Maid** — Compile PNG/TGA → SRD/SPC or AB/assets  
4. **TennisPro** — Build final distribution folder  
5. *(Optional)* **Detective** — Generate UPS patches (if DISTRIBUTE enabled)

TennisPro is the graphics equivalent of **Monokid** from FontTools.

---

## 📁 Input & Output

### **Input folders**
- `DGRV3-GFX` or `DGRV3-AB-GFX`  
- `base_spc` or `base_ab`  
- `danganronpa_files_copy` (Inventor’s working folder)  
- Compiled SPC/AB files inside:
  - `base_spc/<repo_name>/`
  - `base_spc/danganronpa_files_copy/`

### **Output folder**
- **ModifiedFiles-GFX/**  
  Contains ONLY:
  - Modified SPC files (PC/Xbox)
  - Modified AB/sharedassets files (Switch)
  - Cleaned, ready‑to‑ship assets

---

## ⚙️ What TennisPro Does

### 1. **Load configuration**
Reads `ImageConfig.config` to determine:
- Platform (PC / Xbox / Switch)
- Repo name (`DGRV3-GFX` or `DGRV3-AB-GFX`)
- Base folder (`base_spc` or `base_ab`)
- Cloud repo name (for FileOnDemand)

---

### 2. **Validate required folders**
Ensures the following exist:
- Graphics repo  
- Base folder  
- Compiled asset folders  

If anything is missing, TennisPro aborts early.

---

### 3. **Clean empty files**
TennisPro scans:

- `base_spc/<repo_name>/`
- `base_spc/danganronpa_files_copy/`

and deletes **empty files**, which typically come from:
- Failed downloads  
- Failed compilations  
- Corrupted SPC/AB files  

This prevents shipping broken assets.

---

### 4. **Copy compiled assets**
Copies all compiled SPC/AB files from:

base_spc/<repo_name>/

into:

ModifiedFiles-GFX/


This is the core output step.

---

### 5. **Remove leftover metadata**
Deletes:

- `.git/`
- `README.md`
- Any leftover GitHub artifacts

This ensures the distribution folder contains **only game assets**, nothing from the repo itself.

---

### 6. **Final confirmation**
Logs:

All files copied in the distribution folder!


and waits for user exit.

---

## 🛠️ Platform‑Specific Notes

### PC / Xbox
- Uses `base_spc/`
- Removes empty `.spc` files
- Uses `SpcTool.dll` timestamp as a reference for modification detection

### Nintendo Switch
- Uses `base_ab/`
- Removes empty `.ab` / `.assets` files
- Uses `classdata.tpk` timestamp instead of SpcTool.dll

---

## 🔍 Internal Logic Summary

### Folder selection
```cpp
if (UseSwitchConfiguration)
    repo = "DGRV3-AB-GFX";
else
    repo = "DGRV3-GFX";
```

### Base folder selection

```cpp
basefolder = UseSwitchConfiguration ? "base_ab" : "base_spc";
```

### Distribution folder

ModifiedFiles-GFX/

### Cleaning empty files

```cpp
if (!file.is_directory() && std::filesystem::is_empty(file))
    remove(file);
```

### Copying compiled assets

```cpp
std::filesystem::copy(nested_spc, dist,
    copy_options::recursive | copy_options::overwrite_existing);
```

### Removing metadata

```cpp
remove(dist / ".git");
remove(dist / "README.md");
```

## 📦 Final Output

After running TennisPro, you get:

ModifiedFiles-GFX/
│
├── flash/
├── adv/
├── trial/
├── Data/
│   ├── sharedassets0.assets
│   ├── sharedassets1.assets
│   └── ...
└── (all modified SPC/AB files)

This folder is ready for packaging, mod distribution, or UPS patch generation (via Detective).

## Summary

TennisPro is the final cleanup and distribution tool of ImageTools.
It ensures your modded graphics are:

- Clean

- Valid

- Free of empty/corrupted files

- Free of repo metadata

- Ready for release

Without TennisPro, your output would be cluttered with unmodified files, empty SPCs, and leftover Git metadata.