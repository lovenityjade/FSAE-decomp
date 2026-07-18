#include "game/datalink_mode_dispatch.h"

/*
 * The literal target 0x020A7A60 is deliberately kept opaque here.  It is the
 * shared implementation selected by several small wrappers, not part of the
 * catalogued 0x020A8138 body recovered in this module.
 */
extern void Game_DispatchDatalinkMode(void *state, int mode);

/*
 * 0x020A8138..0x020A8143 (catalog/Ghidra body size: 12 bytes).
 *
 * The wrapper loads the target 0x020A7A60 from its literal at 0x020A8144,
 * forces r1 to zero, and tail-branches through ip.  The next independently
 * catalogued game function starts at 0x020A8148.  No SDK- or artifact-
 * classified intermediary is promoted into this game module.
 */
void Game_EnterDatalinkModeZero(void *state)
{
    Game_DispatchDatalinkMode(state, 0);
}

/*
 * 0x020A8148..0x020A8153 (catalog/Ghidra body size: 12 bytes).
 *
 * This sibling wrapper loads the same opaque target 0x020A7A60 from the
 * literal at 0x020A8154 and tail-branches with r1 forced to one.  The next
 * catalog entry at 0x020A8158 is classified SDK (CARD_LockBackup) and is not
 * recovered here; the next game_candidate starts at 0x020A8168.
 */
void Game_EnterDatalinkModeOne(void *state)
{
    Game_DispatchDatalinkMode(state, 1);
}

/*
 * 0x020A8168..0x020A8173 (catalog/Ghidra body size: 12 bytes).
 *
 * After explicitly skipping the SDK-classified wrapper at 0x020A8158, this
 * game_candidate loads 0x020A7A60 from the literal at 0x020A8174 and
 * tail-branches with r1 forced to three.  Selector two belongs to the skipped
 * SDK entry and is intentionally absent.  The next game_candidate starts at
 * 0x020A8178.
 */
void Game_EnterDatalinkModeThree(void *state)
{
    Game_DispatchDatalinkMode(state, 3);
}
