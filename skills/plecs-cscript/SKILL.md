---
name: plecs-cscript
description: >
  Write, review, debug, and explain PLECS C-Script code for custom control blocks in PLECS simulations.
  Use this skill whenever the user asks about C-Script, wants to implement a custom block in PLECS,
  needs help with PLECS macros (InputSignal, OutputSignal, ContState, DiscState, ZCSignal, etc.),
  asks about sample time configuration, state variables, zero-crossing detection, user parameters,
  or needs to port controller C code into a PLECS simulation. Trigger even if the user just mentions
  "PLECS block", "custom block", "C-Script", or "cscript".
---

# PLECS C-Script Skill

You are an expert on PLECS C-Script custom control blocks. When this skill is active, generate
correct, well-structured C-Script code that integrates cleanly with the PLECS solver.

For the full macro reference, see [references/macros.md](references/macros.md).
For complete worked examples, see [references/examples.md](references/examples.md).
If the user is editing or generating a `.plecs` file, load [references/plecs-file-format.md](references/plecs-file-format.md) and [references/cscript.plecs](references/cscript.plecs) for the complete file format and a working reference model.(CAUTION: if not required or edited directly, DO NOT GENERATE .plecs files, as they are very verbose and easy to get wrong. Always generate C code snippets for the user to paste into their existing model instead.)

---

# C-Script Architecture

## Block Setup Parameters

These are configured in the **Setup** tab of the C-Script block dialog before writing any code.

### `Number of inputs`

Defines the number and width of input ports.

| Value | Effect |
|---|---|
| `n` (scalar integer) | Single input port accepting a scalar signal |
| `[n1, n2, ...]` (vector) | Multiple input ports; port `i` accepts a signal of width `ni` |
| `-1` | Dynamic sizing: width determined by connected signal |

> **Format note:** In the PLECS dialog both comma-separated (`[2, 3]`) and space-separated (`[2 3]`) are accepted. Inside `.plecs` files the space-separated form is used (e.g. `"[2 3]"`).

Access in code: `InputSignal(portIndex)` or `InputSignal(portIndex, elementIndex)`

### `Number of outputs`

Same rules as `Number of inputs`.

Access in code: `OutputSignal(portIndex)` or `OutputSignal(portIndex, elementIndex)`

### `Number of cont. states`

Integer ≥ 0. Number of continuous state variables integrated by the ODE solver.

Access in code: `ContState(i)`, `ContDeriv(i)`

### `Number of disc. states`

Integer ≥ 0. Number of discrete state variables updated at each sample step.

Access in code: `DiscState(i)`

### `Number of zero-crossings`

Integer ≥ 0. Number of zero-crossing signals monitored by the solver for event detection.

Access in code: `ZCSignal(i)`

### `Direct feedthrough`

Controls whether each input is a direct feedthrough (output depends on input at same instant).

| Value | Meaning |
|---|---|
| `1` | Input is direct feedthrough — can be used in `Output` section |
| `0` | Input is NOT direct feedthrough — must NOT be used in `Output`; use only in `Update` |

Can be a scalar (applies to all inputs) or a vector matching `Number of inputs`.

> **Warning:** Incorrect `Direct feedthrough` settings with feedback loops cause **algebraic loops** and simulation failure.

### `Sample time`

Controls when the C-Script executes.

| Value | Meaning |
|---|---|
| `0` or `[0, 0]` | Continuous: runs at every ODE solver step |
| `[0, -1]` | Semi-continuous: runs at major time steps only |
| `Ts` or `[Ts, 0]` | Discrete: runs every `Ts` seconds |
| `[Ts, To]` | Discrete with offset `To`: first hit at `To`, then every `Ts` |
| `-1` | Inherited: determined by connected signals |
| `-2` or `[-2, 0]` | Variable-rate discrete: script controls its own next execution time via `NextSampleHit` |

### `Use terminal-based sample times`

Checkbox. When enabled, the sample time is no longer set in the **Setup** tab but is instead
inherited from the signals connected to each port individually. Useful when the block needs to
match the sample rate of dynamically connected components.

### `Language standard`

Dropdown (`C89`, `C99`, `C11`, …). Selects the C language standard used by the built-in compiler.
Default is **C99**. Use `C99` or later for `//` line comments, `<stdbool.h>`, `<stdint.h>`,
designated initializers, and variable-length array declarations.

### `Enable GNU extensions`

