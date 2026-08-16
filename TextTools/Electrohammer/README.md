# TEXTTOOLS - ELECTROHAMMER
## Team DAIX, 2026  

---

## 📌 Overview

**ElectroHammer** is the TextTools component responsible for turning processed Danganronpa V3 script files into **game-ready `.SPC` archives**.

It is the compilation and injection stage of the TextTools pipeline.

At a high level, ElectroHammer takes:

```text
DGRV3/*.txt
````

and turns the relevant files into:

```text
.stx
```

resources, which are then inserted into their corresponding:

```text
.SPC
```

archives.

The complete transformation is:

```text
Translated .txt
      │
      ▼
   STX tool
      │
      ▼
   .stx file
      │
      ▼
   SPC tool
      │
      ▼
Updated .spc archive
```

ElectroHammer is designed to avoid recompiling the entire repository unnecessarily. It consumes the change information produced by **Necronomicon**, determines which folders and files have actually changed, and compiles only those entries unless full compilation has been requested.

The source describes ElectroHammer as the final stage that produces the actual game-ready SPC archives. 

---

# 🎯 Responsibilities

ElectroHammer has two major subsystems:

```text
Entry Management
       +
Compilation
```

### Entry Management decides:

* which files need compilation,
* which files should be ignored,
* which files were changed,
* which TextTools folder a file belongs to,
* which `.SPC` archive receives the file,
* and how entries should be grouped.

### The Compiler decides:

* how `.txt` becomes `.stx`,
* which STX pipeline to use,
* how `.stx` files are inserted into `.spc`,
* how compilation is parallelized,
* and which additional font resources must be injected.

This separation is important:

```text
EntryManagement = WHAT should be compiled?
Compiler        = HOW should it be compiled?
```

---

# 🧩 How ElectroHammer Fits Into TextTools

ElectroHammer sits near the end of the TextTools build pipeline.

A simplified pipeline is:

```text
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
Game-ready SPC archives
```

The exact order of analytical tools can vary during development, but ElectroHammer is fundamentally a **build/output stage**, not a translation-analysis stage.

Its job begins once the script files have reached the state that should actually be injected into the game.

---

# 🔗 Relationship With Necronomicon

ElectroHammer directly consumes two files associated with Necronomicon:

```text
different_hashes.txt
different_hash_folders.txt
```

These files allow ElectroHammer to perform an incremental build.

The intended relationship is:

```text
Necronomicon
     │
     ├── different_hashes.txt
     │
     └── different_hash_folders.txt
                │
                ▼
          ElectroHammer
                │
                ▼
       Compile only changed data
```

ElectroHammer reads these files during startup and passes them to `EntryMG::CalculateEntries()`. 

This is one of the most important performance features of the program.

---

# 🏗️ High-Level Architecture

ElectroHammer consists primarily of:

```text
Electrohammer.cpp
        │
        ├── Program startup
        ├── Config loading
        ├── Repository validation
        ├── Entry calculation
        ├── Compilation
        └── Report generation
                │
                ▼
        entrymanagement.cpp
                │
                ├── Folder selection
                ├── File selection
                ├── SPC mapping
                └── Entry creation
                        │
                        ▼
                 compile.cpp
                        │
                        ├── TXT → STX
                        ├── STX → SPC
                        ├── Parallel compilation
                        └── Font injection
```

The `Entry` structure carries the information required to connect these stages. It stores the source filename, working directories, generated STX destination, and target SPC archive. 

---

# 📁 Required Directory Layout

ElectroHammer assumes it is executed from the TextTools working directory.

It expects:

```text
DGRV3/
```

and specifically:

```text
DGRV3/base_spc/
```

The startup code constructs:

```text
current directory
    └── DGRV3
          └── base_spc
