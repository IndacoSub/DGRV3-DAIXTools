# TEXTTOOLS - HYDRAULICPRESS
## Team DAIX, 2026  

---

## 📌 Overview

**HydraulicPress** is the central compilation and text-processing component of **TextTools**.

Its primary responsibility is to take translated Danganronpa V3 script files, load the project's variable definitions, determine the target platform, and transform the scripts into platform-ready compiled text.

HydraulicPress itself is primarily an **orchestrator**. Rather than implementing every transformation directly, it coordinates several specialized modules:

- **FileUtils** — locates and parses the variable definition file and determines which scripts should be processed.
- **VariableChecker** — detects `VAR_*` tokens that are missing from the variable list.
- **VariableReplacer** — performs variable substitution, `MAKE_*` expansion, signal removal, and platform filtering.
- **Platform** — handles `<PLATFORM_...>` markers.
- **Randomizer** — optionally destroys/randomizes script text for testing and stress-testing.
- **WordCounter** — analyzes word frequency across the processed scripts.
- **StringUtils** — supplies the common string-processing operations used throughout these modules.

HydraulicPress therefore represents the **compilation stage** between a translated script repository and the files that can subsequently be consumed by the rest of the TextTools/game-building pipeline.

The source identifies this version as:

`HydraulicPress v1.2`

The code also notes that most of the implementation dates from **2020–2022**, with later changes including the migration to `TextConfig.config`.

---

## 🎯 Responsibilities

HydraulicPress performs the following high-level operations:

1. Load `TextConfig.config`.
2. Determine the target platform.
3. Locate `vars.txt` or fall back to `vars_bak.txt`.
4. Parse the variable definitions.
5. Determine which script files need processing.
6. Optionally check for missing variables.
7. Replace variables in the selected scripts.
8. Optionally generate baked versions of the scripts.
9. Optionally replace unsupported accented characters through **LooseFloorboard**.
10. Optionally randomize script contents.
11. Optionally generate a word-frequency report.
12. Wait for the user before terminating.

The order is important because later stages operate on the output produced by earlier stages.

---

# 🧩 Position in the TextTools Pipeline

HydraulicPress is downstream of the tools that prepare and validate the translation repository.

A typical high-level pipeline is:

1. **Ropeway** — obtain/update the text repository.
2. **CrammedPiranhas** — extract the relevant SPC/STX assets.
3. **MarkerStone** — compare English and translated line counts.
4. **Necronomicon** — determine which files differ by SHA-512.
5. **Bugvac** — perform character-name transformations.
6. **HydraulicPress** — compile variables, platform-specific content, and other text transformations.
7. Later TextTools components can consume the compiled output for injection/distribution.

HydraulicPress therefore sits at the point where the translation is converted from a variable-rich editing representation into a more final game-oriented representation.

---

# 📁 Expected Working Directory

HydraulicPress operates relative to the **current working directory**.

It expects the translated repository to be available as:

