# TEXTTOOLS - POOLRULES
## Team DAIX, 2026

---

## 📌 Overview

**PoolRules** is the TextTools component responsible for measuring the **translation progress of the Danganronpa V3 script repository**.

Unlike **MarkerStone**, which primarily answers:

> “Do the translated and English files have the same number of lines?”

PoolRules asks the more useful progress question:

> “How much of the script has actually been translated?”

To answer that, PoolRules compares every eligible translated `.txt` file in:

```text
DGRV3/
````

against its corresponding English source in:

```text
DGRV3_EN/
```

It determines, on a **line-by-line basis**, whether each translated line still matches the English original. From that information it calculates:

* translated line counts,
* untranslated line counts,
* translated character counts,
* untranslated character counts,
* per-file translation percentages,
* per-file character ratios,
* per-folder/chapter translation percentages,
* per-folder/chapter character ratios,
* whole-game translation percentage,
* whole-game character ratio,
* and line-count consistency.

It also generates several reports for both human progress tracking and debugging.

PoolRules is therefore the **translation progress analyzer** of TextTools.

---

# 🎯 Responsibilities

PoolRules has six primary responsibilities:

### ✔ 1. Compare translated scripts against English scripts

Every `.txt` file under the configured DGRV3 script folders is paired with the corresponding file in `DGRV3_EN`.

The translated and English versions are then compared line-by-line.

---

### ✔ 2. Determine translated vs untranslated lines

For each English line:

* If the translated version is identical **and** the English line contains more than 3 characters → it is considered **untranslated**.
* Otherwise → it is considered **translated**.

This produces two counters:

```text
Translated lines
Untranslated lines
```

---

### ✔ 3. Count translated and untranslated characters

PoolRules also counts the number of characters represented by:

* translated lines,
* untranslated English lines.

This allows it to calculate a second progress metric based on text length rather than line count.

---

### ✔ 4. Detect line mismatches

PoolRules verifies that the translated and English files contain the same number of relevant lines.

A mismatch is recorded at the file level and propagated upward to the chapter and game levels.

---

### ✔ 5. Aggregate statistics

Statistics are accumulated at three levels:

```text
File
  ↓
Chapter / folder
  ↓
Whole game
```

This allows the tool to report both detailed file-level progress and a single overall completion percentage.

---

### ✔ 6. Produce diagnostic reports

PoolRules generates:

```text
percentage_res.txt
charcount_res.txt
detailed_charcount_rex.txt
file_line_test.txt
```

These reports provide progressively more detailed views of the translation state.

---

# 🧩 How PoolRules Fits Into TextTools

PoolRules is an **analysis/QA tool**, rather than a compilation or modification tool.

A typical TextTools workflow is conceptually:

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
Electrohammer / later packaging tools
```

The exact execution order can vary depending on the development workflow, but PoolRules is primarily concerned with **measuring the state of the translation repository**.

It is especially useful after translation edits and before release/packaging because it can reveal:

* untranslated lines,
* suspiciously incomplete chapters,
* line-count mismatches,
* unexpected file-level discrepancies,
* and overall translation completion.

---

# 🆚 PoolRules vs MarkerStone

PoolRules deliberately overlaps with some functionality from MarkerStone, but the two tools serve different purposes.

| Tool            | Primary purpose                              |
| --------------- | -------------------------------------------- |
| **MarkerStone** | Validate line-count consistency              |
| **PoolRules**   | Analyze translation progress and consistency |

MarkerStone essentially asks:

```text
Do the files have the same number of lines?
```

PoolRules asks:

```text
How many lines were translated?
How many remain untranslated?
How many characters were translated?
How much of each chapter is complete?
How much of the entire game is complete?
Do the files still have matching line structures?
```

Therefore:

```text
MarkerStone = structural sanity check
PoolRules    = translation progress analyzer
```

PoolRules does its own line-count verification rather than relying exclusively on MarkerStone's results.

---

# 📁 Required Repository Structure

PoolRules expects the translated repository to exist in the current working directory as:

```text
DGRV3/
```

It also expects the English repository:

```text
DGRV3_EN/
```

The two repositories should mirror each other structurally.

For example:

```text
DGRV3/
├── chapter1/
│   ├── scene_01.txt
│   └── scene_02.txt
└── chapter2/
    └── scene_01.txt

DGRV3_EN/
├── chapter1/
│   ├── scene_01.txt
│   └── scene_02.txt
└── chapter2/
    └── scene_01.txt
```

PoolRules constructs the English counterpart by replacing the `DGRV3` path component with `DGRV3_EN`.

---

# 🚀 Startup Behavior

PoolRules begins by defining the two repository names:

```text
DGRV3
DGRV3_EN
```

It resolves both relative to:

```text
std::filesystem::current_path()
```

The translated path therefore becomes:

```text
<current directory>/DGRV3
```

