# DGRV3-DAIXTOOLS TOOLCHAIN — TEXTTOOLS
## Team DAIX, 2026

---

## 📌 Overview

**TextTools** is the text-processing and script-compilation subsystem of **DGRV3-DAIXTOOLS**.

If you cannot read the code, this repository is **not** for you.

PLEASE **read the code** before asking any questions.

**Generative AI** was used in the creation of this project’s code, comments, documentation, and miscellaneous supporting files.  

Our translation was not generated or supported by AI.

TextTools' purpose is to take a translated Danganronpa V3 script repository and transform it through a sequence of:

- repository acquisition,
- archive extraction,
- structural validation,
- byte-level change detection,
- character-name processing,
- variable compilation,
- platform filtering,
- translation analysis,
- STX generation,
- SPC injection,
- modified-resource selection,
- and finally UPS patch generation.

TextTools is therefore considerably more than a collection of text-processing utilities.

It is an **end-to-end build pipeline**.

At the beginning of the pipeline, the data looks approximately like:

```text
Translated TXT/PB files
        +
English/reference files
        +
Base SPC resources
        +
Tool archives
````

By the end, the pipeline can produce:

```text
Game-ready modified SPC/PB resources
        +
UPS patches for distribution
```

The fundamental architecture is:

```text
SOURCE
  ↓
VALIDATE
  ↓
DETECT CHANGES
  ↓
PROCESS TEXT
  ↓
COMPILE TEXT
  ↓
BUILD SPC RESOURCES
  ↓
SELECT MODIFIED RESOURCES
  ↓
GENERATE PATCHES
```

---

# ⚠️ IMPORTANT WARNING

**TEXTTOOLS IS NOT A CASUAL MODDING TOOL.**

This toolchain is designed for large-scale translation work and automated game-resource generation.

It is intended for people who need:

* automated processing of thousands of script files,
* reproducible builds,
* automated validation,
* variable replacement,
* platform-specific filtering,
* incremental compilation,
* SPC rebuilding,
* and automated UPS patch generation.

If you only want to:

* edit a handful of lines,
* make a meme mod,
* replace a few names,
* or manually modify one script,

use **Harmony-Tools** instead.

Harmony-Tools is designed for simpler manual workflows.

TextTools exists because manually performing every step across a large translation project is impractical.

And, frankly:

> **This pipeline is for people who enjoy making computers suffer so they don't have to.**

---

# 🧩 TEXTTOOLS ARCHITECTURE

TextTools consists of several layers.

The easiest way to understand the system is to divide the tools into functional groups.

```text
┌──────────────────────────────────────────────┐
│              PIPELINE CONTROL                │
│                                              │
│  StackedBooks → HiddenDoor                  │
└──────────────────────┬───────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│             REPOSITORY PREPARATION           │
│                                              │
│  Ropeway                                     │
│  CrammedPiranhas                             │
└──────────────────────┬───────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│            CHANGE / STRUCTURE ANALYSIS       │
│                                              │
│  MarkerStone                                 │
│  Necronomicon                                │
└──────────────────────┬───────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│               TEXT PROCESSING                │
│                                              │
│  Bugvac                                      │
│  HydraulicPress                              │
│  LooseFloorboard                             │
└──────────────────────┬───────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│             TRANSLATION ANALYSIS             │
│                                              │
│  PoolRules                                   │
└──────────────────────┬───────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│             GAME RESOURCE BUILD              │
│                                              │
│  ElectroHammer                               │
└──────────────────────┬───────────────────────┘
                       │
                       ▼
┌──────────────────────────────────────────────┐
│            RESOURCE DISTRIBUTION             │
│                                              │
│  Crossbow                                    │
│  CrammedPiranhas Part2                       │
│  WhiteSheet                                   │
│  Crossbow Part2                              │
│  Necronomicon Part2                          │
│  WhiteSheet Part2                             │
└──────────────────────────────────────────────┘
```

---

# 🧭 CANONICAL PIPELINE

When executed through **StackedBooks**, the pipeline is controlled by **HiddenDoor**.

The normal canonical order is:

```text
0.  StackedBooks
1.  HiddenDoor
2.  Ropeway
3.  CrammedPiranhas
4.  MarkerStone
5.  Necronomicon
6.  Bugvac
7.  HydraulicPress
8.  PoolRules
9.  ElectroHammer
10. Crossbow
```

If `DISTRIBUTE` mode is enabled, the pipeline continues into the second distribution phase:

```text
11. CrammedPiranhas part2
12. WhiteSheet part1
13. Crossbow part2
14. Necronomicon part2
15. WhiteSheet part2
```

Conceptually:

```text
                    ┌───────────────┐
                    │ StackedBooks  │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │   HiddenDoor  │
                    │  Orchestrator │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │    Ropeway    │
                    │ Repository    │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │ CrammedPiranhas│
                    │   Extraction  │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │  MarkerStone  │
                    │ Line Checking │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │ Necronomicon  │
                    │ SHA Detection │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │     Bugvac    │
                    │ Name Handling │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │HydraulicPress │
                    │ Text Compile  │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │   PoolRules   │
                    │  Statistics   │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │ ElectroHammer │
                    │ TXT → STX → SPC│
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │    Crossbow   │
                    │ Select Assets │
                    └───────┬───────┘
                            │
                       DISTRIBUTE
                            │
                            ▼
                    ┌───────────────┐
                    │CrammedPiranhas│
                    │    Part 2     │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │  WhiteSheet   │
                    │    Part 1     │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │    Crossbow   │
                    │    Part 2     │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │ Necronomicon  │
                    │    Part 2     │
                    └───────┬───────┘
                            │
                            ▼
                    ┌───────────────┐
                    │  WhiteSheet   │
                    │    Part 2     │
                    └───────────────┘
