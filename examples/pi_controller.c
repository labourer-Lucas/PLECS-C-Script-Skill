/*
 * PLECS C-Script: Discrete PI Controller with Anti-Windup
 *
 * Description:
 *   Implements a discrete-time PI controller with output clamping and
 *   integrator anti-windup. Uses a non-direct-feedthrough structure so
 *   the block can be placed inside a feedback loop without causing an
 *   algebraic loop error.
 *
 * Block Setup (Setup tab):
 *   Number of inputs:      1       (error signal e = reference - feedback)
 *   Number of outputs:     1       (control output u)
 *   Number of disc. states: 3      (integral accumulator, previous error,
 *                                   saturation flag)
 *   Direct feedthrough:    0
 *   Sample time:           Ts      (e.g. 1e-4 for 10 kHz control loop)
 *   Parameters:
 *     Kp    -- proportional gain
 *     Ki    -- integral gain  (units: 1/s; multiply by Ts in code)
 *     OutMin -- lower output clamp
 *     OutMax -- upper output clamp
 *
 * Execution order each step:
 *   Output  -> reads PREV_ERR and INTEGRAL from previous step
 *   Update  -> updates INTEGRAL and PREV_ERR with current ERROR input
 */

/* ===== Code declarations ===== */
#define ERROR     InputSignal(0)
#define OUTPUT    OutputSignal(0)

#define INTEGRAL  DiscState(0)   /* Ki*Ts accumulator */
#define PREV_ERR  DiscState(1)   /* error at previous Update call */
#define SAT_FLAG  DiscState(2)   /* 1.0 if output was saturated last step */

#define KP        ParamRealData(0, 0)
#define KI        ParamRealData(1, 0)
#define OUT_MIN   ParamRealData(2, 0)
#define OUT_MAX   ParamRealData(3, 0)
#define TS        SampleTimePeriod(0)

/* ===== Start ===== */
INTEGRAL = 0.0;
PREV_ERR = 0.0;
SAT_FLAG = 0.0;

/* ===== Output ===== */
{
    double raw = KP * PREV_ERR + INTEGRAL;
    if      (raw > OUT_MAX) { raw = OUT_MAX; }
    else if (raw < OUT_MIN) { raw = OUT_MIN; }
    OUTPUT = raw;
}

/* ===== Update ===== */
{
    /* Compute unsaturated output using current error */
    double raw = KP * ERROR + INTEGRAL + KI * TS * ERROR;

    /* Anti-windup: only update integral when output would not saturate */
    int saturated = (raw > OUT_MAX) || (raw < OUT_MIN);
    if (!saturated) {
        INTEGRAL = INTEGRAL + KI * TS * ERROR;
    }

    PREV_ERR = ERROR;
    SAT_FLAG = (double)saturated;
}
