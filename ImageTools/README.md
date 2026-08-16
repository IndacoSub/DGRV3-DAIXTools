# DGRV3-DAIXTOOLS — IMAGETOOLS
## Team DAIX, 2026

---

## 📌 Overview

**ImageTools** is the graphics-processing subsystem of **DGRV3-DAIXTOOLS**.

If you cannot read the code, this repository is **not** for you.

PLEASE **read the code** before asking any questions.

**Generative AI** was used in the creation of this project’s code, comments, documentation, and miscellaneous supporting files.  

Our translation was not generated or supported by AI.

ImageTools' purpose is to automate the complete workflow required to take a graphics repository containing modified PNG/TGA assets and turn those modifications into game-ready resources.

Depending on the target platform, ImageTools produces either:

```text
PC / Xbox
PNG/TGA
   ↓
SRD
   ↓
SPC
````

or:

```text
Nintendo Switch / Unity
PNG/TGA
   ↓
AssetBundle / sharedassets
```

The resulting modified game assets can then either be retained as build output or converted into UPS patches for distribution.

ImageTools is therefore not simply an image converter.

It is an **end-to-end graphics build pipeline**.

---

# ⚠️ IMPORTANT WARNING

**IMAGETOOLS IS NOT A CASUAL MODDING TOOL.**

This pipeline is designed for large-scale automated graphics modification.

It can:

* clone and modify repositories,
* download large binary assets,
* extract archives,
* overwrite generated assets,
* rebuild SPC archives,
* modify Unity AssetBundles,
* create temporary working copies,
* delete intermediate files,
* remove unchanged assets,
* and generate binary distribution patches.

Several stages assume that the input repository and base assets are clean.

**Always keep an untouched copy of your source assets.**

If you only want to replace a handful of textures manually, use **Harmony-Tools** or another dedicated asset editor instead.

ImageTools is intended for people who need:

* large-scale graphics modification,
* automated compilation,
* automated SPC/AB rebuilding,
* reproducible builds,
* FileOnDemand asset acquisition,
* platform-specific compilation,
* or automated distribution patch generation.

And, frankly:

> **It is industrial-strength overkill for a meme mod.**

---

# 🧩 Architecture

ImageTools is composed of five major stages:

```text
                         ┌──────────────────┐
                         │     PIANIST      │
                         │ Master Orchestrator│
                         └────────┬─────────┘
                                  │
                                  ▼
                         ┌──────────────────┐
                         │    ADVENTURER    │
                         │ Repository Setup │
                         └────────┬─────────┘
                                  │
                                  ▼
                         ┌──────────────────┐
                         │     INVENTOR     │
                         │ Tool/Asset Setup │
                         └────────┬─────────┘
                                  │
                                  ▼
                         ┌──────────────────┐
                         │       MAID       │
                         │ Image Compilation│
                         └────────┬─────────┘
                                  │
                                  ▼
                         ┌──────────────────┐
                         │    TENNISPRO     │
                         │ Output Collector │
                         └────────┬─────────┘
                                  │
                           DISTRIBUTE mode
                                  │
                                  ▼
                         ┌──────────────────┐
                         │    DETECTIVE     │
                         │   UPS Generator  │
                         └──────────────────┘
