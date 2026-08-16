# TEXTTOOLS - NECRONOMICON
## Team DAIX, 2026  

---

## 📌 Overview

**Necronomicon** is the integrity and change-detection component of **TextTools**.

Its primary purpose is to determine **which files in the translated repository differ from their English/reference counterparts at the byte level**.

Where **MarkerStone** asks:

> “Do these two files contain the same number of lines?”

Necronomicon asks:

> “Are these two files made of exactly the same bytes?”

That distinction is important.

A file can have the exact same number of lines while still differing because of:

- translated text,
- whitespace,
- punctuation,
- encoding,
- newline style,
- invisible characters,
- binary content,
- or any other byte-level modification.

Necronomicon detects those differences by calculating **SHA-512 digests** from the raw contents of files and comparing the resulting hashes.

The output of Necronomicon is not merely a diagnostic report. Its most important output is:

```text
different_hashes.txt
````

This file becomes an **input to later TextTools stages**, especially HydraulicPress and Electrohammer.

In other words, Necronomicon answers the pipeline's central question:

```text
Which script files actually changed?
```

That allows the rest of TextTools to avoid processing the entire repository unnecessarily.

---

# 🎯 Responsibilities

Necronomicon has two distinct operating modes.

### Normal / Part 1 mode

Compares:

```text
DGRV3/
```

against:

```text
DGRV3_EN/
```

and determines which corresponding files have different SHA-512 hashes.

### `part2` mode

Compares modified files in:

```text
Distribute/
```

against their corresponding:

```text
*_normal.*
```

SHA files.

The two modes serve different stages of the TextTools pipeline.

---

# 🧩 How Necronomicon Fits Into TextTools

The normal TextTools pipeline is approximately:

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
Electrohammer
    ↓
Crossbow
    ↓
WhiteSheet
```

Necronomicon therefore operates immediately after the initial structural validation performed by MarkerStone.

Its results are consumed by multiple later tools.

The most important dependency is:

```text
Necronomicon
    │
    └── different_hashes.txt
            │
            ├── HydraulicPress
            │
            └── Electrohammer
```

HydraulicPress explicitly reads `different_hashes.txt` when `CheckAllFiles` is disabled, using it to determine which script files need variable processing. 

This makes Necronomicon much more than a passive checksum utility.

It is effectively the **change-tracking engine for the compilation pipeline**.

---

# 🏗️ High-Level Architecture

Necronomicon is composed of two principal source components:

```text
Necronomicon.cpp
    │
    ├── repository scanning
    ├── file filtering
    ├── SHA generation
    ├── English/translation comparison
    ├── mismatch reporting
    ├── folder-index generation
    └── part2 comparison
          │
          ▼
calc_sha.h
    │
    └── HashCalc
          ├── DoCalc()
          ├── CalcFast()
          └── CalculateHash()
```

`Necronomicon.cpp` controls **what is hashed and compared**.

`calc_sha.h` controls **how the hash is calculated**.

---

# 🔐 Hashing Engine

The hashing implementation is provided by:

```text
calc_sha.h
```

The core class is:

```cpp
HashCalc
```

The normal hashing path ultimately calls:

```cpp
HashCalc::CalculateHash<SHA512>(...)
```

and uses the configured/private cryptographic dependency exposed through:

```cpp
#define CoolNameLibrary MyLibrary
```

The source comments describe this dependency as something that can either be supplied beside the project or installed through `vcpkg`. 

---

# 🔢 Why SHA-512?

Necronomicon uses SHA-512 as a compact representation of the entire file contents.

Conceptually:

```text
file bytes
    ↓
SHA-512
    ↓
128 hexadecimal characters
```

Two files with identical bytes produce the same digest.

Two files with different contents should produce different digests.

Therefore:

```text
SHA_A == SHA_B
```

means the files are considered identical by the hashing system, while:

```text
SHA_A != SHA_B
```

means Necronomicon treats them as changed.

---

# ⚠️ Important: Necronomicon Hashes Raw Bytes

One of the most important implementation details is that files are opened in:

```cpp
std::ios::binary
```

mode.

The entire file is copied into a string without line-oriented parsing or newline normalization.

The source explicitly describes this as:

```text
exact bytes, no line ending normalization
```

before passing the contents to the SHA-512 implementation. 

Therefore a change such as:

```text
LF
```

→

```text
CRLF
```

can produce a different hash.

Likewise:

```text
"hello"
```

and:

```text
"hello "
```

are different files from Necronomicon's perspective.

This is why the tool is properly described as a **byte-level integrity checker**, rather than merely a text comparison tool.

---

# 📁 Repository Inputs

Normal mode expects two directories in the current working directory:

```text
DGRV3/
DGRV3_EN/
```

These represent:

| Directory   | Purpose                       |
| ----------- | ----------------------------- |
| `DGRV3/`    | translated/current repository |
| `DGRV3_EN/` | English/reference repository  |

Necronomicon constructs both paths from the current working directory. 

If either directory does not exist, normal-mode execution terminates with an error.

---

# 📂 Current Working Directory

Necronomicon does not accept the repository paths as command-line arguments.

Instead it uses:

```cpp
std::filesystem::current_path()
```

and expects the repository structure to exist beneath that location.

Therefore the working directory is significant.

