#include "game/datalink_controller.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "game/datalink_tween_math.h"

static const Game_DatalinkTweenRecord *sExpectedTween;
static int32_t sProgress;
static int sProgressCallCount;
static int sInterpolationCallCount;

int32_t Game_GetDatalinkTweenProgress(
    const Game_DatalinkTweenRecord *tween)
{
    assert(tween == sExpectedTween);
    ++sProgressCallCount;
    return sProgress;
}

static void TestInterpolationCallback(
    Game_DatalinkVector2 *output,
    const Game_DatalinkVector2 *start,
    const Game_DatalinkVector2 *end,
    int progress)
{
    assert(output != NULL);
    assert(start == (const Game_DatalinkVector2 *)(const void *)
        &sExpectedTween->start_x_fx_14);
    assert(end == (const Game_DatalinkVector2 *)(const void *)
        &sExpectedTween->target_x_fx_1c);
    assert(progress == sProgress);
    output->x = start->x + 0x1234;
    output->y = end->y - 0x5678;
    ++sInterpolationCallCount;
}

void MI_CpuFill8(void *destination, uint8_t value, uint32_t size)
{
    assert(destination != NULL);
    assert(value == 0);
    assert(size == 0x58);
    memset(destination, value, size);
}

static void TestUpdateDatalinkTween(void)
{
    Game_DatalinkTweenRecord tween;
    uintptr_t callback_address = (uintptr_t)&TestInterpolationCallback;

    /* The target stores a 32-bit callback. A high PIE host address cannot be
     * represented by the recovered record, so only the ROM-layout tests run
     * on such hosts. */
    if (callback_address > UINT32_MAX) {
        return;
    }

    memset(&tween, 0, sizeof(tween));
    tween.interpolation_callback_address_10 = (uint32_t)callback_address;
    tween.start_x_fx_14 = 0x100000;
    tween.start_y_fx_18 = -0x200000;
    tween.target_x_fx_1c = 0x300000;
    tween.target_y_fx_20 = -0x400000;
    tween.duration_fx_28 = 0x3000;
    tween.elapsed_fx_2c = 0x0800;
    tween.current_x_fx_30 = 0x555555;
    tween.current_y_fx_34 = 0x666666;
    sExpectedTween = &tween;
    sProgress = 0x0600;
    sProgressCallCount = 0;
    sInterpolationCallCount = 0;

    assert(!Game_UpdateDatalinkTween(&tween, 0x1000));
    assert(tween.elapsed_fx_2c == 0x1800);
    assert(tween.current_x_fx_30 == 0x101234);
    assert(tween.current_y_fx_34 == -0x405678);
    assert(sProgressCallCount == 1);
    assert(sInterpolationCallCount == 1);

    sProgress = 0x1000;
    assert(Game_UpdateDatalinkTween(&tween, 0x2000));
    assert(tween.elapsed_fx_2c == tween.duration_fx_28);
    assert(sProgressCallCount == 2);
    assert(sInterpolationCallCount == 2);
}

