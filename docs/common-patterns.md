# PLECS C-Script Design Patterns and Best Practices

Common patterns for implementing control algorithms and signal processing in PLECS C-Script blocks.

---

## Pattern 1: Discrete PI Controller

**Use case:** Digital current or voltage control loop with anti-windup.

**Setup:**
- Number of inputs: `1` (error)
- Number of outputs: `1` (control output)
- Number of disc. states: `3` (integral, previous error, clamped flag)
- Direct feedthrough: `0`
- Sample time: `Ts` (your control period, e.g. `1e-4`)
- Parameters: `Kp` (proportional gain), `Ki` (integral gain), `OutMin`, `OutMax`

**Code declarations:**
```c
#define ERROR     InputSignal(0)
#define OUTPUT    OutputSignal(0)
#define INTEGRAL  DiscState(0)
#define PREV_ERR  DiscState(1)
#define CLAMPED   DiscState(2)
#define KP        ParamRealData(0, 0)
#define KI        ParamRealData(1, 0)
#define OUT_MIN   ParamRealData(2, 0)
#define OUT_MAX   ParamRealData(3, 0)
#define TS        SampleTimePeriod(0)
```

**Start:**
```c
INTEGRAL = 0.0;
PREV_ERR = 0.0;
CLAMPED  = 0.0;
```

**Output:**
```c
double raw = KP * PREV_ERR + INTEGRAL;
/* Clamp output */
if (raw > OUT_MAX)      raw = OUT_MAX;
else if (raw < OUT_MIN) raw = OUT_MIN;
OUTPUT = raw;
```

**Update:**
```c
double raw = KP * ERROR + INTEGRAL + KI * TS * ERROR;
int    clamped = (raw > OUT_MAX) || (raw < OUT_MIN);
/* Anti-windup: only integrate when output is not saturated */
if (!clamped) {
    INTEGRAL = INTEGRAL + KI * TS * ERROR;
}
PREV_ERR = ERROR;
CLAMPED  = (double)clamped;
```

---

## Pattern 2: Continuous First-Order Low-Pass Filter

**Use case:** Filtering a noisy sensor signal in a continuous simulation.

**Setup:**
- Number of inputs: `1`
- Number of outputs: `1`
- Number of cont. states: `1`
- Direct feedthrough: `0` (output is the state, not the input)
- Sample time: `0` (continuous)
- Parameters: `Tau` (time constant in seconds)

**Code declarations:**
```c
#define U    InputSignal(0)
#define Y    OutputSignal(0)
#define X    ContState(0)
#define TAU  ParamRealData(0, 0)
```

**Start:**
```c
X = 0.0;
```

**Output:**
```c
Y = X;
```

**Derivative:**
```c
ContDeriv(0) = (U - X) / TAU;
```

---

## Pattern 3: PWM Generation — Phase Accumulator Method

**Use case:** Variable-frequency PWM carrier for closed-loop power converter control.

> The phase accumulator method avoids the frequency-jump discontinuity of `fmod(CurrentTime, T_sw)`.

**Setup:**
- Number of inputs: `2` (`[1, 1]` — frequency command [Hz], duty cycle command [0..1])
- Number of outputs: `1` (PWM gate signal: 0 or 1)
- Number of disc. states: `2` (phase accumulator, stored duty cycle)
- Direct feedthrough: `[0, 0]`
- Sample time: `Ts` (simulation step size, e.g. `1e-6`)

**Code declarations:**
```c
#define FREQ_CMD  InputSignal(0)
#define DUTY_CMD  InputSignal(1)
#define PWM_OUT   OutputSignal(0)
#define PHASE     DiscState(0)
#define DUTY      DiscState(1)
#define TS        SampleTimePeriod(0)
```

**Start:**
```c
PHASE = 0.0;
DUTY  = 0.5;
```

**Output:**
```c
PWM_OUT = (PHASE < DUTY) ? 1.0 : 0.0;
```

**Update:**
```c
double nextPhase = PHASE + FREQ_CMD * TS;
if (nextPhase >= 1.0) nextPhase -= 1.0;
PHASE = nextPhase;
DUTY  = DUTY_CMD;
```

---

## Pattern 4: Three-Phase SVPWM (Space Vector PWM)

**Use case:** Generating three-phase gate signals for a voltage source inverter.

