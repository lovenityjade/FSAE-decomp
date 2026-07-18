#ifndef FSAE_GAME_G2D_RESOURCE_LOADER_H
#define FSAE_GAME_G2D_RESOURCE_LOADER_H

/*
 * These helpers return the mounted archive file bytes while an NNS_G2d
 * unpacker writes the typed resource pointer through `output`.
 */
const void *Game_LoadG2dCellBank(void **output, const char *path);      /* 0x020bbe50 */
const void *Game_LoadG2dAnimationBank(void **output, const char *path); /* 0x020bbe84 */
const void *Game_LoadG2dCharacterData(void **output, const char *path); /* 0x020bbeb8 */
const void *Game_LoadG2dPaletteData(void **output, const char *path);   /* 0x020bbeec */
const void *Game_LoadG2dScreenData(void **output, const char *path);    /* 0x020bbf20 */

#endif
