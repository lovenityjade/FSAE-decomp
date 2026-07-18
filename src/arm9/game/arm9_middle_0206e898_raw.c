#include "game/arm9_middle_0206e898_raw.h"

/*
 * Raw matching-first recovery for the 32 catalog entries beginning at
 * 0x0206e898. These bodies remain address-named promotion candidates and are
 * not linked byte-matching evidence.
 *
 * Calls whose implicit r0 argument was omitted by Ghidra are written with the
 * owning param_1 so this mechanically faithful draft remains valid C.
 */

extern uint DAT_0206e954;
extern uint DAT_0206e9a8;
extern undefined4 *DAT_0206ec5c;
extern undefined4 *DAT_0206ec60;
extern uint DAT_0206ec64;
extern uint DAT_0206ecf4;
extern undefined4 *DAT_0206edb0;
extern undefined4 *DAT_0206edb4;
extern uint DAT_0206edb8;
extern uint DAT_0206ee58;
extern undefined4 *DAT_0206eefc;
extern undefined4 *DAT_0206ef00;
extern uint DAT_0206ef04;
extern uint DAT_0206ef84;
extern undefined4 *DAT_0206f028;
extern undefined4 *DAT_0206f02c;
extern uint DAT_0206f030;
extern uint DAT_0206f0c0;
extern uint DAT_0206f18c;
extern undefined2 *DAT_0206f348;
extern undefined2 *DAT_0206f34c;
extern byte *DAT_0206f350;
extern uint DAT_0206f3fc;
extern uint DAT_0206f52c;
extern uint DAT_0206f530;
extern uint DAT_0206f534;
extern uint DAT_0206f538;
extern uint DAT_0206f53c;
extern uint DAT_0206f540;
extern uint DAT_0206f5a0;
extern uint DAT_0206f5a4;
extern uint DAT_0206f678;
extern uint DAT_0206f67c;
extern uint DAT_02070090;
extern uint DAT_02070094;
extern uint DAT_02070190;
extern uint DAT_020701d0;
extern uint DAT_020702c4;
extern uint DAT_02070578;
extern uint DAT_0207057c;
extern uint DAT_02070580;
extern uint DAT_02070584;
extern uint DAT_02070588;
extern uint DAT_0207058c;
extern uint DAT_02070590;
extern uint *DAT_02070594;
extern uint DAT_02070650;
extern uint DAT_02070654;
extern uint DAT_0207070c;
extern uint DAT_02070710;
extern uint *DAT_02070714;
extern uint DAT_02070754;
extern uint DAT_02070848;
extern uint DAT_0207084c;
extern uint DAT_02070da0;
extern uint DAT_02070da4;
extern uint DAT_02070da8;
extern uint DAT_02070dac;
extern uint DAT_02070db0;
extern uint DAT_02070db4;
extern uint DAT_02070db8;

/* ================================================================
 * FUN_0206e898 @ 0206e898
 * ================================================================ */

void FUN_0206e898(int param_1)

