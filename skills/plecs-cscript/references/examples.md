# C-Script Worked Examples

Complete, copy-ready examples for common C-Script patterns. Each example lists the required
block configuration and the code for every relevant section.

---

## Example 1 — Times Two (pure combinatorial)

Multiplies the input by 2: $y = 2u$

**Block setup:**
- Inputs: 1, Outputs: 1, Continuous states: 0, Discrete states: 0, ZC signals: 0
- Direct feedthrough: **yes**
- Sample time: `0`

```c
/* Output Function */
OutputSignal(0, 0) = 2.0 * InputSignal(0, 0);
```

---

## Example 2 — Sampled Delay (discrete state)

Holds the input value and outputs it one sample period later: $y[k] = u[k-1]$

**Block setup:**
- Inputs: 1, Outputs: 1, Discrete states: 1, ZC signals: 0
- Direct feedthrough: **no**
- Sample time: `1` (or any desired period `Tp`)

```c
/* Output Function */
OutputSignal(0, 0) = DiscState(0);

/* Update Function */
DiscState(0) = InputSignal(0, 0);
```

---

## Example 3 — Continuous Integrator

Integrates the input signal: $\dot{x}_c = u$, $y = x_c$

**Block setup:**
- Inputs: 1, Outputs: 1, Continuous states: 1, ZC signals: 0
- Direct feedthrough: **no**
- Sample time: `0`

```c
/* Output Function */
OutputSignal(0, 0) = ContState(0);

/* Derivative Function */
ContDeriv(0) = InputSignal(0, 0);
```

---

## Example 4 — Wrapping Integrator (continuous state + zero-crossings)

Integrates the input and wraps the state within $[0,\, 2\pi]$. Useful for phase accumulators (PLL).

**Block setup:**
- Inputs: 1, Outputs: 1, Continuous states: 1, ZC signals: 2
- Direct feedthrough: **no**
- Sample time: `0`

```c
/* Code Declarations */
#define PI 3.141592653589793

/* Output Function */
if (IsMajorStep)
{
    if (ContState(0) > 2.0 * PI)
        ContState(0) -= 2.0 * PI;
    else if (ContState(0) < 0.0)
        ContState(0) += 2.0 * PI;
}
/* ZC signals must update every step (major and minor) */
ZCSignal(0) = ContState(0);             /* fires when state reaches 0 */
ZCSignal(1) = ContState(0) - 2.0 * PI; /* fires when state reaches 2pi */

OutputSignal(0, 0) = ContState(0);

/* Derivative Function */
ContDeriv(0) = InputSignal(0, 0);
```

---

## Example 5 — Saturation (piecewise smooth + zero-crossings)

Clamps the output to $[-1,\ 1]$:

$$
y =
\begin{cases}
  1  & u \geq 1 \\
  u  & -1 < u < 1 \\
  -1 & u \leq -1
\end{cases}
$$

**Block setup:**
- Inputs: 1, Outputs: 1, States: 0, ZC signals: 2
- Direct feedthrough: **yes**
- Sample time: `0`

```c
/* Code Declarations */
static enum { NO_LIMIT, LOWER_LIMIT, UPPER_LIMIT } mode;

/* Output Function */
if (IsMajorStep)
{
    if      (InputSignal(0, 0) >  1.0) mode = UPPER_LIMIT;
    else if (InputSignal(0, 0) < -1.0) mode = LOWER_LIMIT;
    else                               mode = NO_LIMIT;
}
switch (mode)
{
    case NO_LIMIT:
        OutputSignal(0, 0) = InputSignal(0, 0);
        break;
    case UPPER_LIMIT:
        OutputSignal(0, 0) =  1.0;
        break;
    case LOWER_LIMIT:
        OutputSignal(0, 0) = -1.0;
        break;
}
ZCSignal(0) = InputSignal(0, 0) + 1.0; /* lower boundary */
ZCSignal(1) = InputSignal(0, 0) - 1.0; /* upper boundary */
```

> The `static enum mode` variable freezes the active branch across minor steps so the output
> remains piecewise constant during the integration loop.

---

## Example 6 — Turn-on Delay (multiple sample times + user parameter)

