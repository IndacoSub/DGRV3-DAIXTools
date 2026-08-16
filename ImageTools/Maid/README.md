# IMAGETOOLS - MAID
## Team DAIX, 2026

---

## 📌 Overview

**Maid** is the central entry point and dispatcher for **ImageTools**.

Unlike the lower-level ImageTools components, Maid is not primarily responsible for manipulating image data itself. Its job is to determine **which image-compilation pipeline should run**, configure that pipeline for the requested platform, and hand execution over to the appropriate subsystem.

In practical terms:

```text
ImageTools
    │
    ▼
   Maid
    │
    ├── PC / Steam ──────► PC::CompilePC()
    │
    ├── Xbox ────────────► PC::CompilePC()
    │
    └── Switch / Unity ──► Console::CompileConsole()
````

Maid therefore acts as the **orchestration layer** between the global ImageTools configuration and the platform-specific compilation implementations.

The two major pipelines are:

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
Nintendo Switch / Unity
PNG/TGA
   ↓
AssetBundle / sharedassets
```

Maid chooses between these pipelines before any actual image compilation begins. 

---

# 🎯 Responsibilities

Maid has a deliberately small set of high-level responsibilities.

### 1. Determine the current working directory

Maid uses:

```cpp
std::filesystem::current_path()
```

as the root of its operation.

This means ImageTools is designed to be launched from the directory containing the configuration and supporting tools/repositories.

---

### 2. Load `ImageConfig.config`

Maid constructs:

```text
<ImageTools working directory>/ImageConfig.config
```

and passes it to:

```cpp
Configuration::ReadConfig()
```

It then calls:

```cpp
Configuration::ViewDebugCurrentConfig()
```

to display the active configuration. 

---

### 3. Select the graphics repository

Maid chooses between:

```text
DGRV3-GFX
```

and:

```text
DGRV3-AB-GFX
```

depending on the active platform configuration.

---

### 4. Select the base folder

Maid chooses between:

```text
base_spc
```

and:

```text
base_ab
```

The former is used for PC/Xbox's SPC-based pipeline.

The latter is used for Switch/Unity's AssetBundle-based pipeline.

---

### 5. Configure the cloud repository

Maid initializes:

```cpp
Cloud::dl_repo_name
```

with the appropriate platform-specific repository name.

This is important because both PC and console compilation pipelines can use **FileOnDemand** downloading when their required base assets are not present locally.

---

### 6. Dispatch the correct compiler

Finally, Maid calls either:

```cpp
PC::CompilePC(...)
```

or:

```cpp
Console::CompileConsole(...)
```

depending on the configuration.

---

### 7. Finalize execution

After the selected pipeline returns, Maid logs:

```text
All done!
```

and calls:

```cpp
Common::WaitExit();
```

so the application does not immediately close. 

---

# 🧩 Maid's Place in ImageTools

Maid should be thought of as the **front door** to the image compilation system.

A simplified ImageTools architecture is:

```text
                         ImageTools
                             │
                             ▼
                           Maid
                             │
                 ┌───────────┴───────────┐
                 │                       │
                 ▼                       ▼
             PC::CompilePC       Console::CompileConsole
                 │                       │
        ┌────────┴────────┐       ┌──────┴──────┐
        │                 │       │             │
        ▼                 ▼       ▼             ▼
      SRDTool          SpcTool  UAFGJ       Unity assets
        │                 │       │             │
        ▼                 ▼       └──────┬──────┘
       SRD               SPC             │
                                         ▼
                                  .ab / .assets
```

Maid itself does not perform the individual image insertions.

Instead, it establishes the environment in which those operations happen.

---

# 🏗️ Architecture

The Maid source is split into several logical components.

```text
Maid.cpp
│
├── program entry point
├── configuration loading
├── platform selection
├── repository selection
├── cloud repository selection
├── base-folder selection
└── pipeline dispatch
       │
       ├───────────────┐
       ▼               ▼
compile_pc.cpp    compile_console.cpp
       │               │
       ▼               ▼
PC pipeline       Console pipeline
```

The shared structural definitions are provided by:

```text
Maid.h
```

while the platform-specific declarations are provided by:

```text
compile_pc.h
compile_console.h
```

---

# 📁 Core Source Files

| File                  | Responsibility                                           |
| --------------------- | -------------------------------------------------------- |
| `Maid.cpp`            | Main dispatcher and application entry point              |
| `Maid.h`              | Shared `FileStructure` and compilation-batch structures  |
| `compile_pc.cpp`      | PC/Xbox PNG → SRD → SPC pipeline                         |
| `compile_pc.h`        | PC pipeline declarations and special-case lookup tables  |
| `compile_console.cpp` | Switch/Unity PNG/TGA → AssetBundle/sharedassets pipeline |
| `compile_console.h`   | Console pipeline declarations                            |
| `ImageConfig.config`  | Runtime configuration                                    |
| `Common/Config.h`     | Configuration infrastructure                             |
| `Common/Cloud.h`      | Cloud/download configuration and repository information  |
| `SrdTool`             | Inserts images into SRD files                            |
| `SpcTool`             | Inserts SRD/SRDV data into SPC archives                  |
| `UAFGJ`               | Injects assets into Unity `.ab` / `.assets` files        |

---

# ⚙️ Configuration

Maid's behavior is primarily controlled through:

```text
ImageConfig.config
```

The most important configuration values for Maid are:

```text
UseSwitchConfiguration
UseXboxConfiguration
FileOnDemand
MultithreadedCompilation
UseALTs
```

