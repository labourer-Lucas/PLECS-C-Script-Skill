/*
 * PLECS C-Script: Variable-Frequency PWM Generator (Phase Accumulator)
 *
 * Description:
 *   Generates a single-phase PWM output signal with independently
 *   controllable frequency and duty cycle using the phase accumulator
 *   technique. Unlike the fmod(CurrentTime, T_sw) approach, the phase
 *   accumulator method avoids discontinuous phase jumps when the frequency
 *   command changes mid-simulation, making it suitable for closed-loop
 *   frequency control.
 *
 * Block Setup (Setup tab):
 *   Number of inputs:       [1, 1]   port 0 = frequency cmd [Hz]
 *                                    port 1 = duty cycle cmd [0..1]
 *   Number of outputs:      1        PWM output (0 or 1)
 *   Number of disc. states: 2        phase accumulator, stored duty cycle
 *   Direct feedthrough:     [0, 0]   non-direct feedthrough (avoids algebraic loop)
 *   Sample time:            Ts       simulation step size (e.g. 1e-6 s)
 *   Parameters:             (none required; Fsw and D can be wired as inputs)
 *
 * Algorithm:
 *   Each step, the phase is advanced by Fsw * Ts.
 *   When phase >= 1.0, it wraps back by subtracting 1.0 (keeps fractional part).
 *   Output = 1 when phase < duty, 0 otherwise.
 *
 * Caution:
 *   The simulation step size Ts must be small enough to resolve the PWM
 *   rising/falling edges. Rule of thumb: Ts <= 1 / (20 * Fsw_max).
 */

/* ===== Code declarations ===== */
#define FREQ_CMD   InputSignal(0)   /* switching frequency command [Hz] */
#define DUTY_CMD   InputSignal(1)   /* duty cycle command [0..1] */
#define PWM_OUT    OutputSignal(0)  /* PWM gate signal (0 or 1) */

#define PHASE      DiscState(0)     /* phase accumulator in [0, 1) */
#define DUTY       DiscState(1)     /* duty cycle latched from previous Update */

#define TS         SampleTimePeriod(0)

/* ===== Start ===== */
PHASE = 0.0;
DUTY  = 0.5;   /* default 50 % duty cycle at start */

/* ===== Output ===== */
/* Compare stored phase against stored duty to produce gate signal.
 * Both PHASE and DUTY were written by the previous Update call, so
 * there is no direct feedthrough of the inputs to the output. */
PWM_OUT = (PHASE < DUTY) ? 1.0 : 0.0;

/* ===== Update ===== */
{
    /* Advance phase by one step */
    double nextPhase = PHASE + FREQ_CMD * TS;
    /* Wrap: keep phase in [0, 1) */
    if (nextPhase >= 1.0) {
        nextPhase -= 1.0;
    }
    PHASE = nextPhase;

    /* Latch new duty cycle; change takes effect at the next carrier period
     * if you want synchronous updates, or immediately here for simplicity */
    double d = DUTY_CMD;
    if (d > 1.0) d = 1.0;
    if (d < 0.0) d = 0.0;
    DUTY = d;
}
