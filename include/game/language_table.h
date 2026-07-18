#ifndef FSAE_GAME_LANGUAGE_TABLE_H
#define FSAE_GAME_LANGUAGE_TABLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ntmv/m2d/resource_accessor.h"

typedef struct GameLanguageTable {
    uint16_t count;                 /* target +0x00 */
    uint16_t padding_02;
    const uint16_t *locale_ids;     /* target +0x04 */
} GameLanguageTable;

#if UINTPTR_MAX == UINT32_MAX
typedef char GameLanguageTableTargetSizeCheck[
    sizeof(GameLanguageTable) == 8 ? 1 : -1];
typedef char GameLanguageTableIdsOffsetCheck[
    offsetof(GameLanguageTable, locale_ids) == 4 ? 1 : -1];
#endif

bool NtmvM2dBinaryFile_IsValid(
    const void *file,
    uint32_t signature); /* 0x020b7378 */
bool GameLanguageTable_Load(
    GameLanguageTable *table,
    const char *name,
    NtmvM2dArcResourceAccessor *accessor); /* 0x020bf09c */

#endif
