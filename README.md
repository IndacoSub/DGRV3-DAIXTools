# DGRV3-DAIXTOOLS TOOLCHAIN — README
## Team DAIX, 2026

---

This README is the high-level overview for **Team DAIX's DGRV3-DAIXTOOLS**.

DGRV3-DAIXTOOLS is a collection of automated tools and toolchains used to build and maintain the Danganronpa V3 translation/modification.

The project is divided into several major components:

- **TextTools** — text/script processing and injection
- **ImageTools** — graphics processing and rebuilding
- **FontTools** — font processing and rebuilding
- **V3DailyManager** — separate high-level automation for the project's daily-build workflow

Individual toolchains and tools have their own READMEs and source comments for implementation details.

**PLEASE read the code before asking any questions.**

If you can't read the code, this repository is NOT for you.

**Generative AI** was used in the creation of this project's code, comments, documentation, and miscellaneous supporting files.

Our translation was not generated or supported by AI.

---

# TextTools

**TextTools** is the text-processing toolchain.

It contains the programs responsible for processing the game's scripts and text data, validating translated files, performing replacements, generating/inserting text data, and preparing modified files for distribution.

The complete TextTools pipeline is documented in the dedicated:

**`TextTools/README.md`**

Individual tools also have their own READMEs.

The TextTools pipeline is orchestrated by **HiddenDoor**.

For implementation details, prerequisites, configuration, pipeline order, and individual tool behavior, see the TextTools documentation rather than this root README.

---

# ImageTools

**ImageTools** is the graphics-processing toolchain.

It is responsible for processing modified graphics and rebuilding the corresponding game assets.

ImageTools supports the relevant platform-specific workflows, including SPC-based workflows and Nintendo Switch AB/Unity asset workflows.

The complete ImageTools pipeline is documented in the dedicated:

**`ImageTools/README.md`**

Individual ImageTools programs have their own READMEs.

The ImageTools pipeline is orchestrated by **Pianist**.

Again, this README intentionally does not duplicate the implementation details of the individual graphics tools.

---

# FontTools

**FontTools** is the font-processing toolchain.

It is responsible for automating the preparation, compilation, rebuilding, and distribution of modified game fonts.

Among other things, the pipeline handles:

- font repository preparation,
- extraction of required tools,
- font compilation,
- STX/SRDV generation,
- SPC modification,
- modified-file filtering,
- and distribution patch generation.

The complete FontTools pipeline is documented in the dedicated:

**`FontTools/README.md`**

Individual FontTools programs have their own READMEs.

The FontTools pipeline is orchestrated by **Monokuma**.

FontTools has its own configuration and dependency requirements; do not assume that it uses a `FontConfig.config` file merely because TextTools and ImageTools have configuration files.

---

# V3DailyManager

**V3DailyManager is a separate component from the TextTools, ImageTools, and FontTools toolchains.**

Its purpose is to automate the project's **daily-build workflow**.

V3DailyManager interacts with a separate repository called:

**DGRV3-Tools**

---

## What is DGRV3-Tools?

**DGRV3-Tools is a separate Team DAIX repository containing tool distributions and configuration files used by the daily-build workflow.**

It is **not** the same repository as DGRV3-DAIXTOOLS.

The distinction is important:

```text
DGRV3-DAIXTOOLS
    │
    └── Source code for the modern DAIXTOOLS project
        ├── TextTools
        ├── ImageTools
        ├── FontTools
        └── V3DailyManager


DGRV3-Tools
    │
    └── Separate repository containing
        distributed tools/configurations
        used by the daily workflow
````

V3DailyManager is the component that interacts with DGRV3-Tools as part of the daily-build process.

The DGRV3-Tools repository contains, among other things:

```text
FontTools/
ForDaily/
ForDailyPrivate/

ImageInjector/
ImageInjectorForSwitch/
ImageInjectorForXbox/
ImageInjectorNoMultithread/
ImageTools/

