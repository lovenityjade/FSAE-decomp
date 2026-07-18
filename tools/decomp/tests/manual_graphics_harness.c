#include "game/manual_graphics.h"

#include <assert.h>
#include <stdint.h>

typedef struct ClearCall {
    uint32_t value;
    uintptr_t destination;
    uint32_t size;
} ClearCall;

static ClearCall sClearCalls[5];
static unsigned int sClearCount;
static unsigned int sEventCount;
static char sEvents[8];

static void Event(char event)
{
    assert(sEventCount < sizeof(sEvents));
    sEvents[sEventCount++] = event;
}

void GX_SetBankForLCDC(uint16_t banks)
{
    assert(banks == 0x1ff);
    Event('B');
}

void GX_DisableBankForLCDC_02015448(void)
{
    Event('D');
}

void MIi_CpuClearFast(
    uint32_t value,
    void *destination,
    uint32_t size)
{
    ClearCall *call;

    assert(sClearCount < 5);
    call = &sClearCalls[sClearCount++];
    call->value = value;
    call->destination = (uintptr_t)destination;
    call->size = size;
    Event('C');
}

int main(void)
{
    static const ClearCall expected[] = {
        {0x00, 0x06800000, 0x000a4000},
        {0xc0, 0x07000000, 0x00000400},
        {0x00, 0x05000000, 0x00000400},
        {0xc0, 0x07000400, 0x00000400},
        {0x00, 0x05000400, 0x00000400},
    };
    unsigned int index;

    GameManualViewer_ClearGraphicsMemory_020bdabc();

    assert(sEventCount == 7);
    assert(sEvents[0] == 'B');
    assert(sEvents[1] == 'C');
    assert(sEvents[2] == 'D');
    assert(sEvents[3] == 'C');
    assert(sEvents[4] == 'C');
    assert(sEvents[5] == 'C');
    assert(sEvents[6] == 'C');
    assert(sClearCount == 5);
    for (index = 0; index < 5; ++index) {
        assert(sClearCalls[index].value == expected[index].value);
        assert(sClearCalls[index].destination == expected[index].destination);
        assert(sClearCalls[index].size == expected[index].size);
    }
    return 0;
}