```

The canonical order is:

```text
0. Pianist
1. Adventurer
2. Inventor
3. Maid
4. TennisPro
5. Detective     ← DISTRIBUTE mode
```

The individual tools can be run independently for debugging, but **Pianist is the intended master entry point**.

---

# 🔄 Complete Data Flow

The most useful way to understand ImageTools is to follow the assets.

```text
                    GitHub graphics repository
                              │
                              ▼
                         Adventurer
                              │
                              │ clone / download / merge
                              ▼
                     DGRV3-GFX / DGRV3-AB-GFX
                              │
                              ▼
                           Inventor
                              │
                              │ extract tools
                              │ prepare base assets
                              │ create working copy
                              ▼
                  Prepared graphics workspace
                              │
                              ▼
                             Maid
                              │
                  ┌───────────┴───────────┐
                  │                       │
                  ▼                       ▼
             PC / Xbox              Switch / Unity
                  │                       │
              PNG/TGA                  PNG/TGA
                  │                       │
                  ▼                       ▼
                 SRD             AssetBundle/assets
                  │                       │
                  ▼                       ▼
                 SPC                AB/sharedassets
                  │                       │
                  └───────────┬───────────┘
                              ▼
                         TennisPro
                              │
                              │ clean/filter/copy
                              ▼
                     ModifiedFiles-GFX/
                              │
                       DISTRIBUTE mode
                              │
                              ▼
                         Detective
                              │
                              │ original vs modified
                              ▼
                       Distribute-GFX/
                              │
                              ▼
                           *.ups
```

This separation is intentional.

The build stages produce **complete modified game assets**.

The distribution stage produces **patches containing only the differences**.

---

# 🐻 PIANIST — MASTER ORCHESTRATOR

**Pianist** is the top-level ImageTools orchestrator.

It does not perform image compilation itself.

Instead, it runs the ImageTools components in their required order and handles the overall pipeline lifecycle.

Its responsibilities include:

* checking dependencies,
* starting Adventurer,
* starting Inventor,
* starting Maid,
* starting TennisPro,
* conditionally starting Detective,
* monitoring stage failures,
* and producing the overall ImageTools workflow.

Conceptually:

```text
Pianist
  │
  ├── Adventurer
  ├── Inventor
  ├── Maid
  ├── TennisPro
  └── Detective (distribution)
```

Pianist is the **conductor**, not the compiler.

See:

`Pianist/README.md`

---

# 1. ADVENTURER — REPOSITORY ACQUISITION

**Adventurer** is the repository bootstrap stage.

It determines which graphics repository is required and acquires it.

The repository depends on the target platform:

| Platform        | Repository     |
| --------------- | -------------- |
| PC / Steam      | `DGRV3-GFX`    |
| Xbox            | `DGRV3-GFX`    |
| Nintendo Switch | `DGRV3-AB-GFX` |

Adventurer can operate through:

* Git cloning,
* FileOnDemand ZIP acquisition,
* beta-branch handling,
* branch merging,
* and repository preparation.

It does not perform the actual image compilation.

Its output is the graphics repository that Inventor will prepare.

See:

`Adventurer/README.md`

---

# 2. INVENTOR — EXTRACTION & WORKSPACE PREPARATION

**Inventor** prepares everything required by Maid.

Its responsibilities include:

* extracting downloaded repository archives,
* extracting platform-specific tools,
* preparing base asset folders,
* creating `danganronpa_files_copy`,
* and ensuring extracted tools have appropriate permissions.

For PC/Xbox, Inventor prepares tools such as:

```text
SPCTool
SRDTool
```

For Switch, it prepares:

```text
UAFGJ
```

The working directory produced by Inventor is consumed by Maid and later stages.

See:

`Inventor/README.md`

---

# 3. MAID — MASTER IMAGE COMPILATION DISPATCHER

**Maid** is the central compilation dispatcher.

Maid does not itself perform all image processing.

Instead, it:

1. reads `ImageConfig.config`,
2. determines the target platform,
3. selects the correct graphics repository,
4. selects the correct base folder,
5. configures the cloud repository name,
6. dispatches to the correct compilation subsystem.

The platform decision is:

```text
UseSwitchConfiguration = true
        │
        ▼
DGRV3-AB-GFX
base_ab
Console::CompileConsole()
```

Otherwise:

```text
UseSwitchConfiguration = false
        │
        ▼
DGRV3-GFX
base_spc
PC::CompilePC()
```

Xbox and PC both use the PC compilation subsystem.

See:

`Maid/README.md`

---

# 🖥️ PC / XBOX PIPELINE

The PC/Xbox compiler processes graphics using the SPC-based workflow.

Conceptually:

```text
PNG / TGA
    │
    ▼
 SRDTool
    │
    ▼
   SRD
    │
    ▼
 SpcTool
    │
    ▼
   SPC
