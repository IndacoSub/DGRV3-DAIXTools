# FONTTOOLS — MONOPHANIE  
## Team DAIX, 2026  

---

Monophanie is the **second stage** of the FontTools pipeline inside **DGRV3‑DAIXTOOLS**.  
It runs immediately after **Monotaro** (the repository manager) and before **Monodam** (the font compiler).

If Monotaro is the *Git orchestrator*,  
Monophanie is the *bootstrapper* — the component that **prepares every font asset** FontTools will later compile.

Monophanie is responsible for extracting archives, duplicating font folders, renaming internal assets, and ensuring the entire font directory structure is in the exact shape required by the rest of the pipeline.

If Monophanie fails, **FontTools cannot run**.

---

## ⚠️ IMPORTANT WARNING  
Monophanie is **not** a casual modding tool.  
It is part of the industrial‑strength DAIXTOOLS pipeline and assumes:

- You have a correctly cloned **DGRV3-Font** repository  
- You understand SPC/STX/SRDV font formats  
- You know how Harmony‑Tools (old version) behaves  
- You are comfortable with destructive operations (folder overwrites, recursive copies)

If you want simple font editing, use **Harmony‑Tools** instead.

Monophanie is for full automation, reproducibility, and batch processing.

---

## 🧩 ROLE IN THE FONTTOOLS PIPELINE

FontTools consists of six components:

0. **Monokuma** — orchestrator  
1. **Monotaro** — clone/update repo 
2. **Monophanie** — prepare font folders (THIS TOOL)  
3. **Monodam** — compile fonts
4. **Monokid** — filter SPCs  
5. **Monosuke** — generate UPS patches  

Monophanie prepares the entire workspace so Monodam can compile fonts without errors.

---

## 📦 WHAT MONOPHANIE DOES

Monophanie performs **four major tasks**:

### 1. Extract SPCTool & HTFont archives  
Inside `DGRV3-Font/base_spc/`, the following archives must exist:

- `SPCTool.7z` / `SPCTool_Linux.7z`  
- `HTFont.7z` / `HTFont_Linux.7z`  
- `7za.exe` (Windows) or system `7za` (Linux)

Monophanie extracts these archives using:

7za x SPCTool.7z -o<base_spc> -aoa
7za x HTFont.7z -o<base_spc> -aoa


This produces:

- `SpcTool.exe` / `SpcTool`  
- `HTFont.exe` / `HTFont`  
- `assimp.dll` / `libassimp.so`  
- Base font templates  
- Internal STX/SRDV assets

These files are required by Monodam.

---

### 2. Copy base font files into the working directory

Monophanie copies **all extracted files** from `base_spc/` into the root of `DGRV3-Font/`.

This includes:

- STX/SRDV templates  
- SPC archives  
- HTFont dependencies  
- SpcTool executables  

It also duplicates:

base_fonts → base_fonts_copy

`base_fonts_copy` is the folder Monodam modifies.  
`base_fonts` must remain untouched.

---

### 3. Prepare trial fonts (game_font01_* variants)

The game uses multiple font variants:

`game_font01_1_US_DEC`
`game_font01_2_US_DEC`
`game_font01_3_US_DEC`
`game_font01_4_US_DEC`
`game_font01_5_US_DEC`
`game_font01_6_US_DEC`
`game_font01_9_US_DEC`


Monophanie generates these automatically by cloning:

trial_font/game_font01_8_US_DEC


Then it renames:

- `v3_font01_8/` → `v3_font01_<N>/`
- `v3_font01_8.stx` → `v3_font01_<N>.stx`
- `v3_font01_8.srdv` → `v3_font01_<N>.srdv`

This ensures each trial font folder has the correct naming scheme for Monodam.

---

### 4. Normalize STX/SRDV naming conventions

The game expects STX/SRDV files to follow strict naming rules:

`v3_font01_<N>.stx`
`v3_font01_<N>.srdv`


Monophanie enforces this by renaming extracted files and trial font variants.

If naming is wrong, Monodam will fail to insert fonts into SPC archives.

---

## 🛠️ WHY MONOPHANIE EXISTS

You might wonder:

> “Why not just run HTFont directly?”

Because HTFont (old Harmony‑Tools) expects a **very specific folder layout**:

- Correct STX/SRDV names  
- Correct folder names  
- Correct SPC locations  
- Correct dependency placement  
- Correct trial font duplication  

Monophanie ensures all of this is correct.

Without Monophanie, FontTools would require hours of manual setup.

---

## 📁 EXPECTED DGRV3-Font LAYOUT (after Monophanie)

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


This is the exact structure Monodam expects.

---

## 🧪 TROUBLESHOOTING

### “7za not found”
Place `7za.exe` inside `base_spc/` (Windows)  
or install `p7zip-full` (Linux).

### “SPCTool.7z missing”
You must provide SPCTool archives yourself.  
DAIXTOOLS does **not** ship them.

### “HTFont fails to extract”
Your HTFont archive is corrupted or incomplete.  
Ensure it contains:

- HTFont.exe  
- assimp.dll  
- STX/SRDV templates

### “Trial fonts not generated”
Check that:

trial_font/game_font01_8_US_DEC/

exists and contains:

- `v3_font01_8/`  
- `v3_font01_8.stx`  
- `v3_font01_8.srdv`

---

## 🧷 NOTES ABOUT HTFONT

HTFont is part of an **older Harmony‑Tools** version.  
Modern Harmony‑Tools is unified, but FontTools still uses HTFont because:

- It produces stable STX/SRDV output  
- It matches DAIXTOOLS expectations  
- It avoids breaking existing SPC workflows  

DAIXTOOLS will **not** provide HTFont archives.

---

## 🏁 SUMMARY

Monophanie is the FontTools bootstrapper.  
It extracts archives, prepares folders, duplicates trial fonts, and renames assets so Monodam can compile fonts without errors.

If Monophanie succeeds, FontTools can run.  
If Monophanie fails, nothing else will work.

---

## 📜 Final Note

Monophanie is designed for **automation**, not convenience.  
If you want simple font editing, use Harmony‑Tools.  
If you want full automation, reproducibility, and industrial‑strength font compilation, Monophanie is your friend.

