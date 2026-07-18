#include "ntmv/m2d/resource_accessor.h"

/* SDK/static-library dependencies identified in the analysed ARM9 image. */
extern int STD_CopyLString(char *destination, const char *source, int capacity);
extern int STD_TSNPrintf(char *destination, int capacity, const char *format, ...);
extern void *NNS_FndGetArchiveFileByName(const char *path);

/* 0x0200c748: recovered global delete wrapper; the original project name is lost. */
extern void Game_OperatorDelete(void *object);

static const char sThreePartPath[] = "%s/%s/%s";       /* 0x02126bf0 */
static const char sFourPartPath[] = "%s/%s/%s/%s";    /* 0x02126bfc */

/* 0x020b7110 */
static void WriteResourceType(char output[5], uint32_t resource_type)
{
    output[0] = (char)(resource_type >> 24);
    output[1] = (char)(resource_type >> 16);
    output[2] = (char)(resource_type >> 8);
    output[3] = (char)resource_type;
    output[4] = '\0';
}

/* 0x020b713c */
static void WriteLocale(char output[3], uint16_t locale)
{
    output[0] = (char)(locale >> 8);
    output[1] = (char)locale;
    output[2] = '\0';
}

/* 0x020b7154 */
static void *FindThreePartPath(
    char *path_buffer,
    int capacity,
    const char *root_path,
    uint32_t resource_type,
    const char *name)
{
    char type_name[5];

    WriteResourceType(type_name, resource_type);
    (void)STD_TSNPrintf(path_buffer, capacity, sThreePartPath, root_path, type_name, name);
    return NNS_FndGetArchiveFileByName(path_buffer);
}

/* 0x020b71ac */
static void *FindFourPartPath(
    char *path_buffer,
    int capacity,
    const char *root_path,
    uint32_t resource_type,
    uint16_t locale,
    const char *name)
{
    char type_name[5];
    char locale_name[3];

    WriteResourceType(type_name, resource_type);
    WriteLocale(locale_name, locale);
    (void)STD_TSNPrintf(
        path_buffer, capacity, sFourPartPath, root_path, type_name, locale_name, name);
    return NNS_FndGetArchiveFileByName(path_buffer);
}

const NtmvM2dResourceAccessorVTable gNtmvM2dResourceAccessorVTable = {
    NtmvM2dResourceAccessor_Destroy,
    NtmvM2dResourceAccessor_DeletingDestroy,
};

const NtmvM2dArcResourceAccessorVTable gNtmvM2dArcResourceAccessorVTable = {
    NtmvM2dArcResourceAccessor_Destroy,
    NtmvM2dArcResourceAccessor_DeletingDestroy,
    NtmvM2dArcResourceAccessor_GetFile,
    NtmvM2dArcResourceAccessor_GetLocalizedFile,
    NtmvM2dArcResourceAccessor_SetLocale,
    NtmvM2dArcResourceAccessor_HasFile,
};

/* 0x020b70e4, ResourceAccessor vtable slot 0. */
NtmvM2dResourceAccessor *NtmvM2dResourceAccessor_Destroy(
    NtmvM2dResourceAccessor *accessor)
{
    return accessor;
}

/* 0x020b70e8, ResourceAccessor vtable slot 1. */
NtmvM2dResourceAccessor *NtmvM2dResourceAccessor_DeletingDestroy(
    NtmvM2dResourceAccessor *accessor)
{
    Game_OperatorDelete(accessor);
    return accessor;
}

/* 0x020b70fc: base-subobject destructor variant used by ArcResourceAccessor. */
NtmvM2dResourceAccessor *NtmvM2dResourceAccessor_DestroySubobject_020b70fc(
    NtmvM2dResourceAccessor *accessor)
{
    return accessor;
}

/* 0x020b7100 */
NtmvM2dResourceAccessor *NtmvM2dResourceAccessor_Init(
    NtmvM2dResourceAccessor *accessor)
{
    accessor->vtable = &gNtmvM2dResourceAccessorVTable;
    return accessor;
}

/* 0x020b7218 */
NtmvM2dArcResourceAccessor *NtmvM2dArcResourceAccessor_Init(
    NtmvM2dArcResourceAccessor *accessor)
{
    (void)NtmvM2dResourceAccessor_Init((NtmvM2dResourceAccessor *)accessor);
    accessor->vtable = &gNtmvM2dArcResourceAccessorVTable;
    accessor->locale = 0;
    accessor->ready = 0;
    return accessor;
}

/* 0x020b7248 */
bool NtmvM2dArcResourceAccessor_SetRootPath(
    NtmvM2dArcResourceAccessor *accessor, const char *root_path)
{
    accessor->ready = 1;
    (void)STD_CopyLString(accessor->root_path, root_path, 0x7f);
    return true;
}

/* 0x020b7268, ArcResourceAccessor vtable slot 2. */
void *NtmvM2dArcResourceAccessor_GetFile(
    NtmvM2dArcResourceAccessor *accessor, uint32_t resource_type, const char *name)
{
    return FindThreePartPath(
        accessor->path_buffer, 0x7f, accessor->root_path, resource_type, name);
}

/*
 * 0x020b728c, ArcResourceAccessor vtable slot 3.  The fourth ABI parameter is
 * not consumed by the retail routine; its original source-level role remains
 * provisional.
 */
void *NtmvM2dArcResourceAccessor_GetLocalizedFile(
    NtmvM2dArcResourceAccessor *accessor,
    uint32_t resource_type,
    const char *name,
    void *unused_result)
{
    (void)unused_result;
    if (accessor->locale != 0) {
        return FindFourPartPath(
            accessor->path_buffer,
            0x7f,
            accessor->root_path,
            resource_type,
            accessor->locale,
            name);
    }
    return FindThreePartPath(
        accessor->path_buffer, 0x7f, accessor->root_path, resource_type, name);
}

/* 0x020b72e0, ArcResourceAccessor vtable slot 4. */
void NtmvM2dArcResourceAccessor_SetLocale(
    NtmvM2dArcResourceAccessor *accessor, uint16_t locale)
{
    accessor->locale = locale;
}

/* 0x020b72ec, ArcResourceAccessor vtable slot 5. */
bool NtmvM2dArcResourceAccessor_HasFile(
    NtmvM2dArcResourceAccessor *accessor,
    uint32_t resource_type,
    uint16_t locale,
    const char *name)
{
    if (locale != 0) {
        return FindFourPartPath(
                   accessor->path_buffer,
                   0x7f,
                   accessor->root_path,
                   resource_type,
                   locale,
                   name) != NULL;
    }
    return FindThreePartPath(
               accessor->path_buffer, 0x7f, accessor->root_path, resource_type, name) != NULL;
}

/* 0x020b7348, ArcResourceAccessor vtable slot 0. */
NtmvM2dArcResourceAccessor *NtmvM2dArcResourceAccessor_Destroy(
    NtmvM2dArcResourceAccessor *accessor)
{
    (void)NtmvM2dResourceAccessor_DestroySubobject_020b70fc(
        (NtmvM2dResourceAccessor *)accessor);
    return accessor;
}

/* 0x020b735c, ArcResourceAccessor vtable slot 1. */
NtmvM2dArcResourceAccessor *NtmvM2dArcResourceAccessor_DeletingDestroy(
    NtmvM2dArcResourceAccessor *accessor)
{
    (void)NtmvM2dResourceAccessor_DestroySubobject_020b70fc(
        (NtmvM2dResourceAccessor *)accessor);
    Game_OperatorDelete(accessor);
    return accessor;
}
