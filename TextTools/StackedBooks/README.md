# TEXTTOOLS - STACKEDBOOKS
## Team DAIX, 2026  

---

## 📘 Overview

**StackedBooks** is the simplest component in the entire TextTools suite — and intentionally so.  
Its job is *not* to process text, patch files, or manipulate SPC archives.  
Instead, **StackedBooks is a tiny launcher whose sole purpose is to run `HiddenDoor`**, the actual heavy‑duty text compiler used by TextTools.

Think of it as a convenience wrapper:  
A minimal executable that forwards command‑line arguments and ensures `HiddenDoor` is invoked correctly inside the TextTools environment.

---

## 🎯 Purpose

StackedBooks exists to:

- Provide a stable entry point for DailyManager and other automation tools  
- Forward all CLI arguments directly to `HiddenDoor`  
- Check that `HiddenDoor` is present before running  
- Log basic startup information  
- Avoid duplicating logic across multiple tools that need to call `HiddenDoor`

It is essentially the “book on top of the stack” — the last tiny piece that triggers the real text compilation pipeline.

---

## 🧩 How It Fits Into TextTools

StackedBooks is part of the **TextTools orchestration layer**, alongside:

- **Ropeway** — repository manager  
- **CrammedPiranhas** — archive extractor  
- **Electrohammer** — text compiler  
...

StackedBooks is the simplest of them all:  
It does *not* modify files, clone repos, or generate patches.  
It simply ensures that HiddenDoor is executed with the correct arguments.

---

## ⚙️ Internal Behavior

### 1. **Argument Forwarding**
StackedBooks collects all CLI arguments passed to it:

```cpp
std::vector<std::string> args(argv, argv + argc);
std::string args_str = std::accumulate(args.begin(), args.end(), std::string{" "});
```

Everything after the executable name is forwarded unchanged to HiddenDoor.

### 2. Flag Detection

It checks for the --all flag:

```cpp
if (std::any_of(args.begin(), args.end(), [](std::string const& arg) {
    return Common::StringContains(arg, "--all");
}))
```

This flag is simply passed along to HiddenDoor.

### 3. Executable Validation

Before running, it verifies that HiddenDoor.exe (Windows) or ./HiddenDoor (Linux) exists:

```cpp
if (!std::filesystem::exists(current_dir / hiddendoor)) {
    LOG("ERROR: \"" + hiddendoor + "\" could not be found!", HERE, "HiddenDoor");
    return -1;
}
```

If missing, the tool aborts immediately.

### 4. Execution

HiddenDoor is launched with:

```cpp
Common::executeBatch(command.c_str());
```

This is the only action StackedBooks performs.

## 📂 Expected Directory Layout

StackedBooks expects to be placed in the same folder as HiddenDoor.exe and other TextTools executables

DailyManager and other automation tools rely on this predictable structure.

## Logging

StackedBooks logs only two things:

- A startup message

- Any error related to missing HiddenDoor

It does not log compilation steps — HiddenDoor handles all of that.

## 🚀 Typical Usage

### Command-line:

```
StackedBooks.exe --all
```

### DailyManager:

DailyManager invokes StackedBooks automatically as part of its daily text-update routine.

## 📜 Final Notes

StackedBooks is one of the few DAIX tools that never touches game data.
It is purely an orchestration helper, designed for reliability and simplicity.

If HiddenDoor is the “door” into the text pipeline,
StackedBooks is the hand that opens it.