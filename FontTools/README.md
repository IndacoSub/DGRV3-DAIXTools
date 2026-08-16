# DGRV3-DAIXTOOLS — FONTTOOLS
## Team DAIX, 2026

---

## 📌 Overview

**FontTools** is the font-processing subsystem of **DGRV3-DAIXTOOLS**.

If you cannot read the code, this repository is **not** for you.

PLEASE **read the code** before asking any questions.

**Generative AI** was used in the creation of this project’s code, comments, documentation, and miscellaneous supporting files.  

Our translation was not generated or supported by AI.

FontTools is designed to automate the complete process of taking a prepared **DGRV3-Font** repository, compiling its font variants, inserting the resulting font resources into the appropriate SPC archives, filtering those archives down to only the files that actually changed, and — when distribution mode is enabled — generating UPS patches suitable for distribution.

FontTools is deliberately much more ambitious than a conventional font editor.

Its purpose is not:

> "Change one font."

Its purpose is:

> **"Rebuild the game's font system reproducibly and automatically."**

The pipeline is designed for large-scale font work where many font variants must be processed consistently.

---

## ⚠️ IMPORTANT WARNING

**FONTTOOLS IS NOT A CASUAL MODDING TOOL.**

It performs automated operations that can:

- delete files,
- overwrite generated assets,
- modify SPC archives,
- remove unmodified archives from working directories,
- create and destroy temporary files,
- modify Git repositories,
- generate distribution patches,
- and package game assets.

Several components assume that the input repository is clean.

If you do not understand what the pipeline is doing, **do not run the complete pipeline on your only copy of the repository.**

Keep a clean source copy.

---

## 🧭 What FontTools Is For

FontTools is intended for people who need:

- automated processing of many game fonts,
- automated STX/SRDV generation,
- automated SPC modification,
- reproducible font builds,
- branch-aware font repository preparation,
- automated filtering of changed SPCs,
- automated UPS patch generation,
- or integration with a larger translation/build pipeline.

It is particularly useful when manually repeating the same font operations would be impractical.

---

## 🆚 FontTools vs. Harmony-Tools

If you only want to modify a font manually, **use Harmony-Tools instead.**

Harmony-Tools is intended to be the simpler, more direct solution for individual font editing.

FontTools exists for the opposite use case:

| Harmony-Tools | FontTools |
|---|---|
| Manual / interactive | Automated |
| Individual modifications | Large-scale processing |
| Simple workflow | Multi-stage pipeline |
| User-driven | Orchestrator-driven |
| Convenient | Reproducible |
| Single-tool workflow | Multiple cooperating tools |
| Casual modding | Full rebuild/distribution workflow |

FontTools intentionally sacrifices convenience in exchange for automation and control.

And yes:

> **It is absolutely overkill if you only want to change one font.**

---

# 🧩 Architecture

FontTools consists of six logical stages:

```text
                    ┌──────────────────┐
                    │    MONOKUMA      │
                    │ Master Orchestrator│
                    └────────┬─────────┘
                             │
                             ▼
                    ┌──────────────────┐
                    │    MONOTARO      │
                    │ Repository Setup │
                    └────────┬─────────┘
                             │
                             ▼
                    ┌──────────────────┐
                    │   MONOPHANIE     │
                    │ Workspace Setup  │
                    └────────┬─────────┘
                             │
                             ▼
                    ┌──────────────────┐
                    │     MONODAM      │
                    │  Font Compiler   │
                    └────────┬─────────┘
                             │
                             ▼
                    ┌──────────────────┐
                    │     MONOKID      │
                    │   SPC Filter     │
                    └────────┬─────────┘
                             │
                             ▼
                    ┌──────────────────┐
                    │    MONOSUKE      │
                    │  UPS Generator   │
                    └──────────────────┘
````

The canonical order is:

```text
0. Monokuma
1. Monotaro
2. Monophanie
3. Monodam
4. Monokid
5. Monosuke
```

Monokuma is the orchestrator and normally runs the stages automatically.

The individual programs can also be run independently for debugging and development.

---

# 🔄 The Complete Data Flow

The easiest way to understand FontTools is to follow the data.

```text
DGRV3-Font repository
        │
        ▼
     Monotaro
        │
        │ clone / branch preparation
        ▼
