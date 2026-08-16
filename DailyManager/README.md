# V3DAILYMANAGER — DAILY BUILD AUTOMATION SYSTEM
## Team DAIX, 2026
---

This README is the **official high-level and implementation-oriented documentation** for **V3DailyManager**, Team DAIX's automated DAILY build system for Danganronpa V3.

V3DailyManager is **separate from DAIXTOOLS' actual compilation toolchains**.

It does not replace TextTools, ImageTools, or FontTools. Instead, it sits **above** them and acts as the automation layer that:

- prepares a clean build environment,
- selects platforms and toolchain configurations,
- clones the required repositories,
- launches TextTools, ImageTools, and FontTools,
- collects their output,
- reconstructs platform-specific game layouts,
- stores baked text, logs, and SPCs in their respective repositories,
- optionally uploads the results to GitHub,
- creates dated tags,
- and writes URLs used by external automation.

In simplified terms:

> **DAIXTOOLS builds the assets. V3DailyManager turns those builds into an automated daily release.**

The code describes the system as a CI/CD-style pipeline for Danganronpa V3 modding. :contentReference[oaicite:0]{index=0}

---

# ⚠️ EXTREMELY IMPORTANT WARNING

## V3DailyManager is potentially destructive.

V3DailyManager contains two cleanup functions:

- `DeleteEverything()`
- `DeleteEverything2()`

These are **not ordinary cleanup utilities**.

They can recursively delete essentially everything in the working directory except for specific whitelisted executables, DLLs, source files, and selected repository paths.

The intended operating environment is therefore:

```text
an EMPTY directory
````

or:

```text
a directory containing only the V3DailyManager executables
```

Safe mode exists specifically to prevent DAILY from starting in a directory containing unrelated files.

The `--unsafe` argument disables that protection.

### DO NOT run:

```text
V3DailyManager.exe --unsafe
```

inside a directory containing files you care about unless you have explicitly verified what will be deleted.

The source code itself repeatedly warns that these operations are dangerous.

---

# 📌 What V3DailyManager Is

V3DailyManager is the **automation/orchestration component** of the Danganronpa V3 translation build infrastructure.

It belongs conceptually above:

```text
TextTools
ImageTools
FontTools
```

rather than inside any of them.

The architecture can be visualized as:

```text
                         V3DailyManager
                               │
             ┌─────────────────┼─────────────────┐
             │                 │                 │
             ▼                 ▼                 ▼
         TextTools         ImageTools        FontTools
       (StackedBooks)       (Pianist)         (Monokuma)
             │                 │                 │
             └─────────────────┼─────────────────┘
                               │
                               ▼
                    Compiled / baked output
                               │
                               ▼
                     Distribution pipeline
                               │
             ┌─────────────────┼─────────────────┐
             ▼                 ▼                 ▼
          PC build          Switch build      Xbox build
             │                 │                 │
             └─────────────────┼─────────────────┘
                               │
                               ▼
                     Daily repositories
                               │
                               ▼
                        GitHub / Cloud
```

The critical distinction is:

> **V3DailyManager does not perform the actual text, graphics, or font compilation itself.**

It launches the appropriate external toolchains and then organizes their results.

---

# 🧩 Relationship to DAIXTOOLS

The overall ecosystem can be thought of as two layers.

## Layer 1 — DAIXTOOLS

These are the actual asset-generation pipelines:

```text
TextTools
ImageTools
FontTools
```

Their master executables are:

```text
StackedBooks
Pianist
Monokuma
```

They perform the actual modifications and compilation.

---

## Layer 2 — V3DailyManager

V3DailyManager sits above them:

```text
V3DailyManager
      │
      ├── runs StackedBooks
      ├── runs Pianist
      ├── runs Monokuma
      │
      ├── collects output
      ├── creates platform builds
      ├── updates Daily repositories
      └── publishes the results
