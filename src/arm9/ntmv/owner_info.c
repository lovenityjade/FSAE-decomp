#include "ntmv/owner_info.h"

/* 0x020b7b00 */
uint32_t Ntmv_GetSkinColor_020b7b00(void)
{
    NtmvOwnerInfo owner_info;

    OS_GetOwnerInfo(&owner_info);
    return owner_info.favorite_color;
}
