#include "game/arm9_middle_020693fc_raw.h"

/*
 * Raw matching-first recovery for the 32 catalog entries beginning at
 * 0x020693fc. These bodies remain address-named promotion candidates and are
 * not linked byte-matching evidence.
 *
 * Calls whose implicit r0 argument was omitted by Ghidra are written with the
 * owning param_1 so this mechanically faithful draft remains valid C.
 */

extern uint DAT_02069428;
extern uint DAT_02069494;
extern uint DAT_020694f0;
extern uint DAT_020695c4;
extern uint DAT_0206962c;
extern uint DAT_020696a4;
extern uint DAT_0206973c;
extern uint DAT_020697a8;
extern uint DAT_020697e4;
extern uint DAT_02069834;
extern uint DAT_02069838;
extern uint DAT_02069a30;
extern uint DAT_02069a34;
extern uint DAT_02069adc;
extern uint DAT_02069ae0;
extern uint DAT_02069ba0;
extern uint *DAT_02069ba4;
extern uint DAT_02069ba8;
extern uint DAT_02069c1c;
extern uint DAT_02069d60;
extern byte *DAT_02069e04;
extern byte *DAT_02069e08;
extern uint DAT_02069e3c;
extern uint DAT_02069e40;
extern int *DAT_02069f58;
extern uint DAT_02069ff8;
extern uint DAT_0206a0c4;
extern uint DAT_0206a7a8;
extern uint DAT_0206a8d8;
extern uint DAT_0206a8dc;
extern uint DAT_0206a8e0;
extern uint DAT_0206ab5c;
extern uint DAT_0206ab60;
extern uint DAT_0206ab64;
extern uint DAT_0206ab68;
extern byte *DAT_0206b060;
extern uint DAT_0206b064;
extern uint DAT_0206b400;
extern uint DAT_0206b45c;
extern uint PTR_DAT_02069d54;
extern uint PTR_DAT_02069d58;
extern uint PTR_DAT_02069d5c;
extern undefined *PTR_DAT_0206a7ac;
extern uint PTR_DAT_0206a7b0;
extern uint PTR_DAT_0206a7b4;

/* ================================================================
 * FUN_020693fc @ 020693fc
 * ================================================================ */

void FUN_020693fc(undefined4 param_1)

{
  int iVar1;

  iVar1 = DAT_02069428;
  if (*(char *)(DAT_02069428 + 8) != '\x05') {
    return;
  }
  FUN_0205b67c(param_1,0);
  *(char *)(iVar1 + 8) = *(char *)(iVar1 + 8) + '\x01';
  return;
}

/* ================================================================
 * FUN_02069448 @ 02069448
 * ================================================================ */

void FUN_02069448(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  int iVar2;
  int iVar3;

  iVar2 = DAT_02069494;
  iVar1 = (uint)*(byte *)(param_1 + 0xb) * 3;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff;
  iVar3 = iVar2 + iVar1;
  *(ushort *)(param_1 + 0x32) = (ushort)*(byte *)(iVar2 + iVar1);
  *(ushort *)(param_1 + 0x36) = (ushort)*(byte *)(iVar3 + 1);
  FUN_020859e0(param_1,*(undefined1 *)(iVar3 + 2),iVar3,iVar2,param_4);
  return;
}

/* ================================================================
 * FUN_02069498 @ 02069498
 * ================================================================ */

void FUN_02069498(int param_1)

