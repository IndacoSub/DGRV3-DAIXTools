# TEXTTOOLS - MARKERSTONE
## Team DAIX, 2026  

---

## 📌 Overview

**MarkerStone** is the TextTools component responsible for **line‑count validation** across the Danganronpa V3 script.  
Its job is to ensure that translated `.txt` files have **the exact same number of lines** as their English counterparts.  

Why does this matter?  
Because even a single extra or missing newline can break SPC insertion, shift dialogue blocks, corrupt trial scripts, or desynchronize voice/text timing.

MarkerStone is the “script auditor” of TextTools — it guarantees structural integrity before any text injection occurs.

---

## 🎯 Responsibilities

MarkerStone performs four major tasks:

### ✔ 1. Count lines in every English `.txt` file  
It recursively scans:

`DGRV3_EN/`

For each `.txt` file, it creates a companion file:

`<filename>.txt_lines`

containing only the number of newline characters.

### ✔ 2. Count lines in every translated `.txt` file  
It scans:

`DGRV3/`

and counts newlines in each translated file.

### ✔ 3. Compare English vs translated line counts  
For each file:

- If counts match → OK  
- If counts differ → flagged as a mismatch  

MarkerStone records mismatches in memory and logs them to the console.

### ✔ 4. Save a mismatch report  
All mismatched files are written to:

`different_lines.txt`

Each entry includes:

`<filename> (translated_count VS english_count)`


This file is used by later tools to determine which script folders require special handling.

---

## 🧩 How MarkerStone Fits Into TextTools

MarkerStone is stage **3** of the TextTools pipeline:

1. **Ropeway** — clone repo  
2. **CrammedPiranhas** — extract SPC/STX tools  
3. **MarkerStone** — line‑count validation  
4. **Necronomicon** — hash calculation  
5. **Bugvac** — name replacement  
6. **HydraulicPress** — variable replacement  
7. **PoolRules** — percentage calculation  
8. **Electrohammer** — text injection  
9. **Crossbow** — distribution  
10. **WhiteSheet** — UPS patch generation

MarkerStone ensures the script structure is safe before any modifications occur.

---

## 📁 Folder Coverage

MarkerStone categorizes mismatches by folder.  
These folders represent major script sections:

`test`
`subroutine`
`prologue`
`MapObjName`
`game_resident`
`gallery`
`epilogue`
`chapter6`
`chapter5`
`chapter4`
`chapter3`
`chapter2`
`chapter1`
`ainori`


When a mismatch is found, MarkerStone records the index of the folder.  
This helps later tools (especially Electrohammer) know which sections may require special handling.

---

## ⚙️ Internal Behavior

### 1. Validate repository structure
MarkerStone requires:

`DGRV3/`
`DGRV3_EN/`


If either is missing, it aborts.

---

### 2. Save English line counts
For each `.txt` file in `DGRV3_EN`:

- Read entire file into a `stringstream`
- Count `'\n'`
- Write count to `<file>_lines`

Example:

`scene_12.txt_lines → "143"`

---

### 3. Count translated lines
For each `.txt` file in `DGRV3`:

- Read file  
- Count `'\n'`  
- Compare with English count stored in `<file>_lines`

If mismatched:

`Different lines: chapter3/scene_12.txt (145 VS 143)`

---

### 4. Record mismatches
MarkerStone stores:

```
{
"<translated file path>",
{ "<translated line count>", "<english line count>" }
}
```

And also stores the folder index.

---

### 5. Save mismatch report
If mismatches exist:

`different_lines.txt`

is created with entries like:

`chapter3/scene_12.txt (145 VS 143)`
`chapter1/scene_05.txt (210 VS 209)`

---

## 🧪 Command‑Line Flags

### `--all`
Forces MarkerStone to treat **every folder** as mismatched, regardless of actual line counts.

Useful for debugging or forcing full recompilation.

Example:

`MarkerStone.exe --all`

---

## 🚦 Failure Handling

MarkerStone stops execution if:

- English and translated file counts differ  
- Any `.txt` file cannot be opened  
- Companion `_lines` files are missing  
- Repository folders are missing  

It also generates diagnostic files:

`mstone_check_en.txt`
`mstone_check_it.txt`

listing all processed files for debugging.

---

## 📁 Files Created by MarkerStone

| File | Purpose |
|------|---------|
| `<file>.txt_lines` | Stores English line count |
| `different_lines.txt` | List of mismatched files |
| `mstone_check_en.txt` | Diagnostic list of English files |
| `mstone_check_it.txt` | Diagnostic list of translated files |

---

## 🏁 Summary

MarkerStone is the **script integrity validator** of TextTools:

- Counts lines in English and translated scripts  
- Detects mismatches  
- Prevents broken SPC insertion  
- Produces detailed reports  
- Ensures structural consistency before text injection  

Without MarkerStone, TextTools would risk corrupting dialogue, breaking trial logic, and misaligning script blocks.