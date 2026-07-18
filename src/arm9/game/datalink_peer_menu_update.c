#include "game/datalink_peer_menu_update.h"

#include "game/datalink_phase_thirty.h"
#include "game/datalink_scene_command.h"
#include "game/datalink_scene_oam.h"
#include "game/datalink_screen_update.h"

#include <stddef.h>
#include <stdint.h>

enum {
    DATALINK_PEER_INPUT_CONFIRM = 0x0001,
    DATALINK_PEER_INPUT_CANCEL = 0x0002,
    DATALINK_PEER_INPUT_LEFT = 0x0040,
    DATALINK_PEER_INPUT_RIGHT = 0x0080,
    DATALINK_PEER_SOUND_SELECTION = 0x3d,
    DATALINK_PEER_SOUND_ACCEPT = 0x3e,
    DATALINK_PEER_SOUND_REJECT = 0x40,
    DATALINK_PEER_SOUND_CANCEL = 0x44,
    DATALINK_PEER_DISCOVERY_READY_AGE = 9,
    DATALINK_PEER_TRANSITION_COMPLETE = 0x1000,
    DATALINK_PEER_COMPLETION_DELAY = 60,
    DATALINK_PEER_SCREEN_EMPTY = 0x083d,
    DATALINK_PEER_SCREEN_POPULATED = 0x083e,
    DATALINK_PEER_NEXT_STATE_SUCCESS = 0x0c,
    DATALINK_PEER_NEXT_STATE_DEFAULT = 0x0d,
    DATALINK_PEER_NEXT_STATE_ALTERNATE = 0x0e,
    DATALINK_PEER_SCENE_CURSOR_COMMAND = 3,
    DATALINK_PEER_SCENE_COMPLETION_COMMAND = 0x35,
    DATALINK_PEER_SCENE_LIST_COMMAND = 0x17,
    DATALINK_PEER_SCENE_FOOTER_COMMAND = 0x22,
    DATALINK_PEER_SCREEN_REBUILD_RESOURCE = 0x84,
    DATALINK_FX_SHIFT = 12
};

static const int32_t sDatalinkPeerCursorPoints[
    GAME_DATALINK_PEER_MENU_MAX_PEERS
][2] = {
    {40, 48},
    {40, 88},
    {40, 128},
    {40, 168}
};

extern void *Game_GetDatalinkWirelessContext_020909c8(void);
extern int Game_GetDatalinkWirelessState_02090ea4(void *context);
extern int Game_GetDatalinkWirelessPeerCount_02091d38(void *context);
extern void *Game_GetDatalinkWirelessPeer_02091d40(
    void *context,
    int index
);
extern int Game_BeginDatalinkWirelessPeerConnection_02091870(
    void *context,
    void *peer,
    int data_size,
    int argument
);
extern int Game_GetDatalinkWirelessConnectionOutcome_02091cd8(
    void *context
);

extern void Game_BeginDatalinkPeerMenuEntrance_020afcbc(
    Game_DatalinkPeerMenuState *state
);
extern int Game_StartDatalinkPeerDiscovery_020b0f8c(
    Game_DatalinkPeerMenuState *state
);
extern void Game_RefreshDatalinkPeerRoster_020b0d7c(
    Game_DatalinkPeerMenuState *state
);
extern int Game_StopDatalinkPeerDiscovery_020b0fe8(
    Game_DatalinkPeerMenuState *state
);
extern int Game_SelectDatalinkWirelessPeer_020b0e84(
    Game_DatalinkPeerMenuState *state
);
extern void Game_BeginDatalinkPeerConnectedExit_020afd84(
    Game_DatalinkPeerMenuState *state
);
extern void Game_BeginDatalinkPeerCancelledExit_020afe3c(
    Game_DatalinkPeerMenuState *state
);
extern void Game_ArrangeDatalinkPeerSprites_020b0a6c(
    Game_DatalinkPeerMenuState *state,
    Game_DatalinkPeerListRecord *peer,
    unsigned int visible_index
);
extern void Game_ClearDatalinkPeerList_020b0d58(
    Game_DatalinkPeerMenuState *state
);
extern void Game_FinalizeDatalinkPeerMenuExit_020280f0(void);