For example, the expected layout is approximately:

```text
TextTools/
├── Necronomicon.exe
├── TextConfig.config
├── different_hashes.txt
├── DGRV3/
│   └── ...
└── DGRV3_EN/
    └── ...
```

Running Necronomicon from the wrong directory can cause it to look for repositories in the wrong place.

---

# 📥 Files Processed in Normal Mode

The normal hashing mode primarily processes:

```text
.txt
```

files.

When:

```text
UseSwitchConfiguration
```

is enabled, `.pb` files are also considered.

The extension check is case-insensitive because the extension is converted to lowercase before comparison. 

Therefore:

```text
scene.txt
```

is valid, and Switch-mode files such as:

```text
scene.pb
```

can also be valid.

---

# 🎮 Switch Support

Switch builds use binary `.pb` text resources rather than relying exclusively on `.txt`.

Necronomicon therefore changes its accepted file set when:

```text
UseSwitchConfiguration = true
```

is active.

Normal mode becomes:

```text
.txt
.pb   ← Switch
```

instead of simply:

```text
.txt
```

This behavior is implemented in `DidSaveSha()`. 

---

# 📥 Files Processed in Part2

Part2 changes the target file type.

Normal mode:

```text
.txt
```

or Switch:

```text
.pb
```

Part2 mode:

```text
.spc
```

or Switch:

```text
.pb
```

The same `DidSaveSha()` function handles this distinction using its:

```cpp
ispart2
```

parameter. 

---

# 🚫 File Filtering

Before hashing a file, Necronomicon runs:

```cpp
PerformChecks(...)
```

This prevents metadata and generated files from being treated as script content.

The following path substrings are rejected:

```text
_sha
_lines
README
LICENSE
.git
Baked
```

The filtering is substring-based rather than a strict filename comparison. 

For example, a path containing:

```text
README
```

anywhere in the path is excluded.

---

# 🚫 Additional Normal-Mode Exclusions

During the translation comparison pass, Necronomicon additionally ignores:

```text
base_spc
vars_bak
```

These are deliberately excluded because they are not ordinary translated script files being compared against the English script repository. 

---

# 🧮 Creating a SHA File

For every accepted file, Necronomicon performs the following sequence:

```text
file
 ↓
open binary
 ↓
read entire contents
 ↓
calculate SHA-512
 ↓
create <filename>_sha
 ↓
write digest
```

For example:

```text
chapter3/scene_12.txt
```

produces:

```text
chapter3/scene_12.txt_sha
```

The SHA file contains the digest followed by a newline. 

---

# 📄 SHA File Format

A generated SHA file contains a single hexadecimal digest:

```text
<128-character SHA-512 digest>
```

followed by:

```text
\n
```

The comparison code deliberately removes CR/LF characters when reading stored SHA files, ensuring that the newline written to the `_sha` file does not become part of the comparison value. 

---

# 🔄 Normal Mode Workflow

Normal mode performs several passes.

Conceptually:

```text
Pass 1
English repository
    ↓
calculate SHA files

Pass 2
Translated repository
    ↓
determine expected comparison files

Sanity check
    ↓
make sure file counts match

Pass 3
Translated files
    ↓
load translated SHA
    ↓
locate English equivalent
    ↓
load English SHA
    ↓
compare

Pass 4
mismatched files
    ↓
map to folder indexes
```

The implementation explicitly documents these four stages. 

---

# 🇬🇧 Pass 1 — English SHA Generation

Necronomicon recursively scans:

```text
DGRV3_EN/
```

Every candidate file is passed to:

```cpp
DidSaveSha(file, false)
```

If the file passes the extension and filtering rules, a corresponding:

```text
<file>_sha
```

is created.

The paths of successfully processed English files are also stored internally for later sanity checking. 

---

# 🇮🇹 Pass 2 — Translation File Enumeration

Necronomicon then scans:

```text
DGRV3/
```

The translated files are not immediately compared one-by-one.

Instead, Necronomicon first determines how many files **should participate in the comparison**.

It records:

```text
TotalEnglishFiles
TotalItalianFiles
TotalSHAEnglishFiles
TheoreticalItalianSHAFiles
```

These counters are used as a repository consistency check. 

---

# 🛡️ File-Count Sanity Check

Before comparing hashes, Necronomicon verifies that:

```text
TheoreticalItalianSHAFiles
```

equals:

```text
TotalSHAEnglishFiles
```

If they differ, the comparison is considered unreliable and Necronomicon aborts the normal comparison process.

The tool writes diagnostic lists:

```text
sha_en.txt
sha_it.txt
```

containing the paths it considered on each side. 

This prevents a missing or extra script file from silently producing an incomplete comparison.

---

# 📋 Diagnostic File: `sha_en.txt`

When the file-count sanity check fails, Necronomicon writes the shortened English file list to:

```text
sha_en.txt
```

This allows developers to inspect exactly which files were considered hashable on the English side.

---

# 📋 Diagnostic File: `sha_it.txt`

Likewise, the translated-side file list is written to:

```text
sha_it.txt
```

Together:

```text
sha_en.txt
sha_it.txt
```

can be compared to determine why the repository structures are no longer synchronized.

---

# 🔍 Pass 3 — Actual Hash Comparison

