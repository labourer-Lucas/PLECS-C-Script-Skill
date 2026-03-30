# PLECS File Format Reference — C-Script Blocks

This document describes how `.plecs` files store C-Script block configuration so you can
read, generate, or programmatically edit them correctly.

> **Reference model:** [cscript.plecs](cscript.plecs) is a minimal but complete working model containing one C-Script block with all parameters at their defaults. When in doubt about correct syntax or field values, compare against that file.

---

## Overall File Structure

A `.plecs` file is a **plain-text, hierarchical key-value format** (proprietary to Plexim).
It is NOT XML, JSON, or YAML. The grammar is:

```
BlockType {
  KeyName   "value"
  NestedBlock {
    ...
  }
}
```

**Rules:**
- Block delimiters are `{` and `}` on separate lines.
- Every scalar value is a **quoted string**, even numbers and booleans.
- Indentation is 2 spaces per level (cosmetic only — not significant).
- Boolean/enum fields use integer strings: `"1"`, `"2"`, `"3"` (see per-field tables below).
- Multi-line code is stored as a **single quoted string with `\n` as literal newline escapes**.

Top-level structure:

```
Plecs {
  Name     "model_name"
  Version  "5.0"
  ...                         ← simulation settings
  Schematic {
    ...
    Component { ... }         ← one block per component
    Component { ... }
    Line { ... }              ← signal connections
  }
}
```

---

## Component Block — C-Script

A C-Script block is represented as a `Component` block with `Type CScript`:

```
Component {
  Type          CScript
  Name          "C-Script"        ← display name in schematic
  Show          on                ← show name label
  Position      [150, 80]         ← [x, y] pixel position on schematic
  Direction     up                ← port orientation: up | down | left | right
  Flipped       off
  LabelPosition south             ← label position relative to block
  Parameter { ... }               ← one Parameter block per setting
  Parameter { ... }
}
```

Each `Parameter` block has three keys:

```
Parameter {
  Variable  "VariableName"    ← internal name (see table below)
  Value     "value"           ← encoded value (see table below)
  Show      off               ← whether the value is shown on schematic
}
```

---

## C-Script Parameter Reference

### Setup Tab Parameters

| `Variable` | Dialog Label | Type | Value Encoding |
|---|---|---|---|
| `NumInputs` | Number of inputs | expression | `"1"` = scalar, `"[2 3]"` = two ports of widths 2 and 3 |
| `NumOutputs` | Number of outputs | expression | same as `NumInputs` |
| `NumContStates` | Number of cont. states | integer | `"0"`, `"1"`, … |
| `NumDiscStates` | Number of disc. states | integer | `"0"`, `"1"`, … |
| `NumZCSignals` | Number of zero-crossings | integer | `"0"`, `"1"`, … |
| `DirectFeedthrough` | Direct feedthrough | expression | `"1"` = yes (all), `"0"` = no (all), `"[1 0]"` = per-port vector |
| `Ts` | Sample time (s) | expression | `"0"` = continuous, `"1e-3"` = 1 ms discrete, `"[0 -1]"` = semi-continuous, `"-2"` = variable |
| `TerminalBasedSampleTimes` | Use terminal-based sample times | checkbox | `"1"` = unchecked (off), `"2"` = checked (on) |
| `Parameters` | Parameters | string | Comma-separated MATLAB expressions: `"K, tau"` or `""` for none |
| `LangStandard` | Language standard | dropdown | `"1"` = C89, `"2"` = C99, `"3"` = C11 |
| `GnuExtensions` | Enable GNU extensions | checkbox | `"1"` = unchecked (off), `"2"` = checked (on) |
| `HighlightLevel` | Highlight Level | integer | `"0"` = off, higher = more verbose |
| `RuntimeCheck` | Enable runtime checks | checkbox | `"1"` = unchecked (off), `"2"` = checked (on) |

> **Default values** (as seen in a freshly created C-Script block):
> `NumInputs = "1"`, `NumOutputs = "1"`, `NumContStates = "0"`, `NumDiscStates = "0"`,
> `NumZCSignals = "0"`, `DirectFeedthrough = "1"`, `Ts = "0"`,
> `TerminalBasedSampleTimes = "1"`, `Parameters = ""`,
> `LangStandard = "2"` (C99), `GnuExtensions = "1"`, `HighlightLevel = "0"`,
> `RuntimeCheck = "2"` (enabled).

---

### Code Section Parameters

Each code section in the **Code** tab maps to one `Parameter` variable.

| `Variable` | Code Tab Section | Default placeholder |
|---|---|---|
| `Declarations` | Code Declarations | `"//Code declarations here"` |
| `StartFcn` | Start Function | `"//Start function here"` |
| `OutputFcn` | Output Function | `"//Output function here"` |
| `UpdateFcn` | Update Function | `"//Update function here"` |
| `DerivativeFcn` | Derivative Function | `"//Derivative function here"` |
| `TerminateFcn` | Terminate Function | `"//Terminate function here"` |
| `StoreCustomStateFcn` | Store Custom State | `"//Store custom state function here"` |
| `RestoreCustomStateFcn` | Restore Custom State | `"//Restore custom state function here"` |