Not every one of these is consumed directly by `Maid.cpp`.

Some are forwarded into the platform-specific compilation pipeline and influence its internal behavior.

---

# 🎮 Platform Selection

The most important configuration setting is:

```text
UseSwitchConfiguration
```

Maid effectively performs:

```cpp
if (UseSwitchConfiguration) {
    // Switch / Unity
}
else {
    // PC / Xbox
}
```

There is **no independent `UsePCConfiguration` flag** in Maid.

PC is the default when Switch is disabled and Xbox is not selected.

---

# 🟦 Nintendo Switch / Unity

When:

```text
UseSwitchConfiguration = true
```

Maid selects:

```text
Repository:
DGRV3-AB-GFX
```

and:

```text
Base folder:
base_ab
```

It also selects:

```cpp
Cloud::dl_repo_name = Cloud::dl_repo_name_switch;
```

and dispatches:

```cpp
Console::CompileConsole(current_dir, repo, basefolder);
```

This is the Unity-based image pipeline. 

---

# 🟩 PC / Steam

When:

```text
UseSwitchConfiguration = false
UseXboxConfiguration = false
```

Maid selects:

```text
Repository:
DGRV3-GFX
```

and:

```text
Base folder:
base_spc
```

The cloud repository becomes:

```cpp
Cloud::dl_repo_name = Cloud::dl_repo_name_pc;
```

and Maid calls:

```cpp
PC::CompilePC(current_dir, repo, basefolder);
```

This is the standard PC/Steam SPC pipeline.

---

# 🟧 Xbox

When:

```text
UseSwitchConfiguration = false
UseXboxConfiguration = true
```

Maid still uses:

```text
Repository:
DGRV3-GFX
```

and:

```text
Base folder:
base_spc
```

The difference is the cloud repository:

```cpp
Cloud::dl_repo_name = Cloud::dl_repo_name_xbox;
```

The compiler is still:

```cpp
PC::CompilePC(...)
```

Therefore Xbox and PC share the **same image compilation implementation**, with the cloud repository selection distinguishing the platform's base assets. 

---

# 📊 Platform Selection Table

| Switch  | Xbox    | Result         | Graphics Repo  | Base Folder | Cloud Repo | Compiler                  |
| ------- | ------- | -------------- | -------------- | ----------- | ---------- | ------------------------- |
| `true`  | either  | Switch / Unity | `DGRV3-AB-GFX` | `base_ab`   | Switch     | `Console::CompileConsole` |
| `false` | `true`  | Xbox           | `DGRV3-GFX`    | `base_spc`  | Xbox       | `PC::CompilePC`           |
| `false` | `false` | PC / Steam     | `DGRV3-GFX`    | `base_spc`  | PC         | `PC::CompilePC`           |

---

# ⚠️ Switch Takes Priority Over Xbox

If both:

```text
UseSwitchConfiguration = true
UseXboxConfiguration = true
```

are enabled, Maid enters the Switch branch.

The Xbox flag is only consulted inside the non-Switch branch.

Therefore:

```text
Switch = true
Xbox = true
```

does **not** result in an Xbox build.

It results in:

```text
Switch / Unity
```

This is an important configuration precedence rule.

---

# 🔐 Repository Names Are Passed Through `EncryptString`

The source does not directly assign the repository literals in ordinary form.

Instead it uses:

```cpp
EncryptString("DGRV3-GFX")
```

and:

```cpp
EncryptString("DGRV3-AB-GFX")
```

Likewise:

```cpp
EncryptString("base_spc")
```

and:

```cpp
EncryptString("base_ab")
```

are used for the base folders. 

The purpose is presumably to avoid keeping these strings as ordinary plaintext literals in the resulting executable.

For documentation purposes, however, their effective values are:

```text
DGRV3-GFX
DGRV3-AB-GFX
base_spc
base_ab
```

---

# 📂 Working Directory

Maid does not accept the repository location through a command-line argument.

Instead:

```cpp
current_dir = std::filesystem::current_path();
```

is used as the root.

The configuration file is therefore expected at:

```text
<current working directory>/ImageConfig.config
```

and the selected graphics repository is expected at:

```text
<current working directory>/DGRV3-GFX
```

or:

```text
<current working directory>/DGRV3-AB-GFX
```

depending on platform.

---

# 📥 Inputs

Maid's direct input is relatively small.

## Configuration

```text
ImageConfig.config
```

---

## Filesystem state

The selected graphics repository:

```text
DGRV3-GFX/
```

or:

```text
DGRV3-AB-GFX/
```

---

## Cloud configuration

Maid relies on `Common/Cloud.h` for:

* cloud repository names,
* GitHub repository information,
* branch information,
* authentication/token information,
* FileOnDemand download configuration.

The actual downloading is performed by the platform-specific compiler, not by Maid itself.

---

# 📤 Outputs

Maid itself does not produce a dedicated image file format.

Its outputs are primarily **side effects of the selected compiler**.

Depending on the selected pipeline, those outputs include:

### PC / Xbox

```text
.srd
.srdv
.spc
```

These are generated/modified by the PC compilation subsystem.

---

### Switch / Unity

```text
.ab
.assets
```

These are modified by the Unity asset injection subsystem.

---

### Console font injection

The Switch pipeline may additionally modify:

```text
Data/sharedassets0.assets
```

by injecting the configured SDF font data. 

---

# 🧠 Maid Does Not Compile Images Directly