```

This means V3DailyManager is closer to a **build server / release manager** than a traditional modding utility.

---

# 🔄 DAILY PIPELINE

The canonical high-level process is:

```text
1. Parse command-line arguments
2. Determine build platforms
3. Determine enabled toolchains
4. Perform safe-mode validation
5. Authenticate / prepare GitHub access
6. Clean previous build state
7. Clone DGRV3-Tools
8. Clone Daily repositories
9. Run TextTools
10. Run ImageTools
11. Run FontTools
12. Collect generated output
13. Construct platform-specific game layouts
14. Copy patches
15. Copy baked text
16. Copy logs
17. Copy SPC files
18. Copy graphics
19. Copy fonts
20. Remove unwanted files/folders
21. Update repository metadata
22. Commit changes
23. Create daily tag
24. Push commit and tag
25. Write repository URLs
26. Report timing/status
27. Clean up temporary metadata
```

Not every stage necessarily runs for every invocation.

Command-line options can disable individual toolchains, restrict platforms, disable cloud uploads, or replace configurations.

---

# 🎛️ COMMAND-LINE INTERFACE

V3DailyManager parses command-line arguments before starting the actual build.

Arguments are deduplicated before processing.

The supported options include the following.

---

## Platform selection

### `--pc`

Build the PC version.

```text
--pc
```

---

### `--switch`

Build the Nintendo Switch/Unity configuration.

```text
--switch
```

---

### `--xbox`

Build the Xbox configuration.

```text
--xbox
```

---

### `--psvita`

Requests a PSVita build.

```text
--psvita
```

The platform exists in the internal platform enumeration, but unsupported platforms are removed before the actual distribution stage.

---

### `--ps4`

Requests a PS4 build.

```text
--ps4
```

Like PSVita, this platform is currently treated as unsupported.

---

### `--all`

Adds every known platform to the build list.

Conceptually:

```text
PC
Switch
Xbox
PSVita
PS4
Android
iOS
```

The system subsequently removes unsupported platforms.

Therefore:

> `--all` means "attempt all known platform definitions", not "all platforms are guaranteed to build."



---

## Toolchain selection

### `--text-only`

Runs only the text portion of the build.

---

### `--gfx-only`

Runs only the graphics portion.

---

### `--font-only`

Runs only the font portion.

---

## Toolchain exclusion

### `--no-text`

Prevents TextTools from running.

---

### `--no-gfx`

Prevents ImageTools from running.

---

### `--no-font`

Prevents FontTools from running.

---

### Important validation

V3DailyManager refuses to continue if all three are disabled:

```text
--no-text
--no-gfx
--no-font
```

That would leave DAILY with nothing to build.

The program explicitly reports:

```text
Contradicting arguments!
```

and exits. 

---

# ☁️ CLOUD CONTROL

## `--no-cloud`

Disables the GitHub upload stage.

```text
--no-cloud
```

The local build/distribution process can therefore be performed without committing or pushing the Daily repositories.

When cloud upload is disabled, generated URL files report the repositories as unavailable rather than providing repository URLs. 

---

# 🌐 PUBLIC / PRIVATE REPOSITORIES

## `--public`

Switches DAILY from the private repository set to the public repository set.

Without `--public`, the code selects the private repository names.

### Private mode

```text
DGRV3-Daily-Private
DGRV3-Daily-Private-Text
DGRV3-Daily-Private-Log
DGRV3-Daily-Private-SPC
```

### Public mode

```text
DGRV3-Daily
DGRV3-Daily-Text
DGRV3-Daily-Log
DGRV3-Daily-SPC
```

The selection is made centrally when the build begins. 

---

# ⚙️ CONFIGURATION OVERRIDES

V3DailyManager supports independent configuration overrides for each external toolchain.

## `--text-config`

```text
--text-config <folder>
```

Selects a specific TextTools configuration.

---

## `--gfx-config`

```text
--gfx-config <folder>
```

Selects a specific ImageTools configuration.

---

## `--font-config`

```text
--font-config <folder>
```

Selects a specific FontTools configuration.

The following argument is consumed as the configuration name/value as long as it is not another `--` argument.

For example:

```text
V3DailyManager.exe --text-config ForDaily
```

The important architectural detail is that these configurations come from **DGRV3-Tools' configuration folders**, not from inside the individual `TextTools`, `ImageTools`, or `FontTools` directories.



---

# 🖥️ GUI MODE

## `--gui`

Enables GUI-related behavior.

```text
--gui
```

One important interaction is with safe mode.

Normally, safe mode stops execution when more than one file is present in the working directory.

GUI mode allows a slightly more permissive case:

```text
count <= 2
```

instead of the normal threshold.

This is a special-case compatibility behavior rather than a general safety bypass. 

---

# ☠️ UNSAFE MODE

## `--unsafe`

Disables safe mode.

```text
--unsafe
```

This allows DAILY to proceed even when the working directory contains files that would normally cause the safety check to abort.

It does **not** make the cleanup functions safer.

It does the opposite:

> It removes the guard that normally prevents destructive cleanup from occurring in a dirty directory.

Use this option only when the contents of the working directory have been explicitly verified.

---

# 🧹 SAFE MODE

Before doing any significant work, V3DailyManager checks the current directory.

The logic is approximately:

```text
if safe mode:
    count files in current directory

    if count > 1:
        refuse to start

        unless GUI mode is active
        and the count is small enough
```

The purpose is to make the destructive cleanup stage predictable.

The intended model is:

```text
empty directory
      ↓
V3DailyManager
      ↓
clean environment
      ↓
clone repositories
      ↓
build
```

rather than:

```text
random existing files
      ↓
V3DailyManager
      ↓