```text
DGRV3/
````

and the configuration file as:

```text
TextConfig.config
```

The source constructs these paths from the current working directory:

```text
<current directory>/DGRV3/
<current directory>/TextConfig.config
```

The source also expects the variable definition file to be available either as:

```text
vars.txt
```

or, as a fallback:

```text
DGRV3/vars_bak.txt
```

The program does not take an explicit repository path from the command line.

---

# ⚙️ Startup Sequence

HydraulicPress begins by setting the process locale:

```cpp
setlocale(LC_ALL, "it_IT.UTF-8");
```

This is intended to support Italian UTF-8 handling, particularly on Windows.

It then determines:

* the current working directory,
* the `DGRV3` repository path,
* the `TextConfig.config` path.

It logs its version:

```text
HydraulicPress v1.2
```

and announces that configuration has migrated to:

```text
TextConfig.config
```

---

# 🖥️ Command-Line Arguments

HydraulicPress supports two platform-related command-line overrides.

## `--switch`

Forces Switch configuration.

Example:

```text
HydraulicPress.exe --switch
```

The argument is detected using substring matching rather than exact argument equality. Therefore, the implementation is effectively looking for an argument containing:

```text
--switch
```

---

## `--pc`

Forces PC configuration.

Example:

```text
HydraulicPress.exe --pc
```

As with `--switch`, detection uses substring matching.

---

## Platform Override Precedence

The platform-selection logic is:

1. Explicit Switch selection can select Switch unless PC is explicitly forced.
2. Otherwise, `UseSwitchConfiguration` can select Switch.
3. Otherwise, Xbox configuration can select Xbox unless PC is explicitly forced.
4. Otherwise, PC is selected.

The relevant internal state is updated in two places:

```text
Configuration::CurrentPlatform
Distribution::Platform
```

The string used by the platform tag parser is:

```text
PC
SWITCH
XBOX
```

---

# 🎮 Platform-Specific Compilation

HydraulicPress uses the `Distribution::CheckPlatforms()` function from `Platform.h`.

Script lines may contain markers such as:

```text
<PLATFORM_PC>
<PLATFORM_SWITCH>
<PLATFORM_XBOX>
```

A platform-specific line is retained only when its tag matches the currently selected platform.

For example:

```text
<PLATFORM_PC>Hello
```

when compiling for PC becomes:

```text
Hello
```

When compiling for Switch, the same line becomes:

```text
SKIPTHISLINE
```

and is omitted from the normal compiled output.

---

## Multiple Platform Tags

The platform parser also attempts to handle multiple tags:

```text
<PLATFORM_PC><PLATFORM_SWITCH>Text
```

The implementation removes platform markers while retaining the content for the selected platform.

This is intended to support lines that are valid for multiple builds.

---

## Platform Logging

Normal platform processing can emit diagnostic logging showing:

* the original line,
* the selected platform,
* the resulting line.

The `forbaked` parameter controls whether this diagnostic logging is emitted.

Baked processing uses:

```cpp
CheckPlatforms(..., true)
```

so it suppresses this platform-specific diagnostic logging.

---

# 📦 Variable Input

HydraulicPress needs a list of variable definitions before it can compile scripts.

The lookup order is:

```text
1. vars.txt
2. DGRV3/vars_bak.txt
```

`vars.txt` is attempted first from the current working directory.

If it cannot be opened, HydraulicPress logs:

```text
WARNING: Could not find vars.txt!
```

and falls back to:

```text
DGRV3/vars_bak.txt
```

If the backup also cannot be read, compilation fails.

---

# 📄 Variable File Format

The variable parser primarily expects definitions in the form:

```text
NAME, VALUE
```

For example:

```text
VAR_HELLO, Hello
```

The first comma separates the variable name from its value.

The resulting internal representation is effectively:

```text
(name, value)
```

represented by the project's `VarEntry` type.

---

# 📝 Variable Comments

Variable comments can be introduced using `|`.

For example:

```text
VAR_HELLO, Hello | translator note
```

The comment portion is stripped before parsing the variable.

Lines beginning with `|` are also ignored.

Commented-out lines beginning with `/` are ignored.

Empty lines are ignored.

---

# 🔤 Special Variable Definitions

Not every variable definition requires a comma.

If a line contains `<CLT` but has no comma, it is accepted as a variable with an empty value.

For example:

```text
<CLT_SOMETHING>
```

is interpreted approximately as:

```text
("<CLT_SOMETHING>", "")
```

Lines without a comma that do not contain `<CLT` are ignored with a warning.

---

# `SIGNAL_*` Handling During Parsing

The parser contains special handling for entries containing:

```text
SIGNAL
```

In particular, `<SIGNAL_OE>` is treated specially.

`SIGNAL_OE` represents the project's "otherwise empty" marker and does not behave like an ordinary replacement value.

---

# ❄️ IceConvention (DEPRECATED) Parsing

Variable values can contain a colon.

The interpretation depends on:

```text
UseIceConvention
```

from `TextConfig.config`.

When IceConvention (DEPRECATED) is enabled, the portion after the comma and before the colon is treated as the variable value.

Conceptually:

```text
NAME, VALUE : OTHER_DATA
```

becomes:

```text
NAME → VALUE
```

When IceConvention (DEPRECATED) is disabled, the value is taken from the portion after the colon.

This is a parsing convention rather than a generic `NAME, VALUE` rule, so changing `UseIceConvention` (DEPRECATED) can materially alter the variables loaded by HydraulicPress.

---

# 🧹 Variable Input Normalization

After parsing, HydraulicPress removes carriage-return and newline characters from both variable names and values.

This prevents hidden CR/LF characters from becoming part of a replacement key or replacement value.

---

# 📂 Selecting Script Files

HydraulicPress builds a list called:

```text
fts
```

which means, effectively:

```text
files to search
```

These are the script files passed to:

* `VariableChecker`
* `VariableReplacer`
* `Randomizer`
* `WordCounter`

The selection behavior is controlled by:

```text
CheckAllFiles
```

---

## `CheckAllFiles = true`

HydraulicPress recursively scans:

```text
DGRV3/
```

and considers every file that passes `FileUtils::IsFileGood()`.

---

## `CheckAllFiles = false`

HydraulicPress instead reads:

```text
different_hashes.txt
```

This file is expected to have been generated by **Necronomicon**.

Each line is interpreted as a relative path.

HydraulicPress converts each entry into a path relative to the current working directory.

This means HydraulicPress can limit compilation to files whose contents were identified as different instead of scanning the entire repository.

---

# 🚫 Files Excluded from Processing

`FileUtils::IsFileGood()` rejects files that do not contain `.txt`.

It also rejects files containing any of the following substrings:

```text
_sha
_lines
README
LICENSE
.git
Baked
.pb
vars_bak
```

Empty paths are also rejected.

Therefore, the normal processing set is intended to consist of actual script `.txt` files rather than metadata, generated files, backups, or binary text resources.

---

# 🔎 Necronomicon Integration

When:

```text
CheckAllFiles = false
```

HydraulicPress depends on:

```text
different_hashes.txt
```

This creates a direct relationship with **Necronomicon**.

The intended workflow is:

```text
Necronomicon
      ↓