This is perhaps the most important architectural point.

Maid does not contain the actual:

```text
PNG → SRD
SRD → SPC
PNG → Unity Asset
```

conversion algorithms.

Instead:

```text
Maid
    │
    └── selects compiler
            │
            ├── PC::CompilePC()
            │
            └── Console::CompileConsole()
```

The actual compilation logic lives in those modules.

This separation keeps the executable's entry point simple while allowing the PC and console implementations to remain highly specialized.

---

# 🖥️ PC/Xbox Pipeline

The PC compiler is considerably more complicated than the dispatcher itself.

Its high-level process is:

```text
DGRV3-GFX
    │
    ▼
HandleAltFiles()
    │
    ▼
PreprocessPNG()
    │
    ├── determine PNG → SRD
    ├── determine SRD → SPC
    ├── resolve aliases
    ├── resolve special SRDs
    └── detect nested SPC cases
    │
    ▼
CompileImages()
    │
    ├── download missing SPCs
    ├── sort by SRD
    ├── compile SRDs
    ├── sort by SPC
    └── compile SPCs
```

The PC pipeline explicitly describes itself as the:

```text
PNG → SRD → SPC
```

workflow. 

---

# 🖼️ PC/Xbox Image Flow

For a normal image, the conceptual flow is:

```text
PNG
 │
 ▼
SRDTool
 │
 ▼
SRD
 │
 ├── SRDV
 │
 ▼
SpcTool
 │
 ▼
SPC
```

SRDs are compiled before SPCs because the SPC compilation stage depends on the completed SRD output.

The implementation explicitly waits for SRD compilation before beginning SPC compilation. 

---

# 🧩 Shared `FileStructure`

The platform-specific pipelines use the shared:

```cpp
Common::FileStructure
```

type declared in:

```text
Maid.h
```

A `FileStructure` represents one image asset and stores the information necessary to route it through the appropriate compilation pipeline.

Its fields include:

```text
PNG_RepoName
PNG_ActualName
AssetOrBundle_Filename
SRD_Filename
SRDV_Filename
SPC_Filename
```

The Unity pipeline primarily uses:

```text
PNG_RepoName
PNG_ActualName
AssetOrBundle_Filename
```

while the PC/Xbox pipeline primarily uses:

```text
PNG_RepoName
PNG_ActualName
SRD_Filename
SRDV_Filename
SPC_Filename
```

This shared model allows both pipelines to use similar preprocessing concepts while targeting completely different container formats. 

---

# 🟪 Nintendo Switch / Unity Pipeline

When Switch configuration is active, Maid calls:

```cpp
Console::CompileConsole(...)
```

The console pipeline scans:

```text
DGRV3-AB-GFX
```

for:

```text
.png
.PNG
.tga
.TGA
```

assets.

It then determines which Unity container each image belongs to. 

---

# 📦 Unity Container Detection

The console pipeline examines the directory structure surrounding each image.

If the normalized path contains:

```text
sharedassets
```

the target is treated as:

```text
.assets
```

Otherwise it is treated as:

```text
.ab
```

The resulting path is stored in:

```cpp
FileStructure::AssetOrBundle_Filename
```

and tracked in a list of required containers. 

---

# 🚫 ALT Files on Console

The console preprocessing stage explicitly ignores files containing:

```text
ALT_
```

even if ALT functionality is enabled elsewhere.

This prevents alternate PC-style assets from accidentally entering the Unity pipeline. 

---

# 📥 FileOnDemand

One of Maid's most important indirect features is:

```text
FileOnDemand
```

When enabled, the platform-specific compiler downloads only the base containers that are actually required by the images being processed.

This avoids requiring the complete base graphics repository to exist locally.

---

# ☁️ PC FileOnDemand

The PC pipeline records the SPCs referenced by processed images.

If:

```text
FileOnDemand = true
```

it checks each required SPC.

If the SPC is missing:

```text
DownloadSPC(...)
```

is invoked.

The download uses GitHub's raw-content endpoint with the configured cloud repository information. 

---

# ☁️ Console FileOnDemand

The console pipeline behaves similarly.

It collects all required:

```text
.ab
.assets
```

containers.

For each one that does not exist locally, it calls:

```cpp
DownloadABOrAsset(...)
```

The function normalizes the repository path and downloads the required file from the configured cloud repository. 

---

# 🔗 Cloud Repository Selection

This is why Maid must set:

```cpp
Cloud::dl_repo_name
```

before calling the compiler.

The compiler needs to know which repository's directory structure corresponds to the selected platform.

The effective mapping is:

```text
PC
 ↓
Cloud::dl_repo_name_pc

Xbox
 ↓
Cloud::dl_repo_name_xbox

Switch
 ↓
Cloud::dl_repo_name_switch
```

Maid performs this initialization once at startup.

---

# ⚡ Multithreaded Compilation

Maid itself does not create the worker threads.

It forwards execution to compilers which consult:

```text
MultithreadedCompilation
```

The PC and console implementations then group work by output container and process independent groups concurrently.

This is important because compilation is naturally parallelizable at the container level.

---

# 🧵 PC Threading Model

For PC/Xbox, images are first grouped by SRD and SPC.

Conceptually:

```text
PNG entries
    │
    ├── SRD A ──► thread A
    ├── SRD B ──► thread B
    └── SRD C ──► thread C
```

and later:

```text
SPC A ──► thread A
SPC B ──► thread B
SPC C ──► thread C
```

The source uses `std::jthread` when multithreading is enabled.