DELETE EVERYTHING
```



---

# 🗂️ DGRV3-TOOLS

The first major external dependency is:

```text
DGRV3-Tools
```

V3DailyManager clones this repository dynamically.

It contains the toolchain archives and configuration directories required to run the actual DAIXTOOLS pipelines.

The important conceptual structure is:

```text
DGRV3-Tools/
│
├── TextTools/
│   ├── Tools.7z
│   └── ...
│
├── ImageTools/
│   ├── Tools.7z
│   └── ...
│
├── FontTools/
│   ├── Tools.7z
│   └── ...
│
├── ForDaily/
├── ForDailyPrivate/
├── TextInjector/
├── TextInjectorForSwitch/
├── TextInjectorForXbox/
├── ImageInjector/
├── ImageInjectorForSwitch/
├── ImageInjectorForXbox/
└── ...
```

A particularly important design detail is that configuration files are **not** expected to live inside the tool directories.

`Process.cpp` explicitly documents this separation. 

---

# 📦 TOOL ARCHIVE EXECUTION

When a toolchain is launched, V3DailyManager's process helpers:

1. locate the relevant tool directory,
2. extract the appropriate `Tools.7z`,
3. locate/copy the requested configuration,
4. execute the master executable,
5. return control to DAILY.

The three external master programs are:

| Pipeline   | Master executable |
| ---------- | ----------------- |
| TextTools  | `StackedBooks`    |
| ImageTools | `Pianist`         |
| FontTools  | `Monokuma`        |

V3DailyManager is therefore an **executor**, not a replacement for these programs. 

---

# 🧰 REPOSITORIES

DAILY operates on several repositories.

## Tool repository

```text
DGRV3-Tools
```

Contains the toolchain and configurations.

---

## Patch repository

Private:

```text
DGRV3-Daily-Private
```

Public:

```text
DGRV3-Daily
```

Contains the final patch/build distribution.

---

## Text repository

Private:

```text
DGRV3-Daily-Private-Text
```

Public:

```text
DGRV3-Daily-Text
```

Contains baked text.

DAILY intentionally uploads **baked** text rather than randomized text output. 

---

## Log repository

Private:

```text
DGRV3-Daily-Private-Log
```

Public:

```text
DGRV3-Daily-Log
```

Contains generated build logs.

---

## SPC repository

Private:

```text
DGRV3-Daily-Private-SPC
```

Public:

```text
DGRV3-Daily-SPC
```

Contains SPC output.



---

# 🔐 GITHUB AUTHENTICATION

The current source expects GitHub credentials to be supplied by the build environment/source configuration.

The code contains placeholders for:

```text
GitHub username
GitHub password
GitHub token
```

These are passed through VGit/VGitUtils to construct repository URLs.

The actual repository URL is calculated rather than hard-coded as a plain public URL.

This allows the same code to operate against different repository visibility configurations. 

### Developer warning

Credentials should never be committed in plaintext.

The current source uses an `EncryptString()` mechanism around the credential strings, but this should not be interpreted as a replacement for proper secret management.

---

# 🌿 GIT / VGIT INTEGRATION

All major repository operations are performed through:

```text
VGit
VGitUtils
```

rather than directly implementing Git operations inside the DAILY orchestration logic.

The repository helper layer handles:

* Git version checking,
* cloning,
* beta branch cloning,
* URL generation,
* committing,
* tagging,
* pushing,
* and related repository operations.



---

# 🌱 BETA BRANCH HANDLING

When cloning `DGRV3-Tools`, DAILY can request beta branch handling.

The clone operation:

1. obtains the Git version,
2. clones the main repository,
3. optionally attempts beta branch handling,
4. excludes explicitly protected branches,
5. reports failure through `vgit_failed.txt`.

Branches such as:

```text
HEAD
```

and the configured main branch are explicitly excluded from being treated as beta branches.

If beta cloning fails, DAILY creates:

```text
vgit_failed.txt
```

and reports the clone failure. 

---

# 🏗️ BUILD INITIALIZATION

After argument processing and safety validation, `Start()` establishes:

```text
current_dir
```

as the current working directory and:

```text
current_dir / "DGRV3-Tools"
```

as the expected toolchain repository location.

It then determines the repository names, platform list, authentication URLs, and build configuration.

---

# 🖥️ PLATFORM SELECTION

The platform list is constructed from command-line arguments.

If:

```text
--all
```

is specified, all known platform enum values are initially inserted.

Otherwise, if no platform was explicitly selected:

```text
PC
```

is used as the default.

The platform list is then:

1. sorted,
2. deduplicated,
3. filtered for unsupported platforms.



---

# 🚫 UNSUPPORTED PLATFORMS

The internal platform system contains more platforms than the distribution implementation currently supports.

Known platform definitions include:

```text
PC
Switch
Xbox
PSVita
PS4
Android
iOS
```

However, DAILY removes unsupported platforms before building.

Therefore an invocation such as:

```text
--all
```

does not necessarily produce seven builds.

It produces the subset that `Distribution::IsUnsupportedPlatform()` considers supported.

The source documentation identifies PC, Switch, and Xbox as the intended supported targets, while PS4, PSVita, Android, and iOS are currently unsupported. 

---

# 🧹 INITIAL CLEANUP

Before cloning the toolchain, DAILY cleans the previous `DGRV3-Tools` environment.

The older:

```text
DeleteEverything()
```

function removes the existing tool directory and then scans the working directory for removable content.

The newer:

```text
DeleteEverything2()
```

function performs an explicit whitelist-based cleanup.

---

# ☢️ DELETEEVERYTHING()

`DeleteEverything()` is the legacy cleanup implementation.

It protects selected items such as:

```text
7za
ups
autov3.txt
```

and source/project extensions such as:

```text
.h
.cpp
.vcxproj
.sln
.user
.filters
```

Everything else is considered removable.

The Windows implementation uses commands such as:

```text
rmdir /s /q
DeleteFileA()
```

while Linux uses corresponding filesystem/removal operations.



---

# ☢️ DELETEEVERYTHING2()

`DeleteEverything2()` is the newer cleanup system.

It uses a dedicated:

```cpp
shouldDelete(...)
```

predicate.

Protected names include:

```text
7za
ups
D3DCompiler_47_cor3.dll
PenImc_cor3.dll
PresentationNative_cor3.dll
vcruntime140_cor3.dll
wpfgfx_cor3.dll
autov3.txt
```

Protected source/project extensions include:

```text
.h
.cpp
.vcxproj
.sln
.user
.filters
```

Additionally, paths containing:

```text
DGRV3-Tools
DGRV3-Daily
```

are excluded from deletion.

Everything else can be considered a deletion candidate. 

---

# 📊 TRANSLATION PROGRESS

V3DailyManager also acts as a reporting system.

TextTools can produce:

```text
percentage_res.txt
```

DAILY reads this file through:

```cpp
Files::GetPercentages()
```

The function maps the lines to predefined categories:

```text
Extra Mode(s)
Chapter 1
Chapter 2
Chapter 3
Chapter 4
Chapter 5
Chapter 6
Epilogue
Gallery
Generic Text
Map Object Names
Prologue
Subroutine
Test
Total
```

The resulting strings are used when updating Daily repository README files. 

---

# 🐛 HISTORICAL PERCENTAGE QUIRK

`GetPercentages()` contains a deliberate historical oddity.

After reading the categories, it swaps the entries corresponding to:

```text
Extra Mode(s)
```

and:

```text
Prologue
```

The source comments describe this as an inversion involving the historical `ainori`/Prologue mapping.

This is important because the order in `percentage_res.txt` is **not interpreted completely literally**.

It should be treated as compatibility behavior, not redesigned casually.



---

# 📁 FILE COMPARISON

V3DailyManager contains several filesystem helpers for determining whether generated output changed.

## `IsSTDEqualSame()`

Compares two files by:

1. opening both in binary mode,
2. checking their sizes,
3. rewinding them,
4. comparing their byte streams.

Return values are:

```text
-1 = file could not be opened
-2 = file sizes differ
 0 = files are identical
 1 = files differ