TextCrazy/
TextInjector/
TextInjectorForSwitch/
TextInjectorForXbox/
TextInjectorRelease/
TextRandomizer/
TextRandomizerDumb/
TextRandomizerDumbAll/
TextTools/
```

It also contains distributed tools, archives, unpackers, and configuration files.

Examples include:

```text
DGRV3-Tools/
├── FontTools/
│   ├── 7za.exe
│   ├── Tools.7z
│   ├── Tools_Linux.7z
│   └── V3FontUnpacker.exe
│
├── ImageTools/
│   ├── 7za.exe
│   ├── Tools.7z
│   ├── Tools_Linux.7z
│   └── V3ImageUnpacker.exe
│
└── TextTools/
    ├── 7za.exe
    ├── Tools.7z
    ├── Tools_Linux.7z
    └── V3TextUnpacker.exe
```

There are also daily configuration directories such as:

```text
ForDaily/
└── TextConfig.config

ForDailyPrivate/
└── TextConfig.config
```

The exact contents and behavior of DGRV3-Tools are outside the scope of this repository's root README.

See the dedicated **V3DailyManager README** for the details of how DailyManager interacts with it.

---

## Important distinction

Do not confuse these projects:

| Project             | Purpose                                                                                    |
| ------------------- | ------------------------------------------------------------------------------------------ |
| **DGRV3-DAIXTOOLS** | Source repository for the DAIXTOOLS project                                                |
| **TextTools**       | Text/script processing toolchain                                                           |
| **ImageTools**      | Graphics processing toolchain                                                              |
| **FontTools**       | Font processing toolchain                                                                  |
| **V3DailyManager**  | Daily-build automation component                                                           |
| **DGRV3-Tools**     | Separate repository containing distributed tools/configurations used by the daily workflow |

DGRV3-Tools is mentioned here specifically because **V3DailyManager uses it**.

It is not an output directory of DAIXTOOLS and should not be treated as one.

---

# IMPORTANT WARNING

## THESE TOOLCHAINS ARE EXTREMELY COMPLEX AND ABSOLUTELY NOT FOR THE AVERAGE USER.

If you just want to make a *meme mod*, or edit a handful of files manually, use **Harmony-Tools** instead.

Harmony-Tools is simple, safe, and designed for casual modding.

This repository is for:

* people editing **thousands of files**,
* people who need **automated checks**,
* people who need **variable replacement**,
* people who need **SPC rebuilds**,
* people who want **full automation**,
* people who want **full reproducibility**,
* people who want **full control**,
* and, frankly, **masochists**.

If you want the easy path, Harmony-Tools exists.

If you want the **complete, automated, industrial-strength pipeline**, the code is here.

---

# KEY DEPENDENCIES

The following dependencies are used by one or more components of DGRV3-DAIXTOOLS.

Not every dependency is required by every toolchain.

The individual toolchain READMEs should always be consulted for the exact requirements of a particular pipeline.

---

## 7za — 7-Zip Command-Line

**7za** is required for extracting `.7z` archives.

It is used by the toolchains to extract external tools and other archived resources.

The expected executable is:

```text
7za.exe
```

on Windows, or:

```text
7za
```

on systems where it is available through `PATH`.

For example, toolchains may expect it in:

```text
base_spc/
```

or another toolchain-specific directory.

**7za must be provided by the user.**

---

## HTFont

**HTFont.exe** is part of an **older version of Harmony-Tools**, developed by Redssu: [Harmony-Tools](https://github.com/redssu/Harmony-Tools)

Harmony-Tools was originally split into several standalone executables, including `HTFont`, `HTSrd`, `HTSpc`, and others. Modern Harmony-Tools has since been consolidated into a single unified executable. **FontTools deliberately continues to use the older `HTFont` component** rather than the modern unified tool.

This is intentional: HTFont provides stable **STX/SRDV packing routines**, matches the behavior expected by the DAIXTOOLS font pipeline, and preserves compatibility with the existing SPC workflows. Replacing it with the modern Harmony-Tools executable may therefore change behavior or break compatibility with the pipeline.

---

## SPCTool — SPC Injection

**SPCTool is ALWAYS required for SPC injection workflows.**

Even if you do not use STXTool, SPCTool is mandatory whenever a pipeline needs to inject generated data into an SPC archive.

SPCTool originates from:

[https://github.com/CaptainSwag101/DRV3-Sharp](https://github.com/CaptainSwag101/DRV3-Sharp)

The version used by the DAIXTOOLS toolchain is an **edited old version**.

It has been modified so that it does not ask the user for input.

Instead, it simply executes the requested command and exits.

**Do not assume that the latest upstream version is interchangeable with the version expected by DAIXTOOLS.**

---

## SRDTool — SRD(V) Injection

**SRDTool is required for ImageTools workflows involving SRD/SRDV injection.**

SRDTool also originates from:

[https://github.com/CaptainSwag101/DRV3-Sharp](https://github.com/CaptainSwag101/DRV3-Sharp)

As with SPCTool, the version used by DAIXTOOLS is an **edited old version** designed for automated operation.

It does not require interactive user input.

---

## StxTool / DRV3-STX-TOOL

STX tooling is required when a workflow needs to generate `.stx` data from `.txt`.

There are two relevant variants.

### StxTool

The older StxTool originates from:

[https://github.com/CaptainSwag101/DRV3-Sharp](https://github.com/CaptainSwag101/DRV3-Sharp)

The version used by the DAIXTOOLS toolchain is an **edited old version**.

It does not ask for user input.

It executes the command and exits.

### DRV3-STX-TOOL / NewSTXTool

The newer alternative is Liquid-S' **DRV3-STX-TOOL**.

DAIXTOOLS uses the project's `custom` branch:

[https://github.com/IndacoSub/DRV3-STX-TOOL/tree/custom](https://github.com/IndacoSub/DRV3-STX-TOOL/tree/custom)

**No executable will be provided.**

If this version is required, you must compile it yourself.

---

## UAFGJ — Nintendo Switch

**UAFGJ is required by ImageTools when compiling for Nintendo Switch.**

UAFGJ originates from:

[https://github.com/IndacoSub/UABEA/tree/custom](https://github.com/IndacoSub/UABEA/tree/custom)

It is a platform-specific dependency and is not required for every ImageTools configuration.

---

## UPS Binary

The UPS binary is required by distribution stages that generate UPS patches.

The expected executable is generally:

```text
ups.exe
```

on Windows, or:

```text
ups
```

on other platforms.

You must provide your own build.

The upstream project is:

[https://github.com/rameshvarun/ups](https://github.com/rameshvarun/ups)

For example, **WhiteSheet** uses UPS functionality when generating TextTools distribution patches.

---

## .NET Runtime / SDK

A suitable **dotnet runtime / SDK** is required by several components.

This includes:

* **Bugvac (C#)**
* **LooseFloorboard (C#)**
* **UAFGJ**
* some STX/SPC tools depending on platform
* other .NET-based DGRV3-DAIXTOOLS utilities

The exact runtime/SDK requirements should be determined from the individual project configuration and documentation.

---

## External SHA Library

**Necronomicon requires an external SHA implementation.**

Its:

```text
TextTools/Necronomicon/calc_sha.h
```

expects a SHA-512 (or compatible) implementation.

The public repository **does not bundle a SHA library**.

You must provide an appropriate implementation and include/adapt it in:

```text
calc_sha.h
```

See the Necronomicon README and source code for the exact requirements.

---

# BUILD & RUN

The recommended development environment is **Visual Studio**.

The repository may contain projects that can be built using CMake or other methods, but Visual Studio is the recommended environment for the DAIXTOOLS workflow.

---

## 1. Clone the Repository

Clone the repository recursively:

```bash
git clone <repo-url> --recursive
```

Then enter the repository:

```bash
cd <repo-root>
```

The recursive clone is recommended where repository submodules are involved.

---

## 2. Provide External Dependencies

Provide the external tools required by the toolchain you intend to use.

Depending on the component, this can include:

```text
7za
SPCTool
SRDTool
StxTool / NewSTXTool
UPS
UAFGJ
HTFont
external SHA implementation
```

The correct location for each dependency is toolchain-specific.

Consult the corresponding README instead of assuming that all dependencies belong in the repository root.

---

## 3. Configure Required External Libraries

If using Necronomicon, provide the required SHA implementation and adapt:

```text
TextTools/Necronomicon/calc_sha.h
```

as required by the implementation.

---

## 4. Build

Build the projects using Visual Studio.

CMake may be supported by individual projects.

For projects where CMake is applicable, a typical configuration may look like:

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

However:

> **Please use Visual Studio.**

The actual project/solution configuration in the repository is authoritative.

---

## 5. Configure the Toolchains

Configuration is toolchain-specific.

For example:

```text
TextConfig.config
ImageConfig.config
```

are used by the appropriate components.

There is **no `FontConfig.config` in DGRV3-DAIXTOOLS**.

FontTools has its own configuration and behavior, documented in its dedicated README.

Do not assume that all three toolchains use identical configuration files or configuration mechanisms.

---

## 6. Run the Toolchains

The main orchestrators are:

```text
TextTools:
    HiddenDoor

