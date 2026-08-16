# TEXTTOOLS - LOOSEFLOORBOARD
## Team DAIX, 2026  

---

## 📌 Overview

**LooseFloorboard** is a small C# helper program used by **HydraulicPress** to make translated script text compatible with platforms whose fonts or text systems cannot safely display accented or otherwise non-ASCII characters.

Its primary job is to recursively scan the current TextTools working directory for `.txt` files and convert characters outside the basic ASCII range into appropriate ASCII equivalents.

For example, accented Latin characters can be converted into their unaccented forms:

```text
é → e
è → e
à → a
ö → o
ç → c
````

The tool is therefore best understood as an **ASCII compatibility pass** that runs during HydraulicPress compilation.

It does **not** perform variable replacement, script compilation, SPC manipulation, or translation.

Instead, it modifies the textual source files directly so that later compilation stages receive text that is safe for platforms with limited character support.

---

# 🎯 Responsibilities

LooseFloorboard performs five main tasks:

### ✔ 1. Scan the working directory recursively

It searches the current working directory and every subdirectory for:

```text
*.txt
```

files.

---

### ✔ 2. Skip protected files

Certain files and directories are deliberately excluded:

```text
vars_bak.txt
base_spc/
```

This prevents LooseFloorboard from modifying the variable definition backup or files belonging to the original/base SPC resources.

---

### ✔ 3. Fold Unicode characters into ASCII

Each line is passed through:

```text
StringExtensions.FoldToASCII(...)
```

which converts supported non-ASCII characters into ASCII equivalents.

This is primarily intended for accented Latin characters.

---

### ✔ 4. Apply the special Z workaround

One specific file receives an additional transformation:

```text
A-MapListNameAscii.txt
```

For this file only:

```text
z → 2
```

is performed.

This exists because of a platform/font limitation involving the letter `z`.

---

### ✔ 5. Rewrite the original files

LooseFloorboard does not create separate output files.

Instead, for every processed file it:

1. reads the entire file,
2. converts every line,
3. deletes the original,
4. writes the converted content back to the same path.

Therefore the operation is **in-place and destructive**.

---

# 🧩 How LooseFloorboard Fits Into TextTools

LooseFloorboard is a **sub-stage of HydraulicPress**.

The relevant portion of the pipeline is:

```text
HydraulicPress
    │
    ├── Load configuration
    │
    ├── Load variables
    │
    ├── Determine files to process
    │
    ├── Check variables
    │
    ├── Replace variables
    │
    ├── LooseFloorboard
    │      │
    │      └── Convert unsupported characters
    │
    ├── Optional randomization
    │
    └── Optional word counting
```

HydraulicPress only invokes LooseFloorboard when:

```text
ReplaceBlacklistedChars = true
```

is enabled in `TextConfig.config`.

On Windows, HydraulicPress executes:

```text
LooseFloorboard.exe
```

On non-Windows platforms it executes:

```text
./LooseFloorboard
```

The program is therefore not normally launched as an independent stage of the main TextTools pipeline.

---

# 🔗 Relationship With HydraulicPress

HydraulicPress explicitly describes LooseFloorboard as the C# tool responsible for replacing accented characters on platforms that do not support them.

The invocation occurs **after variable replacement**:

```text
VariableUtils::ReplaceVariables(...)
        ↓
LooseFloorboard
        ↓
Randomizer / WordCounter
```

This ordering is important.

HydraulicPress first expands variables into their final text and then runs LooseFloorboard over the resulting `.txt` files.

Therefore, characters introduced by variable expansion can also be affected by LooseFloorboard.

---

# ⚙️ Invocation

LooseFloorboard does not require command-line arguments.

HydraulicPress simply launches it:

```text
Windows:
LooseFloorboard.exe

Other platforms:
./LooseFloorboard
```

The program determines everything it needs from:

```text
Directory.GetCurrentDirectory()
```

and the files beneath that directory.

---

# 📥 Inputs

LooseFloorboard has no formal command-line input format.

Its effective inputs are:

### Working directory

The directory from which the program is launched.

This is normally the root of the TextTools project/repository.

---

### `.txt` files

Every matching file found recursively under the current directory is a candidate for processing.

The search is equivalent to:

```csharp
Directory.EnumerateFiles(
    Directory.GetCurrentDirectory(),
    "*.txt",
    SearchOption.AllDirectories
)
```

---

### File contents

Each selected text file is read using:

```csharp
File.ReadAllLines(...)
```

The contents are then passed line-by-line to `FoldToASCII()`.

---

# 📤 Outputs

LooseFloorboard does not create a separate output directory.

Its output is the **same `.txt` files after modification**.

For example:

```text
Before:

