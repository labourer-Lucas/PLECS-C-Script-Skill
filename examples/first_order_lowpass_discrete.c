/*
 * First-Order Low-Pass Filter — Discrete Time (IIR, Backward Euler)
 * ==================================================================
 * Discretization method:  Backward Euler (Tustin α formula)
 *
 * Continuous prototype:   H(s) = ωc / (s + ωc),  τ = 1/ωc
 * Discrete recurrence:    y[k] = α·u[k] + (1-α)·y[k-1]
 *   where  α = Ts / (Ts + τ) = Ts·ωc / (1 + Ts·ωc)
 *
 * Design notes
 * ─────────────
 * • Backward Euler is unconditionally stable (no oscillation when Ts ≫ τ),
 *   but introduces a slight phase lag compared to the bilinear (Tustin) method.
 * • For accurate analog-equivalent response, choose Ts ≪ τ  (i.e. Ts ≪ 1/ωc).
 * • Direct feedthrough = 1 because y[k] depends on u[k] in the same step.
 *   DiscState(0) carries y[k-1] (the previous output), updated in the Update
 *   section after the Output section has already used it.
 *
 * ── Block Setup ────────────────────────────────────────────────────────────
 *   Number of inputs        : 1
 *   Number of outputs       : 1
 *   Number of cont. states  : 0
 *   Number of disc. states  : 1   (stores previous output y[k-1])
 *   Number of zero-crossings: 0
 *   Direct feedthrough      : 1   (y[k] uses u[k])
 *   Sample time             : Ts  (e.g. 1e-4 for 10 kHz)
 *   Language standard       : C99
 *   Parameters              : fc, Ts
 *                             Parameter 0 — fc : cutoff frequency [Hz]
 *                             Parameter 1 — Ts : sample period [s]
 *                             (Ts must match the Sample time field above)
 * ───────────────────────────────────────────────────────────────────────────
 *
 * NOTE: Ts is passed as an explicit parameter so that α can be precomputed
 * once in Start rather than recomputed every step.  Alternatively you can
 * remove Parameter 1 and compute α using SampleTimePeriod(0) inside Start.
 */


/* === Code Declarations === */

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* Input / output aliases */
#define U       InputSignal(0, 0)   /* u[k]               */
#define Y       OutputSignal(0, 0)  /* y[k]               */

/* Discrete state: previous filtered output y[k-1] */
#define Y_PREV  DiscState(0)

/* Parameter aliases */
#define FC      ParamRealData(0, 0) /* cutoff frequency [Hz] */
#define TS_PARAM ParamRealData(1, 0) /* sample period    [s]  */

/* Precomputed filter coefficient (computed once in Start) */
static double alpha; /* α = Ts·ωc / (1 + Ts·ωc) */


/* === Start Function === */

/*
 * Compute α once at simulation start.
 * α ∈ (0, 1):  α → 0 means very slow filter (heavy smoothing)
 *              α → 1 means filter passes all frequencies (no smoothing)
 *
 * We use the parameter value for Ts here.  If you prefer to rely on the
 * block's own sample time, replace TS_PARAM with SampleTimePeriod(0).
 */
{
    double omega_c = 2.0 * M_PI * FC;
    double ts      = TS_PARAM;
    alpha = (ts * omega_c) / (1.0 + ts * omega_c);
}

Y_PREV = 0.0; /* initialize previous output to zero */


/* === Output Function === */

/*
 * y[k] = α · u[k] + (1 - α) · y[k-1]
 *
 * Direct feedthrough = 1: InputSignal is read here.
 * DiscState holds y[k-1] at this point (updated in the previous Update call).
 */
Y = alpha * U + (1.0 - alpha) * Y_PREV;


/* === Update Function === */

/*
 * Store the output computed this step as the "previous output" for the next
 * sample.  Called once per sample hit, after Output.
 *
 * Note: reading OutputSignal(0,0) here is allowed because Output already ran.
 * Equivalently you could write: Y_PREV = alpha * U + (1.0 - alpha) * Y_PREV;
 */
Y_PREV = Y;