The global counters:

```cpp
SPCCompiled
SRDCompiled
```

are atomic so the main thread can wait for worker completion. 

---

# 🧵 Console Threading Model

The Unity pipeline groups images by:

```text
.ab
```

or:

```text
.assets
```

container.

For example:

```text
Image 1 ─┐
Image 2 ─┼──► sharedassets0.assets
Image 3 ─┘

Image 4 ─┐
Image 5 ─┴──► menu.ab
```

Each container becomes a logical compilation group.

The source notes that this grouping provides a substantial performance improvement because the groups can be processed independently. 

---

# ⏱️ Compilation Timeouts

The console pipeline waits for multithreaded compilation to finish.

If the operation takes longer than:

```text
300 seconds
```

the pipeline logs:

```text
ERROR: Too much time passed calculating ABs!
```

and returns.

The PC pipeline has equivalent five-minute timeout checks for SRD and SPC compilation. 

Therefore a compilation job that hangs indefinitely should eventually be surfaced as a timeout rather than blocking forever.

---

# 🖋️ Switch Font Injection

The console pipeline performs an additional operation after image compilation.

It looks for:

```text
Data/sharedassets0/FOT-HummingStd-D.txt
```

and:

```text
Data/sharedassets0/FOT-NewRodinPro-DB SDF.txt
```

and injects them into:

```text
Data/sharedassets0.assets
```

if both the source font and target asset exist. 

This is a special-case operation outside the normal PNG/TGA compilation pass.

---

# 🔤 Why Font Injection Is Separate

The font files are not ordinary texture PNGs.

They contain SDF font data that needs to be inserted into Unity's serialized asset database.

Consequently, they cannot simply be discovered through the normal image preprocessing system.

Maid's console pipeline explicitly handles them after normal image compilation.

---

# 🧰 UAFGJ

The Unity pipeline uses:

```text
UAFGJ
```

for Unity asset manipulation.

UAFGJ is described by the source as Team DAIX's Unity asset injector built around UABEA's asset/bundle manipulation functionality.

Its role is to replace serialized Unity asset data inside:

```text
.ab
.assets
```

containers.

Maid does not implement this functionality itself.

It invokes the tool through the console compilation subsystem.

---

# 🧰 SRDTool

For PC/Xbox assets, Maid ultimately relies on:

```text
SrdTool
```

to insert PNG data into SRD files.

The compiler invokes it once for each image that belongs to an SRD. 

---

# 🧰 SpcTool

After SRDs are generated, the PC pipeline uses:

```text
SpcTool
```

to insert:

```text
.srd
.srdv
```

data into their target:

```text
.spc
```

archive.

The pipeline deliberately processes SRDs first because SPC compilation depends on them being ready. 

---

# 🖼️ ALT Asset Handling

The PC pipeline supports alternate image files prefixed with:

```text
ALT_
```

For example:

```text
texture.png
ALT_texture.png
```

Maid's PC subsystem uses:

```cpp
HandleAltFiles(...)
```

to decide which version should become the active image.

---

# 🔄 `UseALTs`

When:

```text
UseALTs = true
```

the PC pipeline:

1. finds an image with an `ALT_` counterpart,
2. removes the original,
3. renames the `ALT_` version to the original filename.

Conceptually:

```text
texture.png
ALT_texture.png

        ↓

ALT mode ON

        ↓

texture.png  ← former ALT image
```

When:

```text
UseALTs = false
```

the reverse behavior occurs:

```text
texture.png
ALT_texture.png

        ↓

ALT mode OFF

        ↓

texture.png remains
ALT_texture.png deleted
```

The implementation performs this replacement before image preprocessing begins. 

---

# ⚠️ ALT Handling Is Destructive

`HandleAltFiles()` does not merely tell the compiler which image to use.

It physically:

```text
delete
```

and:

```text
rename
```

files.

Therefore `UseALTs` should be treated as a **filesystem-mutating option**.

Developers should not expect ALT selection to be a temporary in-memory override.

---

# 🧩 PC Special-Case Tables

The PC pipeline contains several hard-coded tables in:

```text
compile_pc.h
```

These are not controlled by Maid itself, but they are part of the pipeline Maid dispatches to.

---

## Minigame Entries

A special mapping exists for:

```text
anagram_US
```

and:

```text
minigame/anagram/answer000.dat
```

This allows the pipeline to inject a specific minigame resource into the appropriate SPC. 

---

# 🚫 SPC Blacklist

The PC compiler explicitly blacklists:

```text
adv_reaction_voice_US
```

from normal SPC compilation.

The source comments indicate that these assets are unsafe/problematic for the standard SPCTool workflow.

Therefore the existence of an SPC does not necessarily mean Maid is allowed to rebuild it.

---

# 🔀 Special SRD Mappings

Two PNGs are explicitly mapped to nonstandard SRD names:

```text
3B48A96E.png
    → caution_download_US.srd

7AEB69E6.png
    → caution_gro_US.srd
```

These override the normal SRD naming convention.

---

# 🔗 SRD Aliases

The alias table resolves naming conflicts such as:

```text
3B48A96E.png
    → kanban_US.png
    → caution_download_US.srd
```

and:

```text
7AEB69E6.png
    → kanban_US.png
    → caution_gro_US.srd
```

These mappings exist because multiple files can share CRC-derived temporary names.

Maid's preprocessing logic therefore cannot always assume:

```text
PNG name
=
actual target texture name
```

---

# 🪆 Matryoshka SPCs

