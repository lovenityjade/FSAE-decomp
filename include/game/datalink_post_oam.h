#ifndef GAME_DATALINK_POST_OAM_H
#define GAME_DATALINK_POST_OAM_H

#include "game/datalink_sprite_oam.h"

#include <stdint.h>

enum {
    GAME_DATALINK_POST_OAM_PARTICIPANT_CAPACITY = 4,
    GAME_DATALINK_POST_OAM_PALETTE_ROWS = 16,
    GAME_DATALINK_POST_OAM_COLORS_PER_ROW = 8,
    GAME_DATALINK_POST_OAM_PALETTE_RECORD_SIZE = 0x68
};

typedef struct Game_DatalinkPostOamHardwareState {
    uint8_t unknown_000[0xD0];
    uint8_t display_state_d0;
    int8_t display_variant_d1;
    uint8_t unknown_0d2[0x0E];
    uint32_t palette_origin_e0;
    uint32_t palette_phase_e4;
    uint8_t unknown_0e8[0x10];
    int32_t brightness_f8;
} Game_DatalinkPostOamHardwareState;

typedef struct Game_DatalinkPostOamState {
    int8_t brightness_00;
    int8_t brightness_step_01;
    uint8_t unknown_02[2];
    uint32_t pending_resource_size_04;
    uint32_t pending_resource_address_08;
    uint32_t palette_dirty_0c;
    uint32_t object_cursor_10;
    uint32_t active_14;
    uint32_t palette_phase_18;
} Game_DatalinkPostOamState;

typedef struct Game_DatalinkPostOamSession {
    uint8_t participant_count_00;
    uint8_t unknown_01[0x8D];
    uint8_t layout_mode_8e;
} Game_DatalinkPostOamSession;

typedef struct Game_DatalinkPostOamLocalSelection {
    uint8_t unknown_00[0x12];
    uint8_t local_participant_index_12;
} Game_DatalinkPostOamLocalSelection;

typedef struct Game_DatalinkPostOamParticipant {
    uint8_t unknown_00[0x24];
    uint32_t marker_variant_24;
    uint8_t unknown_28[0x0C];
} Game_DatalinkPostOamParticipant;

typedef struct Game_DatalinkPostOamOutput {
    uint8_t unknown_000[0x3E0];
    Game_DatalinkOamEntry scratch_oams_3e0[16];
    uint32_t submitted_oam_count_460;
} Game_DatalinkPostOamOutput;

typedef struct Game_DatalinkPostOamRuntime {
    uint8_t unknown_000[0x10];
    uint32_t scene_field_10;
    uint32_t scene_field_14;
    uint8_t unknown_018[0x2C];
    uint8_t resource_context_44[0x14];
    uint32_t resource_handle_58;
    uint8_t unknown_05c[8];
    uint32_t object_list_address_64;
    uint32_t object_count_68;
    uint8_t unknown_06c[0x18C];
    uint8_t readiness_state_1f8;
} Game_DatalinkPostOamRuntime;

typedef struct Game_DatalinkPostOamController {
    uint8_t opaque_00;
} Game_DatalinkPostOamController;

/* Recovered globals; absolute-address integration is maintained separately. */
extern Game_DatalinkPostOamHardwareState
    gGameDatalinkPostOamHardwareState; /* 0x0217D23C */
extern uint8_t
    gGameDatalinkPostOamReleaseResources[]; /* 0x0217A258 */
extern Game_DatalinkPostOamSession
    gGameDatalinkPostOamSession; /* 0x0212C7C8 */
extern Game_DatalinkPostOamLocalSelection
    gGameDatalinkPostOamLocalSelection; /* 0x0212C5BC */
extern Game_DatalinkPostOamParticipant
    gGameDatalinkPostOamParticipants[
        GAME_DATALINK_POST_OAM_PARTICIPANT_CAPACITY
    ]; /* 0x0212CA20 */
extern Game_DatalinkPostOamOutput
    *gGameDatalinkPostOamOutput; /* 0x0217D348 */
extern Game_DatalinkPostOamState
    gGameDatalinkPostOamState; /* 0x02180DA8 */
extern Game_DatalinkPostOamController
    gGameDatalinkPostOamController; /* 0x02180E24 */
extern uint16_t
    gGameDatalinkPostOamPalette
        [GAME_DATALINK_POST_OAM_PALETTE_ROWS]
        [GAME_DATALINK_POST_OAM_COLORS_PER_ROW]; /* 0x02180EE0 */
extern uint8_t
    gGameDatalinkPostOamPaletteRecords
        [GAME_DATALINK_POST_OAM_PALETTE_ROWS]
        [GAME_DATALINK_POST_OAM_PALETTE_RECORD_SIZE]; /* 0x02180FE0 */
extern uint8_t
    gGameDatalinkPostOamDefaultResource[]; /* 0x021250D4 */
extern volatile uint16_t
    gGameDatalinkPostOamFlags; /* 0x0212C584 */

void Game_InitializeDatalinkPostOamSceneGraphics(void); /* 0x020ADB04 */
void Game_ReleaseDatalinkPostOamSceneGraphics(void); /* 0x020ADB68 */
void Game_RenderDatalinkRemoteParticipantOams(void); /* 0x020ADBB8 */
void Game_CommitDatalinkPostOamPendingResource(void); /* 0x020ADF84 */
void Game_UploadDatalinkPostOamPaletteAndObjects(void); /* 0x020ADFF0 */
void Game_UpdateDatalinkPostOamPaletteWave(void); /* 0x020AE130 */

#endif
