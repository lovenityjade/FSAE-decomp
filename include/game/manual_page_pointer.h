#ifndef FSAE_GAME_MANUAL_PAGE_POINTER_H
#define FSAE_GAME_MANUAL_PAGE_POINTER_H

#include "game/input_state.h"
#include "game/manual_page_flow.h"

#include <stdbool.h>

/* 0x020bd474..0x020bd807: page drag, reversal hysteresis and inertia. */
bool GameManualViewer_HandlePagePointer_020bd474(
    GameManualViewerPageFlow *viewer,
    const GameTouchState *pointer);

#endif
