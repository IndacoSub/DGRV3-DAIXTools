# TEXTTOOLS - HIDDENDOOR
## Team DAIX, 2026  

HiddenDoor is the central conductor of the entire TextTools suite.  
If TextTools were a symphony, HiddenDoor is the one holding the baton —  
coordinating every subsystem, enforcing execution order, handling flags,  
and ensuring the full text‑modding pipeline runs cleanly from start to finish.

HiddenDoor does **not** perform text extraction, hashing, replacement, or patching itself.  
Instead, it **runs all other TextTools programs** (except StackedBooks) in the correct order,  
passing arguments, checking dependencies, and stopping execution on failure.

---

## 📌 Purpose

HiddenDoor’s job is simple but critical:

- Run every TextTools component in the correct canonical order  
- Pass through command‑line flags (`--all`, `--report`, `--skip-ropeway`)  
- Check for required dependencies (Git, .NET)  
- Stop execution early if Ropeway/VGit fail  
- Handle multi‑stage tools (programs with “part2”)  
- Produce optional reports  
- Ensure reproducible, deterministic text compilation

HiddenDoor is the “main entrance” to TextTools —  
StackedBooks is just a wrapper that launches HiddenDoor.

---

## 🧩 Pipeline Overview

HiddenDoor executes the following tools in sequence:

| Stage | Tool | Description |
|-------|------|-------------|
| 1 | **Ropeway** | Clone/update the DGRV3 text repository (unless `--skip-ropeway`) |
| 2 | **CrammedPiranhas** | Extract archives (SPC/STX) for text assets |
| 3 | **MarkerStone** | Line counting, text block mapping |
| 4 | **Necronomicon** | Hash calculation, CRC mapping |
| 5 | **Bugvac** | Name replacement (C#) |
| 6 | **HydraulicPress** | Variable → string replacement |
| 7 | **PoolRules** | Percentage calculations |
| 8 | **Electrohammer** | Actual text injection into SPC/STX |
| 9 | **Crossbow** | Copy modified files into distribution folder |
| 10 | **WhiteSheet** *(DISTRIBUTE)* | Generate UPS patches |
| 11 | **CrammedPiranhas (part2)** | Final cleanup |
| 12 | **Crossbow (part2)** | Final distribution cleanup |
| 13 | **Necronomicon (part2)** | Secondary hash pass |
| 14 | **WhiteSheet (part2)** | Final UPS patch pass |

HiddenDoor ensures each stage runs only if the previous one succeeded.

---

## 🏁 Command‑Line Flags

HiddenDoor supports several flags:

### `--all`
Runs every possible stage, including optional ones.

### `--report`
Appends output to `report.txt` for debugging or auditing.

### `--skip-ropeway`
Skips repository cloning.  
Use this only if the repo is already cloned and prepared.

---

## 🔧 Dependency Checks

Before running anything, HiddenDoor verifies:

- **Git** — required by Ropeway  
- **.NET** — required by Electrohammer and older STX/SPC tools  

If either is missing, execution stops immediately.

---

## 🚦 Failure Handling

HiddenDoor stops execution if:

- `ropeway_failed.txt` exists  
- `vgit_failed.txt` exists  
- Any tool returns a non‑zero exit code  
- A required executable is missing  

This prevents corrupted or partial builds.

---

## 🧠 Internal Logic Summary

HiddenDoor:

1. Parses arguments  
2. Builds a list of `Command` objects  
3. Appends `.exe` or `./` depending on platform  
4. Checks file existence  
5. Executes each tool via `Common::executeBatch()`  
6. Stops on first failure  
7. Prints timestamps before and after execution  

It is intentionally simple — reliability over complexity.

---

## 📂 File Structure

HiddenDoor expects the following executables in the working directory:

Ropeway(.exe)
CrammedPiranhas(.exe)
MarkerStone(.exe)
Necronomicon(.exe)
Bugvac(.exe)
HydraulicPress(.exe)
PoolRules(.exe)
Electrohammer(.exe)
Crossbow(.exe)
WhiteSheet(.exe)   (if DISTRIBUTE enabled)


If any are missing, HiddenDoor aborts.

---

## 🧪 Example Usage

### Run full pipeline:

```
HiddenDoor --all
```

### Run pipeline but skip Ropeway:

```
HiddenDoor --skip-ropeway
```

### Generate a report:

```
HiddenDoor --report
```

### Full run with report:

```
HiddenDoor --all --report
```


---

## 🏆 Summary

HiddenDoor is the **master controller** of TextTools:

- It runs everything  
- It enforces order  
- It handles flags  
- It checks dependencies  
- It stops on failure  
- It produces final timestamps  

Without HiddenDoor, TextTools would be a scattered collection of executables.  
With HiddenDoor, it becomes a unified, deterministic, fully automated pipeline.