#include "ntmv/owner_info.h"

#include <assert.h>
#include <string.h>

static unsigned int sOwnerInfoCalls;

void OS_GetOwnerInfo(NtmvOwnerInfo *owner_info)
{
    memset(owner_info, 0xa5, sizeof(*owner_info));
    owner_info->language = 4;
    owner_info->favorite_color = 11;
    ++sOwnerInfoCalls;
}

int main(void)
{
    assert(Ntmv_GetSkinColor_020b7b00() == 11);
    assert(sOwnerInfoCalls == 1);
    return 0;
}