Città molto bella.
È già pronto.
Perché?
```

may become:

```text
After:

Citta molto bella.
E gia pronto.
Perche?
```

The exact conversion depends on the Unicode character and the folding table implemented by `FoldToASCII()`.

---

# 🏗️ Internal Architecture

LooseFloorboard consists of two important pieces:

```text
Program.cs
    │
    └── discovers and rewrites files

ASCIIStringExtensions.cs
    │
    └── FoldToASCII()
            │
            └── performs character conversion
```

The architecture is intentionally simple.

`Program.cs` handles **which files are processed**.

`ASCIIStringExtensions.cs` handles **how individual strings are converted**.

---

# 🟦 Program.cs

The entry point is:

```csharp
static void Main(string[] args)
```

The `args` array is currently unused.

The program begins by displaying:

```text
Current folder: <working directory>
```

It then deliberately waits for five seconds.

---

# ⏱️ Five-Second Startup Delay

The program contains:

```csharp
Thread.Sleep(5000);
```

This means LooseFloorboard waits approximately:

```text
5 seconds
```

before modifying anything.

The source contains the comment:

```text
sleep for 5 seconds... Why though?
```

There is no functional dependency on this delay in the implementation.

It appears to be a historical/debugging artifact.

---

# 📂 File Discovery

LooseFloorboard uses:

```csharp
Directory.EnumerateFiles(
    Directory.GetCurrentDirectory(),
    "*.txt",
    SearchOption.AllDirectories
)
```

This means:

* only `.txt` files are considered,
* searching is recursive,
* files in nested directories are included,
* the search starts from the current working directory.

The matching is performed by the filesystem API's filename pattern rather than by explicitly checking the extension afterward.

---

# ⚠️ Scope Is Broader Than HydraulicPress

An important distinction is that HydraulicPress determines a specific list of script files for its own variable replacement.

LooseFloorboard does **not** reuse that list.

Instead, it performs its own independent recursive scan of:

```text
current working directory/
```

for:

```text
*.txt
```

Therefore LooseFloorboard can potentially process `.txt` files that HydraulicPress itself did not consider part of its normal compilation set.

The exclusions described below are the only explicit filtering performed by `Program.cs`.

---

# 🚫 Exclusion: `vars_bak`

LooseFloorboard checks the complete path:

```csharp
if(str.Contains("vars_bak"))
{
    continue;
}
```

Any matching path is skipped.

This is intended to prevent modification of:

```text
vars_bak.txt
```

because it is the backup variable-definition file used by HydraulicPress.

---

# 🚫 Exclusion: `base_spc`

LooseFloorboard also checks:

```csharp
if(str.Contains("base_spc"))
{
    continue;
}
```

Any `.txt` file whose complete path contains:

```text
base_spc
```

is ignored.

This prevents LooseFloorboard from modifying text belonging to the original/base SPC resources.

---

# ⚠️ Path Filtering Is Substring-Based

Neither exclusion is an exact filename or directory comparison.

The code checks:

```text
Contains("vars_bak")
Contains("base_spc")
```

against the complete path.

Therefore any path containing those strings is skipped, even if the matching text appears somewhere other than the intended filename/directory.

For example, a hypothetical path such as:

```text
some_base_spc_backup/script.txt
```

would also satisfy the `base_spc` test.

---

# 🧾 Filename Detection for the Z Workaround

For every candidate file, LooseFloorboard obtains:

```csharp
Path.GetFileName(file)
```

and stores it in:

```text
fn
```

This means the special Z replacement is based on the **filename only**, not the full directory path.

---

# 🟥 Special Case: `A-MapListNameAscii.txt`

LooseFloorboard creates a list:

```csharp
List<string> blacklisted_z
```

containing:

```text
A-MapListNameAscii.txt
```

The comment identifies this file as:

```text
Map names (contains some Zs)
```

When processing this file, the second argument to `FoldToASCII()` becomes:

```text
true
```

For all other files it is:

```text
false
```

---

# 🔤 The Special Z → 2 Conversion

The special argument is called:

```text
no_z
```

Inside `FoldToASCII()` the ASCII branch contains:

```text
if(c == 'z' && no_z)
{
    foldedString.Append("2");
}
```

Therefore, when processing:

```text
A-MapListNameAscii.txt
```

a lowercase:

```text
z
```

becomes:

```text
2
```

---

# ⚠️ Only Lowercase `z` Is Replaced

The implementation checks:

```text
c == 'z'
```

not:

```text
c == 'z' || c == 'Z'
```

Therefore:

```text
z → 2
```

but:

```text
Z → Z
```

This is an important implementation detail.

The special workaround is therefore **not a general case-insensitive Z replacement**.

---

# 🔤 `FoldToASCII()`

The central conversion routine is:

```csharp
StringExtensions.FoldToASCII(...)
```

Its purpose is to convert Unicode characters that have reasonable ASCII equivalents into those ASCII representations.

The implementation is derived from ASCII-folding logic associated with Lucene.NET and is documented in the source as derivative work.

The general principle is:

```text
Unicode character
        ↓
