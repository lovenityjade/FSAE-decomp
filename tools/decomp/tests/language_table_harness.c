#include "game/language_table.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef struct TestFile {
    uint8_t signature[4];
    uint16_t byte_order;
    uint16_t version;
    uint32_t file_size;
    uint16_t header_size;
    uint16_t block_count;
    uint8_t first_signature[4];
    uint32_t first_size;
    uint16_t first_count;
    uint16_t first_unknown;
    uint32_t first_data;
    uint8_t language_signature[4];
    uint32_t language_size;
    uint16_t language_count;
    uint16_t language_unknown;
    uint16_t locale_ids[3];
} TestFile;

static TestFile sFile;
static uint32_t sLastType;
static const char *sLastName;

static void *GetFile(
    NtmvM2dArcResourceAccessor *accessor,
    uint32_t resource_type,
    const char *name)
{
    (void)accessor;
    sLastType = resource_type;
    sLastName = name;
    return &sFile;
}

static const NtmvM2dArcResourceAccessorVTable sVTable = {
    0,
    0,
    GetFile,
    0,
    0,
    0,
};

static void PrepareFile(void)
{
    memset(&sFile, 0, sizeof(sFile));
    memcpy(sFile.signature, "NTLI", 4);
    sFile.byte_order = 0xfeff;
    sFile.version = 0x0200;
    sFile.file_size = sizeof(sFile);
    sFile.header_size = 16;
    sFile.block_count = 2;
    memcpy(sFile.first_signature, "skip", 4);
    sFile.first_size = 16;
    sFile.first_count = 1;
    sFile.first_data = 0x11223344u;
    memcpy(sFile.language_signature, "mtl1", 4);
    sFile.language_size = 18;
    sFile.language_count = 3;
    sFile.locale_ids[0] = 1;
    sFile.locale_ids[1] = 7;
    sFile.locale_ids[2] = 9;
}

int main(void)
{
    NtmvM2dArcResourceAccessor accessor;
    GameLanguageTable table;

    memset(&accessor, 0, sizeof(accessor));
    accessor.vtable = &sVTable;
    PrepareFile();
    memset(&table, 0, sizeof(table));
    assert(GameLanguageTable_Load(&table, "langs", &accessor));
    assert(sLastType == 0x6e746c69u);
    assert(strcmp(sLastName, "langs") == 0);
    assert(table.count == 3);
    assert(table.locale_ids[0] == 1);
    assert(table.locale_ids[1] == 7);
    assert(table.locale_ids[2] == 9);

    sFile.version = 0x0201;
    assert(!GameLanguageTable_Load(&table, "langs", &accessor));
    sFile.version = 0x0200;
    sFile.byte_order = 0xfffe;
    assert(!GameLanguageTable_Load(&table, "langs", &accessor));
    sFile.byte_order = 0xfeff;
    memcpy(sFile.language_signature, "none", 4);
    assert(!GameLanguageTable_Load(&table, "langs", &accessor));
    return 0;
}