extern void Game_PlayDatalinkSoundEffect(uint32_t sound_id);
extern void Game_ClampDatalinkSelection(
    volatile int8_t *selection,
    int minimum,
    int maximum
);
extern void Game_LoadDatalinkScreenResource(uint16_t resource_id);
extern const Game_DatalinkPeerSprite *Game_GetDatalinkPeerSprite_020a2efc(
    uint8_t sprite_index
);
extern int Game_GetDatalinkPeerOamCapacity_020a4ee0(void);

#if UINTPTR_MAX == UINT32_MAX
_Static_assert(
    offsetof(Game_DatalinkPeerMenuState, ordered_peers_0b4) == 0xb4,
    "datalink peer ordered-list offset"
);
_Static_assert(
    offsetof(Game_DatalinkPeerMenuState, selected_wireless_peer_0c4) ==
        0xc4,
    "datalink selected wireless peer offset"
);
_Static_assert(
    offsetof(Game_DatalinkPeerMenuState, peer_count_0c8) == 0xc8,
    "datalink peer count offset"
);
_Static_assert(
    offsetof(Game_DatalinkPeerMenuState, completion_timer_100) == 0x100,
    "datalink peer completion timer offset"
);
_Static_assert(
    offsetof(Game_DatalinkPeerMenuState, phase_106) == 0x106,
    "datalink peer phase offset"
);
#endif

_Static_assert(
    sizeof(Game_DatalinkPeerListRecord) == 0x20,
    "datalink peer record size"
);
_Static_assert(
    offsetof(Game_DatalinkPeerListRecord, discovery_age_1c) == 0x1c,
    "datalink peer discovery-age offset"
);

static unsigned int Game_GetDatalinkReadyPeerCount(
    const Game_DatalinkPeerMenuState *state)
{
    unsigned int count = 0;

    while (count < state->peer_count_0c8) {
        if (state->ordered_peers_0b4[count]->discovery_age_1c <
            DATALINK_PEER_DISCOVERY_READY_AGE) {
            break;
        }
        ++count;
    }
    return count;
}

static int Game_DatalinkWirelessPeerMatchesRecord(
    const uint8_t *wireless_peer,
    const Game_DatalinkPeerListRecord *record)
{
    unsigned int index;

    if (wireless_peer == NULL) {
        return 0;
    }
    for (index = 0; index < sizeof(record->identity_00); ++index) {
        if (wireless_peer[8U + index] != record->identity_00[index]) {
            return 0;
        }
    }
    return 1;
}

static void Game_UpdateDatalinkPeerCursor(
    Game_DatalinkPeerMenuState *state)
{
    int selection = state->selected_peer_104;

    gGameDatalinkSceneOwner.cursor_transition_duration_56c = 0;
    gGameDatalinkSceneOwner.cursor_x_fx_574 =
        sDatalinkPeerCursorPoints[selection][0] << DATALINK_FX_SHIFT;
    gGameDatalinkSceneOwner.cursor_y_fx_578 =
        sDatalinkPeerCursorPoints[selection][1] << DATALINK_FX_SHIFT;
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkPeerSceneOwner,
        DATALINK_PEER_SCENE_CURSOR_COMMAND);
}

static void Game_UpdateDatalinkPeerScreenResource(
    Game_DatalinkPeerMenuState *state)
{
    uint32_t previous_resource = state->screen_resource_0cc;
    unsigned int ready_count = Game_GetDatalinkReadyPeerCount(state);

    if (ready_count == 0U &&
        state->phase_106 != 4U &&
        state->phase_106 != 5U &&
        state->phase_106 != 6U) {
        state->screen_resource_0cc = DATALINK_PEER_SCREEN_EMPTY;
    } else {
        state->screen_resource_0cc = DATALINK_PEER_SCREEN_POPULATED;
    }
    if ((previous_resource & UINT32_C(0xffff)) !=
        state->screen_resource_0cc) {
        Game_LoadDatalinkScreenResource(
            (uint16_t)state->screen_resource_0cc);
    }
}

