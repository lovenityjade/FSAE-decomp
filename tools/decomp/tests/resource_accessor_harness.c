#include "ntmv/m2d/resource_accessor.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static char sLastLookup[0x100];
static void *const sFound = (void *)(uintptr_t)0x1234;

int STD_CopyLString(char *destination, const char *source, int capacity)
{
    (void)snprintf(destination, (size_t)capacity, "%s", source);
    return (int)strlen(destination);
}

int STD_TSNPrintf(char *destination, int capacity, const char *format, ...)
{
    int result;
    va_list arguments;
    va_start(arguments, format);
    result = vsnprintf(destination, (size_t)capacity, format, arguments);
    va_end(arguments);
    return result;
}

void *NNS_FndGetArchiveFileByName(const char *path)
{
    (void)snprintf(sLastLookup, sizeof(sLastLookup), "%s", path);
    return strstr(path, "missing") == NULL ? sFound : NULL;
}

void Game_OperatorDelete(void *object)
{
    (void)object;
}

int main(void)
{
    NtmvM2dArcResourceAccessor accessor;

    (void)memset(&accessor, 0xcc, sizeof(accessor));
    assert(NtmvM2dArcResourceAccessor_Init(&accessor) == &accessor);
    assert(accessor.vtable == &gNtmvM2dArcResourceAccessorVTable);
    assert(accessor.locale == 0);
    assert(accessor.ready == 0);
    assert(NtmvM2dArcResourceAccessor_SetRootPath(&accessor, "M2M:/arc"));

    assert(NtmvM2dArcResourceAccessor_GetFile(&accessor, 0x74657830, "button") == sFound);
    assert(strcmp(sLastLookup, "M2M:/arc/tex0/button") == 0);

    NtmvM2dArcResourceAccessor_SetLocale(&accessor, 0x656e);
    assert(NtmvM2dArcResourceAccessor_GetLocalizedFile(
               &accessor, 0x666e7430, "title", NULL) == sFound);
    assert(strcmp(sLastLookup, "M2M:/arc/fnt0/en/title") == 0);
    assert(!NtmvM2dArcResourceAccessor_HasFile(
        &accessor, 0x74657830, 0, "missing"));
    return 0;
}
