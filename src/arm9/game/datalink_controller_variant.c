#include "game/datalink_controller.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_VARIANT_SLOT_COUNT = 3,
    DATALINK_VARIANT_ZERO = 0,
    DATALINK_VARIANT_ONE = 1,
    DATALINK_VARIANT_ZERO_MODE = 2,
    DATALINK_VARIANT_ONE_MODE = 1,
    DATALINK_VARIANT_TWEEN_DURATION = 20,
    DATALINK_VARIANT_VERTICAL_OFFSET = 144
};

typedef struct Game_DatalinkVariantPosition {
    int32_t x;
    int32_t y;
} Game_DatalinkVariantPosition;

/* Six-byte ROM table at 0x020DE790. */
static const uint8_t
    sDatalinkVariantTweenIndices[DATALINK_VARIANT_SLOT_COUNT][2] = {
        {11, 12},
        {13, 14},
        {15, 16}
    };

/* Three coordinate pairs at 0x020DE8CC. */
static const Game_DatalinkVariantPosition
    sDatalinkVariantPositions[DATALINK_VARIANT_SLOT_COUNT] = {
        {128, 96},
        {84, 136},
        {172, 136}
    };

/* Opaque 0x020ACCA0 tween configuration. */
extern void Game_ConfigureDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t target_x_fx,
    int32_t target_y_fx,
    uint32_t duration,
    int mode
);

_Static_assert(
    sizeof(Game_DatalinkTweenRecord) == 0x58,
    "datalink controller variant tween stride"
);
_Static_assert(
    offsetof(Game_DatalinkTweenRecord, active_03) == 0x03,
    "datalink controller variant active offset"
);
_Static_assert(
    offsetof(Game_DatalinkTweenRecord, duration_fx_28) == 0x28,
    "datalink controller variant duration offset"
);
_Static_assert(
    offsetof(Game_DatalinkTweenRecord, current_x_fx_30) == 0x30,
    "datalink controller variant x offset"
);
_Static_assert(
    offsetof(Game_DatalinkTweenRecord, current_y_fx_34) == 0x34,
    "datalink controller variant y offset"
);

static int32_t Game_DatalinkVariantCoordinateToFx(int32_t coordinate)
{
    return coordinate * 0x1000;
}

static void Game_SeedDatalinkVariantTween(
    Game_DatalinkTweenRecord *tween,
    int32_t current_x_fx,
    int32_t current_y_fx
)
{
    volatile uint8_t *active = &tween->active_03;
    volatile int32_t *duration = &tween->duration_fx_28;
    volatile int32_t *current_x = &tween->current_x_fx_30;
    volatile int32_t *current_y = &tween->current_y_fx_34;

    *active = 1;
    *duration = 0;
    *current_x = current_x_fx;
    *current_y = current_y_fx;
}

/*
 * 0x020A92E0
 *
 * Complete 416-byte internal controller-variant routine.  Variant zero seeds
 * tween records 11..16 at each target's vertically offset position, marks
 * them active, and configures motion toward the unshifted position in mode
 * two.  Variant one configures the same six records toward the offset
 * position in mode one without directly changing record state.  Other
 * selectors return without side effects.  The incoming controller pointer is
 * forwarded by both wrappers but is not consumed by this target.
 */
void Game_ConfigureDatalinkControllerVariant(
    Game_DatalinkController *controller,
    int variant
)
{
    int slot;

    (void)controller;

    if (variant == DATALINK_VARIANT_ZERO) {
        for (slot = 0; slot < DATALINK_VARIANT_SLOT_COUNT; ++slot) {
            const Game_DatalinkVariantPosition *position =
                &sDatalinkVariantPositions[slot];
            Game_DatalinkTweenRecord *first =
                &gGameDatalinkControllerTweens[
                    sDatalinkVariantTweenIndices[slot][0]
                ];
            Game_DatalinkTweenRecord *second =
                &gGameDatalinkControllerTweens[
                    sDatalinkVariantTweenIndices[slot][1]
                ];
            int32_t target_x_fx =
                Game_DatalinkVariantCoordinateToFx(position->x);
            int32_t target_y_fx =
                Game_DatalinkVariantCoordinateToFx(position->y);
            int32_t offset_y_fx = Game_DatalinkVariantCoordinateToFx(
                position->y + DATALINK_VARIANT_VERTICAL_OFFSET
            );

            Game_SeedDatalinkVariantTween(
                first,
                target_x_fx,
                offset_y_fx
            );
            Game_SeedDatalinkVariantTween(
                second,
                target_x_fx,
                offset_y_fx
            );
            Game_ConfigureDatalinkTween(
                first,
                target_x_fx,
                target_y_fx,
                DATALINK_VARIANT_TWEEN_DURATION,
                DATALINK_VARIANT_ZERO_MODE
            );
            Game_ConfigureDatalinkTween(
                second,
                target_x_fx,
                target_y_fx,
                DATALINK_VARIANT_TWEEN_DURATION,
                DATALINK_VARIANT_ZERO_MODE
            );
        }
        return;
    }

    if (variant == DATALINK_VARIANT_ONE) {
        for (slot = 0; slot < DATALINK_VARIANT_SLOT_COUNT; ++slot) {
            const Game_DatalinkVariantPosition *position =
                &sDatalinkVariantPositions[slot];
            int32_t target_x_fx =
                Game_DatalinkVariantCoordinateToFx(position->x);
            int32_t target_y_fx = Game_DatalinkVariantCoordinateToFx(
                position->y + DATALINK_VARIANT_VERTICAL_OFFSET
            );

            Game_ConfigureDatalinkTween(
                &gGameDatalinkControllerTweens[
                    sDatalinkVariantTweenIndices[slot][0]
                ],
                target_x_fx,
                target_y_fx,
                DATALINK_VARIANT_TWEEN_DURATION,
                DATALINK_VARIANT_ONE_MODE
            );
            Game_ConfigureDatalinkTween(
                &gGameDatalinkControllerTweens[
                    sDatalinkVariantTweenIndices[slot][1]
                ],
                target_x_fx,
                target_y_fx,
                DATALINK_VARIANT_TWEEN_DURATION,
                DATALINK_VARIANT_ONE_MODE
            );
        }
    }
}

/*
 * 0x020A9494
 *
 * Complete 12-byte tail wrapper.  It preserves the incoming controller
 * pointer and selects variant zero in the shared routine at 0x020A92E0.
 */
void Game_SelectDatalinkControllerVariantZero(
    Game_DatalinkController *controller
)
{
    Game_ConfigureDatalinkControllerVariant(
        controller,
        DATALINK_VARIANT_ZERO
    );
}

/*
 * 0x020A94A4
 *
 * Complete 12-byte tail wrapper.  It preserves the incoming controller
 * pointer and selects variant one in the shared routine at 0x020A92E0.
 */
void Game_SelectDatalinkControllerVariantOne(
    Game_DatalinkController *controller
)
{
    Game_ConfigureDatalinkControllerVariant(
        controller,
        DATALINK_VARIANT_ONE
    );
}