Prepared DGRV3-Font repository
        │
        ▼
    Monophanie
        │
        │ extract tools
        │ duplicate base assets
        │ prepare font variants
        ▼
HTFont-ready font workspace
        │
        ▼
     Monodam
        │
        │ HTFont compilation
        │ STX/SRDV generation
        │ SPC insertion
        ▼
Modified SPC archives
        │
        │ list_changed.txt
        ▼
     Monokid
        │
        │ discard untouched SPCs
        ▼
ModifiedFiles-Font/
        │
        ├───────────────┐
        │               │
        ▼               ▼
   normal build     DISTRIBUTE
                        │
                        ▼
                    Monosuke
                        │
                        ▼
                 Distribute-Font/
                   *.ups patches
```

There is also a separate final packaging operation performed by Monokuma for the main `game_resident` font:

```text
game_resident/
        │
        ▼
v3_font00.stx
v3_font00.srdv
        │
        ▼
     Monokuma
        │
        ▼
gr_font.7z
```

---

# 🛠️ TOOL RESPONSIBILITIES

## 0. Monokuma — Master Orchestrator

**Monokuma** controls the complete FontTools workflow.

It:

* checks major dependencies,
* invokes the FontTools stages in order,
* aborts when required stages fail,
* handles distribution mode,
* packages the final `game_resident` font,
* and provides the overall automation layer.

Monokuma does **not** perform the actual font compilation itself.

It coordinates the other tools.

See:

`Monokuma/README.md`

for the complete implementation-level documentation.

---

# 1. Monotaro — Repository Manager

**Monotaro** prepares the `DGRV3-Font` repository.

Its responsibilities include:

* cloning the repository,
* authenticating against GitHub,
* enumerating remote branches,
* optionally processing beta branches,
* merging branches,
* handling merge conflicts,
* and reporting Git/VGit failures.

Its output is the repository that the remaining FontTools stages operate on.

Conceptually:

```text
GitHub
   │
   ▼
Monotaro
   │
   ▼
DGRV3-Font/
```

See:

`Monotaro/README.md`

---

# 2. Monophanie — Workspace Bootstrapper

**Monophanie** prepares the cloned repository for font compilation.

It is responsible for things such as:

* extracting SPCTool,
* extracting HTFont,
* preparing HTFont dependencies,
* copying base font resources,
* creating `base_fonts_copy`,
* creating required trial font variants,
* and normalizing STX/SRDV naming.

Its job is to make the repository conform to the structure expected by Monodam.

Conceptually:

```text
DGRV3-Font/
       │
       ▼
Monophanie
       │
       ▼
HTFont-ready workspace
```

See:

`Monophanie/README.md`

---

# 3. Monodam — Font Compiler

**Monodam** is the actual font compilation stage.

It:

1. discovers prepared font directories,
2. copies HTFont and its dependency into the required working location,
3. invokes HTFont,
4. obtains `.srd` and `.srdv`,
5. renames `.srd` to `.stx`,
6. determines the corresponding SPC,
7. inserts STX/SRDV using SpcTool,
8. records modified SPCs in `list_changed.txt`.

Conceptually:

```text
Prepared font
     │
     ▼
   HTFont
     │
     ├── .srd
     └── .srdv
          │
          ▼
      .stx + .srdv
          │
          ▼
       SpcTool
          │
          ▼
    Modified SPC
```

See:

`Monodam/README.md`

---

# 4. Monokid — SPC Distribution Filter

**Monokid** determines which SPC archives actually need to leave the build workspace.

It reads:

```text
list_changed.txt
```

and uses that information to determine which SPCs were modified by Monodam.

Unmodified SPCs are removed from the filtered working set.

The remaining SPCs are copied into:

```text
ModifiedFiles-Font/
```

This is the bridge between compilation and distribution.

Conceptually:

```text
All SPCs
   │
   ▼
Monokid
   │
   ├── unchanged → discard
   │
   └── changed ──→ ModifiedFiles-Font/
```

See:

`Monokid/README.md`

---

# 5. Monosuke — UPS Distribution Generator

**Monosuke** is the optional distribution stage.

It compares:

```text
original SPC
```

against:

```text
modified SPC
```

and produces:

```text
*.ups
```

patches.

It then removes the SPC and temporary files from the distribution directory, leaving the UPS patches.

Conceptually:

```text
Original SPC ─────┐
                  ├──► Monosuke ──► *.ups
