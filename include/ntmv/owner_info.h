#ifndef FSAE_NTMV_OWNER_INFO_H
#define FSAE_NTMV_OWNER_INFO_H

#include <stdint.h>

/*
 * OS_GetOwnerInfo writes the compact DS owner profile used by UI skin assets.
 * The recovered code currently observes only byte +0x01.
 */
typedef struct NtmvOwnerInfo {
    uint8_t language;       /* +0x00 */
    uint8_t favorite_color; /* +0x01 */
    uint8_t reserved_02[0x52];
} NtmvOwnerInfo;

uint32_t Ntmv_GetSkinColor_020b7b00(void);

void OS_GetOwnerInfo(NtmvOwnerInfo *owner_info);

#endif
