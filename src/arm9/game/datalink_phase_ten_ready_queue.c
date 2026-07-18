#include "game/datalink_phase_ten_ready_queue.h"

#include <stddef.h>

/* Opaque 0x020ACDD8 transition-stack operation. */
extern void Game_PushDatalinkTransition(
    Game_DatalinkFlowState *state,
    uint32_t transition_address
);

_Static_assert(
    offsetof(Game_DatalinkFlowState, ready_transition_address_38) == 0x38,
    "datalink ready transition address offset"
);

/*
 * Extracted ready-queue unit 0x020A85C4..0x020A85D3 inside the catalogued
 * function FUN_020A8198.  This helper is not a promoted Ghidra function.
 *
 * The target reads the 32-bit transition address at state+0x38 before calling
 * helper 0x020ACDD8 with `(state, address)`.  Its final instruction branches to the
 * shared selection finalizer at 0x020A864C, which remains a separate unit.
 */
void Game_QueueDatalinkPhaseTenReadyTransition(
    Game_DatalinkFlowState *state
)
{
    uint32_t transition_address = state->ready_transition_address_38;

    Game_PushDatalinkTransition(state, transition_address);
}
