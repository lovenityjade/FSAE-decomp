#include "game/arm9_middle_0206d8f0_raw.h"

/*
 * Raw matching-first recovery for the 32 catalog entries beginning at
 * 0x0206d8f0. These bodies remain address-named promotion candidates and are
 * not linked byte-matching evidence.
 *
 * Calls whose implicit r0 argument was omitted by Ghidra are written with the
 * owning param_1 so this mechanically faithful draft remains valid C.
 */

extern uint DAT_0206d908;
extern uint DAT_0206d9a4;
extern uint DAT_0206db68;
extern uint DAT_0206dc18;
extern uint DAT_0206dd80;
extern uint DAT_0206dde0;
extern uint DAT_0206df10;
extern uint DAT_0206df14;
extern uint *DAT_0206df18;
extern uint DAT_0206dfe0;
extern char *DAT_0206e114;
extern uint DAT_0206e118;
extern byte *DAT_0206e20c;
extern uint DAT_0206e2b4;
extern uint DAT_0206e2b8;
extern uint DAT_0206e2bc;
extern uint DAT_0206e2c0;
extern uint DAT_0206e2c4;
extern uint DAT_0206e3e8;
extern uint DAT_0206e4f0;
extern undefined4 *DAT_0206e590;
extern undefined4 *DAT_0206e594;
extern uint DAT_0206e614;
extern uint DAT_0206e684;
extern uint DAT_0206e86c;
extern uint DAT_0206e870;
extern uint DAT_0206e874;
extern uint DAT_0206e878;

/* ================================================================
 * FUN_0206d8f0 @ 0206d8f0
 * ================================================================ */

void FUN_0206d8f0(int param_1)