ImageTools:
    Pianist

FontTools:
    Monokuma
```

V3DailyManager is a separate component that handles the project's daily-build workflow and its interaction with DGRV3-Tools.

See the relevant README before attempting to run a complete pipeline.

---

# RUNNING INDIVIDUAL TOOLS

Individual tools can generally be executed directly for debugging.

Examples include:

```text
HydraulicPress.exe
Electrohammer.exe
WhiteSheet.exe part2
```

However, a tool may depend on files generated by previous stages.

Therefore:

> **Running an individual executable is not necessarily equivalent to running the complete pipeline.**

If a tool expects a report, generated file, repository, archive, or directory from an earlier stage, that prerequisite must already exist.

The individual tool README is the authoritative documentation for that tool's input and output contract.

---

# CONFIGURATION

Configuration is intentionally divided by toolchain.

Examples include:

```text
TextConfig.config
ImageConfig.config
```

Configuration can control things such as:

* target platform,
* repository selection,
* build mode,
* distribution mode,
* feature flags,
* repository branches,
* and other toolchain-specific behavior.

The exact available settings are documented by the relevant toolchain.

Do not copy configuration assumptions from one toolchain to another.

---

# TOOLCHAIN DOCUMENTATION

This repository intentionally uses multiple levels of documentation.

The hierarchy is:

```text
DGRV3-DAIXTOOLS README
        │
        ├── TextTools README
        │      └── Individual TextTools READMEs
        │
        ├── ImageTools README
        │      └── Individual ImageTools READMEs
        │
        ├── FontTools README
        │      └── Individual FontTools READMEs
        │
        └── V3DailyManager README
