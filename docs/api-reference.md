# PLECS C-Script API Reference

Complete reference for all macros, parameters, and code sections available in a PLECS C-Script block.

---

## Block Setup Parameters

These are configured in the **Setup** tab of the C-Script block dialog before writing any code.

### `Number of inputs`

Defines the number and width of input ports.

| Value | Effect |
|---|---|
| `n` (scalar integer) | Single input port accepting a scalar signal |
| `[n1, n2, ...]` (vector) | Multiple input ports; port `i` accepts a signal of width `ni` |
| `-1` | Dynamic sizing: width determined by connected signal |

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

### `Parameters`

Named parameters passed to the block. Accessible in code via `ParamRealData(i, j)` (real) or `ParamIntData(i)` (integer).

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
#define U0   InputSignal(0)
#define Y0   OutputSignal(0)
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
OutputSignal(0) = DiscState(0);

/* Or with direct feedthrough */
OutputSignal(0) = InputSignal(0) * ParamRealData(0, 0);
```

> **Rule:** If `Direct feedthrough = 0` for input `i`, do NOT read `InputSignal(i)` in this section.

---

### Update

**When called:** Once per major time step, after `Output`.

**Purpose:** Update discrete states. This is the place to use non-direct-feedthrough inputs.

```c
/* Update integral */
DiscState(0) = DiscState(0) + ParamRealData(1, 0) * SampleTimePeriod(0) * InputSignal(0);
/* Store current input for next Output call */
DiscState(1) = InputSignal(0);
```

---

### Derivative

**When called:** Potentially multiple times per time step by the ODE solver (during sub-steps).

**Purpose:** Compute derivatives of continuous states. Must set `ContDeriv(i)` for every continuous state.

```c
/* First-order lag: tau * dx/dt = u - x */
ContDeriv(0) = (InputSignal(0) - ContState(0)) / ParamRealData(0, 0);
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

## Macro Reference

### Input/Output

| Macro | Arguments | Returns | Description |
|---|---|---|---|
| `InputSignal(i)` | `i`: port index (0-based) | `double` | Value at input port `i` (scalar or first element) |
| `InputSignal(i, j)` | `i`: port, `j`: element | `double` | `j`-th element at input port `i` |
| `OutputSignal(i)` | `i`: port index | lvalue | Assignable output at port `i` |
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
| `SampleHit(i)` | `i`: sample time index | `int` (0/1) | `1` if `i`-th sample time fires at current major step |
| `IsMajorStep` | — | `int` (0/1) | `1` during a major time step; `0` during solver sub-steps |
| `NextSampleHit` | — | lvalue | Set next execution time (only for `Sample time = -2`) |

### Parameters

| Macro | Arguments | Returns | Description |
|---|---|---|---|
| `ParamRealData(i, j)` | `i`: param index, `j`: element | `double` | `j`-th element of `i`-th real parameter |
| `ParamIntData(i)` | `i`: param index | `int` | Value of `i`-th integer parameter |

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
| `Ts > 0` | Discrete | Yes | No |
| `-1` | Inherited | Depends | Depends |
| `-2` | Variable-rate discrete | Yes | No |
