#include "game/datalink_phase_fifteen.h"

#include "game/datalink_transition_status.h"

#include <stddef.h>

enum {
    DATALINK_COMPLETION_FLAG_SET = 1,
    DATALINK_SCREEN_REBUILD_RESOURCE = 0x83
};

/* Opaque 0x02053340 screen-rebuild routine. */
extern void Game_RebuildDatalinkScreen(void);

_Static_assert(
    offsetof(Game_DatalinkCompletionOwner, completion_flag_d0) == 0xD0,
    "datalink first completion flag offset"
);
_Static_assert(
    offsetof(Game_DatalinkCompletionOwner, completion_flag_d1) == 0xD1,
    "datalink second completion flag offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, screen_rebuild_resource_53e) == 0x53E,
    "datalink screen rebuild resource offset"
);

/*
 * Semantic phase-fifteen helper for 0x020A86B0..0x020A86DF inside the
 * catalogued FUN_020A8198.  This block is not a Ghidra function boundary and
 * must not be promoted in the function registry.
 *
 * An incomplete primary transition branches directly to the common render
 * tail with no phase-fifteen effects.  Completion sets the two owner flags in
 * address order, selects screen resource 0x83, and only then invokes the
 * no-argument screen rebuild routine at 0x02053340.  Volatile lvalues retain
 * the three distinct ordered byte writes from the ARM body.
 */
void Game_UpdateDatalinkPhaseFifteen(void)
{
    volatile uint8_t *first_completion_flag =
        &gGameDatalinkCompletionOwner.completion_flag_d0;
    volatile uint8_t *second_completion_flag =
        &gGameDatalinkCompletionOwner.completion_flag_d1;
    volatile uint8_t *screen_rebuild_resource =
        &gGameDatalinkSceneOwner.screen_rebuild_resource_53e;

    if (!Game_IsDatalinkPrimaryTransitionComplete()) {
        return;
    }

    *first_completion_flag = DATALINK_COMPLETION_FLAG_SET;
    *second_completion_flag = DATALINK_COMPLETION_FLAG_SET;
    *screen_rebuild_resource = DATALINK_SCREEN_REBUILD_RESOURCE;
    Game_RebuildDatalinkScreen();
}