and the English path:

```text
<current directory>/DGRV3_EN
```

---

# 🛑 Initial Repository Validation

PoolRules explicitly checks whether:

```text
DGRV3/
```

exists.

If it does not, the program logs an error similar to:

```text
Path does not exist: <path>, aborting!
```

and returns:

```text
EXIT_FAILURE
```

### Important limitation

The initial `main()` check only explicitly validates the translated repository.

`DGRV3_EN/` is passed to `CalculatePercentages()`, but there is no equivalent early top-level existence check for it.

Therefore, a missing English repository can result in errors later during individual file comparisons rather than being rejected immediately.

---

# 📂 Script Folders

PoolRules does **not** recursively scan the entire `DGRV3` repository.

Instead, it uses a fixed list of 14 folders:

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

The order is fixed and is also the order used in the output reports.

---

# 📑 Folder Order

The exact processing order is:

| Index | Folder          |
| ----: | --------------- |
|     1 | `ainori`        |
|     2 | `chapter1`      |
|     3 | `chapter2`      |
|     4 | `chapter3`      |
|     5 | `chapter4`      |
|     6 | `chapter5`      |
|     7 | `chapter6`      |
|     8 | `epilogue`      |
|     9 | `gallery`       |
|    10 | `game_resident` |
|    11 | `MapObjName`    |
|    12 | `prologue`      |
|    13 | `subroutine`    |
|    14 | `test`          |

This is important because `percentage_res.txt` contains results in this exact order.

---

# 🔎 File Discovery

For each folder, PoolRules uses:

```cpp
std::filesystem::recursive_directory_iterator()
```

Therefore, subdirectories beneath each folder are also searched.

Only files whose extension is:

```text
.txt
```

are processed.

Directories are ignored.

---

# 📄 File Input

For every eligible translated `.txt` file, PoolRules:

1. Opens the translated file.
2. Reads it line-by-line.
3. Builds an in-memory vector containing its relevant lines.
4. Counts its characters.
5. Counts its relevant lines.
6. Locates the corresponding English file.
7. Reads the English file.
8. Compares corresponding lines.
9. Calculates translation and character statistics.

---

# 🧹 Line Normalization

Before comparison, PoolRules performs some basic normalization.

For each line it removes:

```text
\r
\n
```

using regular-expression replacement.

This prevents Windows-style carriage returns from affecting the comparison.

---

# `{` and `}` Lines

PoolRules explicitly ignores lines whose first character is:

```text
{
```

or:

```text
}
```

These are treated as structural brackets rather than translatable text.

Consequently, they:

* are not stored in the comparison vector,
* do not contribute to character counts,
* do not contribute to line counts,
* do not participate in translation classification.

---

# Empty / Newline-Only Lines

Lines beginning with newline/carriage-return characters are also skipped.

Because `std::getline()` normally removes the newline delimiter, this primarily matters for unusual input containing those characters directly.

---

# 📊 File-Level Statistics

Each processed file is represented internally by a `FileEntry`.

The structure contains:

```text
f_name
f_total_tr
f_total_utr
f_total_characters_tr
f_total_characters_utr
f_total_lines
f_characters_percentage
f_translation_percentage
f_lines_match
```

These fields represent the complete statistical state of one script file.

---

# 🟢 `f_total_tr`

Number of lines considered translated.

A line is considered translated when:

```text
translated line != English line
```

or when the English line is short enough to trigger the short-line exception.

---

# 🔴 `f_total_utr`

Number of lines considered untranslated.

A line is considered untranslated only when:

```text
translated line == English line
```

and:

```text
English line length > 3
```

---

# 🔤 `f_total_characters_tr`

This counter is based on the **translated file's actual text**.

Every retained translated line contributes:

```text
line.length()
```

to:

```text
f_total_characters_tr
```

This is important because translated text can be longer or shorter than the English source.

---

# 🔤 `f_total_characters_utr`

This counter is based on the **English source line** for lines classified as either translated or untranslated.

Despite its name, the code adds the English line's length to:

```text
f_total_characters_utr
```

for **every English line that is compared**, not only lines classified as untranslated.

Therefore, the naming is somewhat misleading.

This value effectively represents the total English-side character count used as the denominator/reference for the character ratio.

---

# 📏 `f_total_lines`

This is the number of relevant translated lines PoolRules encountered.

Because `{` and `}` lines and certain empty lines are ignored, this is **not necessarily the literal number of physical lines in the file**.

---

# ✅ `f_lines_match`

This boolean verifies that the translated and English files contain the same number of relevant lines.

The exact condition is:

```text
translated translated-line-count
    +
translated untranslated-line-count
    ==
translated relevant-line-count

AND

translated relevant-line-count
    ==
English comparison-line-count
```

In practical terms:

```text
TR relevant lines == EN relevant lines
```

and all English lines must have been classified.

