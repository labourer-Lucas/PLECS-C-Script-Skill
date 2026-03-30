/*
 * PLECS C-Script: Continuous First-Order Low-Pass Filter
 *
 * Description:
 *   Implements a first-order IIR low-pass filter using a continuous ODE state:
 *
 *       tau * dy/dt = u - y
 *       y(0) = y0
 *
 *   The ODE is integrated by the PLECS solver. Set Sample time = 0 for
 *   continuous execution.
 *
 * Block Setup (Setup tab):
 *   Number of inputs:       1       (raw/unfiltered signal)
 *   Number of outputs:      1       (filtered output)
 *   Number of cont. states: 1       (filter state = output value)
 *   Direct feedthrough:     0       (output is the continuous state, not the input)
 *   Sample time:            0       (continuous)
 *   Parameters:
 *     Tau  -- time constant [s] (cutoff freq = 1 / (2*pi*Tau) Hz)
 *     Y0   -- initial output value (default 0)
 *
 * Transfer function equivalent:
 *   H(s) = 1 / (Tau*s + 1)
 *
 * Note:
 *   Direct feedthrough = 0 is safe here because the output Y equals the
 *   continuous state X (not the input U), so there is no instantaneous
 *   input-to-output path.
 */

/* ===== Code declarations ===== */
#define U    InputSignal(0)     /* unfiltered input */
#define Y    OutputSignal(0)    /* filtered output  */
#define X    ContState(0)       /* filter state (= output) */
#define TAU  ParamRealData(0, 0)  /* time constant [s] */
#define Y0   ParamRealData(1, 0)  /* initial value */

/* ===== Start ===== */
X = Y0;

/* ===== Output ===== */
Y = X;

/* ===== Derivative ===== */
/* dx/dt = (u - x) / tau */
ContDeriv(0) = (U - X) / TAU;
