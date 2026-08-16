# TEXTTOOLS - WHITESHEET
## Team DAIX, 2026  

---

## 📌 Overview

**WhiteSheet** is the final packaging component of the TextTools pipeline.

Its purpose is to transform the modified SPC/PB resources produced by the earlier TextTools stages into **UPS patches suitable for distribution**.

WhiteSheet operates in **two distinct stages**, selected through the `part2` command-line argument:

```text
WhiteSheet.exe
````

runs **Part 1**, while:

```text
WhiteSheet.exe part2
```

runs **Part 2**.

The two stages are intentionally separate:

```text
Part 1
    │
    ├── preserve original SPCs
    ├── rename them to *_normal.spc
    ├── copy modified resources into Distribute/
    └── prepare the directory for patch generation
            │
            ▼
        Part 2
            │
            ├── read different_part2.txt
            ├── identify changed resources
            ├── compare normal vs modified files
            ├── generate *_patch.ups
            ├── generate special trial patches
            └── remove temporary/invalid files
```

WhiteSheet therefore sits at the **very end** of the DAIX workflow.

The earlier tools modify the game's resources; WhiteSheet turns those modifications into the actual patch files intended for distribution.

---

# 🎯 Responsibilities

WhiteSheet has four primary responsibilities.

### ✔ 1. Prepare original resources

Before generating patches, WhiteSheet renames the original SPC files already present in:

```text
Distribute/
```

to:

```text
*_normal.spc
```

These files become the **base/reference versions** used by the UPS generator.

---

### ✔ 2. Copy modified resources

WhiteSheet collects modified resources from:

```text
ModifiedFiles/
```

and copies them into:

```text
Distribute/
```

This gives Part 2 both sides of the comparison:

```text
*_normal.spc
        +
modified SPC
        ↓
UPS diff
```

---

### ✔ 3. Generate UPS patches

Part 2 invokes the UPS executable:

```text
DGRV3/base_spc/ups
```

or, on Windows:

```text
DGRV3/base_spc/ups.exe
```

using the equivalent command:

```text
ups diff --base <base> --modified <modified> --output <patch>
```

The resulting patch is named:

```text
*_patch.ups
```

---

### ✔ 4. Handle special trial SPCs

Trial/debate SPCs receive special handling.

The tool does **not** use the normal modified-vs-normal comparison for these resources.

Instead, it explicitly generates patches from:

```text
trial_english_extracted/
```

to:

```text
trial_french_extracted/
```

because the French resources provide better text positioning for the debates.

---

# 🧩 How WhiteSheet Fits Into TextTools

WhiteSheet is the **final packaging stage**.

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
Crossbow
   ↓
WhiteSheet
```

The important distinction is:

```text
ElectroHammer
    ↓
modifies / recompiles SPC resources

Crossbow
    ↓
selects resources for distribution

WhiteSheet Part 1
    ↓
prepares base + modified resources

WhiteSheet Part 2
    ↓
generates final UPS patches
```

WhiteSheet should therefore be considered the **release/packaging boundary** of TextTools.

---

# 🏗️ Two-Stage Architecture

WhiteSheet is implemented as a single executable with two logical modes.

```text
                 WhiteSheet
                     │
             ┌───────┴───────┐
             │               │
          Part 1           Part 2
             │               │
     Prepare inputs      Generate UPS
             │               │
     Distribute/        Distribute/
       *_normal            *_patch.ups
```

The mode is controlled by the presence of:

```text
part2
```

in either of the first two command-line argument positions.

---

# 🚀 Command-Line Usage

## Part 1

Run:

```text
WhiteSheet.exe
```

This prepares the distribution directory.

---

## Part 2

Run:

```text
WhiteSheet.exe part2
```

This generates UPS patches.

The implementation also accepts `part2` as the second user argument, so the following form can also activate Part 2:

```text
WhiteSheet.exe <argument> part2
```

The actual string comparison is performed against the encrypted representation of:

```text
part2
```

---

# 📂 Important Paths

WhiteSheet constructs all of its working paths relative to the **current working directory**.

The main paths are:

```text
ModifiedFiles/
Distribute/
DGRV3/
DGRV3/base_spc/
```

