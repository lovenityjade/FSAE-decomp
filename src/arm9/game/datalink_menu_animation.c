#include "game/datalink_menu_controller.h"

#include "game/datalink_controller.h"
#include "game/datalink_phase_ten_finalizer.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_MENU_ROW_COUNT = 2,
    DATALINK_MENU_VARIANT_COUNT = 2,
    DATALINK_MENU_HORIZONTAL_DISTANCE = 256,
    DATALINK_MENU_AUXILIARY_Y_OFFSET = 24,
    DATALINK_MENU_TWEEN_DURATION = 20,
    DATALINK_MENU_TWEEN_MODE_OUT = 1,
    DATALINK_MENU_TWEEN_MODE_IN = 2,
    DATALINK_MENU_AUXILIARY_TWEEN = 23,
    DATALINK_FX_ONE = 0x1000
};

typedef struct Game_DatalinkMenuPosition {
    int32_t x;
    int32_t y;
} Game_DatalinkMenuPosition;

static const uint8_t
    sDatalinkMenuTweenIndices[DATALINK_MENU_ROW_COUNT]
                              [DATALINK_MENU_VARIANT_COUNT] = {
        {19, 20},
        {21, 22}
    };

static const Game_DatalinkMenuPosition
    sDatalinkMenuPositions[DATALINK_MENU_ROW_COUNT] = {
        {128, 64},
        {128, 120}
    };

static const Game_DatalinkMenuPosition sDatalinkMenuAuxiliaryPosition = {
    24,
    180
};

extern void Game_ConfigureDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t target_x_fx,
    int32_t target_y_fx,
    uint32_t duration,
    int mode
);

_Static_assert(
    offsetof(Game_DatalinkSceneOwner, menu_item_0_duration_aec) == 0xAEC,
    "datalink menu item zero duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, menu_item_0_x_fx_af4) == 0xAF4,
    "datalink menu item zero X offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, menu_item_1_duration_b44) == 0xB44,
    "datalink menu item one duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, menu_item_2_duration_b9c) == 0xB9C,
    "datalink menu item two duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, menu_item_3_duration_bf4) == 0xBF4,
    "datalink menu item three duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, menu_item_4_duration_c4c) == 0xC4C,
    "datalink menu item four duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkSceneOwner, menu_item_4_y_fx_c58) == 0xC58,
    "datalink menu item four Y offset"
);

static int32_t Game_DatalinkMenuCoordinateToFx(int32_t coordinate)
{
    return coordinate * DATALINK_FX_ONE;
}

static void Game_SeedDatalinkMenuScene(int horizontal_offset)
{
    int32_t first_x_fx = Game_DatalinkMenuCoordinateToFx(
        sDatalinkMenuPositions[0].x + horizontal_offset
    );
    int32_t first_y_fx = Game_DatalinkMenuCoordinateToFx(
        sDatalinkMenuPositions[0].y
    );
    int32_t second_x_fx = Game_DatalinkMenuCoordinateToFx(
        sDatalinkMenuPositions[1].x + horizontal_offset
    );
    int32_t second_y_fx = Game_DatalinkMenuCoordinateToFx(
        sDatalinkMenuPositions[1].y
    );

    gGameDatalinkSceneOwner.menu_item_0_duration_aec = 0;
    gGameDatalinkSceneOwner.menu_item_0_x_fx_af4 = first_x_fx;
    gGameDatalinkSceneOwner.menu_item_0_y_fx_af8 = first_y_fx;
    gGameDatalinkSceneOwner.menu_item_1_duration_b44 = 0;
    gGameDatalinkSceneOwner.menu_item_1_x_fx_b4c = first_x_fx;
    gGameDatalinkSceneOwner.menu_item_1_y_fx_b50 = first_y_fx;
    gGameDatalinkSceneOwner.menu_item_2_duration_b9c = 0;
    gGameDatalinkSceneOwner.menu_item_2_x_fx_ba4 = second_x_fx;
    gGameDatalinkSceneOwner.menu_item_2_y_fx_ba8 = second_y_fx;
    gGameDatalinkSceneOwner.menu_item_3_duration_bf4 = 0;
    gGameDatalinkSceneOwner.menu_item_3_x_fx_bfc = second_x_fx;
    gGameDatalinkSceneOwner.menu_item_3_y_fx_c00 = second_y_fx;
    gGameDatalinkSceneOwner.menu_item_4_duration_c4c = 0;
    gGameDatalinkSceneOwner.menu_item_4_x_fx_c54 =
        Game_DatalinkMenuCoordinateToFx(
            sDatalinkMenuAuxiliaryPosition.x
        );
    gGameDatalinkSceneOwner.menu_item_4_y_fx_c58 =
        Game_DatalinkMenuCoordinateToFx(
            sDatalinkMenuAuxiliaryPosition.y +
            DATALINK_MENU_AUXILIARY_Y_OFFSET
        );
}

