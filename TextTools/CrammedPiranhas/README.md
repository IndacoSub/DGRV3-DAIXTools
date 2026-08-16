# TEXTTOOLS - CRAMMEDPIRANHAS
## Team DAIX, 2026  

---

## 📌 Overview

**CrammedPiranhas** is the extraction engine of the TextTools pipeline.  
Its job is to unpack all SPC‑related archives, STX tools, fonts, and platform‑specific assets required for text compilation.  

It is always run early in the HiddenDoor pipeline, immediately after Ropeway clones the repository.

CrammedPiranhas operates in **two modes**:

1. **Normal Mode (Part 1)** — Extract everything needed for text compilation  
2. **Part2 Mode** — Extract only base SPC archives for distribution/UPS patching

It relies entirely on **7za** for archive extraction.

---

## 🎯 Responsibilities

CrammedPiranhas performs the following tasks:

### ✔ 1. Read configuration  
Loads `TextConfig.config` to determine:
- Platform (PC / Xbox / Switch)
- Which SPC archives to extract
- Whether to use legacy or new STXTool

### ✔ 2. Validate repository structure  
Ensures the following exist:

`DGRV3/`
`DGRV3/base_spc/`

If missing, extraction cannot proceed.

### ✔ 3. Extract SPC archives  
Depending on platform:

| Platform | Archive |
|----------|---------|
| **PC** | `danganronpa_spc_legacy.7z` |
| **Xbox** | `danganronpa_spc_xbox.7z` |
| **Switch** | `danganronpa_spc_switch.7z` |

These archives contain the raw SPC files used for text injection.

### ✔ 4. Extract STX tools  
CrammedPiranhas extracts either:

- `NewSTXTool.7z` / `NewSTXTool_Linux.7z`  
or  
- `STXTool.7z` / `STXTool_Linux.7z`  

depending on the `UseNewSTXTool` flag.

These tools are required by **Electrohammer** for STX manipulation.

### ✔ 5. Extract SPCTool  
SPCTool is required for unpacking and repacking SPC files:

`SPCTool.7z`
`SPCTool_Linux.7z`

### ✔ 6. Extract Switch‑specific assets  
If Switch mode is enabled:

- `i18n_switch.7z`  
- `danganronpa_spc_switch.7z`

These contain Unity‑based SPC replacements and localization files.

### ✔ 7. Extract font archives  
CrammedPiranhas extracts:

`gr_font.7z`


This archive contains STX/SRDV font assets used by the game.

### ✔ 8. Part2 Mode (Distribution Mode)  
When launched with `part2`, CrammedPiranhas:

- Creates a `Distribute/` folder  
- Extracts only base SPC archives into it  
- Extracts French and English trial SPCs for better minigame text layout  
- Skips STXTool/SPCTool extraction entirely

This mode is used by **WhiteSheet** and **Crossbow** for UPS patch generation.

---

## 🧩 How It Fits Into TextTools

CrammedPiranhas is stage **2** of the TextTools pipeline:

1. **Ropeway** — clone repo  
2. **CrammedPiranhas** — extract SPC/STX tools  
3. **MarkerStone** — line counting  
4. **Necronomicon** — hash calculation  
5. **Bugvac** — name replacement  
6. **HydraulicPress** — variable replacement  
7. **PoolRules** — percentage calculation  
8. **Electrohammer** — text injection  
9. **Crossbow** — distribution  
10. **WhiteSheet** — UPS patch generation

Without CrammedPiranhas, none of the SPC/STX tools or base archives would exist.

---

## ⚙️ Internal Behavior

### 1. Detect `part2` mode

```cpp
if (StringContains(arg, "part2")) part2 = true;
```

### 2. Extract platform‑specific SPC archives

`danganronpa_spc_legacy.7z`
`danganronpa_spc_xbox.7z`
`danganronpa_spc_switch.7z`

### 3. Extract STXTool or DAIX's version of DRV3-STX-TOOL ("NewSTXTool")

`NewSTXTool.7z` or `STXTool.7z`

### 4. Extract SPCTool

`SPCTool.7z`

### 5. Extract fonts

`gr_font.7z`

### 6. Extract Switch localization

`i18n_switch.7z`

### 7. Extract trial SPCs (part2)

`trial_french.7z`
`trial_english.7z`

### 8. Use 7za for all extraction

`"7za x <file> -o<folder> -aoa"`

## 📁 Folder Structure After Extraction

### Normal Mode:

DGRV3/
    base_spc/
        flash/
        adv/
        trial/
        SPCTool.exe
        NewSTXTool.exe
        gr_font/

### Part2 Mode:

Distribute/
    flash/
    adv/
    trial/
trial_french_extracted/
trial_english_extracted/

## 🧪 Example Usage

### Normal extraction:

`CrammedPiranhas.exe`

### Distribution extraction:

`CrammedPiranhas.exe part2`

### HiddenDoor invocation:

HiddenDoor automatically runs:

`CrammedPiranhas >> report.txt`
`CrammedPiranhas part2 >> report.txt`

## 🚦 Failure Handling

CrammedPiranhas stops execution if:

- DGRV3/ is missing

- base_spc/ is missing

- 7za cannot be found

- Any archive fails to extract

Errors are logged with:

`"ERROR: Extraction failed for <file>"`

## 🏁 Summary

CrammedPiranhas is the extraction backbone of TextTools:

- Unpacks SPC archives

- Extracts STX/SPC tools

- Handles platform differences

- Supports a second‑stage distribution mode

- Provides all raw assets needed for text injection

Without CrammedPiranhas, TextTools would have no SPCs, no STX tools, and no fonts — making text compilation impossible.