**Setup:**
- Number of inputs: `2` (`[1, 1]` — Vd and Vq in rotating frame)
- Number of outputs: `6` (`[1,1,1,1,1,1]` — gate signals for 6 switches)
- Number of disc. states: `3` (phase accumulator, stored Vd, stored Vq)
- Direct feedthrough: `[0, 0]`
- Sample time: `Ts`
- Parameters: `Vdc` (DC bus voltage), `Fsw` (switching frequency)

**Code declarations:**
```c
#include <math.h>

#define VD_CMD   InputSignal(0)
#define VQ_CMD   InputSignal(1)
#define TS       SampleTimePeriod(0)
#define VDC      ParamRealData(0, 0)
#define FSW      ParamRealData(1, 0)
#define PHASE    DiscState(0)
#define VD_PREV  DiscState(1)
#define VQ_PREV  DiscState(2)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
```

**Start:**
```c
PHASE   = 0.0;
VD_PREV = 0.0;
VQ_PREV = 0.0;
```

**Output:**
```c
double theta = PHASE * 2.0 * M_PI;
/* Park inverse: rotating → stationary */
double Valpha = VD_PREV * cos(theta) - VQ_PREV * sin(theta);
double Vbeta  = VD_PREV * sin(theta) + VQ_PREV * cos(theta);

/* Clark inverse: stationary → three-phase */
double Va = Valpha;
double Vb = -0.5 * Valpha + (sqrt(3.0) / 2.0) * Vbeta;
double Vc = -0.5 * Valpha - (sqrt(3.0) / 2.0) * Vbeta;

/* Duty cycles (normalized to [0,1]) */
double da = 0.5 + Va / VDC;
double db = 0.5 + Vb / VDC;
double dc = 0.5 + Vc / VDC;

/* Clamp duty cycles */
if (da > 1.0) da = 1.0; else if (da < 0.0) da = 0.0;
if (db > 1.0) db = 1.0; else if (db < 0.0) db = 0.0;
if (dc > 1.0) dc = 1.0; else if (dc < 0.0) dc = 0.0;

/* Compare against triangular carrier (phase in [0,1]) */
double carrier = PHASE * FSW * TS;  /* triangular approximation */
double tri = fmod(PHASE, 1.0 / FSW) * FSW;  /* 0..1 sawtooth */

OutputSignal(0) = (da > tri) ? 1.0 : 0.0;  /* S1 */
OutputSignal(1) = (da > tri) ? 0.0 : 1.0;  /* S4 */
OutputSignal(2) = (db > tri) ? 1.0 : 0.0;  /* S3 */
OutputSignal(3) = (db > tri) ? 0.0 : 1.0;  /* S6 */
OutputSignal(4) = (dc > tri) ? 1.0 : 0.0;  /* S5 */
OutputSignal(5) = (dc > tri) ? 0.0 : 1.0;  /* S2 */
```

**Update:**
```c
double nextPhase = PHASE + FSW * TS;
if (nextPhase >= 1.0) nextPhase -= 1.0;
PHASE   = nextPhase;
VD_PREV = VD_CMD;
VQ_PREV = VQ_CMD;
```

---

## Pattern 5: Finite State Machine

**Use case:** Mode sequencing, startup/shutdown logic, fault management.

**Setup:**
- Number of inputs: `2` (`[1, 1]` — enable signal, fault signal)
- Number of outputs: `1` (state output for monitoring)
- Number of disc. states: `2` (current state, timer)
- Direct feedthrough: `[0, 0]`
- Sample time: `Ts`

**Code declarations:**
```c
#define ENABLE    InputSignal(0)
#define FAULT     InputSignal(1)
#define STATE_OUT OutputSignal(0)
#define STATE     DiscState(0)
#define TIMER     DiscState(1)
#define TS        SampleTimePeriod(0)

/* State definitions */
#define ST_IDLE    0.0
#define ST_STARTUP 1.0
#define ST_RUN     2.0
#define ST_FAULT   3.0

/* Timeout for startup: 0.1 s */
#define T_STARTUP  0.1
```

**Start:**
```c
STATE = ST_IDLE;
TIMER = 0.0;
```

**Output:**
```c
STATE_OUT = STATE;
```