For every candidate translated file, Necronomicon:

1. identifies the translated file,
2. determines the corresponding English path,
3. reads the translated `_sha`,
4. reads the English `_sha`,
5. removes CR/LF from the stored English digest,
6. compares the strings,
7. records the file if the digests differ.

The core comparison is effectively:

```cpp
if (first_hash != second_hash)
```

with both hashes required to be non-empty. 

---

# 🔁 Translating a Repository Path Into an English Path

Necronomicon uses:

```cpp
ReplaceWithEnglish(...)
```

to transform a path such as:

```text
DGRV3/chapter3/scene_12.txt
```

into:

```text
DGRV3_EN/chapter3/scene_12.txt
```

The function replaces the appropriate occurrence of:

```text
DGRV3
```

with:

```text
DGRV3_EN
```

It contains logic for handling paths where the repository string could occur more than once. 

---

# ⚠️ The English File Is Not Rehashed During Comparison

The comparison pass does not need to calculate the English SHA again.

Instead, it reads the previously generated:

```text
<english file>_sha
```

This is why Pass 1 must complete successfully before comparison begins.

The `_sha` files act as the persisted reference hashes.

---

# 📝 `sha_diff.txt`

Whenever a mismatch is detected, Necronomicon creates a `HashTemp` object containing:

```text
FirstHash
SecondHash
```

and calls:

```cpp
SaveToFile(...)
```

This appends a record to:

```text
sha_diff.txt
```

The generated record has the structure:

```text
Filename: scene_12.txt
First SHA: <translated SHA>
Second SHA: <English SHA>
Same SHA: False
```

followed by a blank line. 

---

# ⚠️ Hash Ordering in `sha_diff.txt`

The report labels the two values:

```text
First SHA
Second SHA
```

The first value is the hash read from the translated/current file.

The second value is the hash read from the corresponding English reference `_sha` file.

Therefore:

```text
First SHA = DGRV3
Second SHA = DGRV3_EN
```

This ordering matters when manually investigating a mismatch.

---

# 📋 `different_hashes.txt`

The most important normal-mode output is:

```text
different_hashes.txt
```

It contains the relative/shortened paths of files whose hashes differ.

For example:

```text
chapter1/scene_05.txt
chapter3/scene_12.txt
game_resident/foo.txt
```

The list is sorted before being written.

Necronomicon writes the paths using:

```cpp
Common::ShortenFilename(j, 2)
```

rather than necessarily writing the full absolute path. 

---

# 🔗 Why `different_hashes.txt` Matters

`different_hashes.txt` is a pipeline control file.

HydraulicPress can use it as its file-selection source:

```text
Necronomicon
    ↓
different_hashes.txt
    ↓
HydraulicPress
    ↓
variable replacement
```

HydraulicPress reads each path and resolves it relative to the current working directory. 

This means that a hash mismatch effectively tells HydraulicPress:

> “This file is different from the English reference. Consider it for processing.”

---

# 📋 `different_hash_folders.txt`

Necronomicon also produces:

```text
different_hash_folders.txt
```

This file contains numeric indexes corresponding to the script folders containing changed files.

For example:

```text
10
12
```

would indicate the corresponding entries in the internal folder table.

The file is deliberately index-based rather than storing folder names. 

---

# 📚 Folder Index Table

The internal folder list is:

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

The list is shared conceptually with other TextTools stages such as MarkerStone and Electrohammer. 

---

# 🧭 Mapping a Mismatch to a Folder

For each changed file, Necronomicon obtains:

```cpp
parent_path().filename()
```

and attempts to find that directory name in the folder table.

For example:

```text
DGRV3/chapter3/scene_12.txt
```

has:

```text
chapter3
```

as its immediate parent directory.

That maps to:

```text
index 10
```

Only unique indexes are retained. 

---

# ⚠️ Unknown Folder Names

If a mismatched file's immediate parent directory is not found in the internal folder table, Necronomicon logs:

```text
Invalid folder string: <folder>
```

It does not invent a new index.

This is important if new script directory categories are introduced into the repository.

The folder table must be updated if those categories are intended to participate in folder-index processing.

---

# 🧹 Report Cleanup

At the beginning of normal mode, Necronomicon removes existing:

```text
different_hash_folders.txt
different_hashes.txt
```

before generating new results. 

This is intentional.

The files represent the **current run**, not an accumulating history.

---

# ⚠️ `sha_diff.txt` Is Different

Unlike the two primary mismatch-list files, `sha_diff.txt` is written using append mode by `HashTemp::SaveToFile()`.

The implementation shown does not perform the same initial cleanup for `sha_diff.txt` that it performs for:

```text
different_hashes.txt
different_hash_folders.txt
```

Therefore developers should be aware that `sha_diff.txt` can behave differently from the primary current-run lists.

If stale records are present, they should not automatically be interpreted as proof that every entry came from the current invocation.

---

# 🧪 `--all` Mode

Necronomicon supports:

```text
--all
```

The purpose is to force the pipeline to treat every known folder as changed.

Instead of running the normal comparison routine, the program adds every folder index:

```text
0
1
2
...
13
```

to `different_indexes`. 

---

# 🔥 What `--all` Actually Means

A subtle but important point:

`--all` does **not** mean:

> “Hash every file and report every hash mismatch.”

Instead, it bypasses the normal `CalculateSome()` selection logic and marks **all known script folders** as needing processing.

Therefore:

```text
Necronomicon --all
```

is effectively a:

```text
force full-folder processing
```

mode.

It is useful when a complete rebuild is desired regardless of what the hash comparison would normally select.

---

# 💻 Command-Line Interface

Normal invocation:

```text
Necronomicon
```

or:

```text
Necronomicon.exe
```

depending on platform.

Force-all invocation:

```text
Necronomicon --all
```

Part2 invocation:

```text
Necronomicon part2
```

The parser also recognizes the relevant flags when they appear as the first or second argument. 

---

# 🔀 Part2 Mode

Part2 is a completely different comparison workflow.

It is activated with:

```text
Necronomicon part2
```

Instead of comparing:

```text
DGRV3/
vs
DGRV3_EN/
```

it operates on:

```text
Distribute/
```

and compares modified distribution files against their corresponding:

```text
_normal
```

reference files.

This mode exists primarily to support the later **WhiteSheet UPS-patching workflow**. 

---

# 📦 Part2 Input Directory

Part2 expects:

```text
Distribute/
```

beneath the current working directory.

The implementation recursively walks this directory.

Directories themselves are ignored.

Each encountered file is initially passed to:

```cpp
DidSaveSha(file, true)
```

so that its SHA can be generated. 

---

# 📦 Part2 File Types

The normal Part2 file types are:

```text
.spc
```

and, when Switch mode is enabled:

```text
.pb
```

This is distinct from normal mode, where `.txt` is the primary file type.

---

# 🧮 Part2 SHA Generation

For every eligible Part2 file:

```text
Distribute/foo.spc
```

Necronomicon creates:

```text
Distribute/foo.spc_sha
```

using the same raw-byte SHA-512 engine.

The important point is that Part2 does not use a different hashing algorithm.

It uses the same:

```text
SHA-512
```

mechanism against a different class of files.

---

# 🔍 Part2 Reference Naming Convention

Suppose the modified file is:

```text
nonstop_01_US.spc
```

The corresponding reference SHA is expected to be:

```text
nonstop_01_US_normal.spc_sha
```

The naming algorithm is:

```text
<original path without extension>
+
"_normal"
+
<original extension>
+
"_sha"
```

For example:

```text
foo.spc
```

becomes:

```text
foo_normal.spc_sha
```

This is explicitly constructed by `DoPart2()`. 

---

# 🚫 Part2 Comparison Exclusions

During the actual Part2 comparison pass, Necronomicon skips files whose paths contain:

```text
_sha
_lines
normal
```

This prevents the tool from treating generated SHA files or the `_normal` reference itself as the modified file being compared. 

---

# 🔄 Part2 Comparison Workflow

The process is:

```text
Distribute/
    ↓
find eligible files
    ↓
calculate SHA
    ↓
create <file>_sha
    ↓
scan files again
    ↓
ignore _sha / _lines / normal
    ↓
construct <file>_normal.ext_sha
    ↓
read both SHA strings
    ↓
compare
    ↓
record changed files
```

---

# 🔤 Part2 Extension Handling

When a mismatch is found, Necronomicon reconstructs the filename using the original extension.

It first attempts the extension as-is/lowercase and then checks an uppercase-extension variant if necessary.

For example, if a modified file is represented with:

```text
.SPC
```

the tool can attempt the corresponding uppercase filename if the lowercase form is not found.

This exists to accommodate inconsistent extension casing in distribution assets. 

---

# 📋 Part2 Output: `different_part2.txt`

If any modified Part2 files differ from their `_normal` counterparts, Necronomicon creates:

```text
different_part2.txt
```

The changed file paths are sorted before being written.

Example:

```text
Distribute/nonstop_01_US.spc
Distribute/hanron_02_US.spc
```

This list is consumed by the subsequent patch-generation stage.

---

# 📋 Part2 Output: `different_part2_short.txt`

Necronomicon also creates:

```text
different_part2_short.txt
```

This contains shortened versions of the changed paths.

The implementation uses:

```cpp
Common::ShortenFilename(j, 1)
```

for this output. 

---

# 🧹 Part2 Report Cleanup

Before writing the Part2 mismatch reports, Necronomicon removes existing:

```text
different_part2.txt
different_part2_short.txt
```

if they exist.

The newly generated files therefore represent the current Part2 comparison run. 

---

# 🔗 Part2 Relationship With WhiteSheet

The Part2 workflow is:

```text
Crossbow part2
    ↓
Distribute/
    ↓
Necronomicon part2
    ↓
different_part2.txt
    ↓
WhiteSheet part2
    ↓
UPS patch generation
```

WhiteSheet explicitly reads:

```text
different_part2.txt
```

to determine which SPC/PB assets require UPS patches. 

Thus Part2 is essentially the **asset-level counterpart** to the normal script comparison mode.

---

# 🔗 Part1 vs Part2