---

# 🔬 Translation Classification Algorithm

The central comparison works as follows.

First, PoolRules stores every relevant translated line:

```text
TR[0]
TR[1]
TR[2]
...
```

Then it reads the corresponding English file:

```text
EN[0]
EN[1]
EN[2]
...
```

For every English line, it accesses the corresponding translated vector entry using a counter.

It then performs:

```cpp
is_the_same = translated_line == english_line;
```

and:

```cpp
short_original = english_line.length() <= 3;
```

The classification is:

```text
if same AND NOT short:
    untranslated
else:
    translated
```

---

# 🧠 The Short-Line Exception

The `<= 3` character rule is one of PoolRules' most important quirks.

If an English line is 3 characters or fewer, it is automatically considered translated even if the translated line is identical.

For example:

```text
EN: Yes
TR: Yes
```

would be treated as:

```text
Translated
```

because:

```text
"Yes".length() == 3
```

Likewise:

```text
EN: No
TR: No
```

is considered translated.

This is intentional according to the source's logic, presumably because very short strings are unreliable indicators of translation work.

---

# ⚠️ Consequence of the Short-Line Rule

The short-line rule means PoolRules does **not** perform a literal equality test for all text.

Instead:

```text
same + >3 characters
    → untranslated

same + <=3 characters
    → translated
```

Therefore, a report showing 100% translation does not necessarily mean every source line differs byte-for-byte from English.

---

# 🔄 English File Resolution

PoolRules uses:

```text
ReplaceWithEnglish()
```

to convert a translated file path into its English equivalent.

For example:

```text
DGRV3/chapter3/scene_12.txt
```

becomes:

```text
DGRV3_EN/chapter3/scene_12.txt
```

The function walks through the filesystem path components.

The first component matching:

```text
DGRV3
```

is replaced with:

```text
DGRV3_EN
```

All remaining path components are preserved.

---

# 🧩 Why Path Replacement Is Used

This allows PoolRules to compare corresponding files without needing a separate filename-mapping table.

For example:

```text
DGRV3/
└── chapter5/
    └── scene_01.txt
```

automatically maps to:

```text
DGRV3_EN/
└── chapter5/
    └── scene_01.txt
```

The directory structure therefore acts as the mapping mechanism.

---

# ⚠️ Path Replacement Limitation

`ReplaceWithEnglish()` only replaces the **first path component matching the repository name**.

If a path does not contain the expected `DGRV3` component, it will not magically construct an English path.

This means PoolRules assumes the repository structure follows the expected layout.

---

# 📐 Translation Percentage

The primary translation percentage is based on **line counts**.

The formula is:

```text
translated lines
------------------------------- × 100
translated lines + untranslated lines
```

or:

```text
TR / (TR + UTR) × 100
```

For example:

```text
Translated = 800
Untranslated = 200
```

produces:

```text
800 / (800 + 200) × 100
= 80%
```

This value is stored as:

```text
f_translation_percentage
```

at the file level.

The same calculation is performed for chapters and the entire game.

---

# 🔤 Character Ratio

PoolRules also calculates a character-based ratio.

The formula is:

```text
translated character count
-------------------------- × 100
English/reference character count
```

Conceptually:

```text
TranslatedC / UntranslatedC × 100
```

This value is stored as:

```text
f_characters_percentage
```

for individual files.

---

# ⚠️ Character Ratio Is Not a Conventional Completion Percentage

The character ratio is fundamentally different from the line-based translation percentage.

If the translation changes the length of the text, the ratio can exceed 100%.

For example:

```text
English:     100 characters
Translated:  130 characters
```

produces:

```text
130%
```

The source explicitly treats this as a **1:1 character ratio**, where 100% means equal character counts.

Therefore:

```text
Translation Ratio
```

and:

```text
Character Ratio
```

should not be interpreted as interchangeable metrics.

---

# 💯 Special 100% Character Cases

If:

```text
untranslated/reference characters == 0
```

or:

```text
translated characters == untranslated/reference characters
```

PoolRules sets:

```text
characters percentage = 100.0
```

This prevents division by zero and explicitly treats a zero-reference case as complete.

---

# 📈 Chapter Aggregation

After every file in a folder has been processed, PoolRules aggregates the results into a `ChapterEntry`.

The chapter accumulates:

```text
Translated lines
Untranslated lines
Translated characters
Untranslated/reference characters
Total lines
Line-match status
```

For example:

```text
chapter3/
├── scene01.txt
├── scene02.txt
└── scene03.txt
```

becomes one aggregated:

```text
ChapterEntry
```

---

# 🏆 Chapter Translation Percentage

The chapter translation percentage is calculated from the accumulated line counts:

```text
chapter translated lines
------------------------ × 100
chapter translated + untranslated lines
```

This produces:

```text
c_translation_percentage
```

---