```

and aborts if either required location is missing. 

---

# 📂 `DGRV3/`

This is the translated script repository.

ElectroHammer recursively scans it looking for candidate entries.

The actual script files are expected to be located underneath the recognized TextTools folders.

---

# 📦 `DGRV3/base_spc/`

This is the destination containing the SPC archives that ElectroHammer modifies.

For example:

```text
DGRV3/base_spc/
├── chap1_text_US.SPC
├── chap2_text_US.SPC
├── game_resident_US.spc
└── ...
```

ElectroHammer passes these archive paths to `SpcTool` when inserting generated STX files. 

---

# ⚙️ Configuration

ElectroHammer reads:

```text
TextConfig.config
```

from the current working directory.

Startup performs:

```text
Configuration::ReadConfig(...)
Configuration::ViewDebugCurrentConfig()
```

before determining which files should be compiled. 

The most important configuration value for ElectroHammer is:

```text
CheckAllFiles
```

---

# 🟢 `CheckAllFiles`

When:

```text
CheckAllFiles = true
```

ElectroHammer ignores the normal incremental file selection logic and considers every recognized file eligible for compilation.

The same setting can be forced from the command line with:

```text
--all
```

The program scans its arguments and enables:

```text
Configuration::ConfigMap["CheckAllFiles"] = true;
```

if an argument contains `--all`. 

---

# 🚀 Command-Line Usage

The significant command-line option is:

```text
--all
```

Example:

```text
Electrohammer.exe --all
```

This forces full compilation.

The option is particularly useful when:

* Necronomicon output is missing,
* all files need to be rebuilt,
* an archive may be stale,
* debugging the build process,
* or testing a complete release build.

---

# ⚠️ `--all` Matching Behavior

The source does not perform an exact argument comparison.

It checks whether an argument **contains**:

```text
--all
```

Therefore, the implementation is effectively substring-based.

The intended usage remains simply:

```text
--all
```

---

# 🗂️ Recognized Script Folders

ElectroHammer recognizes exactly 14 folder names:

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

These are stored in the static `folders` array. 

Any file whose path does not contain one of these recognized folder names is ignored.

---

# 📦 Folder → SPC Mapping

Each recognized folder has a corresponding target SPC archive.

The mapping is positional.

| Folder          | SPC archive                |
| --------------- | -------------------------- |
| `test`          | `test_text_US.SPC`         |
| `subroutine`    | `sub_routine_text_US.SPC`  |
| `prologue`      | `chap0_text_US.SPC`        |
| `MapObjName`    | `map_obj_name_text_US.SPC` |
| `game_resident` | `game_resident_US.spc`     |
| `gallery`       | `gallery_text_US.SPC`      |
| `epilogue`      | `chap7_text_US.SPC`        |
| `chapter6`      | `chap6_text_US.SPC`        |
| `chapter5`      | `chap5_text_US.SPC`        |
| `chapter4`      | `chap4_text_US.SPC`        |
| `chapter3`      | `chap3_text_US.SPC`        |
| `chapter2`      | `chap2_text_US.SPC`        |
| `chapter1`      | `chap1_text_US.SPC`        |
| `ainori`        | `ainori_text_US.SPC`       |

The source defines all 14 entries explicitly in the `fti` array. 

---

# 🔎 How Folder Detection Works

ElectroHammer does not derive the folder using only the immediate parent directory.

Instead, `GetIndex()` searches the complete path for each recognized folder name.

Conceptually:

```text
DGRV3/chapter3/scene_12.txt
        │
        ▼
contains "chapter3"
        │
        ▼
folder index = chapter3's index
        │
        ▼
chap3_text_US.SPC
```

The source uses `Common::StringContains()` for this lookup. 

---

# ⚠️ Folder Detection Quirk

Because folder detection is substring-based, the implementation is less strict than a true path-component comparison.

It searches for folder names **inside the complete path**.

Therefore, developers should avoid introducing unrelated directory names containing one of the recognized folder strings.

---

# 📄 What Files Are Compiled?

ElectroHammer ultimately accepts only files whose path contains:

```text
.txt
```

It rejects several categories of files.

The filtering rules exclude:

```text
_sha
_lines
README
LICENSE
.git
Baked
vars_bak
.pb
```

These filters are explicitly implemented in `CheckEntry()`. 

---

# 🚫 Ignored Files

## SHA files

Anything containing:

```text
_sha
```

is ignored.

These are integrity/verification artifacts rather than script input.

---

## Line-count files

Anything containing:

```text
_lines
```

is ignored.

This prevents MarkerStone-generated line-count artifacts from being compiled as scripts.

---

## README files

Files containing:

```text
README
```

are ignored.

---

## LICENSE files

Files containing:

```text
LICENSE
```

are ignored.

---

## Git metadata

Paths containing:

```text
.git
```

are ignored.

---

## Baked output

Paths containing:

```text
Baked
```

are ignored.

This prevents generated/baked resources from being recursively fed back into the compilation process.

---

## Variable backup

The repository variable backup:

```text
vars_bak
```

is explicitly excluded.

This is important because it is a variable definition file rather than a script intended for STX compilation.

---

## `.pb` files

Switch `.pb` files are rejected immediately.

The source explicitly describes these as binary data that ElectroHammer does not compile. 

---

# 🧹 Incremental File Selection

ElectroHammer does not necessarily compile every valid `.txt`.

Instead, two independent change lists are used:

```text
different_hash_folders.txt
different_hashes.txt
```

These represent:

```text
changed folders
+
changed files
```

The default compilation rule is effectively:

```text
CheckAllFiles
OR
(
    folder changed
    AND
    filename changed
)
```

This is implemented with `optimize_for_size = false`. 

---

# 🧮 Why Both Lists Are Needed

The folder list provides coarse filtering:

```text
chapter3 changed
```

The file list provides fine filtering:

```text
scene_12.txt changed
```

Together they allow ElectroHammer to avoid touching unrelated files.

For example:

```text
different_hash_folders.txt
    10

different_hashes.txt
    DGRV3/chapter3/scene_12.txt
