#include "game/manual_locale.h"

#include "game/language_table.h"

enum {
    GAME_MANUAL_RESOURCE_TYPE = 0x6e746d63 /* "ntmc" */
};

typedef struct GameOwnerInfo {
    uint8_t language;
    uint8_t unknown_01[87];
} GameOwnerInfo;

/* 0x020df004: SDK language enum -> two-letter locale code. */
static const uint16_t sOwnerLanguageLocales[8] = {
    0x6a61, /* ja */
    0x656e, /* en */
    0x6672, /* fr */
    0x6465, /* de */
    0x6974, /* it */
    0x6573, /* es */
    0x7a68, /* zh */
    0x6b6f, /* ko */
};

static const char sManualResourceName[] = "manual"; /* 0x02127178 */
static const char sLanguageTableName[] = "langs";  /* 0x02127180 */

/* 0x020be00c */
void GameManualViewer_SetLocale(
    GameManualViewerLocaleContext *viewer,
    uint32_t requested_locale)
{
    uint16_t locale = (uint16_t)requested_locale;

    if (requested_locale == 0) {
        GameOwnerInfo owner;
        OS_GetOwnerInfoEx(&owner);
        locale = sOwnerLanguageLocales[owner.language];
    }

    if (!viewer->localized_resources.vtable->has_file(
            &viewer->localized_resources,
            GAME_MANUAL_RESOURCE_TYPE,
            locale,
            sManualResourceName)) {
        GameLanguageTable table;
        (void)GameLanguageTable_Load(
            &table,
            sLanguageTableName,
            &viewer->localized_resources);
        locale = table.locale_ids[0];
    }

    viewer->localized_resources.vtable->set_locale(
        &viewer->localized_resources, locale);
    viewer->shared_resources.vtable->set_locale(
        &viewer->shared_resources, locale);
}