# 🧮 Chapter Character Ratio

The chapter character ratio is calculated from accumulated character totals:

```text
chapter translated characters
----------------------------- × 100
chapter reference characters
```

This produces:

```text
c_characters_percentage
```

---

# 🚨 Chapter Line-Mismatch Status

Each chapter starts with its line-match status derived from its contained files.

PoolRules effectively performs:

```text
chapter lines_match =
    file1.lines_match
    AND file2.lines_match
    AND file3.lines_match
    ...
```

Therefore, one mismatched file causes the entire chapter to be marked as having different lines.

PoolRules logs:

```text
<folder> has different lines!
```

when this happens.

---

# 🌍 Whole-Game Aggregation

Once all 14 folders have been processed, PoolRules accumulates their statistics into a `GameEntry`.

The game-level fields include:

```text
g_total_tr
g_total_utr
g_total_lines
g_total_characters_tr
g_total_characters_utr
g_characters_percentage
g_translation_percentage
g_lines_match
```

---

# 🏆 Whole-Game Translation Percentage

The overall completion percentage is:

```text
total translated lines
---------------------- × 100
total translated + untranslated lines
```

This is the primary "how complete is the translation?" metric.

---

# 🔤 Whole-Game Character Ratio

The overall character ratio is:

```text
total translated characters
--------------------------- × 100
total reference characters
```

As with the file and chapter values, this is a ratio rather than a strict completion percentage.

---

# 🚫 Preventing Reported Completion Above 100%

When writing the final completion report, PoolRules clamps the line-based percentage:

```text
min(result, 100.0)
```

This is done for both:

* chapter completion,
* total game completion.

Therefore, even if an unusual internal calculation produces a value above 100%, the displayed completion percentage is capped at:

```text
100%
```

The character ratio is **not subjected to the same output clamp**.

---

# 📊 `percentage_res.txt`

The simplest output generated by PoolRules is:

```text
percentage_res.txt
```

It contains one numeric percentage per line.

The first 14 values correspond to the folders in this order:

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

The final value is the whole-game completion percentage.

Conceptually:

```text
<ainori percentage>
<chapter1 percentage>
<chapter2 percentage>
...
<test percentage>
<total game percentage>
```

---

# 🧮 `percentage_res.txt` Precision

PoolRules uses:

```cpp
std::setprecision(3)
```

when writing the percentages.

Because no `fixed` formatting is specified, this is **three significant digits**, rather than necessarily three digits after the decimal point.

Therefore output may look like:

```text
85.3
```

or:

```text
100
```

depending on the value.

---

# 📜 Console Completion Output

PoolRules also logs each chapter's completion percentage.

The format is conceptually:

```text
Completion ratio for chapter3:    82.5 %    (825/175)
```

where the values in parentheses represent:

```text
translated lines / untranslated lines
```

The output is primarily intended for human-readable progress monitoring.

---

# 📚 `charcount_res.txt`

The second major report is:

```text
charcount_res.txt
```

This contains both character and line statistics for every folder.

Each chapter contains:

```text
<folder name>
[Y] TranslatedC: <translated characters>
[N] TranslatedC: <reference/untranslated characters>
Character Ratio: <ratio>%
[Y] TranslatedL: <translated lines>
[N] TranslatedL: <untranslated lines>
Translation Ratio: <percentage>%
```

A final:

```text
Total:
```

section contains the whole-game equivalents.

---

# 🔍 Meaning of `[Y]` and `[N]`

The report uses:

```text
[Y]
[N]
```

rather than explicitly saying "translated" and "untranslated" in every case.

For lines:

```text
[Y] TranslatedL
[N] TranslatedL
```

means:

```text
[Y] = translated lines
[N] = untranslated lines
```

For characters:

```text
[Y] TranslatedC
[N] TranslatedC
```

the labels refer to the translated/reference character buckets used by the program.

The character-side `[N]` value is effectively the English/reference character total used for the ratio, not strictly a count of only characters belonging to lines classified as untranslated.

---

# 📖 `detailed_charcount_rex.txt`

The most detailed normal report is:

```text
detailed_charcount_rex.txt
```

It contains:

* every chapter,
* chapter totals,
* every processed file,
* per-file line status,
* per-file character counts,
* per-file character ratio,
* per-file translated line count,
* per-file untranslated line count,
* per-file translation percentage,
* whole-game totals.

This is the primary forensic report when investigating why the overall percentage looks unexpected.

---

# 🗂️ Detailed Report Structure

The report is organized approximately as:

```text
ainori

    Lines match: ...
    TranslatedC: ...
    ...
    
    Files:

        file1.txt

            Lines match: ...
            TranslatedC: ...
            UntranslatedC: ...
            Character Ratio: ...
            TranslatedL: ...
            UntranslatedL: ...
            Translation Ratio: ...

        file2.txt
            ...

chapter1
    ...

chapter2
    ...

...

Total:
    ...
```