```

causes ElectroHammer to consider the corresponding chapter and file.

---

# 📄 `different_hash_folders.txt`

This file contains numeric folder indexes.

For example:

```text
10
```

represents the folder whose index is `10` in the internal `folders` array.

The file is read line-by-line and converted with:

```text
std::stoull()
```

Duplicate indexes are removed.

After reading, ElectroHammer deletes the file. 

---

# ⚠️ `different_hash_folders.txt` Is Consumed

This is an important behavior.

After successfully reading the folder-index file:

```text
different_hash_folders.txt
```

is removed.

Therefore, it is not simply a persistent cache.

It is a **one-shot handoff artifact** from the preceding pipeline stage.

---

# 📄 `different_hashes.txt`

This file contains paths to changed files.

ElectroHammer reads each line and checks whether the referenced path exists.

For valid entries, it stores only:

```text
filename()
```

rather than the full path.

The source explicitly states that `CalculateFilesToCompile()` returns filenames rather than complete paths. 

---

# ⚠️ Filename-Only Matching

This is an important implementation detail.

Suppose:

```text
chapter1/scene.txt
```

and:

```text
chapter2/scene.txt
```

both exist.

If `different_hashes.txt` contains one of them, `CalculateFilesToCompile()` stores only:

```text
scene.txt
```

The later comparison also uses only the filename.

Therefore, duplicate filenames across different folders can potentially cause more than one matching file to qualify if both folders are marked as changed.

The folder-level filter reduces this risk, but the file-level comparison itself is not path-unique.

---

# 📋 `Entry`

Every file selected for compilation becomes an:

```cpp
EntryMG::Entry
```

An entry contains:

```text
Filename
CurrentDir
WhereTo
STXFileToInsert
SPCFileToInsert
```

The initial entry creation fills:

```text
Filename
SPCFileToInsert
WhereTo
CurrentDir
```

The STX path is populated later by the compiler. 

---

# 🧩 Entry Lifecycle

An entry begins approximately as:

```text
Filename:
DGRV3/chapter3/scene_12.txt

SPCFileToInsert:
chap3_text_US.SPC

CurrentDir:
<working directory>

WhereTo:
DGRV3/base_spc/

STXFileToInsert:
<empty>
```

Then `Compile()` calculates:

```text
STXFileToInsert
```

and the entry becomes the complete description of the compilation operation.

---

# 🔄 Complete Entry Pipeline

The entry pipeline is:

```text
DGRV3/
   │
   ▼
recursive scan
   │
   ▼
CheckEntry()
   │
   ├── valid folder?
   ├── valid file?
   ├── changed?
   ├── blacklisted?
   └── valid SPC?
   │
   ▼
Entry
   │
   ▼
MoveTXTForSTXTool()
   │
   ▼
STX path
   │
   ▼
STX compilation
   │
   ▼
SPC insertion
```

---

# 🧰 `MoveTXTForSTXTool()`

This function prepares the paths used by the newer STX workflow.

Given:

```text
DGRV3/chapter3/scene_12.txt
```

it constructs:

```text
EXTRACTED_FILES/scene_12.txt
```

and:

```text
REPACKED_FILES/scene_12.stx
```

relative to the tool's working directory. 

---

# 📂 `EXTRACTED_FILES`

When copying is enabled, the source `.txt` is copied into:

```text
EXTRACTED_FILES/
```

using only its filename.

For example:

```text
DGRV3/chapter3/scene_12.txt
```

becomes:

```text
EXTRACTED_FILES/scene_12.txt
```

The copy uses:

```text
overwrite_existing
```

so an existing file with the same name is replaced.

---

# 📂 `REPACKED_FILES`

The expected output STX path is:

```text
REPACKED_FILES/scene_12.stx
```

The function does not rely on a general extension replacement utility.

Instead, it manually rewrites the last three characters:

```text
txt
```

into:

```text
stx
```

This behavior exists because of how the external STX tools produce their output. 

---

# ⚠️ Basename Flattening

`MoveTXTForSTXTool()` uses:

```text
std::filesystem::path(filename).filename()
```

before constructing the destination.

Therefore, directory structure is discarded.

For example:

```text
DGRV3/chapter3/subdir/foo.txt
```

becomes:

```text
EXTRACTED_FILES/foo.txt
```

not:

```text
EXTRACTED_FILES/chapter3/subdir/foo.txt
```

This means duplicate filenames can collide in `EXTRACTED_FILES` and `REPACKED_FILES`.

---

# 🔨 STX Compilation

ElectroHammer supports two STX-generation strategies:

```text
Legacy STX pipeline
New DRV3_STX_TOOL pipeline
```

The choice is controlled at compile time by:

```text
UseNewSTXTool
```

---

# 🕰️ Legacy STX Pipeline

The old pipeline uses:

```text
StxTool
```

for individual files.

The process is:

```text
.txt
 ↓
StxTool
 ↓
.stx
```

The executable is selected as:

```text
StxTool.exe
```

on Windows, and:

```text
StxTool
```

on non-Windows platforms. 

---

# ⚙️ `CalculateStx()`

`CalculateStx()` builds a command resembling:

```text
"StxTool" "scene_12.txt"
```

and executes it through:

```text
Common::executeBatch()
```

After execution, it assumes the STX file exists beside the original TXT and derives the output name by changing:

```text
txt
```

to:

```text
stx
```

The external tool therefore performs the actual conversion; ElectroHammer only orchestrates it. 

---

# 🆕 New STX Pipeline

The newer pipeline uses:

```text
DRV3_STX_TOOL
```

instead of invoking `StxTool` once per file.

Its process is:

```text
Selected .txt files
       │
       ▼
EXTRACTED_FILES/
       │
       ▼
DRV3_STX_TOOL
       │
       ▼
