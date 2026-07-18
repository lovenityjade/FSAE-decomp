#include "game/datalink_transition_status.h"

#include "game/datalink_pair_position.h"

#include <stdint.h>

enum {
    DATALINK_TRANSITION_COMPLETE_FX = 0x1000
};

/*
 * Helper 0x020ACD74 computes the current 20.12 transition progress for a 0x58-byte
 * object.  Its implementation remains an opaque dependency of this unit.
 */
extern int32_t Game_GetDatalinkObjectTransitionProgressFx(
    const Game_DatalinkPositionObject *object
);

/*
 * 0x020A8178..0x020A8193 (catalog/Ghidra body size: 28 bytes).
 *
 * The literal at 0x020A8194..0x020A8197 is 0x0217EB2C, the address of object
 * zero in the recovered Datalink position collection.  The body calls
 * helper 0x020ACD74 once and returns one exactly when its 20.12 result equals
 * 0x1000.  The next game_candidate begins at 0x020A8198.
 */
bool Game_IsDatalinkPrimaryTransitionComplete(void)
{
    return Game_GetDatalinkObjectTransitionProgressFx(
        &gGameDatalinkPositionObjects[0]
    ) == DATALINK_TRANSITION_COMPLETE_FX;
}
