#include "game/datalink_extended_menu_controller.h"

#include "game/datalink_controller.h"
#include "game/datalink_phase_ten_finalizer.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_EXTENDED_MENU_ROW_COUNT = 2,
    DATALINK_EXTENDED_MENU_VARIANT_COUNT = 2,
    DATALINK_EXTENDED_MENU_HORIZONTAL_DISTANCE = 256,
    DATALINK_EXTENDED_MENU_AUXILIARY_Y_OFFSET = 24,
    DATALINK_EXTENDED_MENU_TWEEN_DURATION = 20,
    DATALINK_EXTENDED_MENU_TWEEN_MODE_OUT = 1,
    DATALINK_EXTENDED_MENU_TWEEN_MODE_IN = 2,
    DATALINK_EXTENDED_MENU_AUXILIARY_TWEEN = 23,
    DATALINK_FX_ONE = 0x1000
};

typedef struct Game_DatalinkExtendedMenuPosition {
    int32_t x;
    int32_t y;
} Game_DatalinkExtendedMenuPosition;

static const uint8_t
    sDatalinkExtendedMenuTweenIndices[DATALINK_EXTENDED_MENU_ROW_COUNT]
                                      [DATALINK_EXTENDED_MENU_VARIANT_COUNT] = {
        {26, 27},
        {28, 29}
    };

static const Game_DatalinkExtendedMenuPosition
    sDatalinkExtendedMenuPositions[DATALINK_EXTENDED_MENU_ROW_COUNT] = {
        {128, 64},
        {128, 120}
    };

static const Game_DatalinkExtendedMenuPosition
    sDatalinkExtendedMenuAuxiliaryPosition = {24, 180};

extern void Game_ConfigureDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t target_x_fx,
    int32_t target_y_fx,
    uint32_t duration,
    int mode
);

_Static_assert(
    offsetof(
        Game_DatalinkSceneOwner,
        extended_menu_item_0_duration_d54
    ) == 0xD54,
    "extended menu item zero duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, extended_menu_item_0_x_fx_d5c) ==
        0xD5C,
    "extended menu item zero X offset"
);
_Static_assert(
    offsetof(
        Game_DatalinkSceneOwner,
        extended_menu_item_1_duration_dac
    ) == 0xDAC,
    "extended menu item one duration offset"
);
_Static_assert(
    offsetof(
        Game_DatalinkSceneOwner,
        extended_menu_item_2_duration_e04
    ) == 0xE04,
    "extended menu item two duration offset"
);
_Static_assert(
    offsetof(
        Game_DatalinkSceneOwner,
        extended_menu_item_3_duration_e5c
    ) == 0xE5C,
    "extended menu item three duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, extended_menu_item_3_y_fx_e68) ==
        0xE68,
    "extended menu item three Y offset"
);

static int32_t Game_DatalinkExtendedMenuCoordinateToFx(int32_t coordinate)
{
    return coordinate * DATALINK_FX_ONE;
}

static void Game_SeedDatalinkExtendedMenuScene(int horizontal_offset)
{
    int32_t first_x_fx = Game_DatalinkExtendedMenuCoordinateToFx(
        sDatalinkExtendedMenuPositions[0].x + horizontal_offset
    );
    int32_t first_y_fx = Game_DatalinkExtendedMenuCoordinateToFx(
        sDatalinkExtendedMenuPositions[0].y
    );
    int32_t second_x_fx = Game_DatalinkExtendedMenuCoordinateToFx(
        sDatalinkExtendedMenuPositions[1].x + horizontal_offset
    );
    int32_t second_y_fx = Game_DatalinkExtendedMenuCoordinateToFx(
        sDatalinkExtendedMenuPositions[1].y
    );

    gGameDatalinkSceneOwner.extended_menu_item_0_duration_d54 = 0;
    gGameDatalinkSceneOwner.extended_menu_item_0_x_fx_d5c = first_x_fx;
    gGameDatalinkSceneOwner.extended_menu_item_0_y_fx_d60 = first_y_fx;
    gGameDatalinkSceneOwner.extended_menu_item_1_duration_dac = 0;
    gGameDatalinkSceneOwner.extended_menu_item_1_x_fx_db4 = first_x_fx;
    gGameDatalinkSceneOwner.extended_menu_item_1_y_fx_db8 = first_y_fx;
    gGameDatalinkSceneOwner.extended_menu_item_2_duration_e04 = 0;
    gGameDatalinkSceneOwner.extended_menu_item_2_x_fx_e0c = second_x_fx;
    gGameDatalinkSceneOwner.extended_menu_item_2_y_fx_e10 = second_y_fx;
    gGameDatalinkSceneOwner.extended_menu_item_3_duration_e5c = 0;
    gGameDatalinkSceneOwner.extended_menu_item_3_x_fx_e64 = second_x_fx;
    gGameDatalinkSceneOwner.extended_menu_item_3_y_fx_e68 = second_y_fx;
    gGameDatalinkSceneOwner.menu_item_4_duration_c4c = 0;
    gGameDatalinkSceneOwner.menu_item_4_x_fx_c54 =
        Game_DatalinkExtendedMenuCoordinateToFx(
            sDatalinkExtendedMenuAuxiliaryPosition.x
        );
    gGameDatalinkSceneOwner.menu_item_4_y_fx_c58 =
        Game_DatalinkExtendedMenuCoordinateToFx(
            sDatalinkExtendedMenuAuxiliaryPosition.y +
            DATALINK_EXTENDED_MENU_AUXILIARY_Y_OFFSET
        );
}

