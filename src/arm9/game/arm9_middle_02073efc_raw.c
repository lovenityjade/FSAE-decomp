#include "game/arm9_middle_02073efc_raw.h"

/*
 * Raw matching-first recovery for the 32 catalog entries beginning at
 * 0x02073efc. These bodies remain address-named promotion candidates and are
 * not linked byte-matching evidence.
 *
 * Calls whose implicit register arguments were omitted by Ghidra are written
 * with the live recovered values so this mechanically faithful draft remains
 * valid C.
 */

extern byte *DAT_02073fdc;
extern uint DAT_02073fe0;
extern uint *DAT_02074050;
extern uint *DAT_02074158;
extern uint DAT_02074194;
extern uint DAT_020742c4;
extern uint DAT_020742c8;
extern code *DAT_020742e0;
extern uint DAT_02074344;
extern uint DAT_020743e8;
extern uint DAT_020743ec;
extern byte *DAT_02074678;
extern uint DAT_0207467c;
extern uint DAT_02074788;
extern uint DAT_02074814;
extern uint DAT_02074818;
extern uint DAT_02074aa0;
extern uint DAT_02074b04;
extern byte *DAT_02074b08;
extern char *DAT_02074c5c;
extern uint DAT_02074c60;
extern uint DAT_02074c64;
extern uint DAT_02074c68;
extern uint DAT_02074cf0;
extern uint DAT_02074cf4;
extern uint DAT_02074e7c;
extern uint DAT_02074e80;
extern uint DAT_02074e84;
extern uint DAT_02074e88;
extern uint DAT_02074edc;
extern uint DAT_02075014;
extern uint DAT_02075018;
extern uint DAT_0207501c;
extern byte *DAT_02075020;
extern uint DAT_02075024;
extern uint DAT_02075028;
extern byte *DAT_020750f0;
extern uint DAT_020750f4;
extern byte *DAT_020751d4;
extern uint DAT_020751d8;
extern uint DAT_020751dc;
extern undefined1 *DAT_020751e0;
extern byte *DAT_02075214;
extern uint DAT_02075258;
extern uint DAT_02075430;
extern uint DAT_02075434;
extern uint DAT_02075438;
extern uint DAT_020754a8;
extern byte DAT_06400000;

/* ================================================================
 * FUN_02073efc @ 02073efc
 * ================================================================ */

void FUN_02073efc(int param_1)

{
  char cVar1;
  byte *pbVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  bool bVar7;

  cVar1 = *(char *)(param_1 + 0xb);
  bVar7 = cVar1 == '\0';
  if (bVar7) {
    cVar1 = *(char *)(param_1 + 0x16);
  }
  if (bVar7 && cVar1 == '\0') {
    *(undefined1 *)(param_1 + 0x16) = 1;
    FUN_02076fe0(0x215);
  }
  FUN_02085980(param_1);
  FUN_0204fd40(param_1);
  iVar3 = DAT_02073fe0;
  pbVar2 = DAT_02073fdc;
  if (*(short *)(*(int *)(param_1 + 0x54) + 0x36) + 0x18 != (int)*(short *)(param_1 + 0x36)) {
    return;
  }
  uVar6 = 0;
  if (*DAT_02073fdc != 0) {
    do {
      iVar4 = FUN_02076858(0x58,9,1);
      if (iVar4 != 0) {
        FUN_0205031c(param_1,iVar4);
        *(undefined4 *)(iVar4 + 0x58) = *(undefined4 *)(uVar6 * 0x94 + iVar3 + 0x28);
      }
      uVar5 = uVar6 + 1;
      uVar6 = uVar5 & 0xffff;
    } while ((uVar5 & 0xffff) < (uint)*pbVar2);
  }
  *(undefined1 *)(param_1 + 0x14) = 2;
  *(undefined1 *)(param_1 + 0x17) = 0x5a;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
  return;
}

/* ================================================================
 * FUN_02074020 @ 02074020
 * ================================================================ */

void FUN_02074020(int param_1)

