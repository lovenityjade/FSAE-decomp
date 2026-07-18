#ifndef FSAE_GAME_INPUT_STATE_H
#define FSAE_GAME_INPUT_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct GameButtonState {
    uint16_t pressed;
    uint16_t released;
    uint16_t held;
} GameButtonState;

typedef struct GameTouchRawSample {
    uint16_t x;
    uint16_t y;
    uint16_t touch;
    uint16_t validity;
} GameTouchRawSample;

typedef struct GameTouchState {
    uint16_t x;
    uint16_t y;
    uint8_t held;
    uint8_t pressed;
    uint8_t released;
    uint8_t padding_07;
} GameTouchState;

typedef struct GameTouchSampler {
    GameTouchRawSample samples[9]; /* target +0x00 */
    uint8_t touch_count;           /* target +0x48 */
    uint8_t release_count;         /* target +0x49 */
    uint8_t active;                /* target +0x4a */
    uint8_t padding_4b;
    uint16_t last_raw_x;           /* target +0x4c */
    uint16_t last_raw_y;           /* target +0x4e */
} GameTouchSampler;

#if UINTPTR_MAX == UINT32_MAX
typedef char GameButtonStateTargetSizeCheck[
    sizeof(GameButtonState) == 6 ? 1 : -1];
typedef char GameTouchRawSampleTargetSizeCheck[
    sizeof(GameTouchRawSample) == 8 ? 1 : -1];
typedef char GameTouchStateTargetSizeCheck[
    sizeof(GameTouchState) == 8 ? 1 : -1];
typedef char GameTouchSamplerTargetSizeCheck[
    sizeof(GameTouchSampler) == 0x50 ? 1 : -1];
typedef char GameTouchSamplerActiveOffsetCheck[
    offsetof(GameTouchSampler, active) == 0x4a ? 1 : -1];
#endif

void GameButtonState_Update(GameButtonState *state); /* 0x020be9f0 */
void GameTouchSampler_Init(GameTouchSampler *sampler); /* 0x020bea60 */
void GameTouchSampler_Start(GameTouchSampler *sampler); /* 0x020beaa4 */
bool GameTouchSampler_Update(
    GameTouchSampler *sampler,
    GameTouchState *state); /* 0x020beb00 */

/* Hardware/SDK edges retained outside the recovered game module. */
extern volatile uint16_t gGameKeyInputRegister_04000130;
extern volatile uint16_t gGameXyButtonState_02ffffa8;
void TP_Init(void);
int32_t TP_GetUserInfo(void *calibration);
void TP_SetCalibrateParam(const void *calibration);
void TP_RequestAutoSamplingStartAsync(
    uint32_t vcount,
    uint32_t frequency,
    GameTouchRawSample *samples,
    uint32_t sample_count);
void TP_WaitBusy(uint32_t command);
void TP_CheckError(uint32_t command);
int32_t TP_GetLatestIndexInAuto(void);
void TP_GetCalibratedPoint(
    GameTouchRawSample *calibrated,
    const GameTouchRawSample *raw);

#endif
