#include "game/manual_graphics.h"

#include "game/graphics_state.h"
#include "game/manual_display.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

volatile uint16_t gGamePowerControl_04000304;
volatile uint16_t gGameMainBgControl_04000008[4];
volatile uint16_t gGameSubBg3Control_0400100e;
volatile uint16_t gGameMainWindowInsideControl_04000048;
volatile uint16_t gGameMainWindowOutsideControl_0400004a;
volatile uint16_t gGameSubWindowInsideControl_04001048;
volatile uint16_t gGameSubWindowOutsideControl_0400104a;
volatile uint32_t gGameMainDisplayControl_04000000;
volatile uint32_t gGameSubDisplayControl_04001000;

static char sEvents[20];
static unsigned int sEventCount;
static unsigned int sDisplayBitsCalls;
static unsigned int sBrightnessCalls;

static void Event(char event)
{
    assert(sEventCount + 1 < sizeof(sEvents));
    sEvents[sEventCount++] = event;
    sEvents[sEventCount] = '\0';
}

void GX_SetGraphicsMode(
    uint32_t display_mode,
    uint32_t background_mode,
    uint32_t background_0_mode)
{
    assert(display_mode == 1);
    assert(background_mode == 3);
    assert(background_0_mode == 0);
    Event('M');
}

void GXS_SetGraphicsMode(uint32_t background_mode)
{
    assert(background_mode == 3);
    Event('s');
}

void GX_SetBankForBG(uint32_t bank)
{
    assert(bank == 3);
    Event('B');
}

void GX_SetBankForSubBG(uint32_t bank)
{
    assert(bank == 4);
    Event('b');
}

void GX_SetBankForOBJ(uint32_t bank)
{
    assert(bank == 0x10);
    Event('O');
}

uint32_t Game_GetUiDisplayControlBits(void)
{
    ++sDisplayBitsCalls;
    Event('D');
    return 0x00100010;
}

void GX_SetBankForOBJExtPltt(uint32_t bank)
{
    assert(bank == 0x20);
    Event('E');
}

void GX_SetBankForSubOBJ(uint32_t bank)
{
    assert(bank == 8);
    Event('o');
}

void GX_SetBankForSubOBJExtPltt(uint32_t bank)
{
    assert(bank == 0x100);
    Event('e');
}

void GXx_SetMasterBrightness_(
    uintptr_t register_address,
    int32_t brightness)
{
    assert(sBrightnessCalls < 2);
    assert(register_address == (sBrightnessCalls == 0 ?
        0x0400006cu : 0x0400106cu));
    assert(brightness == 0x10);
    ++sBrightnessCalls;
    Event('H');
}

void Game_SetDefaultMainBlendAlpha(void)
{
    Event('A');
}

int main(void)
{
    unsigned int index;

    gGamePowerControl_04000304 = 0xffff;
    for (index = 0; index < 4; ++index) {
        gGameMainBgControl_04000008[index] = 0xffff;
    }
    gGameSubBg3Control_0400100e = 0xffff;
    gGameMainDisplayControl_04000000 = 0xa5a5a5a5;
    gGameSubDisplayControl_04001000 = 0x5a5a5a5a;
    gGameMainWindowInsideControl_04000048 = 0xa5ff;
    gGameMainWindowOutsideControl_0400004a = 0x5aff;
    gGameSubWindowInsideControl_04001048 = 0xc3ff;
    gGameSubWindowOutsideControl_0400104a = 0x3cff;

    GameManualViewer_SetupGraphics_020bdb34();

    assert(strcmp(sEvents, "MsBbODEoDeHHA") == 0);
    assert(sDisplayBitsCalls == 2);
    assert(sBrightnessCalls == 2);
    assert(gGamePowerControl_04000304 == 0x7fff);
    assert(gGameMainBgControl_04000008[0] == 0xc000);
    assert(gGameMainBgControl_04000008[1] == 0xc401);
    assert(gGameMainBgControl_04000008[2] == 0x8802);
    assert(gGameMainBgControl_04000008[3] == 0x6087);
    assert(gGameSubBg3Control_0400100e == 0x6087);
    assert(gGameMainDisplayControl_04000000 == 0x9395b8b5);
    assert(gGameSubDisplayControl_04001000 == 0x5a5a585a);
    assert(gGameMainWindowInsideControl_04000048 == 0xa5e8);
    assert(gGameMainWindowOutsideControl_0400004a == 0x5af7);
    assert(gGameSubWindowInsideControl_04001048 == 0xc3e8);
    assert(gGameSubWindowOutsideControl_0400104a == 0x3cf7);
    return 0;
}