This allows a developer to drill down from:

```text
Game
  ↓
Chapter
  ↓
File
```

---

# 🧪 `file_line_test.txt`

PoolRules also creates:

```text
file_line_test.txt
```

This is a diagnostic artifact rather than a primary translation report.

Its purpose is to verify that the line-count assumptions used by PoolRules are actually valid for selected test files.

---

# 🧪 Test File List

PoolRules contains a hard-coded collection of test paths.

These include examples from:

```text
subroutine
prologue
game_resident
MapObjName
```

and both translated and English versions of selected files.

Examples include:

```text
prologue/c00_999_002.txt
game_resident/A-MapListNameAscii.txt
game_resident/A-MapName.txt
game_resident/D-Menu.txt
game_resident/J-TutorialButton.txt
game_resident/O-Event.txt
game_resident/S-SaveLoad.txt
MapObjName/ID132_lab_hoshi_text.txt
```

The list is intended to catch cases where line handling behaves unexpectedly.

---

# 🔬 Diagnostic Line Dump

For each test file, PoolRules records:

* shortened filename,
* whether the corresponding file was believed to have matching lines,
* actual number of lines read,
* every line,
* the numeric byte value of every character.

Conceptually:

```text
File: chapter3/scene_12.txt
Lines match (supposedly): Yes
Lines No.: 125

[1] "Some text"
    (83) (111) (109) (101) ...

[2] "Another line"
    (65) (110) (111) ...
```

This makes the file useful for investigating encoding, newline, or line-count problems.

---

# 🗺️ The `linemap`

PoolRules maintains an internal:

```text
std::map<std::string, bool>
```

called:

```text
linemap
```

It records line-match results for files appearing in the hard-coded test list.

The later diagnostic report uses this map to display:

```text
Lines match (supposedly): Yes
```

or:

```text
Lines match (supposedly): No
```

---

# ⚠️ Test-File Matching Quirk

The test-file lookup compares only the **filename component**.

It does not require the complete path to match.

Therefore, if two unrelated directories contain files with the same filename, they can potentially collide in the diagnostic `linemap`.

This does not affect the primary translation calculations, but it can make `file_line_test.txt` misleading for duplicated filenames.

---

# 🔄 Complete Processing Flow

PoolRules can be summarized as:

```text
Current directory
       │
       ├── DGRV3/
       │
       └── DGRV3_EN/
               │
               ▼
      Iterate 14 known folders
               │
               ▼
      Recursively find .txt files
               │
               ▼
        Read translated file
               │
        ┌──────┴──────┐
        │             │
        ▼             ▼
   Count lines    Count chars
        │
        ▼
  Store TR lines
        │
        ▼
 Find corresponding EN file
        │
        ▼
     Read EN lines
        │
        ▼
 Compare TR[i] vs EN[i]
        │
        ├── same + >3 chars
        │       ↓
        │   untranslated
        │
        └── otherwise
                ↓
            translated
        │
        ▼
  Check line counts
        │
        ▼
 Calculate file ratios
        │
        ▼
 Aggregate into chapter
        │
        ▼
 Calculate chapter ratios
        │
        ▼
 Aggregate all chapters
        │
        ▼
 Calculate game ratios
        │
        ▼
 Generate reports
```

---

# 🧱 Internal Data Model

PoolRules uses three levels of data structures.

## `FileEntry`

Represents one script file.

```text
FileEntry
├── filename
├── translated lines
├── untranslated lines
├── translated characters
├── reference characters
├── total lines
├── character ratio
├── translation percentage
└── line-match status
```

---

## `ChapterEntry`

Represents one of the 14 script folders.

```text
ChapterEntry
├── FileEntry[]
├── translated lines
├── untranslated lines
├── total lines
├── translated characters
├── reference characters
├── character ratio
├── translation percentage
└── line-match status
```

---

## `GameEntry`

Represents the entire game.

```text
GameEntry
├── ChapterEntry[14]
├── translated lines
├── untranslated lines
├── total lines
├── translated characters
├── reference characters
├── character ratio
├── translation percentage
└── line-match status
```

This hierarchy is the basis for all aggregation.

---

# 🧮 Aggregation Logic

PoolRules does not recalculate game percentages by averaging chapter percentages.

Instead, it first sums the raw counters:

```text
Chapter 1 translated lines
+ Chapter 2 translated lines
+ ...
+ Chapter 14 translated lines
```

and separately sums the untranslated counts.

The final game percentage is then calculated from these totals.

This is important because chapters with different amounts of text are weighted according to their actual line counts.

---

# ⚖️ Why This Matters

Suppose:

```text
Chapter 1 = 90% complete, 10,000 lines
Chapter 2 = 50% complete, 100 lines
```

A simple average would produce:

```text
70%
```

