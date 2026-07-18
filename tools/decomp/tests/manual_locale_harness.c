#include "game/manual_locale.h"

#include "game/language_table.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static GameManualViewerLocaleContext sViewer;
static uint8_t sOwnerLanguage;
static bool sHasFile;
static uint16_t sFallbackLocale;
static uint16_t sHasFileLocale;
static uint16_t sLocalizedLocale;
static uint16_t sSharedLocale;
static unsigned int sLanguageLoadCount;

void OS_GetOwnerInfoEx(void *owner_info)
{
    uint8_t *bytes = owner_info;
    memset(bytes, 0, 88);
    bytes[0] = sOwnerLanguage;
}

bool GameLanguageTable_Load(
    GameLanguageTable *table,
    const char *name,
    NtmvM2dArcResourceAccessor *accessor)
{
    static uint16_t locales[1];
    assert(strcmp(name, "langs") == 0);
    assert(accessor == &sViewer.localized_resources);
    locales[0] = sFallbackLocale;
    table->count = 1;
    table->locale_ids = locales;
    ++sLanguageLoadCount;
    return true;
}

static bool HasFile(
    NtmvM2dArcResourceAccessor *accessor,
    uint32_t resource_type,
    uint16_t locale,
    const char *name)
{
    assert(accessor == &sViewer.localized_resources);
    assert(resource_type == 0x6e746d63u);
    assert(strcmp(name, "manual") == 0);
    sHasFileLocale = locale;
    return sHasFile;
}

static void SetLocalizedLocale(
    NtmvM2dArcResourceAccessor *accessor,
    uint16_t locale)
{
    assert(accessor == &sViewer.localized_resources);
    sLocalizedLocale = locale;
}

static void SetSharedLocale(
    NtmvM2dArcResourceAccessor *accessor,
    uint16_t locale)
{
    assert(accessor == &sViewer.shared_resources);
    sSharedLocale = locale;
}

static const NtmvM2dArcResourceAccessorVTable sLocalizedVTable = {
    0,
    0,
    0,
    0,
    SetLocalizedLocale,
    HasFile,
};

static const NtmvM2dArcResourceAccessorVTable sSharedVTable = {
    0,
    0,
    0,
    0,
    SetSharedLocale,
    0,
};

int main(void)
{
    memset(&sViewer, 0, sizeof(sViewer));
    sViewer.localized_resources.vtable = &sLocalizedVTable;
    sViewer.shared_resources.vtable = &sSharedVTable;

    sHasFile = true;
    sOwnerLanguage = 2;
    GameManualViewer_SetLocale(&sViewer, 0);
    assert(sHasFileLocale == 0x6672u); /* fr */
    assert(sLocalizedLocale == 0x6672u);
    assert(sSharedLocale == 0x6672u);
    assert(sLanguageLoadCount == 0);

    sHasFile = false;
    sFallbackLocale = 0x656eu; /* en */
    GameManualViewer_SetLocale(&sViewer, 0x6b6fu); /* unavailable ko */
    assert(sHasFileLocale == 0x6b6fu);
    assert(sLanguageLoadCount == 1);
    assert(sLocalizedLocale == 0x656eu);
    assert(sSharedLocale == 0x656eu);
    return 0;
}
