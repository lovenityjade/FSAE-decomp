#include "game/datalink_phase_sixteen.h"

#include "game/datalink_transition_status.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_PHASE_SIXTEEN_RETURN_PHASE = 1
};

/* Opaque 0x020ACDD8 transition-stack operation on the scene owner. */
extern void Game_PushDatalinkSceneTransition(
    Game_DatalinkSceneOwner *scene,
    uint32_t transition_address
);

_Static_assert(
    offsetof(Game_DatalinkFlowState, unavailable_transition_address_3c) ==
        0x3C,
    "datalink unavailable transition address offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, return_phase_depth_48) == 0x48,
    "datalink return phase depth offset"
);
_Static_assert(
    offsetof(Game_DatalinkFlowState, return_phase_stack_4c) == 0x4C,
    "datalink return phase stack offset"
);
_Static_assert(
    sizeof(((Game_DatalinkFlowState *)0)->return_phase_stack_4c) == 0x20,
    "datalink serialized return phase stack size"
);

/*
 * Semantic phase-sixteen helper for 0x020A86E0..0x020A8713 inside the
 * catalogued FUN_020A8198.  It is not a Ghidra function boundary and must not
 * be promoted in the function registry.
 *
 * Completion of the primary transition gates every effect.  The old +0x48
 * depth is captured and incremented before return phase one is stored at
 * +0x4C[old depth].  Only after that stack write does the target load the
 * transition address at +0x3C and push it on the global scene owner.
 */
void Game_UpdateDatalinkPhaseSixteen(Game_DatalinkFlowState *state)
{
    volatile uint32_t *return_phase_depth = &state->return_phase_depth_48;
    volatile uint32_t *return_phase_stack = state->return_phase_stack_4c;
    volatile uint32_t *transition_address =
        &state->unavailable_transition_address_3c;
    uint32_t old_depth;
    uint32_t queued_address;

    if (!Game_IsDatalinkPrimaryTransitionComplete()) {
        return;
    }

    old_depth = *return_phase_depth;
    *return_phase_depth = old_depth + 1U;
    return_phase_stack[old_depth] = DATALINK_PHASE_SIXTEEN_RETURN_PHASE;
    queued_address = *transition_address;
    Game_PushDatalinkSceneTransition(
        &gGameDatalinkSceneOwner,
        queued_address
    );
}