**Update:**
```c
TIMER += TS;
if (STATE == ST_IDLE) {
    if (ENABLE > 0.5) {
        STATE = ST_STARTUP;
        TIMER = 0.0;
    }
} else if (STATE == ST_STARTUP) {
    if (FAULT > 0.5) {
        STATE = ST_FAULT;
        TIMER = 0.0;
    } else if (TIMER >= T_STARTUP) {
        STATE = ST_RUN;
        TIMER = 0.0;
    }
} else if (STATE == ST_RUN) {
    if (FAULT > 0.5) {
        STATE = ST_FAULT;
        TIMER = 0.0;
    } else if (ENABLE < 0.5) {
        STATE = ST_IDLE;
        TIMER = 0.0;
    }
} else if (STATE == ST_FAULT) {
    /* Latch: require manual reset (ENABLE goes low then high) */
    if (ENABLE < 0.5) {
        STATE = ST_IDLE;
        TIMER = 0.0;
    }
}
```

---

## Pattern 6: Lookup Table (1-D Linear Interpolation)

**Use case:** Implementing a piecewise-linear gain schedule or efficiency map.

**Setup:**
- Number of inputs: `1`
- Number of outputs: `1`
- Number of disc. states: `0`
- Direct feedthrough: `1`
- Sample time: `Ts`

**Code declarations:**
```c
#include <string.h>

#define U  InputSignal(0)
#define Y  OutputSignal(0)

/* Breakpoints and values (edit as needed) */
static const double lut_x[] = {0.0, 0.25, 0.5, 0.75, 1.0};
static const double lut_y[] = {0.0, 0.30, 0.65, 0.85, 1.0};
#define LUT_N  (sizeof(lut_x) / sizeof(lut_x[0]))

static double lut_interp(double xq) {
    int n = (int)LUT_N;
    if (xq <= lut_x[0])     return lut_y[0];
    if (xq >= lut_x[n - 1]) return lut_y[n - 1];
    int i;
    for (i = 1; i < n; i++) {
        if (xq <= lut_x[i]) {
            double t = (xq - lut_x[i - 1]) / (lut_x[i] - lut_x[i - 1]);
            return lut_y[i - 1] + t * (lut_y[i] - lut_y[i - 1]);
        }
    }
    return lut_y[n - 1];
}
```

**Output:**
```c
Y = lut_interp(U);
```

---

## Best Practices

### 1. Always use `#define` aliases in Code Declarations

```c
/* Instead of this everywhere: */
OutputSignal(0) = ParamRealData(0,0) * ContState(0) + InputSignal(1);

/* Use aliases: */
#define Y     OutputSignal(0)
#define KP    ParamRealData(0, 0)
#define X     ContState(0)
#define ERROR InputSignal(1)

Y = KP * X + ERROR;
```

### 2. Understand when to use `DiscState` vs. `static` variables

| Use `DiscState` when | Use `static` variables when |
|---|---|
| The value needs to be logged/probed in PLECS | The value is purely internal/temporary |
| The value must reset cleanly on re-simulation | Performance-critical inner loop |
| The value is part of the control state | Helper data that never changes (constants, LUTs) |

### 3. Preventing algebraic loops

If your block's output connects back to its own input (directly or through other direct-feedthrough blocks):
- Set `Direct feedthrough = 0` for those inputs
- Move input-reading code from `Output` to `Update`
- Store computed values in `DiscState` and read them back in `Output`

### 4. Guard side-effects in `Derivative`

```c
/* Derivative section: called multiple times per step by ODE solver */
ContDeriv(0) = (InputSignal(0) - ContState(0)) / ParamRealData(0, 0);

/* BAD: modifying discrete state inside Derivative */
/* DiscState(0) = ContState(0);  ← DO NOT do this */

/* GOOD: guard with IsMajorStep if needed */
if (IsMajorStep) {
    DiscState(0) = ContState(0);
}
```

### 5. Sample time and step size relationship

For a discrete block with `Sample time = Ts`:
- Fixed-step solver: `Ts` must be an integer multiple of the fixed step size `h`.
- Variable-step solver: the solver automatically hits at every `Ts`.
- For PWM generation: the step size `h` must satisfy `h ≤ Ts/2` to resolve the PWM edges.

### 6. Resetting integrators

```c
/* In Update section: conditional integrator reset */
if (InputSignal(1) > 0.5) {  /* reset input on port 1 */
    DiscState(0) = 0.0;       /* reset integral */
} else {
    DiscState(0) = DiscState(0) + KI * TS * InputSignal(0);
}
```