static void Game_ConfigureDatalinkMenuTweens(
    int horizontal_offset,
    int auxiliary_y_offset,
    int mode
)
{
    int row;

    for (row = 0; row < DATALINK_MENU_ROW_COUNT; ++row) {
        const Game_DatalinkMenuPosition *position =
            &sDatalinkMenuPositions[row];
        int variant;

        for (variant = 0;
             variant < DATALINK_MENU_VARIANT_COUNT;
             ++variant) {
            Game_ConfigureDatalinkTween(
                &gGameDatalinkControllerTweens[
                    sDatalinkMenuTweenIndices[row][variant]
                ],
                Game_DatalinkMenuCoordinateToFx(
                    position->x + horizontal_offset
                ),
                Game_DatalinkMenuCoordinateToFx(position->y),
                DATALINK_MENU_TWEEN_DURATION,
                mode
            );
        }
    }

    Game_ConfigureDatalinkTween(
        &gGameDatalinkControllerTweens[DATALINK_MENU_AUXILIARY_TWEEN],
        Game_DatalinkMenuCoordinateToFx(
            sDatalinkMenuAuxiliaryPosition.x
        ),
        Game_DatalinkMenuCoordinateToFx(
            sDatalinkMenuAuxiliaryPosition.y + auxiliary_y_offset
        ),
        DATALINK_MENU_TWEEN_DURATION,
        mode
    );
}

/*
 * 0x020AAD08
 * Complete 292-byte right-side entrance through 0x020AAE2B. The ten-word
 * pool at 0x020AAE2C..0x020AAE53 is excluded.
 */
void Game_EnterDatalinkMenuFromRight(void)
{
    Game_SeedDatalinkMenuScene(DATALINK_MENU_HORIZONTAL_DISTANCE);
    Game_ConfigureDatalinkMenuTweens(
        0,
        0,
        DATALINK_MENU_TWEEN_MODE_IN
    );
}

/*
 * 0x020AAE54
 * Complete 212-byte left-side exit through 0x020AAF27. The nine-word pool at
 * 0x020AAF28..0x020AAF4B is excluded.
 */
void Game_ExitDatalinkMenuToLeft(void)
{
    Game_ConfigureDatalinkMenuTweens(
        -DATALINK_MENU_HORIZONTAL_DISTANCE,
        DATALINK_MENU_AUXILIARY_Y_OFFSET,
        DATALINK_MENU_TWEEN_MODE_OUT
    );
}

/*
 * 0x020AAF4C
 * Complete 292-byte left-side entrance through 0x020AB06F. The ten-word pool
 * at 0x020AB070..0x020AB097 is excluded.
 */
void Game_EnterDatalinkMenuFromLeft(void)
{
    Game_SeedDatalinkMenuScene(-DATALINK_MENU_HORIZONTAL_DISTANCE);
    Game_ConfigureDatalinkMenuTweens(
        0,
        0,
        DATALINK_MENU_TWEEN_MODE_IN
    );
}

/*
 * 0x020AB098
 * Complete 212-byte right-side exit through 0x020AB16B. The nine-word pool at
 * 0x020AB16C..0x020AB18F and next function at 0x020AB190 are excluded.
 */
void Game_ExitDatalinkMenuToRight(void)
{
    Game_ConfigureDatalinkMenuTweens(
        DATALINK_MENU_HORIZONTAL_DISTANCE,
        DATALINK_MENU_AUXILIARY_Y_OFFSET,
        DATALINK_MENU_TWEEN_MODE_OUT
    );
}