```

This is primarily used for `.ups` comparison.

---

## `IsInternallySame()`

Loads both files entirely into memory and compares:

```text
length
contents
```

It is effectively a second comparison mechanism/fallback.



---

# 🔍 INCREMENTAL BUILD CHECKING

DAILY contains logic for detecting whether generated UPS files actually changed.

The relevant helper is:

```cpp
CheckDifferent()
```

It compares generated `.ups` files against the Daily repository's:

```text
LatestAutomaticBuild/Raw/
```

area.

For each generated UPS file it checks whether an equivalent file exists and compares the contents.

If at least one file differs:

```text
build required
```

If all files are identical:

```text
nothing new
```

and the build can terminate early.

---

# ⚠️ CURRENT IMPLEMENTATION QUIRK: FORCE UPDATE

The current `V3DailyManager.cpp` contains:

```cpp
static constexpr bool force_update = true;
```

This is significant.

The incremental `.ups` comparison mechanism still exists, but with `force_update` enabled, the normal optimization is effectively bypassed.

The source explicitly describes the comparison as a lightweight optimization that can avoid unnecessary builds when enabled.

With the current value:

```text
force_update = true
```

DAILY proceeds with the build rather than relying on the comparison result to skip it.

Developers changing this value should understand that the `Raw` comparison mechanism is an optimization, not the core compilation logic. 

---

# 🧰 FILE COPYING

DAILY uses several specialized filesystem helpers.

## `CopyRecursive()`

Recursively copies a directory tree:

```text
source → destination
```

with:

```text
overwrite_existing
recursive
```

options.

This is used for artifacts such as baked text.

---

## `CopyFilesWithFilter()`

Copies only files matching a requested extension.

For example:

```text
.ups
.txt
.spc
```

It supports:

* recursive mode,
* non-recursive mode,
* case-insensitive extension matching for upper/lowercase forms,
* relative path preservation,
* destination directory creation,
* overwrite behavior.

This function is particularly important when moving Daily logs, text, and SPCs while preserving their folder hierarchy. 

---

# 📦 DISTRIBUTION STAGE

The distribution layer is implemented primarily in:

```text
distribution.cpp
```

This is the most complex part of the DAILY pipeline.

It takes the output of:

```text
TextTools
ImageTools
FontTools
```

and transforms it into the platform-specific layout expected by the Daily repositories.



---

# 🔨 DISTRIBUTION PROCESS

For each selected platform, DAILY calls:

```cpp
Distribution::DistributeFiles(...)
```

The distribution process performs the following conceptual operations:

```text
1. Clean temporary build state
2. Run required external toolchains
3. Prepare platform folder structure
4. Copy UPS patches
5. Copy baked text
6. Copy logs
7. Copy SPCs
8. Copy graphics
9. Copy fonts
10. Remove irrelevant platform data
11. Rename/output platform-specific folders
```

The function is called once per selected platform. 

---

# 📝 BAKED TEXT

Text output is collected from:

```text
Baked/
```

and copied into the Daily Text repository.

DAILY intentionally uses the baked output.

It does **not** upload randomized text as the Daily Text artifact.

This distinction matters because TextTools can generate randomized/intermediate output that is not intended to become the canonical Daily translation text. 

---

# 🖼️ GRAPHICS

Graphics output is collected from:

```text
Distribute-GFX/
```

The destination depends on the platform.

For PC/Xbox, graphics are copied into the appropriate platform build tree.

For Unity-oriented platforms such as Switch, the folder handling differs because the game uses a different internal asset layout.



---

# 🔤 FONTS

Font output is collected from:

```text
Distribute-Font/
```

For PC/Xbox, the font output is copied into the platform build.

Unity font support is more limited; the distribution code explicitly contains a TODO regarding Unity font handling.

Therefore:

> Do not assume that the FontTools output is distributed identically on every platform.



---

# 🎮 PC BUILD LAYOUT

The Daily build uses a structure equivalent to:

```text
LatestAutomaticBuild/
└── PC/
    └── data/
        └── win/
