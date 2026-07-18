#include "game/link_operation_state.h"

/* 0x020b2a90..0x020b2aa7 */
bool Game_LinkOperation_HasSucceeded_020b2a90(void)
{
    return gGameLinkOperationState_0212c9a8.result ==
        GAME_LINK_OPERATION_SUCCEEDED;
}

/* 0x020b2aac..0x020b2aaf */
void Game_LinkOperation_NoOp_020b2aac(void)
{
}