More specifically:

| Path                     | Purpose                                      |
| ------------------------ | -------------------------------------------- |
| `ModifiedFiles/`         | Modified resources produced by earlier tools |
| `Distribute/`            | Working distribution directory               |
| `DGRV3/`                 | Game repository                              |
| `DGRV3/base_spc/`        | Base SPC resources and UPS executable        |
| `DGRV3/base_spc/ups`     | UPS patch generator                          |
| `DGRV3/base_spc/ups.exe` | Windows UPS patch generator                  |

---

# ⚙️ Startup Configuration

WhiteSheet loads:

```text
TextConfig.config
```

from the current working directory.

It performs:

```cpp
Configuration::ReadConfig(configfile);
Configuration::ViewDebugCurrentConfig();
```

Therefore WhiteSheet is also affected by the global TextTools configuration.

One particularly important configuration value is:

```text
UseSwitchConfiguration
```

which changes how PB/i18n resources are handled.

---

# 🛑 UPS Tool Requirement

WhiteSheet requires the UPS executable to exist at:

```text
DGRV3/base_spc/ups
```

on non-Windows systems, or:

```text
DGRV3/base_spc/ups.exe
```

on Windows.

If the executable is missing, WhiteSheet stops immediately:

```text
ERROR: UPS patcher not found!
```

and returns:

```text
-1
```

This check occurs before either Part 1 or Part 2 runs.

---

# 📁 `Distribute/` Creation

If:

```text
Distribute/
```

does not exist, WhiteSheet creates it automatically.

This occurs during startup before the selected stage executes.

---

# 🟢 Part 1 — Preparation Stage

Part 1 is the **setup phase**.

Its purpose is to construct the input set required by Part 2.

The basic operation is:

```text
Distribute/
    ↓
rename original SPCs
    ↓
*_normal.spc

ModifiedFiles/
    ↓
copy modified resources
    ↓
Distribute/
```

After Part 1 finishes, `Distribute/` contains both:

```text
original → *_normal.spc
modified → normal filename
```

allowing Part 2 to calculate the binary difference.

---

# 📦 Part 1 Input

Part 1 examines:

```text
Distribute/
```

and:

```text
ModifiedFiles/
```

It also optionally reads:

```text
DGRV3/i18n/
```

when:

```text
UseSwitchConfiguration
```

is enabled.

---

# 🔄 Part 1 — Renaming Original SPCs

WhiteSheet recursively scans:

```text
Distribute/
```

for existing files.

For each eligible non-trial resource, it creates a new name by inserting:

```text
_normal
```

before the extension.

Example:

```text
game_resident_US.spc
```

becomes:

```text
game_resident_US_normal.spc
```

Likewise:

```text
chap3_text_US.SPC
```

is transformed to:

```text
chap3_text_US_normal.spc
```

Notice that the generated `_normal` extension is normalized to lowercase.

---

# 🔍 Part 1 File Filtering

WhiteSheet ignores anything whose path contains:

```text
_extracted
```

These directories are considered temporary extraction output and are not treated as final patch inputs.

---

# 🧪 Trial Files Are Excluded

Part 1 deliberately does **not** rename trial/debate SPCs.

A file is classified as a trial file when it:

1. contains `.spc` or `.SPC`, and
2. contains one of these identifiers:

```text
hanron_
kokoronpa_08_
nico_06_
nonstop_
panic_
```

These files are skipped.

---

# ⚠️ Why Trial Files Are Special

The source comments explain that trial SPCs require a separate treatment because the French resources have better text positioning/layout.

Consequently, WhiteSheet does not create:

```text
trial_US_normal.spc
```

for these files during Part 1.

Instead, Part 2 directly compares:

```text
English trial SPC
        ↓
French trial SPC
```

---

# 📝 Part 1 Filename Construction

For an ordinary file:

```text
<basename>.<extension>
```

WhiteSheet constructs:

```text
<basename>_normal.<lowercase extension>
```

For example:

```text
foo.SPC
```

becomes:

```text
foo_normal.spc
```

The source extension is converted to lowercase before constructing the new filename.

---

# ⚠️ Existing `_normal` Files

Before renaming, WhiteSheet checks whether the source path contains:

