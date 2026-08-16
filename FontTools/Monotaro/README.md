# FONTTOOLS - MONOTARO
## Team DAIX, 2026  
---

This README is dedicated exclusively to **Monotaro**, the first stage of the **FontTools** pipeline inside **DGRV3‑DAIXTOOLS**.

Monotaro is the FontTools equivalent of **Ropeway** (from TextTools):  
it manages **cloning**, **updating**, and **preparing** the **DGRV3‑Font** repository before any font compilation occurs.

If you cannot read the code, this repository is not for you.  
If you *can* read the code, everything you need is already there.

**Generative AI** was used in the creation of this project’s code, comments, documentation, and miscellaneous supporting files.  
Our translation was not generated or supported by AI.

---

## ⚠️ IMPORTANT WARNING  
Monotaro performs **destructive Git operations**.  
It clones repositories, deletes folders, merges branches, and rewrites working directories.

If you do not understand Git, do not use Monotaro.

If you want a safe, simple tool, use **Harmony‑Tools** instead.

FontTools is for:

- people editing **every font** in the game,  
- people who need **automated STX/SRDV generation**,  
- people who need **SPC rebuilds**,  
- people who want **full automation**, **full reproducibility**, and **full control**,  
- and, frankly, **masochists**.

---

## WHAT MONOTARO DOES  
Monotaro is responsible for preparing the **DGRV3‑Font** repository before any compilation happens.

It performs:

### 1. **Authentication**
Constructs authenticated GitHub URLs using:
- username  
- token  
- password (deprecated, but still supported in code)

### 2. **Cloning the main branch**
Equivalent to:

git clone --recursive https://TOKEN@github.com/USER/DGRV3-Font


### 3. **Enumerating all remote branches**
Monotaro runs:

git branch -r > branches.txt
Then parses the file to determine which branches exist.

### 4. **Filtering out unwanted branches**
Branches listed in `taken_branches` (e.g., `main`, `HEAD`) are ignored.

### 5. **Cloning or merging beta branches**
If `--beta` is enabled (default):

- Monotaro attempts to clone **all non‑blacklisted branches**  
- If merging is enabled, it merges them sequentially  
- If any merge produces a **CONFLICT**, Monotaro:
  - logs the conflict into `conflicts.txt`
  - deletes the repo
  - reclones only the first conflict‑free beta branch

### 6. **Writing vgit_failed.txt**
If cloning or merging fails, Monotaro writes:

vgit_failed.txt


This file signals Monokuma (the FontTools orchestrator) to abort early.

---

## WHY MONOTARO EXISTS  
FontTools requires a **very specific repository layout**:

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

Monotaro ensures:

- the repo is **clean**  
- the repo is **up‑to‑date**  
- the repo contains **all branches** needed for multi‑variant font compilation  
- the repo is **prepared** for Monophanie and Monodam

Without Monotaro, FontTools cannot run.

---

## PIPELINE POSITION  
Monotaro is **stage 1** of the FontTools pipeline:

0. **Monokuma** — orchestrator  
1. **Monotaro** — clone/update repo (THIS TOOL)  
2. **Monophanie** — prepare font folders  
3. **Monodam** — compile fonts
4. **Monokid** — filter SPCs  
5. **Monosuke** — generate UPS patches  

---

## HOW MONOTARO WORKS INTERNALLY  
### Git operations are performed through VGit / VGitUtils  
These are custom wrappers around shell commands:

- `git clone`
- `git branch -r`
- `git merge`
- `git add .`
- `git commit -m`
- `git push`
- `git tag`

Monotaro does **not** use libgit2 or any native Git API.  
It is intentionally simple and intentionally dangerous.

### Branch merging logic  
For each beta branch:

1. Attempt merge  
2. Write merge output to `merge_<branch>`  
3. Scan for `"CONFLICT"`  
4. If found:
   - log conflict  
   - delete repo  
   - clone only the first beta branch  
   - stop merging

This ensures the final repo is conflict‑free.

---

## REQUIRED FILES  
Monotaro itself does not require tool archives, but later stages do.

You must provide:

- `HTFont.7z`  
- `SPCTool.7z`  
- `ups.exe`  
- `assimp.dll` / `libassimp.so`  

These must be placed inside:

DGRV3-Font/base_spc/


Monotaro does not validate these files; Monophanie and Monodam do.

---

## OUTPUTS  
Monotaro produces:

- `DGRV3-Font/` — fully cloned and merged repository  
- `branches.txt` — temporary branch list  
- `conflicts.txt` — merge conflict log  
- `vgit_failed.txt` — fatal error indicator  

If `vgit_failed.txt` exists, Monokuma will abort.

---

## TROUBLESHOOTING  
### “vgit_failed.txt appears”
A branch failed to clone or merge.  
Check `conflicts.txt` for details.

### “Repository cloned but empty”
Your token or username is incorrect.

### “Merge conflicts everywhere”
Your beta branches are incompatible.  
Remove them or update your blacklist.

### “Monodam fails later”
Your repo layout is incorrect.  
Ensure Monotaro cloned the correct branches.

---

## FINAL NOTES  
Monotaro is not a user‑friendly tool.  
It is a **pipeline component**, not a standalone application.

If you want to modify fonts manually, use Harmony‑Tools.  
If you want **full automation**, **full reproducibility**, and **industrial‑strength font compilation**, Monotaro is the first step.

Read the code.  
Everything is explained there.
