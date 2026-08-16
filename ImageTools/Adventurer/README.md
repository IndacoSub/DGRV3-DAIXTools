# IMAGETOOLS — ADVENTURER
## Team DAIX, 2026  
---

This README is the dedicated documentation for **Adventurer**, the graphics‑repository manager used inside **FontTools/ImageTools** pipelines of **DAIXTOOLS**.

Adventurer is the **graphics equivalent of Ropeway** (from TextTools).  
It clones, prepares, and normalizes the **DGRV3‑GFX** or **DGRV3‑AB‑GFX** repositories depending on platform, ensuring that ImageTools has a clean, reproducible, multi‑branch working directory before any image patching begins.

If you cannot read the code, this repository is not for you.

Generative AI was used in the creation of this project’s code, comments, documentation, and miscellaneous supporting files.  
Our translation was not generated or supported by AI.

---

## ⚠️ IMPORTANT WARNING  
Adventurer is **not** a casual‑modding tool.  
It is designed for:

- people generating **full graphics pipelines**,  
- people who need **multi‑branch merging**,  
- people who want **automated SPC/AB image workflows**,  
- people who need **reproducible daily builds**,  
- and people who understand Git, DAIXTOOLS, and the DGRV3 asset structure.

If you only want to edit a few textures manually, use **Harmony‑Tools** instead.

Adventurer is the industrial‑strength solution.

---

## WHAT ADVENTURER DOES  
Adventurer performs the following tasks:

### 1. **Reads ImageConfig.config**
This determines:

- PC / Xbox / Switch mode  
- Whether FileOnDemand is enabled  
- Which graphics repo to clone  
- Which base folder to use (`base_spc` vs `base_ab`)

### 2. **Selects the correct graphics repository**
Depending on configuration:

| Platform | Repository | Base Folder |
|---------|------------|-------------|
| PC      | `DGRV3-GFX`     | `base_spc` |
| Xbox    | `DGRV3-GFX`     | `base_spc` |
| Switch  | `DGRV3-AB-GFX`  | `base_ab`  |

### 3. **Constructs authenticated GitHub URLs**
Uses credentials stored in `Cloud.h`:

- Username  
- Token  
- Password (deprecated, token used instead)

### 4. **Clones the main branch**
Equivalent to:

git clone --recursive https://TOKEN@github.com/OWNER/DGRV3-GFX


### 5. **Optionally clones & merges beta branches**
If `--beta` is passed or config enables it:

- Lists all remote branches  
- Filters out blacklisted ones (`main`, `HEAD`, etc.)  
- Merges remaining branches into the working tree  
- Writes `vgit_failed.txt` if conflicts occur  

This is identical to Ropeway’s beta‑branch logic.

### 6. **Downloads ZIP archives (if FileOnDemand disabled)**
If FileOnDemand is **false**, Adventurer bypasses Git and downloads:

https://github.com/OWNER/REPO/archive/refs/heads/main.zip


This is used for platforms where Git LFS or large file downloads are unreliable.

---

## EXPECTED REPOSITORY LAYOUT  
After cloning, Adventurer expects:

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


Adventurer does **not** modify these files.  
It only prepares the repo for ImageTools.

---

## CONFIGURATION DETAILS  
Adventurer reads `ImageConfig.config` using the DAIXTOOLS configuration parser.

Important keys:

| Key | Meaning |
|-----|---------|
| `UseSwitchConfiguration` | Switch mode → clone AB‑GFX |
| `UseXboxConfiguration` | Xbox mode → GFX repo, Xbox base files |
| `FileOnDemand` | If false, download ZIP instead of using Git |
| `dl_repo_name_*` | Repo names for each platform |
| `dl_repo_owner` | GitHub owner |
| `dl_repo_token` | GitHub token |

---

## BRANCH MERGING LOGIC  
Adventurer uses the same beta‑branch merging system as Ropeway:

1. `git branch -r` → write to `branches.txt`  
2. Remove blacklisted branches  
3. Merge remaining branches one by one  
4. Detect conflicts by scanning merge output for `"CONFLICT"`  
5. If conflict → delete repo → clone only first beta branch  
6. Commit each successful merge  

This ensures a **single unified working tree** containing all beta changes.

---

## COMMON OUTPUTS  
Adventurer itself produces:

- `vgit_failed.txt` — cloning/merging error  
- `branches.txt` — temporary branch list  
- A fully prepared graphics repository ready for ImageTools  

It does **not** produce patches or modified files.  
Those are generated later by Pianist and Monosuke.

---

## TROUBLESHOOTING

### “ERROR: UPS patcher not found!”
Your graphics repo is missing `ups.exe`.  
Check `base_spc` or `base_ab`.

### “Repository couldn’t be cloned successfully!”
Your token is invalid or the repo is private.  
Check `Cloud.h`.

### “Switch assets missing”
Ensure `UseSwitchConfiguration=true` and that the repo contains `base_ab`.

### “main.zip download failed”
GitHub throttled your token.  
Try again or enable FileOnDemand.

---

## SUMMARY  
Adventurer is the **graphics bootstrapper** of DAIXTOOLS.  
It clones, merges, downloads, and prepares the graphics repository so ImageTools can run cleanly and reproducibly.

If you want the easy path, use Harmony‑Tools.  
If you want the complete automated graphics pipeline, Adventurer is your entry point.