different_hashes.txt
      ↓
HydraulicPress
      ↓
only changed script files processed
```

This reduces unnecessary processing when only a subset of the translation has changed.

If the selected file list is empty, HydraulicPress exits instead of continuing.

---

# 🧪 Variable Checking

If:

```text
DoCheckVariables
```

is enabled, HydraulicPress invokes:

```cpp
VariableChecker::CheckVariables(variables, fts);
```

The purpose is to find `VAR_*` tokens appearing in scripts that are not represented in the loaded variable list.

---

## VariableChecker Processing

Each selected script is read line-by-line.

Lines without:

```text
VAR_
```

are ignored.

For candidate lines, platform filtering is applied first.

Lines belonging to another platform are therefore not treated as missing variables.

The checker then strips several characters and escape sequences to make `VAR_*` extraction easier.

It can detect:

* a single variable in a line,
* multiple variables in a line,
* duplicate occurrences of the same variable on one line.

Special-looking tokens containing:

```text
CLT
SIGNAL
MAKE_
PLATFORM
PAD
DIG
```

are excluded from the missing-variable report.

---

# 📄 Variable Checker Outputs

When variable checking is enabled, two reports are produced.

### `variablechecker.txt`

The detailed report contains:

* each missing variable,
* every location where it appears,
* source line numbers,
* shortened filenames,
* the original source line when useful.

Example structure:

```text
"VAR_SOMETHING" NOT FOUND in the list! It can be found here:
            Line 123 of chapter3/scene_12.txt
```

Entries are separated by a historical separator consisting of 23 repetitions of:

```text
----------
```

---

### `variablechecker_short.txt`

Contains only the missing variable names, one per line.

This is useful when a compact list is preferable to the full diagnostic report.

---

# 🧠 RAM-Based Variable Checking

HydraulicPress does not blindly run the variable checker.

It measures the system's total physical RAM through `WordCounter`'s platform-specific RAM helper.

Variable checking requires approximately:

```text
7 GB
```

according to the source's own diagnostic threshold.

If the detected amount is below `7.0 GB`, the checker is skipped and a warning is logged.

Importantly, this is a **RAM threshold**, not a precise memory-usage measurement of the checker itself.

---

# 🔧 Variable Replacement

Variable replacement is the primary compilation operation.

HydraulicPress always invokes:

```cpp
VariableUtils::ReplaceVariables(fts, variables, true);
```

The final `true` means that baked output generation is enabled.

Variable replacement operates one file at a time and reads each script line-by-line.

---

# `VAR_*` Replacement

For each loaded variable definition, HydraulicPress searches for the variable name in the current line and replaces occurrences with the associated value.

For example:

```text
VAR_HELLO
```

with:

```text
VAR_HELLO, Hello
```

becomes:

```text
Hello
```

Replacement is not limited to a single occurrence.

---

# 🔁 Recursive Replacement

HydraulicPress supports recursive variable replacement.

This is important when the value of one variable expands into another variable.

Conceptually:

```text
VAR_A
   ↓
VAR_B
   ↓