ASCII equivalent
```

---

# 🧠 Basic ASCII Fast Path

The first major branch in `FoldToASCII()` checks:

```text
c < '\u0080'
```

Characters below hexadecimal `0x80` are already within basic ASCII.

These characters are generally copied directly.

Examples include:

```text
A-Z
a-z
0-9
spaces
punctuation
ASCII control-safe text
```

---

# 🔄 Non-ASCII Processing

Characters at or above:

```text
\u0080
```

are processed through a large switch statement.

This allows the implementation to explicitly map many Unicode characters to ASCII equivalents.

The source documentation describes coverage across multiple Unicode blocks, including:

```text
C1 Controls / Latin-1 Supplement
Latin Extended-A
Latin Extended-B
Latin Extended Additional
Latin Extended-C
Latin Extended-D
IPA Extensions
Phonetic Extensions
General Punctuation
Alphabetic Presentation Forms
Halfwidth and Fullwidth Forms
```

The implementation does not attempt to represent every Unicode character.

Instead, it converts characters for which a practical ASCII equivalent exists.

---

# 📝 Typical Accent Folding

The most important use case is accented Latin text.

Conceptually:

```text
á → a
à → a
â → a
ä → a

é → e
è → e
ê → e
ë → e

í → i
ì → i
î → i
ï → i

ó → o
ò → o
ô → o
ö → o

ú → u
ù → u
û → u
ü → u
```

The exact supported mappings come from the switch table in `ASCIIStringExtensions.cs`.

---

# 🧩 Apostrophe Handling

The implementation contains additional state:

```text
is_accent
is_vowel
is_macron
is_fullwidth_jp
```

These flags allow the routine to preserve certain accent-related information when converting sequences.

When an ASCII character is encountered after an accented vowel, the routine may append:

```text
'
```

under specific conditions.

The relevant conditions include:

```text
!is_macron
!is_fullwidth_jp
is_accent
!use_weird_characters_from_font
```

The current source sets:

```text
use_weird_characters_from_font = false
```

so this compatibility behavior is active where the surrounding character state requires it.

---

# 🧠 Stateful Folding

`FoldToASCII()` is therefore not merely:

```text
character → replacement
```

for every character.

It also tracks the state of the preceding non-ASCII character.

This is why variables such as:

```text
last
is_accent
is_vowel
is_macron
is_fullwidth_jp
```

exist.

This allows the implementation to make decisions based on the relationship between adjacent characters.

---

# 🔤 `last` Character Tracking

The function keeps:

```csharp
char last = '\0';
```

and uses it when returning to ASCII text after processing a non-ASCII character.

This is part of the logic that determines whether an additional apostrophe should be emitted after certain accented characters.

---

# 🌏 Fullwidth and Japanese Handling

The folding implementation contains explicit support for Unicode blocks beyond ordinary accented Latin characters.

Among the supported categories are:

```text
Halfwidth and Fullwidth Forms
```

and other Unicode ranges.

However, the source currently defines:

```text
no_fullwidth = false
```

and:

```text
use_weird_characters_from_font = false
```

These constants therefore affect which alternate representations are selected.

---

# 🧱 Unsupported Characters

Not every Unicode character necessarily has an ASCII replacement.

The folding table only converts characters for which the implementation provides a mapping.

Characters without a suitable mapping may remain unchanged or follow the fallback behavior implemented by the folding routine.

Therefore LooseFloorboard should not be interpreted as a universal:

```text
Unicode → ASCII
```

encoder.

It is specifically an **ASCII folding compatibility filter**.

---

# 📖 File Reading

Each selected file is loaded using:

```csharp
File.ReadAllLines(str)
```

This reads the entire file into memory.

The program then constructs:

```csharp
List<string> outstr
```

containing the transformed lines.

---

# 📝 Line-by-Line Processing

For every input line:

```csharp
foreach (string line in lines)
```

LooseFloorboard calls:

```csharp
StringExtensions.FoldToASCII(
    line,
    blacklisted_z.Contains(fn)
)
```

The resulting string is appended to:

```text
outstr
```

No line is selectively omitted by the conversion routine.

---

# 🔄 In-Place Rewrite

Once every line has been processed:

```csharp
File.Delete(str);
```

deletes the original file.

Then:

```csharp
File.WriteAllLines(str, outstr);
```

recreates the file at the exact same path.

The effective transformation is:

```text
original.txt
    ↓