static void Game_ConfigureDatalinkExtendedMenuTweens(
    int horizontal_offset,
    int auxiliary_y_offset,
    int mode
)
{
    int row;

    for (row = 0; row < DATALINK_EXTENDED_MENU_ROW_COUNT; ++row) {
        const Game_DatalinkExtendedMenuPosition *position =
            &sDatalinkExtendedMenuPositions[row];
        int variant;

        for (variant = 0;
             variant < DATALINK_EXTENDED_MENU_VARIANT_COUNT;
             ++variant) {
            Game_ConfigureDatalinkTween(
                &gGameDatalinkControllerTweens[
                    sDatalinkExtendedMenuTweenIndices[row][variant]
                ],
                Game_DatalinkExtendedMenuCoordinateToFx(
                    position->x + horizontal_offset
                ),
                Game_DatalinkExtendedMenuCoordinateToFx(position->y),
                DATALINK_EXTENDED_MENU_TWEEN_DURATION,
                mode
            );
        }
    }

    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[
            DATALINK_EXTENDED_MENU_AUXILIARY_TWEEN
        ],
        Game_DatalinkExtendedMenuCoordinateToFx(
            sDatalinkExtendedMenuAuxiliaryPosition.x
        ),
        Game_DatalinkExtendedMenuCoordinateToFx(
            sDatalinkExtendedMenuAuxiliaryPosition.y + auxiliary_y_offset
        ),
        DATALINK_EXTENDED_MENU_TWEEN_DURATION,
        mode
    );
}

/* 0x020AB6D0..0x020AB7F3; pool 0x020AB7F4..0x020AB81B excluded. */
void Game_EnterDatalinkExtendedMenuFromRight(void)
{
    Game_SeedDatalinkExtendedMenuScene(
        DATALINK_EXTENDED_MENU_HORIZONTAL_DISTANCE
    );
    Game_ConfigureDatalinkExtendedMenuTweens(
        0,
        0,
        DATALINK_EXTENDED_MENU_TWEEN_MODE_IN
    );
}

/* 0x020AB81C..0x020AB93F; pool 0x020AB940..0x020AB967 excluded. */
void Game_EnterDatalinkExtendedMenuFromLeft(void)
{
    Game_SeedDatalinkExtendedMenuScene(
        -DATALINK_EXTENDED_MENU_HORIZONTAL_DISTANCE
    );
    Game_ConfigureDatalinkExtendedMenuTweens(
        0,
        0,
        DATALINK_EXTENDED_MENU_TWEEN_MODE_IN
    );
}

/* 0x020AB968..0x020ABA3B; pool 0x020ABA3C..0x020ABA5F excluded. */
void Game_ExitDatalinkExtendedMenuToRight(void)
{
    Game_ConfigureDatalinkExtendedMenuTweens(
        DATALINK_EXTENDED_MENU_HORIZONTAL_DISTANCE,
        DATALINK_EXTENDED_MENU_AUXILIARY_Y_OFFSET,
        DATALINK_EXTENDED_MENU_TWEEN_MODE_OUT
    );
}

/*
 * 0x020ABA60..0x020ABB33; pool 0x020ABB34..0x020ABB57 and next function at
 * 0x020ABB58 excluded.
 */
void Game_ExitDatalinkExtendedMenuToLeft(void)
{
    Game_ConfigureDatalinkExtendedMenuTweens(
        -DATALINK_EXTENDED_MENU_HORIZONTAL_DISTANCE,
        DATALINK_EXTENDED_MENU_AUXILIARY_Y_OFFSET,
        DATALINK_EXTENDED_MENU_TWEEN_MODE_OUT
    );
}