Modified SPC ─────┘
```

Monosuke is enabled when FontTools is run in distribution mode.

See:

`Monosuke/README.md`

---

# 📁 DGRV3-Font Repository

The exact contents of the repository evolve during the pipeline.

The most useful way to understand it is to separate the **source repository** from the **generated workspace**.

A typical prepared workspace looks approximately like:

```text
DGRV3-Font/
│
├── base_spc/
│   │
│   ├── base_fonts/
│   │
│   ├── base_fonts_copy/
│   │
│   ├── SPCTool.7z
│   ├── SPCTool_Linux.7z
│   │
│   ├── HTFont.7z
│   ├── HTFont_Linux.7z
│   │
│   ├── 7za.exe / 7za
│   ├── assimp.dll / libassimp.so
│   └── ...
│
├── trial_font/
│   └── *_DEC/
│       └── v3_fontXX/
│
├── game_resident/
│   └── *_DEC/
│       └── v3_fontXX/
│
└── ...
```

The exact structure should always be verified against the individual tool READMEs and source code.

---

# 📦 `base_spc`

`base_spc` is the central tool/resource staging directory.

It contains the archives and helper utilities needed to prepare and compile fonts.

Depending on the stage and platform, this can include:

```text
SPCTool.7z
SPCTool_Linux.7z
HTFont.7z
HTFont_Linux.7z
7za.exe / 7za
assimp.dll / libassimp.so
ups.exe / ups
base_fonts/
```

The contents of this directory are consumed primarily by Monophanie and Monodam.

---

# 📂 `base_fonts`

`base_fonts` represents the original font SPC assets.

It is important to distinguish this from the working copies used later in the pipeline.

Treat the original base assets as **source material**.

Do not casually modify or delete them.

---

# 📂 `base_fonts_copy`

`base_fonts_copy` is the preserved copy used to provide the **unmodified reference SPCs** required by later stages, especially UPS generation.

The distinction is critical:

```text
base_fonts
     │
     └── original/reference assets

base_fonts_copy
     │
     └── preserved originals for comparison
```

Monosuke relies on an untouched reference copy when generating UPS patches.

If the "normal" SPC is already modified, the resulting patch is no longer a meaningful diff against the original game asset.

---

# 📂 `trial_font`

`trial_font` contains the game's multiple trial-font variants.

Monophanie prepares the required variants so that Monodam can process each one automatically.

Typical names include variants such as:

```text
game_font01_1_US_DEC
game_font01_2_US_DEC
game_font01_3_US_DEC
game_font01_4_US_DEC
game_font01_5_US_DEC
game_font01_6_US_DEC
game_font01_9_US_DEC
```

The exact set is determined by the repository and implementation.

---

# 📂 `game_resident`

`game_resident` contains the main game-resident font assets.

The important compiled resource is:

```text
v3_font00.stx
v3_font00.srdv
```

Monokuma uses the resulting `font00` assets for the final:

```text
gr_font.7z
```

package.

---

# 📄 `list_changed.txt`

`list_changed.txt` is one of the most important hand-off files in FontTools.

Monodam records the SPCs it modifies.

Monokid then uses that information to determine which SPCs should survive into:

```text
ModifiedFiles-Font/
```

The data flow is:

```text
Monodam
   │
   ▼
list_changed.txt
   │
   ▼
Monokid
   │
   ▼
ModifiedFiles-Font/
```

If this file is incorrect, missing, stale, or inconsistent with the modified SPCs, the filtering stage can produce incorrect results.

---

# 📤 Main Outputs

Depending on the execution mode, FontTools can produce:

### `ModifiedFiles-Font/`

Contains the SPC archives that survived Monokid's filtering process.

These are the modified SPCs that actually need to be distributed or further processed.

---

### `Distribute-Font/`

Created by Monosuke in distribution mode.

After UPS generation and cleanup, this directory is intended to contain:

```text
*.ups
```

patches rather than complete SPC archives.

---

### `list_changed.txt`

Records SPCs modified by Monodam.

---

### `gr_font.7z`

Created by Monokuma for the final game-resident font package.

It contains the compiled `font00` STX/SRDV resources.

---

### `vgit_failed.txt`

Created when Monotaro encounters a repository acquisition/merge failure that should prevent the pipeline from continuing.

Monokuma uses this as a failure signal.

---

### Other diagnostic files

Individual tools may create additional diagnostic artifacts such as:

```text
branches.txt
conflicts.txt
```

and tool-specific logs.

These are documented in the individual READMEs.

---

# 🔗 INTER-STAGE CONTRACTS

FontTools works because each stage produces the information expected by the next.

The most important contracts are:

```text
Monotaro
   │
   │ DGRV3-Font/
   ▼