PoolRules instead weights them by their actual line counts.

Therefore the larger chapter contributes proportionally more to the game-wide completion percentage.

This makes the whole-game metric much more meaningful.

---

# 🚨 Line Mismatch Handling

A line mismatch does **not immediately terminate the entire program**.

Instead, PoolRules:

1. Marks the individual file as mismatched.
2. Logs the mismatch.
3. Propagates the mismatch to the containing chapter.
4. Propagates it to the game.
5. Still generates the reports.

At the end, `CalculatePercentages()` can return successfully even if line mismatches were detected.

The top-level `main()` then logs:

```text
Different lines or some error! Couldn't calculate percentage(s).
```

when the calculation function returns false.

However, the normal mismatch path inside the file-processing loop primarily records the mismatch and continues.

---

# ⚠️ English File Has More Lines

If the English file produces more comparison lines than the stored translated vector, PoolRules logs:

```text
<filename> has more lines in ENG than TR!
```

and breaks out of the English comparison loop.

This prevents an out-of-range vector access.

The file will subsequently be marked as having mismatched lines.

---

# ⚠️ English File Has Fewer Lines

If the translated file contains more relevant lines than the English file, the comparison counter will not reach the translated vector's full size.

The final line-match check catches this because:

```text
translated lines != English lines
```

and the file becomes:

```text
f_lines_match = false
```

---

# 📌 Important: PoolRules Does Not Automatically Fix Mismatches

PoolRules is an analyzer.

It does not:

* insert missing lines,
* remove extra lines,
* modify translated files,
* modify English files,
* attempt to repair synchronization,
* or call MarkerStone to fix anything.

It only reports the problem.

---

# 🧪 No Command-Line Flags

The provided PoolRules source does not expose a command-line configuration interface.

Unlike some other TextTools programs, there are no documented runtime switches such as:

```text
--all
--pc
--switch
```

The tool simply operates on the repositories it finds in the current working directory.

---

# 💾 Files Modified

PoolRules does **not modify the contents of `DGRV3/` or `DGRV3_EN/`**.

Its operation is read-oriented.

The only files it modifies are its own generated reports.

---

# 📄 Generated Files

| File                         | Purpose                                             |
| ---------------------------- | --------------------------------------------------- |
| `percentage_res.txt`         | Compact chapter + whole-game completion percentages |
| `charcount_res.txt`          | Chapter and game character/line statistics          |
| `detailed_charcount_rex.txt` | Detailed chapter/file-level statistics              |
| `file_line_test.txt`         | Low-level line/character diagnostic dump            |

---

# 🔁 Report Overwrite Behavior

PoolRules removes previous versions of some reports before generating new ones.

For example:

```text
percentage_res.txt
```

is removed if it already exists.

Likewise:

```text
charcount_res.txt
```

is removed before regeneration.

The reports are then recreated.

---

# ⚠️ `file_line_test.txt` Uses Append Mode

Unlike the main reports, the line-test report is opened with:

```text
std::ios::app
```

Therefore, repeated executions can append new diagnostic data to the existing:

```text
file_line_test.txt
```

instead of replacing it.

This means the file can accumulate historical runs.

Developers should delete it manually when a clean diagnostic capture is desired.

---

# 🔐 File Encoding Considerations

PoolRules opens its primary files using:

```text
std::ios::binary
```

and performs string comparisons on the resulting byte sequences after removing CR/LF characters.

It does not perform Unicode-aware normalization.

Therefore, two visually identical strings can still be considered different if their underlying byte sequences differ.

This is particularly relevant for:

* accented characters,
* UTF-8 sequences,
* Unicode normalization,
* unusual punctuation,
* different encoding formats.

---

# ⚠️ Character Counting Is Byte-Oriented

The code uses:

```cpp
std::string::length()
```

for character counts.

For UTF-8 text, this represents the number of **bytes**, not necessarily the number of human-readable Unicode characters/code points.

For example, an accented Unicode character may occupy multiple bytes in UTF-8.

Therefore:

```text
Character Ratio
```

should technically be understood as a **string byte-length ratio**, not a Unicode grapheme/character ratio.

---

# 🧩 Brackets Affect Statistics

Because `{` and `}` lines are ignored, PoolRules intentionally excludes structural wrapper lines from:

* line totals,
* translation comparisons,
* character totals.

This makes the translation percentage focus on actual text rather than file structure.

---

# 🧠 What Counts as "Translated"

The most important conceptual rule in PoolRules is:

```text
Translated
=
TR line differs from EN line
OR
EN line length <= 3
```

It does **not** inspect:

* translation quality,
* language,
* grammar,
* whether only one punctuation mark changed,
* whether the translator merely added whitespace,
* whether a variable changed,
* whether the line was intentionally left in English.

Any difference is enough to classify the line as translated.

---

# ⚠️ False-Positive Translation Possibilities