```

---

# 🐻 STACKEDBOOKS — MASTER ENTRY POINT

**StackedBooks** is the outer wrapper around TextTools.

Its job is intentionally simple:

```text
StackedBooks
     ↓
HiddenDoor
```

It exists primarily so that users have a single high-level executable to launch.

It does not replace HiddenDoor's orchestration logic.

Think of the relationship as:

```text
StackedBooks = launcher
HiddenDoor   = orchestrator
```

---

# 🚪 HIDDENDOOR — PIPELINE ORCHESTRATOR

**HiddenDoor** is the actual master controller of TextTools.

It runs the individual tools in their required order.

It is responsible for:

* launching each component,
* passing appropriate arguments,
* handling the Part 2 distribution sequence,
* and coordinating the complete TextTools workflow.

The individual tools can still be run manually for debugging.

However:

> **HiddenDoor is the canonical way to execute the full pipeline.**

This distinction is important because several tools produce files that are explicitly consumed by later tools.

For example:

```text
Necronomicon
      │
      ▼
different_hashes.txt
      │
      ├──────────────► HydraulicPress
      │
      └──────────────► ElectroHammer
```

and:

```text
Necronomicon Part2
      │
      ▼
different_part2.txt
      │
      ▼
WhiteSheet Part2
```

---

# 1. ROPEWAY — REPOSITORY ACQUISITION

**Ropeway** is the repository bootstrapper.

Its job is to obtain/update the TextTools translation repository.

Conceptually:

```text
GitHub repository
       │
       ▼
     Ropeway
       │
       ▼
      DGRV3/