{
  char cVar1;

  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  *DAT_02074050 = *DAT_02074050 | 8;
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02074054 @ 02074054
 * ================================================================ */

void FUN_02074054(int param_1,int param_2,undefined4 param_3,int param_4)

{
  undefined1 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;

  iVar4 = *(int *)(param_1 + 0x58);
  iVar2 = (int)*(short *)(param_1 + 0x32);
  iVar3 = (int)*(short *)(iVar4 + 0x32);
  if (iVar3 == iVar2) {
    param_2 = (int)*(short *)(param_1 + 0x36);
    param_4 = *(short *)(iVar4 + 0x36) + -0x20;
  }
  if (iVar3 != iVar2 || param_4 != param_2) {
    uVar1 = FUN_02086080(iVar2,(int)*(short *)(param_1 + 0x36),iVar3,
                         (*(short *)(iVar4 + 0x36) + -0x20) * 0x10000 >> 0x10);
    *(undefined1 *)(param_1 + 0x2b) = uVar1;
    FUN_0204fd40(param_1);
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 2;
  return;
}

/* ================================================================
 * FUN_020740e0 @ 020740e0
 * ================================================================ */

void FUN_020740e0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;

  if (*(char *)(param_1 + 0x17) != '\0') {
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
    FUN_0204fd40();
    return;
  }
  if ((*DAT_02074158 & 8) == 0) {
    return;
  }
  iVar1 = FUN_02076858(0x59,0,0,param_4,param_4);
  if (iVar1 == 0) {
    return;
  }
  FUN_0205031c(param_1,iVar1);
  *(int *)(iVar1 + 0x34) = *(int *)(iVar1 + 0x34) + 1;
  *(undefined1 *)(param_1 + 0x14) = 4;
  *(undefined1 *)(param_1 + 0x17) = 0x3c;
  return;
}

/* ================================================================
 * FUN_0207417c @ 0207417c
 * ================================================================ */

void FUN_0207417c(int param_1)

{
  (**(code **)(DAT_02074194 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_02074198 @ 02074198
 * ================================================================ */

void FUN_02074198(int param_1)

{
  byte bVar1;
  int iVar2;

  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x40000001;
  *(undefined2 *)(param_1 + 0x60) = *(undefined2 *)(param_1 + 0x32);
  *(undefined2 *)(param_1 + 0x62) = *(undefined2 *)(param_1 + 0x36);
  bVar1 = FUN_0208455c();
  *(byte *)(param_1 + 0x2b) = bVar1 & 0x1f;
  *(undefined4 *)(param_1 + 0xc) = 0x20;
  FUN_020742cc(param_1);
  iVar2 = FUN_02081b64();
  if (iVar2 == 0) {
    return;
  }
  *(uint *)(param_1 + 0x1c) =
       *(uint *)(param_1 + 0x1c) & 0xfff0ffff |
       (((*(uint *)(param_1 + 0x1c) & 0xfffff) >> 0x10) + 1 & 0xf) << 0x10;
  return;
}

/* ================================================================
 * FUN_02074218 @ 02074218
 * ================================================================ */

void FUN_02074218(int param_1)

{
  short sVar1;
  undefined1 uVar2;
  uint uVar3;

  FUN_0204fd40();
  FUN_02085980(param_1);
  if ((0xc < ((int)*(short *)(param_1 + 0x60) - (int)*(short *)(param_1 + 0x32) & 0xffffU)) ||
     (0xc < (ushort)(*(short *)(param_1 + 0x62) - *(short *)(param_1 + 0x36)))) {
    uVar2 = FUN_02086080((int)*(short *)(param_1 + 0x32),(int)*(short *)(param_1 + 0x36),
                         (int)*(short *)(param_1 + 0x60),(int)*(short *)(param_1 + 0x62));
    *(undefined1 *)(param_1 + 0x2b) = uVar2;
    uVar3 = FUN_0208455c();
    *(byte *)(param_1 + 0x2b) =
         *(char *)(param_1 + 0x2b) + *(char *)(DAT_020742c4 + (uVar3 & 3)) & 0x1f;
  }
  sVar1 = *(short *)(DAT_020742c8 + (*(byte *)(param_1 + 0x17) + 0x40) * 2);
  *(byte *)(param_1 + 0x17) = *(byte *)(param_1 + 0x17) + 1;
  *(short *)(param_1 + 0x3a) = (short)((uint)((int)sVar1 << 2) >> 8) + -8;
  return;
}

/* ================================================================
 * FUN_020742cc @ 020742cc
 * ================================================================ */

void FUN_020742cc(int param_1)

{
  code *UNRECOVERED_JUMPTABLE;

  UNRECOVERED_JUMPTABLE = DAT_020742e0;
  *(undefined1 *)(param_1 + 0x14) = 1;
                    /* WARNING: Could not recover jumptable at 0x020742dc. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)(param_1,0);
  return;
}

/* ================================================================
 * FUN_020742e4 @ 020742e4
 * ================================================================ */

void FUN_020742e4(int param_1)

{
  int iVar1;

  if (*(char *)(param_1 + 0x17) != '\0') {
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
    return;
  }
  iVar1 = FUN_02076858(0x58,8,0);
  if (iVar1 != 0) {
    FUN_0205031c(param_1);
    *(undefined1 *)(param_1 + 0x17) = 0x30;
    return;
  }
  return;
}

/* ================================================================
 * FUN_0207432c @ 0207432c
 * ================================================================ */

void FUN_0207432c(int param_1)

{
  (**(code **)(DAT_02074344 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_02074348 @ 02074348
 * ================================================================ */

void FUN_02074348(int param_1)

{
  int iVar1;
  uint uVar2;

  *(undefined1 *)(param_1 + 0x14) = 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 3;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 1;
  uVar2 = FUN_02049020();
  if (uVar2 == 3) {
    uVar2 = *(byte *)(DAT_020743e8 + 1) + 3 & 0xff;
  }
  uVar2 = FUN_02027ec4(*(ushort *)(DAT_020743ec + uVar2 * 2) & 0xff);
  iVar1 = DAT_020743e8;
  *(uint *)(param_1 + 0x1c) =
       *(uint *)(param_1 + 0x1c) & 0xff00ffff | (uVar2 & 0xf) << 0x14 | (uVar2 & 0xf) << 0x10;
  FUN_020859e0(param_1,*(undefined1 *)(iVar1 + 1));
  return;
}

/* ================================================================
 * FUN_02074508 @ 02074508
 * ================================================================ */

void FUN_02074508(int param_1)

{
  byte *pbVar1;
  int iVar2;
  char cVar3;
  ushort uVar4;
  short sVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;

  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(undefined1 *)(param_1 + 0x17) = 1;
    *(undefined1 *)(param_1 + 0x18) = 0x1e;
    if (*(char *)(param_1 + 10) != '\0') {
      return;
    }
    *(undefined1 *)(param_1 + 0x17) = 2;
    uVar9 = FUN_0208455c();
    if ((uVar9 & 1) != 0) {
      *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + '\x01';
    }
    return;
  }
  cVar3 = *(char *)(param_1 + 0x18);
  *(char *)(param_1 + 0x18) = cVar3 + -1;
  iVar2 = DAT_0207467c;
  pbVar1 = DAT_02074678;
  if (cVar3 != '\0') {
    return;
  }
  uVar9 = 0;
  if (*DAT_02074678 != 0) {
    do {
      iVar6 = FUN_0204bdb8(0xc);
      if (iVar6 != 0) {
        iVar8 = uVar9 * 0x94 + iVar2;
        *(undefined2 *)(iVar6 + 0x32) = *(undefined2 *)(*(int *)(iVar8 + 0x28) + 0x32);
        *(undefined2 *)(iVar6 + 0x36) = *(undefined2 *)(*(int *)(iVar8 + 0x28) + 0x36);
        uVar7 = FUN_0208455c();
        if ((uVar7 & 1) == 0) {
          uVar4 = FUN_0208455c();
          sVar5 = *(short *)(iVar6 + 0x32) - (uVar4 & 0x1f);
        }
        else {
          uVar4 = FUN_0208455c();
          sVar5 = *(short *)(iVar6 + 0x32) + (uVar4 & 0x1f);
        }
        *(short *)(iVar6 + 0x32) = sVar5;
        uVar7 = FUN_0208455c();
        if ((uVar7 & 1) == 0) {
          uVar4 = FUN_0208455c();
          sVar5 = *(short *)(iVar6 + 0x32) - (uVar4 & 0x1f);
        }
        else {
          uVar4 = FUN_0208455c();
          sVar5 = *(short *)(iVar6 + 0x32) + (uVar4 & 0x1f);
        }
        *(short *)(iVar6 + 0x32) = sVar5;
      }
      uVar9 = uVar9 + 1 & 0xff;
    } while (uVar9 < *pbVar1);
  }
  *(undefined1 *)(param_1 + 0x18) = 0x1e;
  cVar3 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar3;
  if (cVar3 != '\0') {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_020746b8 @ 020746b8
 * ================================================================ */

void FUN_020746b8(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  uint uVar1;

  uVar1 = *(uint *)(param_1 + 0x1c) & 0x3fff3f3c | 0x42;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined1 *)(param_1 + 0x20) = 0x60;
  *(uint *)(param_1 + 0x1c) = uVar1;
  *(undefined1 *)(param_1 + 0x1b) = 0x9a;
  FUN_020859e0(param_1,0,0x9a,uVar1,param_4);
  return;
}

/* ================================================================
 * FUN_02074704 @ 02074704
 * ================================================================ */

void FUN_02074704(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;

  FUN_02085980(param_1);
  iVar1 = DAT_02074788;
  if (*(byte *)(param_1 + 0x29) == 2) {
    *(undefined1 *)(param_1 + 0x29) = 0;
    *(undefined1 *)(DAT_02074788 + 0xd) = 1;
    FUN_02076fe0(0x204);
    FUN_02076fe0(0x202);
    return;
  }
  if ((*(byte *)(param_1 + 0x29) & 0x80) == 0) {
    return;
  }
  *(undefined1 *)(DAT_02074788 + 0xd) = 2;
  iVar1 = FUN_02076858(0x62,1,0,iVar1,param_4);
  *(undefined2 *)(iVar1 + 0x32) = 0x80;
  *(undefined2 *)(iVar1 + 0x36) = 0x38;
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_020747a8 @ 020747a8
 * ================================================================ */

void FUN_020747a8(int param_1)

{
  undefined4 uVar1;

  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined1 *)(param_1 + 0x20) = 0x60;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3f3c | 0x41;
  *(undefined1 *)(param_1 + 0x1b) = 0x9b;
  uVar1 = FUN_0208da4c();
  uVar1 = FUN_0208ddcc(uVar1,DAT_02074814);
  CallbackForJpeg(DAT_02074818,uVar1);
  FUN_020859e0(param_1,1);
  return;
}

/* ================================================================
 * FUN_02074934 @ 02074934
 * ================================================================ */

void FUN_02074934(int param_1)

{
  byte bVar1;
  short sVar2;
  char cVar3;
  char cVar4;

  FUN_02085980(param_1);
  bVar1 = *(byte *)(param_1 + 0x60);
  cVar3 = *(char *)(param_1 + 0x15);
  sVar2 = *(short *)(DAT_02074aa0 + (uint)bVar1 * 2);
  *(short *)(param_1 + 0x32) =
       (short)((uint)((int)(short)(ushort)*(byte *)(param_1 + 0x61) *
                     (int)*(short *)(DAT_02074aa0 + (bVar1 + 0x40) * 2)) >> 8) + 0x80;
  *(short *)(param_1 + 0x36) =
       (short)((uint)((int)(short)(ushort)*(byte *)(param_1 + 0x62) * (int)sVar2) >> 8) + 0xac;
  switch(cVar3) {
  case '\0':
    *(byte *)(param_1 + 0x60) = bVar1 - 2;
    cVar4 = *(char *)(param_1 + 0x17) + -1;
    *(char *)(param_1 + 0x17) = cVar4;
    if (cVar4 == '\0') {
      *(undefined1 *)(param_1 + 0x17) = 0x40;
      *(char *)(param_1 + 0x15) = cVar3 + '\x01';
    }
    return;
  case '\x01':
    *(byte *)(param_1 + 0x60) = bVar1 - 4;
    cVar4 = *(char *)(param_1 + 0x17) + -1;
    *(char *)(param_1 + 0x17) = cVar4;
    if (cVar4 == '\0') {
      *(undefined1 *)(param_1 + 0x17) = 0x20;
      *(char *)(param_1 + 0x15) = cVar3 + '\x01';
    }
    return;
  case '\x02':
    break;
  case '\x03':
    break;
  case '\x04':
    break;
  case '\x05':
    *(byte *)(param_1 + 0x60) = bVar1 - 8;
    cVar3 = *(char *)(param_1 + 0x17) + -1;
    *(char *)(param_1 + 0x17) = cVar3;
    if (cVar3 != '\0') {
      return;
    }
    *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
    *(undefined1 *)(param_1 + 0x15) = 0;
    *(undefined1 *)(param_1 + 0x2b) = 0x17;
    *(undefined4 *)(param_1 + 0xc) = 0x800;
    FUN_02076fe0(0x1fb);
    FUN_020725d0(0,8,0);
    return;
  default:
    return;
  }
  *(byte *)(param_1 + 0x60) = bVar1 - 8;
  cVar4 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar4;
  if (cVar4 != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x17) = 0x20;
  *(char *)(param_1 + 0x15) = cVar3 + '\x01';
  *(byte *)(param_1 + 0x61) = *(byte *)(param_1 + 0x61) + 1;
  *(byte *)(param_1 + 0x62) = *(byte *)(param_1 + 0x62) + 1;
  return;
}

/* ================================================================
 * FUN_02074aa4 @ 02074aa4
 * ================================================================ */

void FUN_02074aa4(int param_1)

{
  int iVar1;

  FUN_02085980(param_1);
  FUN_0204fd40(param_1);
  iVar1 = DAT_02074b04;
  if (-0x41 < *(short *)(param_1 + 0x32)) {
    return;
  }
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined1 *)(*(int *)(iVar1 + 0x28) + 0x2a) = 6;
  FUN_020725d0(0,0x10,0);
  *DAT_02074b08 = *DAT_02074b08 + '\x01';
  return;
}

/* ================================================================
 * FUN_02074b0c @ 02074b0c
 * ================================================================ */

void FUN_02074b0c(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;
  char *pcVar2;
  int iVar3;
  int iVar4;
  undefined2 uStack_18;
  short sStack_16;
  undefined4 uStack_14;

  uStack_14 = param_4;
  FUN_02085980(param_1);
  pcVar2 = DAT_02074c5c;
  if ((byte)DAT_02074c5c[0xe] < 5) {
    return;
  }
  cVar1 = *(char *)(param_1 + 0x15);
  if (cVar1 == '\0') {
    uStack_18 = *(undefined2 *)(*(int *)(DAT_02074c60 + 0x28) + 0x32);
    sStack_16 = *(undefined2 *)(*(int *)(DAT_02074c60 + 0x28) + 0x36);
    FUN_0205069c(param_1,&uStack_18,0x1e);
    FUN_02076fe0(DAT_02074c64);
    *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
    return;
  }
  if (cVar1 == '\x01') {
    iVar3 = FUN_020506d8(param_1);
    iVar4 = DAT_02074c60;
    if (iVar3 != 0) {
      uStack_18 = 300;
      sStack_16 = *(short *)(*(int *)(DAT_02074c60 + 0x28) + 0x36) + -0x20;
      FUN_0205069c(param_1,&uStack_18,0x1e);
      *(undefined1 *)(iVar4 + 0xc) = 9;
      FUN_02049f0c(7);
      iVar4 = DAT_02074c68;
      *(byte *)(DAT_02074c68 + 3) = *(byte *)(DAT_02074c68 + 3) & 0xe0 | 1;
      *(byte *)(iVar4 + 4) = *(byte *)(iVar4 + 4) & 0xe0 | 0x10;
      FUN_02076fe0(0x202);
      *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
      return;
    }
    return;
  }
  if (cVar1 != '\x02') {
    return;
  }
  iVar4 = FUN_020506d8(param_1);
  if (iVar4 != 0) {
    *pcVar2 = *pcVar2 + '\x01';
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02074cac @ 02074cac
 * ================================================================ */

void FUN_02074cac(int param_1)

{
  uint uVar1;

  uVar1 = (uint)*(byte *)(param_1 + 0x52);
  if (*(uint *)(param_1 + 0x60) != (*(uint *)(param_1 + 0x60) & *(uint *)(DAT_02074cf0 + uVar1 * 4))
     ) {
    return;
  }
  *(uint *)(DAT_02074cf0 + uVar1 * 4) =
       *(uint *)(DAT_02074cf0 + uVar1 * 4) &
       ~*(uint *)(DAT_02074cf4 + (uint)*(byte *)(param_1 + 0xb) * 4);
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02074d14 @ 02074d14
 * ================================================================ */

void FUN_02074d14(int param_1)

{
  undefined2 uVar1;
  undefined4 uVar2;

  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  uVar2 = FUN_020460e8(*(undefined1 *)(param_1 + 10));
  *(undefined4 *)(param_1 + 0x60) = uVar2;
  uVar2 = FUN_0204d634(*(undefined1 *)(param_1 + 0x52),*(undefined1 *)(param_1 + 0x17));
  *(ushort *)(param_1 + 0x6a) = *(short *)(param_1 + 0x32) >> 8 & 1;
  *(undefined4 *)(param_1 + 100) = uVar2;
  uVar1 = 0x4000;
  if (*(short *)(param_1 + 0x6a) != 0) {
    uVar1 = 0x8000;
  }
  *(char *)(param_1 + 0x18) = (char)*(short *)(param_1 + 0x32);
  *(char *)(param_1 + 0x16) = (char)*(undefined2 *)(param_1 + 0x36);
  *(char *)(param_1 + 0x68) = (char)((int)(uint)*(byte *)(param_1 + 0xb) >> 5);
  *(byte *)(param_1 + 0xb) = *(byte *)(param_1 + 0xb) & 0x1f;
  *(undefined2 *)(param_1 + 0x6a) = uVar1;
  return;
}

/* ================================================================
 * FUN_02074dc4 @ 02074dc4
 * ================================================================ */

void FUN_02074dc4(int param_1)

{
  int iVar1;
  int iVar2;
  char cVar3;
  byte *pbVar4;

  pbVar4 = *(byte **)(param_1 + 100);
  if (*pbVar4 == 0xff) {
    *(uint *)(DAT_02074e7c + (uint)*(byte *)(param_1 + 0x52) * 4) =
         *(uint *)(DAT_02074e7c + (uint)*(byte *)(param_1 + 0x52) * 4) |
         *(uint *)(DAT_02074e80 + (uint)*(byte *)(param_1 + 0xb) * 4);
    FUN_0204be88(param_1);
    return;
  }
  cVar3 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar3;
  iVar2 = DAT_02074e84;
  if (cVar3 != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x17) = *(undefined1 *)(param_1 + 0x18);
  iVar1 = (uint)*(byte *)(param_1 + 0x52) * 4;
  FUN_02074e8c(param_1,((ushort)*pbVar4 << 4 | 8) + *(short *)(iVar2 + iVar1),
               ((ushort)pbVar4[1] << 4 | 8) + *(short *)(DAT_02074e88 + iVar1));
  *(int *)(param_1 + 100) = *(int *)(param_1 + 100) + 2;
  return;
}

/* ================================================================
 * FUN_02074e8c @ 02074e8c
 * ================================================================ */

void FUN_02074e8c(int param_1,undefined2 param_2,undefined2 param_3)

{
  int iVar1;

  iVar1 = FUN_02076858(0,*(undefined1 *)(DAT_02074edc + (uint)*(byte *)(param_1 + 0x68)),1);
  if (iVar1 == 0) {
    return;
  }
  *(undefined2 *)(iVar1 + 0x32) = param_2;
  *(undefined2 *)(iVar1 + 0x36) = param_3;
  *(undefined1 *)(iVar1 + 0x17) = *(undefined1 *)(param_1 + 0x16);
  *(undefined2 *)(iVar1 + 0x50) = *(undefined2 *)(param_1 + 0x6a);
  FUN_020768d8(iVar1);
  return;
}

/* ================================================================
 * FUN_02074ee0 @ 02074ee0
 * ================================================================ */

void FUN_02074ee0(int param_1)

{
  char cVar1;
  byte *pbVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  bool bVar8;

  if (4 < *(byte *)(*(int *)(param_1 + 0x54) + 10)) {
    *(undefined1 *)(param_1 + 0x18) = 1;
  }
  cVar1 = *(char *)(*(int *)(param_1 + 0x54) + 10);
  if ((cVar1 != '\0') && (*(char *)(param_1 + 0x18) == '\0')) {
    if (*(char *)(param_1 + 10) != cVar1) {
      *(undefined4 *)(param_1 + 100) = 0;
    }
    if (*(int *)(param_1 + 100) == 0) {
      FUN_0208da4c();
      iVar3 = FUN_0208de54();
      iVar4 = DAT_02075014;
      if (iVar3 == 0) {
        iVar4 = DAT_02075018;
      }
      FUN_02049f38(*(undefined2 *)(iVar4 + (*(byte *)(*(int *)(param_1 + 0x54) + 10) - 1) * 2));
      *(undefined4 *)(param_1 + 100) = DAT_0207501c;
    }
    else {
      *(int *)(param_1 + 100) = *(int *)(param_1 + 100) + -1;
    }
    *(undefined1 *)(param_1 + 10) = *(undefined1 *)(*(int *)(param_1 + 0x54) + 10);
  }
  iVar3 = DAT_02075028;
  iVar4 = DAT_02075024;
  pbVar2 = DAT_02075020;
  uVar6 = 0;
  if (*DAT_02075020 == 0) {
    return;
  }
  do {
    iVar7 = uVar6 * 0x94 + iVar4;
    iVar5 = *(int *)(iVar7 + 0x28);
    if (iVar3 < *(short *)(iVar5 + 0x32)) {
      cVar1 = *(char *)(iVar5 + 0x14);
      bVar8 = cVar1 == '\x01';
      if (bVar8) {
        cVar1 = *(char *)(iVar5 + 0x3d);
      }
      if (((bVar8 && cVar1 == '\0') && ((*(ushort *)(iVar7 + 0x36) & 0x880) == 0)) &&
         (iVar5 = FUN_02076858(0x55,0,0), iVar5 != 0)) {
        *(undefined4 *)(iVar5 + 0x58) = *(undefined4 *)(iVar7 + 0x28);
        *(undefined2 *)(iVar7 + 0x36) = 0x800;
        *(undefined1 *)(iVar7 + 0xc) = 0xd;
      }
    }
    uVar6 = uVar6 + 1 & 0xff;
  } while (uVar6 < *pbVar2);
  return;
}

/* ================================================================
 * FUN_02075064 @ 02075064
 * ================================================================ */

void FUN_02075064(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  byte *pbVar1;
  ushort uVar2;
  int iVar3;
  short *psVar4;

  pbVar1 = DAT_020750f0;
  uVar2 = 0;
  psVar4 = *(short **)(DAT_020750f4 + (*DAT_020750f0 - 1) * 4);
  if (*DAT_020750f0 != 0) {
    do {
      iVar3 = FUN_02076858(0x66,1,uVar2 & 0xff);
      if (iVar3 != 0) {
        *(int *)(iVar3 + 0x54) = param_1;
        FUN_02050390(param_1,iVar3,(int)*psVar4 << 0x10,(int)psVar4[1] << 0x10,param_4);
        psVar4 = psVar4 + 2;
      }
      uVar2 = uVar2 + 1;
    } while (uVar2 < *pbVar1);
  }
  *(undefined1 *)(param_1 + 0x14) = 1;
  return;
}

/* ================================================================
 * FUN_020750f8 @ 020750f8
 * ================================================================ */

void FUN_020750f8(int param_1)

{
  byte bVar1;
  byte *pbVar2;
  int iVar3;
  undefined4 uVar4;
  undefined1 *puVar5;
  uint uVar6;
  int iVar7;

  pbVar2 = DAT_020751d4;
  bVar1 = *DAT_020751d4;
  uVar6 = FUN_02084600(*(undefined4 *)(param_1 + 0x60));
  iVar3 = DAT_020751d8;
  if (bVar1 != uVar6) {
    return;
  }
  uVar6 = 0;
  if (bVar1 != 0) {
    do {
      iVar7 = FUN_0207553c(uVar6 * 0x94 + iVar3);
      if (iVar7 != 0) {
        return;
      }
      uVar6 = uVar6 + 1 & 0xff;
    } while (uVar6 < *pbVar2);
  }
  puVar5 = DAT_020751e0;
  uVar4 = DAT_020751dc;
  *(undefined1 *)(param_1 + 0x14) = 2;
  *(undefined4 *)(param_1 + 0x60) = uVar4;
  *puVar5 = 2;
  FUN_020547ec();
  FUN_02053958();
  iVar3 = DAT_020751d8;
  pbVar2 = DAT_020751d4;
  uVar6 = 0;
  if (*DAT_020751d4 != 0) {
    do {
      iVar7 = uVar6 * 0x94 + iVar3;
      *(undefined1 *)(iVar7 + 0xc) = 0xf;
      *(undefined2 *)(iVar7 + 8) = 0xc3;
      *(undefined1 *)(iVar7 + 0x38) = 0xb4;
      *(undefined1 *)(iVar7 + 0x39) = 1;
      *(undefined1 *)(*(int *)(iVar7 + 0x28) + 0x2a) = 4;
      uVar6 = uVar6 + 1 & 0xff;
    } while (uVar6 < *pbVar2);
    return;
  }
  return;
}

/* ================================================================
 * FUN_020751e4 @ 020751e4
 * ================================================================ */

void FUN_020751e4(int param_1)

{
  int iVar1;

  FUN_020547ec();
  iVar1 = FUN_02084600(*(undefined4 *)(param_1 + 0x60));
  if (0x10 - (uint)*DAT_02075214 == iVar1) {
    *(undefined1 *)(param_1 + 0x14) = 3;
  }
  return;
}

/* ================================================================
 * FUN_02075218 @ 02075218
 * ================================================================ */

void FUN_02075218(undefined4 param_1)

{
  int iVar1;

  FUN_020547ec();
  iVar1 = FUN_0204bd80(0x2f);
  if (iVar1 == 0) {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02075240 @ 02075240
 * ================================================================ */

void FUN_02075240(int param_1)

{
  (**(code **)(DAT_02075258 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0207525c @ 0207525c
 * ================================================================ */

void FUN_0207525c(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  uint uVar1;

  uVar1 = *(uint *)(param_1 + 0x1c) & 0xffff3fff | 0xc000;
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(undefined1 *)(param_1 + 0x17) = 0x1e;
  *(uint *)(param_1 + 0x1c) = uVar1;
  FUN_020859e0(param_1,0,uVar1,0x1e,param_4);
  return;
}

/* ================================================================
 * FUN_0207528c @ 0207528c
 * ================================================================ */

void FUN_0207528c(int param_1)

{
  int iVar1;
  undefined4 uVar2;

  if (*(char *)(param_1 + 0x17) != '\0') {
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
    return;
  }
  iVar1 = FUN_020811a8();
  if (iVar1 == 0) {
    uVar2 = FUN_0208da4c();
    iVar1 = FUN_0208ddcc(uVar2,0x9a);
    FUN_020845b0(&DAT_06400000 + (*(byte *)(param_1 + 0x53) + 1) * 0x200,iVar1 + 0x200,0x100);
    *(undefined1 *)(param_1 + 0x14) = 2;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
    return;
  }
  return;
}

/* ================================================================
 * FUN_020752f8 @ 020752f8
 * ================================================================ */

void FUN_020752f8(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  bool bVar5;

  FUN_02085980(param_1);
  if (*(int *)(*(int *)(param_1 + 0x54) + 0x60) == DAT_02075430) {
    *(undefined1 *)(param_1 + 0x14) = 3;
    *(undefined1 *)(param_1 + 0x17) = 0x3c;
    FUN_02075528(param_1,*(undefined4 *)(param_1 + 0x58),0x3c,3,param_4);
    return;
  }
  uVar1 = FUN_02084110(param_1);
  iVar3 = DAT_02075438;
  if (uVar1 == 0) {
    *(uint *)(*(int *)(param_1 + 0x54) + 0x60) =
         *(uint *)(*(int *)(param_1 + 0x54) + 0x60) &
         ~*(uint *)(DAT_02075438 + (uint)*(byte *)(param_1 + 0xb) * 4);
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfff0ffff;
  }
  else {
    bVar5 = (uVar1 & 1) != 0;
    uVar2 = uVar1;
    if (bVar5) {
      uVar2 = DAT_02075434;
    }
    if (bVar5) {
      uVar2 = *(uint *)(uVar2 + 0x28);
      *(uint *)(param_1 + 0x58) = uVar2;
    }
    bVar5 = (uVar1 & 2) != 0;
    if (bVar5) {
      uVar2 = DAT_02075434;
    }
    if (bVar5) {
      uVar2 = *(uint *)(uVar2 + 0xbc);
    }
    uVar4 = *(uint *)(iVar3 + (uint)*(byte *)(param_1 + 0xb) * 4);
    if (bVar5) {
      *(uint *)(param_1 + 0x58) = uVar2;
    }
    if ((uVar1 & 4) != 0) {
      *(undefined4 *)(param_1 + 0x58) = *(undefined4 *)(DAT_02075434 + 0x150);
    }
    if ((uVar1 & 8) != 0) {
      *(undefined4 *)(param_1 + 0x58) = *(undefined4 *)(DAT_02075434 + 0x1e4);
    }
    uVar2 = *(uint *)(*(int *)(param_1 + 0x54) + 0x60);
    if ((uVar2 & uVar4) == 0) {
      *(uint *)(*(int *)(param_1 + 0x54) + 0x60) = uVar2 | uVar4;
      *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfff0ffff | 0x10000;
      FUN_02085f80(0x18);
    }
  }
  FUN_0208da4c();
  iVar3 = FUN_0208de54();
  if (iVar3 != 0) {
    return;
  }
  uVar2 = FUN_02084600(uVar1);
  if (1 < uVar2) {
    uVar1 = 1;
  }
  iVar3 = FUN_02093b5c();
  *(byte *)(iVar3 + 8) = *(byte *)(iVar3 + 8) | (byte)uVar1;
  *(undefined1 *)(iVar3 + 9) = 0xff;
  return;
}

/* ================================================================
 * FUN_0207543c @ 0207543c
 * ================================================================ */

void FUN_0207543c(int param_1)

{
  int iVar1;
  char cVar2;
  int iVar3;

  FUN_02085980(param_1);
  cVar2 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar2;
  if (cVar2 != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 4;
  *(undefined2 *)(param_1 + 0x7c) = 0x168;
  iVar3 = FUN_02076858(0x6b,0,0);
  iVar1 = DAT_020754a8;
  if (iVar3 == 0) {
    return;
  }
  *(undefined1 *)(iVar3 + 10) = *(undefined1 *)(param_1 + 0xb);
  FUN_0205031c(*(undefined4 *)(iVar1 + (short)(ushort)*(byte *)(param_1 + 0xb) * 0x94));
  return;
}

