#ifndef FSAE_GAME_MANUAL_VIEWER_LIFECYCLE_H
#define FSAE_GAME_MANUAL_VIEWER_LIFECYCLE_H

#include "game/manual_page_flow.h"

typedef struct NtmvUiRenderer NtmvUiRenderer;

GameManualViewerPageFlow *GameManualViewer_Construct_020bc588(
    GameManualViewerPageFlow *viewer);
GameManualViewerPageFlow *GameManualViewer_Destroy_020bc6c4(
    GameManualViewerPageFlow *viewer);
void GameManualViewer_InitializeHardware_020bc8bc(
    GameManualViewerPageFlow *viewer,
    const uint32_t initial_context[4]);
void GameManualViewer_Initialize_020bc84c(
    GameManualViewerPageFlow *viewer,
    const uint32_t allocator_template[4],
    const char *shared_archive_path,
    const char *localized_archive_path,
    const uint32_t tilemap_context[3],
    uint16_t language);
void GameManualViewer_Setup_020bc99c(
    GameManualViewerPageFlow *viewer,
    void *shared_archive,
    void *localized_archive,
    const uint32_t tilemap_context[3],
    uint16_t language);

void GameManualRenderer_DestroyAndFree_020bc78c(
    void *const *allocator_context,
    NtmvUiRenderer *renderer);
void GameManualPage_DestroyAndFree_020bc7b8(
    void *const *allocator_context,
    void *page);
void GameManualPageIndex_DestroyAndFree_020bc7ec(
    void *const *allocator_context,
    GameManualPageIndex *page_index);
void GameManualPanel_DestroyAndFree_020bc818(
    void *const *allocator_context,
    void *panel);

#endif