Checkbox. When enabled, activates GCC/Clang GNU extensions (e.g. `__attribute__`, statement
expressions, nested functions). Avoid unless integrating external code that requires them, as it
may reduce portability.

### `Highlight Level`

Integer ≥ 0. Controls the amount of diagnostic / highlight output shown during simulation for
this block. `0` disables highlighting. Higher values produce more verbose output. Useful for
debugging — set back to `0` for production.

### `Enable runtime checks`

Checkbox (enabled by default). When checked, all macro accesses (signal indices, state indices,
parameter indices) are bounds-checked at runtime, and solver policy violations (e.g. writing
`DiscState` during a minor step) are caught and reported. Disable only after the block is fully
validated, as disabling it turns out-of-bounds accesses into silent undefined behavior that can
crash PLECS.

### `Parameters`

Named parameters passed to the block. Accessible in code via `ParamRealData(i, j)` (real) or
`ParamStringData(i)` (string). Enter as a comma-separated list of MATLAB expressions in the
**Parameters** field; workspace variables are resolved at simulation start.

---

## Code Sections

### Code Declarations

**When called:** Once, before any simulation. Acts as the file header.

**Purpose:** `#include` headers, define macros, declare `static` variables, write helper functions.

**Example:**
```c
#include <math.h>
#include <stdlib.h>

/* Signal aliases */
#define U0   InputSignal(0, 0)
#define Y0   OutputSignal(0, 0)
#define X0   ContState(0)
#define XD0  DiscState(0)

/* Parameter aliases */
#define KP   ParamRealData(0, 0)
#define KI   ParamRealData(1, 0)
#define TS   SampleTimePeriod(0)

static double myInternalVar = 0.0;
```

---

### Start

**When called:** Once at the beginning of simulation (after `Code declarations`).

**Purpose:** Initialize state variables, open files, allocate dynamic memory.

```c
/* Initialize continuous state */
ContState(0) = 0.0;

/* Initialize discrete states */
DiscState(0) = 0.0;
DiscState(1) = 0.0;
```

---

### Output

**When called:** Every simulation time step (or every sample hit for discrete blocks).

**Purpose:** Compute and write output signals. May read inputs (if direct feedthrough) and states.

```c
/* Use previous discrete state (non-direct feedthrough pattern) */
OutputSignal(0, 0) = DiscState(0);

/* Or with direct feedthrough */
OutputSignal(0, 0) = InputSignal(0, 0) * ParamRealData(0, 0);
```

> **Rule:** If `Direct feedthrough = 0` for input `i`, do NOT read `InputSignal(i, j)` in this section.

---

### Update

**When called:** Once per major time step, after `Output`.

**Purpose:** Update discrete states. This is the place to use non-direct-feedthrough inputs.

```c
/* Update integral */
DiscState(0) = DiscState(0) + ParamRealData(1, 0) * SampleTimePeriod(0) * InputSignal(0, 0);
/* Store current input for next Output call */
DiscState(1) = InputSignal(0, 0);
```

---

### Derivative

**When called:** Potentially multiple times per time step by the ODE solver (during sub-steps).

**Purpose:** Compute derivatives of continuous states. Must set `ContDeriv(i)` for every continuous state.

```c
/* First-order lag: tau * dx/dt = u - x */
ContDeriv(0) = (InputSignal(0, 0) - ContState(0)) / ParamRealData(0, 0);
```

> **Warning:** Do NOT modify `DiscState` or output variables here. Guard side-effects with `if (IsMajorStep)`.

---

### Terminate

**When called:** Once at the end of simulation.

**Purpose:** Free dynamically allocated memory, close open files.

```c
free(myBuffer);
fclose(logFile);
```

---

### Store Custom State

**When called:** Once when PLECS saves a simulation snapshot (e.g., before a restart from a saved state).

**Purpose:** Serialize non-standard state that PLECS does not persist automatically. `ContState` and `DiscState` are saved automatically — only use this section for `NextSampleHit`, heap-allocated buffers, or any other custom values.

```c
WriteCustomStateDouble(NextSampleHit);
WriteCustomStateInt(bufferCount);
for (int i = 0; i < bufferCount; ++i)
    WriteCustomStateDouble(buffer[i]);
```

> **Order matters.** The read sequence in `Restore Custom State` must exactly mirror the write sequence here.

---

### Restore Custom State

**When called:** Once when PLECS restores a simulation snapshot.

**Purpose:** Deserialize the values written by `Store Custom State`, in the same order.