{
  uint uVar1;
  bool bVar2;

  uVar1 = (uint)*(byte *)(param_1 + 0xb);
  bVar2 = *(byte *)(DAT_020694f0 + 2) == uVar1;
  if (bVar2) {
    uVar1 = 2;
  }
  if (!bVar2) {
    uVar1 = 0;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | uVar1 & 3;
  FUN_02085980(param_1);
  if (*(char *)(DAT_020694f0 + 8) != '\x01') {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_020694f4 @ 020694f4
 * ================================================================ */

void FUN_020694f4(int param_1)

{
  FUN_02085980(param_1);
  if (*(char *)(param_1 + 0x15) == '\0') {
    FUN_0202b5d4(0x1f,0x3f);
    FUN_0202b620((int)*(short *)(param_1 + 0x32),(int)*(short *)(param_1 + 0x36),0);
    *(undefined2 *)(param_1 + 0x60) = 0;
    FUN_0204625c(2,8);
    *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
    return;
  }
  if (*(char *)(param_1 + 0x15) != '\x01') {
    return;
  }
  if (*(ushort *)(param_1 + 0x60) < 300) {
    *(ushort *)(param_1 + 0x60) = *(ushort *)(param_1 + 0x60) + 4;
    FUN_0202b620((int)*(short *)(param_1 + 0x32),(int)*(short *)(param_1 + 0x36),
                 (int)*(short *)(param_1 + 0x60));
    return;
  }
  *(undefined1 *)(param_1 + 0x15) = 0;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  FUN_0202b598();
  return;
}

/* ================================================================
 * FUN_0206959c @ 0206959c
 * ================================================================ */

void FUN_0206959c(undefined4 param_1)

{
  FUN_02085980(param_1);
  if (*(byte *)(DAT_020695c4 + 8) < 2) {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_020695c8 @ 020695c8
 * ================================================================ */

void FUN_020695c8(int param_1)

{
  int iVar1;
  int iVar2;

  iVar1 = DAT_0206962c;
  if (*(char *)(param_1 + 0x14) != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x4002;
  iVar2 = iVar1 + (uint)*(byte *)(param_1 + 10) * 6;
  *(undefined2 *)(param_1 + 0x32) = *(undefined2 *)(iVar1 + (uint)*(byte *)(param_1 + 10) * 6);
  *(undefined2 *)(param_1 + 0x36) = *(undefined2 *)(iVar2 + 2);
  *(char *)(param_1 + 0x21) = (char)*(undefined2 *)(iVar2 + 4);
  return;
}

/* ================================================================
 * FUN_0206964c @ 0206964c
 * ================================================================ */

void FUN_0206964c(int param_1)

{
  int iVar1;
  int iVar2;

  iVar1 = DAT_020696a4;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 2;
  iVar2 = iVar1 + (uint)*(byte *)(param_1 + 10) * 6;
  *(undefined2 *)(param_1 + 0x32) = *(undefined2 *)(iVar1 + (uint)*(byte *)(param_1 + 10) * 6);
  *(undefined2 *)(param_1 + 0x36) = *(undefined2 *)(iVar2 + 2);
  *(char *)(param_1 + 0x21) = (char)*(undefined2 *)(iVar2 + 4);
  return;
}

/* ================================================================
 * FUN_02069700 @ 02069700
 * ================================================================ */

void FUN_02069700(int param_1)

{
  *(short *)(param_1 + 0x36) = *(short *)(param_1 + 0x36) + -4;
  if (-0x21 < *(short *)(param_1 + 0x36)) {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02069724 @ 02069724
 * ================================================================ */

void FUN_02069724(int param_1)

{
  (**(code **)(DAT_0206973c + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_02069740 @ 02069740
 * ================================================================ */

void FUN_02069740(int param_1)

{
  int iVar1;
  uint uVar2;

  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0xc000c002;
  *(undefined1 *)(param_1 + 0x2b) = 8;
  uVar2 = FUN_0208455c();
  iVar1 = DAT_020697a8;
  *(uint *)(param_1 + 0xc) = (uVar2 & 0x3f) + 0x40;
  *(char *)(param_1 + 0x21) = (char)*(undefined2 *)(iVar1 + (uint)*(byte *)(param_1 + 10) * 6);
  return;
}

/* ================================================================
 * FUN_020697cc @ 020697cc
 * ================================================================ */

void FUN_020697cc(int param_1)

{
  (**(code **)(DAT_020697e4 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_020697e8 @ 020697e8
 * ================================================================ */

void FUN_020697e8(int param_1)

{
  byte bVar1;
  short sVar2;

  bVar1 = *(byte *)(DAT_02069834 + 9);
  sVar2 = *(short *)(DAT_02069838 + (uint)*(byte *)(param_1 + 0x60) * 2);
  *(short *)(param_1 + 0x32) =
       (short)((uint)((int)(short)(ushort)bVar1 *
                     (int)*(short *)(DAT_02069838 + (*(byte *)(param_1 + 0x60) + 0x40) * 2)) >> 8) +
       0x80;
  *(short *)(param_1 + 0x36) = (short)((uint)((int)(short)(ushort)bVar1 * (int)sVar2) >> 8) + 0x6c;
  return;
}

/* ================================================================
 * FUN_020699a4 @ 020699a4
 * ================================================================ */

void FUN_020699a4(int param_1)

{
  int iVar1;

  iVar1 = DAT_02069a30;
  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3ff03ffc | 0x50002;
    *(undefined2 *)(param_1 + 0x32) = *(undefined2 *)(iVar1 + (uint)*(byte *)(param_1 + 10) * 6);
    *(undefined2 *)(param_1 + 0x36) = *(undefined2 *)(iVar1 + (uint)*(byte *)(param_1 + 10) * 6 + 2)
    ;
    FUN_020859e0(param_1,2);
  }
  FUN_02085980(param_1);
  if (*(char *)(DAT_02069a34 + 8) == '\0') {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02069a38 @ 02069a38
 * ================================================================ */

void FUN_02069a38(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  short sVar1;
  int iVar2;
  int iVar3;

  iVar2 = DAT_02069adc;
  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(undefined1 *)(param_1 + 0x1b) = 0x6b;
    *(undefined1 *)(param_1 + 0x20) = 0x40;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3ff03f3c | 0x50082;
    iVar3 = iVar2 + (uint)*(byte *)(param_1 + 10) * 6;
    *(undefined2 *)(param_1 + 0x32) = *(undefined2 *)(iVar2 + (uint)*(byte *)(param_1 + 10) * 6);
    sVar1 = *(short *)(iVar3 + 2);
    *(short *)(param_1 + 0x36) = sVar1;
    FUN_020859e0(param_1,0,(int)sVar1,iVar3,param_4);
  }
  FUN_02085980(param_1);
  if (*(char *)(DAT_02069ae0 + 8) == '\0') {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02069ae4 @ 02069ae4
 * ================================================================ */

void FUN_02069ae4(int param_1)

{
  int iVar1;
  uint uVar2;

  iVar1 = DAT_02069ba0;
  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 3;
    *(undefined1 *)(param_1 + 0x1b) = 0x4e;
    *(undefined1 *)(param_1 + 0x20) = 0x80;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3ff03f3f | 0x100c0;
    *(undefined2 *)(param_1 + 0x32) = *(undefined2 *)(iVar1 + (uint)*(byte *)(param_1 + 10) * 6);
    *(undefined2 *)(param_1 + 0x36) = *(undefined2 *)(iVar1 + (uint)*(byte *)(param_1 + 10) * 6 + 2)
    ;
  }
  iVar1 = DAT_02069ba8;
  if ((*DAT_02069ba4 & 0x10) == 0) {
    uVar2 = 0;
  }
  else {
    uVar2 = 2;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | uVar2;
  if (*(char *)(iVar1 + 8) != '\0') {
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02069bac @ 02069bac
 * ================================================================ */

void FUN_02069bac(int param_1)

{
  int iVar1;

  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 2;
    *(undefined2 *)(param_1 + 0x32) = 8;
    *(undefined2 *)(param_1 + 0x36) = 0xa0;
  }
  iVar1 = DAT_02069c1c;
  *(undefined1 *)(param_1 + 0x1b) = 0xb2;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfff0ffff | 0x80000;
  if (*(char *)(iVar1 + 8) == '\0') {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02069c20 @ 02069c20
 * ================================================================ */

void FUN_02069c20(int param_1)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  int iVar4;
  uint uVar5;
  bool bVar6;
  ushort local_30 [4];
  undefined2 local_28;
  undefined2 local_26;
  undefined2 local_24;
  undefined2 local_22;
  undefined2 local_20;

  local_26 = *(undefined2 *)(PTR_DAT_02069d58 + 4);
  local_30[0] = *(ushort *)PTR_DAT_02069d5c;
  local_30[1] = *(undefined2 *)(PTR_DAT_02069d5c + 2);
  local_24 = *(undefined2 *)PTR_DAT_02069d54;
  local_22 = *(undefined2 *)(PTR_DAT_02069d54 + 2);
  local_20 = *(undefined2 *)(PTR_DAT_02069d54 + 4);
  local_30[3] = *(undefined2 *)PTR_DAT_02069d58;
  local_28 = *(undefined2 *)(PTR_DAT_02069d58 + 2);
  local_30[2] = *(undefined2 *)(PTR_DAT_02069d5c + 4);
  if (*(char *)(param_1 + 0x14) == '\0') {
    uVar5 = (uint)*(byte *)(param_1 + 0xb);
    uVar1 = local_30[uVar5 + 6];
    uVar2 = local_30[uVar5 + 3];
    uVar3 = local_30[uVar5];
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 2;
    *(ushort *)(param_1 + 0x32) = uVar1 + 8;
    *(ushort *)(param_1 + 0x36) = uVar2 + 0x10;
    FUN_020859e0(param_1,uVar3 & 0xff);
  }
  iVar4 = DAT_02069d60;
  *(undefined1 *)(param_1 + 0x1b) = 0xb3;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfff0ffff | 0x60000;
  uVar5 = (uint)*(byte *)(param_1 + 0xb);
  bVar6 = *(byte *)(iVar4 + 2) == uVar5;
  if (bVar6) {
    uVar5 = 2;
  }
  if (!bVar6) {
    uVar5 = 0;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | uVar5 & 3;
  FUN_02085980(param_1);
  if (*(char *)(DAT_02069d60 + 8) != '\0') {
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02069d64 @ 02069d64
 * ================================================================ */

void FUN_02069d64(void)

{
  byte *pbVar1;
  byte *pbVar2;
  int iVar3;
  int iVar4;
  byte bVar5;

  bVar5 = 0;
  FUN_02076898(0x32,1,0);
  FUN_02076898(0x32,3,0);
  pbVar2 = DAT_02069e08;
  pbVar1 = DAT_02069e04;
  if (*DAT_02069e04 != 0) {
    do {
      iVar3 = FUN_02076898(0x32,2,bVar5);
      if (((iVar3 != 0) && (bVar5 == *pbVar2)) && (iVar4 = FUN_02076898(0x32,0,0), iVar4 != 0)) {
        *(int *)(iVar4 + 0x58) = iVar3;
      }
      bVar5 = bVar5 + 1;
    } while (bVar5 < *pbVar1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02069e0c @ 02069e0c
 * ================================================================ */

void FUN_02069e0c(int param_1)

{
  if (*(char *)(DAT_02069e3c + 5) == '\0') {
    FUN_0204be88(param_1);
    return;
  }
  (**(code **)(DAT_02069e40 + (uint)*(byte *)(param_1 + 10) * 4))();
  return;
}

/* ================================================================
 * FUN_02069e44 @ 02069e44
 * ================================================================ */

void FUN_02069e44(int param_1)

{
  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3ff03ffc | 0x4002;
    FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  }
  *(undefined2 *)(param_1 + 0x32) = *(undefined2 *)(*(int *)(param_1 + 0x58) + 0x32);
  *(undefined2 *)(param_1 + 0x36) = *(undefined2 *)(*(int *)(param_1 + 0x58) + 0x36);
  return;
}

/* ================================================================
 * FUN_02069ea0 @ 02069ea0
 * ================================================================ */

void FUN_02069ea0(int param_1)

{
  char cVar1;
  int *piVar2;
  uint uVar3;
  int iVar4;

  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3ff03fff;
    FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  }
  piVar2 = DAT_02069f58;
  uVar3 = *(uint *)(param_1 + 0x1c);
  *(uint *)(param_1 + 0x1c) = uVar3 & 0xfffffffc;
  iVar4 = *piVar2;
  cVar1 = '\0';
  if (iVar4 != 0) {
    cVar1 = *(char *)(iVar4 + 0x14);
  }
  if (iVar4 != 0 && cVar1 != '\x03') {
    *(uint *)(param_1 + 0x1c) = uVar3 & 0xfffffffc | 2;
    *(ushort *)(param_1 + 0x32) = (*(short *)(*piVar2 + 0x32) >> 4 & 0x3fU) + 0x58;
    *(ushort *)(param_1 + 0x36) = (*(short *)(*piVar2 + 0x36) >> 4 & 0x3fU) + 0x30;
    return;
  }
  return;
}

/* ================================================================
 * FUN_02069f5c @ 02069f5c
 * ================================================================ */

void FUN_02069f5c(int param_1)

{
  int iVar1;

  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(uint *)(param_1 + 0x1c) =
         *(uint *)(param_1 + 0x1c) & 0x3ff03ffc | 2 | (*(byte *)(param_1 + 0xb) + 7 & 0xf) << 0x10 |
         0x4000;
    FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  }
  iVar1 = *(int *)(DAT_02069ff8 + (short)(ushort)*(byte *)(param_1 + 0xb) * 0x94);
  *(ushort *)(param_1 + 0x32) = (*(short *)(iVar1 + 0x32) >> 4 & 0x3fU) + 0x58;
  *(ushort *)(param_1 + 0x36) = (*(short *)(iVar1 + 0x36) >> 4 & 0x3fU) + 0x30;
  FUN_02085980(param_1);
  return;
}

/* ================================================================
 * FUN_02069ffc @ 02069ffc
 * ================================================================ */

void FUN_02069ffc(int param_1)

{
  int iVar1;

  iVar1 = DAT_0206a0c4;
  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3ff03fff | 0x4000;
    *(ushort *)(param_1 + 0x32) = (*(short *)(iVar1 + 0x80) >> 4 & 0x3fU) + 0x58;
    *(ushort *)(param_1 + 0x36) = (*(short *)(iVar1 + 0x82) >> 4 & 0x3fU) + 0x30;
    FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  }
  iVar1 = DAT_0206a0c4;
  if (*(short *)(DAT_0206a0c4 + 0x80) == 0) {
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
  }
  else {
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 2;
    *(ushort *)(param_1 + 0x32) = (*(short *)(iVar1 + 0x80) >> 4 & 0x3fU) + 0x58;
    *(ushort *)(param_1 + 0x36) = (*(short *)(iVar1 + 0x82) >> 4 & 0x3fU) + 0x30;
  }
  FUN_02085980(param_1);
  return;
}

/* ================================================================
 * FUN_0206a0e4 @ 0206a0e4
 * ================================================================ */

void FUN_0206a0e4(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;
  ushort uVar2;
  ushort uVar3;
  short sVar4;
  short sVar5;
  short sVar6;
  undefined *puVar7;
  uint uVar8;
  uint uVar9;
  undefined4 uVar10;
  undefined2 uVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  byte bVar15;
  uint uVar16;
  bool bVar17;

  bVar17 = *(char *)(param_1 + 10) == '\0';
  uVar16 = *(uint *)(param_1 + 0x1c) & 0xffffffcc;
  uVar8 = uVar16 | 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 3;
  if (bVar17) {
    uVar8 = uVar16 | 0x21;
  }
  uVar16 = (uint)*(byte *)(param_1 + 10);
  if (!bVar17) {
    uVar8 = uVar8 | 0x30;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(uint *)(param_1 + 0x1c) = uVar8;
  iVar13 = DAT_0206a7a8;
  bVar15 = *(byte *)(param_1 + 0x2b) ^ 0x10;
  *(undefined1 *)(param_1 + 0xb) = 0;
  iVar12 = ((int)(uint)bVar15 >> 2) + uVar16 * 8;
  sVar4 = *(short *)(param_1 + 0x32);
  uVar2 = *(ushort *)(iVar13 + 0x18);
  uVar3 = *(ushort *)(iVar13 + 0x1a);
  sVar6 = (short)((uVar16 + 1) * 0x80000 >> 0x10);
  *(short *)(param_1 + 0x32) = sVar4 + sVar6;
  puVar7 = PTR_DAT_0206a7ac;
  sVar5 = *(short *)(param_1 + 0x36);
  *(short *)(param_1 + 0x36) = sVar5 + sVar6;
  uVar9 = ((int)((int)sVar5 - (uint)uVar3) >> 4) << 6 | (int)((int)sVar4 - (uint)uVar2) >> 4;
  iVar14 = (int)*(short *)(param_1 + 0x32) + (int)*(short *)(puVar7 + iVar12 * 2);
  iVar13 = (int)*(short *)(param_1 + 0x36) + (int)*(short *)(puVar7 + (iVar12 + 1) * 2);
  *(uint *)(param_1 + 0xc) = (uint)*(ushort *)(PTR_DAT_0206a7b0 + uVar16 * 2);
  uVar8 = uVar9 & 0xffff;
  *(byte *)(param_1 + 0x2b) = bVar15;
  *(short *)(param_1 + 0x68) = (short)iVar14;
  *(short *)(param_1 + 0x6a) = (short)iVar13;
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(undefined1 *)(param_1 + 0x18) = 0x10;
  FUN_020859e0(param_1,uVar16 == 0,iVar13,iVar14,param_4);
  FUN_02081088(param_1,*(undefined2 *)(PTR_DAT_0206a7b4 + (uint)*(byte *)(param_1 + 10) * 2));
  cVar1 = *(char *)(param_1 + 10);
  if ((int)(uint)*(ushort *)(param_1 + 0x50) >> 0xe == 2) {
    if (cVar1 == '\0') {
      FUN_02021c28(0x5b,uVar9 & 0xffff);
      FUN_02021c28(0x5c,uVar8 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x5d,uVar8 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      uVar11 = *(undefined2 *)(param_1 + 0x50);
      uVar16 = uVar8 + 0x41 & 0xffff;
      uVar10 = 0x5e;
    }
    else if (cVar1 == '\x01') {
      FUN_02021c28(0x5f,uVar9 & 0xffff);
      FUN_02021c28(0x60,uVar8 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x61,uVar8 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x62,uVar8 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(99,uVar8 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(100,uVar8 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x65,uVar8 + 0x80 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x66,uVar8 + 0x81 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      uVar16 = uVar8 + 0x82 & 0xffff;
      uVar11 = *(undefined2 *)(param_1 + 0x50);
      uVar10 = 0x67;
    }
    else {
      if (cVar1 != '\x02') goto LAB_0206a79c;
      FUN_02021c28(0x68,uVar9 & 0xffff);
      FUN_02021c28(0x69,uVar8 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x6a,uVar8 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x6b,uVar8 + 3 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x6c,uVar8 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x6d,uVar8 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x6e,uVar8 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x6f,uVar8 + 0x43 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x70,uVar8 + 0x80 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x71,uVar8 + 0x81 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x72,uVar8 + 0x82 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x73,uVar8 + 0x83 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x74,uVar8 + 0xc0 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x75,uVar8 + 0xc1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x76,uVar8 + 0xc2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      uVar11 = *(undefined2 *)(param_1 + 0x50);
      uVar16 = uVar8 + 0xc3 & 0xffff;
      uVar10 = 0x77;
    }
  }
  else if (cVar1 == '\0') {
    FUN_02021c28(0x32,uVar9 & 0xffff);
    FUN_02021c28(0x33,uVar8 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x34,uVar8 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    uVar11 = *(undefined2 *)(param_1 + 0x50);
    uVar16 = uVar8 + 0x41 & 0xffff;
    uVar10 = 0x35;
  }
  else if (cVar1 == '\x01') {
    FUN_02021c28(0x36,uVar9 & 0xffff);
    FUN_02021c28(0x37,uVar8 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x38,uVar8 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x39,uVar8 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x3a,uVar8 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x3b,uVar8 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x3c,uVar8 + 0x80 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x3d,uVar8 + 0x81 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    uVar16 = uVar8 + 0x82 & 0xffff;
    uVar11 = *(undefined2 *)(param_1 + 0x50);
    uVar10 = 0x3e;
  }
  else {
    if (cVar1 != '\x02') goto LAB_0206a79c;
    FUN_02021c28(0x3f,uVar9 & 0xffff);
    FUN_02021c28(0x40,uVar8 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x41,uVar8 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x42,uVar8 + 3 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x43,uVar8 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x44,uVar8 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x45,uVar8 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x46,uVar8 + 0x43 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x47,uVar8 + 0x80 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x48,uVar8 + 0x81 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x49,uVar8 + 0x82 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x4a,uVar8 + 0x83 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x4b,uVar8 + 0xc0 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x4c,uVar8 + 0xc1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x4d,uVar8 + 0xc2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    uVar11 = *(undefined2 *)(param_1 + 0x50);
    uVar16 = uVar8 + 0xc3 & 0xffff;
    uVar10 = 0x4e;
  }
  FUN_02021c28(uVar10,uVar16,uVar11);
LAB_0206a79c:
  FUN_02085f80(0xb);
  return;
}

/* ================================================================
 * FUN_0206a7b8 @ 0206a7b8
 * ================================================================ */

void FUN_0206a7b8(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;
  int iVar2;

  FUN_0204fd40();
  cVar1 = *(char *)(param_1 + 0x18) + -1;
  *(int *)(param_1 + 0x38) = *(int *)(param_1 + 0x38) + -0x8000;
  *(char *)(param_1 + 0x18) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 2;
  *(undefined2 *)(param_1 + 0x32) = *(undefined2 *)(param_1 + 0x68);
  *(undefined2 *)(param_1 + 0x36) = *(undefined2 *)(param_1 + 0x6a);
  *(undefined2 *)(param_1 + 0x3a) = 0xfff0;
  iVar2 = FUN_02076858(0xf,*(char *)(param_1 + 10) + '\b',0,0xfffffff0,param_4);
  if (iVar2 == 0) {
    return;
  }
  FUN_0205031c(param_1);
  return;
}

/* ================================================================
 * FUN_0206a830 @ 0206a830
 * ================================================================ */

void FUN_0206a830(int param_1)

{
  undefined1 uVar1;
  ushort uVar2;
  int iVar3;

  iVar3 = DAT_0206a8d8;
  if (*(char *)(param_1 + 0xb) != '\0') {
    if ((*(byte *)(param_1 + 0x17) & 1) != 0) {
      *(undefined1 *)(DAT_0206a8dc + 0x39) = 1;
    }
    if ((*(byte *)(param_1 + 0x17) & 2) != 0) {
      *(undefined1 *)(DAT_0206a8dc + 0xcd) = 1;
    }
    if ((*(byte *)(param_1 + 0x17) & 4) != 0) {
      *(undefined1 *)(DAT_0206a8dc + 0x161) = 1;
    }
    if ((*(byte *)(param_1 + 0x17) & 8) != 0) {
      *(undefined1 *)(DAT_0206a8dc + 0x1f5) = 1;
    }
    uVar2 = *(ushort *)(iVar3 + (uint)*(byte *)(param_1 + 10) * 2);
    uVar1 = *(undefined1 *)(DAT_0206a8e0 + (uint)*(byte *)(param_1 + 10));
    *(undefined1 *)(param_1 + 0x14) = 3;
    *(uint *)(param_1 + 0xc) = (uint)uVar2;
    *(byte *)(param_1 + 0x2b) = *(byte *)(param_1 + 0x2b) ^ 0x10;
    *(undefined4 *)(param_1 + 0x10) = 0xc000;
    *(undefined1 *)(param_1 + 0x2e) = uVar1;
    FUN_02085f80(0xc);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206a8e4 @ 0206a8e4
 * ================================================================ */

void FUN_0206a8e4(int param_1)

{
  int iVar1;

  FUN_020847c8();
  FUN_0208af0c(param_1);
  if ((*(ushort *)(param_1 + 0x50) & 0x8000) == 0) {
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x40004000;
  }
  iVar1 = FUN_020856cc(param_1,0x1000);
  if (iVar1 != 0) {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 4;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
  FUN_0206a964(param_1);
  FUN_0205747c(0x1e,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_0206a964 @ 0206a964
 * ================================================================ */

void FUN_0206a964(int param_1)

{
  char cVar1;
  byte bVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  undefined4 uVar6;
  int unaff_r8;
  int unaff_r9;

  cVar1 = *(char *)(param_1 + 10);
  if (cVar1 == '\0') {
    iVar3 = FUN_02076858(0xf,0xe,0);
    if (iVar3 != 0) {
      FUN_0205031c(param_1);
    }
    uVar6 = 0x6b;
    unaff_r9 = 3;
    unaff_r8 = DAT_0206ab5c;
  }
  else if (cVar1 == '\x01') {
    iVar3 = FUN_02076858(0xf,0xe,0);
    if (iVar3 != 0) {
      FUN_02050390(param_1,iVar3,0,0x80000);
    }
    iVar3 = FUN_02076858(0xf,0xe,0);
    if (iVar3 != 0) {
      FUN_02050390(param_1,iVar3,0,0xfff80000);
    }
    uVar6 = 0x6c;
    unaff_r9 = 4;
    unaff_r8 = DAT_0206ab60;
  }
  else {
    if (cVar1 != '\x02') goto LAB_0206aadc;
    iVar3 = FUN_02076858(0xf,0xe,0);
    if (iVar3 != 0) {
      FUN_02050390(param_1,iVar3,0xfff00000,0xfff00000);
    }
    iVar3 = FUN_02076858(0xf,0xe,0);
    if (iVar3 != 0) {
      FUN_02050390(param_1,iVar3,0xfff00000,0x100000);
    }
    iVar3 = FUN_02076858(0xf,0xe,0);
    if (iVar3 != 0) {
      FUN_02050390(param_1,iVar3,0x100000,0xfff00000);
    }
    iVar3 = FUN_02076858(0xf,0xe,0);
    if (iVar3 != 0) {
      FUN_02050390(param_1,iVar3,0x100000,0x100000);
    }
    uVar6 = 0x6d;
    unaff_r9 = 5;
    unaff_r8 = DAT_0206ab64;
  }
  FUN_02085f98(param_1,uVar6);
LAB_0206aadc:
  iVar3 = DAT_0206ab68;
  if (-1 < unaff_r9) {
    do {
      uVar4 = FUN_0208455c();
      iVar5 = FUN_02076858(0,*(ushort *)(iVar3 + (uVar4 & 0x1f) * 2) & 0xff,1);
      if (iVar5 != 0) {
        bVar2 = FUN_0208455c();
        *(byte *)(iVar5 + 0x2b) = bVar2 & 0x1f | 0x80;
        FUN_02050390(param_1,iVar5,(int)*(char *)(unaff_r8 + unaff_r9 * 2) << 0x10,
                     (int)*(char *)(unaff_r8 + unaff_r9 * 2 + 1) << 0x10);
      }
      unaff_r9 = (unaff_r9 + -1) * 0x10000 >> 0x10;
    } while (-1 < unaff_r9);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206b000 @ 0206b000
 * ================================================================ */

undefined4 FUN_0206b000(undefined4 param_1)

{
  byte *pbVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;

  iVar2 = DAT_0206b064;
  pbVar1 = DAT_0206b060;
  uVar5 = 0;
  if (*DAT_0206b060 != 0) {
    do {
      iVar3 = FUN_02083ed8(param_1,*(undefined4 *)(uVar5 * 0x94 + iVar2 + 0x28));
      if (iVar3 != 0) {
        return 1;
      }
      uVar4 = uVar5 + 1;
      uVar5 = uVar4 & 0xffff;
    } while ((uVar4 & 0xffff) < (uint)*pbVar1);
  }
  return 0;
}

/* ================================================================
 * FUN_0206b368 @ 0206b368
 * ================================================================ */

void FUN_0206b368(int param_1)

{
  int iVar1;

  *(ushort *)(param_1 + 0x32) = (*(ushort *)(param_1 + 0x32) & 0x3f0) + 8;
  *(ushort *)(param_1 + 0x36) = (*(ushort *)(param_1 + 0x36) & 0x3f0) + 8;
  iVar1 = DAT_0206b400;
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc;
  *(ushort *)(param_1 + 0x7c) =
       (ushort)(((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(iVar1 + 0x1a)) >> 4) << 6
               ) |
       (ushort)((int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(iVar1 + 0x18)) >> 4);
  FUN_0208366c(0x4000,*(undefined2 *)(param_1 + 0x7c),*(undefined2 *)(param_1 + 0x50));
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_0206b404 @ 0206b404
 * ================================================================ */

void FUN_0206b404(int param_1)

{
  int iVar1;

  iVar1 = FUN_02083ca8(*(undefined2 *)(param_1 + 0x7c),*(undefined2 *)(param_1 + 0x50));
  if (iVar1 != DAT_0206b45c) {
    return;
  }
  FUN_0208366c(0x4000,*(undefined2 *)(param_1 + 0x7c),*(undefined2 *)(param_1 + 0x50));
  *(undefined1 *)(param_1 + 0x14) = 2;
  *(undefined1 *)(param_1 + 0x78) = 0;
  *(undefined1 *)(param_1 + 0x7a) = 0;
  *(undefined2 *)(param_1 + 0x7e) = 0x3c;
  FUN_02085f80(0x98);
  return;
}

/* ================================================================
 * FUN_0206b460 @ 0206b460
 * ================================================================ */

/* WARNING: Removing unreachable block (ram,0x0206b4b4) */

void FUN_0206b460(int param_1)

{
  if (*(short *)(param_1 + 0x7e) != 0) {
    *(short *)(param_1 + 0x7e) = *(short *)(param_1 + 0x7e) + -1;
    if ((*(ushort *)(param_1 + 0x7e) & 7) == 0) {
      FUN_0206b4dc(param_1);
    }
    FUN_02085980(param_1);
    return;
  }
  if (*(char *)(param_1 + 0x78) != '\0') {
    *(char *)(param_1 + 0x78) = *(char *)(param_1 + 0x78) + -1;
    *(undefined2 *)(param_1 + 0x7e) = 0x20;
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 1;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}
