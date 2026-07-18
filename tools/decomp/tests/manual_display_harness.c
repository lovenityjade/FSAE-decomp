#include "game/manual_display.h"

#include "ntmv/m2d/scroll_controls.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

volatile uint16_t gGameMainWindow0Registers_04000040[3];
volatile uint16_t gGameSubWindow0Registers_04001040[3];
volatile uint32_t gGameSubDisplayControl_04001000;

static uint8_t sViewerBytes[0x40620];
static uint8_t sModePanel[0xae];
static NtmvM2dScrollIndicator sIndicator;
static uintptr_t sBlendRegisters[2];
static int32_t sBlendValues[2];
static unsigned int sBlendCount;
static NtmvM2dPoint sPosition;
static int16_t sRepositionHeight;
static unsigned int sShutdownStep;
static unsigned int sUnmountCount;
static void *sUnmounted[2];

void TP_RequestAutoSamplingStopAsync(void)
{
    assert(sShutdownStep++ == 0);
}

void TP_WaitBusy(uint32_t command)
{
    assert(command == 4);
    assert(sShutdownStep++ == 1);
}

void TP_CheckError(uint32_t command)
{
    assert(command == 4);
    assert(sShutdownStep++ == 2);
}

void NNS_SndStopSoundAll(void)
{
    assert(sShutdownStep++ == 3);
}

void GX_DispOff(void)
{
    assert(sShutdownStep++ == 4);
}

void GX_SetDefaultDMA(uint32_t dma_channel)
{
    assert(dma_channel == UINT32_MAX);
    assert(sShutdownStep++ == 5);
}

void GX_Init(void)
{
    assert(sShutdownStep++ == 6);
}

bool GX_VBlankIntr(bool enabled)
{
    assert(enabled);
    assert(sShutdownStep++ == 7);
    return false;
}

void Game_SetExternalDisplayState_0218a860(uint32_t state)
{
    assert(state == 0);
    assert(sShutdownStep++ == 8);
}

void NNS_FndUnmountArchive(void *archive)
{
    assert(sUnmountCount < 2);
    sUnmounted[sUnmountCount++] = archive;
}

static void StorePointer(uint32_t offset, const void *pointer)
{
    uintptr_t value = (uintptr_t)pointer;
    uint32_t index;
    for (index = 0; index < sizeof(void *); ++index) {
        sViewerBytes[offset + index] =
            (uint8_t)(value >> (index * 8));
    }
}

void G2x_SetBlendBrightness_(
    uintptr_t register_address,
    uint32_t plane_mask,
    int32_t brightness)
{
    assert(plane_mask == 0x28);
    assert(sBlendCount < 2);
    sBlendRegisters[sBlendCount] = register_address;
    sBlendValues[sBlendCount] = brightness;
    ++sBlendCount;
}

static void SetPosition(
    NtmvM2dUIElement *element,
    const NtmvM2dPoint *position)
{
    (void)element;
    sPosition = *position;
}

void NtmvM2dScrollIndicator_RepositionChildren(
    NtmvM2dScrollIndicator *indicator,
    int16_t height)
{
    assert(indicator == &sIndicator);
    sRepositionHeight = height;
}

static const NtmvM2dUIElementVTable sVTable = {
    0,
    0,
    SetPosition,
    0,
    0,
};

int main(void)
{
    int16_t *history = (int16_t *)(sViewerBytes + 0x40616);

    memset(sViewerBytes, 0xff, sizeof(sViewerBytes));
    memset(sModePanel, 0, sizeof(sModePanel));
    memset(&sIndicator, 0, sizeof(sIndicator));
    StorePointer(0x358, sModePanel);
    StorePointer(0x364, &sIndicator);
    sIndicator.base.base.vtable = &sVTable;
    sIndicator.base.base.local_position.x = 33;

    assert(GameManualViewer_GetPointerContentTop_020bc574(
        (const NtmvM2dManualTocPanel *)sModePanel) == 0x94);
    sModePanel[0xad] = 1;
    assert(GameManualViewer_GetPointerContentTop_020bc574(
        (const NtmvM2dManualTocPanel *)sModePanel) == 0);
    sModePanel[0xad] = 0;

    GameManualViewer_ResetInputHistory(
        (GameManualViewerDisplay *)sViewerBytes);
    assert(history[0] == 0);
    assert(history[1] == 0);
    assert(history[2] == 0);
    assert(*(int16_t *)(sViewerBytes + 0x4061c) == 0);

    sBlendCount = 0;
    GameManualViewer_SetBlendBrightness(
        (GameManualViewerDisplay *)sViewerBytes, -7);
    assert(sBlendCount == 2);
    assert(sBlendRegisters[0] == 0x04000050u);
    assert(sBlendRegisters[1] == 0x04001050u);
    assert(sBlendValues[0] == -7 && sBlendValues[1] == -7);
    sModePanel[0xad] = 1;
    sBlendCount = 0;
    GameManualViewer_SetBlendBrightness(
        (GameManualViewerDisplay *)sViewerBytes, 5);
    assert(sBlendCount == 1);
    assert(sBlendRegisters[0] == 0x04001050u);

    sModePanel[0xad] = 0;
    GameManualViewer_SetRevealWindowOffset(
        (GameManualViewerDisplay *)sViewerBytes, 16);
    assert(gGameMainWindow0Registers_04000040[0] == 0x00f0);
    assert(gGameMainWindow0Registers_04000040[2] == 0x0094);
    assert(gGameSubWindow0Registers_04001040[0] == 0x00f0);
    assert(gGameSubWindow0Registers_04001040[2] == 0x12c0);
    GameManualViewer_SetRevealWindowOffset(
        (GameManualViewerDisplay *)sViewerBytes, -8);
    assert(gGameMainWindow0Registers_04000040[0] == 0x0800);

    GameManualViewer_SetScrollIndicatorHeight(
        (GameManualViewerDisplay *)sViewerBytes, true);
    assert(sPosition.x == 33);
    assert(sPosition.y == 105);
    assert(sRepositionHeight == 0xae);
    GameManualViewer_SetScrollIndicatorHeight(
        (GameManualViewerDisplay *)sViewerBytes, false);
    assert(sPosition.y == 179);
    assert(sRepositionHeight == 0x142);

    sViewerBytes[0x40622] = 1;
    sViewerBytes[0x40623] = 1;
    sViewerBytes[0x40624] = 0;
    sViewerBytes[0x80] = 1;
    sViewerBytes[0xec] = 1;
    gGameSubDisplayControl_04001000 = 0x12350000u;
    sShutdownStep = 0;
    sUnmountCount = 0;
    GameManualViewer_Shutdown((GameManualViewerDisplay *)sViewerBytes);
    assert(sShutdownStep == 9);
    assert((gGameSubDisplayControl_04001000 & 0x00010000u) == 0);
    assert(sUnmountCount == 2);
    assert(sUnmounted[0] == sViewerBytes + 0x18);
    assert(sUnmounted[1] == sViewerBytes + 0x84);
    return 0;
}
