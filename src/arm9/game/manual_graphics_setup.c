#include "game/manual_graphics.h"

#include "game/graphics_state.h"
#include "game/manual_display.h"

#include <stdint.h>

enum {
    GAME_MANUAL_DISPLAY_SWAP = 0x8000,
    GAME_MANUAL_DISPLAY_MODE_MASK = 0x3f000000,
    GAME_MANUAL_DISPLAY_MODE = 0x13000000,
    GAME_MANUAL_LAYER_CONTROL_MASK = 0x00300010,
    GAME_MANUAL_VISIBLE_PLANES_MASK = 0x1f00,
    GAME_MANUAL_VISIBLE_PLANES = 0x1800,
    GAME_MANUAL_MAIN_BG0_CONTROL = 0xc000,
    GAME_MANUAL_MAIN_BG1_CONTROL = 0xc401,
    GAME_MANUAL_MAIN_BG2_CONTROL = 0x8802,
    GAME_MANUAL_BG3_CONTROL = 0x6087,
    GAME_MANUAL_WINDOW_INSIDE_PLANES = 0x28,
    GAME_MANUAL_WINDOW_OUTSIDE_PLANES = 0x37,
    GAME_MANUAL_WINDOW_PLANE_MASK = 0x3f,
    GAME_MANUAL_MASTER_BRIGHTNESS = 0x10,
    GAME_MANUAL_MAIN_MASTER_BRIGHTNESS_REGISTER = 0x0400006c,
    GAME_MANUAL_SUB_MASTER_BRIGHTNESS_REGISTER = 0x0400106c
};

extern void GX_SetGraphicsMode(
    uint32_t display_mode,
    uint32_t background_mode,
    uint32_t background_0_mode);
extern void GXS_SetGraphicsMode(uint32_t background_mode);
extern void GX_SetBankForBG(uint32_t bank);
extern void GX_SetBankForSubBG(uint32_t bank);
extern void GX_SetBankForOBJ(uint32_t bank);
extern void GX_SetBankForOBJExtPltt(uint32_t bank);
extern void GX_SetBankForSubOBJ(uint32_t bank);
extern void GX_SetBankForSubOBJExtPltt(uint32_t bank);
extern void GXx_SetMasterBrightness_(
    uintptr_t register_address,
    int32_t brightness);

static uint16_t ReplaceWindowPlanes(uint16_t control, uint16_t planes)
{
    return (uint16_t)(
        (control & ~GAME_MANUAL_WINDOW_PLANE_MASK) | planes);
}

/* 0x020bdb34 */
void GameManualViewer_SetupGraphics_020bdb34(void)
{
    uint32_t display_bits;

    gGamePowerControl_04000304 &= (uint16_t)~GAME_MANUAL_DISPLAY_SWAP;
    GX_SetGraphicsMode(1, 3, 0);
    GXS_SetGraphicsMode(3);
    GX_SetBankForBG(3);

    gGameMainDisplayControl_04000000 =
        (gGameMainDisplayControl_04000000 &
            ~GAME_MANUAL_DISPLAY_MODE_MASK) |
        GAME_MANUAL_DISPLAY_MODE;
    gGameMainBgControl_04000008[0] = GAME_MANUAL_MAIN_BG0_CONTROL;
    gGameMainBgControl_04000008[1] = GAME_MANUAL_MAIN_BG1_CONTROL;
    gGameMainBgControl_04000008[2] = GAME_MANUAL_MAIN_BG2_CONTROL;
    gGameMainBgControl_04000008[3] = GAME_MANUAL_BG3_CONTROL;

    GX_SetBankForSubBG(4);
    gGameSubBg3Control_0400100e = GAME_MANUAL_BG3_CONTROL;

    GX_SetBankForOBJ(0x10);
    display_bits = Game_GetUiDisplayControlBits();
    gGameMainDisplayControl_04000000 =
        (gGameMainDisplayControl_04000000 &
            ~GAME_MANUAL_LAYER_CONTROL_MASK) |
        display_bits;
    GX_SetBankForOBJExtPltt(0x20);

    GX_SetBankForSubOBJ(8);
    display_bits = Game_GetUiDisplayControlBits();
    gGameSubDisplayControl_04001000 =
        (gGameSubDisplayControl_04001000 &
            ~GAME_MANUAL_LAYER_CONTROL_MASK) |
        display_bits;
    GX_SetBankForSubOBJExtPltt(0x100);

    gGameMainDisplayControl_04000000 =
        (gGameMainDisplayControl_04000000 &
            ~GAME_MANUAL_VISIBLE_PLANES_MASK) |
        GAME_MANUAL_VISIBLE_PLANES;
    gGameSubDisplayControl_04001000 =
        (gGameSubDisplayControl_04001000 &
            ~GAME_MANUAL_VISIBLE_PLANES_MASK) |
        GAME_MANUAL_VISIBLE_PLANES;

    GXx_SetMasterBrightness_(
        GAME_MANUAL_MAIN_MASTER_BRIGHTNESS_REGISTER,
        GAME_MANUAL_MASTER_BRIGHTNESS);
    GXx_SetMasterBrightness_(
        GAME_MANUAL_SUB_MASTER_BRIGHTNESS_REGISTER,
        GAME_MANUAL_MASTER_BRIGHTNESS);
    Game_SetDefaultMainBlendAlpha();

    gGameMainWindowInsideControl_04000048 = ReplaceWindowPlanes(
        gGameMainWindowInsideControl_04000048,
        GAME_MANUAL_WINDOW_INSIDE_PLANES);
    gGameMainWindowOutsideControl_0400004a = ReplaceWindowPlanes(
        gGameMainWindowOutsideControl_0400004a,
        GAME_MANUAL_WINDOW_OUTSIDE_PLANES);
    gGameSubWindowInsideControl_04001048 = ReplaceWindowPlanes(
        gGameSubWindowInsideControl_04001048,
        GAME_MANUAL_WINDOW_INSIDE_PLANES);
    gGameSubWindowOutsideControl_0400104a = ReplaceWindowPlanes(
        gGameSubWindowOutsideControl_0400104a,
        GAME_MANUAL_WINDOW_OUTSIDE_PLANES);
}