```text
normal
```

using `Common::StringContains()`.

If it does, the file is not renamed again.

This is intended to prevent repeated executions from producing:

```text
foo_normal_normal.spc
```

---

# ⚠️ Broad `"normal"` Check

The check is substring-based.

It is not specifically checking for:

```text
_normal
```

Therefore a filename containing the text:

```text
normal
```

anywhere in its path can be treated as already-normalized.

This is an implementation quirk worth remembering when naming resources.

---

# 📋 Part 1 Modified-File Collection

After preparing the base files, WhiteSheet recursively scans:

```text
ModifiedFiles/
```

It collects filenames that should be copied into:

```text
Distribute/
```

It ignores:

```text
_extracted
```

paths.

It also ignores the same special trial SPC patterns used during the base scan.

---

# 🧾 Only the Filename Is Stored

For every modified resource found, WhiteSheet stores:

```cpp
file.path().filename().string()
```

rather than the complete source path.

Therefore:

```text
ModifiedFiles/chapter3/chap3_text_US.spc
```

is stored simply as:

```text
chap3_text_US.spc
```

---

# ⚠️ Consequence of Filename-Only Collection

When copying, WhiteSheet reconstructs the source as:

```text
ModifiedFiles/<filename>
```

and destination as:

```text
Distribute/<filename>
```

This means the original subdirectory structure beneath `ModifiedFiles/` is **not preserved** by this copying step.

The implementation assumes the relevant modified files are available directly by filename under `ModifiedFiles/`.

---

# 📥 Copying Modified Resources

For each collected modified filename:

```text
ModifiedFiles/<file>
```

is copied to:

```text
Distribute/<file>
```

The copy uses:

```cpp
std::filesystem::copy(
    ...,
    std::filesystem::copy_options::recursive
);
```

Despite the `recursive` option, the collected objects are normally files rather than directory trees.

---

# 🎮 Switch Configuration

When:

```text
UseSwitchConfiguration == true
```

WhiteSheet additionally processes:

```text
DGRV3/i18n/
```

This is used for the Unity-based/Switch configuration.

The code comments describe the `i18n` files as containing text that also needs to be included in the patching process.

---

# 📦 Switch `i18n` Copying

WhiteSheet recursively scans:

```text
DGRV3/i18n/
```

and copies each encountered entry into:

```text
Distribute/
```

using its filename.

The destination is constructed as:

```text
Distribute/<filename>
```

rather than preserving the complete source hierarchy.

---

# ⚠️ `i18n` Copying Is Independent

The Switch `i18n` copy operation does not depend on `ModifiedFiles/`.

It is controlled directly by:

```text
Configuration::ConfigMap["UseSwitchConfiguration"]
```

Therefore enabling Switch configuration causes additional files to be placed in `Distribute/` even if they were not part of the ordinary modified-file list.

---

# 🏁 Part 1 Completion

After copying, WhiteSheet calls:

```text
Common::WaitExit()
```

and returns:

```text
EXIT_SUCCESS
```

Part 1 does **not** generate UPS files.

Its job is solely to prepare the inputs.

---

# 🔵 Part 2 — UPS Generation

Part 2 begins when:

```text
part2
```

is supplied.

Its purpose is to take the prepared resources in:

```text
Distribute/
```

and turn them into:

```text
*_patch.ups
```

files.

The key input is:

```text
different_part2.txt
```

---

# 📄 `different_part2.txt`

Part 2 opens:

```text
different_part2.txt
```

and reads it line by line.

Each line is interpreted as a filesystem path.

Directories are ignored.

The remaining paths are inserted into:

```cpp
std::set<std::string> different_spcs;
```

---

# 🧠 Why a `std::set` Is Used

Using a set automatically removes duplicate entries.

For example:

```text
Distribute/chap1.spc
Distribute/chap1.spc
Distribute/chap2.spc
```

becomes:

```text
chap1.spc
chap2.spc
```

inside `different_spcs`.

This prevents the same patch from being generated twice merely because it appeared multiple times in the input file.

---

# ⚠️ Missing `different_part2.txt`

If the file cannot be opened, WhiteSheet logs:

```text
File does not exist: different_part2.txt
```

