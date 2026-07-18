from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCES = (
    PROJECT_ROOT / "src/arm9/game/datalink_lobby_sync.c",
    PROJECT_ROOT / "src/arm9/game/datalink_lobby_network.c",
    PROJECT_ROOT / "src/arm9/game/link_operation_state.c",
)
HARNESS = Path(__file__).with_name("datalink_lobby_sync_harness.c")

FUNCTIONS = (
    ("wireless_loss", 0x020B2190, 0x020B2348, 0x020B235C,
     "08f62707f2d13ea5ae06a74967fe0d3d1154e0ec50490938c1319c9576f034a9",
     "3e167139deac4b4967f921e70312e8190f01064c75a8fffce96b133297af4394",
     (0x02181860, 0x0212C7C8, 0x0217D23C, 0x02171C84, 0x0216F020)),
    ("accumulate_command", 0x020B235C, 0x020B23B0, 0x020B23B8,
     "d7508fe3e4e4a05bccf7272bdc43abfb50647b3eba7ea47675a709564a79f88c",
     "0d967bac44ba124b6fab4d4aaeb7c2e5973463e3e4122cc7d6465aee0f4415dc",
     (0x020DED94, 0x02181860)),
    ("get_command", 0x020B23B8, 0x020B23F4, 0x020B23FC,
     "1fbab50c3e12bc8c4006c3794c20b0fe1fc54e6c338b1a60f26575c8e257e711",
     "77f6a77fcbe8d45bfff89be02ff8e79811372937442c9979085effc4f9773fa0",
     (0x020DEDA4, 0x0217D23C)),
    ("unpack_shared", 0x020B23FC, 0x020B2550, 0x020B2560,
     "c310fdbe7433ed11fdfafc5c44ca33de5a7d76a94790df938bac12cb9f495195",
     "7b3a74070d604b6f98d0d2b97c10b5471cf8ecbc43b01af6333c0ceff971d428",
     (0x020DEDB4, 0x02181860, 0x020DEDC4, 0x0217D348)),
    ("slot_unassigned", 0x020B2560, 0x020B257C, 0x020B2580,
     "c6ec23fdf7a56f63012fc3cca11d8a0364db29c121709702a095e09fb584e5d9",
     "77627ff9b5485162c58f57817df9f05aed6bea6c22984bc92baac720ea5714b9",
     (0x02181860,)),
    ("slots_assigned", 0x020B2580, 0x020B25BC, 0x020B25C0,
     "0a4748356a538c923457fe3a869f6052b747fcdd99e54f10c436b5ebc361889a",
     "6fd2e6677b57ecac443fd8c5e9b2b6a599e5ba85247f88a87b875e7667e7d6d3",
     (0x02181860,)),
    ("players_ready", 0x020B25C0, 0x020B2610, 0x020B2614,
     "a84ab11d7f757e599bb6ba6be855dcb0fd994334edec90336774012dac520b0a",
     "9cde26494fd45073d9f129eabf57a1ae26e4a673427d6b0753e54bdd765453b8",
     (0x02181860,)),
    ("publish_state", 0x020B2614, 0x020B26C8, 0x020B26D0,
     "f39f550d2559ec72b0fb0cfca02f8582d86b2283b6844390378392cf1d82e221",
     "b560a4c9060ccbab83b049a81c06de0b1382fe61c41d7b4232e0ff694549ccbd",
     (0x02181860, 0x0217D348)),
    ("screen_resource", 0x020B26D0, 0x020B2870, 0x020B2898,
     "842ba58c10d51073ea3f4aac1a243057e27786652cf63684fe76b3888dee68a4",
     "2206cc1eb10645a342b4180f6c6450016e29296b535d573bcf41afa372967aec",
     (0x02181860, 0x0000083F, 0x0000084B, 0x00000853, 0x00000852,
      0x00000851, 0x00000841, 0x0000084C, 0x0217D23C, 0x0000084D)),
    ("input_edges", 0x020B2898, 0x020B291C, 0x020B292C,
     "1773df010c9d93c038bf5a02fc9f0f36b5acffa39189319c3f9daa13236466ac",
     "a7b93eceadc410fac6b6893cb76a6ce3a065815db8ac01950af4f19a630171e1",
     (0x0212C9A8, 0x0212C584, 0x0212C58C, 0x0212C514)),
    ("player_names", 0x020B292C, 0x020B2A48, 0x020B2A5C,
     "c30ece34ca1cb1635a8837412915d98fa7ac2983d617fb4476a64fa6d053b390",
     "9f41f5302021800f417b799acb2affc1a3657662b91e394f236eb03dee14d2bd",
     (0x0217D348, 0x0212CA20, 0x02181868, 0x0217A23C, 0x02181860)),
    ("start_link", 0x020B2A5C, 0x020B2A88, 0x020B2A90,
     "249bc26a7260e4134c8702f026a5d4771328f3018bc62585681dfa4de614f777",
     "049612f24c784514c58b0bc48b16452a91dc044b0f79142e6779154bd829f5cf",
     (0x0212C7F0, 0x0212C9A8)),
    ("link_succeeded", 0x020B2A90, 0x020B2AA8, 0x020B2AAC,
     "e578a33b9c14a0d1fee554a2dd77513b2b71be95371ac6d377b9f4c8208bbaf5",
     "d35b2fd4fbe4da4491daac99b1c6f23e436fe2441105e2e50324952e3f8001d6",
     (0x0212C9A8,)),
)

