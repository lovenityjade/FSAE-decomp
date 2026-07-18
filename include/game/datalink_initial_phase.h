#ifndef GAME_DATALINK_INITIAL_PHASE_H
#define GAME_DATALINK_INITIAL_PHASE_H

#include <stdint.h>

typedef struct Game_DatalinkFlowState {
    uint8_t unknown_00[0x2C];
    int8_t active_participant_index_2c;
    uint8_t unknown_2d[7];
    uint32_t primary_transition_address_34;
    uint32_t ready_transition_address_38;
    uint32_t unavailable_transition_address_3c;
    int8_t selected_participant_40;
    uint8_t phase_41;
    uint8_t suppress_redraw_42;
    uint8_t selection_dirty_43;
    uint8_t unknown_44;
    uint8_t phase_timer_45;
    uint8_t unknown_46[2];
    uint32_t return_phase_depth_48;
    uint32_t return_phase_stack_4c[8];
} Game_DatalinkFlowState;

void Game_EnterDatalinkInitialPhase(Game_DatalinkFlowState *state);

#endif