However, it does not immediately return at that point.

The set remains empty.

The subsequent empty-set check then causes Part 2 to terminate:

```text
ERROR: No different SPCs were found! (Exiting)
```

and return:

```text
EXIT_FAILURE
```

---

# 🛑 Empty Difference List

If:

```text
different_spcs.empty()
```

is true, Part 2 exits.

Therefore:

```text
different_part2.txt
```

must contain at least one usable file path for normal Part 2 execution to continue.

---

# 🟡 Switch PB Handling in Part 2

When:

```text
UseSwitchConfiguration == true
```

WhiteSheet performs an additional scan of:

```text
Distribute/
```

It finds `.pb` files and adds them to:

```text
different_spcs
```

unless their filename contains:

```text
_normal
```

This is because PB files contain binary text for the Switch configuration and therefore also need to be patched.

---

# 📦 PB Files and UPS

A PB file normally does not have the same `_normal.spc` naming convention as an SPC.

Nevertheless, Switch configuration adds PB files to the difference set so they can be passed through the same UPS-generation mechanism.

The output naming logic is extension-based, so a PB file can produce:

```text
something_patch.ups
```

provided its expected base file exists.

---

# 🔍 Part 2 Input Filtering

Before generating a patch, WhiteSheet examines the extension.

The extension is converted to lowercase.

Then the tool skips paths whose extension string contains:

```text
.ups
```

or whose input path contains:

```text
_patch
```

or:

```text
_sha
```

This prevents existing patch/hash artifacts from being recursively treated as new patch inputs.

---

# 🧠 Important Filtering Quirk

The checks are substring-based:

```text
contains(".ups")
contains("_patch")
contains("_sha")
```

They are not strict filename-component tests.

Therefore a path containing these strings for unrelated reasons may also be skipped.

---

# 🏗️ Constructing the Modified Path

For every remaining entry in:

```text
different_spcs
```

WhiteSheet treats the path itself as the modified resource.

For example:

```text
Distribute/chap3_text_US.spc
```

becomes:

```text
modified = Distribute/chap3_text_US.spc
```

---

# 🏗️ Constructing the Base Path

The base filename is generated by inserting:

```text
_normal
```

before the extension.

For:

```text
Distribute/chap3_text_US.spc
```

the expected base is:

```text
Distribute/chap3_text_US_normal.spc
```

---

# 🏷️ Constructing the UPS Output

The patch name is generated by inserting:

```text
_patch
```

before the extension.

Thus:

```text
chap3_text_US.spc
```

becomes:

```text
chap3_text_US_patch.ups
```

The output remains in the same general directory as the modified resource.

---

# 🔄 Normal Patch Relationship

The standard comparison therefore looks like:

```text
Distribute/
├── chap3_text_US_normal.spc   ← base
└── chap3_text_US.spc          ← modified
             │
             ▼
          UPS diff
             │
             ▼
└── chap3_text_US_patch.ups
```

---

# 🔧 `CreateUPSPatch()`

The actual patch generation is encapsulated in:

```cpp
CreateUPSPatch()
```

Its inputs are:

```text
ups
base
modified
outname
```

where:

| Parameter  | Meaning                 |
| ---------- | ----------------------- |
| `ups`      | UPS executable          |
| `base`     | Original/reference file |
| `modified` | Modified file           |
| `outname`  | UPS patch output        |

---

# 💻 UPS Command

WhiteSheet constructs:

```text
ups diff --base <base> --modified <modified> --output <outname>
```

The executable and every path are wrapped in quotes.

Conceptually:

```text
"ups" diff --base "base.spc" --modified "modified.spc" --output "patch.ups"
```

This protects paths containing spaces.

---

# 🧹 Quote Sanitization

WhiteSheet contains two helper lambdas:

```text
strip_quotes
quote_wrap
```

### `strip_quotes`

Removes leading/trailing quotation marks.

It also repeatedly strips stray quotes from the beginning/end of the string.

---

### `quote_wrap`

Ensures the final string is surrounded by exactly one pair of quotes.

This combination prevents accidentally producing malformed command lines such as:

```text
""path""
```

---

# 📁 Output Directory Creation

