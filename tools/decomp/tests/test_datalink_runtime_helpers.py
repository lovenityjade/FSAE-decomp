from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000
SOURCE = PROJECT_ROOT / "src/arm9/game/datalink_runtime_helpers.c"
HARNESS = Path(__file__).with_name("datalink_runtime_helpers_harness.c")


class DatalinkRuntimeHelpersRecoveryTests(unittest.TestCase):
    def test_rom_bodies_and_boundaries(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        progress = body(0x020ACD74, 0x020ACD98)
        stack_init = body(0x020ACD98, 0x020ACDB0)
        enter_root = body(0x020ACDB0, 0x020ACDD8)
        push_state = body(0x020ACDD8, 0x020ACE40)
        pop_state = body(0x020ACE40, 0x020ACE9C)
        serialize = body(0x020ACE9C, 0x020ACF5C)
        deserialize = body(0x020ACF5C, 0x020AD020)
        self.assertEqual(len(progress), 36)
        self.assertEqual(len(stack_init), 24)
        self.assertEqual(len(enter_root), 40)
        self.assertEqual(len(push_state), 104)
        self.assertEqual(len(pop_state), 92)
        self.assertEqual(len(serialize), 192)
        self.assertEqual(len(deserialize), 196)
        self.assertEqual(
            hashlib.sha256(progress).hexdigest(),
            "b9fe96eca4012a5cf214e1dcfebbd231598736072c3baa023703266241fd7f7d",
        )
        self.assertEqual(
            hashlib.sha256(stack_init).hexdigest(),
            "162735a098386e9a1c4ed66cba48944fb0976052fe5354410b39987983a1f392",
        )
        self.assertEqual(
            hashlib.sha256(enter_root).hexdigest(),
            "b44a297b6c761d37e6017028fce6840cdd412af6c9e78d0ce180113931c0f2a4",
        )
        self.assertEqual(
            hashlib.sha256(push_state).hexdigest(),
            "bc3ee27305dfe0f1727a90f1b64087f8a508f201ee6b84647b91dc9136c5e71b",
        )
        self.assertEqual(
            hashlib.sha256(pop_state).hexdigest(),
            "314f6f4f93dcce2f1632c64340a90c9dcbef0e1f80293734b10173aca2278d23",
        )
        self.assertEqual(
            hashlib.sha256(serialize).hexdigest(),
            "8f1af1e30f03ff3de47ab9b4922247d876e083746288013217965cea727a805d",
        )
        self.assertEqual(
            hashlib.sha256(deserialize).hexdigest(),
            "7c0c54c9e38443097c224749891ebb918906140a493dadd72f0b8e353c031512",
        )

        # No literal pool separates these bodies.  The prior return and the
        # following state's push/enter helper are deliberately excluded.
        self.assertEqual(read_u32(0x020ACD70), 0xE8BD8038)
        self.assertEqual(read_u32(0x020ACD74), 0xE92D4008)
        self.assertEqual(read_u32(0x020ACD78), 0xE5901028)
        self.assertEqual(read_u32(0x020ACD7C), 0xE3510000)
        self.assertEqual(read_u32(0x020ACD80), 0x03A00A01)
        self.assertEqual(read_u32(0x020ACD84), 0x08BD8008)
        self.assertEqual(read_u32(0x020ACD88), 0xE590002C)
        self.assertEqual(read_u32(0x020ACD8C), 0xE1A01641)
        self.assertEqual(read_u32(0x020ACD90), 0xEBFD82EC)
        self.assertEqual(read_u32(0x020ACD94), 0xE8BD8008)

        self.assertEqual(read_u32(0x020ACD98), 0xE3A01000)
        self.assertEqual(read_u32(0x020ACD9C), 0xE3A02007)
        self.assertEqual(read_u32(0x020ACDA0), 0xE5802004)
        self.assertEqual(read_u32(0x020ACDA4), 0xE5801008)
        self.assertEqual(read_u32(0x020ACDA8), 0xE5C0102C)
        self.assertEqual(read_u32(0x020ACDAC), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020ACDB0), 0xE92D4008)
        self.assertEqual(read_u32(0x020ACDD4), 0xE8BD8008)
        self.assertEqual(read_u32(0x020ACDD8), 0xE92D4038)
        self.assertEqual(read_u32(0x020ACE3C), 0xE8BD8038)
        self.assertEqual(read_u32(0x020ACE40), 0xE92D4010)
        self.assertEqual(read_u32(0x020ACE48), 0xE1D402DC)
        self.assertEqual(read_u32(0x020ACE4C), 0xE0840100)
        self.assertEqual(read_u32(0x020ACE50), 0xE590000C)
        self.assertEqual(read_u32(0x020ACE58), 0xE5911010)
        self.assertEqual(read_u32(0x020ACE5C), 0xE12FFF31)
        self.assertEqual(read_u32(0x020ACE60), 0xE1D402DC)
        self.assertEqual(read_u32(0x020ACE68), 0xE0840100)
        self.assertEqual(read_u32(0x020ACE70), 0xE5801008)
        self.assertEqual(read_u32(0x020ACE78), 0xE2400001)
        self.assertEqual(read_u32(0x020ACE7C), 0xE5C4002C)
        self.assertEqual(read_u32(0x020ACE80), 0xE1D402DC)
        self.assertEqual(read_u32(0x020ACE88), 0xE590000C)
        self.assertEqual(read_u32(0x020ACE90), 0xE591100C)
        self.assertEqual(read_u32(0x020ACE94), 0xE12FFF31)
        self.assertEqual(read_u32(0x020ACE98), 0xE8BD8010)
        self.assertEqual(read_u32(0x020ACE9C), 0xE92D41F0)
        self.assertEqual(read_u32(0x020ACEB8), 0xE2880004)
        self.assertEqual(read_u32(0x020ACEBC), 0xEBFDA3DE)
        self.assertEqual(read_u32(0x020ACECC), 0xE5871000)
        self.assertEqual(read_u32(0x020ACEE4), 0xE288002C)
        self.assertEqual(read_u32(0x020ACEF4), 0xEBFDA3D0)
        self.assertEqual(read_u32(0x020ACF04), 0xE5870000)
        self.assertEqual(read_u32(0x020ACF08), 0xE1D802DC)
        self.assertEqual(read_u32(0x020ACF18), 0xE28D5000)
        self.assertEqual(read_u32(0x020ACF20), 0xE590100C)
        self.assertEqual(read_u32(0x020ACF34), 0xEBFDA3C0)
        self.assertEqual(read_u32(0x020ACF50), 0x3AFFFFF1)
        self.assertEqual(read_u32(0x020ACF58), 0xE8BD81F0)

        self.assertEqual(read_u32(0x020ACF5C), 0xE92D41F0)
        self.assertEqual(read_u32(0x020ACF78), 0xE2881004)
        self.assertEqual(read_u32(0x020ACF7C), 0xEBFDA3AE)
        self.assertEqual(read_u32(0x020ACF90), 0xE5870000)
        self.assertEqual(read_u32(0x020ACFA8), 0xE59D0004)
        self.assertEqual(read_u32(0x020ACFB0), 0xE5880008)
        self.assertEqual(read_u32(0x020ACFB8), 0xEBFDA39F)
        self.assertEqual(read_u32(0x020ACFCC), 0xE1D802DC)
        self.assertEqual(read_u32(0x020ACFDC), 0xE28D5000)
        self.assertEqual(read_u32(0x020ACFEC), 0xEBFDA392)
        self.assertEqual(read_u32(0x020AD008), 0xE580100C)
        self.assertEqual(read_u32(0x020AD014), 0x3AFFFFF1)
        self.assertEqual(read_u32(0x020AD01C), 0xE8BD81F0)
        self.assertEqual(read_u32(0x020AD020), 0xE92D4070)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "datalink_runtime_helpers_test"
            subprocess.run(
                [
                    compiler,
                    "-std=c11",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-pedantic",
                    "-I",
                    str(PROJECT_ROOT / "include"),
                    str(SOURCE),
                    str(HARNESS),
                    "-o",
                    str(executable),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )
            subprocess.run([str(executable)], check=True, cwd=PROJECT_ROOT)

    def test_arm946es_syntax(self) -> None:
        compiler = shutil.which("clang")
        if compiler is None:
            self.skipTest("clang is unavailable")
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "datalink_runtime_helpers.o"
            subprocess.run(
                [
                    compiler,
                    "--target=arm-none-eabi",
                    "-mcpu=arm946e-s",
                    "-marm",
                    "-std=c11",
                    "-Wall",
                    "-Wextra",
                    "-Werror",
                    "-pedantic",
                    "-ffreestanding",
                    "-I",
                    str(PROJECT_ROOT / "include"),
                    "-c",
                    str(SOURCE),
                    "-o",
                    str(output),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )


if __name__ == "__main__":
    unittest.main()
