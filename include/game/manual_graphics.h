#ifndef FSAE_GAME_MANUAL_GRAPHICS_H
#define FSAE_GAME_MANUAL_GRAPHICS_H

#include <stdint.h>

void GameManualViewer_ClearGraphicsMemory_020bdabc(void);
void GameManualViewer_SetupGraphics_020bdb34(void);

extern volatile uint16_t gGamePowerControl_04000304;
extern volatile uint16_t gGameMainBgControl_04000008[4];
extern volatile uint16_t gGameSubBg3Control_0400100e;
extern volatile uint16_t gGameMainWindowInsideControl_04000048;
extern volatile uint16_t gGameMainWindowOutsideControl_0400004a;
extern volatile uint16_t gGameSubWindowInsideControl_04001048;
extern volatile uint16_t gGameSubWindowOutsideControl_0400104a;

#endif