Final text
```

The replacement loop continues while unresolved `VAR_*` content remains in a state that the implementation considers recursively replaceable.

A hard limit is present to prevent an infinite replacement loop.

The code stops after more than:

```text
1000
```

unchanged recursive iterations.

This protects against pathological or cyclic variable definitions.

---

# 🧩 `MAKE_*` Variables

HydraulicPress contains specialized handling for variables whose names contain:

```text
MAKE_
```

These may accept an argument.

Example:

```text
MAKE_HELLO(Mario)
```

A corresponding variable definition can use the placeholder:

```text
MY_ARG
```

The implementation extracts the argument between the first `(` following `MAKE_` and the corresponding `)`.

It then substitutes that argument into both:

* the variable name,
* the variable value.

Conceptually:

```text
MAKE_HELLO(MY_ARG)
```

with a value such as:

```text
Hello, MY_ARG!
```

and input:

```text
MAKE_HELLO(Mario)
```

produces a replacement equivalent to:

```text
Hello, Mario!
```

---

## `MAKE_*` Limitations

The implementation explicitly assumes a single argument.

The relevant parsing logic finds the first `)` and does not implement a general multi-argument parser.

Therefore, complex nested or multi-argument `MAKE_*` expressions should not be assumed to work.

---

# 🧼 Empty-Line Replacement

If:

```text
DoReplaceEmpty
```

is enabled, empty input lines are converted into unique placeholders.

The format is:

```text
EMPTY_<filename>L<line>
```

For example:

```text
EMPTY_scene_12.txtL45
```

This gives an otherwise empty line a deterministic identity.

However, the Randomizer explicitly refuses to run when `DoReplaceEmpty` is enabled.

Therefore:

```text
DoRandomize = true
DoReplaceEmpty = true
```

is an incompatible configuration.

---

# 🚦 SIGNAL Processing

HydraulicPress handles `SIGNAL_*` markers separately from ordinary variables.

Before normal processing, `<SIGNAL_OE>` is removed.

If:

```text
DoRemoveSignals
```

is enabled, `VariableUtils::RemoveSignal()` is also used.

---

## `SIGNAL_OE`

`SIGNAL_OE` is the special non-terminal signal.

Its behavior is:

```text
<SIGNAL_OE>
```

is removed while preserving the text that follows it.

---

## Other `SIGNAL_*` Tags

Other signal types are treated as terminal markers.

For example:

```text
Text<SIGNAL_NDR>More text
```

is effectively reduced to:

```text
Text
```

The first non-`OE` signal causes the signal and everything following it to be removed.

If a malformed signal has no terminating `>` character, the implementation conservatively removes everything from that signal onward.

---

# 🏗️ Normal Output vs Baked Output

HydraulicPress generates two conceptual forms of the compiled text.

## Normal compiled output

The normal output retains the project's translation-oriented markers according to the configured processing rules.

Platform filtering is applied through:

```text
Distribution::CheckPlatforms()
```

and lines belonging to other platforms are omitted.

---

## Baked output

Baked output is generated because HydraulicPress calls:

```cpp
ReplaceVariables(..., true);
```

Baked files are placed underneath:

```text
Baked/
```

with the containing script folder preserved.

For example, a source file conceptually like:

```text
DGRV3/chapter3/scene_12.txt
```

is written to something equivalent to:

```text
Baked/chapter3/scene_12.txt
```

---

# 🧱 Baked Platform Filtering

The setting:

```text
NoPlatformInBaked
```

controls whether platform tags are removed from baked output.

When enabled, HydraulicPress applies platform processing to the baked text.

If a baked line resolves to:

```text
SKIPTHISLINE
```

that line is omitted.

---

# 🏷️ Baked CLT Removal

The setting:

```text
NoCLTInBaked
```

controls removal of CLT definitions from baked output.

When enabled, HydraulicPress iterates over the loaded variable list and removes entries whose names contain:

```text
<CLT
```

This allows the baked representation to have its CLT markup stripped.

---

# 🗺️ Variable Replacement Map

Every successful replacement is recorded internally as a `SuperVarEntry`.

The recorded information includes:

1. Filename and line number.
2. Original line content.
3. Variable name.
4. Variable value.
5. Resulting line.

The report is written to:

```text
var_replace_map.txt
```

The output uses a format conceptually similar to:

```text
[scene_12.txt|45] "original text" --> ("VAR_NAME" --> "replacement value") --> "final text"
```

This makes the file useful for auditing exactly how a translation line changed during compilation.

---

# 🔨 File Overwriting Behavior

Variable replacement does not write a separate normal compiled directory.

Instead, it rewrites the selected source `.txt` file.

The original file is opened for output and replaced with the generated stringstream contents.

Therefore, HydraulicPress should be treated as a **destructive compilation step** for the normal script files.

The baked copy is separate, but the normal processed file itself is overwritten.

---

# 🧊 Accent / Blacklisted Character Replacement

HydraulicPress can invoke an external component named:

```text
LooseFloorboard
```

when:

```text
ReplaceBlacklistedChars
```

is enabled.

On Windows:

```text
LooseFloorboard.exe
```

is executed.

On non-Windows systems:

```text
./LooseFloorboard
```

is executed.

The purpose is to replace accented characters that are unsupported by the selected platform.

HydraulicPress itself does not implement the accent-replacement rules; it delegates them to LooseFloorboard.

---

# 🎲 Text Randomizer

The Randomizer is an optional testing/stress-testing stage controlled by:

```text
DoRandomize
```

It operates on the same `fts` file list used by the rest of HydraulicPress.

There are two fundamentally different behaviors controlled by:

```text
SmartRandomization
```

---

# 🔀 Smart Randomization

When:

```text
SmartRandomization = true
```

all eligible script lines are collected into a single global pool.

The pool is then shuffled using:

```text
std::mt19937
```

seeded from `std::random_device`.

The shuffled lines are distributed back into the original files.

This means text can move:

```text
file A → file B
file B → file C
file C → file A
```

while preserving the number of eligible text lines assigned to each file.

---

# 🎰 Non-Smart Randomization

When:

```text
SmartRandomization = false
```

the original text is not globally shuffled.

Instead, every eligible line is replaced with a newly generated random uppercase alphanumeric string.

The generated string has a length between:

```text
5 and 30 characters
```

and the first character is forced into the uppercase-letter range.

The purpose is primarily stress-testing rather than producing meaningful text.

---

# 🧱 Bracket Preservation

The Randomizer recognizes files that contain structural brace lines:

```text
{
}
```

It preserves these braces instead of treating them as ordinary text.

The intended structure:

```text
{
text
text
}
```

therefore remains structurally bracketed after randomization.

---

# 🏷️ CLT WEAK / AGREE Preservation

The Randomizer has special handling for:

```text
<CLT=cltWEAK>
<CLT=cltAGREE>
```

These markers are recorded before text is shuffled.

During reconstruction, the markers are removed from the randomized line and then reapplied to the corresponding output line.

The marker is inserted at a random word.

There is also a 1-in-3 chance that the marker spans multiple words.

The closing marker is:

```text
<CLT=cltNORMAL>
```

This means a randomized line may become conceptually:

```text
<CLT=cltWEAK>Randomized text<CLT=cltNORMAL>
```

or the equivalent using `cltAGREE`.

---

# 🚫 DIG and PAD Handling

During Randomizer output generation, lines containing:

```text
DIG
```

are replaced with:

```text
DIG_SKIPPED
```

Lines containing:

```text
PAD
```

are replaced with:

```text
PAD_SKIPPED
```

Empty or space-only lines are replaced with:

```text
EMPTY_LINE
```

This is deliberate test-output behavior rather than normal translation compilation.

---

# 📊 Randomizer Report

The Randomizer produces:

```text
randomizer_report.txt
```

The report records where randomized text originated and where it ended up.

This is particularly useful when `SmartRandomization` is enabled, because text can be moved between files.

The report provides a mapping between original occurrences and randomized destinations.

---

# ⚠️ Randomizer Limitations

HydraulicPress refuses to perform randomization when:

```text
DoReplaceEmpty = true
```

The source explicitly describes these settings as incompatible.

Randomization also destroys the semantic usefulness of the processed script, so it should be considered a testing/debugging feature rather than a normal release compilation step.

---

# 📚 Word Counter

The optional WordCounter stage is controlled by:

```text
DoCountWords
```

When enabled, HydraulicPress calls:

```cpp
CountUtils::CountWords(fts);
```

The result is written to:

```text
words_counted.txt
```

---

# 🔢 Word Counting Process

WordCounter:

1. Opens each selected script.
2. Removes known metadata and CLT markers.
3. Removes punctuation/noise.
4. Splits the remaining text into words.
5. Rejects invalid/special tokens.
6. Removes common English words.
7. Counts occurrences.
8. Tracks the files in which each word appears.
9. Sorts the results by frequency.
10. Writes the top 200 words.

---

# 🧹 Word Filtering

The WordCounter deliberately removes a large list of common English words such as:

```text
the
and
is
to
you
that
this
with
from
for
```

This is intended to make the report more useful for vocabulary analysis.

It also ignores numerous translation/game-specific tokens, including:

```text
EMPTY_
<PLATFORM...>
<SIGNAL...>
MAKE_
```

and the project's CLT markers.

Italian translator metadata such as:

```text
-- Tradotto automaticamente
Alt:
NDR:
È consigliato ricontrollare questa traduzione.
```

is also removed.

---

# 🏆 Word Counter Output

The output is limited to the top:

```text
200
```

words.

Each entry contains approximately:

* rank,
* word,
* occurrence count,
* percentage of total counted words,
* list of files containing the word.

The report is saved as:

```text
words_counted.txt
```

Any existing file with that name is removed before the new report is created.

---

# 🧮 Percentage Calculation

Word percentages are calculated as:

```text
(number / total) * 100
```

The result is stored as a `long double`.

This percentage represents the proportion of counted word occurrences represented by the particular word.

---

# 💾 RAM Requirements for Word Counting

HydraulicPress requires approximately:

```text
15 GB
```

of RAM before it will run the word-counting stage.

If the detected RAM is below:

```text
15.0 GB
```

the operation is skipped and a warning is logged.

Again, this is the source's threshold rather than a guarantee that exactly 15 GB is consumed.

---

# 🧰 StringUtils

`StringUtils.cpp` contains general-purpose functionality used throughout HydraulicPress.

Important helpers include:

### `SplitByCharacter()`

Splits text using stream extraction and removes the specified delimiter from each resulting token.

When requested, it normalizes newline and tab variants.

---

### `CountOccurrences()`

Counts non-overlapping occurrences of a substring.

For example:

```text
CountOccurrences("aaa", "a")
```

returns:

```text
3
```

---

### `ReplaceSubstring()`

Replaces every occurrence of one substring with another.

This function is heavily used by the variable and randomization systems.

---

### `IsAlphanumericString()`

Determines whether a string is acceptable to the Randomizer's generated-string validation.

It rejects numerous special markers and tokens including:

```text
CLT
SIGNAL
MAKE_
VAR_
PAD
DIF
```

along with punctuation-only strings and structural braces.

---

### `TrimDirectories()`

Returns only the filename component of a path.

For example:

```text
DGRV3/chapter3/scene_12.txt
```

becomes:

```text
scene_12.txt
```

---

### `CreateRandomWeakAgree()`

Used by the Randomizer to insert:

```text
<CLT=cltWEAK>
```

or:

```text
<CLT=cltAGREE>
```

into randomized text.

The insertion position is random, and the marker may span multiple words.

---

### `ToLower()`

Converts a string to lowercase.

WordCounter uses this for stop-word comparisons.

---

# 🔗 Internal Module Relationships

HydraulicPress is best understood as a dependency graph rather than a single monolithic transformation.

```text
                         TextConfig.config
                                │
                                ▼
                         HydraulicPress
                                │
             ┌──────────────────┼──────────────────┐
             │                  │                  │
             ▼                  ▼                  ▼
         FileUtils          Platform          RAM detection
             │                  │
       ┌─────┴─────┐            │
       │           │            │
       ▼           ▼            ▼
   vars.txt   different_hashes  platform
       │           │
       └─────┬─────┘
             ▼
        selected files
             │
     ┌───────┼───────────────┐
     │       │               │
     ▼       ▼               ▼
