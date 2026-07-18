#ifndef GAME_DATALINK_CONTROLLER_H
#define GAME_DATALINK_CONTROLLER_H

#include "game/datalink_initial_phase.h"
#include "game/datalink_runtime.h"

#include <stdbool.h>
#include <stdint.h>

#define GAME_DATALINK_INTERFACE_D_ADDRESS UINT32_C(0x0217E3D4)

typedef struct Game_DatalinkController {
    uint8_t unknown_00[0x04];
    uint32_t mode_04;
    uint32_t owner_address_08;
    uint8_t unknown_0c[0x24];
    uint32_t field_30;
    int32_t transition_stack_34[8];
    uint32_t interface_address_54;
    uint8_t participant_slots_58[2];
    uint8_t participant_count_5a;
    int8_t selected_row_5b;
    uint8_t transition_pending_5c;
    uint8_t transition_phase_5d;
    uint8_t transition_countdown_5e;
    uint8_t auxiliary_state_5f;
    uint16_t transition_timer_60;
} Game_DatalinkController;

typedef struct Game_DatalinkTweenRecord {
    uint8_t slot_index_00;
    uint8_t initialized_01;
    uint8_t unknown_02;
    uint8_t active_03;
    uint8_t unknown_04;
    uint8_t frame_counter_05;
    uint8_t unknown_06[0x0a];
    uint32_t interpolation_callback_address_10;
    int32_t start_x_fx_14;
    int32_t start_y_fx_18;
    int32_t target_x_fx_1c;
    int32_t target_y_fx_20;
    uint8_t unknown_24[4];
    int32_t duration_fx_28;
    int32_t elapsed_fx_2c;
    int32_t current_x_fx_30;
    int32_t current_y_fx_34;
    uint8_t unknown_38[0x18];
    int32_t cell_animation_delta_fx_50;
    uint32_t cell_animation_address_54;
} Game_DatalinkTweenRecord;

typedef struct Game_DatalinkState Game_DatalinkState;
typedef struct Game_DatalinkStateVTable {
    void (*slot_00)(Game_DatalinkState *state);
    void (*slot_04)(Game_DatalinkState *state);
    void (*slot_08)(Game_DatalinkState *state);
    void (*enter_0c)(Game_DatalinkState *state);
    void (*exit_10)(Game_DatalinkState *state);
} Game_DatalinkStateVTable;

typedef struct Game_DatalinkStateStack {
    uint8_t unknown_00[4];
    uint32_t state_code_04;
    uint32_t result_08;
    uintptr_t state_addresses_0c[8];
    int8_t depth_2c;
    uint8_t unknown_2d[3];
} Game_DatalinkStateStack;

struct Game_DatalinkState {
    const Game_DatalinkStateVTable *vtable_00;
    uint32_t unknown_04;
    Game_DatalinkStateStack *owner_stack_08;
};

typedef struct Game_DatalinkControllerPanelOwner {
    uint8_t unknown_000[0x74C];
    int32_t first_duration_74c;
    uint8_t unknown_750[4];
    int32_t first_x_fx_754;
    int32_t first_y_fx_758;
    uint8_t unknown_75c[0x48];
    int32_t second_duration_7a4;
    uint8_t unknown_7a8[4];
    int32_t second_x_fx_7ac;
    int32_t second_y_fx_7b0;
} Game_DatalinkControllerPanelOwner;

extern Game_DatalinkTweenRecord gGameDatalinkControllerTweens[32];
extern Game_DatalinkTweenRecord gGameDatalinkControllerPrimaryTween;
extern Game_DatalinkTweenRecord gGameDatalinkControllerSecondaryTween;
extern Game_DatalinkControllerPanelOwner gGameDatalinkControllerPanelOwner;

extern volatile uint8_t gGameDatalinkControllerActiveSlot;
extern volatile uint16_t gGameDatalinkControllerInput;

/* 0x020acbf4..0x020acc27: clears and initializes one tween record. */
void Game_InitializeDatalinkTweenRecord_020acbf4(
    Game_DatalinkTweenRecord *tween,
    uint8_t slot_index
);

/* 0x020acc2c..0x020acc7b: advances tween and attached cell animation. */
void Game_UpdateDatalinkTweenRecord_020acc2c(
    Game_DatalinkTweenRecord *tween
);

/* 0x020acc7c..0x020acc9f: advances and upper-clamps tween elapsed time. */
bool Game_AdvanceDatalinkTween_020acc7c(
    Game_DatalinkTweenRecord *tween,
    int32_t delta_fx
);

/* 0x020acca0..0x020acd0f; callback literal pool follows at 0x020acd10. */
void Game_ConfigureDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t target_x_fx,
    int32_t target_y_fx,
    uint32_t duration_frames,
    int mode
);

/* 0x020acd1c..0x020acd73: advances and interpolates the current position. */
bool Game_UpdateDatalinkTween(
    Game_DatalinkTweenRecord *tween,
    int32_t delta_fx
);

/* 0x020acd74..0x020acd97: returns 20.12 tween progress. */
int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween
);

/* 0x020acd98..0x020acdaf: initializes state code, result and depth. */
void Game_DatalinkStateStack_Init_020acd98(
    Game_DatalinkStateStack *stack
);

/* 0x020acdb0..0x020acdd7: installs and enters the root state. */
void Game_DatalinkStateStack_EnterRootState(
    Game_DatalinkStateStack *stack,
    Game_DatalinkState *state
);

/* 0x020acdd8..0x020ace3f: exits current state and pushes a new one. */
void Game_DatalinkStateStack_PushState(
    Game_DatalinkStateStack *stack,
    Game_DatalinkState *state
);

/* 0x020ace40..0x020ace9b: exits current state and restores the previous one. */
void Game_DatalinkStateStack_PopState(
    Game_DatalinkStateStack *stack
);

/* 0x020ace9c..0x020acf5b: serializes control fields and return states. */
void Game_DatalinkStateStack_Serialize(
    const Game_DatalinkStateStack *stack,
    uint8_t **cursor
);

/* 0x020acf5c..0x020ad01f: inverse of the state-stack serializer. */
void Game_DatalinkStateStack_Deserialize(
    Game_DatalinkStateStack *stack,
    const uint8_t **cursor
);

void Game_InitializeDatalinkController(Game_DatalinkController *controller);

void Game_BeginDatalinkControllerTransition(
    Game_DatalinkController *controller,
    uint32_t unused_argument,
    uint32_t transition_argument,
    uint32_t ignored_fourth_argument
);

void Game_RebuildDatalinkControllerScreen(void);

int Game_PrepareDatalinkControllerVariant(
    Game_DatalinkController *controller
);

void Game_ConfigureDatalinkControllerVariant(
    Game_DatalinkController *controller,
    int variant
);

void Game_SelectDatalinkControllerVariantZero(
    Game_DatalinkController *controller
);

void Game_SelectDatalinkControllerVariantOne(
    Game_DatalinkController *controller
);

int Game_UpdateDatalinkControllerAnimation(
    Game_DatalinkController *controller,
    uint32_t unused_second_argument,
    uint32_t unused_third_argument,
    uint32_t unused_fourth_argument
);

int Game_UpdateDatalinkControllerSelection(
    Game_DatalinkController *controller
);

void Game_RefreshDatalinkControllerTilemaps(
    Game_DatalinkController *controller
);

void Game_BuildDatalinkControllerDescriptors(
    const Game_DatalinkController *controller
);

int Game_UpdateDatalinkControllerParticipantSelection(
    Game_DatalinkController *controller
);

int Game_UpdateDatalinkControllerParticipantAction(
    Game_DatalinkController *controller
);

#endif
