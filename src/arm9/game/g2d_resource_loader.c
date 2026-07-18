#include "game/g2d_resource_loader.h"

#include <stddef.h>

extern const void *NNS_FndGetArchiveFileByName(const char *path);
extern int NNS_G2dGetUnpackedCellBank(const void *file, void **output);
extern int NNS_G2dGetUnpackedAnimBank(const void *file, void **output);
extern int NNS_G2dGetUnpackedCharacterData(const void *file, void **output);
extern int NNS_G2dGetUnpackedPaletteData(const void *file, void **output);
extern int NNS_G2dGetUnpackedScreenData(const void *file, void **output);

typedef int (*NtmvG2dUnpackFunction)(const void *file, void **output);

static const void *LoadAndUnpack(
    void **output, const char *path, NtmvG2dUnpackFunction unpack)
{
    const void *file = NNS_FndGetArchiveFileByName(path);

    if (file != NULL && unpack(file, output) != 0) {
        return file;
    }
    return NULL;
}

/* 0x020bbe50 */
const void *Game_LoadG2dCellBank(void **output, const char *path)
{
    return LoadAndUnpack(output, path, NNS_G2dGetUnpackedCellBank);
}

/* 0x020bbe84 */
const void *Game_LoadG2dAnimationBank(void **output, const char *path)
{
    return LoadAndUnpack(output, path, NNS_G2dGetUnpackedAnimBank);
}

/*
 * 0x020bbeb8.  The target at 0x020c2f44 is the NNS_G2d character-data
 * unpacker by its neighboring palette/screen routines and parsed header flow;
 * its static-library name was not independently recovered, so the name stays
 * descriptive until the matching phase.
 */
const void *Game_LoadG2dCharacterData(void **output, const char *path)
{
    return LoadAndUnpack(output, path, NNS_G2dGetUnpackedCharacterData);
}

/* 0x020bbeec */
const void *Game_LoadG2dPaletteData(void **output, const char *path)
{
    return LoadAndUnpack(output, path, NNS_G2dGetUnpackedPaletteData);
}

/* 0x020bbf20 */
const void *Game_LoadG2dScreenData(void **output, const char *path)
{
    return LoadAndUnpack(output, path, NNS_G2dGetUnpackedScreenData);
}