VariableChecker VariableReplacer  LooseFloorboard
                     │
                     ├───────────────┐
                     │               │
                     ▼               ▼
                 normal output    Baked/
                     │
             ┌───────┴────────┐
             ▼                ▼
         Randomizer       WordCounter
             │                │
             ▼                ▼
 randomizer_report.txt  words_counted.txt
```

---

# 🔄 Complete Execution Order

The actual `main()` execution order is:

### 1. Initialize locale

```text
it_IT.UTF-8
```

### 2. Resolve repository/configuration paths

```text
DGRV3/
TextConfig.config
```

### 3. Parse command-line platform overrides

```text
--switch
--pc
```

### 4. Delete legacy output

If present:

```text
HydraulicPress.txt
```

is deleted.

### 5. Read configuration

```text
TextConfig.config
```

### 6. Locate variable definitions

```text
vars.txt
```

or:

```text
DGRV3/vars_bak.txt
```

### 7. Parse variables

The variable definitions become a vector of name/value pairs.

### 8. Select files

Either:

```text
DGRV3/**/*.txt
```

or:

```text
different_hashes.txt
```

depending on `CheckAllFiles`.

### 9. Select platform

One of:

```text
PC
SWITCH
XBOX
```

### 10. Check variables

Only if:

```text
DoCheckVariables = true
```

and RAM is sufficient.

### 11. Replace variables

Always invoked.

### 12. Generate baked output

Enabled because HydraulicPress calls `ReplaceVariables(..., true)`.

### 13. Replace unsupported accented characters

Only if:

```text
ReplaceBlacklistedChars = true
```

### 14. Randomize

Only if:

```text
DoRandomize = true
```

and RAM/configuration permit it.

### 15. Count words

Only if:

```text
DoCountWords = true
```

and at least approximately 15 GB of RAM is available.

### 16. Wait before exit

The program calls:

```cpp
Common::WaitExit();
```

at the end.

---

# 📄 Files Read

HydraulicPress directly or indirectly consumes:

| File                   | Purpose                                                  |
| ---------------------- | -------------------------------------------------------- |
| `TextConfig.config`    | Controls compilation behavior                            |
| `vars.txt`             | Primary variable definition file                         |
| `DGRV3/vars_bak.txt`   | Fallback variable definition file                        |
| `different_hashes.txt` | List of changed scripts when `CheckAllFiles` is disabled |
| `DGRV3/**/*.txt`       | Script source files                                      |

---

# 📄 Files Generated

Depending on configuration, HydraulicPress can generate:

| File / Directory            | Purpose                              |
| --------------------------- | ------------------------------------ |
| `variablechecker.txt`       | Detailed missing-variable report     |
| `variablechecker_short.txt` | Compact missing-variable list        |
| `var_replace_map.txt`       | Replacement audit log                |
| `Baked/`                    | Platform/CLT-processed baked scripts |
| `randomizer_report.txt`     | Randomization mapping report         |
| `words_counted.txt`         | Top-200 vocabulary report            |

The normal script files themselves are also overwritten by `VariableReplacer`.

---

# ⚠️ Important Destructive Behavior

HydraulicPress should **not** be treated as a read-only analysis tool.

The normal variable replacement stage opens the original selected `.txt` files for output and writes the compiled contents back to them.

Therefore:

```text
DGRV3/*.txt
```

can be modified in place.

This is particularly important because HydraulicPress may be followed by additional stages in the TextTools pipeline.

A clean source/repository state should be preserved if the original variable-bearing scripts need to be retained.

---

# 🧪 Diagnostic / Testing Features

Several HydraulicPress features are clearly intended for development or QA rather than ordinary release compilation:

* `DoCheckVariables`
* `DoRandomize`
* `SmartRandomization`
* `randomizer_report.txt`
* `words_counted.txt`
* `var_replace_map.txt`
* detailed platform logging

These features allow the translation team to inspect:

* missing variables,
* replacement behavior,
* platform-specific text,
* randomized script structure,
* vocabulary distribution.

---

# 🐛 Special Cases and Quirks

## `vars.txt` is checked before the repository backup

The code explicitly prefers:

```text
vars.txt
```

even though most of the surrounding tooling historically used:

```text
vars_bak.txt
```

The fallback exists because the source comments acknowledge that `vars.txt` was not originally accounted for by the other programs.

---

## `HydraulicPress.txt` is legacy

If:

```text
HydraulicPress.txt
```

exists in the working directory, it is deleted during startup.

The current implementation does not use it as a primary compilation output.

---

## Platform selection has an intentional PC/Xbox relationship

The source notes that the Xbox version uses the PC version's general text configuration:

```text
XBOX
```

is nevertheless assigned to:

```text
Distribution::Platform
```

so platform markers can still distinguish Xbox-specific content.

---

## Platform parsing has fallback behavior

If platform-tag parsing encounters an unexpected structure, `CheckPlatforms()` contains fallback behavior rather than throwing an error.

Malformed or unusual platform-marker layouts should therefore not be assumed to fail cleanly.

---

## Variable replacement is recursive

A replacement value can expose another variable and cause additional replacement passes.

This is useful but also means cyclic variable definitions can cause repeated processing until the hard iteration limit is reached.

---

## Randomizer is intentionally destructive

Randomizer deletes each input file before reconstructing it.

Its purpose is testing, not preserving source text.

---

## WordCounter is not a neutral dictionary counter

It intentionally removes:

* common English words,
* game-specific markers,
* translator metadata,
* CLT tags,
* platform markers,
* signal markers,
* `MAKE_` tokens,
* `EMPTY_` placeholders.

Consequently, `words_counted.txt` represents the project's **filtered vocabulary**, not every literal token in the files.

---

# 🛑 Failure and Early-Exit Conditions

HydraulicPress terminates with failure when:

* `vars.txt` and `vars_bak.txt` cannot be read.
* the variable list is empty.
* no files are selected for processing.

Individual modules may instead skip work when their resource requirements are not met.

For example:

```text
DoCheckVariables = true
RAM < 7 GB
```

causes variable checking to be skipped rather than terminating the entire compilation.

Likewise:

```text
DoCountWords = true
RAM < 15 GB
```

causes word counting to be skipped.

---

# 💡 Developer Notes

## HydraulicPress is primarily an orchestrator

The majority of the actual transformation logic lives outside `main()`.

The main executable is responsible for:

* configuration,
* ordering,
* platform selection,
* file selection,
* resource checks,
* module invocation.

This separation makes the individual components independently reusable within the TextTools codebase.

---

## File selection is a major integration point

`FileUtils::GetFilesToSearch()` determines the exact set of files passed to nearly every later stage.

Consequently, changing:

```text
CheckAllFiles
```

can affect:

* variable validation,
* variable replacement,
* randomization,
* word counting.

It is not merely an optimization flag.

---

## Baked output is always requested by HydraulicPress

The call is hard-coded as:

```cpp
VariableUtils::ReplaceVariables(fts, variables, true);
```

Therefore, HydraulicPress always requests baked output from VariableReplacer.

Whether platform and CLT markers are stripped from that baked output is separately controlled by:

```text
NoPlatformInBaked
NoCLTInBaked
```

---

## RAM detection measures total physical RAM

The helper named:

```text
GetAvailableRAMInGB()
```

ultimately derives its value from total physical memory.

On Windows it uses:

```text
GlobalMemoryStatusEx
```

and on non-Windows systems it uses:

```text
_SC_PHYS_PAGES
_SC_PAGE_SIZE
```

Thus the threshold checks should be understood as **system RAM capacity checks**, not exact available-free-memory measurements.

---

# 🧭 Recommended Operational Model

A safe conceptual workflow for HydraulicPress is:

```text
1. Prepare DGRV3
        ↓
2. Ensure vars.txt / vars_bak.txt is valid
        ↓
3. Run Necronomicon if using incremental processing
        ↓
4. Verify different_hashes.txt
        ↓
5. Configure TextConfig.config
        ↓
6. Select PC / Switch / Xbox
        ↓
7. Run HydraulicPress
        ↓
8. Review variablechecker.txt
        ↓
9. Review var_replace_map.txt
        ↓
10. Inspect Baked/
        ↓
11. Run downstream packaging/injection tools
```

For development builds, the optional Randomizer and WordCounter stages can be enabled as additional validation tools.

---

# 🏁 Summary

HydraulicPress is the **text compilation orchestrator of TextTools**.

Its central job is to transform the variable-rich translated scripts in `DGRV3/` into platform-filtered, variable-resolved script data.

Its major responsibilities are:

* loading `TextConfig.config`,
* selecting the target platform,
* loading `vars.txt` or `vars_bak.txt`,
* selecting files using either a full scan or `different_hashes.txt`,
* checking for missing variables,
* recursively replacing `VAR_*` definitions,
* resolving `MAKE_*` arguments,
* handling `SIGNAL_*` markers,
* filtering `<PLATFORM_...>` lines,
* generating baked scripts,
* optionally replacing unsupported accented characters,
* optionally randomizing scripts for testing,
* optionally producing vocabulary statistics.

The most important architectural relationship is:

```text
Necronomicon
      ↓
different_hashes.txt
      ↓
FileUtils
      ↓
HydraulicPress
      ↓
VariableChecker
      ↓
VariableReplacer
      ↓
Platform filtering + Baked output
      ↓
LooseFloorboard / Randomizer / WordCounter
      ↓
Compiled script data
```

In short, **HydraulicPress is the stage that turns the translation repository's editable script representation into compiled, platform-aware text suitable for the later stages of the TextTools pipeline.**