```

Ropeway handles repository acquisition rather than text compilation.

Depending on configuration, it can also deal with beta branches and repository merging.

The result is a working `DGRV3` repository for downstream tools.

---

# 2. CRAMMEDPIRANHAS — TOOL & ASSET EXTRACTION

**CrammedPiranhas** prepares the binary resources required by TextTools.

It extracts things such as:

* SPC archives,
* STX tools,
* SPCTool,
* fonts,
* Switch localization resources,
* and other platform-specific assets.

It operates in two major modes.

## Normal mode

Prepares the environment for text compilation.

Conceptually:

```text
base_spc/*.7z
      │
      ▼
CrammedPiranhas
      │
      ├── SPC archives
      ├── STX tools
      ├── SPCTool
      ├── fonts
      └── platform assets
```

## Part 2 mode

Prepares the reference assets needed by the distribution/patching stage.

```text
CrammedPiranhas part2
        │
        ▼
Distribute/
```

The Part 2 invocation intentionally skips some of the normal compiler-tool extraction because its purpose is different.

---

# 3. MARKERSTONE — STRUCTURAL VALIDATION

**MarkerStone** checks the structural consistency of the translation.

Its primary comparison is:

```text
DGRV3_EN/
    VS
DGRV3/
```

It counts lines and determines whether corresponding English and translated scripts contain the expected number of lines.

Its most important question is:

```text
"Did the translation preserve the script's structure?"
```

This is important because a line-count mismatch can indicate:

* missing lines,
* extra lines,
* incorrect newline handling,
* or structural changes that could affect later injection.

MarkerStone produces information used to identify problematic sections.

---

# 4. NECRONOMICON — CHANGE DETECTION

**Necronomicon** performs byte-level change detection.

Where MarkerStone asks:

```text
"Do these files have the same line structure?"
```

Necronomicon asks:

```text
"Do these files contain the same bytes?"
```

It computes SHA-512 hashes and compares translated files against their English/reference counterparts.

Its most important output is:

```text
different_hashes.txt
```

It also produces:

```text
different_hash_folders.txt
sha_diff.txt
```

These files are not merely reports.

They are **pipeline metadata**.

The critical relationship is:

```text
Necronomicon
      │
      ├── different_hashes.txt
      │          │
      │          ├── HydraulicPress
      │          └── ElectroHammer
      │
      └── different_hash_folders.txt
                 │
                 └── ElectroHammer
```

This enables incremental processing.

Instead of recompiling the entire game, later tools can determine which files/folders actually changed.

---

# 5. BUGVAC — CHARACTER NAME PROCESSING

**Bugvac** is the character-name transformation stage.

Its purpose is to perform the project's name/surname substitutions in the script data.

The C# implementation exists separately from the C++ TextTools components but participates in the same pipeline.

Conceptually:

```text
Translated script
       │
       ▼
     Bugvac
       │
       ▼
Name-transformed script
```

Bugvac is therefore a **content transformation stage**, not a validation or compilation stage.

Its output is consumed by later text-processing stages.

---

# 6. HYDRAULICPRESS — SCRIPT COMPILATION

**HydraulicPress** is the central text-processing stage.

It takes the translation repository and transforms the editable script representation into a more final, platform-aware representation.

Its responsibilities include:

* loading `TextConfig.config`,
* selecting the platform,
* loading `vars.txt` or `vars_bak.txt`,
* determining which files require processing,
* checking variables,
* replacing `VAR_*` tokens,
* expanding `MAKE_*` constructs,
* handling `SIGNAL_*` markers,
* applying platform filters,
* generating baked scripts,
* optionally running LooseFloorboard,
* optionally randomizing text,
* and optionally generating word-frequency statistics.

The central processing relationship is:

```text
Necronomicon
      │
      ▼
different_hashes.txt
      │
      ▼
HydraulicPress
      │
      ├── VariableChecker
      ├── VariableReplacer
      ├── Platform
      ├── LooseFloorboard
      ├── Randomizer
      └── WordCounter
      │
      ▼
Processed DGRV3 text
```

HydraulicPress therefore bridges:

```text
translation source
       ↓
compiled script representation
```

---

# 7. LOOSEFLOORBOARD — CHARACTER COMPATIBILITY

**LooseFloorboard** is a helper used by HydraulicPress.

Its purpose is to replace unsupported or blacklisted accented characters with compatible alternatives.

This is especially relevant when a target platform cannot safely display certain Unicode characters.

The relationship is:

```text
HydraulicPress
      │
      ▼
LooseFloorboard
      │
      ▼
Character-compatible script
```

LooseFloorboard is therefore not a standalone pipeline stage in the same sense as Necronomicon or ElectroHammer.

It is a **subcomponent of the HydraulicPress processing stage**.

---

# 8. POOLRULES — TRANSLATION STATISTICS

**PoolRules** analyzes translation progress.

Unlike Necronomicon, PoolRules does not primarily determine what must be rebuilt.

Instead, it measures the state of the translation.

It tracks quantities such as:

* translated lines,
* untranslated lines,
* total lines,
* translated characters,
* untranslated characters,
* translation percentage,
* character percentage,
* and line-count consistency.

It processes the recognized TextTools folders in a fixed 14-folder set:

```text
ainori
chapter1
chapter2
chapter3
chapter4
chapter5
chapter6
epilogue
gallery
game_resident
MapObjName
prologue
subroutine
test
```

The translation percentage is calculated from translated versus total line counts.

The character percentage is based on translated versus untranslated character counts.

PoolRules therefore answers:

```text
"How complete is the translation?"
```

rather than:

```text
"Which files should be compiled?"
```

Its reports are primarily analytical.

---

# 9. ELECTROHAMMER — TEXT → STX → SPC

**ElectroHammer** is the main game-resource compiler.

It takes processed text and turns it into resources the game can actually consume.

Its fundamental transformation is:

```text
TXT
 │
 ▼
STX
 │
 ▼
SPC
```

More specifically:

```text
DGRV3/*.txt
      │
      ▼
Entry Management
      │
      ▼
Selected script entries
      │
      ▼
STX compiler
      │
      ▼
*.stx
      │
      ▼
SpcTool
      │
      ▼
DGRV3/base_spc/*.SPC
```

ElectroHammer has two major conceptual layers:

```text
EntryManagement
        +
Compiler
```

EntryManagement determines:

```text
WHAT should be compiled?
```

The compiler determines:

```text
HOW should it be compiled?
```

ElectroHammer also uses Necronomicon's change metadata to support incremental compilation.

The relationship is:

```text
Necronomicon
      │
      ├── different_hashes.txt
      └── different_hash_folders.txt
                  │
                  ▼
            ElectroHammer
                  │
                  ▼
          Compile changed data
```

This is one of the most important performance features of the entire TextTools pipeline.

---

# 🔨 ELECTROHAMMER OUTPUT

The major output of ElectroHammer is the modified SPC set inside:

```text
DGRV3/base_spc/
```

These archives contain the generated STX resources inserted into the game's existing SPC structure.

Depending on the selected STX pipeline, ElectroHammer may use:

```text
STXTool
```

or:

```text
NewSTXTool / DRV3_STX_TOOL
```

The new pipeline may use intermediate directories such as:

```text
EXTRACTED_FILES/
REPACKED_FILES/
```

and produce:

```text
eh_report.txt
```

as a build manifest.

---

# 10. CROSSBOW — MODIFIED RESOURCE SELECTION

**Crossbow** is the first distribution-selection stage.

ElectroHammer modifies game resources.

Crossbow determines:

```text
"Which of those resources actually need to be distributed?"
```

It uses:

```text
different_hashes.txt
```

to determine which chapters/folders contain modifications.

Conceptually:

```text
Necronomicon
      │
      ▼
different_hashes.txt
      │
      ▼
Crossbow
      │
      ├── determine modified chapters
      ├── find matching SPC/PB resources
      ├── map them to distribution paths
      └── copy them
             │
             ▼
       ModifiedFiles/
```

Crossbow therefore performs **selection**, not compilation.

---

# 📦 CROSSBOW RESOURCE MAPPING

Crossbow maps resources into distribution-oriented structures.

Typical categories include:

```text
game_resident/
wrd_data/
wrd_script/007/
master/
```

The exact mapping is determined by the implementation.

PB resources receive different treatment from normal SPC resources.

Crossbow also contains special handling for trial/minigame resources.

---

# 💩 PART 2 — WHY IT EXISTS

TextTools has a second distribution phase because not every game resource follows the same path as the primary translated script SPCs.

Some assets require:

* separate reference extraction,
* special trial handling,
* modified-vs-normal comparison,
* and a second UPS generation pass.

Therefore the distribution pipeline is intentionally split.

The Part 2 architecture is:

```text
Crossbow Part 1
      │
      ▼
ModifiedFiles/
      │
      ▼
CrammedPiranhas Part 2
      │
      ▼
Distribute/
      │
      ▼
WhiteSheet Part 1
      │
      ▼
Crossbow Part 2
      │
      ▼
Necronomicon Part 2
      │
      ▼
different_part2.txt
      │
      ▼
WhiteSheet Part 2
      │
      ▼
Final UPS patches
```

---

# 11. CRAMMEDPIRANHAS PART 2

The second invocation of CrammedPiranhas is not simply a duplicate of the first.

It prepares the **reference distribution assets**.

The command is conceptually:

```text
CrammedPiranhas part2
```

It creates/prepares:

```text
Distribute/
```

and extracts the base resources required by the distribution stage.

It can also prepare English/French trial SPCs for the special trial-resource workflow.

---

# 12. WHITESHEET — UPS PATCH GENERATION

**WhiteSheet** is the patch-generation component.

Its fundamental purpose is:

```text
original resource
       +
modified resource
       ↓
UPS patch
```

The important distinction is:

```text
ElectroHammer
    ↓
creates the modified game resource

WhiteSheet
    ↓
creates the distributable difference
```

WhiteSheet therefore operates at the **distribution layer**, not the text-compilation layer.

---

# 🧬 NORMAL DISTRIBUTION FLOW

The primary distribution sequence is:

```text
ElectroHammer
      │
      ▼
Modified SPCs
      │
      ▼
Crossbow
      │
      ▼
ModifiedFiles/
      │
      ▼
WhiteSheet
      │
      ▼
UPS patches
```

The patches represent the binary difference between the original/reference resource and the modified resource.

---

# 13. CROSSBOW PART 2

Crossbow Part 2 handles the special distribution resources prepared during the second stage.

It is invoked as:

```text
Crossbow part2
```

Its rules differ from normal Crossbow operation.

Part 2 includes special handling for trial resources such as:

```text
hanron
nonstop
panic
kokoronpa
nico
```

and can rename French trial resources into the corresponding US naming convention.

This stage prepares the exact set of resources that Necronomicon Part 2 will compare.

---

# 14. NECRONOMICON PART 2

Necronomicon's second mode performs binary comparison on the distribution resources.

Normal mode compares:

```text
DGRV3/
    VS
DGRV3_EN/
```

Part 2 compares modified distribution assets against their `_normal` counterparts.

Conceptually:

```text
Distribute/
     │
     ├── modified.spc
     └── modified_normal.spc
             │
             ▼
       Necronomicon part2
             │
             ▼
       different_part2.txt
```

Its main outputs are:

```text
different_part2.txt
different_part2_short.txt
```

These become inputs to the final WhiteSheet stage.

---

# 15. WHITESHEET PART 2

WhiteSheet Part 2 consumes:

```text
different_part2.txt
```

and generates UPS patches for the resources identified by Necronomicon Part 2.

The final relationship is:

```text
Crossbow Part 2
      │
      ▼
Distribute/
      │
      ▼
Necronomicon Part 2
      │
      ▼
different_part2.txt
      │
      ▼
WhiteSheet Part 2
      │
      ▼
Final UPS patches
```

This is effectively the distribution counterpart of the normal:

```text
Necronomicon
      ↓
different_hashes.txt
      ↓
ElectroHammer
```

relationship.

---

# 🧠 THE MOST IMPORTANT PIPELINE CONTRACTS

TextTools works because individual tools produce metadata that later tools consume.

These files should therefore be treated as **pipeline contracts**.

---

## `different_lines.txt`

Produced by:

```text
MarkerStone
```

Purpose:

```text
Line-count mismatch information
```

Used to identify structural inconsistencies.

---

## `different_hashes.txt`

Produced by:

```text
Necronomicon
```

Consumed by:

```text
HydraulicPress
Crossbow
ElectroHammer
```

Purpose:

```text
Identify changed script files
```

---

## `different_hash_folders.txt`

Produced by:

```text
Necronomicon
```

Consumed especially by:

```text
ElectroHammer
```

Purpose:

```text
Identify changed TextTools folders
```

---

## `percentage_res.txt`

Produced by:

```text
PoolRules
```

Purpose:

```text
Translation progress statistics
```

This is primarily analytical rather than a direct compilation dependency.

---

## `file_copied.txt`

Produced by:

```text
Crossbow
```

Purpose:

```text
Record distribution-copy operations
```

---

## `different_part2.txt`

Produced by:

```text
Necronomicon part2
```

Consumed by:

```text
WhiteSheet part2
```

Purpose:

```text
Identify changed distribution assets
```

---

## `different_part2_short.txt`

Produced by:

```text
Necronomicon part2
```

Purpose:

```text
Shortened representation of Part 2 changes
```

---

# 📂 EXPECTED DGRV3 REPOSITORY

TextTools expects a repository named:

```text
DGRV3/
```

A typical layout is:

```text
DGRV3/
│
├── ainori/
├── base_spc/
├── chapter1/
├── chapter2/
├── chapter3/
├── chapter4/
├── chapter5/
├── chapter6/
├── epilogue/
├── gallery/
├── game_resident/
├── i18n/
├── MapObjName/
├── prologue/
├── subroutine/
└── test/
```

The recognized script folders are:

```text
test
subroutine
prologue
MapObjName
game_resident
gallery
epilogue
chapter6
chapter5
chapter4
chapter3
chapter2
chapter1
ainori
```

The same general folder set is important to multiple tools.

In particular:

* MarkerStone
* Necronomicon
* ElectroHammer
* Crossbow
* PoolRules

must remain conceptually synchronized.

---

# 📁 `DGRV3_EN`

The English/reference repository is:

```text
DGRV3_EN/
```

It provides the baseline against which translated scripts are checked.

It is primarily consumed by:

```text
MarkerStone
Necronomicon
PoolRules
```

Conceptually:

```text
DGRV3_EN/
     │
     ├── structural reference
     │        ↓
     │    MarkerStone
     │
     ├── byte reference
     │        ↓
     │    Necronomicon
     │
     └── translation reference
              ↓
           PoolRules
```

---

# 📦 `base_spc`

`DGRV3/base_spc/` contains the SPC archives and supporting tools required for compilation.

It may contain archives such as:

```text
7za
danganronpa_spc_legacy.7z
danganronpa_spc_switch.7z
danganronpa_spc_xbox.7z
gr_font.7z
i18n_switch.7z
i18n_xbox.7z
NewSTXTool.7z
NewSTXTool_Linux.7z
SPCTool.7z
SPCTool_Linux.7z
STXTool.7z
STXTool_Linux.7z
trial_english.7z
trial_french.7z
ups
ups.exe
```

The exact subset required depends on the platform and pipeline stage.

These files are external dependencies and are not treated as ordinary translated source files.

---

# 🧾 `vars.txt` / `vars_bak.txt`

HydraulicPress requires a variable definition file.

The normal lookup is:

```text
vars.txt
```

with:

```text
vars_bak.txt
```

serving as a fallback.

These files define the project's variable substitutions and are therefore part of the text-compilation environment.

A missing or invalid variable file can cause HydraulicPress to produce incomplete or incorrect output.

---

# ⚙️ TEXTCONFIG.CONFIG

The central configuration file is:

```text
TextConfig.config
```

It is shared by the TextTools ecosystem.

It controls behavior such as:

* platform selection,
* repository cloning behavior,
* character replacement,
* variable processing,
* distribution behavior,
* STX-tool selection,
* and other pipeline options.

The exact set of configuration keys should always be verified against the source code.

The important architectural principle is:

```text
TextConfig.config
       │
       ▼
individual TextTools components
```

rather than each executable having an entirely independent configuration system.

---

# 🖥️ PLATFORM SUPPORT

TextTools has platform-specific behavior.

At minimum, the pipeline distinguishes between:

```text
PC
Xbox
Nintendo Switch
```

Some components share implementations between platforms while others have explicit platform-specific behavior.

For example:

* PC and Xbox use the SPC-oriented workflow.
* Switch can process `.pb` resources and Switch-specific localization assets.
* Character filtering may differ by platform.
* CrammedPiranhas selects different SPC archives.
* HydraulicPress applies platform-specific text filtering.

Therefore:

> **Do not assume that a successful PC build implies an equivalent Switch build.**

---

# 🔤 FILE TYPES

TextTools primarily processes:

```text
.txt
```

script files.

Depending on platform and stage it may also process:

```text
.pb
.spc
.stx
.srdv
```

These extensions do not all represent the same layer of the pipeline.

A useful abstraction is:

```text
.txt
 │
 │ editable / processed script
 ▼
.stx
 │
 │ compiled text resource
 ▼
.spc
 │
 │ game archive
 ▼
UPS
 │
 │ distribution patch
 ▼
end user
```

Switch-specific workflows can additionally involve:

```text
.pb
```

resources.

---

# 🧠 INCREMENTAL BUILD DESIGN

One of the most important characteristics of TextTools is that it does not need to blindly rebuild everything.

The incremental model is:

```text
English reference
       │
       ▼
Necronomicon
       │
       ▼
Find changed files
       │
       ▼
different_hashes.txt
       │
       ├──────────────┐
       ▼              ▼
HydraulicPress    ElectroHammer
       │              │
       │              ▼
       │        compile changed
       │          resources
       ▼
process changed
scripts
```

This is especially important when working with thousands of files.

A full rebuild can be forced in tools that support an `--all` option, but incremental processing is the intended optimization.

---

# 🧪 FULL BUILD VS INCREMENTAL BUILD

## Incremental build

The normal workflow is:

```text
English/reference
       ↓
Necronomicon
       ↓
change lists
       ↓
process only relevant data
```

Advantages:

* faster builds,
* less unnecessary SPC modification,
* less disk I/O,
* easier iteration.

---

## Full build

A full build ignores some change-selection metadata and processes everything relevant.

This is useful for:

* debugging,
* validating a clean build,
* recovering from stale metadata,
* testing compiler behavior,
* and verifying that incremental processing is not hiding a problem.

The exact full-build flag is tool-specific.

For example, ElectroHammer supports:

```text
Electrohammer.exe --all
```

---

# 🧹 CLEAN BUILD PRINCIPLE

The safest way to run TextTools is from a clean, known state.

Conceptually:

```text
Clean repository
       ↓
Ropeway
       ↓
CrammedPiranhas
       ↓
MarkerStone
       ↓
Necronomicon
       ↓
Bugvac
       ↓
HydraulicPress
       ↓
PoolRules
       ↓
ElectroHammer
       ↓
Crossbow
       ↓
Distribution stages
```

Avoid manually modifying generated SPCs and then expecting the incremental system to understand those changes.

The change-detection metadata describes the state of the source repository.

It does not magically understand arbitrary modifications made afterward.

---

# 🚨 DESTRUCTIVE OPERATIONS

TextTools is not a read-only analysis suite.

Various stages can:

* overwrite processed scripts,
* create baked files,
* modify SPC archives,
* extract archives over existing directories,
* delete temporary files,
* create distribution directories,
* rename distribution resources,
* and generate/overwrite UPS patches.

In particular:

```text
HydraulicPress
ElectroHammer
Crossbow
WhiteSheet
```

should be treated as potentially destructive build tools.

Always maintain an untouched copy of:

```text
DGRV3/
DGRV3_EN/
base_spc/
```

when developing or debugging.

---

# 🔗 TOOL RESPONSIBILITY MATRIX

| Responsibility         | StackedBooks | HiddenDoor | Ropeway | CrammedPiranhas | MarkerStone | Necronomicon | Bugvac | HydraulicPress | PoolRules | ElectroHammer | Crossbow | WhiteSheet |   |
| ---------------------- | :----------: | :--------: | :-----: | :-------------: | :---------: | :----------: | :----: | :------------: | :-------: | :-----------: | :------: | :--------: | - |
| Launch pipeline        |       ✅      |            |         |                 |             |              |        |                |           |               |          |            |   |
| Orchestrate stages     |              |      ✅     |         |                 |             |              |        |                |           |               |          |            |   |
| Acquire repository     |              |            |    ✅    |                 |             |              |        |                |           |               |          |            |   |
| Extract tools/assets   |              |            |         |        ✅        |             |              |        |                |           |               |          |            |   |
| Line validation        |              |            |         |                 |      ✅      |              |        |                |           |               |          |            |   |
| SHA change detection   |              |            |         |                 |             |       ✅      |        |                |           |               |          |            |   |
| Name replacement       |              |            |         |                 |             |              |    ✅   |                |           |               |          |            |   |
| Variable processing    |              |            |         |                 |             |              |        |                |     ✅     |               |          |            |   |
| Character filtering    |              |            |         |                 |             |              |        |                |     ✅     |               |          |            |   |
| Translation statistics |              |            |         |                 |             |              |        |                |           |       ✅       |          |            |   |
| TXT → STX              |              |            |         |                 |             |              |        |                |           |               |     ✅    |            |   |
| STX → SPC              |              |            |         |                 |             |              |        |                |           |               |     ✅    |            |   |
| Select modified SPCs   |              |            |         |                 |             |              |        |                |           |               |          |      ✅     |   |
| Generate UPS patches   |              |            |         |                 |             |              |        |                |           |               |          |      ✅     |   |

---

# 🧬 DATA FLOW SUMMARY

The complete normal pipeline can be represented as:

```text
                 ┌──────────────────┐
                 │  DGRV3 Git Repo  │
                 └────────┬─────────┘
                          │
                          ▼
                      Ropeway
                          │
                          ▼
                 ┌──────────────────┐
                 │     DGRV3/       │
                 └────────┬─────────┘
                          │
                          ▼
                 CrammedPiranhas
                          │
                          ▼
                Tools + Base SPCs
                          │
                          ▼
                    MarkerStone
                          │
                          ▼
                 different_lines.txt
                          │
                          ▼
                   Necronomicon
                          │
             ┌────────────┴────────────┐
             ▼                         ▼
   different_hashes.txt      different_hash_folders.txt
             │                         │
             └────────────┬────────────┘
                          ▼
                        Bugvac
                          │
                          ▼
                  HydraulicPress
                          │
             ┌────────────┼─────────────┐
             ▼            ▼             ▼
        Variables      Platform     Characters
             │            │             │
             └────────────┼─────────────┘
                          ▼
                     PoolRules
                          │
                          ▼
                   ElectroHammer
                          │
                          ▼
                       .STX
                          │
                          ▼
                    SpcTool
                          │
                          ▼
                 Modified SPCs
                          │
                          ▼
                      Crossbow
                          │
                          ▼
                  ModifiedFiles/
```

---

# 📦 DISTRIBUTION DATA FLOW

The distribution branch is:

```text
Modified SPCs
      │
      ▼
   Crossbow
      │
      ▼
ModifiedFiles/
      │
      ▼
CrammedPiranhas part2
      │
      ▼
Distribute/
      │
      ▼
 WhiteSheet
      │
      ▼
 UPS patches
```

The Part 2 continuation adds:

```text
Crossbow Part2
      │
      ▼
Necronomicon Part2
      │
      ▼
different_part2.txt
      │
      ▼
WhiteSheet Part2
      │
      ▼
Additional UPS patches
```

---

# 🧠 WHAT EACH MAJOR STAGE IS REALLY ANSWERING

This is perhaps the easiest way to understand TextTools.

### Ropeway

> **Where is the source repository?**

### CrammedPiranhas

> **Do we have the tools and base resources required to build it?**

### MarkerStone

> **Is the translation structurally compatible with the reference?**

### Necronomicon

> **What actually changed?**

### Bugvac

> **Which character-name transformations must be applied?**

### HydraulicPress

> **How should the editable translation be transformed into final script data?**

### PoolRules

> **How complete is the translation?**

### ElectroHammer

> **How do we turn the processed script into game resources?**

### Crossbow

> **Which generated resources should be distributed?**

### WhiteSheet

> **How do we turn those modifications into patches?**

---

# 🔄 WHY THE ORDER MATTERS

The pipeline order is not arbitrary.

For example, running:

```text
ElectroHammer
```

before:

```text
HydraulicPress
```

would mean compiling text before the variable/platform transformations have necessarily been applied.

Likewise, running:

```text
WhiteSheet
```

before:

```text
Crossbow
```

would deprive it of the intended modified-resource selection.

And running:

```text
ElectroHammer
```

without valid Necronomicon metadata can cause incremental compilation to omit resources that should have been rebuilt.

The pipeline is therefore best understood as a chain of contracts:

```text
Stage N
   │
   ├── produces files
   ├── produces metadata
   └── establishes assumptions
              │
              ▼
           Stage N+1
```

---

# 📊 TEXTTOOLS VS. A SIMPLE TEXT EDITOR

A normal text editor performs:

```text
open file
   ↓
edit file
   ↓
save file
```

TextTools performs:

```text
repository acquisition
       ↓
archive preparation
       ↓
structural validation
       ↓
cryptographic change detection
       ↓
name processing
       ↓
variable compilation
       ↓
platform filtering
       ↓
translation statistics
       ↓
STX compilation
       ↓
SPC insertion
       ↓
resource selection
       ↓
binary patch generation
```

That is why TextTools should be treated as a **build system**, not a text editor.

---

# 🛠️ EXTERNAL DEPENDENCIES

TextTools relies on external binary utilities.

Depending on platform and configuration, these can include:

```text
7za
SPCTool
STXTool
NewSTXTool / DRV3_STX_TOOL
UPS
```

Additional archives provide:

```text
SPC files
fonts
Switch localization resources
trial resources
```

These dependencies are intentionally treated separately from the core TextTools source.

Do not assume that cloning the source repository alone produces a complete runnable build environment.

---

# 🖥️ WINDOWS / LINUX

TextTools contains platform-specific external tools.

Examples include:

```text
7za.exe
SPCTool.exe
STXTool.exe
ups.exe
```

and Linux equivalents such as:

```text
7za
SPCTool
STXTool
ups
```

The correct tool must be available for the operating system on which the pipeline is being executed.

---

# 📄 IMPORTANT GENERATED FILES

| File                         | Producer           | Purpose                          |
| ---------------------------- | ------------------ | -------------------------------- |
| `different_lines.txt`        | MarkerStone        | Line-count mismatches            |
| `different_hashes.txt`       | Necronomicon       | Changed script files             |
| `different_hash_folders.txt` | Necronomicon       | Changed folder indexes           |
| `sha_diff.txt`               | Necronomicon       | Detailed hash comparisons        |
| `percentage_res.txt`         | PoolRules          | Translation statistics           |
| `variablechecker.txt`        | HydraulicPress     | Variable diagnostics             |
| `var_replace_map.txt`        | HydraulicPress     | Variable replacement information |
| `eh_report.txt`              | ElectroHammer      | Compilation manifest             |
| `file_copied.txt`            | Crossbow           | Distribution copy log            |
| `different_part2.txt`        | Necronomicon Part2 | Changed distribution resources   |
| `different_part2_short.txt`  | Necronomicon Part2 | Shortened Part2 change list      |

Individual tools may generate additional temporary or diagnostic files.

---

# 🚨 TROUBLESHOOTING PHILOSOPHY

When something goes wrong, **debug the earliest failed stage first**.

Do not begin by blaming the final output.

Use this model:

```text
Repository wrong?
       ↓
Check Ropeway

Assets/tools missing?
       ↓
Check CrammedPiranhas

Line counts wrong?
       ↓
Check MarkerStone

Wrong files considered changed?
       ↓
Check Necronomicon

Names wrong?
       ↓
Check Bugvac

Variables/platform filtering wrong?
       ↓
Check HydraulicPress

Statistics wrong?
       ↓
Check PoolRules

STX missing/wrong?
       ↓
Check ElectroHammer

Wrong SPCs selected?
       ↓
Check Crossbow

UPS wrong/missing?
       ↓
Check WhiteSheet
```

This avoids wasting time debugging downstream behavior caused by an upstream failure.

---

# 🧪 RECOMMENDED DEBUGGING WORKFLOW

When developing or modifying TextTools, the safest approach is to verify each major boundary.

## Step 1 — Repository

Verify:

```text
DGRV3/
DGRV3_EN/
```

exist and contain the expected folders.

---

## Step 2 — Base assets

Verify:

```text
DGRV3/base_spc/
```

contains the required tools and archives.

---

## Step 3 — Structural validation

Run:

```text
MarkerStone
```

and inspect:

```text
different_lines.txt
```

---

## Step 4 — Change detection

Run:

```text
Necronomicon
```

and inspect:

```text
different_hashes.txt
different_hash_folders.txt
```

---

## Step 5 — Text processing

Run:

```text
Bugvac
HydraulicPress
```

and inspect the resulting script files and diagnostics.

---

## Step 6 — Translation analysis

Run:

```text
PoolRules
```

and inspect:

```text
percentage_res.txt
```

---

## Step 7 — Compilation

Run:

```text
ElectroHammer
```

and inspect:

```text
eh_report.txt
```

and the generated SPC archives.

---

## Step 8 — Distribution

Run:

```text
Crossbow
```

and verify:

```text
ModifiedFiles/
```

---

## Step 9 — Patch generation

Only after the modified-resource set is correct should you run:

```text
WhiteSheet
```

---

# 🧹 STALE METADATA

One particularly important debugging issue is stale change-detection metadata.

For example:

```text
different_hashes.txt
```

describes the result of a particular Necronomicon run.

If the source repository changes afterward, that file may no longer describe reality.

The same applies to:

```text
different_hash_folders.txt
different_part2.txt
```

Therefore:

> **Do not blindly reuse old change lists after changing the source repository.**

Regenerate them when debugging suspicious incremental behavior.

---

# 🧨 FULL REBUILD AS A DIAGNOSTIC

If an incremental build produces unexpected results, perform a full rebuild where supported.

For example:

```text
Electrohammer.exe --all
```

If:

```text
--all
```

produces the expected result but the incremental build does not, the problem is probably in:

```text
different_hashes.txt
different_hash_folders.txt
```

rather than in the actual STX/SPC compiler.

This distinction can save a considerable amount of debugging time.

---

# 🧭 DESIGN PHILOSOPHY

TextTools follows several major design principles.

## Automation

Repeated operations are automated rather than performed manually.

---

## Incremental Processing

Only changed content should need to be rebuilt when the necessary metadata is available.

---

## Deterministic Pipeline Stages

Each tool has a defined role and produces outputs consumed by later tools.

---

## Separation of Analysis and Compilation

Tools such as:

```text
MarkerStone
Necronomicon
PoolRules
```

analyze the translation.

Tools such as:

```text
HydraulicPress
ElectroHammer
```

transform and compile it.

Tools such as:

```text
Crossbow
WhiteSheet
```

prepare it for distribution.

---

## Reproducibility

The intended build model is:

```text
Same source
+
Same configuration
+
Same base resources
+
Same tool versions
=
Equivalent build
```

In practice, exact binary reproducibility also depends on the versions and behavior of external tools.

For release builds, preserve the exact external dependencies used.

---

# 📚 INDIVIDUAL TOOL DOCUMENTATION

This README is intentionally the **high-level architectural document**.

Each major component has its own dedicated README containing implementation details.

| Tool                | Documentation Role             |
| ------------------- | ------------------------------ |
| **StackedBooks**    | Top-level launcher             |
| **HiddenDoor**      | Pipeline orchestrator          |
| **Ropeway**         | Repository management          |
| **CrammedPiranhas** | Asset/tool extraction          |
| **MarkerStone**     | Line-count validation          |
| **Necronomicon**    | SHA-512 change detection       |
| **Bugvac**          | Character-name processing      |
| **HydraulicPress**  | Variable/script compilation    |
| **LooseFloorboard** | Character compatibility helper |
| **PoolRules**       | Translation statistics         |
| **ElectroHammer**   | STX/SPC compilation            |
| **Crossbow**        | Modified-resource distribution |
| **WhiteSheet**      | UPS patch generation           |

The individual READMEs should be consulted before modifying a specific component.

---

# 📋 RESPONSIBILITY SUMMARY

```text
STACKEDBOOKS
    │
    └── Launch HiddenDoor

HIDDENDOOR
    │
    └── Run pipeline

ROPEWAY
    │
    └── Acquire repository

CRAMMEDPIRANHAS
    │
    └── Prepare tools + base resources

MARKERSTONE
    │
    └── Validate line structure

NECRONOMICON
    │
    └── Detect byte-level changes

BUGVAC
    │
    └── Process character names

HYDRAULICPRESS
    │
    └── Compile/process script text

POOLRULES
    │
    └── Measure translation progress

ELECTROHAMMER
    │
    └── TXT → STX → SPC

CROSSBOW
    │
    └── Select modified resources

CRAMMEDPIRANHAS PART2
    │
    └── Prepare distribution references

WHITESHEET
    │
    └── Generate UPS patches

CROSSBOW PART2
    │
    └── Prepare special distribution resources

NECRONOMICON PART2
    │
    └── Detect binary distribution changes

WHITESHEET PART2
    │
    └── Generate final Part2 UPS patches
```

---

# 🏁 COMPLETE PIPELINE AT A GLANCE

```text
                         TEXTTOOLS
                             │
                             ▼
                       StackedBooks
                             │
                             ▼
                        HiddenDoor
                             │
                             ▼
                          Ropeway
                             │
                             ▼
                    CrammedPiranhas
                             │
                             ▼
                        MarkerStone
                             │
                             ▼
                       Necronomicon
                             │
                             ▼
                           Bugvac
                             │
                             ▼
                      HydraulicPress
                             │
                             ▼
                         PoolRules
                             │
                             ▼
                      ElectroHammer
                             │
                             ▼
                         Crossbow
                             │
                      DISTRIBUTE?
                       /          \
                     NO            YES
                     │              │
                     ▼              ▼
                   DONE      CrammedPiranhas
                                    │
                                    ▼
                               WhiteSheet
                                    │
                                    ▼
                                Crossbow
                                  Part2
                                    │
                                    ▼
                              Necronomicon
                                  Part2
                                    │
                                    ▼
                               WhiteSheet
                                  Part2
                                    │
                                    ▼
                              FINAL UPS
```

---

# 🧠 THE ENTIRE SYSTEM IN ONE DIAGRAM

```text
┌─────────────────────────────────────────────────────────────┐
│                        SOURCE DATA                           │
│                                                             │
│   DGRV3/                 DGRV3_EN/          base_spc/       │
│   translated scripts    reference scripts  base resources  │
└───────────────┬──────────────┬──────────────────┬───────────┘
                │              │                  │
                └──────────────┼──────────────────┘
                               ▼
                         MarkerStone
                               │
                     structural validation
                               │
                               ▼
                        Necronomicon
                               │
                       byte-level diff
                               │
               ┌───────────────┴───────────────┐
               ▼                               ▼
     different_hashes.txt          different_hash_folders.txt
               │                               │
               └───────────────┬───────────────┘
                               ▼
                           Bugvac
                               │
                               ▼
                      HydraulicPress
                               │
                   variables/platform/etc.
                               │
                               ▼
                         PoolRules
                               │
                         statistics
                               │
                               ▼
                       ElectroHammer
                               │
                           TXT → STX
                               │
                           STX → SPC
                               │
                               ▼
                         Modified SPC
                               │
                               ▼
                           Crossbow
                               │
                               ▼
                      ModifiedFiles/
                               │
                               ▼
                         WhiteSheet
                               │
                               ▼
                           UPS PATCH
```

---

# 📦 FINAL OUTPUT MODEL

TextTools ultimately produces one of two useful things.

## Build output

```text
DGRV3/base_spc/
    └── modified game SPC resources
```

These are the actual rebuilt game resources.

---

## Distribution output

```text
Distribute/
    └── *.ups
```

These are the patches intended to distribute the changes without distributing complete original game resources.

Therefore:

```text
BUILD
    ↓
modified SPC/PB resources

DISTRIBUTION
    ↓
UPS patches
```

These are two different goals.

---

# 🏁 SUMMARY

**TextTools is Team DAIX's automated script-processing and game-resource build system for Danganronpa V3.**

Its fundamental workflow is:

```text
Acquire
   ↓
Prepare
   ↓
Validate
   ↓
Detect changes
   ↓
Transform text
   ↓
Measure translation
   ↓
Compile
   ↓
Inject
   ↓
Select modified resources
   ↓
Patch
```

The most important relationships are:

```text
MarkerStone
    ↓
structural validation
```

```text
Necronomicon
    ↓
different_hashes.txt
    ↓
HydraulicPress + ElectroHammer
```

```text
HydraulicPress
    ↓
processed script
    ↓
ElectroHammer
```

```text
ElectroHammer
    ↓
modified SPC
    ↓
Crossbow
```

```text
Crossbow
    ↓
distribution resources
    ↓
WhiteSheet
    ↓
UPS
```

and for the second distribution phase:

```text
Crossbow Part2
    ↓
Necronomicon Part2
    ↓
different_part2.txt
    ↓
WhiteSheet Part2
    ↓
additional UPS patches
```

The individual tools are deliberately specialized.

```text
Ropeway          → obtain
CrammedPiranhas  → prepare
MarkerStone      → validate
Necronomicon     → detect
Bugvac           → transform names
HydraulicPress   → compile text
PoolRules        → measure
ElectroHammer    → build resources
Crossbow         → select
WhiteSheet       → patch
```

And the system tying them together is:

```text
                    HiddenDoor
                         │
                         ▼
                deterministic pipeline
                         │
        ┌────────────────┼────────────────┐
        ▼                ▼                ▼
      ANALYZE          BUILD          DISTRIBUTE
        │                │                │
        ▼                ▼                ▼
 MarkerStone        HydraulicPress     Crossbow
 Necronomicon       ElectroHammer      WhiteSheet
 PoolRules                               │
                                        ▼
                                       UPS
```

TextTools is therefore best understood not as a set of independent utilities, but as a **translation build system**.

Its goal is to turn:

```text
human-authored translation
```

into:

```text
validated,
processed,
compiled,
game-ready,
and distributable
Danganronpa V3 resources.
```

If you cannot read the code, this repository is not for you.

If you can:

> **Welcome to TextTools.**