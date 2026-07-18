#ifndef GAME_DATALINK_SCREEN_UPDATE_H
#define GAME_DATALINK_SCREEN_UPDATE_H

#include <stdint.h>

typedef struct Game_DatalinkScreenUpdateState {
    uint8_t unknown_00[4];
    uint32_t saved_shared_value_04;
    uint32_t screen_buffer_requires_upload_08;
    int8_t clear_x_0c;
    int8_t clear_y_0d;
    int8_t clear_width_0e;
    int8_t clear_height_0f;
    uint32_t character_buffer_requires_upload_10;
    uint8_t unknown_14[0x34];
    uint16_t *screen_buffer_48;
    void *character_buffer_4c;
    uint8_t unknown_50[8];
    uint16_t display_control_58;
    uint8_t setup_bytes_5a[10];
    uint8_t unknown_64[4];
    uint32_t setup_active_68;
    uint32_t participant_count_6c;
    uint8_t unknown_70[0x0c];
    void *render_context_member_7c;
} Game_DatalinkScreenUpdateState;

typedef struct Game_DatalinkScreenConfigRecord {
    uint16_t layout_flags;
    uint16_t resource_id;
    uint16_t cue_id;
} Game_DatalinkScreenConfigRecord;

extern const Game_DatalinkScreenConfigRecord
    gGameDatalinkScreenConfigRecords[];

extern Game_DatalinkScreenUpdateState
    gGameDatalinkScreenUpdateState;
extern volatile uint32_t gGameDatalinkScreenSharedValue;

void Game_CommitDatalinkScreenUpdate(void);
void Game_ChangeDatalinkState(
    int state_id,
    uint32_t participant_count,
    uintptr_t character_buffer_address,
    uintptr_t screen_buffer_address);

#endif
