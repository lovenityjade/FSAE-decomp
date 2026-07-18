#include "game/datalink_renderer_preamble.h"

#include "game/datalink_phase_ten_finalizer.h"
#include "game/datalink_slot_readiness.h"

#include <stddef.h>

enum {
    DATALINK_RENDERED_SLOT_COUNT = 3
};

/* ROM table 0x020DE7E4, copied into FUN_020A8904's stack frame. */
static const Game_DatalinkRenderDescriptor
    sDatalinkRenderDescriptors[GAME_DATALINK_RENDER_DESCRIPTOR_COUNT] = {
        {0x15, 6, 0, UINT32_C(0x0209CA38)},
        {0x16, 5, 0, UINT32_C(0x0209CAB0)},
        {0x08, 6, 0, UINT32_C(0x0209CAF0)},
        {0x0A, 6, 0, UINT32_C(0x0209CB40)},
        {0x02, 5, 0, UINT32_C(0x0209CB90)},
        {0x0D, 6, 0, UINT32_C(0x0209CBE0)},
        {0x0F, 6, 0, UINT32_C(0x0209CC30)},
        {0x1A, 4, 0, UINT32_C(0x0209CC80)},
        {0x1B, 4, 0, UINT32_C(0x0209CD18)},
        {0x1C, 4, 0, UINT32_C(0x0209CDB0)}
    };

/* Opaque 0x020AD964 scene-object command. */
extern void Game_ApplyDatalinkSceneCommand(
    Game_DatalinkSceneOwner *scene,
    int command
);

_Static_assert(
    sizeof(Game_DatalinkRenderDescriptor) == 8,
    "datalink render descriptor stride"
);
_Static_assert(
    offsetof(Game_DatalinkRenderDescriptor, callback_address_04) == 4,
    "datalink render callback address offset"
);
_Static_assert(
    sizeof(sDatalinkRenderDescriptors) == 0x50,
    "datalink ROM render descriptor table size"
);

/*
 * Semantic extraction of the descriptor-copy operation in
 * Range: 0x020A8904..0x020A8933.  The machine range also contains the parent
 * function prologue and stack setup; this helper represents only its coherent
 * 80-byte ROM-to-local data copy.  It is not a Ghidra function boundary.
 */
void Game_CopyDatalinkRenderDescriptors(
    Game_DatalinkRenderDescriptor
        descriptors[GAME_DATALINK_RENDER_DESCRIPTOR_COUNT]
)
{
    volatile Game_DatalinkRenderDescriptor *destination = descriptors;
    int index;

    for (index = 0;
         index < GAME_DATALINK_RENDER_DESCRIPTOR_COUNT;
         ++index) {
        destination[index].callback_argument_00 =
            sDatalinkRenderDescriptors[index].callback_argument_00;
        destination[index].tile_offset_01 =
            sDatalinkRenderDescriptors[index].tile_offset_01;
        destination[index].reserved_02 =
            sDatalinkRenderDescriptors[index].reserved_02;
        destination[index].callback_address_04 =
            sDatalinkRenderDescriptors[index].callback_address_04;
    }
}

/*
 * Semantic internal helper for 0x020A8934..0x020A895F inside the catalogued
 * FUN_020A8904.  Slots zero through two are queried in ascending order.  Each
 * ready slot immediately issues the scene command whose value equals that
 * slot index; unavailable slots have no command.  This helper is not promoted
 * in the Ghidra function registry.
 */
void Game_IssueReadyDatalinkSlotCommands(void)
{
    int slot;

    for (slot = 0; slot < DATALINK_RENDERED_SLOT_COUNT; ++slot) {
        if (Game_IsDatalinkPlayerSlotReady(slot) != 0) {
            Game_ApplyDatalinkSceneCommand(
                &gGameDatalinkSceneOwner,
                slot
            );
        }
    }
}