| Property         | Normal Mode                    | Part2                              |
| ---------------- | ------------------------------ | ---------------------------------- |
| Command          | `Necronomicon`                 | `Necronomicon part2`               |
| Primary input    | `DGRV3/` + `DGRV3_EN/`         | `Distribute/`                      |
| Main file type   | `.txt`                         | `.spc`                             |
| Switch file type | `.pb`                          | `.pb`                              |
| Reference        | English repository             | `_normal` file                     |
| Main output      | `different_hashes.txt`         | `different_part2.txt`              |
| Secondary output | `different_hash_folders.txt`   | `different_part2_short.txt`        |
| Main consumer    | HydraulicPress / Electrohammer | WhiteSheet                         |
| Purpose          | Detect changed scripts         | Detect changed distribution assets |

---

# 🧠 HashCalc Internals

The `HashCalc` class provides three public hashing interfaces.

---

## `DoCalc()`

The simplest entry point is:

```cpp
HashCalc::DoCalc(message)
```

It selects:

```cpp
CoolNameLibrary::SHA512
```

as its hash type and delegates to:

```cpp
CalculateHash<Type>(message)
```

This provides the normal SHA-512 interface. 

---

## `CalcFast()`

The actual fast implementation is:

```cpp
HashCalc::CalcFast<Type2>(message)
```

It:

1. creates the hash object,
2. allocates the digest buffer,
3. calculates the digest,
4. feeds the digest to a hexadecimal encoder,
5. writes the encoded result into a string,
6. returns that string.

The source comments state that this produces uppercase hexadecimal output. 

---

# 🔢 Digest Size

SHA-512 produces:

```text
512 bits
```

which corresponds to:

```text
64 bytes
```

or:

```text
128 hexadecimal characters
```

The implementation allocates the digest according to:

```cpp
Type2::DIGESTSIZE
```

so the generic hashing template can theoretically support other algorithms as well.

---

# 🧩 Generic Hash Template

`CalcFast()` is templated:

```cpp
template<typename Type2 = CoolNameLibrary::SHA512>
```

and `CalculateHash()` is likewise templated.

Therefore the infrastructure could theoretically be instantiated with other hash types supported by the dependency.

However, Necronomicon itself explicitly invokes:

```cpp
SHA512
```

for its repository comparison logic.

So SHA-512 is the actual application-level contract.

---

# 🐌 Slow/Fallback Mode

`CalculateHash()` contains:

```cpp
bool constexpr slow = false;
```

The `slow` branch is currently disabled.

If it were enabled, the current fallback implementation would simply return the input string instead of calculating a cryptographic digest.

That is explicitly described in the source as a temporary/no-hash fallback. 

This is therefore **not a real cryptographic fallback implementation**.

It is effectively a placeholder.

---

# ⚠️ Developer Warning: Do Not Enable the Current Slow Branch

The disabled branch should not be mistaken for an alternative SHA-512 implementation.

If:

```cpp
slow = true;
```

were enabled in the code as currently written:

```text
CalculateHash(input)
```

would return:

```text
input
```

rather than:

```text
SHA512(input)
```

That would destroy the semantic meaning of the generated `_sha` files.

The branch should therefore be treated as a development placeholder unless replaced with a genuine hashing implementation.

---

# 📤 Hash Output Stability

Necronomicon relies on the digest being serialized consistently.

The hexadecimal encoder produces a stable textual representation.

The resulting value is written to disk as:

```text
<digest>\n
```

and later read back for comparison.

The comparison removes CR/LF characters from the loaded English SHA value so that the storage newline does not affect the logical hash comparison. 

---

# 🛡️ Error Handling During SHA Generation

`DidSaveSha()` uses exception-enabled streams.

Failures during:

```text
open
read
hash
write
```

are caught.

The tool logs either:

```text
ERROR: Failed to compute or write SHA for ...
```

or:

```text
ERROR: Unknown failure while processing ...
```

and returns:

```text
false
```

rather than allowing the individual failure to silently appear successful. 

---

# ⚠️ Error Handling During Repository Scanning

The English SHA pass catches exceptions around individual files.

Likewise, comparison processing catches exceptions on individual files and logs:

```text
ERROR comparing file: ...
```

This means one problematic file can produce a diagnostic while the surrounding scan may continue.

The file-count sanity check provides an additional layer of protection against silently proceeding with an incomplete repository. 

---

# 🚨 Missing English SHA

During comparison, Necronomicon expects the English `_sha` file to exist.

If the corresponding reference hash cannot be found, the comparison cannot be completed for that file.

The code logs the problem and continues through the comparison loop rather than treating that individual missing reference as a valid match. 

This distinction is important:

```text
missing reference SHA
```

does **not** mean:

```text
hashes match
```

---

# 🚨 Empty Hashes

A mismatch is only recorded when:

```text
first_hash != empty
AND
second_hash != empty
AND
first_hash != second_hash
```

Therefore two empty values are not treated as a meaningful mismatch.

This protects the mismatch report from being polluted by completely missing hash data. 

---

# 📊 Why Both MarkerStone and Necronomicon Exist

MarkerStone and Necronomicon overlap conceptually but serve different purposes.

### MarkerStone

Checks:

```text
line count
```

It answers:

> Do these files have the same structural number of lines?

### Necronomicon

Checks:

```text
SHA-512 of raw bytes
```

It answers:

> Are these files byte-for-byte equivalent?

Therefore:

```text
MarkerStone
    ↓
structural integrity

Necronomicon
    ↓
content integrity
```