When the input rises from 0 to 1, the output follows after a configurable delay (if input is still
1 at that time). When input falls to 0 the output resets immediately.

**Block setup:**
- Inputs: 1, Outputs: 1, Discrete states: 1 (`PREV_INPUT`), ZC signals: 0
- Direct feedthrough: **yes**
- Sample time: `[0, -1; -2, 0]` (semi-continuous + discrete-variable)
- Parameters: 1 scalar (delay time)

```c
/* Code Declarations */
#include <float.h>
#define PREV_INPUT DiscState(0)
#define DELAY      ParamRealData(0, 0)

/* Start Function */
if (NumParameters != 1)
{
    SetErrorMessage("One parameter required (delay time).");
    return;
}
if (ParamNumDims(0) != 2
    || ParamDim(0, 0) != 1 || ParamDim(0, 1) != 1
    || DELAY <= 0.0)
{
    SetErrorMessage("Delay time must be a positive scalar.");
    return;
}

/* Output Function */
if (InputSignal(0, 0) == 0.0)
{
    OutputSignal(0, 0) = 0.0;
    NextSampleHit = DBL_MAX;
}
else if (PREV_INPUT == 0.0)
{
    /* Rising edge detected — schedule output after DELAY */
    NextSampleHit = CurrentTime + DELAY;
    /* Guard against floating-point cancellation when DELAY << CurrentTime */
    if (NextSampleHit == CurrentTime)
        NextSampleHit = CurrentTime * (1.0 + DBL_EPSILON);
}
else if (IsSampleHit(1))
{
    /* Delay has elapsed and input is still high */
    OutputSignal(0, 0) = 1.0;
    NextSampleHit = DBL_MAX;
}

/* Update Function */
PREV_INPUT = InputSignal(0, 0);

/* Store Custom State */
WriteCustomStateDouble(NextSampleHit);

/* Restore Custom State */
NextSampleHit = ReadCustomStateDouble();
```

> `IsSampleHit(1)` checks sample time row index 1 (the discrete-variable entry).
> `IsSampleHit(0)` would check the semi-continuous entry.

---

## Example 7 — External C Files

Calls `sum()` and `product()` defined in a companion `.c` file located next to the model file.

**Block setup:**
- Inputs: 2, Outputs: 2, States: 0, ZC signals: 0
- Direct feedthrough: **yes**
- Sample time: `0`

**`cscript_example_external_files.h`** (alongside the model):

```c
float sum(float a, float b);
float product(float a, float b);
```

**`cscript_example_external_files.c`** (alongside the model):

```c
#include "cscript_example_external_files.h"

float sum(float a, float b)     { return a + b; }
float product(float a, float b) { return a * b; }
```

```c
/* Code Declarations */
#include "cscript_example_external_files.h"
#include "cscript_example_external_files.c"

#define In1  InputSignal(0, 0)
#define In2  InputSignal(1, 0)
#define Out1 OutputSignal(0, 0)
#define Out2 OutputSignal(1, 0)

/* Output Function */
Out1 = (double)sum((float)In1, (float)In2);
Out2 = (double)product((float)In1, (float)In2);
```

> The model directory is automatically on the include search path, so relative paths work.

---

## Example 8 — Parameterized Gain with Matrix Parameter

A gain block where the gain matrix is passed as a user parameter. Computes $y = K \cdot u$ for a
$m \times n$ matrix $K$, $n$-element input, $m$-element output.

**Block setup:**
- Inputs: 1 (width $n$), Outputs: 1 (width $m$), States: 0
- Direct feedthrough: **yes**
- Sample time: `0`
- Parameters: 1 matrix ($m \times n$)

```c
/* Start Function */
if (NumParameters != 1 || ParamNumDims(0) != 2)
{
    SetErrorMessage("Expected one 2-D matrix parameter.");
    return;
}

/* Output Function */
int m = ParamDim(0, 0); /* rows */
int n = ParamDim(0, 1); /* cols */
for (int i = 0; i < m; ++i)
{
    double acc = 0.0;
    for (int j = 0; j < n; ++j)
        acc += ParamRealData(0, i + m*j) * InputSignal(0, j);
    OutputSignal(0, i) = acc;
}
```