EXPECTED_RANGE_SYMBOLS = {
    0x020B19D8: "Game_UpdateDatalinkMultiplayerSessionFrame_020b19d8",
    0x020B2190: "Game_HandleDatalinkLobbyWirelessLoss_020b2190",
    0x020B235C: "Game_AccumulateDatalinkLobbySlotCommand_020b235c",
    0x020B23B8: "Game_GetDatalinkLobbySlotCommand_020b23b8",
    0x020B23FC: "Game_UnpackDatalinkLobbySharedState_020b23fc",
    0x020B2560: "Game_IsDatalinkLobbySlotUnassigned_020b2560",
    0x020B2580: "Game_AreDatalinkLobbySlotsAssigned_020b2580",
    0x020B25C0: "Game_AreDatalinkLobbyPlayersReady_020b25c0",
    0x020B2614: "Game_PublishDatalinkLobbyStateWord_020b2614",
    0x020B26D0: "Game_UpdateDatalinkLobbyScreenResource_020b26d0",
    0x020B2898: "Game_UpdateDatalinkLobbyInputEdges_020b2898",
    0x020B292C: "Game_SynchronizeDatalinkLobbyPlayerNames_020b292c",
    0x020B2A5C: "Game_StartDatalinkLobbyLinkOperation_020b2a5c",
    0x020B2A90: "Game_LinkOperation_HasSucceeded_020b2a90",
    0x020B2AAC: "Game_LinkOperation_NoOp_020b2aac",
    0x020B2AB0: "Game_CompleteDatalinkLobbyNetworkTransition_020b2ab0",
}


