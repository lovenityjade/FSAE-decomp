#ifndef FSAE_GAME_MANUAL_TOC_INPUT_H
#define FSAE_GAME_MANUAL_TOC_INPUT_H

#include "game/manual_page_flow.h"
#include "ntmv/m2d/items_panel.h"
#include "ntmv/m2d/scroll_controls.h"

#include <stdbool.h>

bool GameManualViewer_HandleTocPointer(
    GameManualViewerPageFlow *viewer,
    const NtmvM2dItemsPointerState *pointer); /* 0x020bd2d8 */
bool GameManualViewer_HandleScrollPointer(
    GameManualViewerPageFlow *viewer,
    const NtmvM2dScrollPointerState *pointer); /* 0x020bd3b0 */
bool GameManualViewer_HandleDpadScroll(
    GameManualViewerPageFlow *viewer); /* 0x020bd830 */
bool GameManualViewer_HandleButtons(
    GameManualViewerPageFlow *viewer); /* 0x020bd968 */

#endif
