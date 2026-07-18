#ifndef FSAE_GAME_MANUAL_PAGE_FLOW_H
#define FSAE_GAME_MANUAL_PAGE_FLOW_H

#include <stdbool.h>
#include <stdint.h>

#include "ntmv/m2d/resource_accessor.h"
#include "ntmv/m2d/manual_toc_panel.h"

typedef struct GameManualViewerPageFlow GameManualViewerPageFlow;

typedef struct GameManualPageRecord {
    const char *page_name;                 /* target +0x00 */
    const uint16_t *toc_text;              /* target +0x04 */
    const uint16_t *title;                 /* target +0x08 */
    const char *const *subpage_names;      /* target +0x0c */
    const uint16_t *const *subpage_texts;  /* target +0x10 */
    uint16_t child_count;                  /* target +0x14 */
    uint16_t reserved_16;
} GameManualPageRecord;

typedef struct GameManualPageIndex {
    const GameManualPageRecord *records;
    uint16_t record_count;
    uint16_t reserved_06;
} GameManualPageIndex;

GameManualPageRecord *GameManualPageRecord_Construct_020b89e4(
    GameManualPageRecord *record);
void GameManualPageRecord_Destroy_020b8a04(
    GameManualPageRecord *record,
    void *const *allocator_context);
void GameManualPageRecord_LoadDefinition_020b8a5c(
    GameManualPageRecord *record,
    const uint16_t definition[6],
    const void *name_pool,
    const void *text_pool,
    void *const *allocator_context);

#if UINTPTR_MAX == UINT32_MAX
typedef char GameManualPageRecordTargetSizeCheck[
    sizeof(GameManualPageRecord) == 0x18 ? 1 : -1];
typedef char GameManualPageIndexTargetSizeCheck[
    sizeof(GameManualPageIndex) == 0x08 ? 1 : -1];
#endif

GameManualPageIndex *GameManualPageIndex_Construct(
    GameManualPageIndex *page_index); /* 0x020b8c00 */
void GameManualPageIndex_Destroy_020b8c10(
    GameManualPageIndex *page_index,
    void *const *allocator_context);
void GameManualPageIndex_FreeRecords_020b8c2c(
    void *const *allocator_context,
    GameManualPageRecord *records,
    uint32_t record_count);
GameManualPageIndex *GameManualPageIndex_Allocate(
    void *const *allocator_context); /* 0x020be128 */
bool GameManualPageIndex_Load_020b8c78(
    GameManualPageIndex *page_index,
    const char *resource_name,
    NtmvM2dArcResourceAccessor *accessor,
    void *allocator_context);
bool GameManualPageIndex_Parse_020b8cbc(
    GameManualPageIndex *page_index,
    const void *resource,
    void *const *allocator_context);
GameManualPageRecord *GameManualPageIndex_AllocateRecords_020b8e28(
    void *const *allocator_context,
    uint32_t record_count);

void GameManualViewer_InitializePageIndex(
    GameManualViewerPageFlow *viewer); /* 0x020be0c4 */
void GameManualViewer_UnloadCurrentPage(
    GameManualViewerPageFlow *viewer); /* 0x020bdfc0 */
void GameManualViewer_ShowRequestedPage(
    GameManualViewerPageFlow *viewer,
    bool show_guide); /* 0x020bda3c */
void GameManualViewer_RequestExit(
    GameManualViewerPageFlow *viewer); /* 0x020bda70 */
void GameManualViewer_SetTilemapContext(
    GameManualViewerPageFlow *viewer,
    const uint32_t context_words[3]); /* 0x020bda90 */
void GameManualViewer_ShowGuideFallback(
    GameManualViewerPageFlow *viewer); /* 0x020be1f4 */
void GameManualViewer_ShowSelectedPage(
    GameManualViewerPageFlow *viewer); /* 0x020be154 */

void GameManualViewer_LoadPage_020bdd60(
    GameManualViewerPageFlow *viewer,
    const char *page_name,
    NtmvM2dArcResourceAccessor *accessor,
    bool reset_scroll);

int32_t NtmvM2dManualTocPanel_GetPrimarySelection_020bac20(
    const NtmvM2dManualTocPanel *panel);
int32_t NtmvM2dManualTocPanel_GetSecondarySelection_020bac58(
    const NtmvM2dManualTocPanel *panel);

#endif