```

This README is deliberately high-level.

It should answer:

> **What is DGRV3-DAIXTOOLS, what are its major components, what dependencies does it have, and where do I go for detailed documentation?**

The toolchain READMEs answer:

> **How does each complete pipeline work?**

The individual tool READMEs answer:

> **What exactly does this particular executable do?**

The source code answers:

> **What does the implementation actually do?**

If you need implementation details, **read the relevant README and the code.**

---

# TROUBLESHOOTING

When a pipeline fails, do not immediately assume that the final tool that crashed is responsible for the problem.

DAIXTOOLS is composed of many sequential stages.

A failure in an earlier stage can result in:

* missing files,
* malformed files,
* missing directories,
* invalid generated data,
* incorrect hashes,
* incorrect line counts,
* or other invalid inputs for later stages.

Recommended procedure:

1. Identify the first meaningful failure.
2. Read the README for that tool.
3. Read the source code.
4. Check the tool's expected inputs.
5. Check the output of the previous stage.
6. Check required external dependencies.
7. Run the failing tool independently if useful.
8. Correct the underlying problem.
9. Rerun the affected stage or perform a clean build.

---

# CLEAN BUILDS

A clean working environment is strongly recommended when debugging or validating changes.

Many tools expect repositories and generated directories to contain the files produced by previous stages.

For reproducible builds:

1. Start from the expected repository state.
2. Use the intended configuration.
3. Provide the correct external tools.
4. Run the pipeline from its expected working directory.
5. Avoid manually modifying intermediate output.
6. Preserve logs when investigating failures.

If behavior becomes inconsistent after repeated runs, a clean build should be one of the first diagnostic steps.

---

# DESTRUCTIVE OPERATIONS

Some tools intentionally delete, replace, or overwrite files.

Depending on the tool, this can include:

* deleting empty files,
* cleaning generated directories,
* overwriting compiled assets,
* rebuilding SPC/AB containers,
* removing repository metadata,
* recreating distribution folders,
* or replacing generated outputs.

This behavior is intentional.

**Do not point DAIXTOOLS at directories containing unrelated or irreplaceable data.**

Use dedicated working directories and keep backups of anything important.

---

# LEGAL & LICENSE

* Do not commit proprietary binaries or game assets to the public repository.
* The toolchain intentionally avoids bundling certain third-party libraries and binaries.
* You must obtain and provide required third-party software yourself.
* Respect the licenses of all third-party tools used with DAIXTOOLS.
* Do not redistribute proprietary game assets without appropriate authorization.

This repository is licensed under the **ISC license**, with exemptions for specific companies or groups noted below.

**Spike Chunsoft Co., Ltd.** may, at their discretion, relicense applicable code under either the MIT license or the GPLv2 license.

**Too Kyo Games, LLC** may, at their discretion, relicense applicable code under either the MIT license or the GPLv2 license.

Third-party software remains subject to its own licenses.

---

# CONTRIBUTING

Contributions are welcome.

Recommended workflow:

1. Fork the repository.
2. Create a feature branch.
3. Make your changes.
4. Build the affected project(s).
5. Test the affected tool independently.
6. Test the relevant pipeline.
7. Run the full pipeline when practical.
8. Document behavioral changes.
9. Document new external dependencies.
10. Open a pull request.

When changing a tool's behavior, update its individual README as well as any affected high-level documentation.

Do not add proprietary binaries, game assets, credentials, or private repository information to commits.

---

# QUICK REFERENCE

| Component          | Purpose                                                                                     | Main Entry Point |
| ------------------ | ------------------------------------------------------------------------------------------- | ---------------- |
| **TextTools**      | Text/script processing and injection                                                        | `HiddenDoor`     |
| **ImageTools**     | Graphics processing and rebuilding                                                          | `Pianist`        |
| **FontTools**      | Font processing and rebuilding                                                              | `Monokuma`       |
| **V3DailyManager** | Daily-build automation                                                                      | `V3DailyManager` |
| **DGRV3-Tools**    | Separate repository used by V3DailyManager for daily-build tool distributions/configuration | —                |

---

# FINAL SUMMARY

**DGRV3-DAIXTOOLS is Team DAIX's automated toolchain collection for the Danganronpa V3 translation/modification.**

Its primary processing toolchains are:

```text
TextTools
    └── Text and script processing

ImageTools
    └── Graphics processing

FontTools
    └── Font processing
```

These toolchains contain numerous individual programs, each responsible for a specific stage of the overall build process.

At a higher level, **V3DailyManager** provides automation for the project's daily-build workflow and interacts with the separate **DGRV3-Tools** repository.

The individual READMEs provide the actual implementation details.