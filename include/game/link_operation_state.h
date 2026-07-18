#ifndef FSAE_GAME_LINK_OPERATION_STATE_H
#define FSAE_GAME_LINK_OPERATION_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
    GAME_LINK_OPERATION_SUCCEEDED = 1
};

/* Only the prefix proven by the accessors in this recovery is exposed. */
typedef struct GameLinkOperationState {
    uint8_t reserved_00[8];
    int32_t result; /* +0x08: zero while pending, one on success */
    uint8_t reserved_0c[2];
    uint8_t lobby_session_visible_0e;
    uint8_t reserved_0f;
} GameLinkOperationState;

typedef char GameLinkOperationStateResultOffsetCheck[
    offsetof(GameLinkOperationState, result) == 0x08 ? 1 : -1];
typedef char GameLinkOperationStateLobbyVisibilityOffsetCheck[
    offsetof(GameLinkOperationState, lobby_session_visible_0e) == 0x0e ?
        1 : -1];

extern GameLinkOperationState gGameLinkOperationState_0212c9a8;

/* 0x020b2a90..0x020b2aa7; literal at 0x020b2aa8 excluded. */
bool Game_LinkOperation_HasSucceeded_020b2a90(void);

/* 0x020b2aac..0x020b2aaf: intentionally empty link-update hook. */
void Game_LinkOperation_NoOp_020b2aac(void);

#endif