```

The PC compilation subsystem handles considerably more than this simplified diagram suggests.

It can include:

* image discovery,
* ALT file resolution,
* SRD alias handling,
* Matryoshka SPC handling,
* missing SPC acquisition,
* SRD generation,
* SPC rebuilding,
* minigame asset handling,
* cleanup of unchanged output,
* and optional multithreaded compilation.

The exact behavior is controlled by configuration and repository state.

---

# 🎮 SWITCH / UNITY PIPELINE

The Switch pipeline is fundamentally different.

Instead of rebuilding SPC archives, it operates on Unity resources.

Conceptually:

```text
PNG / TGA
    │
    ▼
UAFGJ / Unity processing
    │
    ├── AssetBundle
    └── sharedassets
```

The console compiler handles:

* AssetBundle injection,
* `.assets` / sharedassets modification,
* UAFGJ,
* FileOnDemand AB/asset acquisition,
* multithreaded compilation,
* and SDF font injection into `sharedassets0.assets`.

The Switch pipeline therefore should not be thought of as a variation of the SPC compiler.

It is a separate compilation backend selected by Maid.

---

# 4. TENNISPRO — OUTPUT COLLECTION & CLEANUP

**TennisPro** takes the output of Maid and converts it into the clean build-output directory:

```text
ModifiedFiles-GFX/
```

Its responsibilities include:

* scanning compiled output,
* removing empty files,
* removing unwanted repository metadata,
* copying compiled SPC/AB assets,
* and preparing a clean distribution-ready asset tree.

TennisPro is therefore the **output sanitation stage**.

Conceptually:

```text
Maid output
    │
    ▼
TennisPro
    │
    ├── remove empty files
    ├── remove metadata
    └── collect modified assets
    │
    ▼
ModifiedFiles-GFX/
```

See:

`TennisPro/README.md`

---

# 5. DETECTIVE — UPS PATCH GENERATION

**Detective** is the optional final distribution stage.

It is normally invoked only when distribution mode is enabled.

Detective takes:

```text
original assets
```

and:

```text
ModifiedFiles-GFX/
```

and produces:

```text
Distribute-GFX/
    *.ups
```

The process is conceptually:

```text
Original asset
      │
      ├──────────────┐
      │              │
      ▼              ▼
 base version    modified version
      │              │
      └──────┬───────┘
             ▼
         Detective
             │
             ▼
          *.ups
```

The original and modified assets are temporarily paired and compared.

After patch generation, Detective removes non-UPS files from the distribution directory.

See:

`Detective/README.md`

---

# 📁 REPOSITORIES

ImageTools operates on two different graphics repositories.

---

## `DGRV3-GFX`

Used by:

* PC / Steam
* Xbox

A typical repository contains areas such as:

```text
DGRV3-GFX/
│
├── base_spc/
├── boot/
├── flash/
├── MANUAL/
├── minigame/
├── noimages/
└── ...
```

`base_spc` contains the tools and base resources required for the SPC workflow.

The exact repository contents should always be verified against the source code.

---

## `DGRV3-AB-GFX`

Used by:

* Nintendo Switch
* Unity-based graphics processing

A typical structure contains:

```text
DGRV3-AB-GFX/
│
├── base_ab/
├── Data/
└── ...
```

`base_ab` contains the Switch-specific tools and base assets.

---

# 📂 `base_spc`

`base_spc` is the working resource/tool directory for the PC/Xbox pipeline.

It can contain archives and executables such as:

```text
7za.exe / 7za
SPCTool.7z
SPCTool_Linux.7z
SRDTool.7z
SRDTool_Linux.7z
NewSTXTool.7z
NewSTXTool_Linux.7z
ups.exe / ups
```

The exact extracted contents depend on the platform and build configuration.

---

# 📂 `base_ab`

`base_ab` is the equivalent working directory for Switch/Unity builds.

It contains the tools required by the console compilation pipeline, including:

```text
UAFGJ.7z
```

and the other platform-independent helper tools required by the configured workflow.

---

# 📂 `danganronpa_files_copy`

`danganronpa_files_copy` is a particularly important working directory.

Inventor creates it as a working copy of the graphics repository when appropriate.

Later stages use it as the source/reference workspace.

Conceptually:

```text
Graphics repository
        │
        ▼
