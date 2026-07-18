#ifndef FSAE_NTMV_M2D_RESOURCE_ACCESSOR_H
#define FSAE_NTMV_M2D_RESOURCE_ACCESSOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct NtmvM2dResourceAccessor NtmvM2dResourceAccessor;
typedef struct NtmvM2dArcResourceAccessor NtmvM2dArcResourceAccessor;

typedef struct NtmvM2dResourceAccessorVTable {
    NtmvM2dResourceAccessor *(*destroy)(NtmvM2dResourceAccessor *accessor);
    NtmvM2dResourceAccessor *(*deleting_destroy)(NtmvM2dResourceAccessor *accessor);
} NtmvM2dResourceAccessorVTable;

struct NtmvM2dResourceAccessor {
    const NtmvM2dResourceAccessorVTable *vtable;
};

typedef struct NtmvM2dArcResourceAccessorVTable {
    NtmvM2dArcResourceAccessor *(*destroy)(NtmvM2dArcResourceAccessor *accessor);
    NtmvM2dArcResourceAccessor *(*deleting_destroy)(NtmvM2dArcResourceAccessor *accessor);
    void *(*get_file)(
        NtmvM2dArcResourceAccessor *accessor, uint32_t resource_type, const char *name);
    void *(*get_localized_file)(
        NtmvM2dArcResourceAccessor *accessor,
        uint32_t resource_type,
        const char *name,
        void *unused_result);
    void (*set_locale)(NtmvM2dArcResourceAccessor *accessor, uint16_t locale);
    bool (*has_file)(
        NtmvM2dArcResourceAccessor *accessor,
        uint32_t resource_type,
        uint16_t locale,
        const char *name);
} NtmvM2dArcResourceAccessorVTable;

/*
 * RTTI names:
 *   ntmv::m2d::ResourceAccessor    typeinfo 0x02126b58, vtable 0x02126b88
 *   ntmv::m2d::ArcResourceAccessor typeinfo 0x02126ba0, vtable 0x02126bb4
 */
struct NtmvM2dArcResourceAccessor {
    const NtmvM2dArcResourceAccessorVTable *vtable; /* +0x000 */
    char root_path[0x7f];                           /* +0x004 */
    char path_buffer[0x7f];                         /* +0x083 */
    uint16_t locale;                                /* +0x102 */
    uint8_t ready;                                  /* +0x104 */
    uint8_t reserved_105[3];
};

#if UINTPTR_MAX == UINT32_MAX
typedef char NtmvM2dArcResourceAccessorSizeCheck[
    sizeof(NtmvM2dArcResourceAccessor) == 0x108 ? 1 : -1];
typedef char NtmvM2dArcResourceAccessorPathOffsetCheck[
    offsetof(NtmvM2dArcResourceAccessor, path_buffer) == 0x83 ? 1 : -1];
typedef char NtmvM2dArcResourceAccessorLocaleOffsetCheck[
    offsetof(NtmvM2dArcResourceAccessor, locale) == 0x102 ? 1 : -1];
#endif

extern const NtmvM2dResourceAccessorVTable gNtmvM2dResourceAccessorVTable;
extern const NtmvM2dArcResourceAccessorVTable gNtmvM2dArcResourceAccessorVTable;

NtmvM2dResourceAccessor *NtmvM2dResourceAccessor_Init( /* 0x020b7100 */
    NtmvM2dResourceAccessor *accessor);
NtmvM2dResourceAccessor *NtmvM2dResourceAccessor_Destroy( /* 0x020b70e4 */
    NtmvM2dResourceAccessor *accessor);
NtmvM2dResourceAccessor *NtmvM2dResourceAccessor_DeletingDestroy( /* 0x020b70e8 */
    NtmvM2dResourceAccessor *accessor);
NtmvM2dResourceAccessor *NtmvM2dResourceAccessor_DestroySubobject_020b70fc(
    NtmvM2dResourceAccessor *accessor);

NtmvM2dArcResourceAccessor *NtmvM2dArcResourceAccessor_Init( /* 0x020b7218 */
    NtmvM2dArcResourceAccessor *accessor);
bool NtmvM2dArcResourceAccessor_SetRootPath( /* 0x020b7248 */
    NtmvM2dArcResourceAccessor *accessor, const char *root_path);
void *NtmvM2dArcResourceAccessor_GetFile( /* 0x020b7268 */
    NtmvM2dArcResourceAccessor *accessor, uint32_t resource_type, const char *name);
void *NtmvM2dArcResourceAccessor_GetLocalizedFile( /* 0x020b728c */
    NtmvM2dArcResourceAccessor *accessor,
    uint32_t resource_type,
    const char *name,
    void *unused_result);
void NtmvM2dArcResourceAccessor_SetLocale( /* 0x020b72e0 */
    NtmvM2dArcResourceAccessor *accessor, uint16_t locale);
bool NtmvM2dArcResourceAccessor_HasFile( /* 0x020b72ec */
    NtmvM2dArcResourceAccessor *accessor,
    uint32_t resource_type,
    uint16_t locale,
    const char *name);
NtmvM2dArcResourceAccessor *NtmvM2dArcResourceAccessor_Destroy( /* 0x020b7348 */
    NtmvM2dArcResourceAccessor *accessor);
NtmvM2dArcResourceAccessor *NtmvM2dArcResourceAccessor_DeletingDestroy( /* 0x020b735c */
    NtmvM2dArcResourceAccessor *accessor);

#endif