The PC pipeline also contains mappings for **nested SPCs**.

These are SPC files that themselves contain other SPC files.

For example:

```text
chara_name_000.spc
        │
        ▼
chara_name_US.spc
```

The source describes these as "Matryoshka SPCs".

They cannot be handled by the normal flat SPC insertion process and therefore require special pre-repacked input.

---

# ⚠️ Matryoshka SPC Limitation

Maid does not dynamically unpack and recursively rebuild these nested SPC structures.

The special mapping instead tells the pipeline how the already-prepared nested asset relates to the outer SPC.

Developers should therefore regard these as **manual/pre-repacked assets**, not ordinary automatically generated SPCs.

---

# 🧹 Cleanup of Unchanged Assets

The PC pipeline also contains logic for removing or preserving assets that have not changed.

This is relevant when ImageTools is operating against a repository containing both modified and unchanged data.

The purpose is to prevent unnecessary output files from remaining in the build tree.

The cleanup stage is part of `PC::CompilePC`, not the Maid dispatcher itself.

---

# 📦 Repository Structure

The PC pipeline expects roughly:

```text
DGRV3-GFX/
├── base_spc/
│   └── <cloud repository>/
│       └── ...
└── ...
```

The Switch pipeline expects:

```text
DGRV3-AB-GFX/
├── base_ab/
│   └── <cloud repository>/
│       └── ...
└── ...
```

The exact contents below the platform-specific base directory are determined by the corresponding graphics repository.

---

# 🗂️ PC Output Location

The PC compiler constructs:

```cpp
where = dgrv3path / basefolder / Cloud::dl_repo_name;
```

Therefore the selected output/base repository location is effectively:

```text
DGRV3-GFX/base_spc/<cloud repository>/
```

for PC/Xbox.

The same structure is used with:

```text
DGRV3-AB-GFX/base_ab/<cloud repository>/
```

for Switch/Unity. 

---

# 🗂️ Console Output Location

The console compiler receives:

```text
DGRV3-AB-GFX
base_ab
```

and constructs its working output location from:

```text
dgrv3path / basefolder / Cloud::dl_repo_name
```

The target `.ab` and `.assets` paths are then derived from the PNG directory structure. 

---

# 🛡️ Repository Validation

The PC pipeline verifies that its selected repository exists.

If:

```text
DGRV3-GFX
```

cannot be found, it logs an error and returns:

```cpp
false
```

If the expected output/base directory does not exist, the compiler attempts to create it. 

The console pipeline likewise operates from the selected graphics repository.

---

# ⚠️ Maid's Return-Value Limitation

One important implementation detail is that `Maid.cpp` does not inspect the return value of:

```cpp
PC::CompilePC(...)
```

or:

```cpp
Console::CompileConsole(...)
```

It simply invokes the selected function and then proceeds to:

```text
All done!
```

and:

```cpp
Common::WaitExit();
```

Therefore:

```text
All done!
```

should **not automatically be interpreted as proof that every compilation operation succeeded**.

It means the dispatched function returned control to Maid.

This is an important distinction for automation and CI integration.

---

# 🚨 Configuration Failure Considerations

Maid itself assumes that:

```text
ImageConfig.config
```

can be loaded and that the relevant configuration keys exist.

The actual error handling for malformed or missing configuration is primarily delegated to:

```text
Configuration::ReadConfig()
```

rather than implemented inside Maid's `main()`.

Therefore configuration validation belongs to the shared configuration subsystem.

---

# 🔐 Cloud Authentication

The PC and console download helpers use information from:

```text
Common::Cloud
```

including:

```text
dl_repo_name
dl_repo_owner
dl_repo_name
dl_branch
dl_repo_token
```

The exact download behavior is implemented by the platform-specific compiler.

Maid's responsibility is to initialize the correct:

```text
Cloud::dl_repo_name
```

before that code executes.

---

# 🌐 FileOnDemand and GitHub

When FileOnDemand is active, the compiler constructs raw GitHub download commands.

The console implementation uses a request resembling:

```text
https://raw.githubusercontent.com/<owner>/<repo>/<branch>/<path>
```

and supplies the configured authentication token. 

The PC implementation performs equivalent SPC retrieval.

Therefore ImageTools can operate without a complete local copy of every base asset, provided the required assets are available from the configured cloud repository.

---

# ⚠️ GitHub 404 Quirk

The download implementation contains an important workaround.

GitHub's raw endpoint can return:

```text
404: Not Found
```

as the **file contents** rather than necessarily providing a convenient failure condition for the surrounding command.

The compiler therefore checks the downloaded file contents and rejects a file whose contents are exactly:

```text
404: Not Found
```

as invalid. 

This is a quirk of the download layer, not Maid itself.

---

# 🔤 Case-Sensitivity Quirks

The PC downloader contains special handling for uppercase/lowercase asset naming.

For example, it can transform:

```text
us
```

into:

```text
US
```

and retry `.spc` versus `.SPC` when necessary. 

The console downloader similarly performs path normalization and retry logic.

This is important because the local filesystem and remote repository can have different case-sensitivity characteristics.

---

# 🧠 Why Maid Is a Dispatcher Rather Than a Compiler

The architecture deliberately separates:

```text
decision
```

from:

```text
execution
```

Maid decides:

```text
Which platform?
Which repository?
Which base folder?
Which cloud repository?
```

The selected subsystem decides:

```text
Which images?
Which containers?
Which special cases?
Which external tool?
How should the data be injected?
```

