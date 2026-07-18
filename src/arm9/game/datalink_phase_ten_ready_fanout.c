#include "game/datalink_phase_ten_ready_fanout.h"

#include <stdint.h>

typedef struct Game_DatalinkFanoutPosition {
    int32_t x;
    int32_t y;
} Game_DatalinkFanoutPosition;

enum {
    DATALINK_FANOUT_ROW_COUNT = 3,
    DATALINK_SELECTION_ROW_HEIGHT = 48,
    DATALINK_FANOUT_DURATION = 20,
    DATALINK_FANOUT_MODE = 2,
    DATALINK_FX_ONE = 0x1000
};

/* ROM table 0x020DE7D0, copied to the parent's stack before phase dispatch. */
static const uint8_t sDatalinkFanoutPairs[DATALINK_FANOUT_ROW_COUNT][2] = {
    {5, 6},
    {7, 8},
    {9, 10}
};

/* ROM tables at 0x020DE8FC and 0x020DE92C. */
static const Game_DatalinkFanoutPosition sDatalinkFanoutPrimaryPositions
    [DATALINK_FANOUT_ROW_COUNT] = {
        {44, 48},
        {44, 96},
        {44, 144}
    };
static const Game_DatalinkFanoutPosition sDatalinkFanoutPairPositions
    [DATALINK_FANOUT_ROW_COUNT] = {
        {24, 30},
        {24, 78},
        {24, 126}
    };

/* Opaque 0x020ACCA0 transition setup. */
extern void Game_StartDatalinkObjectTransition(
    Game_DatalinkPositionObject *object,
    int32_t target_x_fx,
    int32_t target_y_fx,
    int32_t duration,
    int mode
);

/*
 * Ready fanout unit 0x020A84C0..0x020A856F inside FUN_020A8198.
 *
 * `initial_selection` is the signed selection captured by the preceding lead
 * unit before any transition call.  The target retains its row*48 offset in
 * a callee-saved register throughout all nine calls; it does not reload the
 * state's selection.  For each row, calls occur in this exact order:
 *
 * 1. primary object row (indices 0, 1, 2) using the 0x020DE8FC table;
 * 2. first paired object (5, 7, 9) using the 0x020DE92C coordinates;
 * 3. second paired object (6, 8, 10) reusing those paired coordinates.
 *
 * All Y coordinates subtract initial_selection*48 before conversion to 20.12.
 * Duration 20 and mode two are retained from the ready lead setup.
 */
void Game_StartDatalinkPhaseTenReadyFanout(int initial_selection)
{
    int32_t row_offset = initial_selection * DATALINK_SELECTION_ROW_HEIGHT;
    int row;

    for (row = 0; row < DATALINK_FANOUT_ROW_COUNT; ++row) {
        int32_t pair_x_fx;
        int32_t pair_y_fx;

        Game_StartDatalinkObjectTransition(
            &gGameDatalinkPositionObjects[row],
            sDatalinkFanoutPrimaryPositions[row].x * DATALINK_FX_ONE,
            (sDatalinkFanoutPrimaryPositions[row].y - row_offset) *
                DATALINK_FX_ONE,
            DATALINK_FANOUT_DURATION,
            DATALINK_FANOUT_MODE
        );

        pair_x_fx =
            sDatalinkFanoutPairPositions[row].x * DATALINK_FX_ONE;
        pair_y_fx =
            (sDatalinkFanoutPairPositions[row].y - row_offset) *
            DATALINK_FX_ONE;
        Game_StartDatalinkObjectTransition(
            &gGameDatalinkPositionObjects[sDatalinkFanoutPairs[row][0]],
            pair_x_fx,
            pair_y_fx,
            DATALINK_FANOUT_DURATION,
            DATALINK_FANOUT_MODE
        );
        Game_StartDatalinkObjectTransition(
            &gGameDatalinkPositionObjects[sDatalinkFanoutPairs[row][1]],
            pair_x_fx,
            pair_y_fx,
            DATALINK_FANOUT_DURATION,
            DATALINK_FANOUT_MODE
        );
    }
}
