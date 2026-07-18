#include "game/input_state.h"

enum {
    GAME_KEY_RIGHT = 0x10,
    GAME_KEY_LEFT = 0x20,
    GAME_KEY_UP = 0x40,
    GAME_KEY_DOWN = 0x80,
    GAME_KEY_MASK = 0x2fff,
    GAME_TOUCH_STABLE_SAMPLES = 3
};

/* 0x020be9f0 */
void GameButtonState_Update(GameButtonState *state)
{
    uint16_t held = (uint16_t)(
        ~(gGameKeyInputRegister_04000130 |
          gGameXyButtonState_02ffffa8) & GAME_KEY_MASK);
    uint16_t changed;

    if ((held & GAME_KEY_LEFT) != 0) {
        held = (uint16_t)(held & (uint16_t)~GAME_KEY_RIGHT);
    }
    if ((held & GAME_KEY_UP) != 0) {
        held = (uint16_t)(held & (uint16_t)~GAME_KEY_DOWN);
    }

    changed = (uint16_t)(state->held ^ held);
    state->pressed = (uint16_t)(changed & held);
    state->released = (uint16_t)(state->held & changed);
    state->held = held;
}

/* 0x020bea60 */
void GameTouchSampler_Init(GameTouchSampler *sampler)
{
    int32_t index;

    sampler->touch_count = 0;
    sampler->release_count = 0;
    sampler->active = 0;
    sampler->last_raw_x = 0;
    sampler->last_raw_y = 0;
    for (index = 0; index < 9; ++index) {
        sampler->samples[index].touch = 0;
        sampler->samples[index].validity = 0;
        sampler->samples[index].x = 0;
        sampler->samples[index].y = 0;
    }
}

/* 0x020beaa4 */
void GameTouchSampler_Start(GameTouchSampler *sampler)
{
    uint32_t calibration[2];

    TP_Init();
    if (TP_GetUserInfo(calibration) != 0) {
        TP_SetCalibrateParam(calibration);
    }
    TP_RequestAutoSamplingStartAsync(0, 4, sampler->samples, 9);
    TP_WaitBusy(2);
    TP_CheckError(2);
}

static uint8_t SaturatingIncrement3(uint8_t value)
{
    if (value < GAME_TOUCH_STABLE_SAMPLES) {
        ++value;
    }
    return value;
}

/* 0x020beb00 */
bool GameTouchSampler_Update(
    GameTouchSampler *sampler,
    GameTouchState *state)
{
    uint8_t was_active = sampler->active;
    int32_t latest = TP_GetLatestIndexInAuto();
    int32_t offset;
    GameTouchRawSample raw;
    GameTouchRawSample calibrated;

    for (offset = 0; offset < 4; ++offset) {
        int32_t index = latest + offset - 3;
        GameTouchRawSample *sample;
        if (index < 0) {
            index += 9;
        }
        sample = &sampler->samples[index];

        if (sample->touch == 0) {
            sampler->touch_count = 0;
            sampler->release_count =
                SaturatingIncrement3(sampler->release_count);
            if (sampler->release_count >= GAME_TOUCH_STABLE_SAMPLES) {
                sampler->active = 0;
            }
        } else {
            sampler->release_count = 0;
            sampler->touch_count =
                SaturatingIncrement3(sampler->touch_count);
            if (sampler->touch_count >= GAME_TOUCH_STABLE_SAMPLES) {
                sampler->active = 1;
            }
            if (sampler->active != 0 && sample->validity == 0) {
                sampler->last_raw_x = sample->x;
                sampler->last_raw_y = sample->y;
            }
        }
    }

    state->held = sampler->active;
    state->pressed = (uint8_t)(was_active == 0 && sampler->active != 0);
    state->released = (uint8_t)(was_active != 0 && sampler->active == 0);

    raw.x = sampler->last_raw_x;
    raw.y = sampler->last_raw_y;
    raw.touch = 1;
    raw.validity = 0;
    TP_GetCalibratedPoint(&calibrated, &raw);
    state->x = calibrated.x;
    state->y = calibrated.y;
    return state->pressed != 0;
}