Because comparison is binary, changes such as:

```text
Hello!
```

→

```text
Hello.
```

are considered translated.

Likewise:

```text
Hello
```

→

```text
Hello 
```

may be considered translated if the underlying strings differ.

Therefore, PoolRules measures **textual difference**, not translation quality.

---

# ⚠️ False-Negative Translation Possibilities

Conversely, a legitimate translation that happens to be identical to the English source is considered untranslated if the English line contains more than 3 characters.

For example:

```text
EN: Radio
TR: Radio
```

is considered:

```text
Untranslated
```

even if "Radio" is a perfectly valid Italian word.

---

# 📊 Interpreting the Metrics

PoolRules produces two fundamentally different progress measurements.

## Translation Percentage

```text
Translated lines
---------------- × 100
All relevant lines
```

Use this as the primary completion metric.

---

## Character Ratio

```text
Translated bytes
---------------- × 100
English/reference bytes
```

Use this as a secondary indicator of how the translated text compares in length with the source.

---

# 🏁 Example Interpretation

Suppose PoolRules reports:

```text
Translation Ratio: 80%
Character Ratio: 115%
```

This does **not** mean:

```text
115% translated
```

It means approximately:

```text
80% of relevant lines have been classified as translated
```

while:

```text
the translated text contains 115% as many bytes as the English reference
```

The latter can happen naturally because translated dialogue may be longer than English.

---

# 🛠️ Developer Notes

## PoolRules is intentionally self-contained

The source does not depend on MarkerStone's generated files to calculate its main statistics.

It performs its own line comparison.

This makes it useful as an independent validation layer.

---

## Fixed folder list

Adding a new script folder to the repository does not automatically make PoolRules analyze it.

The folder must be added to:

```cpp
possible_folders
```

in the source.

This also means the output structure assumes exactly 14 chapter/folder entries.

---

## Reports assume all 14 folders exist

The program stores chapter results in:

```cpp
std::array<ChapterEntry, 14>
```

and later accesses each entry by the fixed folder order.

If one of the expected folders is missing or contains no eligible `.txt` files, the function can abort.

The source explicitly checks:

```text
No file entries found in chapter: <folder>
Aborting.
```

---

## Empty folders are fatal

PoolRules expects every listed folder to contain at least one eligible `.txt` file.

If:

```text
F_Entries.size() == 0
```

for a folder, `CalculatePercentages()` returns false.

Therefore, the tool is tightly coupled to the expected repository structure.

---

## Missing English counterparts are not cleanly handled

PoolRules constructs the English counterpart path but does not perform a strong explicit existence check before opening it.

Consequently, a missing English file can result in an empty/unusable comparison stream and ultimately a line mismatch.

Developers should ensure that `DGRV3_EN/` mirrors `DGRV3/` before running the tool.

---

# 🧪 Diagnostic Design

The hard-coded `test_files` list is particularly useful for debugging the assumptions behind line matching.

It appears to exist because certain script files historically caused uncertainty around:

* line counts,
* bracket handling,
* newline behavior,
* file structure,
* and character encoding.

The diagnostic report goes below the normal statistics layer and exposes raw line data and byte values.

---

# 🔬 Why `file_line_test.txt` Is Useful

If PoolRules claims:

```text
scene_12.txt has different lines
```

a developer can use:

```text
file_line_test.txt
```

to inspect the raw representation of a selected test file.

The byte dump can reveal situations such as:

```text
\r
\n
```

differences or unexpected encoded characters.

It is therefore a low-level debugging aid rather than a translation report.

---

# ⚠️ Source-Level Quirks

### 1. The character field names are somewhat misleading

The field:

```text
f_total_characters_utr
```

sounds like it should contain only characters belonging to untranslated lines.

In reality, the code increments it for every English comparison line.

Therefore it functions more like:

```text
English/reference character total
```

than a literal untranslated-character count.

---

### 2. Character ratio can exceed 100%

Unlike translation completion, the raw character ratio is not inherently capped.

Longer translated text can produce:

```text
110%
120%
130%
```

etc.

Only the line-based completion percentage is clamped to 100% in `percentage_res.txt`.

---

### 3. Short English lines automatically count as translated

This is a deliberate heuristic but can make the line-based percentage slightly higher than a strict equality comparison would produce.

---

### 4. Comparison is positional

PoolRules compares:

```text
TR[0] ↔ EN[0]
TR[1] ↔ EN[1]
TR[2] ↔ EN[2]
```

It does not search for matching lines elsewhere.

A single inserted/deleted line can therefore shift every subsequent comparison and cause many lines to appear translated even when they are not.

This is one of the strongest reasons why line integrity must be maintained.

---

# 🔗 Relationship With MarkerStone

MarkerStone and PoolRules are complementary.

A useful conceptual relationship is:

```text
MarkerStone
    │
    └── "Are TR and EN structurally aligned?"
                    │
                    ▼
                PoolRules
                    │
                    └── "Given that alignment, how much is translated?"
```

If line counts do not match, PoolRules' line-by-line translation classification becomes less trustworthy after the point where the files diverge.

Therefore, a clean MarkerStone result increases confidence in PoolRules' percentages.

---

# 🔗 Relationship With Necronomicon

PoolRules does not directly consume:

```text
different_hashes.txt
```

and does not use SHA-512 to decide which files changed.

Its purpose is broader:

```text
Necronomicon
    ↓
Which files differ?
```

versus:

```text
PoolRules
    ↓
How much of the translation is complete?
```

They answer different questions and can be run independently.

---

# 🔗 Relationship With HydraulicPress

HydraulicPress is a compilation/transformation stage.

PoolRules is an analysis stage.

This distinction is important because HydraulicPress can modify script contents through:

* variable replacement,
* platform processing,
* signal handling,
* baked output generation.

PoolRules should therefore be run against the intended **translation-analysis representation** of the repository rather than an arbitrary generated output directory.

In particular, PoolRules explicitly scans:

```text
DGRV3/
```

and ignores:

```text
Baked/
```

because `Baked/` is outside its fixed repository-folder traversal.

---

# 🧭 Recommended Usage

A sensible translation-development workflow is:

```text
1. Edit translation
       ↓
2. Verify DGRV3 structure
       ↓
3. Run MarkerStone
       ↓
4. Fix line mismatches if necessary
       ↓
5. Run PoolRules
       ↓
6. Review percentage_res.txt
       ↓
7. Review charcount_res.txt
       ↓
8. Inspect detailed_charcount_rex.txt
       ↓
9. Investigate suspicious files
       ↓
10. Continue compilation / packaging
```

If PoolRules reports an unexpectedly low or high completion percentage, the first files to inspect should be the ones flagged by the detailed report as having unusual:

* line counts,
* translation counts,
* character ratios,
* or line mismatches.

---

# 🧰 Failure Handling

PoolRules can fail when:

* `DGRV3/` does not exist.
* An expected script folder contains no eligible `.txt` files.
* The repository structure is incomplete.
* A comparison encounters an unexpected line arrangement.
* The calculation function encounters another internal error.

Line mismatches themselves are primarily **reported**, not automatically repaired.

---

# 📄 Input / Output Summary

## Inputs

| Input                 | Purpose                                        |
| --------------------- | ---------------------------------------------- |
| `DGRV3/`              | Translated script repository                   |
| `DGRV3_EN/`           | English reference repository                   |
| Fixed folder list     | Determines which repository areas are analyzed |
| `.txt` files          | Actual translation data                        |
| Hard-coded test paths | Diagnostic line validation                     |

---

## Outputs

| Output                       | Purpose                                            |
| ---------------------------- | -------------------------------------------------- |
| `percentage_res.txt`         | Compact completion percentages                     |
| `charcount_res.txt`          | Character and line statistics                      |
| `detailed_charcount_rex.txt` | File/chapter/game detailed analysis                |
| `file_line_test.txt`         | Raw line/byte diagnostic information               |
| Console logs                 | Progress, completion ratios, and mismatch warnings |

---

# 🔐 Data Safety

PoolRules is fundamentally a **read-and-report tool**.

It does not rewrite the translated or English script repositories.

Its normal side effects are the creation/replacement/appending of its report files.

This makes it considerably safer to run against an active translation repository than tools such as HydraulicPress or Randomizer, which can modify script contents.

---

# 🏁 Summary

PoolRules is the **translation progress analyzer of TextTools**.

Its core job is to compare:

```text
DGRV3/
```

against:

```text
DGRV3_EN/
```

and turn those comparisons into meaningful translation statistics.

At the file level it determines:

* translated lines,
* untranslated lines,
* translated character bytes,
* English/reference character bytes,
* translation percentage,
* character ratio,
* and line-count consistency.

It then aggregates those statistics through:

```text
File
 ↓
Chapter
 ↓
Whole Game
```

and produces both compact and highly detailed reports.

The central translation rule is:

```text
TR line == EN line
AND
EN length > 3
        ↓
Untranslated

Otherwise
        ↓
Translated
```

The central completion formula is:

```text
Translated Lines
──────────────────────────── × 100
Translated + Untranslated Lines
```

while the character metric is:

```text
Translated Character Bytes
─────────────────────────── × 100
English Reference Bytes
```

The resulting reports provide the translation team with a quantitative view of progress while simultaneously checking whether the translated and English script structures remain aligned.

In short:

```text
MarkerStone → "Are the scripts structurally synchronized?"

PoolRules   → "How much of the synchronized script is translated?"
```

PoolRules is therefore the **progress meter, statistical analyzer, and secondary structural validator** for the TextTools translation pipeline.