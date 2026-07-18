#include "game/format_buffer.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static void TestWriteFits(void)
{
    uint8_t storage[8] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
    static const uint8_t source[] = {'A', 'B', 'C'};
    GameFormatBuffer output = {storage, sizeof(storage), 2};

    assert(GameFormatBuffer_Write(&output, source, sizeof(source)) == 1);
    assert(output.length == 5);
    assert(storage[0] == 0xAA && storage[1] == 0xAA);
    assert(memcmp(storage + 2, source, sizeof(source)) == 0);
    assert(storage[5] == 0xAA); /* The callback adds no terminator. */
}

static void TestWriteTruncates(void)
{
    uint8_t storage[5] = {0, 0, 0, 0, 0};
    static const uint8_t source[] = {'W', 'X', 'Y', 'Z'};
    GameFormatBuffer output = {storage, sizeof(storage), 3};

    assert(GameFormatBuffer_Write(&output, source, sizeof(source)) == 1);
    assert(output.length == sizeof(storage));
    assert(storage[3] == 'W');
    assert(storage[4] == 'X');
}

static void TestFullBufferConsumesNothing(void)
{
    uint8_t storage[2] = {0x11, 0x22};
    static const uint8_t source[] = {'Q'};
    GameFormatBuffer output = {storage, sizeof(storage), sizeof(storage)};

    assert(GameFormatBuffer_Write(&output, source, sizeof(source)) == 1);
    assert(output.length == sizeof(storage));
    assert(storage[0] == 0x11 && storage[1] == 0x22);
}

int main(void)
{
    TestWriteFits();
    TestWriteTruncates();
    TestFullBufferConsumesNothing();
    return 0;
}
