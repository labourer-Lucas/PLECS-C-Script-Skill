/*
 * PLECS C-Script: Finite State Machine (FSM) — Converter Startup/Run/Fault
 *
 * Description:
 *   Implements a four-state FSM for managing a power converter's operating
 *   modes: IDLE → STARTUP → RUN → FAULT, with a fault latch that requires
 *   the enable signal to be toggled to clear.
 *
 *   State transitions:
 *     IDLE    --[enable=1]--> STARTUP
 *     STARTUP --[timer >= T_STARTUP]--> RUN
 *     STARTUP --[fault=1]--> FAULT
 *     RUN     --[fault=1]--> FAULT
 *     RUN     --[enable=0]--> IDLE
 *     FAULT   --[enable=0]--> IDLE   (latch cleared by toggling enable)
 *
 * Block Setup (Setup tab):
 *   Number of inputs:      [1, 1]   port 0 = enable (0/1)
 *                                   port 1 = fault  (0/1)
 *   Number of outputs:     [1, 1]   port 0 = run_enable gate signal (0/1)
 *                                   port 1 = state code for monitoring
 *   Number of disc. states: 2       current state, time-in-state counter
 *   Direct feedthrough:    [0, 0]
 *   Sample time:           Ts
 *   Parameters:
 *     T_STARTUP_P  -- startup dwell time [s] before transitioning to RUN
 */

/* ===== Code declarations ===== */
#define ENABLE     InputSignal(0)    /* enable command (>0.5 = on) */
#define FAULT      InputSignal(1)    /* fault signal  (>0.5 = fault active) */

#define GATE_EN    OutputSignal(0)   /* 1 when converter should run */
#define STATE_MON  OutputSignal(1)   /* numeric state code for oscilloscope probe */

#define STATE      DiscState(0)      /* current FSM state (encoded as double) */
#define TIMER      DiscState(1)      /* time spent in current state [s] */

#define TS              SampleTimePeriod(0)
#define T_STARTUP_P     ParamRealData(0, 0)

/* State codes — use floating-point representation for DiscState */
#define ST_IDLE     0.0
#define ST_STARTUP  1.0
#define ST_RUN      2.0
#define ST_FAULT    3.0

/* ===== Start ===== */
STATE = ST_IDLE;
TIMER = 0.0;

/* ===== Output ===== */
/* Gate enable is active only in RUN state */
GATE_EN   = (STATE == ST_RUN) ? 1.0 : 0.0;
STATE_MON = STATE;

/* ===== Update ===== */
{
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
        } else if (TIMER >= T_STARTUP_P) {
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
        /* Fault is latched — clear by toggling enable low */
        if (ENABLE < 0.5) {
            STATE = ST_IDLE;
            TIMER = 0.0;
        }
    }
}