Inventor
        │
        ▼
danganronpa_files_copy/
        │
        ├── Maid
        ├── TennisPro
        └── Detective
```

The purpose of maintaining a copy is to avoid treating the original repository tree as the only working reference.

---

# 📤 `ModifiedFiles-GFX`

`ModifiedFiles-GFX` is the primary compiled-output directory.

It contains the game assets that ImageTools considers relevant after compilation and cleanup.

Depending on platform, it can contain:

```text
PC / Xbox:
    *.spc

Switch:
    *.ab
    *.assets
```

The directory may preserve the original repository folder hierarchy.

For example:

```text
ModifiedFiles-GFX/
│
├── flash/
│   └── adv/
│       └── ...
│
├── trial/
│   └── ...
│
└── Data/
    ├── sharedassets0.assets
    ├── sharedassets1.assets
    └── ...
```

This is the output used by Detective for distribution builds.

---

# 📦 `Distribute-GFX`

`Distribute-GFX` is the final patch-distribution directory.

It is produced by Detective.

Before cleanup it can temporarily contain:

```text
*_normal.spc
*_normal.ab
*_normal.assets

modified.spc
modified.ab
modified.assets
```

After Detective finishes, only UPS patches should remain:

```text
Distribute-GFX/
│
├── *_patch.ups
├── *_patch.ups
└── ...
```

This separation is important:

```text
ModifiedFiles-GFX/
    = complete modified assets

Distribute-GFX/
    = distributable binary patches
```

---

# ⚙️ CENTRAL CONFIGURATION

ImageTools uses:

```text
ImageConfig.config
```

as its central configuration file.

The configuration must be available in the expected working directory.

Maid reads it before selecting the compilation backend. 

The configuration controls major behaviors such as:

* platform selection,
* FileOnDemand,
* Xbox mode,
* multithreaded compilation,
* repository selection,
* and related cloud/download settings.

---

# 🖥️ PLATFORM CONFIGURATION

## PC

Typical configuration:

```text
UseSwitchConfiguration = false
UseXboxConfiguration   = false
```

Results in:

```text
Repository:
DGRV3-GFX

Base:
base_spc

Compiler:
PC::CompilePC()
```

---

## Xbox

Typical configuration:

```text
UseSwitchConfiguration = false
UseXboxConfiguration   = true
```

Results in:

```text
Repository:
DGRV3-GFX

Base:
base_spc

Compiler:
PC::CompilePC()
```

The Xbox build uses the PC compilation implementation while selecting the Xbox cloud repository configuration.

---

## Nintendo Switch

Typical configuration:

```text
UseSwitchConfiguration = true
```

Results in:

```text
Repository:
DGRV3-AB-GFX

Base:
base_ab

Compiler:
Console::CompileConsole()
```

---

# ☁️ FILEONDEMAND

`FileOnDemand` changes how ImageTools obtains large base assets.

When FileOnDemand is enabled, the compilation pipeline can request missing assets from the configured cloud repository rather than requiring the entire asset set to already exist locally.

This is particularly useful for repositories containing very large binary resources.

Conceptually:

```text
Asset required
     │
     ▼
Exists locally?
   │       │
  YES      NO
   │       │
   │       ▼
   │   FileOnDemand
   │       │
   │       ▼
   │   Cloud download
   │
   └───────┴──────► Compiler