ReadAllLines()
    ↓
Fold every line
    ↓
Delete original
    ↓
Write converted file
```

---

# ⚠️ Destructive File Handling

This delete-and-recreate behavior is an important developer consideration.

LooseFloorboard does **not**:

```text
create a backup
```

before deleting the original file.

It also does not write to a temporary path and atomically replace the original.

Therefore an interruption or filesystem error between:

```text
File.Delete()
```

and:

```text
File.WriteAllLines()
```

could leave the original file unavailable.

---

# ⚠️ Encoding Considerations

The program relies on:

```csharp
File.ReadAllLines()
File.WriteAllLines()
```

without explicitly specifying an encoding.

Consequently, encoding behavior is determined by the .NET implementation/runtime being used.

This is especially relevant because LooseFloorboard exists specifically to deal with Unicode compatibility.

Developers should verify the resulting file encoding when porting the tool to a different .NET runtime or platform.

---

# ⚠️ Line Structure

LooseFloorboard processes complete lines rather than performing arbitrary byte-level replacement.

Therefore its transformations operate on decoded text.

This is appropriate for accent folding but means the tool is fundamentally an **encoding-aware text processor**, not a binary patcher.

---

# 📁 Files That Can Be Modified

In principle, LooseFloorboard can modify:

```text
*.txt
```

anywhere below the current working directory.

For example:

```text
DGRV3/
├── chapter1/
│   ├── scene_01.txt
│   └── scene_02.txt
├── chapter2/
│   └── scene_05.txt
├── gallery/
│   └── names.txt
└── ...
```

provided those files do not fall under the exclusion rules.

---

# 🚫 Files That Are Intentionally Skipped

The main exclusions are:

```text
vars_bak.txt
```

and:

```text
anything whose path contains base_spc
```

The tool does not independently recognize:

```text
README
LICENSE
_sha
_lines
.pb
.stx
```

as exclusions.

If such a file were a `.txt` file and did not match the two explicit skip conditions, it could be processed.

---

# ⚠️ Difference From HydraulicPress File Filtering

HydraulicPress's `FileUtils::IsFileGood()` has a much larger set of filters.

It excludes things such as:

```text
README
LICENSE
.git
_sha
_lines
Baked
.pb
vars_bak
```

LooseFloorboard does not use that filtering function.

It has its own simpler search logic.

Therefore developers should not assume that:

```text
"HydraulicPress would ignore this file"
```

means:

```text
"LooseFloorboard will ignore this file."
```

---

# 🔗 Interaction With Variable Replacement

The intended order is:

```text
Translation
    ↓
HydraulicPress
    ↓
Variable replacement
    ↓
LooseFloorboard
    ↓
ASCII-compatible text
```

This means variable values are expanded before accent folding occurs.

For example, if a variable expands to:

```text
Perché
```

LooseFloorboard can subsequently turn that into:

```text
Perche
```

when the active platform requires ASCII-compatible text.

---

# 🔗 Interaction With Randomizer

HydraulicPress runs LooseFloorboard before the optional randomizer:

```text
Variable replacement
        ↓
LooseFloorboard
        ↓
