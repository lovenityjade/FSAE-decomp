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


class ItemsPanelRecoveryTests(unittest.TestCase):
    def test_rom_rtti_layout_vtable_destroy_and_helpers(self) -> None:
        arm9 = (PROJECT_ROOT / "build/rom/sections/arm9.bin").read_bytes()

        def read_u32(address: int) -> int:
            return struct.unpack_from("<I", arm9, address - ARM9_BASE)[0]

        def read_i16s(address: int, count: int) -> tuple[int, ...]:
            return struct.unpack_from(
                "<" + "h" * count, arm9, address - ARM9_BASE
            )

        def body(start: int, end: int) -> bytes:
            return arm9[start - ARM9_BASE : end - ARM9_BASE]

        # GNU RTTI and direct UIPanel base.
        self.assertEqual(read_u32(0x02126E74), 0x020DFAE0)
        self.assertEqual(read_u32(0x02126E78), 0x02126ED4)
        self.assertEqual(read_u32(0x02126E7C), 0x02126D5C)
        name_offset = 0x02126ED4 - ARM9_BASE
        expected_name = b"N4ntmv3m2d6detail10ItemsPanelE\0"
        self.assertEqual(
            arm9[name_offset : name_offset + len(expected_name)], expected_name
        )

        # Specialized destructor; render and setters inherited verbatim.
        self.assertEqual(read_u32(0x02126EB8), 0)
        self.assertEqual(read_u32(0x02126EBC), 0x02126E74)
        self.assertEqual(
            tuple(read_u32(0x02126EC0 + index * 4) for index in range(5)),
            (0x020B931C, 0x020B81DC, 0x020B7E48, 0x020B7E5C, 0x020B7E70),
        )

        # The constructor body is exactly 88 bytes.  Its vtable address-point
        # literal at +0x58 and the following Configure entry at +0x5c are
        # deliberately excluded from this recovery boundary.
        constructor = body(0x020B8F0C, 0x020B8F64)
        self.assertEqual(len(constructor), 88)
        self.assertEqual(
            hashlib.sha256(constructor).hexdigest(),
            "01e2516b4a20c7e7895a547391935fee21fffb5d540ec3419a975ed952804151",
        )
        self.assertEqual(read_u32(0x020B8F0C), 0xE92D4010)
        self.assertEqual(read_u32(0x020B8F60), 0xE8BD8010)
        self.assertEqual(read_u32(0x020B8F64), 0x02126EC0)
        self.assertEqual(read_u32(0x020B8F68), 0xE92D4FF0)

        # The separate factory proves the complete ItemsPanel size is 0x40.
        self.assertEqual(read_u32(0x020BA7E8), 0xE3A01040)
        # Constructor initializes +0x20..+0x2c, +0x30, +0x38/+0x3a/+0x3c.
        self.assertEqual(read_u32(0x020B8F34), 0xE1C432B0)
        self.assertEqual(read_u32(0x020B8F48), 0xE1C422BA)
        self.assertEqual(read_u32(0x020B8F4C), 0xE1C422BC)
        self.assertEqual(read_u32(0x020B8F50), 0xE5843030)
        self.assertEqual(read_u32(0x020B8F54), 0xE1C433B8)
        self.assertEqual(read_u32(0x020B8F58), 0xE1C433BA)
        self.assertEqual(read_u32(0x020B8F5C), 0xE5C4103C)

        # Configure is one exact 744-byte body.  The following two literal
        # words and the allocator entry at 0x020b9258 remain outside it.
        configure = body(0x020B8F68, 0x020B9250)
        self.assertEqual(len(configure), 744)
        self.assertEqual(
            hashlib.sha256(configure).hexdigest(),
            "89f29cc00fca999ff6baf650bb0f834ba7cc1a7c5c77b0589d096b635b5526e5",
        )
        self.assertEqual(read_u32(0x020B8F68), 0xE92D4FF0)
        self.assertEqual(read_u32(0x020B924C), 0xE8BD8FF0)
        self.assertEqual(read_u32(0x020B9250), 0x020DEF10)
        self.assertEqual(read_u32(0x020B9254), 0x02126E64)
        self.assertEqual(read_u32(0x020B9258), 0xE92D4008)

        # It commits the fixed 256x192 geometry, text context, visible-row
        # count and total child count before allocating children/+0x30/+0x34.
        self.assertEqual(read_u32(0x020B8F80), 0xE3A04C01)
        self.assertEqual(read_u32(0x020B8F84), 0xE3A000C0)
        self.assertEqual(read_u32(0x020B8F88), 0xE1CA40B8)
        self.assertEqual(read_u32(0x020B8F94), 0xE1CA00BA)
        self.assertEqual(read_u32(0x020B8F98), 0xE58A301C)
        self.assertEqual(read_u32(0x020B8FB0), 0xE1CA32B4)
        self.assertEqual(read_u32(0x020B8FCC), 0xE1CA11B8)
        self.assertEqual(read_u32(0x020B8FE0), 0xE58A0014)
        self.assertEqual(read_u32(0x020B8FFC), 0xE58A0030)
        self.assertEqual(read_u32(0x020B9018), 0xE58A0034)

        # Circular rows occupy children index +4.  Their Y step is 23,
        # height is 20, alternate -1 is recognized, and indices become -1.
        self.assertEqual(read_u32(0x020B9058), 0xE3E04000)
        self.assertEqual(read_u32(0x020B90A8), 0xE3A00017)
        self.assertEqual(read_u32(0x020B90B0), 0xE3A00014)
        self.assertEqual(read_u32(0x020B9114), 0xE5801010)
        self.assertEqual(read_u32(0x020B9120), 0xE18140B0)

        # Two fixed ObjButtons and two hidden separator UIStatics finish the
        # tree.  The literal tables are exactly [(33,-23),(32,115)] and
        # separator Y [-29,121].
        self.assertEqual(read_u32(0x020B9144), 0xE59F4104)
        self.assertEqual(read_u32(0x020B91BC), 0xE3A07000)
        self.assertEqual(read_u32(0x020B91C4), 0xE3A0B020)
        self.assertEqual(read_u32(0x020B91C8), 0xE3E0401A)
        self.assertEqual(read_u32(0x020B91E8), 0xE3A01027)
        self.assertEqual(read_u32(0x020B924C), 0xE8BD8FF0)
        self.assertEqual(read_u32(0x020B9250), 0x020DEF10)
        self.assertEqual(read_u32(0x020B9254), 0x02126E64)
        self.assertEqual(read_i16s(0x020DEF10, 4), (33, -23, 32, 115))
        self.assertEqual(read_i16s(0x02126E64, 2), (-29, 121))

        # The three immediately following support factories prove a 0x2c
        # ManualButton, two-byte item indices and four-byte button pointers.
        self.assertEqual(read_u32(0x020B9260), 0xE3A0102C)
        self.assertEqual(read_u32(0x020B9294), 0xE1A01081)
        self.assertEqual(read_u32(0x020B92E0), 0xE1A01101)
        self.assertEqual(read_u32(0x020B9318), 0xE8BD8010)

        # The destructor's two typed array releases are separate 24-byte
        # functions.  Both skip null, load allocator +0x00 and delegate to
        # NNS_FndFreeToAllocator; only their PC-relative BL words differ.
        free_row_buttons = body(0x020B9350, 0x020B9368)
        free_row_item_indices = body(0x020B9368, 0x020B9380)
        self.assertEqual(len(free_row_buttons), 24)
        self.assertEqual(len(free_row_item_indices), 24)
        self.assertEqual(
            hashlib.sha256(free_row_buttons).hexdigest(),
            "aec0f8fbf9373745d04a7841e893d9dd0657870ed632e4666633b14ce478d9cd",
        )
        self.assertEqual(
            hashlib.sha256(free_row_item_indices).hexdigest(),
            "896ee95d8b12dc80a2b1e9015d1792976ddbf3ec545ee1af3b3c5fdc438313ba",
        )
        self.assertEqual(free_row_buttons[:16], free_row_item_indices[:16])
        self.assertEqual(free_row_buttons[20:], free_row_item_indices[20:])
        self.assertEqual(read_u32(0x020B934C), 0xE8BD8038)
        self.assertEqual(read_u32(0x020B9350), 0xE92D4008)
        self.assertEqual(read_u32(0x020B9354), 0xE3510000)
        self.assertEqual(read_u32(0x020B9358), 0x08BD8008)
        self.assertEqual(read_u32(0x020B935C), 0xE5900000)
        self.assertEqual(read_u32(0x020B9360), 0xEB002638)
        self.assertEqual(read_u32(0x020B9364), 0xE8BD8008)
        self.assertEqual(read_u32(0x020B9368), 0xE92D4008)
        self.assertEqual(read_u32(0x020B936C), 0xE3510000)
        self.assertEqual(read_u32(0x020B9370), 0x08BD8008)
        self.assertEqual(read_u32(0x020B9374), 0xE5900000)
        self.assertEqual(read_u32(0x020B9378), 0xEB002632)
        self.assertEqual(read_u32(0x020B937C), 0xE8BD8008)
        self.assertEqual(read_u32(0x020B9380), 0xE92D4070)

        # SetItemCount 0x020b9380 computes ceil(count / +0x24), delegates
        # page-control visibility and page zero, clears +0x2a/+0x2c, then
        # clears +0x29 and hides all (+0x24 + 1) circular buttons.
        self.assertEqual(read_u32(0x020B9388), 0xE1C612B0)
        self.assertEqual(read_u32(0x020B938C), 0xE1D612B4)
        self.assertEqual(read_u32(0x020B9390), 0xE1D602B0)
        self.assertEqual(read_u32(0x020B939C), 0xEBFD5169)
        self.assertEqual(read_u32(0x020B93A0), 0xE1C602B8)
        self.assertEqual(read_u32(0x020B93A8), 0xEB0000B6)
        self.assertEqual(read_u32(0x020B93B8), 0xEB0000D8)
        self.assertEqual(read_u32(0x020B93C4), 0xE1C612BA)
        self.assertEqual(read_u32(0x020B93C8), 0xE1C612BC)
        self.assertEqual(read_u32(0x020B93F0), 0xE5C04029)
        self.assertEqual(read_u32(0x020B9404), 0xE12FFF32)
        self.assertEqual(read_u32(0x020B9420), 0xE8BD8070)

        # 0x020b9424 returns false only when +0x2a already matches and
        # interaction +0x3c is active.  Otherwise it enables interaction,
        # clears the old row visual, stores the new item, then selects it.
        self.assertEqual(read_u32(0x020B942C), 0xE1D602FA)
        self.assertEqual(read_u32(0x020B9434), 0xE1500005)
        self.assertEqual(read_u32(0x020B943C), 0xE5D6003C)
        self.assertEqual(read_u32(0x020B9444), 0x13A00000)
        self.assertEqual(read_u32(0x020B9448), 0x18BD8070)
        self.assertEqual(read_u32(0x020B9458), 0xE5C6403C)
        self.assertEqual(read_u32(0x020B945C), 0xEB0000C4)
        self.assertEqual(read_u32(0x020B9468), 0xE1C652BA)
        self.assertEqual(read_u32(0x020B946C), 0xEB0000C0)
        self.assertEqual(read_u32(0x020B9470), 0xE1A00004)

        # 0x020b9478 returns false for selected=-1; otherwise its quotient
        # is selected_item / visible_row_count_minus_one (no +1) and is
        # sign-truncated before delegating to 0x020b9740.
        self.assertEqual(read_u32(0x020B9480), 0xE1D402FA)
        self.assertEqual(read_u32(0x020B9484), 0xE3700001)
        self.assertEqual(read_u32(0x020B9488), 0x03A00000)
        self.assertEqual(read_u32(0x020B948C), 0x08BD8010)
        self.assertEqual(read_u32(0x020B9490), 0xE1D412B4)
        self.assertEqual(read_u32(0x020B9494), 0xEBFD512B)
        self.assertEqual(read_u32(0x020B9498), 0xE1A01800)
        self.assertEqual(read_u32(0x020B94A0), 0xE1A01841)
        self.assertEqual(read_u32(0x020B94A4), 0xEB0000A5)

        # Full pointer handler boundary 0x020b94ac..0x020b9687.  It clears
        # exactly five output bytes, gates hidden/moving panels and consumes
        # pointer state bytes active/pressed/released at offsets 4/5/6.
        self.assertEqual(read_u32(0x020B94AC), 0xE92D47F0)
        self.assertEqual(read_u32(0x020B94C0), 0xE5C84000)
        self.assertEqual(read_u32(0x020B94D0), 0xE5C84004)
        self.assertEqual(read_u32(0x020B94D4), 0xE5D91010)
        self.assertEqual(read_u32(0x020B94E4), 0x01D923F8)
        self.assertEqual(read_u32(0x020B94E8), 0x01D913FA)
        self.assertEqual(read_u32(0x020B94FC), 0xE5D51004)
        self.assertEqual(read_u32(0x020B9508), 0xE5D51005)
        self.assertEqual(read_u32(0x020B95AC), 0xE5D50006)

        # Press hit-tests, rejects the -1 sentinel, extracts lock bit 1 and
        # stores active index +0x2c plus output bytes 4/3 only when unlocked.
        self.assertEqual(read_u32(0x020B9528), 0xEB00020A)
        self.assertEqual(read_u32(0x020B952C), 0xE2441001)
        self.assertEqual(read_u32(0x020B9544), 0xE5D11010)
        self.assertEqual(read_u32(0x020B9548), 0xE1A01F01)
        self.assertEqual(read_u32(0x020B9550), 0x01C902BC)
        self.assertEqual(read_u32(0x020B9554), 0x05C84004)
        self.assertEqual(read_u32(0x020B9558), 0x05C84003)
        self.assertEqual(read_u32(0x020B95A4), 0xEBFFFA3A)

        # Release resets +0x2c to -1, requires hover bit 2, clears it, then
        # routes indices 0/1 to page change and indices >3 to circular rows.
        self.assertEqual(read_u32(0x020B95B4), 0x11D952FC)
        self.assertEqual(read_u32(0x020B95CC), 0xE1C912BC)
        self.assertEqual(read_u32(0x020B95DC), 0xE1A00E80)
        self.assertEqual(read_u32(0x020B95F0), 0xE3C00004)
        self.assertEqual(read_u32(0x020B95F4), 0xE5C10010)
        self.assertEqual(read_u32(0x020B9618), 0xEB000048)
        self.assertEqual(read_u32(0x020B961C), 0xE5C80000)

        # Row mapping divides first_visible and the rotated physical slot by
        # row_count (+0x24 + 1), sets output byte 2, selects the signed item,
        # and stores the bool result in byte 1 before the exact boundary.
        self.assertEqual(read_u32(0x020B962C), 0xE1D902B4)
        self.assertEqual(read_u32(0x020B9630), 0xE1D972F2)
        self.assertEqual(read_u32(0x020B9648), 0xE5C84002)
        self.assertEqual(read_u32(0x020B964C), 0xEBFD50BD)
        self.assertEqual(read_u32(0x020B9660), 0xEBFD50B8)
        self.assertEqual(read_u32(0x020B9674), 0xEBFFFF6A)
        self.assertEqual(read_u32(0x020B9678), 0xE5C80001)
        self.assertEqual(read_u32(0x020B9684), 0xE8BD87F0)

        # 0x020b9740 is a bool helper: identical page returns false without
        # touching the target; changed page calls 0x020b9720 then commits
        # target_scroll_y = first_visible_item * 23 at +0x3a.
        self.assertEqual(read_u32(0x020B9748), 0xE1D422F6)
        self.assertEqual(read_u32(0x020B974C), 0xE1520001)
        self.assertEqual(read_u32(0x020B9750), 0x03A00000)
        self.assertEqual(read_u32(0x020B9758), 0xEBFFFFF0)
        self.assertEqual(read_u32(0x020B975C), 0xE1D422F2)
        self.assertEqual(read_u32(0x020B9760), 0xE3A01017)
        self.assertEqual(read_u32(0x020B9764), 0xE3A00001)
        self.assertEqual(read_u32(0x020B9768), 0xE1610182)
        self.assertEqual(read_u32(0x020B976C), 0xE1C413BA)
        self.assertEqual(read_u32(0x020B9770), 0xE8BD8010)

        # Its helper 0x020b9774 requires a selection and active interaction,
        # enforces first_visible <= item < first_visible + +0x24, divides by
        # (+0x24 + 1) for the circular slot, and writes only byte +0x29.
        self.assertEqual(read_u32(0x020B977C), 0xE1D502FA)
        self.assertEqual(read_u32(0x020B9788), 0x15D5103C)
        self.assertEqual(read_u32(0x020B9794), 0xE1D522F2)
        self.assertEqual(read_u32(0x020B9798), 0xE1500002)
        self.assertEqual(read_u32(0x020B97A0), 0xE1D512B4)
        self.assertEqual(read_u32(0x020B97A8), 0xE1500001)
        self.assertEqual(read_u32(0x020B97B4), 0xE2811001)
        self.assertEqual(read_u32(0x020B97C0), 0xEBFD5060)
        self.assertEqual(read_u32(0x020B97CC), 0xE5C04029)
        self.assertEqual(read_u32(0x020B97D0), 0xE8BD8038)

        # 0x020b9688 exposes children 0..3 only when page_count >= 2 and
        # always locks the two separator statics at indices 2/3.
        self.assertEqual(read_u32(0x020B9690), 0xE1D602B8)
        self.assertEqual(read_u32(0x020B969C), 0xE3500002)
        self.assertEqual(read_u32(0x020B96A0), 0x31A05004)
        self.assertEqual(read_u32(0x020B96B8), 0xE12FFF32)
        self.assertEqual(read_u32(0x020B96EC), 0xE12FFF32)
        self.assertEqual(read_u32(0x020B9710), 0xE3810002)
        self.assertEqual(read_u32(0x020B971C), 0xE8BD8070)

        # 0x020b9720 stores page index, computes first_visible as page*+0x24
        # and tail-calls the still bounded-out page-button lock helper.
        self.assertEqual(read_u32(0x020B9720), 0xE1C012B6)
        self.assertEqual(read_u32(0x020B9724), 0xE1D022F6)
        self.assertEqual(read_u32(0x020B9728), 0xE1D012B4)
        self.assertEqual(read_u32(0x020B9730), 0xE0010192)
        self.assertEqual(read_u32(0x020B9734), 0xE1C012B2)
        self.assertEqual(read_u32(0x020B9738), 0xE12FFF1C)
        self.assertEqual(read_u32(0x020B973C), 0x020B97D4)

        # 0x020b97d4..0x020b9843 returns unchanged for page_count <= 1.
        # Otherwise previous is locked for page_index <= 0 and next for
        # page_index + 1 >= page_count; both paths replace only flag bit 1.
        self.assertEqual(read_u32(0x020B97D4), 0xE1D012B8)
        self.assertEqual(read_u32(0x020B97D8), 0xE3510001)
        self.assertEqual(read_u32(0x020B97DC), 0x912FFF1E)
        self.assertEqual(read_u32(0x020B97E0), 0xE5902014)
        self.assertEqual(read_u32(0x020B97E4), 0xE1D012F6)
        self.assertEqual(read_u32(0x020B97F0), 0xE3510000)
        self.assertEqual(read_u32(0x020B97F4), 0xD3A01001)
        self.assertEqual(read_u32(0x020B9800), 0xE3C22002)
        self.assertEqual(read_u32(0x020B9804), 0xE1821F21)
        self.assertEqual(read_u32(0x020B9808), 0xE5C31010)
        self.assertEqual(read_u32(0x020B980C), 0xE1D022F6)
        self.assertEqual(read_u32(0x020B9810), 0xE1D012B8)
        self.assertEqual(read_u32(0x020B9818), 0xE1520001)
        self.assertEqual(read_u32(0x020B981C), 0xA3A02001)
        self.assertEqual(read_u32(0x020B9834), 0xE3C11002)
        self.assertEqual(read_u32(0x020B9838), 0xE1810F20)
        self.assertEqual(read_u32(0x020B983C), 0xE5C20010)
        self.assertEqual(read_u32(0x020B9840), 0xE12FFF1E)
        self.assertEqual(read_u32(0x020B9844), 0xE92D47F0)

        # Destructor frees +0x34, then +0x30, before the UIPanel base.
        self.assertEqual(read_u32(0x020B9328), 0xE5951034)
        self.assertEqual(read_u32(0x020B9334), 0xE5951030)
        self.assertEqual(read_u32(0x020B9348), 0xEBFFFB7A)

        # Auxiliary insertion uses row-count field +0x24 and children +0x14.
        self.assertEqual(read_u32(0x020B9D30), 0xE1D032B4)
        self.assertEqual(read_u32(0x020B9D34), 0xE590C014)
        self.assertEqual(read_u32(0x020B9D50), 0xE78C2100)
        # Hit-test uses the same signed row-count bound and returns -1 on miss.
        self.assertEqual(read_u32(0x020B9D60), 0xE1D702B4)
        self.assertEqual(read_u32(0x020B9DA8), 0xE7900105)
        self.assertEqual(read_u32(0x020B9DE8), 0xE3E00000)

        # 0x020b9844 clamps against +0x24, advances 8-byte row records,
        # delegates to 0x020b98f8, and hides unused row buttons from +0x30.
        self.assertEqual(read_u32(0x020B984C), 0xE1DA02B4)
        self.assertEqual(read_u32(0x020B98B8), 0xE0891185)
        self.assertEqual(read_u32(0x020B98BC), 0xEB00000D)
        self.assertEqual(read_u32(0x020B98C4), 0xE59A0030)

        # 0x020b98f8 chooses a slot modulo (+0x24 + 1), reads the text
        # context at +0x1c, then commits animation/visibility/index arrays.
        self.assertEqual(read_u32(0x020B9904), 0xE1D902B4)
        self.assertEqual(read_u32(0x020B994C), 0xE599101C)
        self.assertEqual(read_u32(0x020B9B14), 0xE5990030)
        self.assertEqual(read_u32(0x020B9B18), 0xE5D81004)
        self.assertEqual(read_u32(0x020B9B20), 0xEB000812)
        self.assertEqual(read_u32(0x020B9B3C), 0xE5991034)
        self.assertEqual(read_u32(0x020B9B44), 0xE18170B0)
        self.assertEqual(read_u32(0x020B9B50), 0x02126EF4)
        self.assertEqual(read_u32(0x020B9B54), 0x0000020A)

        # 0x020b9b58 is the sibling bounded updater: same clamp/modulo and
        # arrays, but a single 0x18x4 canvas starting at tile Y slot*3+0x12.
        self.assertEqual(read_u32(0x020B9B64), 0xE1D902B4)
        self.assertEqual(read_u32(0x020B9BD8), 0xE3A00018)
        self.assertEqual(read_u32(0x020B9BE8), 0xE0870087)
        self.assertEqual(read_u32(0x020B9BEC), 0xE2800012)
        self.assertEqual(read_u32(0x020B9BF8), 0xE599101C)
        self.assertEqual(read_u32(0x020B9C30), 0xE7980186)
        self.assertEqual(read_u32(0x020B9CBC), 0xE0881186)
        self.assertEqual(read_u32(0x020B9CC0), 0xE5D11004)
        self.assertEqual(read_u32(0x020B9CC4), 0xE7900107)
        self.assertEqual(read_u32(0x020B9CE4), 0xE5991034)
        self.assertEqual(read_u32(0x020B9CEC), 0xE181A0B0)
        self.assertEqual(read_u32(0x020B9D2C), 0x0000020A)

        # 0x020b9df0 is the ItemsPanel scroll animator: it compares +0x38
        # against +0x3a, steps by 12, divides by the 23-pixel row height,
        # rotates +0x30 slots, updates +0x29 selection and checks +0x34.
        self.assertEqual(read_u32(0x020B9DFC), 0xE1DA33FA)
        self.assertEqual(read_u32(0x020B9E00), 0xE1DA03F8)
        self.assertEqual(read_u32(0x020B9E20), 0xE280000C)
        self.assertEqual(read_u32(0x020B9E38), 0xE240000C)
        self.assertEqual(read_u32(0x020B9E4C), 0xE1CA33B8)
        self.assertEqual(read_u32(0x020B9E54), 0xE59F214C)
        self.assertEqual(read_u32(0x020B9E88), 0xE1DA12B0)
        self.assertEqual(read_u32(0x020B9EC0), 0xE59A0030)
        self.assertEqual(read_u32(0x020B9EE0), 0xE3A01017)
        self.assertEqual(read_u32(0x020B9F18), 0xE1DA32FA)
        self.assertEqual(read_u32(0x020B9F28), 0x15DA003C)
        self.assertEqual(read_u32(0x020B9F50), 0xE5C12029)
        self.assertEqual(read_u32(0x020B9F54), 0xE59A1034)
        self.assertEqual(read_u32(0x020B9F70), 0xE12FFF3B)
        self.assertEqual(read_u32(0x020B9F80), 0xEBFFFE5C)
        self.assertEqual(read_u32(0x020B9F9C), 0xE3A00001)
        self.assertEqual(read_u32(0x020B9FA8), 0xB21642C9)

        # The only function before ManualTocPanel begins is the row-provider
        # callback 0x020b9fac, referenced literally by its 0x020baef0 caller.
        self.assertEqual(read_u32(0x020BAEF0), 0x020B9FAC)
        self.assertEqual(read_u32(0x020B9FB0), 0xE5913000)
        self.assertEqual(read_u32(0x020B9FB4), 0xE3A01018)
        self.assertEqual(read_u32(0x020B9FB8), 0xE1033182)
        self.assertEqual(read_u32(0x020B9FBC), 0xE5932004)
        self.assertEqual(read_u32(0x020B9FC0), 0xE1D210B0)
        self.assertEqual(read_u32(0x020B9FD4), 0xE1D311B4)
        self.assertEqual(read_u32(0x020B9FF0), 0xE5802000)
        self.assertEqual(read_u32(0x020B9FF4), 0xE5801004)
        self.assertEqual(read_u32(0x020BA000), 0x02126EFC)
        placeholder_offset = 0x02126EFC - ARM9_BASE
        self.assertEqual(
            arm9[placeholder_offset : placeholder_offset + 12],
            "(---)\0".encode("utf-16le"),
        )

    def test_public_c_recovery(self) -> None:
        compiler = shutil.which("cc")
        if compiler is None:
            self.skipTest("host C compiler is unavailable")

        with tempfile.TemporaryDirectory() as temporary:
            executable = Path(temporary) / "items_panel_test"
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
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/items_panel.c"),
                    str(Path(__file__).with_name("items_panel_harness.c")),
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
            output = Path(temporary) / "items_panel.o"
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
                    str(PROJECT_ROOT / "src/arm9/ntmv/m2d/items_panel.c"),
                    "-o",
                    str(output),
                ],
                check=True,
                cwd=PROJECT_ROOT,
                env={**os.environ, "CCACHE_DISABLE": "1"},
            )


if __name__ == "__main__":
    unittest.main()