class DatalinkLobbySyncRecoveryTests(unittest.TestCase):
    def test_range_symbols_have_one_source_owner_and_one_config_entry(
        self,
    ) -> None:
        definition = re.compile(
            r"^(?:bool|uint32_t|void)\s+"
            r"(Game_[A-Za-z0-9_]+_(020b[0-9a-f]{4}))\s*"
            r"\([^;]*?\)\s*\{",
            re.MULTILINE | re.DOTALL,
        )
        source_owners: dict[int, list[tuple[str, str]]] = {}
        source_directory = PROJECT_ROOT / "src/arm9/game"
        for source in source_directory.glob("*.c"):
            for symbol, suffix in definition.findall(source.read_text()):
                address = int(suffix, 16)
                if 0x020B19D8 <= address <= 0x020B3293:
                    source_owners.setdefault(address, []).append(
                        (symbol, source.name)
                    )

        self.assertEqual(set(source_owners), set(EXPECTED_RANGE_SYMBOLS))
        for address, expected_name in EXPECTED_RANGE_SYMBOLS.items():
            with self.subTest(source_owner=f"0x{address:08X}"):
                self.assertEqual(len(source_owners[address]), 1)
                self.assertEqual(source_owners[address][0][0], expected_name)

        config = json.loads(
            (PROJECT_ROOT / "config/decomp/arm9-symbols.json").read_text()
        )
        config_entries: dict[int, list[str]] = {}
        for symbol in config["symbols"]:
            address = int(symbol["address"], 16)
            if 0x020B19D8 <= address <= 0x020B3293:
                config_entries.setdefault(address, []).append(symbol["name"])

        self.assertEqual(set(config_entries), set(EXPECTED_RANGE_SYMBOLS))
        for address, expected_name in EXPECTED_RANGE_SYMBOLS.items():
            with self.subTest(config_entry=f"0x{address:08X}"):
                self.assertEqual(config_entries[address], [expected_name])

    def test_rom_bodies_pools_tables_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        total_body_bytes = 0
        for entry in FUNCTIONS:
            name, start, body_end, extent_end, body_sha, extent_sha, pool = entry
            with self.subTest(function=name):
                recovered = body(start, body_end)
                extent = body(start, extent_end)
                total_body_bytes += len(recovered)
                self.assertEqual(hashlib.sha256(recovered).hexdigest(), body_sha)
                self.assertEqual(hashlib.sha256(extent).hexdigest(), extent_sha)
                self.assertEqual(
                    tuple(read_u32(body_end + index * 4)
                          for index in range(len(pool))),
                    pool,
                )
        self.assertEqual(total_body_bytes, 2172)

        anchors = {
            0x020B2190: 0xE92D41F0, 0x020B2344: 0xE8BD81F0,
            0x020B235C: 0xE92D4038, 0x020B23AC: 0xE8BD8038,
            0x020B23B8: 0xE92D4008, 0x020B23F0: 0xE8BD8008,
            0x020B23FC: 0xE92D4038, 0x020B254C: 0xE8BD8038,
            0x020B2560: 0xE59F1014, 0x020B2578: 0xE12FFF1E,
            0x020B2580: 0xE59F0034, 0x020B25B8: 0xE12FFF1E,
            0x020B25C0: 0xE59F0048, 0x020B260C: 0xE12FFF1E,
            0x020B2614: 0xE92D4010, 0x020B26C4: 0xE8BD8010,
            0x020B26D0: 0xE92D40F8, 0x020B286C: 0xE8BD80F8,
            0x020B2898: 0xE92D4070, 0x020B2918: 0xE8BD8070,
            0x020B292C: 0xE92D43F8, 0x020B2A44: 0xE8BD83F8,
            0x020B2A5C: 0xE92D4008, 0x020B2A84: 0xE8BD8008,
            0x020B2A90: 0xE59F0010, 0x020B2AA4: 0xE12FFF1E,
            0x020B2AAC: 0xE12FFF1E,
        }
        for address, expected in anchors.items():
            self.assertEqual(read_u32(address), expected, hex(address))

        self.assertEqual(
            struct.unpack("<4I", body(0x020DED94, 0x020DEDA4)),
            (0, 0, 4, 8),
        )
        self.assertEqual(
            struct.unpack("<4I", body(0x020DEDA4, 0x020DEDB4)),
            (0, 0, 4, 8),
        )
        self.assertEqual(
            struct.unpack("<4I", body(0x020DEDB4, 0x020DEDC4)),
            (0, 0, 2, 4),
        )
        self.assertEqual(
            struct.unpack("<4I", body(0x020DEDC4, 0x020DEDD4)),
            (6, 7, 8, 9),
        )

    def test_host_c_syntax(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        subprocess.run(
            [compiler, "-std=c11", "-Wall", "-Wextra", "-Werror",
             "-pedantic", "-I", str(PROJECT_ROOT / "include"),
             "-fsyntax-only", *(str(source) for source in SOURCES),
             str(HARNESS)],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )

    def test_arm946es_syntax(self) -> None:
        compiler = shutil.which("clang")
        if compiler is None:
            self.skipTest("clang is unavailable")
        subprocess.run(
            [compiler, "--target=arm-none-eabi", "-mcpu=arm946e-s", "-marm",
             "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
             "-ffreestanding", "-I", str(PROJECT_ROOT / "include"),
             "-fsyntax-only", *(str(source) for source in SOURCES)],
            check=True,
            cwd=PROJECT_ROOT,
            env={**os.environ, "CCACHE_DISABLE": "1"},
        )


if __name__ == "__main__":
    unittest.main()
