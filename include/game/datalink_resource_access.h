#ifndef GAME_DATALINK_RESOURCE_ACCESS_H
#define GAME_DATALINK_RESOURCE_ACCESS_H

#include <stdint.h>

extern const uint8_t *gGameDatalinkPrimaryResourceBase;
extern const uint8_t *gGameDatalinkAlternateResourceBase;

const void *Game_GetDatalinkResourceAddress(
    uint32_t resource_id,
    int use_alternate_archive
);

uint32_t Game_GetDatalinkResourceSize(
    uint32_t resource_id,
    int use_alternate_archive
);

#endif
