#include "game/datalink_tween_math.h"

#include <assert.h>

const int16_t FX_SinCosTable_[8192] = {
    [0] = 0,
    [1024] = 2896,
    [2048] = 4096,
    [3072] = 2896,
    [4096] = 0
};

static void TestStartAndEndPoints(void)
{
    const Game_DatalinkVector2 start = {10, 30};
    const Game_DatalinkVector2 end = {50, 70};
    Game_DatalinkVector2 output = {0, 0};

    Game_DatalinkInterpolateVectorLinear(&output, &start, &end, 0);
    assert(output.x == 10);
    assert(output.y == 30);

    Game_DatalinkInterpolateVectorLinear(&output, &start, &end, 0x1000);
    assert(output.x == 50);
    assert(output.y == 70);
}

static void TestMidpoint(void)
{
    const Game_DatalinkVector2 start = {8, 40};
    const Game_DatalinkVector2 end = {24, 80};
    Game_DatalinkVector2 output = {0, 0};

    Game_DatalinkInterpolateVectorLinear(&output, &start, &end, 0x800);
    assert(output.x == 16);
    assert(output.y == 60);
}

static void TestSignedCoordinates(void)
{
    const Game_DatalinkVector2 start = {-32, 16};
    const Game_DatalinkVector2 end = {32, -16};
    Game_DatalinkVector2 output = {0, 0};

    Game_DatalinkInterpolateVectorLinear(&output, &start, &end, 0x400);
    assert(output.x == -16);
    assert(output.y == 8);
}

static void TestSinusoidalEndpoints(void)
{
    const Game_DatalinkVector2 start = {0, 512};
    const Game_DatalinkVector2 end = {512, 0};
    Game_DatalinkVector2 output = {0, 0};

    Game_DatalinkInterpolateVectorEaseInSine(
        &output, &start, &end, 0);
    assert(output.x == 0);
    assert(output.y == 512);
    Game_DatalinkInterpolateVectorEaseOutSine(
        &output, &start, &end, 0x1000);
    assert(output.x == 512);
    assert(output.y == 0);
}

static void TestSinusoidalMidpointWeights(void)
{
    const Game_DatalinkVector2 start = {0, 0};
    const Game_DatalinkVector2 end = {512, 512};
    Game_DatalinkVector2 output = {0, 0};

    Game_DatalinkInterpolateVectorEaseInSine(
        &output, &start, &end, 0x800);
    assert(output.x == 93);
    assert(output.y == 93);

    Game_DatalinkInterpolateVectorEaseOutSine(
        &output, &start, &end, 0x800);
    assert(output.x == 418);
    assert(output.y == 418);
}

static void TestSinusoidalSignedCoordinates(void)
{
    const Game_DatalinkVector2 start = {-512, -512};
    const Game_DatalinkVector2 end = {512, 512};
    Game_DatalinkVector2 output = {0, 0};

    Game_DatalinkInterpolateVectorEaseInSine(
        &output, &start, &end, 0x800);
    assert(output.x == -326);
    assert(output.y == -326);

    Game_DatalinkInterpolateVectorEaseOutSine(
        &output, &start, &end, 0x800);
    assert(output.x == 324);
    assert(output.y == 324);
}

int main(void)
{
    TestStartAndEndPoints();
    TestMidpoint();
    TestSignedCoordinates();
    TestSinusoidalEndpoints();
    TestSinusoidalMidpointWeights();
    TestSinusoidalSignedCoordinates();
    return 0;
}