This makes it possible to change the PC compiler without changing platform selection logic, and vice versa.

---

# 🔄 Full PC/Xbox Execution Flow

A complete PC/Xbox run can be represented as:

```text
Start Maid
    │
    ▼
current_path()
    │
    ▼
Read ImageConfig.config
    │
    ▼
UseSwitchConfiguration == false
    │
    ▼
Select DGRV3-GFX
    │
    ▼
Select base_spc
    │
    ├── Xbox? ──► cloud repo = Xbox
    │
    └── otherwise ──► cloud repo = PC
    │
    ▼
PC::CompilePC()
    │
    ▼
Validate DGRV3-GFX
    │
    ▼
Handle ALT files
    │
    ▼
Preprocess PNG/TGA
    │
    ▼
Resolve SRD/SPC mappings
    │
    ▼
Download missing SPCs if required
    │
    ▼
Compile SRDs
    │
    ▼
Compile SPCs
    │
    ▼
Cleanup
    │
    ▼
return to Maid
    │
    ▼
"All done!"
```

---

# 🔄 Full Switch/Unity Execution Flow

A complete Switch run is:

```text
Start Maid
    │
    ▼
current_path()
    │
    ▼
Read ImageConfig.config
    │
    ▼
UseSwitchConfiguration == true
    │
    ▼
Select DGRV3-AB-GFX
    │
    ▼
Select base_ab
    │
    ▼
cloud repo = Switch
    │
    ▼
Console::CompileConsole()
    │
    ▼
Scan PNG/TGA files
    │
    ▼
Map each image to .ab / .assets
    │
    ▼
Download missing containers if required
    │
    ▼
Group by container
    │
    ▼
Inject images through UAFGJ
    │
    ▼
Inject SDF fonts
    │
    ▼
return to Maid
    │
    ▼
"All done!"
```

---

# 🧩 Interaction With Adventurer

Maid is not responsible for cloning repositories.

That responsibility belongs to the ImageTools repository acquisition component, referred to elsewhere in the ImageTools pipeline as **Adventurer**.

Maid assumes the required graphics repository is available locally, unless FileOnDemand allows individual base assets to be retrieved remotely.

Conceptually:

```text
Adventurer
    ↓
DGRV3-GFX / DGRV3-AB-GFX
    ↓
Maid
    ↓
compilation
```

---

# 🧩 Interaction With Inventor

Likewise, Maid does not perform extraction of source image assets.

The extraction/preparation stage is handled by other ImageTools components.

Maid expects the selected graphics repository to contain the PNG/TGA assets that its preprocessors know how to map.

---

# 🧩 Interaction With TennisPro

Distribution is downstream of compilation.

The conceptual relationship is:

```text
Maid
    ↓
compiled SPC / AB / assets
    ↓
TennisPro
    ↓
distribution
```

Maid's role ends after the platform-specific image compilation pipeline completes.

---

# 🧩 Interaction With Detective

Patch generation is also downstream.

The broad ImageTools relationship is:

```text
Maid
    ↓
modified game assets
    ↓
TennisPro
    ↓
Detective
    ↓
patch generation
```

Maid therefore participates in the **asset production** side of the pipeline rather than final patch packaging.

---

# 🧪 Empty Image Repository

The PC compiler explicitly checks whether preprocessing produced an empty `FileStructure` vector.

If:

```text
fs_vec.empty()
```

is true, it logs:

```text
ERROR: Empty file vector...
```

and returns failure. 

This generally indicates that the selected repository contained no usable images or that the repository structure did not match the expected format.

---

# 🧪 Invalid Console Container

The console compiler groups image entries according to their target `.ab` or `.assets` path.

If a target cannot be found in the known container list, it logs an error.

It also performs bounds checks on the calculated grouping index to prevent invalid vector access. 

---

# 🧪 Invalid PC Container

The PC sorting routines similarly locate each entry's target SPC or SRD in the corresponding container list.

If the container cannot be found or the calculated index is outside the split array, an error is logged rather than blindly indexing the vector.

This is especially important because the compilation system relies heavily on grouping entries by their output archive.

---

# ⚠️ Performance Considerations

The most expensive work is not performed by Maid itself.

Maid's overhead is negligible compared with:

* SRD generation,
* SPC repacking,
* Unity asset rewriting,
* cloud downloads.

The main performance-related configuration is therefore:

```text
MultithreadedCompilation
```

When enabled, the platform compilers can process independent output containers concurrently.

---

# 📈 Why Compilation Is Grouped by Container

Parallelizing individual images indiscriminately would be dangerous because multiple images may need to modify the same archive simultaneously.

Instead, Maid's platform compilers group images by destination.

For example:

```text
Texture A ─┐
Texture B ─┼──► archive_01.spc
Texture C ─┘
```

becomes one compilation unit.

A different archive can be processed simultaneously:

```text
archive_02.spc ──► another worker
```

This preserves archive-level serialization while still allowing parallelism.

The same principle applies to Unity `.ab` and `.assets` files. 

---

# 🧵 Atomic Progress Counters

The PC and console headers define atomic counters:

```cpp
PC::SPCCompiled
PC::SRDCompiled
Console::ABOrAssetCompiled
```

These are used by the multithreaded wait loops.

Their purpose is not to determine correctness of the output itself.

They are simply progress/completion counters that allow the main thread to know when the worker groups have finished.

---

# ⚠️ Threading Limitation

The compilers use detached `std::jthread` objects and then explicitly wait for their atomic counters to reach the expected number of groups.

