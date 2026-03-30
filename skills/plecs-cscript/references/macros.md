# C-Script Macro Reference

Full reference for all macros available in PLECS C-Script code sections.
**Access column:** R = read-only, W = write-only, R/W = read and write.

---

## Block Topology

| Macro | Type | Access | Description |
|---|---|:---:|---|
| `NumInputTerminals` | `int` | R | Number of input terminals on the block. |
| `NumOutputTerminals` | `int` | R | Number of output terminals on the block. |
| `NumInputSignals(int i)` | `int` | R | Width (number of elements) of the signal on the `i`th input terminal. |
| `NumOutputSignals(int i)` | `int` | R | Width (number of elements) of the signal on the `i`th output terminal. |
| `NumContStates` | `int` | R | Number of continuous state variables. |
| `NumDiscStates` | `int` | R | Number of discrete state variables. |
| `NumZCSignals` | `int` | R | Number of registered zero-crossing signals. |
| `NumParameters` | `int` | R | Number of user-defined external parameters. |
| `NumSampleTime` | `int` | R | Number of registered sample times. |

---

## Time and Step Information

| Macro | Type | Access | Description |
|---|---|:---:|---|
| `CurrentTime` | `double` | R | Current simulation time. During the start function, this is the simulation start time. |
| `IsMajorStep` | `int` | R | Returns `1` during major time steps, `0` during minor (integration) steps. |
| `IsSampleHit(int i)` | `int` | R | Returns `1` if the `i`th sample time (0-based row in the sample time matrix) has a hit in the current step. |
| `NextSampleHit` | `double` | R/W | Read or set the next simulation time at which the block should execute. Only relevant for discrete-variable sample time (`-2`). Must be set to a value strictly greater than `CurrentTime` before the update function returns. |
| `SampleTimePeriod(int i)` | `double` | R | Period of the `i`th sample time. Returns the resolved value for inherited sample times. |
| `SampleTimeOffset(int i)` | `double` | R | Offset of the `i`th sample time. Returns the resolved value for inherited sample times. |

---

## Signals

| Macro | Type | Access | Description |
|---|---|:---:|---|
| `InputSignal(int i, int j)` | `double` | R | Value of the `j`th element (0-based) of the `i`th input terminal (0-based). Requires **Input has direct feedthrough** to be checked if accessed in the output function. |
| `OutputSignal(int i, int j)` | `double` | R/W | Value of the `j`th element (0-based) of the `i`th output terminal (0-based). **May only be written during the output function call.** Do not use pointer arithmetic — signal values are not contiguous in memory. |
| `ZCSignal(int i)` | `double` | R/W | The `i`th zero-crossing signal (0-based). Assign a continuous expression whose zero indicates an event. The solver monitors sign changes to locate event times. Update in both major and minor steps. |

---

## State Variables

| Macro | Type | Access | Description |
|---|---|:---:|---|
| `ContState(int i)` | `double` | R/W | Value of the `i`th continuous state (0-based). May **not** be written during minor time steps. Initialize in the start function. |
| `ContDeriv(int i)` | `double` | R/W | Time derivative of the `i`th continuous state. Set in the derivative function. Should be smooth during minor steps. |
| `DiscState(int i)` | `double` | R/W | Value of the `i`th discrete state (0-based). May **not** be written during minor time steps. Read in the output function; write in the update function. Initialize in the start function. |

---

## User Parameters

Parameters are entered as a comma-separated MATLAB expression list in the block dialog.
They can be scalars, vectors, matrices, 3-D arrays, or strings.

| Macro | Type | Access | Description |
|---|---|:---:|---|
| `ParamNumDims(int i)` | `int` | R | Number of dimensions of the `i`th parameter (0-based). Strings return `1`. |
| `ParamDim(int i, int j)` | `int` | R | Size of the `j`th dimension of the `i`th parameter. For strings, returns the byte length (UTF-8, may exceed character count). |
| `ParamRealData(int i, int j)` | `double` | R | Value of the `j`th element (linear, column-major index) of the `i`th parameter. Produces a runtime error if the parameter is a string. |
| `ParamStringData(int i)` | `char*` | R | Pointer to a UTF-8 encoded, null-terminated C string for the `i`th parameter. Returns `NULL` (or runtime error) if the parameter is not a string. |

### Indexing into a 3-D parameter array

```c
int nRows = ParamDim(0, 0);
int nCols = ParamDim(0, 1);
// element at (row, col, page) — all 0-based
double val = ParamRealData(0, row + nRows*(col + nCols*page));
```

---

## Custom State Serialization

Used to persist non-standard state across simulation restarts (e.g. `NextSampleHit`, heap-allocated
buffers). Continuous and discrete states are saved automatically.

Called in order: **write** during Store Custom State; **read** during Restore Custom State.
Multiple values are serialized/deserialized sequentially — the read order must match the write order.

| Macro | Type | Access | Description |
|---|---|:---:|---|
| `WriteCustomStateDouble(double val)` | `void` | W | Serialize one `double` value. |
| `WriteCustomStateInt(int val)` | `void` | W | Serialize one `int` value. |
| `WriteCustomStateData(void *data, int len)` | `void` | W | Serialize `len` raw bytes. **Platform-dependent byte order** — avoid for portability. |
| `ReadCustomStateDouble()` | `double` | R | Deserialize one `double` value. |
| `ReadCustomStateInt()` | `int` | R | Deserialize one `int` value. |
| `ReadCustomStateData(void *data, int len)` | `void` | R | Deserialize `len` raw bytes into `data`. |

> **Prefer `double`/`int` over raw data.** `WriteCustomStateData` does not handle endianness.
> To store a vector of doubles portably:
> ```c
> WriteCustomStateInt(n);
> for (int i = 0; i < n; ++i) WriteCustomStateDouble(v[i]);
> ```

---

## Error and Warning Reporting

| Macro | Type | Access | Description |
|---|---|:---:|---|
| `SetErrorMessage(char *msg)` | `void` | W | Report a fatal error. Simulation terminates after the current step. Always follow with `return;`. The `msg` pointer **must point to static memory** (string literal or static array). |
| `SetWarningMessage(char *msg)` | `void` | W | Report a non-fatal warning. Warning is cleared automatically when the C-Script function returns. The `msg` pointer must point to static memory. |

**Example:**

```c
if (DELAY <= 0.)
{
    SetErrorMessage("Delay must be positive.");
    return;
}
```
