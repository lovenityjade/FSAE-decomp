#ifndef FSAE_GAME_MANUAL_DISPLAY_H
#define FSAE_GAME_MANUAL_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

typedef struct GameManualViewerDisplay GameManualViewerDisplay;
typedef struct NtmvM2dManualTocPanel NtmvM2dManualTocPanel;

int32_t GameManualViewer_GetPointerContentTop_020bc574(
    const NtmvM2dManualTocPanel *toc_panel);

void GameManualViewer_ResetInputHistory(
    GameManualViewerDisplay *viewer); /* 0x020be2f8 */
void GameManualViewer_SetBlendBrightness(
    GameManualViewerDisplay *viewer,
    int32_t brightness); /* 0x020be32c */
void GameManualViewer_SetRevealWindowOffset(
    GameManualViewerDisplay *viewer,
    int32_t horizontal_offset); /* 0x020be370 */
void GameManualViewer_SetScrollIndicatorHeight(
    GameManualViewerDisplay *viewer,
    bool compact_layout); /* 0x020be3e0 */
void GameManualViewer_Shutdown(
    GameManualViewerDisplay *viewer); /* 0x020be244 */

void G2x_SetBlendBrightness_(
    uintptr_t blend_control_register,
    uint32_t plane_mask,
    int32_t brightness);

extern volatile uint16_t gGameMainWindow0Registers_04000040[3];
extern volatile uint16_t gGameSubWindow0Registers_04001040[3];
extern volatile uint32_t gGameMainDisplayControl_04000000;
extern volatile uint32_t gGameSubDisplayControl_04001000;

void TP_RequestAutoSamplingStopAsync(void);
void TP_WaitBusy(uint32_t command);
void TP_CheckError(uint32_t command);
void NNS_SndStopSoundAll(void);
void GX_DispOff(void);
void GX_SetDefaultDMA(uint32_t dma_channel);
void GX_Init(void);
bool GX_VBlankIntr(bool enabled);
bool Game_IsExternalDisplayStateActive_0218a870(void);
void Game_SetExternalDisplayState_0218a860(uint32_t state);
void NNS_FndUnmountArchive(void *archive);

#endif