Before running UPS, `CreateUPSPatch()` creates:

```text
outname.parent_path()
```

using:

```cpp
std::filesystem::create_directories()
```

Therefore the destination directory for a patch does not have to exist beforehand.

---

# 🚨 UPS Execution Failure

WhiteSheet executes the UPS command through:

```text
Common::executeBatch()
```

If the command returns a non-zero result:

1. an error is logged,
2. any output patch that was created is deleted,
3. `CreateUPSPatch()` returns `false`.

This prevents a failed UPS invocation from leaving behind an apparently valid patch file.

---

# ⚠️ Failure Does Not Abort the Entire Patch Batch

`CreateUPSPatch()` returns:

```text
false
```

when generation fails.

However, the caller does not use the return value to immediately terminate the entire Part 2 loop.

Therefore WhiteSheet can continue attempting other patches after one patch fails.

---

# ⚠️ Missing Base File

Before invoking the UPS tool, WhiteSheet checks whether the calculated base exists.

If it does not, it logs:

```text
WARNING: Base does not exist: "<base>"
```

It then performs an additional uppercase-extension check.

---

# 🔠 Uppercase Extension Handling

WhiteSheet initially builds the base filename using a lowercase extension.

For example:

```text
modified.SPC
```

results in an expected base:

```text
modified_normal.spc
```

If that does not exist, WhiteSheet constructs another candidate using an uppercase extension:

```text
modified_normal.SPC
```

If that exists, it logs:

```text
Base *does* exist with uppercase extension, though
```

---

# ⚠️ Important Uppercase-Extension Quirk

The uppercase candidate is only used for detection/logging.

The code does **not** replace the original `base` variable with `base2`.

Therefore the subsequent call to:

```text
CreateUPSPatch()
```

still receives the original lowercase-extension base path.

This means an uppercase-only base file may still cause the UPS command to receive a nonexistent lowercase path.

This is an implementation limitation, not merely a documentation detail.

---

# ⚠️ Missing Modified File

WhiteSheet also checks:

```text
std::filesystem::exists(modified)
```

and logs:

```text
WARNING: Modified does not exist: "<modified>"
```

if it is missing.

It nevertheless proceeds to call:

```text
CreateUPSPatch()
```

with that path.

The UPS tool therefore remains responsible for ultimately rejecting the missing input.

---

# 🧪 Standard Patch Example

Given:

```text
Distribute/
├── chapter3_text_US_normal.spc
└── chapter3_text_US.spc
```

WhiteSheet constructs:

```text
base:
chapter3_text_US_normal.spc

modified:
chapter3_text_US.spc

output:
chapter3_text_US_patch.ups
```

and executes the UPS diff operation.

---

# 🧪 Multiple Patch Example

If:

```text
different_part2.txt
```

contains:

```text
Distribute/chapter1_text_US.spc
Distribute/chapter3_text_US.spc
Distribute/game_resident_US.spc
```

WhiteSheet generates:

```text
Distribute/chapter1_text_US_patch.ups
Distribute/chapter3_text_US_patch.ups
Distribute/game_resident_US_patch.ups
```

assuming the corresponding base/modified files are available.

---

# 🧪 Special Trial Patch Generation

Trial SPCs bypass `different_part2.txt`.

Instead, WhiteSheet explicitly creates patches between:

```text
DGRV3/base_spc/trial_english_extracted/
```

and:

```text
DGRV3/base_spc/trial_french_extracted/
```

The output is placed into:

```text
Distribute/wrd_data/
```

---

# 🇫🇷 Why English → French?

The tool deliberately uses:

```text
English
    ↓
French
```

rather than:

```text
English
    ↓
Modified English
```

for these trial resources.

The reason is practical rather than linguistic:

> The French trial resources provide better text positioning/layout in debate sequences.

Therefore the French SPC acts as the modified binary input for the UPS patch.

---

# 🧪 Trial Patch Naming

A French trial resource such as:

```text
hanron_01_FR.spc
```

is paired with:

```text
hanron_01_US.spc
```

as its English/base counterpart.

The resulting patch is:

```text
hanron_01_US_patch.ups
```

The patch is deliberately named after the **US resource**, because that is the resource it patches.