```

The patch output is placed into the relevant game directories.

The distribution system handles paths such as:

```text
game_resident
wrd_data
wrd_script
```

depending on the generated patch.

---

# 🎮 XBOX BUILD LAYOUT

The Xbox target uses the PC-style game structure but with its own output location:

```text
LatestAutomaticBuild/
└── Xbox/
    └── Data/
        └── WIN/
```

The source currently treats Xbox alongside PC for several graphics/font operations. 

---

# 🎮 NINTENDO SWITCH BUILD LAYOUT

Switch uses the Unity-style layout:

```text
LatestAutomaticBuild/
└── Switch/
    └── Data/
        └── StreamingAssets/
            └── Switch/
```

Unity platform handling differs from PC/Xbox.

In particular, DAILY removes irrelevant Unity platform directories after copying the appropriate data.

---

# 🧹 UNITY PLATFORM CLEANUP

For Unity-based distributions, `DeleteUnneededPlatforms()` removes directories that are not relevant to the current target.

The whitelist includes:

```text
master
```

and the selected internal platform directory.

Everything else is removed.

This prevents a Switch build from accidentally retaining directories intended for other Unity targets. 

---

# 🩹 UPS PATCH DISTRIBUTION

Generated patches originate from:

```text
Distribute/
```

DAILY collects `.ups` files and places them into the appropriate platform game hierarchy.

This is what transforms a generic collection of UPS files into an actual Daily build layout.

The destination structure is designed around the game's existing directories, particularly:

```text
game_resident
wrd_data
wrd_script
```

---

# 📦 SPC DISTRIBUTION

The main SPC output directory is:

```text
ModifiedFiles/
```

Font-specific output:

```text
ModifiedFiles-Font/
```

Graphics-specific output:

```text
ModifiedFiles-GFX/
```

DAILY copies the appropriate SPC output into:

```text
DGRV3-Daily-SPC
```

or:

```text
DGRV3-Daily-Private-SPC
```

depending on repository mode.

---

# 🧾 LOG DISTRIBUTION

Logs are copied into:

```text
DGRV3-Daily-Log
```

or:

```text
DGRV3-Daily-Private-Log
```

The same filtered-copy machinery used for other artifacts preserves the appropriate directory structure. 

---

# 🧹 FINAL PATCH REPOSITORY CLEANUP

Before uploading, DAILY removes several folders that are not supposed to become part of the Daily patch repository:

```text
x64
V3DailyManager
Release
```

It also calls:

```cpp
Files::DeleteUPSNormals(...)
```

to remove UPS "normal" artifacts that are not intended for final distribution.



---

# ☁️ CLOUD UPLOAD PIPELINE

When cloud upload is enabled, DAILY uploads multiple repositories.

The general sequence is:

```text
Update .gitignore
       ↓
Update README
       ↓
git add
       ↓
commit
       ↓
create YYYY.MM.DD tag
       ↓
push main
       ↓