REPACKED_FILES/*.stx
```

The source explicitly describes this tool as processing all files at once. 

---

# 📦 New Pipeline Preparation

For every entry, ElectroHammer calls:

```text
MoveTXTForSTXTool(..., true)
```

which copies the source TXT into:

```text
EXTRACTED_FILES/
```

The new STX tool is then executed once for the entire entry collection.

---

# ⚠️ New STX Tool Is Not Actually Threaded

Even though `Compile()` has a multithreaded mode, the new STX pipeline does not launch one `DRV3_STX_TOOL` process per entry.

Instead:

```text
Move all files
     ↓
one DRV3_STX_TOOL invocation
```

The parallelization happens later, during SPC grouping.

The source even questions this internally with the comment:

```text
Is this even multithreaded?
```

The effective design is therefore:

```text
STX generation = batch operation
SPC injection  = parallelized by archive
```

---

# 📦 SPC Injection

Once an STX exists, ElectroHammer inserts it into the appropriate SPC archive using:

```text
SpcTool
```

The command is conceptually:

```text
SpcTool "target.spc" insert "file.stx"
```

The source constructs the command and passes it to:

```text
Common::executeBatch()
```

without performing the archive modification itself. 

---

# 🧭 SPC Target Resolution

Every entry carries:

```text
SPCFileToInsert
```

For example:

```text
chapter1
    ↓
chap1_text_US.SPC
```

and:

```text
game_resident
    ↓
game_resident_US.spc
```

The archive is resolved before compilation rather than discovered dynamically from the filesystem.

---

# 🗃️ Grouping by SPC

ElectroHammer uses:

```text
SplitType
```

which is:

```cpp
std::array<std::vector<Entry>, 14>
```

Each array slot represents one SPC archive.

Entries are grouped according to their `SPCFileToInsert`. 

---

# 🚀 Why Group by SPC?

SPC insertion is parallelized at the archive level.

Instead of:

```text
entry1 → SPC
entry2 → SPC
entry3 → SPC
...
```

all being treated as one serial stream, ElectroHammer creates groups:

```text
Group 0 → test_text_US.SPC
Group 1 → sub_routine_text_US.SPC
Group 2 → chap0_text_US.SPC
...
```

and processes different groups concurrently.

The source notes that this produces a substantial performance improvement compared with purely serial processing. 

---

# ⚡ Multithreaded STX Compilation

When the legacy pipeline is active and multiple hardware threads are available, ElectroHammer launches a detached `std::jthread` for each STX entry.

Conceptually:

```text
Entry 1 → thread
Entry 2 → thread
Entry 3 → thread
...
```

The program then waits for the atomic:

```text
STXCompiled
```

to reach the total entry count. 

---

# ⚡ Multithreaded SPC Compilation

After STX compilation is complete, entries are grouped by SPC.

Then one detached thread is launched per SPC group.

Conceptually:

```text
chapter1 entries
      │
      └── Thread A → chap1_text_US.SPC

chapter2 entries
      │
      └── Thread B → chap2_text_US.SPC

game_resident entries
      │
      └── Thread C → game_resident_US.spc
```

The program waits until:

```text
SPCCompiled
```

equals the number of groups. 

---

# 🧵 Single-Thread Fallback

ElectroHammer checks:

```cpp
std::thread::hardware_concurrency()
```

If the result is:

```text
0
```

or:

```text
1
```

it uses the single-thread path.

The source explicitly falls back to single-threaded compilation when no useful hardware concurrency is available. 

---

# ⚠️ Thread Synchronization

The multithreaded implementation uses atomic counters:

```text
STXCompiled
SPCCompiled
```

as completion counters.

The program waits using loops equivalent to:

```text
while (STXCompiled < entry_count);
```

and:

```text
while (SPCCompiled < group_count);
```

This is a busy-wait rather than a condition-variable-based synchronization mechanism.

The source also deliberately sleeps for 500 ms after STX compilation before starting SPC grouping, apparently as an additional safety margin. 

---

# ⚠️ External Tool Failures

ElectroHammer delegates actual compilation to:

```text
StxTool
DRV3_STX_TOOL
SpcTool
```

through:

```text
Common::executeBatch()
```

The shown compiler code does not provide detailed per-command return-code handling.

Therefore, the presence of an entry in the compilation pipeline does not by itself guarantee that the external tool succeeded.

Developers should verify the resulting STX/SPC files when diagnosing failed builds.

---

# 🔤 Manual Font Injection

After all normal SPC entries have been compiled, ElectroHammer performs an additional manual injection step.

It explicitly inserts:

```text
v3_font00.stx
v3_font00.srdv
```

into:

```text
game_resident_US.spc
```

The source states that these fonts are not part of the normal entry list and must be inserted separately. 

---

# 🖋️ Font Injection Flow

The final stage is:

```text
All normal SPCs compiled
          ↓
Create font entry
          ↓
v3_font00.stx
          ↓
Insert into game_resident_US.spc

          ↓

v3_font00.srdv
          ↓
Insert into game_resident_US.spc
```

This happens after the main SPC compilation has completed.

---

# ⚠️ Font Packaging Limitation

The source explicitly notes:

```text
FONTS IN THE 7Z/ZIP/RAR NEED TO BE UPDATED MANUALLY
```

Therefore, injecting the fonts into the SPC archive is not the entire font-distribution workflow.

External distribution packages containing those font files may still require manual updating.

---

# 🚫 Blacklist

ElectroHammer contains a configurable blacklist:

```cpp
blacklist
```

Its purpose is to prevent specific files from being compiled.

The current source has the blacklist entries commented out, including:

```text
SaveLoad
A-CharacterName
```

The comments explain why.

---

# ⚠️ `SaveLoad`

The source notes that `game_resident`'s `SaveLoad` has issues with STXTool and should remain excluded until the underlying repacking issue is fixed or the old tool is no longer used.

The entry is currently commented out rather than active. 

---

# ⚠️ `A-CharacterName`

The source also documents an issue involving:

```text
game_resident/A-CharacterName
```

Specifically, character names in the overworld can become blank when this resource is processed with STXTool.

Again, the blacklist entry is currently commented out.

This means the warning is preserved in the source as historical/development guidance, but the current build does not actively skip the file because of that entry.

---

# 🧾 Skipped Files

When a file is skipped because it matches an active blacklist entry, its path is appended to:

```text
skipped_files
```

At the end of the program, ElectroHammer prints those skipped paths to the console. 

This is useful for detecting resources intentionally excluded from a build.

---

# 📊 `eh_report.txt`

After compilation, ElectroHammer generates:

```text
eh_report.txt
```

in the current working directory.

If an old report exists, it is deleted first. 

---

# 📜 Report Format

Each compiled entry is represented as:

```text
TXT
    -->
STX
    -->
SPC
```

For example, conceptually:

```text
chapter3/scene_12.txt
    --> REPACKED_FILES/scene_12.stx
    --> base_spc/chap3_text_US.SPC
```

The report pads the columns to fixed widths for readability. 

---

# 🗺️ What `eh_report.txt` Represents

The report is essentially a build manifest.

It tells developers:

```text
Which TXT was processed?
        ↓
Which STX was generated?
        ↓
Which SPC received it?
```

This makes it useful when investigating incorrect archive placement.

---

# ⏱️ Build Timing

ElectroHammer starts a timer before entry calculation and compilation.

After completion, it reports:

```text
Creating STX && Inserting SPC took:
<N> seconds!
```

This provides a simple benchmark for incremental vs full builds. 

---

# 🛑 Missing `DGRV3`

If:

```text
DGRV3/
```

does not exist, ElectroHammer logs an error and exits with:

```text
-1
```

The check is performed before compilation begins. 

---

# 🛑 Missing `base_spc`

If:

```text
DGRV3/base_spc/
```

does not exist, ElectroHammer also aborts.

This is required because the SPC archives must be available as insertion targets. 

---

# ⚠️ Missing `different_hashes.txt`

If:

```text
different_hashes.txt
```

does not exist, ElectroHammer logs a warning.

`CalculateFilesToCompile()` then returns an empty list. 

Unless:

```text
--all
```

is active, this can prevent normal incremental file selection.

---

# ⚠️ Missing `different_hash_folders.txt`

This has a different behavior.

If:

```text
different_hash_folders.txt
```

does not exist, ElectroHammer warns that it will compile **every folder**.

It then waits five seconds:

```text
You have 5 seconds to abort...
```

before proceeding.

If no folder indexes are ultimately available, all 14 folder indexes are selected. 

---

# 🐢 Why Missing Folder Data Is Expensive

Compiling all folders is described by the source as:

```text
VERY SLOW
```

because the normal incremental optimization has been lost.

The five-second delay is therefore an intentional safety mechanism intended to give the developer an opportunity to cancel an unexpectedly expensive full build.

---

# 🧹 Consumption of Incremental Data

ElectroHammer treats:

```text
different_hash_folders.txt
```

as temporary build metadata.

It reads the file and then removes it.

By contrast:

```text
different_hashes.txt
```

is read but not removed by `CalculateFilesToCompile()`.

This distinction matters if another pipeline component expects either file to remain available after ElectroHammer runs.

---

# 🔄 Full Compilation Mode

When:

```text
CheckAllFiles = true
```

ElectroHammer effectively bypasses the incremental selection criteria.

The normal condition:

```text
folder changed
AND
filename changed
```

is replaced by:

```text
compile everything eligible
```

This is useful for clean rebuilds.

---

# 🧠 Incremental Compilation Logic

The normal logic can be represented as:

```text
For every filesystem entry:
        │
        ├── recognized folder?
        │       └── no → skip
        │
        ├── directory?
        │       └── yes → determine SPC, do not compile
        │
        ├── CheckAllFiles?
        │       └── yes → continue
        │
        ├── folder changed?
        │       └── no → skip
        │
        ├── filename changed?
        │       └── no → skip
        │
        ├── valid .txt?
        │       └── no → skip
        │
        ├── excluded artifact?
        │       └── yes → skip
        │
        ├── blacklisted?
        │       └── yes → skip
        │
        └── create Entry
```

This is the core decision-making mechanism of EntryManagement. 

---

# 📁 Recursive Scanning

The program uses:

```text
std::filesystem::recursive_directory_iterator
```

over:

```text
DGRV3/
```

Therefore, nested directories are traversed automatically.

However, only files that survive the complete filtering process become compilation entries.

---

# 🗂️ Directories Are Not Compilation Entries

`CheckEntry()` can encounter directories.

When it does, it uses the directory path to determine the corresponding SPC archive but returns:

```text
false
```

so the directory itself is not compiled.

Its purpose is essentially to establish the mapping context for the contained files. 

---

# 🔗 Entry → STX → SPC

The most important data relationship in ElectroHammer is:

```text
Entry.Filename
       │
       ▼
MoveTXTForSTXTool()
       │
       ▼
Entry.STXFileToInsert
       │
       ▼
SpcTool
       │
       ▼
Entry.SPCFileToInsert
```

The entry therefore acts as the bridge between source text and game archive.

---

# 🧱 Legacy Pipeline in Detail

When:

```text
UseNewSTXTool = false
```

ElectroHammer uses:

```text
CalculateAllOld<true, true>()
```

The function:

1. derives the `.stx` filename,
2. checks whether that STX already exists,
3. invokes `StxTool` if necessary,
4. invokes `SpcTool` to insert the STX.

The STX existence check means an already-generated STX can be reused during the legacy path. 

---

# 🆕 New Pipeline in Detail

When:

```text
UseNewSTXTool = true
```

ElectroHammer:

1. copies selected TXT files into `EXTRACTED_FILES`,
2. invokes `DRV3_STX_TOOL` once,
3. expects the resulting STX files under `REPACKED_FILES`,
4. inserts each STX into its assigned SPC.

This avoids invoking the legacy STX converter individually for every file. 

---

# 🔀 Pipeline Selection

The compiler uses compile-time branching:

```cpp
if constexpr (UseNewSTXTool)
```

rather than a runtime command-line selection.

Therefore, switching between the old and new STX workflows requires the corresponding build/configuration change in the source.

This is different from:

```text
--all
```

which is a runtime option.

---

# 🧵 Compilation Modes

There are effectively four combinations in the underlying templated helpers:

```text
COMPILE_STX
COMPILE_SPC
```

The important production paths are:

```text
<true, true>
```

for the legacy complete pipeline, and:

```text
<true, false>
```

when STX generation is performed separately from SPC insertion.

The new pipeline uses the same template structure to selectively perform its STX and SPC phases. 

---

# 📦 SPC Group Validation

Before grouping entries, ElectroHammer finds the target SPC name inside the `fti` array.

If the SPC name cannot be found, it logs:

```text
ERROR: Invalid .spc file
```

It also validates:

* the resulting array index,
* the split array size,
* and the maximum number of SPC groups.

This protects the grouping logic from invalid mappings. 

---

# ⚠️ Static 14-SPC Design

ElectroHammer assumes exactly:

```text
14
```

SPC categories.

This is encoded as:

```cpp
constexpr static std::uint64_t SpcFiles = 14;
```

and:

```cpp
std::array<std::vector<Entry>, 14>
```

Adding a new script archive therefore requires source-level changes to:

* `folders`,
* `fti`,
* `SpcFiles`,
* and potentially related build logic. 

---

# 🧭 Folder Indexes

The internal folder index is positional.

The indexes are:

| Index | Folder          |
| ----: | --------------- |
|     0 | `test`          |
|     1 | `subroutine`    |
|     2 | `prologue`      |
|     3 | `MapObjName`    |
|     4 | `game_resident` |
|     5 | `gallery`       |
|     6 | `epilogue`      |
|     7 | `chapter6`      |
|     8 | `chapter5`      |
|     9 | `chapter4`      |
|    10 | `chapter3`      |
|    11 | `chapter2`      |
|    12 | `chapter1`      |
|    13 | `ainori`        |

These indexes are significant because `different_hash_folders.txt` stores the indexes directly.

---

# 🔄 Important Cross-Tool Contract

Because `different_hash_folders.txt` stores numeric indexes, **Necronomicon and ElectroHammer must agree on the folder ordering**.

If one tool changes the ordering while the other does not, the incremental build can compile the wrong folders.

Therefore, the `folders` ordering is effectively an interface contract between tools.

---

# 📋 Output Files

ElectroHammer generates or modifies:

| File / Directory             | Purpose                                      |
| ---------------------------- | -------------------------------------------- |
| `DGRV3/base_spc/*.SPC`       | Actual game archives receiving compiled text |
| `EXTRACTED_FILES/`           | TXT staging area for the new STX pipeline    |
| `REPACKED_FILES/`            | Generated STX staging area                   |
| `eh_report.txt`              | Compilation manifest                         |
| `different_hash_folders.txt` | Consumed incremental folder metadata         |

---

# 📦 Files Modified in the Game Build

The most important side effect is modification of the SPC archives under:

```text
DGRV3/base_spc/
```

`SpcTool` inserts the generated `.stx` resources directly into these archives.

Therefore, ElectroHammer is **not a read-only analyzer**.

It modifies build artifacts.

---

# ⚠️ Existing SPC Archives

Because `SpcTool` performs insertion into existing archives, ElectroHammer expects the target SPC files to already exist.

The program validates the existence of:

```text
DGRV3/base_spc/
```

but the shown startup code does not individually validate every one of the 14 SPC files before compilation.

A missing or invalid target archive therefore becomes an external-tool/build error rather than an early ElectroHammer validation failure.

---

# 🧪 Build Report Example

A conceptual `eh_report.txt` entry looks like:

```text
chapter3/scene_12.txt
    --> REPACKED_FILES/scene_12.stx
    --> base_spc/chap3_text_US.SPC
```

The exact spacing is generated programmatically so that the three columns line up.

---

# 🛠️ Developer Notes

## 1. ElectroHammer is an orchestrator

It does not itself understand the internal STX or SPC binary formats.

Instead, it orchestrates:

```text
StxTool
DRV3_STX_TOOL
SpcTool
```

through external process execution.

This keeps ElectroHammer focused on build orchestration.

---

## 2. STX output paths are partly assumed

The code derives the expected STX path by changing the extension rather than verifying the actual output location reported by the STX tool.

This means the external tool's output convention is an implicit contract.

---

## 3. File names are flattened

The new STX staging directories receive only the basename.

Therefore:

```text
folderA/foo.txt
folderB/foo.txt
```

can collide.

This is a significant limitation for repositories containing duplicate filenames.

---

## 4. Filename-only incremental matching

`different_hashes.txt` entries are converted to filenames.

This means the incremental selection is not fully path-specific.

---

## 5. Folder matching is substring-based

`GetIndex()` searches for folder names inside the full path rather than requiring an exact path component.

Developers should take care when adding similarly named directories.

---

## 6. Busy-wait synchronization

The multithreaded build uses atomic counters with polling loops.

This is functional but not especially efficient from a synchronization perspective.

A future implementation could replace this with:

```text
condition_variable
future
latch
barrier
```

or another blocking synchronization primitive.

---

## 7. Detached threads

The STX and SPC worker threads are launched using detached `std::jthread`s.

Completion is tracked through atomics rather than explicit thread joining.

This makes the correctness of the completion counters particularly important.

---

# ⚠️ Potential Build Failure Points

Developers should investigate the following when ElectroHammer produces unexpected results:

### Missing repository

```text
DGRV3/
```

### Missing archive directory

```text
DGRV3/base_spc/
```

### Missing incremental metadata

```text
different_hashes.txt
different_hash_folders.txt
```

### Missing external tools

```text
StxTool
DRV3_STX_TOOL
SpcTool
```

### Invalid SPC mapping

An entry that cannot be mapped to one of the 14 target archives.

### Duplicate filenames

Potential collisions in:

```text
EXTRACTED_FILES/
REPACKED_FILES/
```

### Incorrect folder indexes

Mismatch between Necronomicon and ElectroHammer folder ordering.

### Existing archive corruption

An invalid SPC may cause `SpcTool` insertion to fail.

---

# 🔍 Troubleshooting Strategy

When an expected file does not appear in the final SPC:

### Step 1 — Check the file exists

```text
DGRV3/<folder>/<file>.txt
```

---

### Step 2 — Check its folder is recognized

The parent path must contain one of:

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

---

### Step 3 — Check the file is eligible

It must survive the filters for:

```text
.txt
_sha
_lines
README
LICENSE
.git
Baked
vars_bak
.pb
```

---

### Step 4 — Check incremental metadata

If not using:

```text
--all
```

verify that:

```text
different_hash_folders.txt
```

contains the correct folder index and:

```text
different_hashes.txt
```

contains the expected filename.

---

### Step 5 — Check the generated entry

Look in:

```text
eh_report.txt
```

If the file is absent, the problem is likely in EntryManagement.

If it appears but the STX/SPC result is wrong, the problem is likely in the compiler or external tools.

---

### Step 6 — Check the staging files

For the new STX pipeline:

```text
EXTRACTED_FILES/<filename>.txt
REPACKED_FILES/<filename>.stx
```

should exist.

---

### Step 7 — Check the target SPC

Finally verify:

```text
DGRV3/base_spc/<target>.SPC
```

contains the expected resource.

---

# 🧪 Recommended Full-Rebuild Test

When debugging an incremental build, use:

```text
Electrohammer.exe --all
```

This removes the dependency on:

```text
different_hash_folders.txt
different_hashes.txt
```

for file selection.

If the file appears during a full build but not during an incremental build, the problem is likely in the change-detection metadata rather than the STX/SPC compilation itself.

---

# 🔗 Relationship With HydraulicPress

HydraulicPress modifies/prepares script text before ElectroHammer compiles it.

Conceptually:

```text
HydraulicPress
     │
     ├── variable replacement
     ├── platform handling
     └── script preprocessing
             │
             ▼
        DGRV3/*.txt
             │
             ▼
        ElectroHammer
             │
             ├── TXT → STX
             └── STX → SPC
```

This means ElectroHammer should generally operate on the final processed text intended for injection.

---

# 🔗 Relationship With PoolRules

PoolRules measures translation progress.

ElectroHammer consumes the resulting scripts.

Their roles are therefore different:

```text
PoolRules
    ↓
"How complete is the translation?"

ElectroHammer
    ↓
"Turn the current translation into game resources."
```

PoolRules does not feed its percentage reports directly into ElectroHammer.

---

# 🔗 Relationship With MarkerStone

MarkerStone validates line-count integrity.

ElectroHammer assumes the script files are in a structurally usable state and does not perform MarkerStone's line-count comparison itself.

A useful workflow is:

```text
MarkerStone
    ↓
Verify script structure
    ↓
ElectroHammer
    ↓
Compile and inject
```

---

# 🔗 Relationship With Necronomicon

This is the most direct dependency:

```text
Necronomicon
    ↓
SHA comparison
    ↓
different_hashes.txt
different_hash_folders.txt
    ↓
ElectroHammer
    ↓
incremental compilation
```

Necronomicon determines **what changed**.

ElectroHammer determines **what must be rebuilt because of those changes**.

---

# 🧠 Conceptual Model

ElectroHammer can be understood as three layers:

```text
┌──────────────────────────────────────┐
│          ENTRY MANAGEMENT            │
│                                      │
│  DGRV3 → candidate files → Entries   │
└──────────────────┬───────────────────┘
                   │
                   ▼
┌──────────────────────────────────────┐
│             COMPILER                 │
│                                      │
│  TXT → STX → SPC                     │
└──────────────────┬───────────────────┘
                   │
                   ▼
┌──────────────────────────────────────┐
│           GAME RESOURCES             │
│                                      │
│      DGRV3/base_spc/*.SPC            │
└──────────────────────────────────────┘
```

---

# 📌 Important Implementation Contracts

ElectroHammer relies on several source-level contracts.

### Folder contract

The 14 folder names and their ordering must remain synchronized with the hash-generation tools.

### SPC contract

The `fti` array must match the folder indexes.

### STX contract

`StxTool` / `DRV3_STX_TOOL` must produce STX files where ElectroHammer expects them.

### SPC tool contract

`SpcTool` must accept the generated command format:

```text
SpcTool "<archive>" insert "<stx>"
```

### Repository contract

The expected structure must contain:

```text
DGRV3/
DGRV3/base_spc/
```

---

# 🚨 Special Cases and Quirks

## `different_hash_folders.txt` is deleted

This is easy to overlook.

ElectroHammer consumes it and removes it after reading.

---

## Missing folder metadata causes a five-second pause

This protects against accidental full recompilation.

---

## `--all` bypasses incremental selection

Useful for debugging and clean builds.

---

## Duplicate filenames can collide

Both incremental matching and STX staging are filename-oriented.

---

## Font injection happens after normal compilation

Fonts are not part of the normal entry list.

---

## Blacklist entries are currently commented out

The source documents known problematic files, but the current blacklist does not actively exclude them.

---

## STX and SPC compilation are externally delegated

ElectroHammer is primarily an orchestration layer rather than a binary compiler.

---

# 📄 Input / Output Summary

## Inputs

| Input                        | Purpose                       |
| ---------------------------- | ----------------------------- |
| `DGRV3/`                     | Source translation repository |
| `DGRV3/base_spc/`            | Target SPC archives           |
| `TextConfig.config`          | Runtime configuration         |
| `different_hashes.txt`       | Changed-file information      |
| `different_hash_folders.txt` | Changed-folder indexes        |
| `StxTool`                    | Legacy TXT → STX converter    |
| `DRV3_STX_TOOL`              | New batch TXT → STX converter |
| `SpcTool`                    | STX → SPC injector            |
| `v3_font00.stx`              | Font resource                 |
| `v3_font00.srdv`             | Font resource                 |

---

## Outputs

| Output                 | Purpose                                         |
| ---------------------- | ----------------------------------------------- |
| `DGRV3/base_spc/*.SPC` | Game-ready archive resources                    |
| `EXTRACTED_FILES/`     | New STX pipeline input staging                  |
| `REPACKED_FILES/`      | New STX pipeline output staging                 |
| `eh_report.txt`        | TXT → STX → SPC build manifest                  |
| Console log            | Build progress, warnings, skipped files, timing |

---

# 🏁 Summary

ElectroHammer is the **final text-to-game-resource compiler in TextTools**.

Its fundamental job is:

```text
Take processed TXT scripts
        ↓
Determine which files actually need rebuilding
        ↓
Convert TXT → STX
        ↓
Insert STX → SPC
        ↓
Inject required fonts
        ↓
Produce updated game-ready SPC archives
```

Its most important optimization is incremental compilation.

Necronomicon provides:

```text
different_hashes.txt
different_hash_folders.txt
```

and ElectroHammer uses those files to avoid rebuilding unchanged content.

The entry-management stage determines:

```text
WHAT to compile
```

while the compiler determines:

```text
HOW to compile it
```

The resulting pipeline is:

```text
DGRV3/*.txt
     │
     ▼
Change detection
     │
     ▼
EntryManagement
     │
     ▼
Selected Entries
     │
     ├───────────────────────┐
     ▼                       ▼
Legacy StxTool        DRV3_STX_TOOL
     │                       │
     └───────────┬───────────┘
                 ▼
              *.stx
                 │
                 ▼
              SpcTool
                 │
                 ▼
        DGRV3/base_spc/*.SPC
                 │
                 ▼
        Manual font injection
                 │
                 ▼
        Game-ready resources
```

In short:

```text
Necronomicon → detects what changed
ElectroHammer → rebuilds what changed
SpcTool       → places the rebuilt resources into the game archives
```

ElectroHammer is therefore the **build-and-injection engine that turns TextTools' processed script data into the actual SPC resources consumed by the game**.
