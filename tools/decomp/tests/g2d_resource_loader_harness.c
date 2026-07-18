#include "game/g2d_resource_loader.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

static int sUnpackResult = 1;
static const char sFile[] = "archive-file";
static const char sResource[] = "unpacked";

const void *NNS_FndGetArchiveFileByName(const char *path)
{
    return strcmp(path, "missing") == 0 ? NULL : sFile;
}

static int Unpack(const void *file, void **output)
{
    assert(file == sFile);
    *output = (void *)sResource;
    return sUnpackResult;
}

int NNS_G2dGetUnpackedCellBank(const void *file, void **output) { return Unpack(file, output); }
int NNS_G2dGetUnpackedAnimBank(const void *file, void **output) { return Unpack(file, output); }
int NNS_G2dGetUnpackedCharacterData(const void *file, void **output) { return Unpack(file, output); }
int NNS_G2dGetUnpackedPaletteData(const void *file, void **output) { return Unpack(file, output); }
int NNS_G2dGetUnpackedScreenData(const void *file, void **output) { return Unpack(file, output); }

int main(void)
{
    void *output = NULL;

    assert(Game_LoadG2dCellBank(&output, "cell") == sFile && output == sResource);
    assert(Game_LoadG2dAnimationBank(&output, "anim") == sFile);
    assert(Game_LoadG2dCharacterData(&output, "char") == sFile);
    assert(Game_LoadG2dPaletteData(&output, "palette") == sFile);
    assert(Game_LoadG2dScreenData(&output, "screen") == sFile);
    assert(Game_LoadG2dCellBank(&output, "missing") == NULL);
    sUnpackResult = 0;
    assert(Game_LoadG2dScreenData(&output, "invalid") == NULL);
    return 0;
}
