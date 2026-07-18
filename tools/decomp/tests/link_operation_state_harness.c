#include "game/link_operation_state.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

GameLinkOperationState gGameLinkOperationState_0212c9a8;

int main(void)
{
    GameLinkOperationState before;
    const int32_t non_success_results[] = {0, -1, 2, 5};
    size_t index;

    memset(&gGameLinkOperationState_0212c9a8, 0xa5,
           sizeof(gGameLinkOperationState_0212c9a8));
    for (index = 0;
         index < sizeof(non_success_results) / sizeof(non_success_results[0]);
         ++index) {
        gGameLinkOperationState_0212c9a8.result = non_success_results[index];
        assert(!Game_LinkOperation_HasSucceeded_020b2a90());
    }

    gGameLinkOperationState_0212c9a8.result = GAME_LINK_OPERATION_SUCCEEDED;
    assert(Game_LinkOperation_HasSucceeded_020b2a90());

    before = gGameLinkOperationState_0212c9a8;
    Game_LinkOperation_NoOp_020b2aac();
    assert(memcmp(&before, &gGameLinkOperationState_0212c9a8,
                  sizeof(before)) == 0);
    return 0;
}