Using both provides a much stronger pre-compilation validation system.

---

# 🧪 Example: Same Line Count, Different Hash

Consider:

```text
English:
Hello world.
Goodbye.

Translated:
Ciao mondo.
Arrivederci.
```

Both files contain:

```text
2 lines
```

so MarkerStone may find:

```text
line count = match
```

But their SHA-512 values will differ.

Necronomicon therefore records the file as changed.

This is exactly the distinction between **structural comparison** and **content comparison**.

---

# 🧪 Example: Invisible Difference

Suppose two files appear visually identical:

```text
Hello world.
```

but one uses:

```text
LF
```

and the other:

```text
CRLF
```

Because Necronomicon hashes the raw binary contents, the SHA values can differ.

Thus a visually invisible formatting change can still cause:

```text
different_hashes.txt
```

to include the file.

---

# 🧪 Example: Trailing Whitespace

These are logically different to Necronomicon:

```text
Hello
```

and:

```text
Hello␠
```

where `␠` represents a trailing space.

The raw bytes differ, therefore the SHA-512 digest differs.

---

# 🧪 Example: Translation Change

A legitimate translation:

```text
Hello, Makoto!
```

→

```text
Ciao, Makoto!
```

naturally changes the SHA.

Necronomicon is therefore **not intended to prove that a translation is incorrect**.

It identifies that the file differs from the reference.

Later tools interpret that difference as a reason to process the file.

---

# 🧠 What a Hash Mismatch Means

A mismatch means:

```text
translated bytes != English bytes
```

It does **not** inherently mean:

```text
corruption
```

or:

```text
error
```

In a translation project, most intended translated files are expected to differ.

Necronomicon's job is to identify those differences reliably so the rest of the pipeline knows which assets have changed.

---

# 🔗 Interaction With Electrohammer

Electrohammer uses Necronomicon's mismatch information to determine which script entries need compilation.

Its entry-management layer explicitly consumes:

```text
different_hashes.txt
different_hash_folders.txt
```

to decide which `.txt` files should be compiled into SPC archives. 

This means Necronomicon effectively determines the **scope of Electrohammer's compilation work**.

---

# 🔗 Interaction With HydraulicPress

HydraulicPress can operate in two broad file-selection modes.

When `CheckAllFiles` is enabled:

```text
scan repository
```

When it is disabled:

```text
read different_hashes.txt
```

and process those files.

The relevant code explicitly documents this behavior. 

Thus Necronomicon can act as HydraulicPress's incremental-build manifest.

---

# 🔗 Interaction With Crossbow

Crossbow also consumes:

```text
different_hashes.txt
```

to determine which changed SPC/PB assets should be distributed.

This allows Crossbow to avoid blindly copying every base SPC.

The result is an incremental distribution workflow:

```text
changed script
    ↓
compiled SPC
    ↓
Crossbow
    ↓
only relevant distribution assets
```

---

# 🔗 Interaction With WhiteSheet

WhiteSheet does not normally consume the normal-mode hash report directly for its final UPS patch generation.

Instead, its Part2 workflow relies on:

```text
different_part2.txt
```

which is generated by:

```text
Necronomicon part2
```

WhiteSheet then uses that list to identify modified SPC/PB files for patch creation. 

---

# 🔄 Complete Normal Pipeline

A typical incremental script build can therefore look like:

```text
DGRV3_EN/
      │
      │ reference
      ▼
Necronomicon
      ▲
      │
DGRV3/
      │
      ▼
SHA comparison
      │
      ├── same
      │     └── no entry in different_hashes.txt
      │
      └── different
            │
            ├── different_hashes.txt
            │
            ├── different_hash_folders.txt
            │
            └── sha_diff.txt
                    │
                    ▼
             HydraulicPress
                    │
                    ▼
             Electrohammer
                    │
                    ▼
                 Crossbow
```

---

# 🔄 Complete Part2 Pipeline

The asset-patching side is:

```text
Crossbow part2
      │
      ▼
Distribute/
      │
      ▼
Necronomicon part2
      │
      ├── calculate modified SHA
      │
      ├── compare against *_normal SHA
      │
      ▼
different_part2.txt
      │
      ▼
WhiteSheet part2
      │
      ▼
UPS patch generation
```

This is the second major role of Necronomicon.

---

# 🗂️ Generated Files

| File                         | Mode          | Purpose                                                   |
| ---------------------------- | ------------- | --------------------------------------------------------- |
| `<file>_sha`                 | Both          | SHA-512 digest for an individual file                     |
| `different_hashes.txt`       | Normal        | List of translated files whose hashes differ from English |
| `different_hash_folders.txt` | Normal        | Numeric folder indexes containing mismatches              |
| `sha_diff.txt`               | Normal        | Detailed hash comparison records                          |
| `sha_en.txt`                 | Normal, error | English candidate-file diagnostic list                    |
| `sha_it.txt`                 | Normal, error | Translated candidate-file diagnostic list                 |
| `different_part2.txt`        | Part2         | Full list of modified distribution assets                 |
| `different_part2_short.txt`  | Part2         | Shortened list of modified distribution assets            |

---

# 📄 Output Lifecycle

## Normal mode

At startup, Necronomicon removes:

```text
different_hashes.txt
different_hash_folders.txt
```