static void Game_SubmitDatalinkPeerSprite(uint8_t sprite_index)
{
    const Game_DatalinkPeerSprite *sprite =
        Game_GetDatalinkPeerSprite_020a2efc(sprite_index);

    Game_SubmitVisibleDatalinkSceneOams(
        &gGameDatalinkPeerSceneOwner,
        sprite->oam_entries_24 + sprite->oam_start_index_44,
        Game_GetDatalinkPeerOamCapacity_020a4ee0());
}

static void Game_RenderDatalinkReadyPeers(
    Game_DatalinkPeerMenuState *state)
{
    unsigned int index = 0;

    while (index < Game_GetDatalinkReadyPeerCount(state)) {
        Game_DatalinkPeerListRecord *peer = state->ordered_peers_0b4[index];
        uint8_t command;

        Game_SubmitDatalinkPeerSprite(peer->sprite_index_07);
        Game_SubmitDatalinkPeerSprite(
            (uint8_t)(peer->sprite_index_07 + 1U));

        if ((int)index == state->selected_peer_104) {
            command = peer->tween_index_06;
        } else {
            Game_DatalinkTweenRecord *current =
                &gGameDatalinkControllerTweens[peer->tween_index_06];
            Game_DatalinkTweenRecord *next = current + 1;

            next->duration_fx_28 = 0;
            next->current_x_fx_30 = (int32_t)(
                (uint32_t)current->current_x_fx_30 &
                UINT32_C(0xfffff000));
            next->current_y_fx_34 = (int32_t)(
                (uint32_t)current->current_y_fx_34 &
                UINT32_C(0xfffff000));
            command = (uint8_t)(peer->tween_index_06 + 1U);
        }
        Game_ApplyDatalinkSceneCommand(
            &gGameDatalinkPeerSceneOwner,
            command);
        ++index;
    }

    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkPeerSceneOwner,
        DATALINK_PEER_SCENE_LIST_COMMAND);
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkPeerSceneOwner,
        DATALINK_PEER_SCENE_FOOTER_COMMAND);
}

static void Game_UpdateDatalinkPeerSelectionPhase(
    Game_DatalinkPeerMenuState *state,
    int wireless_state)
{
    int8_t previous_selection;
    unsigned int ready_count;

    if (wireless_state != 5) {
        if (wireless_state != 4) {
            state->phase_106 = 1;
        }
        return;
    }

    Game_RefreshDatalinkPeerRoster_020b0d7c(state);
    if (gGameDatalinkPeerMenuInput == DATALINK_PEER_INPUT_CANCEL &&
        Game_StopDatalinkPeerDiscovery_020b0fe8(state) != 0) {
        Game_PlayDatalinkSoundEffect(DATALINK_PEER_SOUND_CANCEL);
        Game_BeginDatalinkPeerCancelledExit_020afe3c(state);
        state->phase_106 = 7;
        return;
    }
    if (Game_GetDatalinkReadyPeerCount(state) == 0U) {
        return;
    }

    previous_selection = state->selected_peer_104;
    if (gGameDatalinkPeerMenuInput == DATALINK_PEER_INPUT_LEFT) {
        --state->selected_peer_104;
    } else if (gGameDatalinkPeerMenuInput == DATALINK_PEER_INPUT_RIGHT) {
        ++state->selected_peer_104;
    } else if (gGameDatalinkPeerMenuInput == DATALINK_PEER_INPUT_CONFIRM) {
        state->phase_106 = 3;
    }

    ready_count = Game_GetDatalinkReadyPeerCount(state);
    Game_ClampDatalinkSelection(
        &state->selected_peer_104,
        0,
        (int)ready_count - 1);
    if (previous_selection != state->selected_peer_104) {
        Game_PlayDatalinkSoundEffect(DATALINK_PEER_SOUND_SELECTION);
    }
    Game_UpdateDatalinkPeerCursor(state);
}