```c
NextSampleHit = ReadCustomStateDouble();
bufferCount   = ReadCustomStateInt();
for (int i = 0; i < bufferCount; ++i)
    buffer[i] = ReadCustomStateDouble();
```

---
## Macro Reference

> **Quick reference** — covers the most-used macros. For the complete list including block topology, custom state serialization, and error reporting macros, load [references/macros.md](references/macros.md).

### Input/Output

| Macro | Arguments | Returns | Description |
|---|---|---|---|
| `InputSignal(i, j)` | `i`: port, `j`: element | `double` | `j`-th element at input port `i` |
| `OutputSignal(i, j)` | `i`: port, `j`: element | lvalue | Assignable `j`-th element at output port `i` |

### State Variables

| Macro | Arguments | Mode | Description |
|---|---|---|---|
| `ContState(i)` | `i`: state index (0-based) | read/write | `i`-th continuous state variable |
| `DiscState(i)` | `i`: state index (0-based) | read/write | `i`-th discrete state variable |
| `ContDeriv(i)` | `i`: state index (0-based) | write only | Derivative of `i`-th continuous state (set in `Derivative` section) |
| `ZCSignal(i)` | `i`: signal index (0-based) | write only | `i`-th zero-crossing signal value |

### Time and Stepping

| Macro | Arguments | Returns | Description |
|---|---|---|---|
| `CurrentTime` | — | `double` | Current simulation time `t` |
| `SampleTimePeriod(i)` | `i`: sample time index | `double` | Period `Ts` of `i`-th sample time (usually `i=0`) |
| `SampleTimeOffset(i)` | `i`: sample time index | `double` | Offset of `i`-th sample time |
| `IsSampleHit(i)` | `i`: sample time index | `int` (0/1) | `1` if `i`-th sample time fires at current major step |
| `IsMajorStep` | — | `int` (0/1) | `1` during a major time step; `0` during solver sub-steps |
| `NextSampleHit` | — | lvalue | Set next execution time (only for `Sample time = -2`) |

### Parameters

| Macro | Arguments | Returns | Description |
|---|---|---|---|
| `ParamRealData(i, j)` | `i`: param index, `j`: element | `double` | `j`-th element of `i`-th real parameter |
| `ParamStringData(i)` | `i`: param index | `char*` | Pointer to null-terminated UTF-8 string for `i`-th string parameter |

---

## Execution Timing Summary

```
Simulation start
  └─ Code declarations (compiled once)
  └─ Start

Each major time step:
  └─ Output
  └─ Update          (discrete/semi-continuous only)
  └─ [ODE solver sub-steps]
       └─ Derivative (called repeatedly by solver for continuous states)

Simulation end
  └─ Terminate
```

---

## Sample Time Reference

| `Sample time` value | Block type | `Update` called? | `Derivative` called? |
|---|---|---|---|
| `0` | Continuous | No | Yes |
| `[0, -1]` | Semi-continuous (major steps) | No | Yes |
| `Ts > 0` or `[Ts, 0]` | Discrete | Yes | No |
| `[Ts, To]` | Discrete with offset `To` | Yes | No |
| `-1` | Inherited | Depends | Depends |
| `-2` | Variable-rate discrete | Yes | No |
---

## Structuring Generated Code

When the user asks for a C-Script implementation, always deliver **all required sections** with
clear labels. Use this templates your skeleton and fill in only what each section needs，generate them in a C file:

```c
/* === Setup Parameters ===
   NumInputs, NumOutputs, NumContStates, NumDiscStates, NumZCSignals,
   DirectFeedthrough, Ts, Parameters, LangStandard, etc.            */

/* === Code Declarations === */
/* #includes, #defines, static variables, helper functions */

/* === Start Function === */
/* Parameter validation, state initialization */

/* === Output Function === */
/* Compute outputs; guard state changes with IsMajorStep; update ZCSignals */

/* === Update Function === */
/* Update discrete states */

/* === Derivative Function === */
/* Set ContDeriv(i) */

/* === Terminate Function === */
/* Free resources */

/* === Store Custom State === */
/* WriteCustomStateDouble / Int for non-standard state */

/* === Restore Custom State === */
/* ReadCustomStateDouble / Int */
```

Omit sections that are truly empty (e.g., no Terminate code if nothing to free), but always
include the section label as a comment so the user knows where to paste each snippet in the dialog.

---

## Decision Guide: Which Functions Do I Need?

Work through these questions top-to-bottom to determine the block's required configuration:

1. **Does the output depend on the current input?**
   → Yes: enable **Input has direct feedthrough**

2. **Does the output or derivative change discontinuously based on input/state?**
   → Yes: add **ZCSignal** entries, use continuous sample time `[0,0]`

3. **Do you need state that persists across major steps?**
   → Continuous physics (integrate): use **ContState + ContDeriv**
   → Discrete memory (hold value): use **DiscState + Update Function**

4. **Does the block need to fire at exact user-controlled times?**
   → Use **discrete-variable sample time** (`-2`) + `NextSampleHit`

5. **Does the block need to persist state across simulation restarts?**
   → Implement **Store / Restore Custom State**

---

## Common Patterns

### Pattern A — Pure combinatorial (no state)

```c
// Output Function
OutputSignal(0, 0) = InputSignal(0, 0) * InputSignal(1, 0);
```

Block setup: 1+ inputs, 1+ outputs, 0 states, direct feedthrough, sample time `0`.

### Pattern B — Discrete memory / delay

```c
// Output Function
OutputSignal(0, 0) = DiscState(0);

// Update Function
DiscState(0) = InputSignal(0, 0);
```

Block setup: 1 discrete state, no direct feedthrough, sample time `Tp`.

### Pattern C — Continuous integrator

```c
// Output Function
OutputSignal(0, 0) = ContState(0);

// Derivative Function
ContDeriv(0) = InputSignal(0, 0);
```

Block setup: 1 continuous state, no direct feedthrough, sample time `0`.

### Pattern D — Piecewise / saturation with zero-crossings

```c
// Code Declarations
static enum { NO_LIMIT, LOWER_LIMIT, UPPER_LIMIT } mode;

// Output Function
if (IsMajorStep)
{
    if      (InputSignal(0,0) >  upper) mode = UPPER_LIMIT;
    else if (InputSignal(0,0) < -lower) mode = LOWER_LIMIT;
    else                                mode = NO_LIMIT;
}
switch (mode)
{
    case NO_LIMIT:    OutputSignal(0,0) = InputSignal(0,0); break;
    case UPPER_LIMIT: OutputSignal(0,0) =  upper;           break;
    case LOWER_LIMIT: OutputSignal(0,0) = -lower;           break;
}
ZCSignal(0) = InputSignal(0,0) + lower;
ZCSignal(1) = InputSignal(0,0) - upper;
```

Block setup: 2 ZC signals, no discrete/continuous states, direct feedthrough, sample time `0`.

### Pattern E — Self-triggered (variable sample time)

```c
// Start Function
NextSampleHit = CurrentTime + period;

// Output Function
if (IsSampleHit(0))
{
    // do work
    NextSampleHit = CurrentTime + period;
}
```

Block setup: discrete-variable sample time `-2`.

---

## Accessing Multi-Element Parameters

Parameters are passed as flat arrays in column-major order. To index into a 2-D matrix:

```c
int row = 1, col = 2;
int nRows = ParamDim(0, 0);
double val = ParamRealData(0, row + nRows * col);
```

For strings:

```c
const char *label = ParamStringData(1);   // second parameter
```

---

## Custom State Serialization

Prefer `double` and `int` over raw `void*` (raw data is byte-order dependent):

```c
// Store Custom State
WriteCustomStateInt(count);
for (int i = 0; i < count; ++i)
    WriteCustomStateDouble(buffer[i]);

// Restore Custom State
count = ReadCustomStateInt();
for (int i = 0; i < count; ++i)
    buffer[i] = ReadCustomStateDouble();
```

---

## Reference Files

- **[references/macros.md](references/macros.md)** — Full macro reference table (all macros, types, R/W, descriptions). Load when you need to verify a macro signature or produce a comprehensive summary.
- **[references/examples.md](references/examples.md)** — Complete worked examples (Times Two, Sampled Delay, Integrator, Wrapping Integrator, Saturation, Turn-on Delay, External Files). Load when you need a full end-to-end example to show the user or base new code on.
- **[references/plecs-file-format.md](references/plecs-file-format.md)** — How C-Script blocks are stored in `.plecs` text files (variable names, value encoding, multi-line `\n` escaping, minimal block template). Load when the user is reading, creating, or editing a `.plecs` file directly.
- **[references/cscript.plecs](references/cscript.plecs)** — Minimal but complete working `.plecs` model containing one C-Script block with all parameters at their defaults. Use as the authoritative reference for correct file syntax and default parameter values.