Monophanie
   │
   │ prepared font directories
   ▼
Monodam
   │
   │ modified SPCs
   │ list_changed.txt
   ▼
Monokid
   │
   │ ModifiedFiles-Font/
   ▼
Monosuke
   │
   │ Distribute-Font/*.ups
   ▼
Distribution
```

This is why individual stages should not be casually rearranged.

---

# 🚦 FAILURE MODEL

The complete pipeline is intentionally strict.

The general rule is:

```text
Stage fails
    │
    ▼
Monokuma stops
```

For example:

```text
Monotaro fails
     ↓
Do not compile

Monophanie fails
     ↓
Do not compile

Monodam fails
     ↓
Do not filter

Monokid fails
     ↓
Do not generate patches

Monosuke fails
     ↓
Distribution failed
```

Some individual tools have non-fatal warnings internally.

Those details are documented in their dedicated READMEs.

---

# 🧨 DESTRUCTIVE OPERATIONS

Several stages modify or delete data.

## Monotaro

Can:

* delete repositories,
* reclone repositories,
* merge branches,
* rewrite Git state.

---

## Monophanie

Can:

* overwrite extracted files,
* copy directories recursively,
* create/replace working assets.

---

## Monodam

Can:

* delete temporary files,
* overwrite `.stx`,
* modify SPC archives in place.

---

## Monokid

Can:

* delete SPCs that are not considered modified,
* recreate `ModifiedFiles-Font/`.

---

## Monosuke

Can:

* create temporary SPC copies,
* rename files,
* generate patches,
* delete all non-UPS output from `Distribute-Font/`.

---

## Monokuma

As the orchestrator, Monokuma can indirectly trigger **all of the above**.

Therefore:

> **Always run the complete pipeline against a disposable/generated workspace or a clean clone.**

---

# 🧠 CLEAN BUILD PRINCIPLE

FontTools is designed around the assumption that the source assets are clean.

The preferred workflow is:

```text
Clean source
     │
     ▼
Monotaro
     │
     ▼
Fresh DGRV3-Font
     │
     ▼
Monophanie
     │
     ▼
Fresh build workspace
     │
     ▼
Monodam
     │
     ▼
Monokid
     │
     ▼
Monosuke
```

Avoid repeatedly running the entire pipeline against a previously modified workspace unless you specifically understand which files each stage preserves and overwrites.

This is particularly important for SPCs.

---

# ⚠️ ABOUT HTFONT

HTFont deserves special mention.

It comes from an older generation of **Harmony-Tools**.

The older Harmony-Tools architecture contained separate utilities for operations such as:

```text
HTFont
HTSrd
HTSpc
...
```

Modern Harmony-Tools has since moved toward a unified tool.

FontTools nevertheless continues to use the legacy HTFont workflow because:

* its STX/SRDV output is already compatible,
* its behavior is established,
* existing SPC workflows depend on it,
* changing compilers would introduce unnecessary compatibility risk.

The project therefore treats HTFont as a compatibility dependency, not merely as an interchangeable executable.

**DAIXTOOLS does not provide these proprietary/third-party tool archives.**

---

# 🖥️ PLATFORM SUPPORT

FontTools contains platform-specific handling for at least:

```text
Windows
Linux
```

Common differences include executable naming:

```text
Windows:
HTFont.exe
SpcTool.exe
7za.exe
ups.exe

Linux:
HTFont
SpcTool
7za
ups
```

HTFont also requires the appropriate Assimp runtime:

```text
Windows:
assimp.dll

Linux:
libassimp.so
```

Always use the archive intended for the target platform.

---

# ⚙️ PERFORMANCE

FontTools intentionally favors predictable sequential processing over aggressive parallelism.

The major expensive operations are:

### HTFont

Font packing can be CPU-intensive.

### SpcTool

SPC modification is primarily I/O-heavy.

### UPS generation

UPS creation can involve reading large original and modified SPCs.

The pipeline generally processes these operations sequentially.

This makes execution slower than a hypothetical parallel implementation, but it simplifies ordering and reduces the risk of multiple tools simultaneously modifying related assets.

---

# 🔐 REPRODUCIBILITY

Reproducibility is one of the main design goals of FontTools.

A clean build should conceptually be:

```text
same repository
+
same font inputs
+
same tool versions
+
same configuration
=
same generated assets
```

In practice, reproducibility also depends on:

* the exact HTFont build,
* the exact SpcTool build,
* the exact source repository revision,
* the branch-selection behavior,
* the original SPC assets,
* and the platform/toolchain environment.

For serious release builds, preserve the exact tool versions used.

---

# 📦 DISTRIBUTION MODE

FontTools supports a distribution-oriented workflow.

When distribution mode is disabled:

```text
Monokuma
   │
   ▼
Monokid
   │
   ▼
ModifiedFiles-Font/
```

When distribution mode is enabled:

```text
Monokuma
   │
   ▼
Monokid
   │
   ▼
ModifiedFiles-Font/
   │
   ▼
Monosuke
   │
   ▼
Distribute-Font/
   │
   ▼
*.ups
```

The purpose of this mode is to distribute the **differences** between original and modified assets rather than distributing complete modified SPC archives.

---

# 🧩 WHY UPS PATCHES?

A complete SPC archive can contain game assets that should not be redistributed as part of a modification package.

A UPS patch instead represents the binary differences between:

```text
original SPC
```

and:

```text
modified SPC
```

The resulting distribution therefore consists of:

```text
game_font01_1_patch.ups
game_font01_2_patch.ups
game_font01_3_patch.ups
...
```

rather than complete SPC archives.

This is the intended distribution model for FontTools.

---

# 🐻 MONOKUMA FINAL PACKAGING

After the FontTools stages have completed, Monokuma also handles the main game-resident font package.

The relevant resources are:

```text
v3_font00.stx
v3_font00.srdv
```

from the compiled `game_resident` font.

Monokuma packages them into:

```text
gr_font.7z
```

This is separate from the UPS distribution process.

In other words:

```text
FontTools build
      │
      ├──► ModifiedFiles-Font/
      │
      ├──► Distribute-Font/*.ups
      │
      └──► gr_font.7z
```

---

# 🧪 TROUBLESHOOTING

## `7za` not found

Check that the expected 7-Zip executable is available.

Windows:

```text
7za.exe
```

Linux:

```text
7za
```

The exact location expected by the pipeline is documented by Monophanie/Monokuma.

---

## HTFont fails

Check:

* HTFont was extracted correctly,
* the correct platform version was used,
* `assimp.dll` / `libassimp.so` is available,
* the font directory has the expected structure,
* and the prepared font data was generated by Monophanie.

---

## SPC not found

Check:

```text
base_spc/base_fonts/
```

and verify that the expected original SPC archive exists.

Also verify the `_DEC` naming conventions used by the font directories.

---

## `list_changed.txt` is empty

Possible causes include:

* no font directories were discovered,
* the directory naming convention is wrong,
* HTFont did not generate the expected resources,
* no valid SPC mapping was found,
* or Monodam did not successfully insert anything.

Check the Monodam README/logs before continuing.

---

## `ModifiedFiles-Font` is empty

This usually means Monokid found no SPCs that matched the entries in:

```text
list_changed.txt
```

Verify that:

* Monodam actually modified SPCs,
* `list_changed.txt` contains the expected entries,
* the relevant SPCs still exist,
* and the repository has not been altered between stages.

---

## `Distribute-Font` contains no UPS files

Check:

1. Monodam modified at least one SPC.
2. Monokid produced `ModifiedFiles-Font/`.
3. `base_fonts_copy` contains the untouched originals.
4. The UPS executable is available.
5. Distribution mode was actually enabled.

---

## `gr_font.7z` is missing

Check that Monokuma can locate:

```text
v3_font00.stx
v3_font00.srdv
```

inside the expected `game_resident` output.

---

# 📚 INDIVIDUAL TOOL DOCUMENTATION

FontTools is intentionally documented at two levels.

This README explains:

* architecture,
* pipeline order,
* data flow,
* responsibilities,
* major dependencies,
* output artifacts,
* and operational expectations.

Each individual tool has its own README for implementation-specific details.

| Tool           | Documentation                                 |
| -------------- | --------------------------------------------- |
| **Monokuma**   | Master orchesator / complete pipeline         |
| **Monotaro**   | Repository cloning and branch management      |
| **Monophanie** | Archive extraction and workspace preparation  |
| **Monodam**    | Font compilation and SPC insertion            |
| **Monokid**    | SPC filtering and modified-file extraction    |
| **Monosuke**   | UPS patch generation and distribution cleanup |

**Read the individual README before modifying or replacing any stage.**

---

# 🧭 Pipeline at a Glance

```text
┌─────────────────────────────────────────────────────────────┐
│                         MONOKUMA                            │
│                   Master Orchestrator                      │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                         MONOTARO                            │
│                Clone / Update / Merge Repo                  │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           │ DGRV3-Font/
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                       MONOPHANIE                            │
│             Extract Tools / Prepare Font Data               │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           │ prepared font directories
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                         MONODAM                             │
│        HTFont → STX/SRDV → SpcTool → Modified SPC           │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           │ list_changed.txt
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                         MONOKID                             │
│              Keep Only Modified SPC Archives                │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           │ ModifiedFiles-Font/
                           ▼
                 ┌─────────┴──────────┐
                 │                    │
                 │ NORMAL BUILD       │ DISTRIBUTE
                 │                    │
                 │                    ▼
                 │              ┌──────────────┐
                 │              │   MONOSUKE   │
                 │              │ UPS Generator │
                 │              └──────┬───────┘
                 │                     │
                 │                     ▼
                 │              Distribute-Font/
                 │                  *.ups
                 │
                 ▼
          Modified SPC assets


                         AND

                 game_resident font00
                         │
                         ▼
                     MONOKUMA
                         │
                         ▼
                    gr_font.7z
```

---

# 🧠 Design Philosophy

FontTools is built around a few core principles.

## Automation

The pipeline should eliminate repetitive manual work.

---

## Determinism

Given equivalent inputs and tool versions, the build should produce equivalent results.

---

## Separation of Responsibilities

Each executable has one primary job:

```text
Monotaro   → repository
Monophanie → preparation
Monodam    → compilation
Monokid    → filtering
Monosuke   → distribution
Monokuma   → orchestration
```

---

## Reuse of Established Binary Tools

FontTools does not attempt to reinvent every proprietary game asset format.

Instead, it coordinates established tools such as:

```text
HTFont
SpcTool
UPS
7-Zip
Git
```

---

## Minimal Distribution

The build pipeline can produce complete modified assets internally while the distribution pipeline reduces those changes to patches.

That separation is deliberate:

```text
BUILD
  ↓
complete modified assets

DISTRIBUTION
  ↓
minimal binary patches
```

---

# 🏁 Final Summary

**FontTools is an automated font build and distribution pipeline for DGRV3-DAIXTOOLS.**

Its complete workflow is:

```text
Monotaro
    ↓
Acquire / prepare DGRV3-Font
    ↓
Monophanie
    ↓
Extract and prepare font assets
    ↓
Monodam
    ↓
Compile fonts + rebuild SPCs
    ↓
Monokid
    ↓
Keep only modified SPCs
    ↓
┌───────────────────────┐
│                       │
│ Normal build          │ Distribution build
│                       │
▼                       ▼
ModifiedFiles-Font/    Monosuke
                        ↓
                    UPS patches
                        ↓
                  Distribute-Font/
```

Meanwhile, Monokuma packages the final `game_resident` font resources into:

```text
gr_font.7z
```

The entire system is designed around:

* **automation**
* **reproducibility**
* **large-scale font processing**
* **strict pipeline ordering**
* **deterministic asset preparation**
* **SPC reconstruction**
* **minimal patch-based distribution**

It is not the easy way to edit fonts.

It is the way to automate the entire job.

If you only need to edit a font:

> **Use Harmony-Tools.**

If you need the entire DGRV3 font pipeline automated:

> **Welcome to FontTools.**