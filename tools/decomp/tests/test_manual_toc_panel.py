from __future__ import annotations

import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest


PROJECT_ROOT = Path(__file__).resolve().parents[3]
ARM9_BASE = 0x02004000


class ManualTocPanelRecoveryTests(unittest.TestCase):
    def test_rom_rtti_layout_destroy_render_and_helpers(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        def read_i16s(address: int, count: int) -> tuple[int, ...]:
            return struct.unpack_from(
                "<" + "h" * count, arm9, address - ARM9_BASE
            )

        # Direct UIElement base in the GNU RTTI record.
        self.assertEqual(read_u32(0x02126E68), 0x020DFAE0)
        self.assertEqual(read_u32(0x02126E6C), 0x02126E9C)
        self.assertEqual(read_u32(0x02126E70), 0x02126CE0)
        name_offset = 0x02126E9C - ARM9_BASE
        expected_name = b"N4ntmv3m2d14ManualTocPanelE\0"
        self.assertEqual(
            arm9[name_offset : name_offset + len(expected_name)], expected_name
        )

        # Two overridden slots followed by the three inherited setters.
        self.assertEqual(read_u32(0x02126E80), 0)
        self.assertEqual(read_u32(0x02126E84), 0x02126E68)
        self.assertEqual(
            tuple(read_u32(0x02126E88 + index * 4) for index in range(5)),
            (0x020BA80C, 0x020BA84C, 0x020B7E48, 0x020B7E5C, 0x020B7E70),
        )

        # Constructor address point and independent factory size proof.
        self.assertEqual(read_u32(0x020BA0CC), 0x02126E88)
        self.assertEqual(read_u32(0x020BCD3C), 0xE3A010B0)
        # It constructs embedded UIPanels at +0x20 and +0x3c.
        self.assertEqual(read_u32(0x020BA018), 0xE2840020)
        self.assertEqual(read_u32(0x020BA030), 0xE284003C)
        # Constructor constants at +0x98/+0x9a/+0x9c/+0x9e.
        self.assertEqual(read_u32(0x020BA048), 0xE1C429BA)
        self.assertEqual(read_u32(0x020BA04C), 0xE1C419BC)
        self.assertEqual(read_u32(0x020BA050), 0xE1C409BE)
        self.assertEqual(read_u32(0x020BA068), 0xE1C439B8)

        # Configure (0x020ba0d0) sets the 256x192 root at (128, 96), uses
        # threshold 0x40 to select one of two 0x46-byte button tables, and
        # loads the three table pointers from its literal pool.
        self.assertEqual(read_u32(0x020BA0E8), 0xE3A000C0)
        self.assertEqual(read_u32(0x020BA0EC), 0xE3A02C01)
        self.assertEqual(read_u32(0x020BA108), 0xE3A0A080)
        self.assertEqual(read_u32(0x020BA110), 0xE3A06060)
        self.assertEqual(read_u32(0x020BA11C), 0xE3550040)
        self.assertEqual(read_u32(0x020BA128), 0xE3A00046)
        self.assertEqual(read_u32(0x020BA710), 0x020DEF5C)
        self.assertEqual(read_u32(0x020BA714), 0x020DEF18)
        self.assertEqual(read_u32(0x020BA718), 0x020DEF36)

        # Three 10-byte OBJ-text definitions at 0x020def18.
        self.assertEqual(read_i16s(0x020DEF18, 5), (20, 0, 39, 12, 2))
        self.assertEqual(read_i16s(0x020DEF22, 5), (21, 24, 39, 16, 2))
        self.assertEqual(read_i16s(0x020DEF2C, 5), (22, 24, 40, 16, 2))

        # Two 0x12-byte ItemsPanel definitions at 0x020def36.
        self.assertEqual(
            read_i16s(0x020DEF36, 9),
            (0, -36, 26, 25, 7, -30, 186, 5, 1),
        )
        self.assertEqual(
            read_i16s(0x020DEF48, 9),
            (137, -59, 34, -1, 13, 31, 186, 7, 2),
        )

        compact_buttons = (
            (-118, 1, 18, 18, 37, -1, -1),
            (-60, 1, 93, 18, 27, -1, 0),
            (6, 1, 22, 18, 23, 24, -1),
            (-56, 0, 64, 18, 28, -1, 1),
            (56, 0, 64, 18, 29, -1, 2),
        )
        wide_buttons = (
            (-118, 1, 18, 18, 37, -1, -1),
            (-60, 1, 93, 18, 27, -1, 0),
            (6, 1, 22, 18, 23, 24, -1),
            (-65, 0, 120, 18, 30, -1, 1),
            (65, 0, 120, 18, 31, -1, 2),
        )
        self.assertEqual(
            tuple(read_i16s(0x020DEF5C + index * 0x0E, 7) for index in range(5)),
            compact_buttons,
        )
        self.assertEqual(
            tuple(read_i16s(0x020DEFA2 + index * 0x0E, 7) for index in range(5)),
            wide_buttons,
        )

        # The constructed tree uses a scroll position (75, 0), a content
        # owner at (0, 150), and footer owner at (0, 86).  Final stores set
        # split-mode window origins before enabling control 2's alternate.
        self.assertEqual(read_u32(0x020BA4FC), 0xE3A0004B)
        self.assertEqual(read_u32(0x020BA508), 0xE3A000AE)
        self.assertEqual(read_u32(0x020BA5E4), 0xE3A000AB)
        self.assertEqual(read_u32(0x020BA5EC), 0xE3A00096)
        self.assertEqual(read_u32(0x020BA630), 0xE3A05015)
        self.assertEqual(read_u32(0x020BA66C), 0xE3A07056)
        self.assertEqual(read_u32(0x020BA6D4), 0xE3A01002)
        self.assertEqual(read_u32(0x020BA6E8), 0xE1C92AB4)
        self.assertEqual(read_u32(0x020BA6F8), 0xE1C93AB0)
        self.assertEqual(read_u32(0x020BA6FC), 0xE1C9AAB2)
        self.assertEqual(read_u32(0x020BA700), 0xE1C92AB6)
        self.assertEqual(read_u32(0x020BA704), 0xEB000163)

        # SetTocData has an exact 0x020ba948..0x020ba9a3 boundary.  It stores
        # +0x14 before forwarding record_count +4 to primary ItemsPanel,
        # refreshes rows, takes the empty/non-empty branch, then navigation.
        self.assertEqual(read_u32(0x020BA948), 0xE92D4010)
        self.assertEqual(read_u32(0x020BA950), 0xE5841014)
        self.assertEqual(read_u32(0x020BA954), 0xE1D110B4)
        self.assertEqual(read_u32(0x020BA958), 0xE594005C)
        self.assertEqual(read_u32(0x020BA95C), 0xEBFFFA87)
        self.assertEqual(read_u32(0x020BA964), 0xEB000192)
        self.assertEqual(read_u32(0x020BA968), 0xE5940014)
        self.assertEqual(read_u32(0x020BA96C), 0xE1D000B4)
        self.assertEqual(read_u32(0x020BA970), 0xE3500000)

        # Non-empty: primary item zero, secondary mode true.  Empty uses its
        # dedicated initializer.  Both converge on navigation 0x020bb208.
        self.assertEqual(read_u32(0x020BA97C), 0xE3A01000)
        self.assertEqual(read_u32(0x020BA980), 0xEB0000A9)
        self.assertEqual(read_u32(0x020BA988), 0xE3A01001)
        self.assertEqual(read_u32(0x020BA98C), 0xEB0003C1)
        self.assertEqual(read_u32(0x020BA994), 0xEB0001EB)
        self.assertEqual(read_u32(0x020BA99C), 0xEB000219)
        self.assertEqual(read_u32(0x020BA9A0), 0xE8BD8010)
        self.assertEqual(read_u32(0x020BA9A4), 0xE92D4FF0)

        # HandlePointer is one complete 0x27c-byte transaction ending at
        # 0x020bac1f.  It clears the two-halfword result before the hidden and
        # animation guards, then gives controls priority over both lists.
        self.assertEqual(read_u32(0x020BA9A8), 0xE24DD014)
        self.assertEqual(read_u32(0x020BA9B8), 0xE1C960B0)
        self.assertEqual(read_u32(0x020BA9BC), 0xE1C960B2)
        self.assertEqual(read_u32(0x020BA9C0), 0xE5DA1010)
        self.assertEqual(read_u32(0x020BA9D4), 0xEB000355)
        self.assertEqual(read_u32(0x020BAA10), 0xEB00021E)
        self.assertEqual(read_u32(0x020BAA4C), 0xEBFFFA96)
        self.assertEqual(read_u32(0x020BAA84), 0xE3560002)

        # The list-action priority is primary page, secondary page, primary
        # selection, secondary selection, then the two activation cases.
        self.assertEqual(read_u32(0x020BAA98), 0xE5DD0008)
        self.assertEqual(read_u32(0x020BAABC), 0xE5DD000D)
        self.assertEqual(read_u32(0x020BAAD4), 0xE5DD0009)
        self.assertEqual(read_u32(0x020BAB1C), 0xE5DD000E)
        self.assertEqual(read_u32(0x020BAB50), 0xE5DD000A)
        self.assertEqual(read_u32(0x020BABC8), 0xE5DD000F)
        self.assertEqual(read_u32(0x020BAB6C), 0xE59A2014)
        self.assertEqual(read_u32(0x020BAB80), 0xE1D601B4)
        self.assertEqual(read_u32(0x020BAB9C), 0xEBFFF8D5)

        # An otherwise unhandled press/release is captured only inside the
        # embedded content panel.  The epilogue and following accessor prove
        # the exact boundary at 0x020bac20.
        self.assertEqual(read_u32(0x020BABD8), 0xE5D80005)
        self.assertEqual(read_u32(0x020BABE4), 0xE5D80006)
        self.assertEqual(read_u32(0x020BAC08), 0xEBFFF469)
        self.assertEqual(read_u32(0x020BAC1C), 0xE8BD8FF0)
        self.assertEqual(read_u32(0x020BAC20), 0xE590005C)
        self.assertEqual(read_u32(0x020BAC24), 0xE1D002FA)
        self.assertEqual(read_u32(0x020BAC28), 0xE12FFF1E)

        # SelectPrimaryItem is exactly 0x020bac2c..0x020bac57.  An unchanged
        # ItemsPanel selection returns early; a change synchronizes its page
        # and always resets the secondary selection.
        self.assertEqual(read_u32(0x020BAC2C), 0xE92D4010)
        self.assertEqual(read_u32(0x020BAC34), 0xE594005C)
        self.assertEqual(read_u32(0x020BAC38), 0xEBFFF9F9)
        self.assertEqual(read_u32(0x020BAC3C), 0xE3500000)
        self.assertEqual(read_u32(0x020BAC40), 0x08BD8010)
        self.assertEqual(read_u32(0x020BAC44), 0xE594005C)
        self.assertEqual(read_u32(0x020BAC48), 0xEBFFFA0A)
        self.assertEqual(read_u32(0x020BAC50), 0xEB00013C)
        self.assertEqual(read_u32(0x020BAC54), 0xE8BD8010)
        self.assertEqual(read_u32(0x020BAC58), 0xE5900060)

        # The secondary accessor is the separate 0x020bac58..60 unit.
        self.assertEqual(read_u32(0x020BAC5C), 0xE1D002FA)
        self.assertEqual(read_u32(0x020BAC60), 0xE12FFF1E)

        # SelectSecondaryItem spans exactly 0x020bac64..94.  It exits if the
        # selection is unchanged, then exits again if the selected item stays
        # on the current page; only a page move refreshes secondary rows.
        self.assertEqual(read_u32(0x020BAC64), 0xE92D4010)
        self.assertEqual(read_u32(0x020BAC6C), 0xE5940060)
        self.assertEqual(read_u32(0x020BAC70), 0xEBFFF9EB)
        self.assertEqual(read_u32(0x020BAC74), 0xE3500000)
        self.assertEqual(read_u32(0x020BAC78), 0x08BD8010)
        self.assertEqual(read_u32(0x020BAC7C), 0xE5940060)
        self.assertEqual(read_u32(0x020BAC80), 0xEBFFF9FC)
        self.assertEqual(read_u32(0x020BAC84), 0xE3500000)
        self.assertEqual(read_u32(0x020BAC88), 0x08BD8010)
        self.assertEqual(read_u32(0x020BAC90), 0xEB0000FA)
        self.assertEqual(read_u32(0x020BAC94), 0xE8BD8010)
        self.assertEqual(read_u32(0x020BAC98), 0xE5C010AD)

        # SetSecondaryState is the eight-instruction tail-call unit ending in
        # the literal at 0x020bacb4.  It stores +0xad and passes !active to
        # ManualButton_SetAlternateAnimation at 0x020bbb70.
        self.assertEqual(read_u32(0x020BAC9C), 0xE31100FF)
        self.assertEqual(read_u32(0x020BACA0), 0x03A01001)
        self.assertEqual(read_u32(0x020BACA4), 0xE590006C)
        self.assertEqual(read_u32(0x020BACA8), 0xE59FC004)
        self.assertEqual(read_u32(0x020BACAC), 0x13A01000)
        self.assertEqual(read_u32(0x020BACB0), 0xE12FFF1C)
        self.assertEqual(read_u32(0x020BACB4), 0x020BBB70)

        # The following update routine starts independently at 0x020bacb8.
        self.assertEqual(read_u32(0x020BACB8), 0xE92D4030)
        self.assertEqual(read_u32(0x020BACBC), 0xE24DD014)

        # Update phase 1 moves content Y by at most 32 toward 0/150 according
        # to +0xad, then anchors footer Y at 236-contentY.
        self.assertEqual(read_u32(0x020BACC4), 0xE5D500AD)
        self.assertEqual(read_u32(0x020BACCC), 0xE1D542F6)
        self.assertEqual(read_u32(0x020BACE0), 0xE2540020)
        self.assertEqual(read_u32(0x020BACF8), 0xE2850020)
        self.assertEqual(read_u32(0x020BAD08), 0xE3540096)
        self.assertEqual(read_u32(0x020BAD10), 0xE2840020)
        self.assertEqual(read_u32(0x020BAD20), 0xC3A04096)
        self.assertEqual(read_u32(0x020BAD44), 0xE26410EC)
        self.assertEqual(read_u32(0x020BAD60), 0xE12FFF32)

        # Phase 2 moves secondary X toward +0x9a by at most 32, derives
        # primary X as min(secondaryX-137, 0), then updates the window target
        # only on arrival.
        self.assertEqual(read_u32(0x020BAD64), 0xE5950060)
        self.assertEqual(read_u32(0x020BAD68), 0xE1D529FA)
        self.assertEqual(read_u32(0x020BAD94), 0xE3500020)
        self.assertEqual(read_u32(0x020BADD0), 0xE12FFF32)
        self.assertEqual(read_u32(0x020BADD8), 0xE2441089)
        self.assertEqual(read_u32(0x020BADE4), 0xC3A01000)
        self.assertEqual(read_u32(0x020BAE00), 0xE12FFF32)
        self.assertEqual(read_u32(0x020BAE14), 0xEB000290)

        # Phase 3 advances the first window X toward +0x9e with the same
        # signed 32-pixel clamp.
        self.assertEqual(read_u32(0x020BAE18), 0xE1D51AF0)
        self.assertEqual(read_u32(0x020BAE1C), 0xE1D509FE)
        self.assertEqual(read_u32(0x020BAE44), 0xE3510020)
        self.assertEqual(read_u32(0x020BAE64), 0xE1C50AB0)

        # Phase 4 animates primary rows with the exact TOC callback literal;
        # +0xae gates marker/secondary visibility based on scroll completion.
        self.assertEqual(read_u32(0x020BAE68), 0xE595005C)
        self.assertEqual(read_u32(0x020BAE70), 0xE59F1078)
        self.assertEqual(read_u32(0x020BAE74), 0xEBFFFBDD)
        self.assertEqual(read_u32(0x020BAE7C), 0x15D500AE)
        self.assertEqual(read_u32(0x020BAE90), 0xE1D013F8)
        self.assertEqual(read_u32(0x020BAE94), 0xE1D003FA)
        self.assertEqual(read_u32(0x020BAEB0), 0xE12FFF32)
        self.assertEqual(read_u32(0x020BAEC4), 0xE12FFF32)
        self.assertEqual(read_u32(0x020BAEF0), 0x020B9FAC)

        # Phase 5 ticks controls 1..4 in order.  The epilogue at 0x020baeec
        # and next prologue at 0x020baef4 prove Update's exact boundary.
        self.assertEqual(read_u32(0x020BAEC8), 0xE5950068)
        self.assertEqual(read_u32(0x020BAED0), 0xE595006C)
        self.assertEqual(read_u32(0x020BAED8), 0xE5950070)
        self.assertEqual(read_u32(0x020BAEE0), 0xE5950074)
        self.assertEqual(read_u32(0x020BAEEC), 0xE8BD8030)
        self.assertEqual(read_u32(0x020BAEF4), 0xE92D4008)

        # PublishWindowRegisters spans 0x020baef4..0x020bafac, followed by its
        # 0x1ff literal.  It preserves DISPCNT bits 11..12, toggles only the
        # three window bits and publishes all three packed origins.
        self.assertEqual(read_u32(0x020BAEFC), 0xE3A02301)
        self.assertEqual(read_u32(0x020BAF08), 0xE2022C1F)
        self.assertEqual(read_u32(0x020BAF14), 0x0382C007)
        self.assertEqual(read_u32(0x020BAF1C), 0x13C2C007)
        self.assertEqual(read_u32(0x020BAF28), 0xE3C22C1F)
        self.assertEqual(read_u32(0x020BAF30), 0xE5812000)
        self.assertEqual(read_u32(0x020BAF34), 0xE1D0CAF2)
        self.assertEqual(read_u32(0x020BAF38), 0xE1D0EAF0)
        self.assertEqual(read_u32(0x020BAF58), 0xE5812010)
        self.assertEqual(read_u32(0x020BAF5C), 0xE1D0CAF6)
        self.assertEqual(read_u32(0x020BAF60), 0xE1D0EAF4)
        self.assertEqual(read_u32(0x020BAF80), 0xE5812014)
        self.assertEqual(read_u32(0x020BAF84), 0xE1D02AFA)
        self.assertEqual(read_u32(0x020BAF88), 0xE1D0CAF8)
        self.assertEqual(read_u32(0x020BAF9C), 0xE20330FF)
        self.assertEqual(read_u32(0x020BAFA8), 0xE5810018)
        self.assertEqual(read_u32(0x020BAFAC), 0xE8BD8008)
        self.assertEqual(read_u32(0x020BAFB0), 0x000001FF)

        # RefreshPrimaryRows is exactly 0x020bafb4..0x020bb07f.  It bounds
        # record_count-first_visible by visible_rows, builds 0x18-byte source
        # records into eight-byte target rows and calls ItemsPanel_UpdateRows.
        self.assertEqual(read_u32(0x020BAFB4), 0xE92D41F0)
        self.assertEqual(read_u32(0x020BAFB8), 0xE24DD038)
        self.assertEqual(read_u32(0x020BAFBC), 0xE5901014)
        self.assertEqual(read_u32(0x020BAFC4), 0xE1D140B4)
        self.assertEqual(read_u32(0x020BAFD4), 0xE590105C)
        self.assertEqual(read_u32(0x020BAFD8), 0xE1D132F2)
        self.assertEqual(read_u32(0x020BAFDC), 0xE1D122B4)
        self.assertEqual(read_u32(0x020BAFF8), 0xE1540821)
        self.assertEqual(read_u32(0x020BAFFC), 0x91A02004)
        self.assertEqual(read_u32(0x020BB00C), 0xE59F6068)
        self.assertEqual(read_u32(0x020BB02C), 0xE0288C9E)
        self.assertEqual(read_u32(0x020BB030), 0xE598E004)
        self.assertEqual(read_u32(0x020BB040), 0x07876181)
        self.assertEqual(read_u32(0x020BB044), 0xE1D8E1B4)
        self.assertEqual(read_u32(0x020BB05C), 0xE5CE8004)
        self.assertEqual(read_u32(0x020BB068), 0xE590005C)
        self.assertEqual(read_u32(0x020BB070), 0xEBFFF9F3)
        self.assertEqual(read_u32(0x020BB078), 0xE8BD81F0)
        self.assertEqual(read_u32(0x020BB07C), 0x02126EFC)
        self.assertEqual(read_i16s(0x02126EFC, 6), (40, 45, 45, 45, 41, 0))
        self.assertEqual(read_u32(0x020BB080), 0xE92D4070)

        # RefreshSecondaryRows spans 0x020bb080..0x020bb147.  With a primary
        # selection it follows record.child_texts +0x10, bounds child_count by
        # the secondary viewport and emits simple rows with alternate=false.
        self.assertEqual(read_u32(0x020BB084), 0xE24DD038)
        self.assertEqual(read_u32(0x020BB088), 0xE590105C)
        self.assertEqual(read_u32(0x020BB090), 0xE1D152FA)
        self.assertEqual(read_u32(0x020BB0A0), 0xE155000C)
        self.assertEqual(read_u32(0x020BB0A8), 0xE5901014)
        self.assertEqual(read_u32(0x020BB0AC), 0xE5904060)
        self.assertEqual(read_u32(0x020BB0B8), 0xE1012185)
        self.assertEqual(read_u32(0x020BB0BC), 0xE1D432F2)
        self.assertEqual(read_u32(0x020BB0C0), 0xE1D121B4)
        self.assertEqual(read_u32(0x020BB0C4), 0xE1D442B4)
        self.assertEqual(read_u32(0x020BB0DC), 0xE1540822)
        self.assertEqual(read_u32(0x020BB0E4), 0x91A02004)
        self.assertEqual(read_u32(0x020BB0F4), 0xE59F4048)
        self.assertEqual(read_u32(0x020BB100), 0xE591C010)
        self.assertEqual(read_u32(0x020BB108), 0xE79CC106)
        self.assertEqual(read_u32(0x020BB10C), 0xE785C186)
        self.assertEqual(read_u32(0x020BB118), 0x07854186)
        self.assertEqual(read_u32(0x020BB124), 0xE5CCE004)
        self.assertEqual(read_u32(0x020BB130), 0xE5900060)
        self.assertEqual(read_u32(0x020BB138), 0xEBFFFA86)
        self.assertEqual(read_u32(0x020BB140), 0xE8BD8070)
        self.assertEqual(read_u32(0x020BB144), 0x02126EFC)
        self.assertEqual(read_u32(0x020BB148), 0xE92D4038)

        # InitializeEmptySelection spans 0x020bb148..0x020bb1bb.  It derives
        # the secondary count from the selected 0x18-byte record, refreshes
        # the list and selects child zero only for a zero +0x00 marker.
        self.assertEqual(read_u32(0x020BB150), 0xE595005C)
        self.assertEqual(read_u32(0x020BB158), 0xE1D022FA)
        self.assertEqual(read_u32(0x020BB168), 0x0A000009)
        self.assertEqual(read_u32(0x020BB16C), 0xE5951014)
        self.assertEqual(read_u32(0x020BB170), 0xE3A00018)
        self.assertEqual(read_u32(0x020BB178), 0xE1001082)
        self.assertEqual(read_u32(0x020BB17C), 0xE1D011B4)
        self.assertEqual(read_u32(0x020BB188), 0xE5900000)
        self.assertEqual(read_u32(0x020BB190), 0x03A04001)
        self.assertEqual(read_u32(0x020BB194), 0xE5950060)
        self.assertEqual(read_u32(0x020BB198), 0xEBFFF878)
        self.assertEqual(read_u32(0x020BB1A0), 0xEBFFFFB6)
        self.assertEqual(read_u32(0x020BB1A8), 0x08BD8038)
        self.assertEqual(read_u32(0x020BB1B0), 0xE3A01000)
        self.assertEqual(read_u32(0x020BB1B4), 0xEBFFFEAA)
        self.assertEqual(read_u32(0x020BB1B8), 0xE8BD8038)
        self.assertEqual(read_u32(0x020BB1BC), 0xE590C070)

        # Destructor dispatches footer +0x3c, then content +0x20.
        self.assertEqual(read_u32(0x020BA814), 0xE285003C)
        self.assertEqual(read_u32(0x020BA828), 0xE2850020)
        # Render stores content-relative Y at +0xa2 and +0xaa.
        self.assertEqual(read_u32(0x020BA8FC), 0xE1C50AB2)
        self.assertEqual(read_u32(0x020BA900), 0xE1C50ABA)
        # Split mode writes X=0x40 at +0xa4; both branches write Y at +0xa6.
        self.assertEqual(read_u32(0x020BA92C), 0x03A01040)
        self.assertEqual(read_u32(0x020BA930), 0x01C51AB4)
        self.assertEqual(read_u32(0x020BA934), 0x01C50AB6)
        self.assertEqual(read_u32(0x020BA93C), 0x11C50AB6)

        # Navigation is a separate leaf at 0x020bb1bc..0x020bb207.  It loads
        # controls +0x70/+0x74 and edits only lock bit 1 before the next
        # function's prologue at 0x020bb208.
        self.assertEqual(read_u32(0x020BB1BC), 0xE590C070)
        self.assertEqual(read_u32(0x020BB1C4), 0xE5DC3010)
        self.assertEqual(read_u32(0x020BB1C8), 0x03A01001)
        self.assertEqual(read_u32(0x020BB1D4), 0xE1A01F81)
        self.assertEqual(read_u32(0x020BB1E8), 0xE5902074)
        self.assertEqual(read_u32(0x020BB1D8), 0xE3C32002)
        self.assertEqual(read_u32(0x020BB1F8), 0xE3C11002)
        self.assertEqual(read_u32(0x020BB200), 0xE5C20010)
        self.assertEqual(read_u32(0x020BB204), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020BB208), 0xE92D4070)

        # UpdateNavigation spans 0x020bb208..0x020bb28f.  Its next direction
        # first checks for another primary record, then the current record's
        # remaining children; previous uses the initial-child helper only for
        # primary record zero.
        self.assertEqual(read_u32(0x020BB210), 0xE595005C)
        self.assertEqual(read_u32(0x020BB218), 0xE1D022FA)
        self.assertEqual(read_u32(0x020BB224), 0xBA000015)
        self.assertEqual(read_u32(0x020BB228), 0xE595C014)
        self.assertEqual(read_u32(0x020BB230), 0xE59C3000)
        self.assertEqual(read_u32(0x020BB234), 0xE3A00018)
        self.assertEqual(read_u32(0x020BB238), 0xE0203092)
        self.assertEqual(read_u32(0x020BB23C), 0xE1DC30B4)
        self.assertEqual(read_u32(0x020BB244), 0xE1D442FA)
        self.assertEqual(read_u32(0x020BB24C), 0xA1D031B4)
        self.assertEqual(read_u32(0x020BB258), 0xB3A01001)
        self.assertEqual(read_u32(0x020BB264), 0xCA000002)
        self.assertEqual(read_u32(0x020BB268), 0xEBFFF722)
        self.assertEqual(read_u32(0x020BB270), 0xDA000001)
        self.assertEqual(read_u32(0x020BB284), 0xE1A02006)
        self.assertEqual(read_u32(0x020BB288), 0xEBFFFFCB)
        self.assertEqual(read_u32(0x020BB28C), 0xE8BD8070)
        self.assertEqual(read_u32(0x020BB290), 0xE92D41F0)

        # HandleControls is exactly 0x020bb290..0x020bb44f.  It clears the
        # two-halfword action, captures one of nine controls on press, tracks
        # hover while active and dispatches a nine-entry table on release.
        self.assertEqual(read_u32(0x020BB294), 0xE24DD008)
        self.assertEqual(read_u32(0x020BB29C), 0xE5D62004)
        self.assertEqual(read_u32(0x020BB2B0), 0xE1C750B0)
        self.assertEqual(read_u32(0x020BB2B4), 0xE1C750B2)
        self.assertEqual(read_u32(0x020BB2BC), 0x0A00002C)
        self.assertEqual(read_u32(0x020BB2C0), 0xE5D60005)
        self.assertEqual(read_u32(0x020BB2D4), 0xE3A07009)
        self.assertEqual(read_u32(0x020BB2E0), 0xE2880064)
        self.assertEqual(read_u32(0x020BB2EC), 0xEBFFF6E5)
        self.assertEqual(read_u32(0x020BB2F0), 0xE247100A)
        self.assertEqual(read_u32(0x020BB300), 0xE5911064)
        self.assertEqual(read_u32(0x020BB318), 0xE1C809B8)
        self.assertEqual(read_u32(0x020BB33C), 0xEB000E97)
        self.assertEqual(read_u32(0x020BB340), 0xE1D809F8)
        self.assertEqual(read_u32(0x020BB36C), 0xEBFFF2C8)
        self.assertEqual(read_u32(0x020BB374), 0xE5D61006)
        self.assertEqual(read_u32(0x020BB38C), 0xE1C829B8)
        self.assertEqual(read_u32(0x020BB3A8), 0x0A000025)
        self.assertEqual(read_u32(0x020BB3B4), 0xE3C22004)
        self.assertEqual(read_u32(0x020BB3BC), 0x908FF101)
        self.assertEqual(
            tuple(read_u32(0x020BB3C0 + index * 4) for index in range(10)),
            (
                0xEA00001F,
                0xEA000007,
                0xEA00000A,
                0xEA00000C,
                0xEA000012,
                0xEA000014,
                0xEA000019,
                0xEA000018,
                0xEA000013,
                0xEA000014,
            ),
        )
        self.assertEqual(read_u32(0x020BB3F0), 0xEB00009A)
        self.assertEqual(read_u32(0x020BB3FC), 0xEB0000C2)
        self.assertEqual(read_u32(0x020BB40C), 0xEB0000AD)
        self.assertEqual(read_u32(0x020BB420), 0xEB00000A)
        self.assertEqual(read_u32(0x020BB42C), 0xEB00004E)
        self.assertEqual(read_u32(0x020BB440), 0xEB0000D0)
        self.assertEqual(read_u32(0x020BB44C), 0xE8BD81F0)
        self.assertEqual(read_u32(0x020BB450), 0xE92D40F8)

        # The six dispatch helpers are contiguous and independently bounded:
        # previous 0x450..56b, next 0x56c..65f, guide state 0x660..697,
        # guarded guide request 0x698..6c7, secondary state 0x6c8..70b and
        # center activation 0x70c..72f.
        self.assertEqual(read_u32(0x020BB458), 0xE5D600AD)
        self.assertEqual(read_u32(0x020BB468), 0x18BD80F8)
        self.assertEqual(read_u32(0x020BB470), 0xE1D052FA)
        self.assertEqual(read_u32(0x020BB490), 0xE1D011B4)
        self.assertEqual(read_u32(0x020BB494), 0xE1D272FA)
        self.assertEqual(read_u32(0x020BB4A0), 0xEBFFF694)
        self.assertEqual(read_u32(0x020BB4C4), 0xEBFFFDD8)
        self.assertEqual(read_u32(0x020BB4F4), 0xEBFFFDDA)
        self.assertEqual(read_u32(0x020BB500), 0xEB0000E4)
        self.assertEqual(read_u32(0x020BB530), 0xEB000112)
        self.assertEqual(read_u32(0x020BB538), 0xEBFFFF32)
        self.assertEqual(read_u32(0x020BB544), 0xE1C011B8)
        self.assertEqual(read_u32(0x020BB560), 0xEB000E0E)
        self.assertEqual(read_u32(0x020BB568), 0xE8BD80F8)
        self.assertEqual(read_u32(0x020BB56C), 0xE92D4038)

        self.assertEqual(read_u32(0x020BB574), 0xE5D510AD)
        self.assertEqual(read_u32(0x020BB584), 0x18BD8038)
        self.assertEqual(read_u32(0x020BB590), 0xE1D112FA)
        self.assertEqual(read_u32(0x020BB5AC), 0xE1D231B4)
        self.assertEqual(read_u32(0x020BB5BC), 0xE1D222FA)
        self.assertEqual(read_u32(0x020BB5D4), 0xE1D020B4)
        self.assertEqual(read_u32(0x020BB5EC), 0xEBFFFD8E)
        self.assertEqual(read_u32(0x020BB5F8), 0xEB0000A6)
        self.assertEqual(read_u32(0x020BB60C), 0xEBFFFD94)
        self.assertEqual(read_u32(0x020BB624), 0xEB0000D5)
        self.assertEqual(read_u32(0x020BB62C), 0xEBFFFEF5)
        self.assertEqual(read_u32(0x020BB638), 0xE1C011B8)
        self.assertEqual(read_u32(0x020BB654), 0xEB000DD1)
        self.assertEqual(read_u32(0x020BB65C), 0xE8BD8038)
        self.assertEqual(read_u32(0x020BB660), 0xE92D4010)

        self.assertEqual(read_u32(0x020BB668), 0xE5D400AC)
        self.assertEqual(read_u32(0x020BB67C), 0xEB0000BF)
        self.assertEqual(read_u32(0x020BB68C), 0xEB000DC3)
        self.assertEqual(read_u32(0x020BB690), 0xE5D400AC)
        self.assertEqual(read_u32(0x020BB694), 0xE8BD8010)
        self.assertEqual(read_u32(0x020BB698), 0xE92D4038)

        self.assertEqual(read_u32(0x020BB6A4), 0xEB000021)
        self.assertEqual(read_u32(0x020BB6B0), 0x18BD8038)
        self.assertEqual(read_u32(0x020BB6B8), 0xEBFFFFE8)
        self.assertEqual(read_u32(0x020BB6BC), 0xE5C40000)
        self.assertEqual(read_u32(0x020BB6C4), 0xE8BD8038)
        self.assertEqual(read_u32(0x020BB6C8), 0xE92D4010)

        self.assertEqual(read_u32(0x020BB6D0), 0xE5D400AD)
        self.assertEqual(read_u32(0x020BB6E4), 0xEBFFFD6B)
        self.assertEqual(read_u32(0x020BB6E8), 0xE594006C)
        self.assertEqual(read_u32(0x020BB6F0), 0xE1C011B8)
        self.assertEqual(read_u32(0x020BB700), 0xEB000DA6)
        self.assertEqual(read_u32(0x020BB708), 0xE8BD8010)
        self.assertEqual(read_u32(0x020BB70C), 0xE5901068)

        self.assertEqual(read_u32(0x020BB714), 0xE1C121B8)
        self.assertEqual(read_u32(0x020BB71C), 0xE59FC008)
        self.assertEqual(read_u32(0x020BB728), 0xE12FFF1C)
        self.assertEqual(read_u32(0x020BB72C), 0x020BEDA0)

        # IsAnimating is exactly 0x020bb730..0x020bb787.  It accepts both
        # vertical endpoints, then compares secondary X and primary scroll.
        self.assertEqual(read_u32(0x020BB730), 0xE1D012F6)
        self.assertEqual(read_u32(0x020BB74C), 0xB1A02003)
        self.assertEqual(read_u32(0x020BB754), 0x05901060)
        self.assertEqual(read_u32(0x020BB758), 0x01D029FA)
        self.assertEqual(read_u32(0x020BB76C), 0x0590005C)
        self.assertEqual(read_u32(0x020BB77C), 0x03A03000)
        self.assertEqual(read_u32(0x020BB784), 0xE12FFF1E)

        # SetMode spans 0x020bb788..0x020bb85b.  Modes 0/1/2 select target
        # X 137/0/137, drive both marker visibilities, and either update the
        # window target immediately or play transition feedback 3.
        self.assertEqual(read_u32(0x020BB788), 0xE92D4038)
        self.assertEqual(read_u32(0x020BB790), 0xE1C419BC)
        self.assertEqual(read_u32(0x020BB7B0), 0xE3A01089)
        self.assertEqual(read_u32(0x020BB7C4), 0xE5922010)
        self.assertEqual(read_u32(0x020BB7D8), 0xE3A01089)
        self.assertEqual(read_u32(0x020BB82C), 0xE1D419FA)
        self.assertEqual(read_u32(0x020BB840), 0xE3A01003)
        self.assertEqual(read_u32(0x020BB844), 0xE3A02080)
        self.assertEqual(read_u32(0x020BB854), 0xEB000000)
        self.assertEqual(read_u32(0x020BB858), 0xE8BD8038)

        # Mode helper stores -96/0/-48 at +0x9e for modes 0/1/2.
        self.assertEqual(read_u32(0x020BB85C), 0xE1D019BC)
        self.assertEqual(read_u32(0x020BB878), 0x01C019BE)
        self.assertEqual(read_u32(0x020BB884), 0xE1C019BE)
        self.assertEqual(read_u32(0x020BB890), 0xE1C019BE)
        self.assertEqual(read_u32(0x020BB894), 0xE12FFF1E)

        # UpdateSecondaryMode is exactly 0x020bb898..0x020bb97f.  It bounds
        # the selected primary row, requires children, positions the marker
        # in 23-pixel rows, sets animation frame zero, and chooses mode.
        self.assertEqual(read_u32(0x020BB898), 0xE92D40F8)
        self.assertEqual(read_u32(0x020BB8A0), 0xE597C05C)
        self.assertEqual(read_u32(0x020BB8B4), 0xE5C750AE)
        self.assertEqual(read_u32(0x020BB8E8), 0xE1D001B4)
        self.assertEqual(read_u32(0x020BB8F4), 0xE2822001)
        self.assertEqual(read_u32(0x020BB8FC), 0xE0030192)
        self.assertEqual(read_u32(0x020BB91C), 0xE12FFF32)
        self.assertEqual(read_u32(0x020BB92C), 0xEBFFF1AE)
        self.assertEqual(read_u32(0x020BB93C), 0xE5C700AE)
        self.assertEqual(read_u32(0x020BB978), 0xEBFFFF82)
        self.assertEqual(read_u32(0x020BB97C), 0xE8BD80F8)

        # SetInteractionMode is the independent 0x020bb980..0x020bb9ff unit.
        # It stores state/control selection and updates both ItemsPanels in
        # inactive and enabled call orders before the next constructor.
        self.assertEqual(read_u32(0x020BB980), 0xE92D4038)
        self.assertEqual(read_u32(0x020BB98C), 0xE5C410AC)
        self.assertEqual(read_u32(0x020BB990), 0xE5C01029)
        self.assertEqual(read_u32(0x020BB9AC), 0xEBFFF770)
        self.assertEqual(read_u32(0x020BB9B8), 0xE5C0503C)
        self.assertEqual(read_u32(0x020BB9CC), 0xE8BD8038)
        self.assertEqual(read_u32(0x020BB9D8), 0xE5C0503C)
        self.assertEqual(read_u32(0x020BB9E4), 0xEBFFF762)
        self.assertEqual(read_u32(0x020BB9F8), 0xEBFFF75D)
        self.assertEqual(read_u32(0x020BB9FC), 0xE8BD8038)
        self.assertEqual(read_u32(0x020BBA00), 0xE92D4010)

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")

        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "manual_toc_panel_test"
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
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/ui_element.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/ui_panel.c"),
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/manual_toc_panel.c"),
                    str(Path(__file__).with_name("manual_toc_panel_harness.c")),
                    "-o",
                    str(executable),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )
            subprocess.run([str(executable)], check=True, cwd=PROJECT_ROOT)


if __name__ == "__main__":
    unittest.main()