int main(void)
{
    Game_DatalinkTweenRecord tween;
    unsigned char expected[0x58];
    unsigned int index;

    assert(sizeof(tween) == 0x58);
    assert(offsetof(Game_DatalinkTweenRecord, slot_index_00) == 0x00);
    assert(offsetof(Game_DatalinkTweenRecord, initialized_01) == 0x01);
    assert(offsetof(Game_DatalinkTweenRecord, active_03) == 0x03);
    assert(
        offsetof(
            Game_DatalinkTweenRecord,
            interpolation_callback_address_10) == 0x10);
    assert(offsetof(Game_DatalinkTweenRecord, duration_fx_28) == 0x28);
    assert(offsetof(Game_DatalinkTweenRecord, elapsed_fx_2c) == 0x2c);

    memset(&tween, 0xa5, sizeof(tween));
    memset(expected, 0, sizeof(expected));
    expected[0x00] = 0x7f;
    expected[0x01] = 1;
    expected[0x03] = 1;
    expected[0x10] = 0xb4;
    expected[0x11] = 0xca;
    expected[0x12] = 0x0a;
    expected[0x13] = 0x02;
    Game_InitializeDatalinkTweenRecord_020acbf4(&tween, 0x7f);
    assert(memcmp(&tween, expected, sizeof(expected)) == 0);

    memset(&tween, 0xa5, sizeof(tween));
    expected[0x00] = 0xff;
    Game_InitializeDatalinkTweenRecord_020acbf4(&tween, 0xff);
    assert(memcmp(&tween, expected, sizeof(expected)) == 0);

    memset(&tween, 0xa5, sizeof(tween));
    tween.duration_fx_28 = 100;
    tween.elapsed_fx_2c = 20;
    assert(!Game_AdvanceDatalinkTween_020acc7c(&tween, 30));
    assert(tween.elapsed_fx_2c == 50);

    assert(Game_AdvanceDatalinkTween_020acc7c(&tween, 50));
    assert(tween.elapsed_fx_2c == 100);

    tween.elapsed_fx_2c = 90;
    assert(Game_AdvanceDatalinkTween_020acc7c(&tween, 1000));
    assert(tween.elapsed_fx_2c == 100);

    tween.elapsed_fx_2c = 50;
    assert(!Game_AdvanceDatalinkTween_020acc7c(&tween, -20));
    assert(tween.elapsed_fx_2c == 30);

    tween.duration_fx_28 = 0;
    tween.elapsed_fx_2c = 0;
    assert(Game_AdvanceDatalinkTween_020acc7c(&tween, 0));
    assert(tween.elapsed_fx_2c == 0);

    memset(&tween, 0xa5, sizeof(tween));
    tween.current_x_fx_30 = -0x123450;
    tween.current_y_fx_34 = 0x6789a0;
    Game_ConfigureDatalinkTween(
        &tween, 0x111000, -0x222000, 7, 0);
    assert(tween.interpolation_callback_address_10 == 0x020acab4);
    assert(tween.start_x_fx_14 == -0x123450);
    assert(tween.start_y_fx_18 == 0x6789a0);
    assert(tween.target_x_fx_1c == 0x111000);
    assert(tween.target_y_fx_20 == -0x222000);
    assert(tween.duration_fx_28 == 7 * 0x1000);
    assert(tween.elapsed_fx_2c == 0);
    assert(tween.current_x_fx_30 == -0x123450);
    assert(tween.current_y_fx_34 == 0x6789a0);
    assert(tween.active_03 == 0xa5);
    for (index = 0; index < sizeof(tween.unknown_24); ++index) {
        assert(tween.unknown_24[index] == 0xa5);
    }

    Game_ConfigureDatalinkTween(&tween, 1, 2, 0, 1);
    assert(tween.interpolation_callback_address_10 == 0x020acb04);
    assert(tween.duration_fx_28 == 0);
    Game_ConfigureDatalinkTween(&tween, 3, 4, 0x000fffff, 2);
    assert(tween.interpolation_callback_address_10 == 0x020acb7c);
    assert((uint32_t)tween.duration_fx_28 == 0xfffff000);

    tween.interpolation_callback_address_10 = 0x12345678;
    Game_ConfigureDatalinkTween(&tween, 5, 6, 9, 3);
    assert(tween.interpolation_callback_address_10 == 0x12345678);
    assert(tween.target_x_fx_1c == 5 && tween.target_y_fx_20 == 6);
    assert(tween.duration_fx_28 == 9 * 0x1000);
    assert(tween.elapsed_fx_2c == 0);

    Game_ConfigureDatalinkTween(&tween, 7, 8, 10, -1);
    assert(tween.interpolation_callback_address_10 == 0x12345678);
    assert(tween.target_x_fx_1c == 7 && tween.target_y_fx_20 == 8);
    TestUpdateDatalinkTween();
    return 0;
}
