#include "game/input_state.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

volatile uint16_t gGameKeyInputRegister_04000130;
volatile uint16_t gGameXyButtonState_02ffffa8;

static int32_t sLatest;
static unsigned int sInitCount;
static unsigned int sSetCalibrationCount;
static unsigned int sStartCount;
static unsigned int sWaitCount;
static unsigned int sCheckCount;

void TP_Init(void)
{
    ++sInitCount;
}

int32_t TP_GetUserInfo(void *calibration)
{
    uint32_t *words = calibration;
    words[0] = 0x11223344u;
    words[1] = 0x55667788u;
    return 1;
}

void TP_SetCalibrateParam(const void *calibration)
{
    const uint32_t *words = calibration;
    assert(words[0] == 0x11223344u);
    assert(words[1] == 0x55667788u);
    ++sSetCalibrationCount;
}

void TP_RequestAutoSamplingStartAsync(
    uint32_t vcount,
    uint32_t frequency,
    GameTouchRawSample *samples,
    uint32_t sample_count)
{
    assert(vcount == 0);
    assert(frequency == 4);
    assert(samples != 0);
    assert(sample_count == 9);
    ++sStartCount;
}

void TP_WaitBusy(uint32_t command)
{
    assert(command == 2);
    ++sWaitCount;
}

void TP_CheckError(uint32_t command)
{
    assert(command == 2);
    ++sCheckCount;
}

int32_t TP_GetLatestIndexInAuto(void)
{
    return sLatest;
}

void TP_GetCalibratedPoint(
    GameTouchRawSample *calibrated,
    const GameTouchRawSample *raw)
{
    assert(raw->touch == 1);
    assert(raw->validity == 0);
    *calibrated = *raw;
    calibrated->x = (uint16_t)(raw->x + 10);
    calibrated->y = (uint16_t)(raw->y + 20);
}

static void SetFourSamples(
    GameTouchSampler *sampler,
    uint16_t touch,
    uint16_t validity)
{
    int32_t index;
    for (index = 0; index < 4; ++index) {
        sampler->samples[index].x = (uint16_t)(100 + index);
        sampler->samples[index].y = (uint16_t)(200 + index);
        sampler->samples[index].touch = touch;
        sampler->samples[index].validity = validity;
    }
    sLatest = 3;
}

int main(void)
{
    GameButtonState buttons = {0, 0, 0};
    GameTouchSampler sampler;
    GameTouchState touch;

    gGameKeyInputRegister_04000130 = 0x2fffu;
    gGameXyButtonState_02ffffa8 = 0x2fffu;
    GameButtonState_Update(&buttons);
    assert(buttons.held == 0);

    /* Left/up win over their opposing directions exactly like the ROM. */
    gGameKeyInputRegister_04000130 =
        (uint16_t)(0x2fffu & (uint16_t)~0x00f0u);
    gGameXyButtonState_02ffffa8 = 0;
    GameButtonState_Update(&buttons);
    assert((buttons.held & 0x20u) != 0);
    assert((buttons.held & 0x40u) != 0);
    assert((buttons.held & 0x10u) == 0);
    assert((buttons.held & 0x80u) == 0);
    assert(buttons.pressed == buttons.held);

    gGameKeyInputRegister_04000130 = 0x2fffu;
    gGameXyButtonState_02ffffa8 = 0x2fffu;
    GameButtonState_Update(&buttons);
    assert(buttons.held == 0);
    assert(buttons.released == 0x60u);

    memset(&sampler, 0xff, sizeof(sampler));
    GameTouchSampler_Init(&sampler);
    assert(sampler.samples[8].validity == 0);
    assert(sampler.touch_count == 0);
    assert(sampler.active == 0);
    GameTouchSampler_Start(&sampler);
    assert(sInitCount == 1);
    assert(sSetCalibrationCount == 1);
    assert(sStartCount == 1);
    assert(sWaitCount == 1);
    assert(sCheckCount == 1);

    SetFourSamples(&sampler, 1, 0);
    memset(&touch, 0, sizeof(touch));
    assert(GameTouchSampler_Update(&sampler, &touch));
    assert(touch.held == 1);
    assert(touch.pressed == 1);
    assert(touch.released == 0);
    assert(touch.x == 113);
    assert(touch.y == 223);

    SetFourSamples(&sampler, 0, 0);
    assert(!GameTouchSampler_Update(&sampler, &touch));
    assert(touch.held == 0);
    assert(touch.pressed == 0);
    assert(touch.released == 1);
    /* Last valid raw coordinate is retained while released. */
    assert(touch.x == 113);
    assert(touch.y == 223);
    return 0;
}