Randomizer
```

Therefore randomized text is generated after accent folding.

If `DoRandomize` is enabled, the randomizer's output does not depend on LooseFloorboard for ordinary generated ASCII strings.

---

# 🔗 Interaction With WordCounter

HydraulicPress also invokes word counting after LooseFloorboard.

The general order is:

```text
Variable replacement
        ↓
LooseFloorboard
        ↓
Randomization (optional)
        ↓
Word counting (optional)
```

Therefore, when enabled, the word counter sees the post-folding text.

---

# ⚙️ Configuration

LooseFloorboard itself does not parse `TextConfig.config`.

The configuration decision is made by HydraulicPress.

The relevant setting is:

```text
ReplaceBlacklistedChars
```

When it is:

```text
false
```

HydraulicPress does not launch LooseFloorboard.

When it is:

```text
true
```

HydraulicPress launches it automatically.

---

# 🎮 Platform Purpose

The source describes LooseFloorboard as a workaround for platforms/font systems that cannot properly display accented characters.

The intended use case is therefore primarily:

```text
platform has limited character/font support
        ↓
ReplaceBlacklistedChars = true
        ↓
HydraulicPress invokes LooseFloorboard
        ↓
Accents folded to ASCII
        ↓
Text becomes platform-compatible
```

The exact platform decision is made by HydraulicPress configuration rather than by LooseFloorboard itself.

---

# 🧪 Example Transformation

Suppose a script contains:

```text
{
Perché sei qui?
È già troppo tardi.
L'università è chiusa.
}
```

After ASCII folding, it may become:

```text
{
Perche sei qui?
E gia troppo tardi.
L'universita e chiusa.
}
```

The purpose is not to improve the translation.

The purpose is to make the text representable using a restricted character set.

---

# 🧪 Special Map-Name Example

For:

```text
A-MapListNameAscii.txt
```

consider:

```text
Zebra
zebra
```

The normal ASCII text is preserved:

```text
Zebra
```

but lowercase `z` is specially replaced:

```text
zebra
```

→

```text
2ebra
```

This is because `no_z` is enabled for that filename.

---

# 🧪 Ordinary File Example

For:

```text
chapter3/scene_12.txt
```

the `no_z` flag is:

```text
false
```

so:

```text
zebra
```

remains:

```text
zebra
```

while accented characters are still subject to normal ASCII folding.

---

# 🛑 Failure Considerations

The implementation contains no dedicated exception handling around:

```text
File.ReadAllLines()
File.Delete()
File.WriteAllLines()
```

Therefore filesystem errors may propagate as .NET exceptions rather than being converted into structured TextTools error messages.

Potential failure causes include:

* file permissions,
* read failures,
* write failures,
* files being locked by another process,
* invalid/unexpected encoding,
* deletion failures,
* insufficient storage.

---

# ⚠️ No Success/Failure Contract to HydraulicPress

HydraulicPress launches LooseFloorboard through:

```text
Common::executeBatch(...)
```

and then immediately logs:

```text
Accented characters replaced!
```

The source does not show a detailed validation of LooseFloorboard's resulting files before continuing.

Therefore the orchestration layer should not be interpreted as performing a deep verification of every transformation.

LooseFloorboard is treated as a subordinate preprocessing utility.

---

# 🧠 Important Developer Quirks

## 1. Five-second delay

The program intentionally sleeps for five seconds at startup.

This appears unnecessary for the conversion logic and is likely historical.

---

## 2. No command-line options

`args` is accepted but unused.

There is no documented CLI mode for selecting directories, files, or conversion modes.

---

## 3. Current working directory matters

The tool operates relative to:

```text
Directory.GetCurrentDirectory()
```

Launching it from the wrong directory can therefore cause it to scan the wrong files.

---

## 4. Recursive scanning is broad

Every matching `.txt` file beneath the working directory is a candidate.

---

## 5. Filtering uses substring matching

The exclusions:

```text
vars_bak
base_spc
```

are detected with `Contains()`.

---

## 6. Only one filename gets the Z workaround

The special list currently contains:

```text
A-MapListNameAscii.txt
```

Adding another filename requires modifying the source.

---

## 7. Lowercase `z` only

The special replacement affects:

```text
z
```

not:

```text
Z
```

---

## 8. Files are rewritten in place

No separate output directory is produced.

---

## 9. Original files are deleted first

The rewrite sequence is:

```text
delete → recreate
```

rather than:

```text
write temporary → atomic replace
```

---

## 10. No backup is created

Running LooseFloorboard is therefore not reversible through the tool itself.

---

# 🧩 Source Components

LooseFloorboard consists primarily of:

```text
LooseFloorboard/
├── Program.cs
└── ASCIIStringExtensions.cs
```

### `Program.cs`

Responsible for:

* startup,
* working-directory detection,
* file enumeration,
* exclusion rules,
* special filename detection,
* reading files,
* invoking `FoldToASCII()`,
* deleting originals,
* writing converted files.

### `ASCIIStringExtensions.cs`

Responsible for:

* Unicode/ASCII conversion,
* accent folding,
* Unicode block handling,
* special character mappings,
* stateful accent handling,
* the optional `z → 2` conversion.

---

# 🔄 Complete Workflow

```text
HydraulicPress
      │
      │ ReplaceBlacklistedChars == true
      ▼