static void Game_UpdateDatalinkPeerConnectPhase(
    Game_DatalinkPeerMenuState *state,
    void *wireless_context,
    int wireless_state)
{
    Game_DatalinkPeerListRecord *selected_record;
    void *candidate_peer = NULL;
    int index;

    if (wireless_state != 2) {
        return;
    }

    selected_record = state->ordered_peers_0b4[
        (unsigned int)(uint8_t)state->selected_peer_104];
    index = 0;
    while (index < Game_GetDatalinkWirelessPeerCount_02091d38(
            wireless_context)) {
        void *wireless_peer = Game_GetDatalinkWirelessPeer_02091d40(
            wireless_context,
            index);

        candidate_peer = wireless_peer;
        if (Game_DatalinkWirelessPeerMatchesRecord(
                (const uint8_t *)wireless_peer,
                selected_record) != 0 &&
            wireless_peer == state->selected_wireless_peer_0c4) {
            break;
        }
        ++index;
    }

    if (candidate_peer == state->selected_wireless_peer_0c4 &&
        Game_BeginDatalinkWirelessPeerConnection_02091870(
            wireless_context,
            candidate_peer,
            0x34,
            0) != 0) {
        state->phase_106 = 5;
        Game_PlayDatalinkSoundEffect(DATALINK_PEER_SOUND_ACCEPT);
    } else {
        state->phase_106 = 1;
        Game_PlayDatalinkSoundEffect(DATALINK_PEER_SOUND_REJECT);
    }
}

static void Game_UpdateDatalinkPeerConnectedPhase(
    Game_DatalinkPeerMenuState *state,
    void *wireless_context,
    int wireless_state)
{
    int next_state;

    if (wireless_state == 0x0b) {
        Game_BeginDatalinkPeerConnectedExit_020afd84(state);
        state->phase_106 = 6;
        return;
    }
    if (wireless_state != 2) {
        return;
    }

    switch (Game_GetDatalinkWirelessConnectionOutcome_02091cd8(
        wireless_context)) {
    case 2:
        next_state = DATALINK_PEER_NEXT_STATE_SUCCESS;
        break;
    case 3:
        next_state = DATALINK_PEER_NEXT_STATE_ALTERNATE;
        break;
    default:
        state->phase_106 = 1;
        return;
    }
    state->phase_106 = 9;
    state->completion_timer_100 = 0;
    Game_ChangeDatalinkState(next_state, 0, 0, 0);
}

static void Game_UpdateDatalinkPeerExitCountdown(
    Game_DatalinkPeerMenuState *state,
    int cancelled)
{
    unsigned int index;

    --state->exit_countdown_102;
    if (state->exit_countdown_102 == 0) {
        Game_ClearDatalinkPeerList_020b0d58(state);
        if (cancelled != 0) {
            Game_DatalinkStateStack_PopState(
                (Game_DatalinkStateStack *)(void *)
                    &gGameDatalinkPeerSceneOwner);
        } else {
            Game_FinalizeDatalinkPeerMenuExit_020280f0();
            gGameDatalinkSceneOwner.screen_rebuild_resource_53e =
                DATALINK_PEER_SCREEN_REBUILD_RESOURCE;
        }
        return;
    }

    for (index = 0; index < state->peer_count_0c8; ++index) {
        Game_ArrangeDatalinkPeerSprites_020b0a6c(
            state,
            state->ordered_peers_0b4[index],
            index);
    }
}

static int Game_DatalinkConnectionOutcomeState(void *wireless_context)
{
    int outcome = Game_GetDatalinkWirelessConnectionOutcome_02091cd8(
        wireless_context);

    if (outcome == 2) {
        return DATALINK_PEER_NEXT_STATE_SUCCESS;
    }
    if (outcome == 3) {
        return DATALINK_PEER_NEXT_STATE_ALTERNATE;
    }
    return DATALINK_PEER_NEXT_STATE_DEFAULT;
}