---

## Multi-line Code Encoding

Code with multiple lines is stored as a **single string with `\n`** (backslash + n) representing
newlines inside the quoted value. Indentation uses spaces or `\t` (backslash + t).

**Example — a discrete integrator output + update section:**

```
Parameter {
  Variable  "OutputFcn"
  Value     "OutputSignal(0, 0) = DiscState(0);"
  Show      off
}
Parameter {
  Variable  "UpdateFcn"
  Value     "DiscState(0) = InputSignal(0, 0);"
  Show      off
}
```

**Example — multi-line code with `\n`:**

```
Parameter {
  Variable  "OutputFcn"
  Value     "if (IsMajorStep)\n{\n  if (InputSignal(0,0) >= 1.)\n    OutputSignal(0,0) = 1.;\n  else\n    OutputSignal(0,0) = 0.;\n}\nZCSignal(0) = InputSignal(0,0) - 1.;"
  Show      off
}
```

When generating or editing `.plecs` files programmatically, always:
1. Replace every newline `\n` in the code string with the literal two-character sequence `\n`.
2. Escape any existing backslashes (`\` → `\\`) and double quotes (`"` → `\"`) in the code before embedding.

---

## Internal-Only Parameter

| `Variable` | Purpose |
|---|---|
| `DialogGeometry` | Stores last window position/size of the C-Script dialog. Safe to omit or set to `""` when generating files. |

---

## Minimal C-Script Block Template

The smallest valid C-Script block (one scalar input → one scalar output, pure feedthrough):

```
Component {
  Type          CScript
  Name          "MyBlock"
  Show          on
  Position      [200, 100]
  Direction     up
  Flipped       off
  LabelPosition south
  Parameter {
    Variable      "NumInputs"
    Value         "1"
    Show          off
  }
  Parameter {
    Variable      "NumOutputs"
    Value         "1"
    Show          off
  }
  Parameter {
    Variable      "NumContStates"
    Value         "0"
    Show          off
  }
  Parameter {
    Variable      "NumDiscStates"
    Value         "0"
    Show          off
  }
  Parameter {
    Variable      "NumZCSignals"
    Value         "0"
    Show          off
  }
  Parameter {
    Variable      "DirectFeedthrough"
    Value         "1"
    Show          off
  }
  Parameter {
    Variable      "Ts"
    Value         "0"
    Show          off
  }
  Parameter {
    Variable      "TerminalBasedSampleTimes"
    Value         "1"
    Show          off
  }
  Parameter {
    Variable      "Parameters"
    Value         ""
    Show          off
  }
  Parameter {
    Variable      "LangStandard"
    Value         "2"
    Show          off
  }
  Parameter {
    Variable      "GnuExtensions"
    Value         "1"
    Show          off
  }
  Parameter {
    Variable      "RuntimeCheck"
    Value         "2"
    Show          off
  }
  Parameter {
    Variable      "HighlightLevel"
    Value         "0"
    Show          off
  }
  Parameter {
    Variable      "Declarations"
    Value         ""
    Show          off
  }
  Parameter {
    Variable      "StartFcn"
    Value         ""
    Show          off
  }
  Parameter {
    Variable      "OutputFcn"
    Value         "OutputSignal(0, 0) = 2.0 * InputSignal(0, 0);"
    Show          off
  }
  Parameter {
    Variable      "UpdateFcn"
    Value         ""
    Show          off
  }
  Parameter {
    Variable      "DerivativeFcn"
    Value         ""
    Show          off
  }
  Parameter {
    Variable      "TerminateFcn"
    Value         ""
    Show          off
  }
  Parameter {
    Variable      "StoreCustomStateFcn"
    Value         ""
    Show          off
  }
  Parameter {
    Variable      "RestoreCustomStateFcn"
    Value         ""
    Show          off
  }
}
```

---

## Common Editing Tasks

### Change sample time to 1 ms discrete

```
Parameter {
  Variable  "Ts"
  Value     "1e-3"
  Show      off
}
```

### Add two continuous states and enable derivative function

```
Parameter {
  Variable  "NumContStates"
  Value     "2"
  Show      off
}
Parameter {
  Variable  "DerivativeFcn"
  Value     "ContDeriv(0) = InputSignal(0, 0);\nContDeriv(1) = ContState(0);"
  Show      off
}
```

### Pass a gain parameter named `K`

```
Parameter {
  Variable  "Parameters"
  Value     "K"
  Show      off
}
```

Then in code: `ParamRealData(0, 0)` reads the value of `K`.

### Enable GNU extensions and switch to C11

```
Parameter {
  Variable  "LangStandard"
  Value     "3"
  Show      off
}
Parameter {
  Variable  "GnuExtensions"
  Value     "2"
  Show      off
}
```
