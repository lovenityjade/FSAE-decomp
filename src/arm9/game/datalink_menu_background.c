#include "game/datalink_menu_controller.h"

#include <stdint.h>

enum {
    DATALINK_MENU_BACKGROUND_SIZE = 0x800,
    DATALINK_MENU_BACKGROUND_OFFSET = 0,
    DATALINK_MENU_PRIMARY_TARGET = 1,
    DATALINK_MENU_SECONDARY_TARGET = 2
};

extern void Game_ClearDatalinkMenuTransferBuffer(
    void *buffer,
    uint32_t size
);
extern void Game_FlushDatalinkRenderRange(
    const void *buffer,
    uint32_t size
);
extern void Game_TransferDatalinkRenderBuffer(
    int target,
    const void *source,
    uint32_t destination_offset,
    uint32_t size
);

/*
 * 0x020AB4C4
 *
 * Complete 88-byte menu-background clear/upload helper through 0x020AB51B.
 * The pointer literal at 0x020AB51C, the one-instruction stub at 0x020AB520,
 * and the next catalogued constructor at 0x020AB524 are excluded. The target
 * reloads the global buffer before every flush/transfer operation.
 */
void Game_ClearDatalinkMenuBackground(
    Game_DatalinkMenuController *unused_controller
)
{
    void *buffer;

    (void)unused_controller;

    buffer = gGameDatalinkMenuTransferBuffer;
    Game_ClearDatalinkMenuTransferBuffer(
        buffer,
        DATALINK_MENU_BACKGROUND_SIZE
    );

    buffer = gGameDatalinkMenuTransferBuffer;
    Game_FlushDatalinkRenderRange(
        buffer,
        DATALINK_MENU_BACKGROUND_SIZE
    );

    buffer = gGameDatalinkMenuTransferBuffer;
    Game_TransferDatalinkRenderBuffer(
        DATALINK_MENU_PRIMARY_TARGET,
        buffer,
        DATALINK_MENU_BACKGROUND_OFFSET,
        DATALINK_MENU_BACKGROUND_SIZE
    );

    buffer = gGameDatalinkMenuTransferBuffer;
    Game_TransferDatalinkRenderBuffer(
        DATALINK_MENU_SECONDARY_TARGET,
        buffer,
        DATALINK_MENU_BACKGROUND_OFFSET,
        DATALINK_MENU_BACKGROUND_SIZE
    );
}
