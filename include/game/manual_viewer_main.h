#ifndef FSAE_GAME_MANUAL_VIEWER_MAIN_H
#define FSAE_GAME_MANUAL_VIEWER_MAIN_H

#include "game/manual_page_flow.h"

/* 0x020bce14..0x020bcfff: fades, dispatches input, renders and shuts down. */
void GameManualViewer_RunMainLoop_020bce14(
    GameManualViewerPageFlow *viewer);

#endif
