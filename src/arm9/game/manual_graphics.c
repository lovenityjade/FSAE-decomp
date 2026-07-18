#include "game/manual_graphics.h"

#include <stdint.h>

enum {
    GAME_MANUAL_ALL_LCDC_BANKS = 0x1ff,
    GAME_MANUAL_VRAM_ADDRESS = 0x06800000,
    GAME_MANUAL_VRAM_SIZE = 0x000a4000,
    GAME_MANUAL_MAIN_OAM_ADDRESS = 0x07000000,
    GAME_MANUAL_MAIN_PALETTE_ADDRESS = 0x05000000,
    GAME_MANUAL_SUB_OAM_ADDRESS = 0x07000400,
    GAME_MANUAL_SUB_PALETTE_ADDRESS = 0x05000400,
    GAME_MANUAL_OAM_PALETTE_SIZE = 0x400,
    GAME_MANUAL_DISABLED_OAM_WORD = 0xc0
};

extern void GX_SetBankForLCDC(uint16_t banks);
extern void GX_DisableBankForLCDC_02015448(void);
extern void MIi_CpuClearFast(
    uint32_t value,
    void *destination,
    uint32_t size);

static void *GraphicsAddress(uintptr_t address)
{
    return (void *)address;
}

/* 0x020bdabc */
void GameManualViewer_ClearGraphicsMemory_020bdabc(void)
{
    GX_SetBankForLCDC(GAME_MANUAL_ALL_LCDC_BANKS);
    MIi_CpuClearFast(
        0,
        GraphicsAddress(GAME_MANUAL_VRAM_ADDRESS),
        GAME_MANUAL_VRAM_SIZE);
    GX_DisableBankForLCDC_02015448();

    MIi_CpuClearFast(
        GAME_MANUAL_DISABLED_OAM_WORD,
        GraphicsAddress(GAME_MANUAL_MAIN_OAM_ADDRESS),
        GAME_MANUAL_OAM_PALETTE_SIZE);
    MIi_CpuClearFast(
        0,
        GraphicsAddress(GAME_MANUAL_MAIN_PALETTE_ADDRESS),
        GAME_MANUAL_OAM_PALETTE_SIZE);
    MIi_CpuClearFast(
        GAME_MANUAL_DISABLED_OAM_WORD,
        GraphicsAddress(GAME_MANUAL_SUB_OAM_ADDRESS),
        GAME_MANUAL_OAM_PALETTE_SIZE);
    MIi_CpuClearFast(
        0,
        GraphicsAddress(GAME_MANUAL_SUB_PALETTE_ADDRESS),
        GAME_MANUAL_OAM_PALETTE_SIZE);
}
