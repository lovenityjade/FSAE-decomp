#include "game/datalink_resource_access.h"

#include <stdint.h>

/* External helper at 0x0209B700 reads an offset from the primary archive index. */
extern uint32_t Game_GetDatalinkPrimaryResourceOffset(
    uint32_t resource_id
);

/* External helper at 0x0209B714 subtracts adjacent primary-archive index offsets. */
extern uint32_t Game_GetDatalinkPrimaryResourceSize(
    uint32_t resource_id
);

/* External helper at 0x0209B734 reads an offset from the alternate archive index. */
extern uint32_t Game_GetDatalinkAlternateResourceOffset(
    uint32_t resource_id
);

/* External helper at 0x0209B748 subtracts adjacent alternate-archive index offsets. */
extern uint32_t Game_GetDatalinkAlternateResourceSize(
    uint32_t resource_id
);

/*
 * 0x020AC3D0
 *
 * Complete 52-byte resource-address wrapper.
 * The two-word pool at 0x020AC404..0x020AC40B and the independent size
 * wrapper beginning at 0x020AC40C are excluded.
 */
const void *Game_GetDatalinkResourceAddress(
    uint32_t resource_id,
    int use_alternate_archive
)
{
    if (use_alternate_archive != 0) {
        return gGameDatalinkAlternateResourceBase +
               Game_GetDatalinkAlternateResourceOffset(resource_id);
    }

    return gGameDatalinkPrimaryResourceBase +
           Game_GetDatalinkPrimaryResourceOffset(resource_id);
}

/*
 * 0x020AC40C
 *
 * Complete 28-byte resource-size wrapper.
 * It has no literal pool; the independent function beginning at 0x020AC428
 * is excluded.
 */
uint32_t Game_GetDatalinkResourceSize(
    uint32_t resource_id,
    int use_alternate_archive
)
{
    if (use_alternate_archive != 0) {
        return Game_GetDatalinkAlternateResourceSize(resource_id);
    }

    return Game_GetDatalinkPrimaryResourceSize(resource_id);
}