This works as a lightweight synchronization mechanism, but it means the pipeline's correctness depends on each worker eventually incrementing its corresponding counter.

The five-minute timeout exists as protection against a worker that never completes.

---

# 🛠️ Developer Notes

## 1. Maid should remain thin

The architecture strongly favors keeping platform-specific compilation logic out of `Maid.cpp`.

If new platform-specific behavior is required, it should generally belong in a dedicated compiler subsystem rather than expanding the dispatcher.

---

## 2. Platform selection is centralized

Do not duplicate the platform-selection logic elsewhere without good reason.

The authoritative top-level decision is:

```text
UseSwitchConfiguration
        │
        ├── true  → Console
        └── false → PC/Xbox
```

---

## 3. Xbox is not a separate compiler

Do not expect an:

```text
compile_xbox.cpp
```

pipeline based on the current architecture.

Xbox uses:

```text
PC::CompilePC()
```

with a different cloud repository selection.

---

## 4. `base_spc` does not mean "PC only"

Both:

```text
PC
Xbox
```

use:

```text
base_spc
```

because both use the SRD/SPC-based image pipeline.

---

## 5. `base_ab` is Switch-specific

The Switch/Unity pipeline uses:

```text
base_ab
```

because its target containers are Unity AssetBundles and `.assets` files rather than SPC archives.

---

# ⚠️ Developer Notes on `FileOnDemand`

`FileOnDemand` does not eliminate the need for the graphics repository.

The source images still need to be available for preprocessing.

What FileOnDemand avoids is requiring every **base/output container** to be present locally.

For PC:

```text
PNG source
    +
missing SPC
    ↓
download SPC
```

For Switch:

```text
PNG/TGA source
    +
missing .ab/.assets
    ↓
download container
```

---

# ⚠️ Developer Notes on ALT Processing

ALT replacement occurs before PC preprocessing.

This means the resulting:

```text
PNG_ActualName
```

can effectively represent a file that originally existed as:

```text
ALT_<name>
```

but was renamed to the normal filename.

This is why `PNG_RepoName` and `PNG_ActualName` are kept as separate fields in `FileStructure`.

---

# ⚠️ Developer Notes on `PNG_RepoName` vs `PNG_ActualName`

The two fields should not be treated as interchangeable.

```text
PNG_RepoName
```

represents the original repository path.

```text
PNG_ActualName
```

represents the actual file that should be used for compilation after preprocessing/alias/ALT resolution.

This distinction becomes particularly important for special-case mappings.

---

# ⚠️ Developer Notes on Hard-Coded Mappings

The PC pipeline contains several hard-coded mappings for:

* special SRDs,
* aliases,
* minigame files,
* blacklisted SPCs,
* nested SPCs.

These are effectively part of the game's asset-format knowledge base.

If the repository layout changes, these tables may require maintenance.

---

# ⚠️ Developer Notes on New Asset Types

The PC preprocessor primarily expects PNG/TGA-style image assets.

The console preprocessor explicitly accepts:

```text
.png
.PNG
.tga
.TGA
```

If a new image format is introduced, simply placing it in the graphics repository will not necessarily make Maid process it.

The corresponding preprocessing logic must first recognize it.

---

# 🧪 Recommended Execution Environment

A typical Maid installation should contain something resembling:

```text
ImageTools/
├── Maid.exe
├── ImageConfig.config
├── DGRV3-GFX/
│   └── ...
├── DGRV3-AB-GFX/
│   └── ...
├── SrdTool
├── SpcTool
├── UAFGJ
└── Common/
    └── ...
```

The exact layout depends on how the complete ImageTools distribution is packaged.

The critical requirement is that Maid's current working directory can resolve:

```text
ImageConfig.config
```

and the selected graphics repository.

---

# 🚀 Basic Usage

Maid is intended to be launched without complicated command-line arguments.

Conceptually:

```text
Maid.exe
```

The program then:

1. determines its working directory,
2. loads `ImageConfig.config`,
3. selects the platform,
4. configures the cloud repository,
5. launches the corresponding compiler,
6. waits for completion,
7. prints the final status.

---

# 🧪 Example: PC Configuration

Given:

```text
UseSwitchConfiguration = false
UseXboxConfiguration = false
```

Maid resolves:

```text
Platform:
PC / Steam

Repository:
DGRV3-GFX

Base:
base_spc

Cloud repository:
PC

Compiler:
PC::CompilePC()
```

The resulting workflow is:

```text
PNG/TGA
 ↓
SRD
 ↓
SPC
```

---

# 🧪 Example: Xbox Configuration

Given:

```text
UseSwitchConfiguration = false
UseXboxConfiguration = true
```

Maid resolves:

```text
Platform:
Xbox

Repository:
DGRV3-GFX

Base:
base_spc

Cloud repository:
Xbox

Compiler:
PC::CompilePC()
```

The actual image-processing implementation is still the PC compiler.

---

# 🧪 Example: Switch Configuration

Given:

```text
UseSwitchConfiguration = true
```

Maid resolves:

```text
Platform:
Nintendo Switch / Unity

Repository:
DGRV3-AB-GFX

Base:
base_ab

Cloud repository:
Switch

Compiler:
Console::CompileConsole()
```

The resulting workflow is:

```text
PNG/TGA
 ↓
.ab / .assets
```

followed by optional font injection.

---

# 🧭 Decision Tree

Maid's entire platform-selection logic can be summarized as:

```text
                         Start
                           │
                           ▼
                 Read ImageConfig.config
                           │
                           ▼
             UseSwitchConfiguration?
                    │             │
                  YES             NO
                    │              │
                    ▼              ▼
               Switch        UseXboxConfiguration?
                    │          │            │
                    │        YES            NO
                    │          │            │
                    │          ▼            ▼
                    │        Xbox          PC
                    │          │            │
                    └──────────┴────────────┘
                               │
                               ▼
                     Select cloud repository
                               │
                               ▼
                     Select base directory
                               │
                               ▼
                         Dispatch compiler
```

---

# 🧠 Design Philosophy

Maid follows a simple principle:

> **Decide once, delegate everything else.**

It does not attempt to understand every individual image.

Instead, it establishes:

```text
What platform?
```

then:

```text
What repository?
```

then:

```text
What base data?
```

and finally:

```text
Which compiler?
```

The platform-specific subsystem then takes responsibility for all of the complicated asset handling.

This separation is what keeps Maid itself relatively small despite ImageTools having a very complicated compilation pipeline.

---

# 📊 Responsibility Breakdown

| Responsibility                     | Maid | PC Compiler | Console Compiler |
| ---------------------------------- | :--: | :---------: | :--------------: |
| Load configuration                 |   ✅  |      —      |         —        |
| Select platform                    |   ✅  |      —      |         —        |
| Select graphics repo               |   ✅  |      —      |         —        |
| Select cloud repo                  |   ✅  |     uses    |       uses       |
| Select base folder                 |   ✅  |     uses    |       uses       |
| Scan images                        |   —  |      ✅      |         ✅        |
| Resolve ALT files                  |   —  |      ✅      |         —        |
| Resolve SRD aliases                |   —  |      ✅      |         —        |
| Resolve Matryoshka SPCs            |   —  |      ✅      |         —        |
| Download missing SPCs              |   —  |      ✅      |         —        |
| Download missing `.ab` / `.assets` |   —  |      —      |         ✅        |
| Generate SRDs                      |   —  |      ✅      |         —        |
| Generate SPCs                      |   —  |      ✅      |         —        |
| Inject Unity assets                |   —  |      —      |         ✅        |
| Inject SDF fonts                   |   —  |      —      |         ✅        |
| Multithread compilation            |   —  |      ✅      |         ✅        |
| Final application wait             |   ✅  |      —      |         —        |

---

# ⚠️ What Maid Does *Not* Do

It is useful to explicitly state what is outside Maid's responsibility.

Maid does not itself:

* decode PNGs,
* create SRDs,
* create SPCs,
* manipulate Unity serialized assets,
* clone Git repositories,
* extract SPC archives,
* distribute final assets,
* generate patches,
* calculate image differences,
* perform image compression directly.

Those responsibilities belong to other ImageTools components or external utilities.

Maid's job is orchestration.

---

# 🔗 Overall ImageTools Relationship

A simplified ImageTools pipeline can be visualized as:

```text
Repository Acquisition
        │
        ▼
     Adventurer
        │
        ▼
Graphics Repository
        │
        ▼
      Inventor
        │
        ▼
Prepared Graphics
        │
        ▼
       Maid
        │
        ├───────────────────┐
        ▼                   ▼
   PC / Xbox            Switch / Unity
        │                   │
        ▼                   ▼
   SRD → SPC            AB / Assets
        │                   │
        └─────────┬─────────┘
                  ▼
             TennisPro
                  │
                  ▼
              Detective
                  │
                  ▼
                Patches
```

The exact surrounding pipeline can vary, but Maid occupies the **central compilation stage**.

---

# 🏁 Summary

Maid is the **master dispatcher for ImageTools**.

Its core purpose is not to manipulate image files directly, but to transform a high-level configuration into a correctly configured platform-specific compilation run.

Its logic can be reduced to:

```text
ImageConfig.config
        │
        ▼
Determine platform
        │
        ├── PC/Xbox
        │      │
        │      ├── DGRV3-GFX
        │      ├── base_spc
        │      └── PC::CompilePC()
        │
        └── Switch/Unity
               │
               ├── DGRV3-AB-GFX
               ├── base_ab
               └── Console::CompileConsole()
```

The PC/Xbox branch performs:

```text
PNG/TGA
 ↓
SRD
 ↓
SPC
```

with support for:

* ALT image replacement,
* special SRD mappings,
* aliases,
* minigame assets,
* Matryoshka SPCs,
* SPC blacklisting,
* FileOnDemand downloads,
* multithreaded compilation,
* cleanup of unchanged assets.

The Switch/Unity branch performs:

```text
PNG/TGA
 ↓
AssetBundle / sharedassets
```

with support for:

* `.png` / `.PNG`,
* `.tga` / `.TGA`,
* AssetBundle/sharedassets detection,
* FileOnDemand downloads,
* multithreaded container compilation,
* UAFGJ-based Unity asset injection,
* SDF font injection.

The most important architectural relationship is:

```text
                    Maid
                     │
          ┌──────────┴──────────┐
          │                     │
          ▼                     ▼
     PC::CompilePC()     Console::CompileConsole()
          │                     │
          ▼                     ▼
       SRD/SPC              AB/.assets
```

Maid is therefore best understood as the **conductor rather than the musician**.

It decides **which compilation orchestra should play**, supplies the correct repository and platform context, and then lets the specialized compiler perform the actual work.

Its design is intentionally thin, centralized, and platform-aware:

> **Maid chooses the pipeline; the pipeline performs the compilation.**