push tag
```

The helper responsible for this is:

```cpp
Remote::UploadToCloud()
```



---

# 🧹 AUTOMATIC `.gitignore` GENERATION

Before committing, DAILY regenerates `.gitignore`.

It blocks heavy or intermediate formats such as:

```text
*.7z
*.cpk
*.srd
*.sfl
*.stx
*.ab
*.pb
*.assets
*.arc
```

It also blocks generated metadata such as:

```text
*_url.txt
*license.txt
autov3.txt
```

Repository-specific rules determine whether:

```text
*.spc
*.txt
*.ups
```

are allowed.

For example:

* Text repositories allow text but reject patches.
* Patch repositories allow UPS files but reject text.
* SPC repositories allow SPCs but reject patches/text.



---

# 📝 DAILY README GENERATION

DAILY automatically updates:

```text
README.md
README_EN.md
```

before uploading.

The generated information includes:

* translation progress,
* timestamps,
* repository information,
* links to related repositories.

The percentage data comes from:

```text
percentage_res.txt
```

and is formatted by:

```cpp
Files::GetPercentages()
```

---

# 🏷️ DAILY TAGGING

Each cloud upload generates a date-based tag.

The source describes the format as:

```text
YYYY.MM.DD
```

The commit message contains the update time and platform/build metadata.

The resulting operation is effectively:

```text
commit
+
dated tag
+
push main
+
push tag
```

This makes each Daily build reproducible/retrievable through a dated Git tag. 

---

# 🔗 GENERATED URL FILES

After the build, DAILY writes repository URLs into:

```text
patches_url.txt
text_url.txt
log_url.txt
spc_url.txt
```

These are intended for external automation and scripts.

They point toward the relevant repository's tag area.

When cloud upload is disabled, the files contain:

```text
Unavailable
```

instead. 

---

# ⏱️ BUILD TIMING

V3DailyManager records the start and end times of the build.

The final output reports processing duration in seconds/minutes.

This is useful for the intended CI/CD-style workflow because DAILY is designed to run repeatedly rather than as a one-off manual compilation.

---

# 📂 IMPORTANT WORKING DIRECTORIES

Depending on the stage, DAILY may create or use directories such as:

```text
DGRV3-Tools/
DGRV3-Daily/
DGRV3-Daily-Text/
DGRV3-Daily-Log/
DGRV3-Daily-SPC/

Baked/
Distribute/
Distribute-GFX/
Distribute-Font/

ModifiedFiles/
ModifiedFiles-GFX/
ModifiedFiles-Font/

LatestAutomaticBuild/
```

The exact presence of each directory depends on the selected toolchains, platform, and cloud settings.

---

# 📤 FINAL OUTPUT STRUCTURE

A successful build conceptually produces something similar to:

```text
LatestAutomaticBuild/
│
├── PC/
│   └── data/
│       └── win/
│
├── Switch/
│   └── Data/
│       └── StreamingAssets/
│           └── Switch/
│
└── Xbox/
    └── Data/
        └── WIN/
```

alongside the repository-specific artifact trees:

```text
DGRV3-Daily/
DGRV3-Daily-Text/
DGRV3-Daily-Log/
DGRV3-Daily-SPC/
```

and metadata:

```text
patches_url.txt
text_url.txt
log_url.txt
spc_url.txt
```

---

# 🔄 COMPONENT INTERACTION

The most important relationships are:

```text
                    V3DailyManager
                           │
             ┌─────────────┴─────────────┐
             │                           │
        Remote::*                   Process::*
             │                           │
       Git/VGit layer             Tool execution
             │                           │
             │             ┌─────────────┼─────────────┐
             │             ▼             ▼             ▼
             │        StackedBooks     Pianist      Monokuma
             │          TextTools    ImageTools    FontTools
             │             │             │             │
             └─────────────┴─────────────┴─────────────┘
                           │
                           ▼
                    Distribution::*
                           │
              ┌────────────┼────────────┐
              ▼            ▼            ▼
             PC         Switch        Xbox
              │            │            │
              └────────────┼────────────┘
                           ▼
                    Daily repositories
                           │
                           ▼
                         GitHub
```

Supporting modules include:

```text
files.cpp
    filesystem operations,
    comparisons,
    copying,
    cleanup,
    percentages

process.cpp
    external executable launching

distribution.cpp
    platform-specific distribution

remote.cpp
    GitHub/VGit operations