---

# 📂 Trial Patch Destination

All special trial patches are written to:

```text
Distribute/wrd_data/
```

rather than the ordinary directory containing the source SPC.

---

# 🧾 Hardcoded Trial Set

The trial patches are not discovered dynamically.

The source contains explicit `CreateUPSPatch()` calls.

The supported trial families include:

```text
hanron
nonstop
panic
kokoronpa
nico
```

---

# ⚠️ Trial Files Are Not Driven by `different_part2.txt`

This is important.

Adding a trial file to:

```text
different_part2.txt
```

is not the intended mechanism for enabling its special patch generation.

The current implementation explicitly lists the trial files in source code.

Adding new trial resources therefore requires modifying WhiteSheet itself.

---

# 🧪 Trial Directory Structure

The expected source layout is:

```text
DGRV3/
└── base_spc/
    ├── trial_english_extracted/
    │   ├── hanron_01_US.spc
    │   ├── hanron_02_US.spc
    │   └── ...
    │
    └── trial_french_extracted/
        ├── hanron_01_FR.spc
        ├── hanron_02_FR.spc
        └── ...
```

The generated patches go to:

```text
Distribute/
└── wrd_data/
    ├── hanron_01_US_patch.ups
    ├── hanron_02_US_patch.ups
    └── ...
```

---

# 🧹 Final Cleanup

After all ordinary and trial patches have been attempted, WhiteSheet performs a cleanup pass over:

```text
Distribute/
```

recursively.

Its goal is to leave behind only valid UPS patches.

---

# 🗑️ What Gets Deleted

Every non-directory file is inspected.

A file is preserved only if:

1. its path contains `.ups`, and
2. its file size is greater than:

```text
0x20
```

which is:

```text
32 bytes
```

Everything else is scheduled for deletion.

---

# 📏 UPS Validity Heuristic

WhiteSheet does **not** parse the UPS file format to validate it.

Instead, it uses:

```text
file size > 0x20
```

as its validity test.

Therefore:

```text
UPS > 32 bytes
    → considered valid

UPS ≤ 32 bytes
    → considered invalid
```

This is explicitly described in the source as an arbitrary threshold.

---

# ⚠️ The 32-Byte Check Is Heuristic

A file being larger than 32 bytes does not necessarily guarantee that it is a semantically valid UPS patch.

Likewise, a valid-but-unusually-small file would be deleted.

The check exists primarily as a practical safeguard against failed or empty UPS outputs.

---

# 🗑️ Non-UPS Files Are Deleted

The cleanup means that files such as:

```text
.spc
.pb
.txt
.stx
```

remaining in `Distribute/` after Part 2 are removed.

This is intentional.

The final `Distribute/` directory is meant to contain patch artifacts, not the temporary source files used to create them.

---

# 🧹 Cleanup Is Recursive

Unlike Crossbow's Part 2 cleanup, WhiteSheet uses:

```cpp
std::filesystem::recursive_directory_iterator(distr)
```

Therefore the cleanup also examines files inside:

```text
Distribute/wrd_data/
Distribute/wrd_script/
...
```

and other nested directories.

---

# 🗂️ Directories Are Preserved

Directories themselves are skipped during cleanup.

Only files are scheduled for removal.

This means empty directories may remain after the cleanup operation.

---

# 🧹 Deletion Process

WhiteSheet first builds a list:

```cpp
std::vector<std::string> todelete;
```

It does not immediately delete each file during the scan.

After scanning is complete, it loops over the list and removes each file.

This avoids modifying the directory tree while the recursive iterator is actively traversing it.

---

# 🛠️ Temporary `tempfile.exe`

At the end of Part 2, WhiteSheet checks for:

```text
tempfile.exe
```

in the current working directory.

If it exists, WhiteSheet removes it.

This appears to be cleanup for a temporary UPS executable.

---

# 🏁 Part 2 Completion

After cleanup, WhiteSheet logs:

```text
Patch files generated!
```

removes any leftover:

```text
tempfile.exe
```

and calls:

```text
Common::WaitExit()
```

---

# 📊 Final Output

A successful Part 2 run is intended to leave:

```text
Distribute/
├── *_patch.ups
└── wrd_data/
    └── *_patch.ups
```

with non-UPS temporary resources removed.

---

# 🔗 Relationship With Crossbow

WhiteSheet directly follows Crossbow.

The expected relationship is:

```text
Crossbow
    ↓
ModifiedFiles/
    ↓
WhiteSheet Part 1
    ↓
Distribute/
    ├── original_normal.spc
    └── modified.spc
    ↓
WhiteSheet Part 2
    ↓
*_patch.ups
```

Crossbow determines **what needs to be distributed**.

WhiteSheet determines **how those distributed resources become patches**.

---

# 🔗 Relationship With ElectroHammer

ElectroHammer produces the modified SPC resources.

WhiteSheet does not itself inject translated text.

Its role begins after those modifications already exist.

The relationship is:

```text
ElectroHammer
    ↓
modified SPC
    ↓
Crossbow
    ↓
ModifiedFiles/
    ↓
WhiteSheet
    ↓
UPS patch
```

---

# 📄 Important Input Files

| File / Directory           | Purpose                                    |
| -------------------------- | ------------------------------------------ |
| `TextConfig.config`        | Global TextTools configuration             |
| `ModifiedFiles/`           | Modified resources from Crossbow           |
| `Distribute/`              | Working distribution directory             |
| `DGRV3/`                   | Game repository                            |
| `DGRV3/base_spc/`          | Base SPC resources                         |
| `DGRV3/base_spc/ups(.exe)` | UPS generator                              |
| `DGRV3/i18n/`              | Switch/i18n resources                      |
| `different_part2.txt`      | List of resources requiring UPS generation |
| `trial_english_extracted/` | English trial bases                        |
| `trial_french_extracted/`  | French trial modified resources            |

---

# 📄 Files Created or Modified

## Part 1

WhiteSheet creates/changes:

```text
Distribute/*_normal.spc
Distribute/<modified resources>
```

and, when enabled:

```text
Distribute/<i18n resources>
```

---

## Part 2

WhiteSheet creates:

```text
Distribute/*_patch.ups
Distribute/wrd_data/*_patch.ups
```

It may also remove:

```text
Distribute/*.spc
Distribute/*.pb
Distribute/*.txt
Distribute/*.stx
```

and any UPS files that are 32 bytes or smaller.

---

# ⚠️ Important Special Cases

## 1. Trial SPCs never follow the ordinary Part 1 workflow

They are skipped during `_normal` generation.

---

## 2. Trial patches use French SPCs

The French files are intentionally treated as the modified side.

---

## 3. Trial patches are hardcoded

They are not dynamically discovered.

---

## 4. Switch builds automatically include PB files

When `UseSwitchConfiguration` is enabled, `.pb` files in `Distribute/` are added to the patch set.

---

## 5. Switch builds also copy `i18n`

The contents of:

```text
DGRV3/i18n/
```

are copied into `Distribute/`.

---

## 6. Existing `_normal` files are protected

WhiteSheet does not repeatedly rename files containing `normal`.

---

## 7. `_extracted` resources are ignored

This prevents temporary extraction artifacts from entering the patch pipeline.

---

## 8. Failed UPS files are deleted

`CreateUPSPatch()` removes the output if the UPS command fails.

---

## 9. Cleanup removes all non-valid UPS files

Part 2 is destructive toward leftover non-UPS resources in `Distribute/`.

---

# 🚨 Developer Warning: Part 2 Is Destructive

Part 2 should not be treated as a harmless read-only operation.

After patch generation it recursively scans:

```text
Distribute/
```

and deletes every file that is not considered a valid UPS file.

Therefore developers should **not place unrelated files inside `Distribute/` before running Part 2**.

For example:

```text
Distribute/
├── some_notes.txt
├── debug.bin
└── chapter1_patch.ups
```

will result in:

```text
Distribute/
└── chapter1_patch.ups
```

after cleanup, assuming the UPS patch passes the size test.

---

# ⚠️ Developer Warning: Part 1 Also Mutates `Distribute/`

Part 1 does not merely copy files.

It renames existing resources:

```text
foo.spc
```

to:

```text
foo_normal.spc
```

