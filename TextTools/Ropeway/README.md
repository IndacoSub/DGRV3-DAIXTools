# TEXTTOOLS - ROPEWAY
## Team DAIX, 2026  
---

## 📌 Overview

**Ropeway** is the first stage of the **TextTools** pipeline.  
Its job is to **clone, prepare, and normalize the DGRV3 text repository**, ensuring that all later tools (CrammedPiranhas, MarkerStone, Necronomicon, Electrohammer, etc.) operate on a clean, predictable, multi‑branch workspace.

Ropeway is built on top of **VGit** and **VGitUtils**, Team DAIX’s custom Git wrapper library.  
It performs multi‑branch cloning, merging, filtering, and folder preparation — all through shell‑based Git commands.

If HiddenDoor is the conductor of TextTools, Ropeway is the *railway system* that delivers the raw text files to the station.

---

## 🎯 Purpose

Ropeway performs three core tasks:

1. **Clone the DGRV3 repository**  
   - Using authenticated GitHub URLs  
   - Using the English branch as the baseline

2. **Duplicate the English branch into `DGRV3_EN`**  
   - This folder is used for comparison, patching, and UPS generation

3. **Clone or merge additional branches (beta branches)**  
   - Controlled by config flags or `--beta`  
   - Uses VGitUtils to merge multiple branches into a single working tree  
   - Detects merge conflicts and falls back safely

Ropeway ensures that the entire text pipeline starts with a **clean, complete, multi‑branch repository**.

---

## 🧩 How Ropeway Fits Into TextTools

Ropeway is stage **1** of the TextTools pipeline:

1. **Ropeway** — clone & prepare repo  
2. **CrammedPiranhas** — extract SPC/STX archives  
3. **MarkerStone** — line counting  
4. **Necronomicon** — hash calculation  
5. **Bugvac** — name replacement  
6. **HydraulicPress** — variable replacement  
7. **PoolRules** — percentage calculation  
8. **Electrohammer** — text injection  
9. **Crossbow** — distribution  
10. **WhiteSheet** — UPS patch generation

Without Ropeway, none of the other tools have a repository to operate on.

---

## ⚙️ Features & Responsibilities

### ✔ 1. Authentication & URL Construction
Ropeway uses:

- GitHub username  
- GitHub token  
- GitHub repo owner  

to build an authenticated HTTPS URL:

```cpp
CalculateRepoURL(username, token, password, owner, repo);
```

Password authentication is deprecated; token is required.

### ✔ 2. Clone English Branch First

Ropeway clones the english branch:

```cpp
VGit::CloneRepositoryBranch(repo_url, "english");
```

This becomes the baseline folder:

DGRV3/

Then it duplicates this folder into:

DGRV3_EN/

This second folder is used for comparison and patch generation.

### ✔ 3. Switch Original Clone to Main Branch

After cloning English, Ropeway switches the original folder to the main branch:

```
git checkout master
git fetch origin
git pull
```

This ensures the main branch is up‑to‑date before merging beta branches.

### ✔ 4. Beta Branch Cloning & Merging

If enabled via config or --beta, Ropeway attempts to merge all non‑blacklisted branches:

```cpp
VGitUtils::CloneBeta(true, repo, repo_url, blacklist);
```

This process:

	- Lists all remote branches

	- Filters out blacklisted ones

	- Merges remaining branches

	- Detects conflicts

	- Falls back to cloning a single branch if merging fails

	- Logs conflicts to conflicts.txt

	- Writes vgit_failed.txt on fatal errors
	
## 📁 Folder Structure After Ropeway

After Ropeway finishes, the workspace looks like:

DGRV3/                ← main branch + merged beta branches
DGRV3_EN/             ← english branch (baseline)
TextConfig.config
vgit_failed.txt       (if cloning failed)
conflicts.txt         (if merge conflicts occurred)

This structure is required by all downstream tools.

## 🧪 Command‑Line Flags

`--beta`: Forces beta branch merging even if disabled in config.

## 🔧 Configuration

Ropeway reads:

`TextConfig.config`

Important keys include:

CloneBetas — whether to merge beta branches

## 🚦 Failure Handling

Ropeway stops execution if:

	- GitHub credentials are missing

	- The repository cannot be cloned

	- Beta merging fails

	- Merge conflicts cannot be resolved

It writes: `vgit_failed.txt` so HiddenDoor can abort early.

## 🧠 Internal Logic Summary

- Read config

- Build authenticated GitHub URL

- Clone English branch

- Duplicate English → DGRV3_EN

- Switch original clone to main branch

- Optionally merge beta branches

- Log errors

- Exit cleanly

## 🏁 Summary

Ropeway is the foundation of TextTools:

- It clones the repo

- Prepares English and main branches

- Merges beta branches

- Ensures a clean workspace

- Provides deterministic input for all later tools