and regenerates them.

Therefore downstream tools should treat these as **current-run manifests**.

---

## Part2 mode

Part2 removes:

```text
different_part2.txt
different_part2_short.txt
```

before writing new results.

Again, these represent the current comparison.

---

# ⚠️ Important Output Quirk: Empty Mismatch Set

If no mismatches are found, Necronomicon does not necessarily produce a useful populated:

```text
different_hashes.txt
```

Instead, the code logs:

```text
WARNING: No different indexes found!
```

when the folder-index vector is empty.

Downstream tools should therefore handle the absence or emptiness of the mismatch manifest gracefully. 

---

# ⚠️ Important Output Quirk: `different_hashes.txt` Contains Paths, Not Hashes

Despite its name:

```text
different_hashes.txt
```

does **not** contain the SHA values themselves.

It contains:

```text
paths of files whose hashes differ
```

The actual hash values are recorded in:

```text
sha_diff.txt
```

This distinction is important when writing tools that consume the reports.

---

# ⚠️ Important Output Quirk: `different_hash_folders.txt` Contains Numbers

Likewise:

```text
different_hash_folders.txt
```

does not contain:

```text
chapter1
chapter3
```

It contains numeric indexes:

```text
12
10
```

which must be interpreted using the folder table.

---

# 🛠️ Developer Notes

## 1. SHA files are generated alongside source files

A file such as:

```text
scene.txt
```

gets:

```text
scene.txt_sha
```

rather than a separate checksum directory.

---

## 2. SHA files are themselves excluded

Because `_sha` is explicitly filtered, previously generated checksum files do not become input to subsequent hashing passes.

---

## 3. `_lines` files are excluded

This keeps MarkerStone's companion files outside the hashing set.

---

## 4. The hashing is content-based, not timestamp-based

Necronomicon does not use:

```text
file modification time
```

or:

```text
file size
```

as its primary comparison mechanism.

It hashes the actual bytes.

---

## 5. File paths are shortened for reports

The internal comparison uses filesystem paths, but reports use `Common::ShortenFilename()`.

Consumers should therefore treat the report format as **repository-relative paths**, not arbitrary absolute paths.

---

## 6. The repository layout is part of the contract

The following assumptions are hard-coded:

```text
DGRV3
DGRV3_EN
```

as well as the known folder names.

Changing those names requires corresponding source changes.

---

## 7. The English branch is the reference

The translated repository is compared against:

```text
DGRV3_EN/
```

The tool does not perform symmetric comparison of two arbitrary repositories.

---

# ⚠️ Configuration Dependency

Necronomicon loads:

```text
TextConfig.config
```

at startup.

This is used in particular for:

```text
UseSwitchConfiguration
```

which determines whether `.pb` files participate in hashing. 

Therefore the active configuration can change the set of files considered by the tool.

---

# 🧪 Reproducibility Considerations

For reliable results, developers should ensure:

* the same repository layout is used,
* the correct `TextConfig.config` is active,
* the correct platform mode is selected,
* files are not being modified while Necronomicon scans them,
* previous generated reports are not manually mixed with current results,
* `_sha` files are not manually edited.

Because hashes are derived from raw bytes, even seemingly insignificant changes affect the result.

---

# ⚠️ Concurrent Modification

Necronomicon reads files while generating their hashes.

If another process modifies a file while it is being read, the resulting digest may represent a transient state.

For deterministic results, the repository should be treated as read-only while the scan is running.

---

# 🔒 Integrity vs Authenticity

SHA-512 provides an integrity fingerprint.

Necronomicon can tell that:

```text
file A
```

does not match:

```text
file B
```

according to their digests.

It does **not** provide:

* digital signatures,
* author authentication,
* cryptographic provenance,
* proof that a file is "official."

The tool is therefore an **integrity/change detector**, not an authentication system.

---

# 🧠 Why Necronomicon Is Called a "Diff Engine"

Necronomicon does not perform a conventional text diff such as:

```diff
- Hello world
+ Ciao mondo
```

Instead, it performs:

```text
file A
 ↓
SHA-512
 ↓
digest A

file B
 ↓
SHA-512
 ↓
digest B

digest A != digest B
 ↓
changed
```

The actual textual difference is not calculated.

The tool's job is simply to answer:

```text
same or different?
```

The detailed hash report provides evidence of that determination, but not a line-by-line explanation of the content change.

---

# 🧭 Normal-Mode Decision Tree

The normal process can be summarized as:

```text
Start
 │
 ├── part2?
 │      └── YES → Part2 workflow
 │
 └── NO
       │
       ├── DGRV3 exists?
       │      └── NO → error
       │
       ├── DGRV3_EN exists?
       │      └── NO → error
       │
       ├── --all?
       │      └── YES → mark all known folders
       │
       └── NO
              │
              ▼
       Hash English files
              │
              ▼
       Enumerate translated files
              │
              ▼
       File counts match?
          │           │
         NO          YES
          │           │
          ▼           ▼
       diagnostics   compare hashes
       + abort           │
                         ▼
                    mismatches?
                    │         │
                   NO        YES
                    │         │
                    ▼         ▼
                  finish   save reports
```

---

# 📦 Example Repository State

Before running Necronomicon:

```text
DGRV3/
├── chapter1/
│   ├── scene_01.txt
│   └── scene_02.txt
└── chapter3/
    └── scene_12.txt

DGRV3_EN/
├── chapter1/
│   ├── scene_01.txt
│   └── scene_02.txt
└── chapter3/
    └── scene_12.txt
```

After hashing, the English tree will contain checksum companions:

```text
DGRV3_EN/
├── chapter1/
│   ├── scene_01.txt
│   ├── scene_01.txt_sha
│   ├── scene_02.txt
│   └── scene_02.txt_sha
└── chapter3/
    ├── scene_12.txt
    └── scene_12.txt_sha
```

The translated files are then compared against those reference hashes.

---

# 🧪 Example Mismatch

Suppose:

```text
DGRV3/chapter3/scene_12.txt
```

produces:

```text
ABC123...
```

while:

```text
DGRV3_EN/chapter3/scene_12.txt_sha
```

contains:

```text
DEF456...
```

Necronomicon records:

```text
different_hashes.txt
```

with:

```text
chapter3/scene_12.txt
```

and:

```text
sha_diff.txt
```

with the corresponding two digest values.

It also maps:

```text
chapter3
```

to folder index:

```text
10
```

and records that index in:

```text
different_hash_folders.txt
```

---

# 🧪 Example of a Clean Match

If:

```text
translated SHA
=
English SHA
```

then the file is not added to:

```text
different_hashes.txt
```

and no mismatch record is generated for it.

This means downstream incremental tools have no reason to treat the file as changed.

---

# 🧩 Relationship to MarkerStone

The two tools form a useful validation pair:

```text
MarkerStone
    │
    └── line-count integrity

Necronomicon
    │
    └── byte-content integrity
```

MarkerStone can catch structural differences cheaply.

Necronomicon catches actual content differences regardless of whether the number of lines changed.

Together they establish:

```text
structure is consistent
+
changed files are known
=
safe incremental processing
```

---

# 🧩 Relationship to HydraulicPress

HydraulicPress's file-selection mechanism directly demonstrates why Necronomicon's output is operationally important.

When `CheckAllFiles` is disabled, HydraulicPress calls:

```text
ReadDifferentHashesFile("different_hashes.txt")
```

and converts each listed path into a filesystem path before processing it. 

Thus Necronomicon effectively determines the **incremental workload** for HydraulicPress.

---

# 🧩 Relationship to Electrohammer

Electrohammer similarly uses the hash reports to determine which entries need to be compiled.

Its entry-management code accepts:

```text
different_indexes
different_files
```

as inputs when deciding whether a script entry should be compiled. 

Therefore the hash comparison influences not just variable processing but eventual SPC compilation.

---

# 🧩 Relationship to WhiteSheet

WhiteSheet's patch workflow operates on the output of Part2:

```text
different_part2.txt
```

Necronomicon therefore participates in the final binary-distribution stage as well as the earlier script stage.

The two roles can be summarized as:

```text
Normal Necronomicon
    → identifies changed scripts

Part2 Necronomicon
    → identifies changed packaged assets
```

---

# 🏁 Summary

Necronomicon is the **cryptographic change detector of TextTools**.

Its normal job is:

```text
DGRV3/
    vs
DGRV3_EN/

        ↓

SHA-512 every relevant file

        ↓

compare digests

        ↓

identify changed files

        ↓

different_hashes.txt
different_hash_folders.txt
sha_diff.txt
```

Its Part2 job is:

```text
Distribute/
    vs
*_normal.*

        ↓

SHA-512

        ↓

identify changed SPC/PB assets

        ↓

different_part2.txt
different_part2_short.txt
```

Its most important characteristics are:

* **Uses SHA-512**
* **Hashes raw file bytes**
* **Preserves newline/encoding differences in the hash calculation**
* **Supports `.txt` files in normal mode**
* **Supports `.pb` files when Switch configuration is enabled**
* **Supports `.spc` files in Part2**
* **Generates `<file>_sha` checksum files**
* **Compares translated files against `DGRV3_EN`**
* **Produces `different_hashes.txt` as an incremental-build manifest**
* **Produces `different_hash_folders.txt` for folder-level selection**
* **Produces `sha_diff.txt` for detailed mismatch diagnostics**
* **Provides `--all` to force all known folders into the processing set**
* **Provides `part2` for distribution/SPC comparison**
* **Produces `different_part2.txt` for WhiteSheet**
* **Uses `TextConfig.config` to determine Switch-specific hashing behavior**
* **Relies on a private/external hashing dependency**
* **Contains a disabled placeholder "slow" path that is not a genuine hash implementation**

The most important conceptual distinction is:

```text
MarkerStone
    = "Did the structure change?"

Necronomicon
    = "Did the bytes change?"
```

And the most important pipeline relationship is:

```text
Necronomicon
      ↓
different_hashes.txt
      ↓
HydraulicPress / Electrohammer
      ↓
incremental compilation
```

So although Necronomicon looks like a checksum utility on the surface, its real role inside TextTools is broader:

> **Necronomicon is the component that converts raw file differences into actionable build information.**

It determines which parts of the translation project have changed, records the evidence for those changes, and feeds that information forward into the tools that actually rebuild and distribute the modified game assets.