{
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x4000c000;
  *(undefined1 *)(param_1 + 0x17) = 0;
  *(undefined2 *)(param_1 + 0x32) = 0x78;
  *(undefined2 *)(param_1 + 0x36) = 0x58;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_0206e8e4 @ 0206e8e4
 * ================================================================ */

void FUN_0206e8e4(int param_1)

{
  *(undefined1 *)(param_1 + 0x1b) = 0xb0;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xff00fffc | 0x660001;
  FUN_02085980(param_1);
  if (*(char *)(param_1 + 0x21) != '`') {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_0206e93c @ 0206e93c
 * ================================================================ */

void FUN_0206e93c(int param_1)

{
  (**(code **)(DAT_0206e954 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206e958 @ 0206e958
 * ================================================================ */

void FUN_0206e958(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  uint uVar1;
  int iVar2;

  iVar2 = *(byte *)(param_1 + 0x14) + 1;
  uVar1 = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x4000c000;
  *(char *)(param_1 + 0x14) = (char)iVar2;
  *(uint *)(param_1 + 0x1c) = uVar1;
  *(undefined1 *)(param_1 + 0x17) = 0;
  *(undefined1 *)(param_1 + 0x18) = 0;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10),uVar1,iVar2,param_4);
  *(undefined4 *)(DAT_0206e9a8 + (uint)*(byte *)(param_1 + 0xb) * 4) = 0;
  return;
}

/* ================================================================
 * FUN_0206ebbc @ 0206ebbc
 * ================================================================ */

void FUN_0206ebbc(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 local_28 [4];
  undefined4 uStack_18;
  undefined4 uStack_14;
  undefined4 uStack_10;

  puVar2 = DAT_0206ec60;
  puVar1 = DAT_0206ec5c;
  uStack_10 = param_4;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x4000c000;
  *(undefined1 *)(param_1 + 0x17) = 0;
  *(undefined1 *)(param_1 + 0x18) = 0;
  local_28[3] = *puVar1;
  uStack_18 = puVar1[1];
  uStack_14 = puVar1[2];
  local_28[0] = *puVar2;
  local_28[1] = puVar2[1];
  local_28[2] = puVar2[2];
  *(short *)(param_1 + 0x32) = (short)local_28[*(byte *)(param_1 + 0xb) + 3];
  *(short *)(param_1 + 0x36) = (short)local_28[*(byte *)(param_1 + 0xb)] + -0xc;
  *(short *)(param_1 + 0x36) = *(short *)(param_1 + 0x36) + 9;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  *(undefined4 *)(DAT_0206ec64 + (uint)*(byte *)(param_1 + 0xb) * 4) = 0;
  return;
}

/* ================================================================
 * FUN_0206ec68 @ 0206ec68
 * ================================================================ */

void FUN_0206ec68(int param_1)

{
  *(short *)(param_1 + 0x36) = *(short *)(param_1 + 0x36) + -8;
  *(undefined1 *)(param_1 + 0x1b) = 0xb0;
  *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xff00fffc | 0x660001;
  FUN_02085980(param_1);
  if (-9 < *(short *)(param_1 + 0x36)) {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_0206ecdc @ 0206ecdc
 * ================================================================ */

void FUN_0206ecdc(int param_1)

{
  (**(code **)(DAT_0206ecf4 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206ecf8 @ 0206ecf8
 * ================================================================ */

void FUN_0206ecf8(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 *puVar1;
  undefined4 local_48 [4];
  undefined4 local_38;
  undefined4 uStack_34;
  undefined4 local_30 [4];
  undefined4 local_20;
  undefined4 uStack_1c;
  undefined4 uStack_18;

  puVar1 = DAT_0206edb0;
  uStack_18 = param_4;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x4000c000;
  local_30[0] = *puVar1;
  local_30[1] = puVar1[1];
  local_30[2] = puVar1[2];
  local_30[3] = puVar1[3];
  *(undefined1 *)(param_1 + 0x17) = 0;
  *(undefined1 *)(param_1 + 0x18) = 0;
  local_20 = puVar1[4];
  uStack_1c = puVar1[5];
  local_48[0] = *DAT_0206edb4;
  local_48[1] = DAT_0206edb4[1];
  local_48[2] = DAT_0206edb4[2];
  local_48[3] = DAT_0206edb4[3];
  local_38 = DAT_0206edb4[4];
  uStack_34 = DAT_0206edb4[5];
  *(short *)(param_1 + 0x32) = (short)local_30[*(byte *)(param_1 + 0xb)];
  *(short *)(param_1 + 0x36) = (short)local_48[*(byte *)(param_1 + 0xb)] + -8;
  *(short *)(param_1 + 0x36) = *(short *)(param_1 + 0x36) + 9;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  *(undefined4 *)(DAT_0206edb8 + (uint)*(byte *)(param_1 + 0xb) * 4) = 0;
  return;
}

/* ================================================================
 * FUN_0206ee40 @ 0206ee40
 * ================================================================ */

void FUN_0206ee40(int param_1)

{
  (**(code **)(DAT_0206ee58 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206ee5c @ 0206ee5c
 * ================================================================ */

void FUN_0206ee5c(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 local_28 [4];
  undefined4 uStack_18;
  undefined4 uStack_14;
  undefined4 uStack_10;

  puVar2 = DAT_0206ef00;
  puVar1 = DAT_0206eefc;
  uStack_10 = param_4;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x4000c000;
  *(undefined1 *)(param_1 + 0x17) = 0;
  *(undefined1 *)(param_1 + 0x18) = 0;
  local_28[3] = *puVar1;
  uStack_18 = puVar1[1];
  uStack_14 = puVar1[2];
  local_28[0] = *puVar2;
  local_28[1] = puVar2[1];
  local_28[2] = puVar2[2];
  *(short *)(param_1 + 0x32) = (short)local_28[*(byte *)(param_1 + 0xb) + 3];
  *(short *)(param_1 + 0x36) = (short)local_28[*(byte *)(param_1 + 0xb)] + -4;
  *(short *)(param_1 + 0x36) = *(short *)(param_1 + 0x36) + 9;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  *(undefined4 *)(DAT_0206ef04 + (uint)*(byte *)(param_1 + 0xb) * 4) = 0;
  return;
}

/* ================================================================
 * FUN_0206ef08 @ 0206ef08
 * ================================================================ */

void FUN_0206ef08(int param_1)

{
  *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + '\x01';
  *(undefined1 *)(param_1 + 0x1b) = 0xb0;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xff00fffc | 0x660001;
  FUN_02085980(param_1);
  if (*(byte *)(param_1 + 0x17) < 0x259) {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_0206ef6c @ 0206ef6c
 * ================================================================ */

void FUN_0206ef6c(int param_1)

{
  (**(code **)(DAT_0206ef84 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206ef88 @ 0206ef88
 * ================================================================ */

void FUN_0206ef88(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 local_28 [4];
  undefined4 uStack_18;
  undefined4 uStack_14;
  undefined4 uStack_10;

  puVar2 = DAT_0206f02c;
  puVar1 = DAT_0206f028;
  uStack_10 = param_4;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x4000c000;
  *(undefined1 *)(param_1 + 0x17) = 0;
  *(undefined1 *)(param_1 + 0x18) = 0;
  local_28[3] = *puVar1;
  uStack_18 = puVar1[1];
  uStack_14 = puVar1[2];
  local_28[0] = *puVar2;
  local_28[1] = puVar2[1];
  local_28[2] = puVar2[2];
  *(short *)(param_1 + 0x32) = (short)local_28[*(byte *)(param_1 + 0xb) + 3];
  *(short *)(param_1 + 0x36) = (short)local_28[*(byte *)(param_1 + 0xb)] + -0xc;
  *(short *)(param_1 + 0x36) = *(short *)(param_1 + 0x36) + 9;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  *(undefined4 *)(DAT_0206f030 + (uint)*(byte *)(param_1 + 0xb) * 4) = 0;
  return;
}

/* ================================================================
 * FUN_0206f0a8 @ 0206f0a8
 * ================================================================ */

void FUN_0206f0a8(int param_1)

{
  (**(code **)(DAT_0206f0c0 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206f0c4 @ 0206f0c4
 * ================================================================ */

void FUN_0206f0c4(int param_1)

{
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x4000c000;
  *(undefined1 *)(param_1 + 0x17) = 0;
  *(undefined2 *)(param_1 + 0x32) = 0x78;
  *(undefined2 *)(param_1 + 0x36) = 0x61;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_0206f114 @ 0206f114
 * ================================================================ */

void FUN_0206f114(int param_1)

{
  char cVar1;

  *(undefined1 *)(param_1 + 0x1b) = 0xb0;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xff00fffc | 0x660001;
  FUN_02085980(param_1);
  cVar1 = *(char *)(param_1 + 0x17);
  *(char *)(param_1 + 0x17) = cVar1 + '\x01';
  if (cVar1 != -0x10) {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_0206f174 @ 0206f174
 * ================================================================ */

void FUN_0206f174(int param_1)

{
  (**(code **)(DAT_0206f18c + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206f190 @ 0206f190
 * ================================================================ */

void FUN_0206f190(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  undefined2 uVar2;
  undefined2 uVar3;
  byte *pbVar4;
  char cVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  char cVar9;
  undefined2 local_30 [4];
  undefined2 local_28;
  undefined2 local_26;
  undefined4 uStack_24;

  uStack_24 = param_4;
  uVar6 = (uint)*(byte *)(param_1 + 0xb);
  local_28 = DAT_0206f348[1];
  uVar8 = 0;
  local_30[2] = DAT_0206f34c[2];
  local_30[3] = *DAT_0206f348;
  local_26 = DAT_0206f348[2];
  uVar2 = local_30[uVar6 + 3];
  local_30[0] = *DAT_0206f34c;
  local_30[1] = DAT_0206f34c[1];
  uVar3 = local_30[uVar6];
  cVar9 = '\0';
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined2 *)(param_1 + 0x36) = uVar3;
  *(undefined1 *)(param_1 + 0x17) = 0;
  *(undefined2 *)(param_1 + 0x32) = uVar2;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3ff03ffc | 0x4000c000;
  pbVar4 = DAT_0206f350;
  if (uVar6 == 0) {
    uVar6 = *(uint *)(DAT_0206f350 + 0x28) & 0x40000;
    uVar8 = *(uint *)(DAT_0206f350 + 0x28) & 0x80000;
  }
  else {
    if (uVar6 != 1) {
      if (uVar6 == 2) {
        cVar5 = FUN_02048ee4();
        uVar6 = (uint)*pbVar4;
        iVar7 = 0;
        cVar9 = cVar5 + -1;
        if (uVar6 != 0) {
          do {
            iVar1 = iVar7 * 8;
            iVar7 = iVar7 + 1;
            if ((*(uint *)(pbVar4 + iVar1 + 0x28) & 0x1000000) != 0) {
              uVar8 = uVar8 + 1;
            }
          } while (iVar7 < (int)uVar6);
        }
        if (uVar8 == uVar6) {
          cVar9 = '\b';
        }
        else {
          uVar8 = 0;
          iVar7 = 0;
          if (uVar6 != 0) {
            do {
              iVar1 = iVar7 * 8;
              iVar7 = iVar7 + 1;
              if ((*(uint *)(DAT_0206f350 + iVar1 + 0x28) & 0x800000) != 0) {
                uVar8 = uVar8 + 1;
              }
            } while (iVar7 < (int)uVar6);
          }
          if (uVar8 == uVar6) {
            cVar9 = cVar5 + '\x04';
          }
          else {
            uVar8 = 0;
            iVar7 = 0;
            if (uVar6 != 0) {
              do {
                iVar1 = iVar7 * 8;
                iVar7 = iVar7 + 1;
                if ((*(uint *)(DAT_0206f350 + iVar1 + 0x28) & 8) != 0) {
                  uVar8 = uVar8 + 1;
                }
              } while (iVar7 < (int)uVar6);
            }
            if (uVar8 == uVar6) {
              cVar9 = cVar5 + '\x02';
            }
          }
        }
      }
      goto LAB_0206f32c;
    }
    uVar6 = *(uint *)(DAT_0206f350 + 0x28) & 0x2000;
    uVar8 = *(uint *)(DAT_0206f350 + 0x28) & 0x4000;
  }
  cVar9 = uVar6 != 0;
  if (uVar8 != 0) {
    cVar9 = '\x02';
  }
LAB_0206f32c:
  FUN_020859e0(param_1,*(char *)(param_1 + 10) + cVar9);
  return;
}

/* ================================================================
 * FUN_0206f3e4 @ 0206f3e4
 * ================================================================ */

void FUN_0206f3e4(int param_1)

{
  (**(code **)(DAT_0206f3fc + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206f400 @ 0206f400
 * ================================================================ */

void FUN_0206f400(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  uint unaff_r7;
  bool bVar4;

  uVar3 = *(undefined4 *)(DAT_0206f52c + (uint)*(byte *)(param_1 + 10) * 4);
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(undefined4 *)(param_1 + 0x60) = uVar3;
  if (*(char *)(param_1 + 0xb) != -1) {
    uVar3 = FUN_020460e8();
    *(undefined4 *)(param_1 + 0x7c) = uVar3;
    *(undefined1 *)(param_1 + 0x14) = 2;
  }
  uVar1 = DAT_0206f53c;
  uVar3 = DAT_0206f534;
  bVar4 = (*(uint *)(param_1 + 0x60) & *(uint *)(DAT_0206f530 + (uint)*(byte *)(param_1 + 0x52) * 4)
          ) != 0;
  if (bVar4) {
    unaff_r7 = DAT_0206f538;
  }
  if (!bVar4) {
    unaff_r7 = 0;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfff0ffff | (unaff_r7 & 0xf) << 0x10;
  *(undefined4 *)(param_1 + 0x48) = uVar3;
  *(undefined1 *)(param_1 + 0x3f) = 0x39;
  *(short *)(param_1 + 0x5c) = (short)uVar1;
  *(undefined1 *)(param_1 + 0x3c) = 0x99;
  *(undefined1 *)(param_1 + 0x40) = 0x3f;
  FUN_020859e0(param_1,0,0x3f,0x99,param_4);
  iVar2 = DAT_0206f540;
  FUN_0208366c(0,(((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_0206f540 + 0x1a))
                  >> 4) << 6 |
                 (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_0206f540 + 0x18)) >>
                 4) & 0xffff,*(undefined2 *)(param_1 + 0x50));
  FUN_0208365c(0xf,(((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(iVar2 + 0x1a)) >> 4)
                    << 6 | (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(iVar2 + 0x18))
                           >> 4) & 0xffff,*(undefined2 *)(param_1 + 0x50));
  return;
}

/* ================================================================
 * FUN_0206f544 @ 0206f544
 * ================================================================ */

void FUN_0206f544(int param_1)

{
  uint uVar1;
  uint uVar2;
  bool bVar3;

  uVar2 = DAT_0206f5a0;
  if ((*(byte *)(param_1 + 0x41) & 0x80) == 0) {
    return;
  }
  uVar1 = *(uint *)(DAT_0206f5a0 + (uint)*(byte *)(param_1 + 0x52) * 4) ^ *(uint *)(param_1 + 0x60);
  bVar3 = (*(uint *)(param_1 + 0x60) & uVar1) != 0;
  *(uint *)(DAT_0206f5a0 + (uint)*(byte *)(param_1 + 0x52) * 4) = uVar1;
  if (bVar3) {
    uVar2 = DAT_0206f5a4;
  }
  if (!bVar3) {
    uVar2 = 0;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfff0ffff | (uVar2 & 0xf) << 0x10;
  *(undefined1 *)(param_1 + 0x3d) = 0xe8;
  FUN_02085f80(0x16);
  return;
}

/* ================================================================
 * FUN_0206f5a8 @ 0206f5a8
 * ================================================================ */

void FUN_0206f5a8(int param_1)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  bool bVar5;

  iVar1 = DAT_0206f678;
  if ((*(byte *)(param_1 + 0x41) & 0x80) != 0) {
    uVar3 = (uint)*(byte *)(param_1 + 0x52);
    uVar4 = *(uint *)(param_1 + 0x60);
    uVar2 = uVar4 ^ *(uint *)(DAT_0206f678 + uVar3 * 4) & ~(*(uint *)(param_1 + 0x7c) ^ uVar4);
    *(uint *)(DAT_0206f678 + uVar3 * 4) = uVar2;
    *(uint *)(param_1 + 0x78) = uVar2;
    bVar5 = (uVar4 & *(uint *)(iVar1 + uVar3 * 4)) != 0;
    if (bVar5) {
      uVar3 = DAT_0206f67c;
    }
    if (!bVar5) {
      uVar3 = 0;
    }
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfff0ffff | (uVar3 & 0xf) << 0x10;
    *(undefined1 *)(param_1 + 0x3d) = 0xe8;
    FUN_02085f80(0x16);
  }
  iVar1 = DAT_0206f678;
  uVar2 = *(uint *)(DAT_0206f678 + (uint)*(byte *)(param_1 + 0x52) * 4);
  uVar3 = *(uint *)(param_1 + 0x78) ^ uVar2;
  if ((*(uint *)(param_1 + 0x60) & uVar3) == 0) {
    return;
  }
  bVar5 = (*(uint *)(param_1 + 0x60) & uVar2) != 0;
  if (bVar5) {
    uVar3 = DAT_0206f67c;
  }
  if (!bVar5) {
    uVar3 = 0;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfff0ffff | (uVar3 & 0xf) << 0x10;
  *(undefined4 *)(param_1 + 0x78) = *(undefined4 *)(iVar1 + (uint)*(byte *)(param_1 + 0x52) * 4);
  FUN_02085f80(0x16);
  return;
}

/* ================================================================
 * FUN_0207005c @ 0207005c
 * ================================================================ */

void FUN_0207005c(int param_1)

{
  if (*(char *)(param_1 + 10) == '\0') {
    (**(code **)(DAT_02070090 + (uint)*(byte *)(param_1 + 0x14) * 4))();
    return;
  }
  (**(code **)(DAT_02070094 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_02070140 @ 02070140
 * ================================================================ */

void FUN_02070140(int param_1)

{
  int iVar1;

  iVar1 = *(int *)(DAT_02070190 + (*(byte *)(param_1 + 10) - 1) * 4);
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 2;
  *(ushort *)(param_1 + 0x32) = (ushort)*(byte *)(iVar1 + (uint)*(byte *)(param_1 + 0xb));
  *(undefined2 *)(param_1 + 0x36) = 0x28;
  *(undefined2 *)(param_1 + 0x3a) = 0xffc0;
  FUN_020859e0(param_1);
  return;
}

/* ================================================================
 * FUN_020701b8 @ 020701b8
 * ================================================================ */

void FUN_020701b8(int param_1)

{
  (**(code **)(DAT_020701d0 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_020701d4 @ 020701d4
 * ================================================================ */

void FUN_020701d4(int param_1)

{
  uint uVar1;
  int iVar2;
  uint uVar3;

  iVar2 = *(int *)(DAT_020702c4 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94);
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 3;
  uVar3 = *(uint *)(param_1 + 0x1c);
  uVar1 = *(uint *)(iVar2 + 0x1c) & 0xc0000000;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = uVar3 & 0x3fffffff | uVar1;
  iVar2 = (int)(uint)*(byte *)(iVar2 + 0x2a) >> 1;
  *(char *)(param_1 + 0x2a) = (char)iVar2;
  *(byte *)(param_1 + 0x2b) = (byte)((uint)((iVar2 + 1) * 0x40000000) >> 0x1b);
  *(undefined1 *)(param_1 + 0x18) = 6;
  *(undefined2 *)(param_1 + 100) = 0;
  *(undefined2 *)(param_1 + 0x6a) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0x140;
  *(undefined1 *)(param_1 + 0x20) = 0x3c;
  *(uint *)(param_1 + 0x1c) = uVar3 & 0x3ff0ff3f | uVar1 | 0x600c0;
  *(undefined1 *)(param_1 + 0x1b) = 0x2a;
  FUN_020859e0(param_1,7);
  if (*(char *)(param_1 + 0xb) == '\0') {
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
    *(undefined1 *)(param_1 + 0x17) = 0x28;
    FUN_020702c8(param_1);
    return;
  }
  *(undefined1 *)(param_1 + 0x17) = 0x46;
  return;
}

/* ================================================================
 * FUN_020702c8 @ 020702c8
 * ================================================================ */

void FUN_020702c8(int param_1)

{
  char cVar1;
  int iVar2;
  short sVar3;
  byte bVar4;
  int iVar5;
  int iVar6;
  int iVar7;

  iVar5 = (short)(ushort)*(byte *)(param_1 + 10) * 0x94;
  iVar6 = *(int *)(DAT_0207057c + iVar5);
  if (*(char *)(DAT_02070578 + iVar5) == '\0') {
    FUN_0204be88(param_1);
    return;
  }
  *(undefined2 *)(param_1 + 0x3a) = *(undefined2 *)(iVar6 + 0x3a);
  iVar7 = DAT_02070584;
  iVar2 = DAT_02070580;
  if (*(char *)(param_1 + 0xb) == '\0') {
    bVar4 = *(byte *)(DAT_02070588 + iVar5);
    iVar7 = DAT_0207058c;
    if ((bVar4 & 1) == 0) {
      FUN_0204be88(param_1);
      return;
    }
  }
  else {
    if (*(byte *)(DAT_02070580 + iVar5) < 0x14) {
      if (*(byte *)(DAT_02070580 + iVar5) != 0) {
        *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
        return;
      }
      FUN_0204be88(param_1);
      return;
    }
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
    bVar4 = *(byte *)(iVar2 + iVar5);
  }
  if (bVar4 == 0) {
    FUN_0204be88(param_1);
    return;
  }
  FUN_02085980(param_1);
  if (*(char *)(param_1 + 0x17) == '\0') {
    if (*(char *)(param_1 + 0xb) != '\0') {
      *(undefined2 *)(param_1 + 100) = 10;
    }
    if (*(short *)(param_1 + 0x6a) == 0) {
      FUN_02085f80(0x1b);
      iVar5 = FUN_02076a20(param_1,0x12,0);
      if (iVar5 != 0) {
        *(undefined2 *)(iVar5 + 0x50) = *(undefined2 *)(param_1 + 0x50);
      }
      *(short *)(param_1 + 0x6a) = *(short *)(param_1 + 0x6a) + 1;
    }
    else if (((*DAT_02070594 & 0x1f) == 0) && (iVar5 = FUN_02076a20(param_1,7,0), iVar5 != 0)) {
      *(undefined2 *)(iVar5 + 0x50) = *(undefined2 *)(param_1 + 0x50);
    }
  }
  else {
    bVar4 = *(char *)(param_1 + 0x17) - 1;
    *(byte *)(param_1 + 0x17) = bVar4;
    if (*(char *)(param_1 + 0xb) == '\0') {
      if ((bVar4 & 3) == 0) {
        *(short *)(param_1 + 100) = *(short *)(param_1 + 100) + 1;
      }
    }
    else {
      iVar5 = (0x46 - (uint)bVar4) * 10;
      *(short *)(param_1 + 100) =
           (short)(iVar5 + (int)((ulonglong)((longlong)DAT_02070590 * (longlong)iVar5) >> 0x20) >> 6
                  ) - (short)(iVar5 >> 0x1f);
    }
  }
  *(uint *)(param_1 + 0x1c) =
       *(uint *)(param_1 + 0x1c) & 0xffff3fff | *(uint *)(iVar6 + 0x1c) & 0xc000;
  iVar5 = (uint)*(byte *)(param_1 + 0x2a) * 2;
  *(short *)(param_1 + 0x32) = *(short *)(iVar6 + 0x32) + (short)*(char *)(iVar7 + iVar5);
  *(short *)(param_1 + 0x36) = *(short *)(iVar6 + 0x36) + (short)*(char *)(iVar7 + iVar5 + 1);
  *(undefined2 *)(param_1 + 0x50) = *(undefined2 *)(iVar6 + 0x50);
  switch((uint)*(byte *)(param_1 + 0x2a)) {
  case 0:
    sVar3 = *(short *)(param_1 + 0x36) - *(short *)(param_1 + 100);
    break;
  case 1:
    sVar3 = *(short *)(param_1 + 0x32) + *(short *)(param_1 + 100);
    goto LAB_0207050c;
  case 2:
    sVar3 = *(short *)(param_1 + 0x36) + *(short *)(param_1 + 100);
    break;
  case 3:
    sVar3 = *(short *)(param_1 + 0x32) - *(short *)(param_1 + 100);
LAB_0207050c:
    *(short *)(param_1 + 0x32) = sVar3;
  default:
    goto switchD_020704b4_default;
  }
  *(short *)(param_1 + 0x36) = sVar3;
switchD_020704b4_default:
  cVar1 = *(char *)(param_1 + 0x18);
  *(char *)(param_1 + 0x18) = cVar1 + -1;
  if (cVar1 == '\0') {
    *(undefined1 *)(param_1 + 0x18) = 6;
    *(byte *)(param_1 + 0x2b) = *(byte *)(param_1 + 0x2b) ^ 0x10;
  }
  FUN_0204fd40(param_1);
  if (*(char *)(param_1 + 0xb) != '\0') {
    return;
  }
  if ((*(byte *)(DAT_02070588 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) & 0x40) != 0) {
    *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  }
  return;
}

/* ================================================================
 * FUN_02070598 @ 02070598
 * ================================================================ */

void FUN_02070598(int param_1)

{
  byte bVar1;
  int iVar2;
  int iVar3;

  iVar3 = *(int *)(DAT_02070650 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94);
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  FUN_0208ab14(param_1,iVar3);
  bVar1 = *(byte *)(iVar3 + 0x2a);
  *(byte *)(param_1 + 0x2a) = bVar1;
  iVar2 = DAT_02070654;
  *(undefined1 *)(param_1 + 0x1b) = 0x7f;
  *(undefined1 *)(param_1 + 0x20) = 0;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfff0ff3f | 0x100c0;
  iVar3 = (int)(uint)bVar1 >> 1;
  if ((*(byte *)(iVar2 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) & 0x10) != 0) {
    FUN_020859e0(param_1,iVar3 + 4);
    return;
  }
  FUN_020859e0(param_1,iVar3);
  return;
}

/* ================================================================
 * FUN_02070658 @ 02070658
 * ================================================================ */

void FUN_02070658(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;

  iVar1 = (short)(ushort)*(byte *)(param_1 + 10) * 0x94;
  FUN_0208ab14(param_1,*(undefined4 *)(DAT_0207070c + iVar1),iVar1,param_4,param_4);
  FUN_02085980(param_1);
  if (*(char *)(DAT_02070710 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) == '\0') {
    if ((*(byte *)(param_1 + 0x29) & 0x80) != 0) {
      *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
      FUN_020859e0(param_1,((int)(uint)*(byte *)(param_1 + 0x2a) >> 1) + 8);
      return;
    }
    FUN_0204be88(param_1);
    return;
  }
  if ((*DAT_02070714 & 3) != 0) {
    return;
  }
  *(uint *)(param_1 + 0x1c) =
       *(uint *)(param_1 + 0x1c) & 0xfff0ffff |
       ((*(uint *)(param_1 + 0x1c) & 0xfffff) >> 0x10 ^ 7) << 0x10;
  return;
}

/* ================================================================
 * FUN_0207073c @ 0207073c
 * ================================================================ */

void FUN_0207073c(int param_1)

{
  (**(code **)(DAT_02070754 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_02070758 @ 02070758
 * ================================================================ */

void FUN_02070758(int param_1)

{
  char cVar1;
  undefined2 uVar2;
  undefined4 uVar3;
  int iVar4;
  undefined1 extraout_r1;
  undefined1 uVar5;
  bool bVar6;

  FUN_0208ae78();
  cVar1 = *(char *)(param_1 + 0x14);
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(char *)(param_1 + 0x14) = cVar1 + '\x01';
  *(undefined4 *)(param_1 + 0x7c) = 0;
  if (*(char *)(param_1 + 0xb) != '\0') {
    *(char *)(param_1 + 0x14) = cVar1 + '\x02';
    FUN_020709fc(param_1);
    return;
  }
  *(undefined1 *)(param_1 + 0x18) = 0xff;
  *(undefined1 *)(param_1 + 0x17) = 0xff;
  bVar6 = (*(byte *)(param_1 + 10) & 0xfb) != 0;
  uVar3 = 0xff;
  if (bVar6) {
    uVar3 = DAT_02070848;
  }
  *(byte *)(param_1 + 0x2a) = *(byte *)(param_1 + 10);
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 1;
  if (!bVar6) {
    uVar3 = DAT_0207084c;
  }
  *(undefined4 *)(param_1 + 0x48) = uVar3;
  *(undefined2 *)(param_1 + 0x70) = *(undefined2 *)(param_1 + 0x32);
  *(undefined2 *)(param_1 + 0x72) = *(undefined2 *)(param_1 + 0x36);
  *(undefined1 *)(param_1 + 0x78) = *(undefined1 *)(param_1 + 0x2a);
  uVar2 = FUN_02083c84(param_1);
  *(undefined2 *)(param_1 + 0x74) = uVar2;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 0x2a));
  iVar4 = FUN_02076858(0x53,0,1);
  uVar5 = extraout_r1;
  if (iVar4 != 0) {
    uVar5 = *(undefined1 *)(param_1 + 0x2a);
  }
  *(int *)(param_1 + 0x58) = iVar4;
  if (iVar4 == 0) {
    FUN_0204be88(param_1);
    return;
  }
  *(undefined1 *)(iVar4 + 0x2a) = uVar5;
  *(int *)(*(int *)(param_1 + 0x58) + 0x54) = param_1;
  return;
}

/* ================================================================
 * FUN_020709fc @ 020709fc
 * ================================================================ */

/* WARNING: Restarted to delay deadcode elimination for space: stack */

void FUN_020709fc(int param_1)

{
  char cVar1;
  short sVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  uint extraout_r1;
  bool bVar7;
  uint uVar8;
  int unaff_r5;
  int iVar9;
  uint uVar10;
  undefined1 *puVar11;
  bool bVar12;
  undefined8 uVar13;
  undefined4 *local_18b0;
  byte abStack_18ac [52];
  undefined1 auStack_1878 [3088];
  undefined1 auStack_c68 [52];
  undefined1 auStack_c34 [3088];

  iVar4 = *(int *)(param_1 + 0x54);
  local_18b0 = (undefined4 *)(param_1 + 100);
  puVar11 = (undefined1 *)(iVar4 + 0x17);
  iVar9 = *(int *)(param_1 + 100);
  if (*(char *)(iVar4 + 0x14) != '\x01') {
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
    return;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  if (iVar9 != 0) {
    unaff_r5 = *(int *)(param_1 + 0x68);
  }
  *(undefined4 *)(param_1 + 0x30) = *(undefined4 *)(iVar4 + 0x30);
  *(undefined4 *)(param_1 + 0x34) = *(undefined4 *)(iVar4 + 0x34);
  *(undefined4 *)(param_1 + 0x38) = *(undefined4 *)(iVar4 + 0x38);
  *(undefined1 *)(param_1 + 0x2a) = *(undefined1 *)(iVar4 + 0x2a);
  iVar3 = DAT_02070da8;
  if (iVar9 != 0 && unaff_r5 != 0) {
    cVar1 = *(char *)(DAT_02070da0 + (uint)*(byte *)(iVar9 + 10) * 0x94);
    bVar12 = cVar1 == '\0';
    if (bVar12) {
      cVar1 = *(char *)(DAT_02070da0 + (uint)*(byte *)(unaff_r5 + 10) * 0x94);
    }
    if (bVar12 && cVar1 == '\0') {
      *(undefined1 *)(iVar4 + 0x14) = 4;
      iVar9 = *(int *)(param_1 + 0x68);
      *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
      *(undefined1 *)(param_1 + 0x18) = 1;
      *(undefined1 *)(param_1 + 0x17) = 1;
      *(undefined2 *)(param_1 + 0x6e) = 0;
      *(undefined1 *)(param_1 + 0x6c) = 0;
      iVar4 = DAT_02070da4;
      *(undefined1 *)(DAT_02070da4 + (uint)*(byte *)(iVar9 + 10) * 0x94) = 0x13;
      *(undefined1 *)(iVar4 + (uint)*(byte *)(*(int *)(param_1 + 100) + 10) * 0x94) =
           *(undefined1 *)(iVar4 + (uint)*(byte *)(iVar9 + 10) * 0x94);
      return;
    }
  }
  uVar10 = 0;
  do {
    if (iVar9 != 0) {
      iVar4 = (uint)*(byte *)(iVar9 + 10) * 0x94 + iVar3;
      if (*(byte *)(iVar4 + 2) == 0) {
        uVar8 = (uint)*(byte *)(param_1 + 0x2a);
        uVar5 = uVar8;
        if (uVar10 == 0) {
          uVar5 = uVar8 ^ 4;
        }
        if (((*(byte *)(iVar4 + 0xd) & 0x80) == 0) && ((uint)*(byte *)(iVar4 + 0xd) != uVar5 * 4)) {
          *(undefined1 *)(iVar4 + 2) = 0x81;
          *(undefined4 *)(iVar9 + 0x10) = 0x18000;
          *(undefined4 *)(iVar9 + 0xc) = 0x80;
          *(undefined1 *)(iVar9 + 0x2b) =
               *(undefined1 *)((uint)*(byte *)(iVar9 + 10) * 0x94 + iVar3 + 0xd);
          iVar4 = (uint)*(byte *)(iVar9 + 10) * 0x94 + iVar3;
          *(ushort *)(iVar4 + 0x36) = *(ushort *)(iVar4 + 0x36) & (ushort)DAT_02070db4;
          FUN_0208da4c();
          uVar13 = FUN_0208de54();
          if ((int)uVar13 == 0) {
            FUN_0209a02c(auStack_c34);
            FUN_02093bbc(auStack_c68);
            iVar4 = FUN_02093b5c();
            FUN_02093bbc(auStack_c68);
            bVar7 = false;
            bVar12 = false;
            uVar5 = 0;
            uVar8 = extraout_r1;
            if (*(char *)(iVar4 + 1) != '\0') {
              FUN_0209a02c(auStack_1878);
              FUN_02093bbc(abStack_18ac);
              uVar8 = (uint)*(byte *)(iVar9 + 10);
              uVar5 = (uint)abStack_18ac[0];
              bVar7 = true;
              if (uVar8 != uVar5) {
                bVar12 = true;
              }
            }
            uVar13 = CONCAT44(uVar8,uVar5);
            if (bVar7) {
              uVar13 = FUN_02093bbc(abStack_18ac);
            }
            if (bVar12) {
              uVar13 = FUN_0209a060(iVar4 + 0x34);
            }
          }
          cVar1 = *(char *)(param_1 + 0xb);
          if (cVar1 == '\0') {
LAB_02070d18:
            if (cVar1 == '\0') {
              iVar4 = (uint)*(byte *)(iVar9 + 10) * 0x94 + iVar3;
              uVar13 = CONCAT44(*(undefined4 *)(iVar4 + 0x90),iVar4);
            }
            iVar4 = (int)uVar13;
            if (cVar1 == '\0' && (int)((ulonglong)uVar13 >> 0x20) == param_1) goto LAB_02070d30;
          }
          else {
            iVar4 = (uint)*(byte *)(iVar9 + 10) * 0x94 + iVar3;
            uVar13 = CONCAT44(*(int *)(param_1 + 0x54),iVar4);
            if (*(int *)(iVar4 + 0x90) != *(int *)(param_1 + 0x54)) goto LAB_02070d18;
LAB_02070d30:
            *(undefined4 *)(iVar4 + 0x90) = 0;
          }
          *(undefined1 *)((uint)*(byte *)(iVar9 + 10) * 0x94 + iVar3 + 0xc) = 1;
          *local_18b0 = 0;
          *puVar11 = 0xff;
        }
        else {
          *(short *)(iVar9 + 0x32) =
               *(short *)(param_1 + 0x32) + (short)*(char *)(uVar10 + DAT_02070dac + uVar8 * 2);
          *(short *)(iVar9 + 0x36) =
               *(short *)(param_1 + 0x36) +
               (short)*(char *)(uVar10 + DAT_02070db0 + (uint)*(byte *)(param_1 + 0x2a) * 2);
          *(char *)(iVar9 + 0x2a) = (char)uVar5;
          *(undefined1 *)((uint)*(byte *)(iVar9 + 10) * 0x94 + iVar3 + 0x1a) = 0x80;
        }
      }
      else if ((*(byte *)(iVar4 + 2) & 0x80) != 0) {
        sVar2 = *(short *)(iVar9 + 0x32);
        iVar6 = (int)sVar2;
        iVar4 = (int)*(short *)(param_1 + 0x32) +
                (int)*(char *)(uVar10 + DAT_02070dac + (uint)*(byte *)(param_1 + 0x2a) * 2);
        if (iVar6 != iVar4) {
          if (iVar4 < iVar6) {
            *(short *)(iVar9 + 0x32) = sVar2 + -1;
          }
          if (iVar6 < iVar4) {
            *(short *)(iVar9 + 0x32) = sVar2 + 1;
          }
        }
        sVar2 = *(short *)(iVar9 + 0x36);
        iVar6 = (int)sVar2;
        iVar4 = (int)*(short *)(param_1 + 0x36) +
                (int)*(char *)(uVar10 + DAT_02070db0 + (uint)*(byte *)(param_1 + 0x2a) * 2);
        if (iVar6 != iVar4) {
          if (iVar4 < iVar6) {
            *(short *)(iVar9 + 0x36) = sVar2 + -1;
          }
          if (iVar6 < iVar4) {
            *(short *)(iVar9 + 0x36) = sVar2 + 1;
          }
        }
      }
    }
    local_18b0 = (undefined4 *)(param_1 + 0x68);
    uVar10 = uVar10 + 1 & 0xff;
    iVar9 = *(int *)(param_1 + 0x68);
    puVar11 = (undefined1 *)(*(int *)(param_1 + 0x54) + 0x18);
    if (1 < uVar10) {
      *(undefined1 *)(param_1 + 0x21) =
           *(undefined1 *)(DAT_02070db8 + (uint)*(byte *)(param_1 + 0x2a) * 9);
      return;
    }
  } while( true );
}
