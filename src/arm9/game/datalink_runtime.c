#include "game/datalink_runtime.h"

#include "game/datalink_slot_refresh_all.h"

#include <stddef.h>

extern void Game_ConstructDatalinkRuntimeBase(Game_DatalinkRuntime *runtime);

/* Calls the first virtual method of the interface at the target address. */
extern void Game_InitializeDatalinkInterface(uint32_t interface_address);

extern void Game_InitializeDatalinkResource(int resource_id);

extern void Game_FinalizeDatalinkRuntimeBase(
    Game_DatalinkRuntime *runtime,
    uint32_t primary_interface_address
);

_Static_assert(
    offsetof(Game_DatalinkStaticArena, guard_004) == 0x04,
    "Datalink static guard 04 offset"
);
_Static_assert(
    offsetof(Game_DatalinkStaticArena, guard_014) == 0x14,
    "Datalink static guard 14 offset"
);
_Static_assert(
    offsetof(Game_DatalinkStaticArena, guard_018) == 0x18,
    "Datalink static guard 18 offset"
);
_Static_assert(
    offsetof(Game_DatalinkStaticArena, interface_a_vtable_050) == 0x50,
    "Datalink interface A offset"
);
_Static_assert(
    offsetof(Game_DatalinkStaticArena, interface_c_primary_vtable_0b0) == 0xB0,
    "Datalink interface C primary offset"
);
_Static_assert(
    offsetof(Game_DatalinkStaticArena, interface_c_secondary_vtable_0e0) == 0xE0,
    "Datalink interface C secondary offset"
);
_Static_assert(
    offsetof(Game_DatalinkStaticArena, interface_b_vtable_160) == 0x160,
    "Datalink interface B offset"
);

_Static_assert(
    offsetof(Game_DatalinkRuntime, field_04) == 0x04,
    "Datalink runtime field 04 offset"
);
_Static_assert(
    offsetof(Game_DatalinkRuntime, interface_a_34) == 0x34,
    "Datalink runtime interface A offset"
);
_Static_assert(
    offsetof(Game_DatalinkRuntime, interface_b_38) == 0x38,
    "Datalink runtime interface B offset"
);
_Static_assert(
    offsetof(Game_DatalinkRuntime, interface_c_3c) == 0x3C,
    "Datalink runtime interface C offset"
);
_Static_assert(
    offsetof(Game_DatalinkRuntime, local_player_40) == 0x40,
    "Datalink runtime local player offset"
);
_Static_assert(
    offsetof(Game_DatalinkRuntime, state_41) == 0x41,
    "Datalink runtime state 41 offset"
);
_Static_assert(
    offsetof(Game_DatalinkRuntime, state_43) == 0x43,
    "Datalink runtime state 43 offset"
);
_Static_assert(
    offsetof(Game_DatalinkRuntime, pending_count_48) == 0x48,
    "Datalink runtime pending count offset"
);
_Static_assert(sizeof(Game_DatalinkRuntime) == 0x4C, "Datalink runtime size");

enum {
    DATALINK_STATIC_INITIALIZED = 1,
    DATALINK_INTERFACE_A_VTABLE = 0x02124B40,
    DATALINK_INTERFACE_B_VTABLE = 0x02124B64,
    DATALINK_INTERFACE_C_PRIMARY_VTABLE = 0x021250E0,
    DATALINK_INTERFACE_C_SECONDARY_VTABLE = 0x02125108,
    DATALINK_RUNTIME_RESOURCE_ID = 0x193
};

static void Game_InitializeDatalinkStaticInterfaces(void)
{
    Game_DatalinkStaticArena *arena = &gGameDatalinkStaticArena;

    if ((arena->guard_018 & DATALINK_STATIC_INITIALIZED) == 0U) {
        arena->interface_c_primary_vtable_0b0 =
            DATALINK_INTERFACE_C_PRIMARY_VTABLE;
        arena->interface_c_secondary_vtable_0e0 =
            DATALINK_INTERFACE_C_SECONDARY_VTABLE;
        arena->guard_018 |= DATALINK_STATIC_INITIALIZED;
    }

    if ((arena->guard_014 & DATALINK_STATIC_INITIALIZED) == 0U) {
        arena->interface_a_vtable_050 = DATALINK_INTERFACE_A_VTABLE;
        arena->guard_014 |= DATALINK_STATIC_INITIALIZED;
    }

    if ((arena->guard_004 & DATALINK_STATIC_INITIALIZED) == 0U) {
        arena->guard_004 |= DATALINK_STATIC_INITIALIZED;
        arena->interface_b_vtable_160 = DATALINK_INTERFACE_B_VTABLE;
    }
}

/*
 * 0x020A74FC..0x020A75F3 (Ghidra/catalog body size: 248 bytes).
 * Literal pool: 0x020A75F4..0x020A761B; next function: 0x020A761C.
 *
 * Static arena proof:
 * - base 0x0217E2D4, guards +0x04/+0x14/+0x18;
 * - interface A at +0x50 = 0x0217E324;
 * - interface C at +0xB0 = 0x0217E384, secondary vtable at +0xE0;
 * - interface B at +0x160 = 0x0217E434.
 *
 * Runtime proof:
 * - target addresses are stored at +0x34/+0x38/+0x3C and their first virtual
 *   methods are called in A/B/C order;
 * - +0x04, +0x41, +0x43 and +0x48 are cleared;
 * - local player byte 0x02171DFC is copied to +0x40;
 * - resource 0x193 is initialized, all three Datalink slots are refreshed,
 *   then the base finalizer receives interface A.
 */
void Game_ConstructDatalinkRuntime(Game_DatalinkRuntime *runtime)
{
    Game_InitializeDatalinkStaticInterfaces();
    Game_ConstructDatalinkRuntimeBase(runtime);

    runtime->interface_a_34 = GAME_DATALINK_INTERFACE_A_ADDRESS;
    runtime->interface_b_38 = GAME_DATALINK_INTERFACE_B_ADDRESS;
    runtime->field_04 = 0;
    runtime->interface_c_3c = GAME_DATALINK_INTERFACE_C_ADDRESS;

    Game_InitializeDatalinkInterface(runtime->interface_a_34);
    Game_InitializeDatalinkInterface(runtime->interface_b_38);
    Game_InitializeDatalinkInterface(runtime->interface_c_3c);

    runtime->local_player_40 = gGameLocalPlayerSlot;
    runtime->state_41 = 0;
    runtime->pending_count_48 = 0;
    runtime->state_43 = 0;

    Game_InitializeDatalinkResource(DATALINK_RUNTIME_RESOURCE_ID);
    Game_RefreshAllDatalinkPlayerSlots();
    Game_FinalizeDatalinkRuntimeBase(runtime, runtime->interface_a_34);
}