```

The appropriate cloud repository is selected by Maid.

Maid sets:

```cpp
Cloud::dl_repo_name
```

according to platform. 

---

# 🧵 MULTITHREADED COMPILATION

ImageTools supports multithreaded compilation.

The relevant configuration option is:

```text
MultithreadedCompilation
```

When enabled, the platform-specific compilation subsystems can process independent compilation jobs concurrently.

This is important for large graphics repositories where compiling hundreds or thousands of assets sequentially would be unnecessarily slow.

The feature is handled by the compilation backends rather than by Maid itself.

Maid simply passes control to the appropriate compiler.

---

# 📦 REQUIRED TOOLS

Depending on platform and configuration, ImageTools may require:

```text
7za
SPCTool
SRDTool
NewSTXTool
UPS
UAFGJ
```

The repository normally provides these as archives rather than shipping the unpacked executables directly.

Typical archives include:

```text
7za.exe / 7za

SPCTool.7z
SPCTool_Linux.7z

SRDTool.7z
SRDTool_Linux.7z

NewSTXTool.7z
NewSTXTool_Linux.7z

ups.exe / ups
```

Switch builds additionally require:

```text
UAFGJ.7z
```

These dependencies are intentionally not bundled with DAIXTOOLS.

---

# 🖥️ WINDOWS / LINUX

ImageTools supports platform-specific executable layouts.

Typical Windows executables:

```text
7za.exe
ups.exe
SPCTool.exe
...
```

Typical Linux executables:

```text
7za
ups
SPCTool
...
```

Inventor also handles executable permissions on Linux.

Do not assume a Windows executable can simply be substituted for its Linux counterpart.

---

# 🔗 STAGE CONTRACTS

ImageTools works because each stage produces the information and files expected by the next.

The most important hand-offs are:

```text
Adventurer
    │
    │ graphics repository
    ▼
Inventor
    │
    │ extracted tools
    │ prepared working copy
    ▼
Maid
    │
    │ compiled SPC/AB/assets
    ▼
TennisPro
    │
    │ ModifiedFiles-GFX/
    ▼
Detective
    │
    │ original + modified
    ▼