```

---

# 🧠 INTERNAL MODULE RESPONSIBILITIES

## `V3DailyManager.cpp`

Main orchestration layer.

Responsible for:

* argument parsing,
* global build-state selection,
* safe mode,
* repository selection,
* platform selection,
* build sequencing,
* invoking distribution,
* cloud-upload sequencing,
* timing,
* final metadata.



---

## `files.cpp`

Filesystem utility layer.

Responsible for:

* percentage loading,
* file comparisons,
* recursive copying,
* filtered copying,
* directory creation,
* directory deletion,
* cleanup,
* UPS comparison,
* file counting.

It also contains the two dangerous cleanup implementations.

---

## `process.cpp`

External process layer.

Responsible for:

* platform-name conversion,
* locating toolchain components,
* extracting tool archives,
* selecting configuration folders,
* launching TextTools,
* launching ImageTools,
* launching FontTools.



---

## `distribution.cpp`

Build assembly layer.

Responsible for:

* platform-specific layout creation,
* running/distributing tool outputs,
* UPS placement,
* baked text placement,
* log placement,
* SPC placement,
* GFX placement,
* font placement,
* Unity cleanup.



---

## `remote.cpp`

Remote/Git layer.

Responsible for:

* repository cloning,
* beta handling,
* GitHub URL generation,
* `.gitignore` generation,
* README generation,
* committing,
* tagging,
* pushing.



---

# 🧪 SPECIAL CASES AND QUIRKS

## 1. `--all` does not mean every platform will build

Unsupported platforms are filtered afterward.

---

## 2. PC is the default platform

If no platform is specified, DAILY inserts PC.

---

## 3. All toolchains can be disabled accidentally

The program explicitly rejects a state where:

```text
no_text == true
no_gfx == true
no_font == true
```

---

## 4. Safe mode counts files, not "important" files

The protection is based on the number of files found in the working directory.

Do not assume a file is harmless simply because DAILY might not care about its contents.

---

## 5. `--unsafe` is genuinely unsafe

It does not merely suppress a warning.

It permits the destructive cleanup stage to proceed.

---

## 6. Configuration directories are separate from tool directories

DAILY expects tool archives inside:

```text
TextTools/
ImageTools/
FontTools/
```

while actual configurations are stored elsewhere in `DGRV3-Tools`.

---

## 7. Baked text is canonical Daily text

Randomized/intermediate text is deliberately not what DAILY uploads to the Daily Text repository.

---

## 8. Unity distribution is different

Switch uses a Unity-style directory structure and receives special cleanup.

---

## 9. Font handling is not identical across platforms

PC/Xbox have explicit font distribution logic.

Unity font support is more limited.

---

## 10. The incremental build system currently has force-update enabled

The code contains:

```cpp
static constexpr bool force_update = true;
```

so the UPS comparison optimization is currently bypassed.

---

## 11. `GetPercentages()` contains historical ordering behavior

The category list is modified after reading the percentage file.

Do not casually reorder it without checking the producer of `percentage_res.txt`.

---

# 🐧 WINDOWS / LINUX NOTES

The code contains platform-specific behavior.

Examples include:

### Executable naming

Windows:

```text
V3DailyManager.exe
7za.exe
ups.exe
```

Linux:

```text
V3DailyManager
7za
ups
```

### Cleanup

Windows uses Windows deletion APIs/commands in several locations.

Linux uses:

```text
rm
```

and filesystem operations.

### Permissions

The distribution code also performs:

```text
chmod -R +x *
```

on non-Windows systems.

This is necessary because extracted tool executables may otherwise lack execute permission.

---

# 🛠️ DEVELOPER NOTES

## V3DailyManager is intentionally stateful

A large portion of the program relies on global build-state variables such as:

```text
private_repo
build_platforms
no_text
no_gfx
no_font
no_cloud
safe_mode
has_gui
specific_text_config
specific_gfx_config
specific_font_config
```

This makes argument parsing directly influence later pipeline stages.

Changes to argument handling should therefore be tested against the entire pipeline rather than in isolation.

---

## The filesystem layer is security-critical

Changes to:

```text
DeleteEverything()
DeleteEverything2()
shouldDelete()
```

should be treated as high-risk changes.

A small whitelist modification can change what DAILY destroys.

---

## Distribution changes can affect multiple platforms

`Distribution::DistributeFiles()` is called once for every selected platform.

A change to shared distribution logic can therefore affect:

```text
PC
Switch
Xbox
```

simultaneously.

---

## Repository behavior is coupled to filename conventions

`UploadToCloud()` identifies repository type from repository names such as:

```text
-Text
-Log
-SPC
-Private
```

Changing repository naming conventions can therefore affect `.gitignore` generation and upload behavior.



---

# 🚨 FAILURE CONDITIONS

Common failure scenarios include:

### Dirty working directory

```text
Refusing to start because of the files inside this folder.
```

Solution:

Use a clean DAILY directory.

Do **not** immediately solve this with `--unsafe`.

---

### Tool repository failure

If `DGRV3-Tools` cannot be cloned, the pipeline cannot launch the external toolchains.

Beta clone failures can result in:

```text
vgit_failed.txt
```

---

### Missing configuration

If a selected:

```text
--text-config
--gfx-config
--font-config
```

configuration cannot be found, the corresponding external toolchain cannot be initialized correctly.

---

### Unsupported platform

The platform may be accepted during argument parsing and then removed later by:

```text
Distribution::IsUnsupportedPlatform()
```

This is expected behavior.

---

### No output

If the external toolchains produce no relevant output, there may be nothing to distribute or upload.

---

### No cloud repositories

With:

```text
--no-cloud
```

local generation can proceed, but GitHub upload does not occur and the generated URL files report:

```text
Unavailable
```

---

# 📋 RECOMMENDED OPERATING MODEL

The intended DAILY environment is:

```text
C:\DAILY\
│
├── V3DailyManager.exe
├── 7za.exe
├── ups.exe
└── ...
```

with no unrelated files.

Then:

```text
V3DailyManager.exe --pc
```

or a more complete invocation such as:

```text
V3DailyManager.exe --pc --switch --xbox
```

DAILY creates/clones the required repositories and performs the build from there.

---

# 🧭 EXAMPLE INVOCATIONS

## Default PC build

```text
V3DailyManager.exe
```

If no platform is selected, PC becomes the default.

---

## PC + Switch

```text
V3DailyManager.exe --pc --switch
```

---

## Full supported platform attempt

```text
V3DailyManager.exe --all
```

Unsupported platform definitions are subsequently removed.

---

## Text-only build

```text
V3DailyManager.exe --text-only
```

---

## Graphics-only build

```text
V3DailyManager.exe --gfx-only
```

---

## Font-only build

```text
V3DailyManager.exe --font-only
```

---

## Build without cloud upload

```text
V3DailyManager.exe --no-cloud
```

---

## Public Daily repositories

```text
V3DailyManager.exe --public
```

---

## Explicit TextTools configuration

```text
V3DailyManager.exe --text-config ForDaily
```

---

## Explicit multi-tool configuration

```text
V3DailyManager.exe ^
    --text-config ForDaily ^
    --gfx-config ImageInjector ^
    --font-config ForDaily