Launch LooseFloorboard
      │
      ▼
Print current directory
      │
      ▼
Wait 5 seconds
      │
      ▼
Create special filename list
      │
      ▼
Recursively enumerate *.txt
      │
      ├── path contains "vars_bak"?
      │       └── YES → skip
      │
      ├── path contains "base_spc"?
      │       └── YES → skip
      │
      ▼
Read entire file
      │
      ▼
Process every line
      │
      ▼
FoldToASCII()
      │
      ├── ASCII character
      │      └── preserve
      │
      ├── accented Unicode
      │      └── fold to ASCII equivalent
      │
      └── A-MapListNameAscii.txt
             └── lowercase z → 2
      │
      ▼
Delete original file
      │
      ▼
Write converted lines
      │
      ▼
Next .txt file
      │
      ▼
Finished
      │
      ▼
Return to HydraulicPress
```

---

# 📊 Pipeline Position

Within the broader TextTools system, LooseFloorboard belongs specifically inside HydraulicPress:

```text
MarkerStone
    ↓
Necronomicon
    ↓
Bugvac
    ↓
HydraulicPress
    │
    ├── VariableChecker
    ├── VariableReplacer
    ├── LooseFloorboard
    ├── Randomizer
    └── WordCounter
    ↓
ElectroHammer
    ↓
Crossbow
    ↓
WhiteSheet
```

LooseFloorboard is therefore **not a replacement for HydraulicPress**.

It is a helper invoked by HydraulicPress under a particular configuration.

---

# 🏁 Summary

LooseFloorboard is the **ASCII compatibility layer** of TextTools.

Its purpose is straightforward:

> Take translated `.txt` script files containing characters that may not be supported by the target platform and fold those characters into ASCII-compatible representations.

Its core behavior is:

```text
Find *.txt
    ↓
Skip vars_bak
    ↓
Skip base_spc
    ↓
Read lines
    ↓
Fold Unicode → ASCII
    ↓
Special-case A-MapListNameAscii.txt
    ↓
Delete original
    ↓
Write converted file
```

It is called automatically by HydraulicPress when:

```text
ReplaceBlacklistedChars = true
```

and is executed **after variable replacement** but before HydraulicPress's optional randomization and word-counting stages.

The most important characteristics to remember are:

* **C# utility**
* **Called by HydraulicPress**
* **No meaningful CLI arguments**
* **Recursively scans `.txt` files**
* **Converts supported Unicode characters to ASCII**
* **Skips paths containing `vars_bak`**
* **Skips paths containing `base_spc`**
* **Special-cases `A-MapListNameAscii.txt`**
* **Changes lowercase `z` to `2` in that file**
* **Rewrites files in place**
* **Deletes originals before recreating them**
* **Does not create backups**
* **Exists primarily for platform/font compatibility**

In short:

```text
LooseFloorboard
= "Make the text safe for platforms that cannot handle the original characters."
```

It is a small but important preprocessing step between **HydraulicPress's variable expansion** and the later stages that turn the processed text into compiled game resources.

```

The source confirms that **HydraulicPress directly launches LooseFloorboard when `ReplaceBlacklistedChars` is enabled**, immediately after variable replacement.

The C# implementation confirms the recursive `.txt` scan, `vars_bak`/`base_spc` exclusions, five-second delay, in-place rewriting, and special `A-MapListNameAscii.txt` handling.

The ASCII folding implementation is contained in `ASCIIStringExtensions.cs`; it provides the Unicode-to-ASCII conversion and the special lowercase `z` handling.

```