Distribute-GFX/*.ups
```

The stages should therefore not be arbitrarily reordered.

For example:

```text
Maid → Detective
```

is not equivalent to:

```text
Maid → TennisPro → Detective
```

because Detective expects the cleaned modified-file set produced by TennisPro.

---

# 🧨 DESTRUCTIVE OPERATIONS

ImageTools is not a read-only pipeline.

Different stages may:

### Adventurer

* delete failed repository clones,
* recreate repositories,
* merge branches,
* modify Git state.

### Inventor

* extract over working directories,
* create working copies,
* manipulate extracted tool directories.

### Maid

* overwrite generated SRD/SPC/AB resources,
* rebuild archives,
* clean unchanged or temporary compilation output.

### TennisPro

* remove empty files,
* remove repository metadata,
* overwrite distribution files.

### Detective

* create temporary base/modified pairs,
* rename base files,
* generate UPS files,
* delete everything except final patches.

Therefore:

> **Never treat the build workspace as your backup.**

---

# 🧹 CLEAN BUILD PRINCIPLE

The safest workflow is:

```text
Clean repository
       │
       ▼
Adventurer
       │
       ▼
Fresh graphics repository
       │
       ▼
Inventor
       │
       ▼
Fresh working copy
       │
       ▼
Maid
       │
       ▼
TennisPro
       │
       ▼
Detective
```

Avoid repeatedly running the pipeline on a partially modified workspace without understanding which stage owns which files.

This is especially important for SPC and Unity assets.

---

# 📊 RESPONSIBILITY MATRIX

| Responsibility             | Pianist | Adventurer | Inventor | Maid | TennisPro | Detective |
| -------------------------- | :-----: | :--------: | :------: | :--: | :-------: | :-------: |
| Orchestrate pipeline       |    ✅    |            |          |      |           |           |
| Clone graphics repo        |         |      ✅     |          |      |           |           |
| Download repository ZIP    |         |      ✅     |          |      |           |           |
| Merge beta branches        |         |      ✅     |          |      |           |           |
| Extract archives           |         |            |     ✅    |      |           |           |
| Prepare tools              |         |            |     ✅    |      |           |           |
| Create working copy        |         |            |     ✅    |      |           |           |
| Select platform compiler   |         |            |          |   ✅  |           |           |
| Compile PC/Xbox graphics   |         |            |          |   ✅  |           |           |
| Compile Switch graphics    |         |            |          |   ✅  |           |           |
| Generate SRDs              |         |            |          |   ✅  |           |           |
| Rebuild SPCs               |         |            |          |   ✅  |           |           |
| Inject Unity assets        |         |            |          |   ✅  |           |           |
| Collect modified assets    |         |            |          |      |     ✅     |           |
| Clean output               |         |            |          |      |     ✅     |           |
| Generate UPS patches       |         |            |          |      |           |     ✅     |
| Final distribution cleanup |         |            |          |      |           |     ✅     |

---

# 🧠 DESIGN PHILOSOPHY

ImageTools follows several major design principles.

## Automation

Manual repetition is eliminated wherever possible.

The pipeline is intended to process large numbers of assets without requiring the developer to individually invoke conversion tools.

---

## Separation of Responsibilities

Each component has a relatively focused role:

```text
Adventurer → acquire
Inventor   → prepare
Maid       → compile
TennisPro  → collect
Detective  → patch
Pianist    → orchestrate
```

This allows individual components to be debugged without rewriting the entire pipeline.

---

## Platform Abstraction

The user does not need to manually choose different compilation executables for each platform.

Instead:

```text
ImageConfig.config
        │
        ▼
      Maid
        │
        ├── PC/Xbox
        │      ↓
        │   PC compiler
        │
        └── Switch
               ↓
          Console compiler
```

Maid centralizes that decision.

---

## Reproducibility

The intended build model is:

```text
Same repository
+
Same configuration
+
Same base assets
+
Same tool versions
=
Same build
```

In practice, exact reproducibility also depends on:

* external tool versions,
* repository revisions,
* branch selection,
* downloaded assets,
* platform,
* and configuration.

For release builds, preserve the exact tool versions used.

---

# ⚡ PERFORMANCE

ImageTools can process very large numbers of assets.

The major performance characteristics are:

### Repository / asset acquisition

Primarily network and I/O bound.

---

### Extraction

Primarily I/O bound.

---

### Image compilation

Can be CPU-heavy, particularly when generating SRD/SPC resources.

---

### SPC rebuilding

Primarily I/O-heavy.

---

### Unity AssetBundle processing

Can involve both CPU and I/O costs depending on the asset set.

---

### UPS generation

Generally much lighter than complete asset compilation because it operates on already-generated binaries.

---

## Multithreading

Unlike the older documentation may suggest, ImageTools **does support multithreaded compilation**.

The feature is controlled by:

```text
MultithreadedCompilation
```

and is implemented by the compilation backends rather than by Pianist itself.

---

# 📦 OUTPUTS

A normal ImageTools build can produce:

```text
ModifiedFiles-GFX/
```

containing complete modified game assets.

A distribution build additionally produces:

```text
Distribute-GFX/
```

containing UPS patches.

Other diagnostic artifacts may include:

```text
imagetools_log.txt
vgit_failed.txt
adventurer_failed.txt
```

and tool-specific logs or temporary files.

---

# 📄 `imagetools_log.txt`

This is the primary high-level ImageTools log.

It is useful for determining:

* which stage failed,
* whether a repository was acquired,
* whether compilation started,
* and whether the final pipeline completed.

For component-specific failures, consult the individual tool's logs and README.

---

# 📄 `vgit_failed.txt`

This indicates a repository acquisition/merge failure.

If present after Adventurer runs, the graphics repository should be considered suspect until the underlying failure is resolved.

Common causes include:

* invalid credentials,
* inaccessible private repositories,
* Git/network failures,
* branch merge conflicts,
* or incomplete repository acquisition.

---

# 📄 `adventurer_failed.txt`

This is a higher-level indication that the Adventurer stage failed.

If it exists, downstream stages should not be assumed to have valid input.

---

# 🚨 FAILURE HANDLING

ImageTools is designed as a staged pipeline.

The general dependency chain is:

```text
Adventurer failure
       ↓
Inventor cannot safely continue
       ↓
Maid cannot safely compile
       ↓
TennisPro cannot collect valid output
       ↓
Detective cannot generate valid patches
```

Likewise:

```text
Maid failure
       ↓
TennisPro output may be incomplete
       ↓
Detective must not be trusted
```

The correct response to a failure is to fix the failing stage rather than simply forcing later stages to run.

---

# 🔍 TROUBLESHOOTING

## "Repository could not be cloned"

Check:

* GitHub credentials,
* repository name,
* repository owner,
* network connectivity,
* beta branch configuration,
* and `vgit_failed.txt`.

---

## "main.zip could not be downloaded"

If FileOnDemand/repository ZIP acquisition is being used:

* verify network connectivity,
* verify GitHub credentials/token configuration,
* check GitHub rate limits,
* and verify that the repository exists.

---

## "7za not found"

Verify that:

```text
7za.exe
```

or:

```text
7za
```

is available where Inventor expects it.

---

## "SPCTool not found"

Check:

```text
base_spc/
```

and verify that the appropriate SPCTool archive was supplied and successfully extracted.

---

## "SRDTool not found"

Check:

```text
base_spc/
```

for the required SRDTool archive.

---

## "UAFGJ not found"

This normally indicates a Switch configuration problem or an incomplete `base_ab` setup.

Verify:

```text
UseSwitchConfiguration=true
```

and ensure:

```text
UAFGJ.7z
```

is available.

---

## "Textures were not rebuilt"

Check:

1. Maid selected the correct platform.
2. The graphics repository contains the expected files.
3. Inventor prepared the working directory.
4. The required compiler tools were extracted.
5. FileOnDemand did not fail to acquire missing assets.
6. The Maid/compilation logs for the specific asset.

---

## `ModifiedFiles-GFX` is empty

Possible causes include:

* Maid did not produce modifications,
* TennisPro filtered out empty/invalid files,
* no assets were actually changed,
* the wrong repository was selected,
* or the build failed before output collection.

Check Maid before debugging TennisPro.

---

## `Distribute-GFX` is empty

If Detective ran but generated no patches, verify:

* `ModifiedFiles-GFX/` contains modified assets,
* the original assets are available,
* the base/reference paths are correct,
* the UPS executable is present,
* and the files are actually different from their originals.

An empty distribution directory does **not necessarily mean Detective failed**.

It may mean there were no differences to package.

---

# 🧪 DEBUGGING INDIVIDUAL STAGES

Although Pianist is the intended entry point, each component can be useful independently during development.

A useful debugging strategy is:

```text
Adventurer
    ↓
verify repository

Inventor
    ↓
verify tools + workspace

Maid
    ↓
verify compilation

TennisPro
    ↓
verify output

Detective
    ↓
verify patches
```

Do not immediately debug Detective if Maid failed.

Do not debug Maid if Inventor never prepared the repository correctly.

The pipeline's stages are intentionally hierarchical.

---

# 📚 INDIVIDUAL DOCUMENTATION

This README is the **architectural overview**.

The individual tools have dedicated documentation covering implementation-level behavior.

| Tool           | Role                                              |
| -------------- | ------------------------------------------------- |
| **Pianist**    | Master orchestrator                               |
| **Adventurer** | Repository acquisition                            |
| **Inventor**   | Tool/archive extraction and workspace preparation |
| **Maid**       | Platform dispatcher and compilation entry point   |
| **TennisPro**  | Output collection and cleanup                     |
| **Detective**  | UPS patch generation                              |

Read the individual README before modifying, replacing, or debugging a specific component.

---

# 🧭 ImageTools at a Glance

```text
                       ┌──────────────┐
                       │   Pianist    │
                       │ Orchestrator │
                       └──────┬───────┘
                              │
                              ▼
                       ┌──────────────┐
                       │  Adventurer  │
                       │ Acquire Repo │
                       └──────┬───────┘
                              │
                              ▼
                       ┌──────────────┐
                       │   Inventor   │
                       │ Prepare Data │
                       └──────┬───────┘
                              │
                              ▼
                       ┌──────────────┐
                       │     Maid     │
                       │    Compile   │
                       └──────┬───────┘
                              │
                 ┌────────────┴────────────┐
                 │                         │
                 ▼                         ▼
             PC / Xbox                 Switch
                 │                         │
             PNG/TGA                   PNG/TGA
                 │                         │
                 ▼                         ▼
                SRD                AssetBundle/assets
                 │                         │
                 ▼                         ▼
                SPC                 AB/sharedassets
                 │                         │
                 └────────────┬────────────┘
                              │
                              ▼
                       ┌──────────────┐
                       │  TennisPro   │
                       │ Collect/Clean│
                       └──────┬───────┘
                              │
                              ▼
                    ModifiedFiles-GFX/
                              │
                       DISTRIBUTE mode
                              │
                              ▼
                       ┌──────────────┐
                       │  Detective   │
                       │  UPS Patches │
                       └──────┬───────┘
                              │
                              ▼
                       Distribute-GFX/
                              │
                              ▼
                            *.ups
```

---

# 🏁 Summary

**ImageTools is Team DAIX's automated graphics build and distribution pipeline for Danganronpa V3.**

Its workflow is:

```text
Adventurer
    ↓
Acquire graphics repository
    ↓
Inventor
    ↓
Extract tools + prepare workspace
    ↓
Maid
    ↓
Compile graphics
    ↓
TennisPro
    ↓
Clean + collect modified assets
    ↓
Detective
    ↓
Generate UPS patches
```

With platform-specific compilation:

```text
PC / Xbox
    PNG/TGA
       ↓
      SRD
       ↓
      SPC
```

and:

```text
Switch / Unity
    PNG/TGA
       ↓
 AssetBundle
       ↓
sharedassets / .assets
```

The fundamental distinction is:

```text
BUILD
  ↓
complete modified game assets

DISTRIBUTION
  ↓
binary UPS patches
```

ImageTools is designed around:

* **automation**
* **platform-aware compilation**
* **large-scale asset processing**
* **reproducibility**
* **FileOnDemand acquisition**
* **SPC/AssetBundle rebuilding**
* **clean output collection**
* **minimal patch-based distribution**

It is not the easy way to replace a texture.

It is the automated way to rebuild the graphics pipeline.

If you want the easy path:

> **Use Harmony-Tools.**

If you want the complete graphics build system:

> **Welcome to ImageTools.**

````

One correction I would make **specifically compared with your current README** is the performance section. Your current top-level README says:

> `ImageTools does not use multithreading.`

That is now contradicted by the actual Maid documentation: `MultithreadedCompilation` explicitly enables parallel SRD/SPC/AB compilation. :contentReference[oaicite:7]{index=7}

So I would definitely fix that rather than merely polishing the prose.

I also like the idea of keeping **TennisPro as the ImageTools equivalent of FontTools' Monokid**, but I'd phrase it as a *conceptual equivalent*, not imply that their implementations are interchangeable. TennisPro collects/cleans graphics output, while Monokid's filtering logic is more specifically tied to the changed-SPC list.

And the biggest architectural improvement is making the README explain this distinction clearly:

```text
                    IMAGE BUILD
                        │
                        ▼
              ModifiedFiles-GFX/
                        │
                        │ DISTRIBUTE
                        ▼
                  PATCH BUILD
                        │
                        ▼
                Distribute-GFX/
                        │
                        ▼
                      *.ups
````

That makes the role of **Pianist → Adventurer → Inventor → Maid → TennisPro → Detective** immediately understandable without forcing somebody to read six separate READMEs first.