```

---

# 🔬 DEBUGGING STRATEGY

When debugging V3DailyManager, it is generally better to isolate the pipeline.

For example:

```text
--text-only
```

can determine whether the problem originates in TextTools.

Likewise:

```text
--gfx-only
```

and:

```text
--font-only
```

can isolate the other pipelines.

`--no-cloud` is useful when debugging local distribution without introducing GitHub state changes.

For filesystem problems, inspect:

```text
DGRV3-Tools/
Baked/
Distribute/
Distribute-GFX/
Distribute-Font/
ModifiedFiles/
ModifiedFiles-GFX/
ModifiedFiles-Font/
LatestAutomaticBuild/
```

before investigating the GitHub layer.

---

# 🏁 FINAL PIPELINE SUMMARY

V3DailyManager can be summarized as:

```text
                 COMMAND LINE
                      │
                      ▼
                Argument Parser
                      │
                      ▼
                  Safe Mode
                      │
                      ▼
             Repository Selection
                      │
                      ▼
                Platform Filter
                      │
                      ▼
              Clean Environment
                      │
                      ▼
              Clone DGRV3-Tools
                      │
                      ▼
       ┌──────────────┼──────────────┐
       │              │              │
       ▼              ▼              ▼
   TextTools      ImageTools      FontTools
  StackedBooks     Pianist        Monokuma
       │              │              │
       └──────────────┼──────────────┘
                      ▼
              Generated Outputs
                      │
                      ▼
              Distribution Layer
                      │
       ┌──────────────┼──────────────┐
       ▼              ▼              ▼
      PC           Switch          Xbox
       │              │              │
       └──────────────┼──────────────┘
                      ▼
              Daily Repositories
                      │
          ┌───────────┼───────────┐
          ▼           ▼           ▼
       Patches      Text        Logs/SPCs
          │           │           │
          └───────────┼───────────┘
                      ▼
                GitHub Upload
                      │
                      ▼
                 Commit + Tag
                      │
                      ▼
              Generated URL files
```

---

# 📦 COMMON OUTPUTS

| Output                  | Purpose                           |
| ----------------------- | --------------------------------- |
| `DGRV3-Tools/`          | Cloned toolchain repository       |
| `DGRV3-Daily*/`         | Daily patch repository            |
| `DGRV3-Daily*-Text/`    | Baked text repository             |
| `DGRV3-Daily*-Log/`     | Build log repository              |
| `DGRV3-Daily*-SPC/`     | SPC repository                    |
| `Baked/`                | Baked TextTools output            |
| `Distribute/`           | Text patch distribution output    |
| `Distribute-GFX/`       | ImageTools distribution output    |
| `Distribute-Font/`      | FontTools distribution output     |
| `ModifiedFiles/`        | Modified text SPC/PB output       |
| `ModifiedFiles-GFX/`    | Modified graphics SPC/AB output   |
| `ModifiedFiles-Font/`   | Modified font SPC output          |
| `LatestAutomaticBuild/` | Platform-specific assembled build |
| `percentage_res.txt`    | Translation progress input        |
| `different_lines.txt`   | May originate from TextTools      |
| `vgit_failed.txt`       | Beta clone failure marker         |
| `patches_url.txt`       | Patch repository/tag URL          |
| `text_url.txt`          | Text repository/tag URL           |
| `log_url.txt`           | Log repository/tag URL            |
| `spc_url.txt`           | SPC repository/tag URL            |

---

# 🧠 FINAL NOTES

V3DailyManager should not be confused with a normal "build script."

It is an entire **release automation system**.

Its responsibilities span:

```text
environment preparation
        ↓
source/toolchain acquisition
        ↓
configuration selection
        ↓
external compilation
        ↓
platform assembly
        ↓
artifact organization
        ↓
repository maintenance
        ↓
Git commit/tag/push
        ↓
external automation metadata
```

The most important architectural rule is:

> **V3DailyManager orchestrates; TextTools, ImageTools, and FontTools actually build.**

The most important operational rule is:

> **Run DAILY in a deliberately clean directory.**

And the most important developer rule is:

> **Treat the filesystem cleanup code as dangerous infrastructure, not ordinary utility code.**

The project was designed for Team DAIX's automated Danganronpa V3 translation/build workflow rather than casual manual modding. The source itself describes DAILY as a separate CI/CD-style system from DAIXTOOLS.

---

# 📚 RELATED COMPONENTS

V3DailyManager interacts directly with:

```text
TextTools
    └── StackedBooks

ImageTools
    └── Pianist

FontTools
    └── Monokuma

VGit / VGitUtils
    └── Git repository operations

DGRV3-Tools
    └── Toolchain + configuration distribution
```

For detailed implementation behavior, the relevant source files are:

```text
V3DailyManager.cpp
files.cpp
process.cpp
distribution.cpp
remote.cpp
```

Those files collectively define the actual DAILY pipeline.