{
  (**(code **)(DAT_0206d908 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206d90c @ 0206d90c
 * ================================================================ */

void FUN_0206d90c(int param_1)

{
  int iVar1;

  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x40004001;
  iVar1 = FUN_0208da4c();
  *(undefined1 *)(param_1 + 0x16) = *(undefined1 *)(iVar1 + 0xcb0);
  FUN_0206d5e0(param_1);
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  iVar1 = FUN_0208da4c();
  FUN_0205069c(param_1,DAT_0206d9a4 + (uint)*(byte *)(iVar1 + 0xcb0) * 4,1);
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
  iVar1 = FUN_02076858(0x46,0x14,0);
  *(int *)(param_1 + 0x58) = iVar1;
  *(int *)(iVar1 + 0x54) = param_1;
  return;
}

/* ================================================================
 * FUN_0206da74 @ 0206da74
 * ================================================================ */

void FUN_0206da74(int param_1)

{
  uint uVar1;
  bool bVar2;

  uVar1 = (uint)*(byte *)(param_1 + 10);
  bVar2 = uVar1 == 10;
  if (bVar2) {
    uVar1 = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0xc000;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x4000c000;
  if (bVar2) {
    *(uint *)(param_1 + 0x1c) = uVar1 | 0x80000000;
  }
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined1 *)(param_1 + 0x17) = 0x78;
  *(undefined1 *)(param_1 + 0x18) = 8;
  FUN_0206d5e0(param_1);
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_0206dadc @ 0206dadc
 * ================================================================ */

void FUN_0206dadc(int param_1)

{
  int iVar1;

  if (*(char *)(param_1 + 0x18) != '\0') {
    *(char *)(param_1 + 0x18) = *(char *)(param_1 + 0x18) + -1;
    return;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  FUN_02085980(param_1);
  iVar1 = FUN_0208da4c();
  if (*(char *)(param_1 + 0xb) != *(char *)(iVar1 + 0xcb0)) {
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206db30 @ 0206db30
 * ================================================================ */

void FUN_0206db30(int param_1)

{
  int iVar1;

  (**(code **)(DAT_0206db68 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  iVar1 = FUN_0208da4c();
  if (*(char *)(param_1 + 0xb) == *(char *)(iVar1 + 0xcb0)) {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_0206db6c @ 0206db6c
 * ================================================================ */

void FUN_0206db6c(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  uint uVar1;

  uVar1 = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x40004001;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = uVar1;
  *(undefined1 *)(param_1 + 0x17) = 0xf0;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10),0xf0,uVar1,param_4);
  return;
}

/* ================================================================
 * FUN_0206dbb0 @ 0206dbb0
 * ================================================================ */

void FUN_0206dbb0(int param_1)

{
  char cVar1;

  FUN_0204fd40();
  FUN_02085980(param_1);
  cVar1 = *(char *)(param_1 + 0x17);
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  if (cVar1 == '\0') {
    *(undefined1 *)(param_1 + 0xb) = 0xff;
  }
  return;
}

/* ================================================================
 * FUN_0206dbe0 @ 0206dbe0
 * ================================================================ */

void FUN_0206dbe0(int param_1)

{
  int iVar1;

  (**(code **)(DAT_0206dc18 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  iVar1 = FUN_0208da4c();
  if (*(char *)(param_1 + 0xb) == *(char *)(iVar1 + 0xcb0)) {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_0206dc1c @ 0206dc1c
 * ================================================================ */

void FUN_0206dc1c(int param_1)

{
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x40008000;
  *(undefined1 *)(param_1 + 0x17) = 1;
  *(undefined1 *)(param_1 + 0x18) = 8;
  FUN_0206d5e0(param_1);
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_0206dc6c @ 0206dc6c
 * ================================================================ */

void FUN_0206dc6c(int param_1)

{
  char cVar1;

  if (*(char *)(param_1 + 0x18) != '\0') {
    *(char *)(param_1 + 0x18) = *(char *)(param_1 + 0x18) + -1;
    return;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  cVar1 = *(char *)(param_1 + 0x17);
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  if (cVar1 == '\0') {
    *(undefined1 *)(param_1 + 0x17) = 0x10;
    *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
    FUN_0206dcc8(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206dcc8 @ 0206dcc8
 * ================================================================ */

void FUN_0206dcc8(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  undefined2 *puVar4;

  uVar2 = FUN_0208455c();
  puVar4 = (undefined2 *)(DAT_0206dd80 + (uVar2 & 3) * 8);
  iVar3 = FUN_02076858(0x46,0xb,*(undefined1 *)(param_1 + 0xb),uVar2 << 0x1e,param_4);
  if (iVar3 == 0) {
    return;
  }
  *(undefined2 *)(iVar3 + 0x32) = *puVar4;
  *(undefined2 *)(iVar3 + 0x36) = puVar4[1];
  *(char *)(iVar3 + 0x2b) = (char)puVar4[2];
  *(uint *)(iVar3 + 0xc) = (ushort)puVar4[3] & 0xff;
  uVar2 = FUN_0208455c();
  switch(uVar2 & 0xf) {
  case 0:
    cVar1 = *(char *)(iVar3 + 0x2b) + '\x01';
    break;
  case 1:
    cVar1 = *(char *)(iVar3 + 0x2b) + -1;
    break;
  case 2:
    cVar1 = *(char *)(iVar3 + 0x2b) + '\x02';
    break;
  case 3:
    cVar1 = *(char *)(iVar3 + 0x2b) + -2;
    break;
  default:
    goto switchD_0206dd28_default;
  }
  *(char *)(iVar3 + 0x2b) = cVar1;
switchD_0206dd28_default:
  *(byte *)(iVar3 + 0x2b) = *(byte *)(iVar3 + 0x2b) & 0x1f;
  return;
}

/* ================================================================
 * FUN_0206dd84 @ 0206dd84
 * ================================================================ */

void FUN_0206dd84(int param_1)

{
  char cVar1;
  int iVar2;

  cVar1 = *(char *)(param_1 + 0x17);
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  if (cVar1 == '\0') {
    iVar2 = FUN_0208455c();
    *(byte *)(param_1 + 0x17) = (byte)((uint)(iVar2 << 0x1c) >> 0x19);
    *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + -1;
  }
  FUN_02085980(param_1);
  return;
}

/* ================================================================
 * FUN_0206ddc8 @ 0206ddc8
 * ================================================================ */

void FUN_0206ddc8(int param_1)

{
  (**(code **)(DAT_0206dde0 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206dde4 @ 0206dde4
 * ================================================================ */

void FUN_0206dde4(int param_1)

{
  char cVar1;
  int iVar2;

  if (*(char *)(param_1 + 0x18) != '\0') {
    *(char *)(param_1 + 0x18) = *(char *)(param_1 + 0x18) + -1;
    return;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  FUN_02085980(param_1);
  cVar1 = *(char *)(param_1 + 0x17);
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  if (cVar1 == '\0') {
    *(undefined1 *)(param_1 + 0x17) = 0x78;
    FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  }
  iVar2 = FUN_0208da4c();
  if (*(char *)(param_1 + 0xb) != *(char *)(iVar2 + 0xcb0)) {
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206de60 @ 0206de60
 * ================================================================ */

void FUN_0206de60(int param_1)

{
  int iVar1;
  byte bVar2;
  ushort uVar3;
  int iVar4;
  char cVar5;
  int iVar6;
  bool bVar7;

  iVar6 = DAT_0206df10;
  if (*(byte *)(param_1 + 0x18) < 0xc) {
    *(byte *)(param_1 + 0x18) = *(byte *)(param_1 + 0x18) + 1;
    return;
  }
  bVar7 = *(char *)(param_1 + 0x14) == '\0';
  if (bVar7) {
    *(undefined1 *)(param_1 + 0x17) = 0;
  }
  bVar2 = *(byte *)(param_1 + 0x17);
  if (bVar7) {
    *(undefined1 *)(param_1 + 0x14) = 1;
  }
  iVar4 = DAT_0206df14;
  iVar1 = (uint)bVar2 * 2;
  *(short *)(DAT_0206df14 + 0x16) =
       *(short *)(DAT_0206df14 + 0x16) + (short)*(char *)(iVar6 + iVar1);
  uVar3 = *DAT_0206df18;
  *(short *)(iVar4 + 0x18) = *(short *)(iVar4 + 0x18) + (short)*(char *)(iVar6 + iVar1 + 1);
  if ((uVar3 & 0xf) == 0) {
    cVar5 = '\0';
    if (bVar2 != 0x17) {
      cVar5 = bVar2 + 1;
    }
    *(char *)(param_1 + 0x17) = cVar5;
  }
  iVar6 = FUN_0208da4c();
  if (*(char *)(param_1 + 0xb) != *(char *)(iVar6 + 0xcb0)) {
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206df1c @ 0206df1c
 * ================================================================ */

void FUN_0206df1c(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  int iVar3;

  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(undefined1 *)(param_1 + 0x17) = 0xc;
    *(undefined1 *)(param_1 + 0x18) = 0;
  }
  cVar1 = *(char *)(param_1 + 0x17);
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  if (cVar1 == '\0') {
    uVar2 = FUN_0208da4c();
    iVar3 = FUN_0208ddcc(uVar2,0x34);
    FUN_02029c60(iVar3 + (uint)*(ushort *)(DAT_0206dfe0 + (uint)*(byte *)(param_1 + 0x18) * 2),0xf,1
                );
    *(undefined1 *)(param_1 + 0x17) = 0xc;
    if (*(char *)(param_1 + 0x18) == '\x05') {
      cVar1 = '\0';
    }
    else {
      cVar1 = *(char *)(param_1 + 0x18) + '\x01';
    }
    *(char *)(param_1 + 0x18) = cVar1;
  }
  iVar3 = FUN_0208da4c();
  if (*(char *)(param_1 + 0xb) != *(char *)(iVar3 + 0xcb0)) {
    uVar2 = FUN_0208da4c();
    iVar3 = FUN_0208ddcc(uVar2,0x33);
    FUN_02029c60(iVar3 + 0x1e0,0xf,1);
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206e000 @ 0206e000
 * ================================================================ */

void FUN_0206e000(int param_1)

{
  int iVar1;

  if (*(char *)(param_1 + 0x18) != '\0') {
    *(char *)(param_1 + 0x18) = *(char *)(param_1 + 0x18) + -1;
    return;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  FUN_02085980(param_1);
  iVar1 = FUN_0208da4c();
  if (*(char *)(param_1 + 0xb) != *(char *)(iVar1 + 0xcb0)) {
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206e054 @ 0206e054
 * ================================================================ */

void FUN_0206e054(int param_1)

{
  int iVar1;

  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x40004000;
    FUN_0206e1b8(param_1);
  }
  else if (*(char *)(param_1 + 0x14) != '\x01') {
    return;
  }
  iVar1 = FUN_0208da4c();
  *(char *)(param_1 + 0x21) =
       *DAT_0206e114 + *(char *)(DAT_0206e118 + (uint)*(byte *)(iVar1 + 0xcb0)) + -2;
  *(short *)(param_1 + 0x32) = *(short *)(*(int *)(param_1 + 0x54) + 0x32) + 0x1c;
  *(short *)(param_1 + 0x36) = *(short *)(*(int *)(param_1 + 0x54) + 0x36) + -8;
  iVar1 = FUN_0208da4c();
  if ((*(char *)(iVar1 + 0xcb0) != '\x04') &&
     (iVar1 = FUN_0208da4c(), *(char *)(iVar1 + 0xcb0) != '\x05')) {
    return;
  }
  *(undefined2 *)(param_1 + 0x36) = 0xff;
  *(undefined2 *)(param_1 + 0x32) = 0xff;
  return;
}

/* ================================================================
 * FUN_0206e11c @ 0206e11c
 * ================================================================ */

void FUN_0206e11c(int param_1)

{
  int iVar1;

  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 1;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x40004000;
  }
  else if (*(char *)(param_1 + 0x14) != '\x01') {
    return;
  }
  iVar1 = *(int *)(param_1 + 0x54);
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | *(uint *)(iVar1 + 0x1c) & 3;
  *(ushort *)(param_1 + 0x32) =
       (ushort)*(byte *)(param_1 + 0xb) * 0xb + *(short *)(iVar1 + 0x32) + 10;
  *(short *)(param_1 + 0x36) = *(short *)(iVar1 + 0x36) + 0x10;
  FUN_0206e210(param_1);
  return;
}

/* ================================================================
 * FUN_0206e1b8 @ 0206e1b8
 * ================================================================ */

void FUN_0206e1b8(undefined4 param_1)

{
  byte *pbVar1;
  int iVar2;
  byte bVar3;

  pbVar1 = DAT_0206e20c;
  bVar3 = 0;
  if (*DAT_0206e20c != 0) {
    do {
      iVar2 = FUN_02076858(0x46,0x15,bVar3);
      if (iVar2 != 0) {
        *(undefined4 *)(iVar2 + 0x54) = param_1;
      }
      bVar3 = bVar3 + 1;
    } while (bVar3 < *pbVar1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206e210 @ 0206e210
 * ================================================================ */

void FUN_0206e210(int param_1)

{
  byte bVar1;
  int iVar2;
  uint uVar3;

  iVar2 = FUN_0208da4c();
  uVar3 = (uint)(*(byte *)(DAT_0206e2b8 + (uint)*(byte *)(param_1 + 0xb) * 8) &
                *(byte *)(DAT_0206e2b4 + (uint)*(byte *)(iVar2 + 0xcb0)));
  if (uVar3 != 0) {
    iVar2 = FUN_0208da4c();
    uVar3 = (int)uVar3 >> *(sbyte *)(DAT_0206e2bc + (uint)*(byte *)(iVar2 + 0xcb0)) & 0xff;
    iVar2 = FUN_0208da4c();
    *(undefined1 *)(param_1 + 0x21) = *(undefined1 *)(DAT_0206e2c0 + (uint)*(byte *)(iVar2 + 0xcb0))
    ;
    if (uVar3 == 3) {
      iVar2 = FUN_0208da4c();
      bVar1 = *(byte *)(DAT_0206e2c4 + *(byte *)(iVar2 + 0xcb0) + 4);
    }
    else {
      bVar1 = *(byte *)(DAT_0206e2c4 + uVar3);
    }
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfff0ffff | (bVar1 & 0xf) << 0x10;
    return;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
  return;
}

/* ================================================================
 * FUN_0206e2e4 @ 0206e2e4
 * ================================================================ */

void FUN_0206e2e4(int param_1)

{
  uint uVar1;
  bool bVar2;

  uVar1 = (uint)*(byte *)(param_1 + 10);
  bVar2 = uVar1 == 10;
  if (bVar2) {
    uVar1 = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0xc000;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x4000c000;
  if (bVar2) {
    *(uint *)(param_1 + 0x1c) = uVar1 | 0x80000000;
  }
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined1 *)(param_1 + 0x17) = 0x78;
  *(undefined1 *)(param_1 + 0x18) = 8;
  FUN_0206d5e0(param_1);
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_0206e34c @ 0206e34c
 * ================================================================ */

void FUN_0206e34c(int param_1)

{
  int iVar1;
  uint uVar2;
  bool bVar3;

  uVar2 = (uint)*(byte *)(param_1 + 0x18);
  bVar3 = uVar2 == 0;
  if (bVar3) {
    uVar2 = *(uint *)(param_1 + 0x1c) & 0xff00fffc | 0x660000;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xff00ffff | 0x660000;
  if (bVar3) {
    uVar2 = uVar2 | 1;
  }
  *(undefined1 *)(param_1 + 0x1b) = 0xb0;
  if (!bVar3) {
    *(char *)(param_1 + 0x18) = (char)uVar2 + -1;
    return;
  }
  *(uint *)(param_1 + 0x1c) = uVar2;
  FUN_02085980(param_1);
  iVar1 = FUN_0208da4c();
  if (*(char *)(param_1 + 0xb) != *(char *)(iVar1 + 0xcb0)) {
    FUN_0204be88(param_1);
    *(undefined1 *)(param_1 + 0x1b) = 0x6a;
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206e3d0 @ 0206e3d0
 * ================================================================ */

void FUN_0206e3d0(int param_1)

{
  (**(code **)(DAT_0206e3e8 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206e3ec @ 0206e3ec
 * ================================================================ */

void FUN_0206e3ec(int param_1)

{
  uint uVar1;
  bool bVar2;

  uVar1 = (uint)*(byte *)(param_1 + 10);
  bVar2 = uVar1 == 10;
  if (bVar2) {
    uVar1 = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0xc000;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x4000c000;
  if (bVar2) {
    *(uint *)(param_1 + 0x1c) = uVar1 | 0x80000000;
  }
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined1 *)(param_1 + 0x17) = 0x78;
  *(undefined1 *)(param_1 + 0x18) = 8;
  FUN_0206d5e0(param_1);
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_0206e454 @ 0206e454
 * ================================================================ */

void FUN_0206e454(int param_1)

{
  int iVar1;
  uint uVar2;
  bool bVar3;

  uVar2 = (uint)*(byte *)(param_1 + 0x18);
  bVar3 = uVar2 == 0;
  if (bVar3) {
    uVar2 = *(uint *)(param_1 + 0x1c) & 0xff00fffc | 0x880000;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xff00ffff | 0x880000;
  if (bVar3) {
    uVar2 = uVar2 | 1;
  }
  *(undefined1 *)(param_1 + 0x1b) = 0xb1;
  if (!bVar3) {
    *(char *)(param_1 + 0x18) = (char)uVar2 + -1;
    return;
  }
  *(uint *)(param_1 + 0x1c) = uVar2;
  FUN_02085980(param_1);
  iVar1 = FUN_0208da4c();
  if (*(char *)(param_1 + 0xb) != *(char *)(iVar1 + 0xcb0)) {
    FUN_0204be88(param_1);
    *(undefined1 *)(param_1 + 0x1b) = 0x6a;
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206e4d8 @ 0206e4d8
 * ================================================================ */

void FUN_0206e4d8(int param_1)

{
  (**(code **)(DAT_0206e4f0 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206e4f4 @ 0206e4f4
 * ================================================================ */

void FUN_0206e4f4(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 local_48 [4];
  undefined4 local_38;
  undefined4 local_34 [4];
  undefined4 local_24;
  undefined4 uStack_20;

  puVar1 = DAT_0206e594;
  uStack_20 = param_4;
  local_34[0] = *DAT_0206e590;
  local_34[1] = DAT_0206e590[1];
  local_34[2] = DAT_0206e590[2];
  local_34[3] = DAT_0206e590[3];
  puVar2 = DAT_0206e590 + 4;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x4000c000;
  *(undefined1 *)(param_1 + 0x17) = 0;
  *(undefined1 *)(param_1 + 0x18) = 0;
  local_48[0] = *puVar1;
  local_48[1] = puVar1[1];
  local_48[2] = puVar1[2];
  local_48[3] = puVar1[3];
  local_24 = *puVar2;
  local_38 = puVar1[4];
  *(short *)(param_1 + 0x32) = (short)local_34[*(byte *)(param_1 + 0xb)];
  *(short *)(param_1 + 0x36) = (short)local_48[*(byte *)(param_1 + 0xb)];
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_0206e598 @ 0206e598
 * ================================================================ */

void FUN_0206e598(int param_1)

{
  char cVar1;

  *(undefined1 *)(param_1 + 0x1b) = 0xb1;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xff00fffc | 0x770001;
  FUN_02085980(param_1);
  cVar1 = *(char *)(param_1 + 0x17);
  *(char *)(param_1 + 0x17) = cVar1 + '\x01';
  if (cVar1 != '\x1c') {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_0206e5fc @ 0206e5fc
 * ================================================================ */

void FUN_0206e5fc(int param_1)

{
  (**(code **)(DAT_0206e614 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206e618 @ 0206e618
 * ================================================================ */

void FUN_0206e618(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  uint uVar2;

  iVar1 = DAT_0206e684;
  uVar2 = (uint)*(byte *)(param_1 + 0xb);
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x4000c000;
  *(undefined1 *)(param_1 + 0x17) = 0;
  *(undefined1 *)(param_1 + 0x18) = 0;
  *(undefined4 *)(iVar1 + uVar2 * 4) = 0xfffffff6;
  *(undefined2 *)(param_1 + 0x32) = 0xe0;
  *(undefined2 *)(param_1 + 0x36) = 0x40;
  if (uVar2 == 7) {
    *(undefined2 *)(param_1 + 0x32) = 0xff;
  }
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10),0xe0,uVar2,param_4);
  return;
}

/* ================================================================
 * FUN_0206e688 @ 0206e688
 * ================================================================ */

void FUN_0206e688(int param_1)

{
  longlong lVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int iVar5;

  uVar3 = (uint)*(byte *)(param_1 + 0xb);
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xff00ffff | 0x770000;
  iVar2 = DAT_0206e86c;
  *(undefined1 *)(param_1 + 0x1b) = 0xb1;
  iVar4 = DAT_0206e870;
  switch(uVar3) {
  case 0:
    iVar5 = *(int *)(iVar2 + uVar3 * 4) + 1;
    *(int *)(iVar2 + uVar3 * 4) = iVar5;
    *(short *)(param_1 + 0x32) = 0xe4 - (short)(iVar5 / 2);
    iVar5 = *(int *)(iVar2 + uVar3 * 4);
    iVar5 = iVar5 * iVar5;
    *(short *)(param_1 + 0x36) =
         ((short)(int)((longlong)iVar4 * (longlong)iVar5 >> 0x23) - (short)(iVar5 >> 0x1f)) + 0x4e;
    iVar2 = *(int *)(iVar2 + uVar3 * 4);
    goto joined_r0x0206e7d8;
  case 1:
    iVar5 = *(int *)(iVar2 + uVar3 * 4) + 1;
    *(int *)(iVar2 + uVar3 * 4) = iVar5;
    *(short *)(param_1 + 0x32) = (short)(iVar5 / 2) + 0xee;
    iVar5 = *(int *)(iVar2 + uVar3 * 4);
    iVar5 = iVar5 * iVar5;
    *(short *)(param_1 + 0x36) =
         ((short)(int)((longlong)iVar4 * (longlong)iVar5 >> 0x22) - (short)(iVar5 >> 0x1f)) + 0x52;
    iVar2 = *(int *)(iVar2 + uVar3 * 4);
    goto joined_r0x0206e82c;
  case 2:
    iVar4 = *(int *)(iVar2 + uVar3 * 4) + 1;
    lVar1 = (longlong)DAT_0206e874;
    *(int *)(iVar2 + uVar3 * 4) = iVar4;
    *(short *)(param_1 + 0x32) =
         ((short)((ulonglong)(lVar1 * iVar4) >> 0x20) - (short)(iVar4 >> 0x1f)) + 0xea;
    iVar4 = *(int *)(iVar2 + uVar3 * 4);
    iVar4 = iVar4 * iVar4;
    *(short *)(param_1 + 0x36) =
         ((short)(iVar4 + (int)((ulonglong)((longlong)DAT_0206e878 * (longlong)iVar4) >> 0x20) >> 4)
         - (short)(iVar4 >> 0x1f)) + 0x4a;
    iVar2 = *(int *)(iVar2 + uVar3 * 4);
joined_r0x0206e7d8:
    if (0x14 < iVar2) {
LAB_0206e738:
      FUN_0204be88(param_1);
    }
    break;
  case 3:
    iVar4 = *(int *)(iVar2 + uVar3 * 4) + 1;
    lVar1 = (longlong)DAT_0206e874;
    *(int *)(iVar2 + uVar3 * 4) = iVar4;
    *(short *)(param_1 + 0x32) =
         0xe0 - ((short)((ulonglong)(lVar1 * iVar4) >> 0x20) - (short)(iVar4 >> 0x1f));
    iVar4 = *(int *)(iVar2 + uVar3 * 4);
    iVar4 = iVar4 * iVar4;
    *(short *)(param_1 + 0x36) =
         ((short)(iVar4 + (int)((ulonglong)((longlong)DAT_0206e878 * (longlong)iVar4) >> 0x20) >> 4)
         - (short)(iVar4 >> 0x1f)) + 0x4c;
    iVar2 = *(int *)(iVar2 + uVar3 * 4);
joined_r0x0206e82c:
    if (iVar2 < 0x10) break;
    goto LAB_0206e738;
  case 4:
    break;
  case 5:
    break;
  case 6:
    break;
  case 7:
    iVar4 = *(int *)(iVar2 + uVar3 * 4) + 1;
    *(int *)(iVar2 + uVar3 * 4) = iVar4;
    if (iVar4 < 0x5b) {
      return;
    }
    FUN_0204be88(param_1);
    return;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  FUN_02085980(param_1);
  return;
}
