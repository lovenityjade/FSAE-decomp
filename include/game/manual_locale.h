#ifndef FSAE_GAME_MANUAL_LOCALE_H
#define FSAE_GAME_MANUAL_LOCALE_H

#include <stddef.h>
#include <stdint.h>

#include "ntmv/m2d/resource_accessor.h"

typedef struct GameManualViewerLocaleContext {
    uint8_t unknown_00000[0x403f8];
    NtmvM2dArcResourceAccessor shared_resources;    /* target +0x403f8 */
    NtmvM2dArcResourceAccessor localized_resources; /* target +0x40500 */
} GameManualViewerLocaleContext;

#if UINTPTR_MAX == UINT32_MAX
typedef char GameManualViewerSharedResourcesOffsetCheck[
    offsetof(GameManualViewerLocaleContext, shared_resources) == 0x403f8 ? 1 : -1];
typedef char GameManualViewerLocalizedResourcesOffsetCheck[
    offsetof(GameManualViewerLocaleContext, localized_resources) == 0x40500 ? 1 : -1];
typedef char GameManualViewerLocaleContextTargetSizeCheck[
    sizeof(GameManualViewerLocaleContext) == 0x40608 ? 1 : -1];
#endif

void GameManualViewer_SetLocale(
    GameManualViewerLocaleContext *viewer,
    uint32_t requested_locale); /* 0x020be00c */

void OS_GetOwnerInfoEx(void *owner_info);

#endif