static void Game_UpdateDatalinkPeerCompletionPhase(
    Game_DatalinkPeerMenuState *state,
    void *wireless_context)
{
    if (state->resumed_session_107 != 0U) {
        Game_ChangeDatalinkState(
            Game_DatalinkConnectionOutcomeState(wireless_context),
            0,
            0,
            0);
        state->resumed_session_107 = 0;
        return;
    }

    if (state->completion_timer_100 <= DATALINK_PEER_COMPLETION_DELAY) {
        ++state->completion_timer_100;
        return;
    }

    gGameDatalinkPhaseThirtyObjectOwner.position_transition_duration_69c = 0;
    gGameDatalinkPhaseThirtyObjectOwner.position_x_fx_6a4 =
        224 << DATALINK_FX_SHIFT;
    gGameDatalinkPhaseThirtyObjectOwner.position_y_fx_6a8 =
        192 << DATALINK_FX_SHIFT;
    Game_ApplyDatalinkSceneCommand(
        &gGameDatalinkPeerSceneOwner,
        DATALINK_PEER_SCENE_COMPLETION_COMMAND);
    if (gGameDatalinkPeerMenuInput != DATALINK_PEER_INPUT_CONFIRM) {
        return;
    }

    Game_CommitDatalinkScreenUpdate();
    state->phase_106 = 0;
    state->completion_timer_100 = 0;
    Game_PlayDatalinkSoundEffect(DATALINK_PEER_SOUND_ACCEPT);
    Game_ClearDatalinkPeerList_020b0d58(state);
}

static void Game_UpdateDatalinkPeerPostConnectionPhase(
    Game_DatalinkPeerMenuState *state)
{
    if (state->completion_timer_100 < DATALINK_PEER_COMPLETION_DELAY + 1) {
        ++state->completion_timer_100;
        return;
    }
    if (gGameDatalinkPeerMenuInput != DATALINK_PEER_INPUT_CONFIRM) {
        return;
    }

    Game_CommitDatalinkScreenUpdate();
    state->phase_106 = 0;
    state->completion_timer_100 = 0;
    Game_PlayDatalinkSoundEffect(DATALINK_PEER_SOUND_ACCEPT);
    Game_ClearDatalinkPeerList_020b0d58(state);
}

/*
 * 0x020B0014..0x020B0777: complete wireless-peer menu update.
 * Its eleven-word literal pool at 0x020b0778..0x020b07a3 is excluded.
 */
void Game_UpdateDatalinkPeerMenu_020b0014(
    Game_DatalinkPeerMenuState *state)
{
    void *wireless_context = Game_GetDatalinkWirelessContext_020909c8();
    int wireless_state = Game_GetDatalinkWirelessState_02090ea4(
        wireless_context);

    switch (state->phase_106) {
    case 0:
        Game_BeginDatalinkPeerMenuEntrance_020afcbc(state);
        state->selected_peer_104 = 0;
        state->phase_106 = 1;
        break;
    case 1:
        if (Game_GetDatalinkTweenProgress(
                &gGameDatalinkPeerMenuTransitionTween) ==
            DATALINK_PEER_TRANSITION_COMPLETE) {
            if (Game_StartDatalinkPeerDiscovery_020b0f8c(state) != 0) {
                state->phase_106 = 2;
            } else if (gGameDatalinkPeerMenuInput ==
                    DATALINK_PEER_INPUT_CANCEL &&
                wireless_state == 2) {
                Game_PlayDatalinkSoundEffect(DATALINK_PEER_SOUND_CANCEL);
                Game_BeginDatalinkPeerCancelledExit_020afe3c(state);
                state->phase_106 = 7;
            }
        }
        break;
    case 2:
        Game_UpdateDatalinkPeerSelectionPhase(state, wireless_state);
        break;
    case 3:
        state->phase_106 =
            Game_SelectDatalinkWirelessPeer_020b0e84(state) == 0 ? 1 : 4;
        break;
    case 4:
        Game_UpdateDatalinkPeerConnectPhase(
            state,
            wireless_context,
            wireless_state);
        break;
    case 5:
        Game_UpdateDatalinkPeerConnectedPhase(
            state,
            wireless_context,
            wireless_state);
        break;
    case 6:
        Game_UpdateDatalinkPeerExitCountdown(state, 0);
        break;
    case 7:
        Game_UpdateDatalinkPeerExitCountdown(state, 1);
        if (state->exit_countdown_102 == 0) {
            return;
        }
        break;
    case 8:
        Game_UpdateDatalinkPeerCompletionPhase(state, wireless_context);
        return;
    case 9:
        Game_UpdateDatalinkPeerPostConnectionPhase(state);
        break;
    default:
        break;
    }

    Game_UpdateDatalinkPeerScreenResource(state);
    Game_RenderDatalinkReadyPeers(state);
}