Therefore running Part 1 changes the contents of the distribution directory.

Repeated execution is expected to be safe for already-normalized files, but the directory should still be treated as a working area rather than immutable source storage.

---

# 🛠️ Developer Notes

## Adding a New Trial File

To support a new trial SPC, update both:

1. the trial detection logic used during Part 1, and
2. the hardcoded `CreateUPSPatch()` calls in Part 2.

Simply placing the file in:

```text
trial_french_extracted/
```

does not automatically cause a patch to be generated.

---

## Adding a New Switch Resource

If a new Switch-specific binary resource needs patching, ensure it is correctly represented in `Distribute/` and that the appropriate configuration causes WhiteSheet to include it.

PB files receive automatic handling when:

```text
UseSwitchConfiguration
```

is enabled.

---

## Changing the UPS Executable

The UPS executable location is hardcoded relative to:

```text
DGRV3/base_spc/
```

Changing the tool location requires a source modification.

---

## Changing the Patch Threshold

The current validity threshold is:

```text
0x20
```

or:

```text
32 bytes
```

Changing it requires modifying the cleanup logic.

---

## Changing the Patch Naming Scheme

The ordinary patch name is generated by:

```text
<basename>_patch.ups
```

Changing this requires modifying the Part 2 output-path construction.

---

# 🧠 Conceptual Model

WhiteSheet can be understood as a **three-file transformation**:

```text
Original game resource
        │
        ▼
*_normal.spc
        │
        │       Modified resource
        │              │
        │              ▼
        └──────────► UPS diff
                       │
                       ▼
                 *_patch.ups
```

The `_normal` file is the reference state.

The ordinary SPC/PB is the modified state.

The UPS file is the compact difference between those two states.

---

# 🔄 Complete WhiteSheet Workflow

## Part 1

```text
Start
  │
  ▼
Read TextConfig.config
  │
  ▼
Verify UPS executable
  │
  ▼
Create Distribute/ if needed
  │
  ▼
Scan Distribute/
  │
  ├── ignore _extracted
  ├── ignore trial SPCs
  └── rename normal resources
          │
          ▼
      *_normal.spc
          │
          ▼
Scan ModifiedFiles/
  │
  ├── ignore _extracted
  ├── ignore trial SPCs
  └── collect modified filenames
          │
          ▼
Copy modified resources
          │
          ▼
If Switch:
copy DGRV3/i18n/
          │
          ▼
Part 1 complete
```

---

## Part 2

```text
Start
  │
  ▼
Read different_part2.txt
  │
  ▼
Build unique changed-resource set
  │
  ▼
If Switch:
add PB files
  │
  ▼
For each changed resource
  │
  ├── calculate *_normal base
  ├── calculate *_patch.ups output
  ├── verify paths
  └── invoke UPS diff
          │
          ▼
Generate special trial patches
English → French
          │
          ▼
Scan Distribute/
          │
          ▼
Delete non-UPS files
and UPS files ≤ 32 bytes
          │
          ▼
Delete tempfile.exe
          │
          ▼
Final UPS distribution
```

---

# 🏁 Summary

WhiteSheet is the **final patch-generation component of TextTools**.

It does not translate text, compile scripts, or modify SPC contents itself.

Instead, it converts the completed resource modifications into a distributable patch format.

Its workflow is divided into two stages:

### Part 1 — Preparation

```text
Original SPC
    ↓
*_normal.spc

ModifiedFiles/
    ↓
Distribute/
```

### Part 2 — Patch Generation

```text
*_normal.spc
       +
modified SPC
       ↓
UPS diff
       ↓
*_patch.ups
```

Special trial resources follow a separate path:

```text
English trial SPC
       +
French trial SPC
       ↓
UPS diff
       ↓
US-named trial patch
```

The final result is a `Distribute/` directory containing the UPS patches required for release.

In short:

```text
ElectroHammer
    ↓
Build modified game resources

Crossbow
    ↓
Select and distribute modified resources

WhiteSheet Part 1
    ↓
Create the base/modified comparison set

WhiteSheet Part 2
    ↓
Generate the final UPS patches
```

**WhiteSheet is therefore the final bridge between the TextTools build pipeline and the actual distributable patch files.**