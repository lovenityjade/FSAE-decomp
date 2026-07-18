#include "game/arm9_middle_0206b4dc_raw.h"

/*
 * Raw matching-first recovery for the 32 catalog entries beginning at
 * 0x0206b4dc. These bodies remain address-named promotion candidates and are
 * not linked byte-matching evidence.
 *
 * Calls whose implicit r0 argument was omitted by Ghidra are written with the
 * owning param_1 so this mechanically faithful draft remains valid C.
 */

extern uint DAT_0206b5ac;
extern uint DAT_0206b5b0;
extern uint DAT_0206b5b4;
extern uint DAT_0206b644;
extern uint *DAT_0206b71c;
extern uint DAT_0206b720;
extern uint DAT_0206b724;
extern uint DAT_0206b728;
extern uint DAT_0206b72c;
extern uint DAT_0206b7ec;
extern int *DAT_0206b8f0;
extern uint DAT_0206b8f4;
extern uint DAT_0206b8f8;
extern uint DAT_0206b8fc;
extern int *DAT_0206ba54;
extern uint DAT_0206ba58;
extern uint DAT_0206ba5c;
extern uint DAT_0206ba60;
extern uint *DAT_0206bbec;
extern uint DAT_0206bbf0;
extern uint DAT_0206bc5c;
extern uint DAT_0206bd64;
extern uint DAT_0206bd68;
extern uint DAT_0206bd6c;
extern uint DAT_0206beb4;
extern uint DAT_0206beb8;
extern uint DAT_0206bebc;
extern uint DAT_0206bec0;
extern uint DAT_0206bf24;
extern uint DAT_0206c024;
extern uint DAT_0206c028;
extern uint DAT_0206c31c;
extern uint DAT_0206c320;
extern uint DAT_0206c324;
extern uint DAT_0206c3b0;
extern uint DAT_0206c4b8;
extern undefined1 *DAT_0206c4bc;
extern code *DAT_0206c4dc;
extern uint DAT_0206c550;
extern uint DAT_0206c554;
extern uint DAT_0206c6e4;
extern uint DAT_0206c6e8;
extern uint DAT_0206c6ec;
extern uint DAT_0206c6f0;
extern char *DAT_0206c6f4;
extern int *DAT_0206c6f8;
extern byte *DAT_0206c6fc;
extern uint DAT_0206c700;
extern undefined2 *DAT_0206c7c8;
extern undefined2 *DAT_0206c84c;

/* ================================================================
 * FUN_0206b4dc @ 0206b4dc
 * ================================================================ */

void FUN_0206b4dc(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  uint uVar2;
  int iVar3;

  if (*(char *)(param_1 + 0x78) == '\0') {
    uVar2 = FUN_0208455c();
    uVar2 = uVar2 & 0x1f;
  }
  else {
    uVar2 = FUN_0208455c();
    uVar2 = uVar2 & 0xf;
  }
  if (*(char *)(DAT_0206b5ac + uVar2) == -1) {
    if (2 < *(byte *)(param_1 + 0x7a)) {
      return;
    }
    *(byte *)(param_1 + 0x7a) = *(byte *)(param_1 + 0x7a) + 1;
    *(byte *)(param_1 + 0x78) = *(char *)(param_1 + 0x78) << 1 | 1;
    return;
  }
  iVar3 = FUN_02076858(0,*(char *)(DAT_0206b5ac + uVar2),1);
  if (iVar3 == 0) {
    return;
  }
  iVar1 = (uint)*(byte *)(param_1 + 10) * 2;
  FUN_02050390(param_1,iVar3,(int)*(char *)(DAT_0206b5b0 + iVar1) << 0x10,
               (int)*(char *)(DAT_0206b5b0 + iVar1 + 1) << 0x10,param_4);
  uVar2 = FUN_0208455c();
  *(byte *)(iVar3 + 0x2b) =
       *(char *)(DAT_0206b5b4 + (uVar2 & 7)) + (*(byte *)(param_1 + 10) ^ 2) * '\b' & 0x1f | 0x80;
  return;
}

/* ================================================================
 * FUN_0206b5d4 @ 0206b5d4
 * ================================================================ */

void FUN_0206b5d4(int param_1)

{
  undefined4 uVar1;

  uVar1 = DAT_0206b644;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined1 *)(param_1 + 0x2e) = 1;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xffffffcc | 0x11;
  *(undefined4 *)(param_1 + 0x48) = uVar1;
  *(undefined1 *)(param_1 + 0x40) = 0x38;
  *(undefined1 *)(param_1 + 0x3f) = 0x1f;
  *(undefined1 *)(param_1 + 0x3c) = 0xd9;
  *(undefined2 *)(param_1 + 0x5c) = 0x55;
  *(short *)(param_1 + 0x36) = *(short *)(param_1 + 0x36) + -4;
  *(undefined2 *)(param_1 + 0x3a) = 0xfffc;
  return;
}

/* ================================================================
 * FUN_0206b648 @ 0206b648
 * ================================================================ */

void FUN_0206b648(int param_1)

{
  char cVar1;
  uint uVar2;
  int iVar3;

  if ((*DAT_0206b71c & 7) == 0) {
    uVar2 = *(byte *)(param_1 + 0x17) + 1;
    cVar1 = *(char *)(DAT_0206b720 + (uVar2 & 7));
    *(char *)(param_1 + 0x17) = (char)uVar2;
    *(short *)(param_1 + 0x3a) = cVar1 + -8;
  }
  if ((*(byte *)(param_1 + 0x41) & 0x80) == 0) {
    return;
  }
  iVar3 = *(int *)(param_1 + 0x4c);
  *(undefined1 *)(DAT_0206b724 + (short)(ushort)*(byte *)(iVar3 + 10) * 0x94) = 8;
  *(undefined1 *)(DAT_0206b728 + (short)(ushort)*(byte *)(iVar3 + 10) * 0x94) = 1;
  iVar3 = (short)(ushort)*(byte *)(iVar3 + 10) * 0x94;
  *(byte *)(DAT_0206b72c + iVar3) = *(byte *)(DAT_0206b72c + iVar3) & 0x7f;
  *(undefined1 *)(param_1 + 0x17) = 0x3c;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  FUN_0206b768(param_1);
  *(undefined1 *)(param_1 + 0x2e) = 0;
  *(byte *)(param_1 + 0x3c) = *(byte *)(param_1 + 0x3c) & 0x7f;
  FUN_0202604c(*(undefined1 *)(*(int *)(param_1 + 0x4c) + 10),1);
  return;
}

/* ================================================================
 * FUN_0206b730 @ 0206b730
 * ================================================================ */

void FUN_0206b730(int param_1)

{
  char cVar1;

  FUN_0206b768(param_1);
  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  FUN_02025d9c(*(undefined4 *)(param_1 + 0x4c),0xa0);
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_0206b768 @ 0206b768
 * ================================================================ */

void FUN_0206b768(int param_1)

{
  uint uVar1;
  uint uVar2;

  FUN_0205031c(*(undefined4 *)(param_1 + 0x4c),param_1);
  uVar2 = *(uint *)(param_1 + 0x1c);
  uVar1 = *(uint *)(*(int *)(param_1 + 0x4c) + 0x1c) & 0xc0000000;
  *(uint *)(param_1 + 0x1c) = uVar2 & 0x3fffffff | uVar1;
  *(uint *)(param_1 + 0x1c) =
       uVar2 & 0x3fff3fff | uVar1 | *(uint *)(*(int *)(param_1 + 0x4c) + 0x1c) & 0xc000;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 3;
  *(short *)(param_1 + 0x3a) = *(short *)(param_1 + 0x3a) + -0x10;
  return;
}

/* ================================================================
 * FUN_0206b7d4 @ 0206b7d4
 * ================================================================ */

void FUN_0206b7d4(int param_1)

{
  (**(code **)(DAT_0206b7ec + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206b7f0 @ 0206b7f0
 * ================================================================ */

void FUN_0206b7f0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  undefined4 uVar4;
  bool bVar5;

  if ((*(byte *)(param_1 + 10) & 0x80) == 0) {
    if (*(char *)(param_1 + 0xb) == '\0') {
      FUN_02085fac();
    }
  }
  else {
    iVar1 = FUN_02076858(0x38,0,1,param_4,param_4);
    if (iVar1 == 0) {
      FUN_0204be88(param_1);
      return;
    }
    FUN_0205031c(param_1,iVar1);
    *DAT_0206b8f0 = iVar1;
    *(int *)(param_1 + 0x58) = iVar1;
    *(int *)(iVar1 + 0x54) = param_1;
    *(uint *)(DAT_0206b8f4 + (uint)*(byte *)(param_1 + 0x52) * 4) =
         *(uint *)(DAT_0206b8f4 + (uint)*(byte *)(param_1 + 0x52) * 4) |
         *(uint *)(DAT_0206b8f8 + (*(byte *)(param_1 + 10) & 0x1f) * 4);
  }
  bVar5 = *(char *)(param_1 + 0xb) != '\0';
  uVar2 = (uint)bVar5;
  uVar4 = *(undefined4 *)(DAT_0206b8f8 + (*(byte *)(param_1 + 10) & 0x1f) * 4);
  *(bool *)(param_1 + 10) = bVar5;
  uVar3 = uVar2;
  if (uVar2 == 0) {
    uVar3 = DAT_0206b8fc;
  }
  *(undefined4 *)(param_1 + 0x7c) = uVar4;
  if (uVar2 == 0) {
    *(uint *)(param_1 + 0x48) = uVar3;
  }
  else {
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
    *(undefined1 *)(param_1 + 0x17) = 0xf;
    *(undefined1 *)(param_1 + 0x1a) = 0;
    FUN_020859e0(param_1,1);
  }
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  return;
}

/* ================================================================
 * FUN_0206b900 @ 0206b900
 * ================================================================ */

void FUN_0206b900(int param_1)

{
  int *piVar1;
  byte bVar2;
  int iVar3;
  int iVar4;

  if (*(char *)(param_1 + 10) == '\0') {
    if ((*DAT_0206ba54 != 0) && (*(int *)(param_1 + 0x58) == *DAT_0206ba54)) {
      *(uint *)(DAT_0206ba58 + (uint)*(byte *)(param_1 + 0x52) * 4) =
           *(uint *)(DAT_0206ba58 + (uint)*(byte *)(param_1 + 0x52) * 4) | *(uint *)(param_1 + 0x7c)
      ;
      return;
    }
    *(uint *)(DAT_0206ba58 + (uint)*(byte *)(param_1 + 0x52) * 4) =
         *(uint *)(DAT_0206ba58 + (uint)*(byte *)(param_1 + 0x52) * 4) & ~*(uint *)(param_1 + 0x7c);
    if (*(byte *)(param_1 + 0x47) != 0) {
      iVar4 = *(int *)(DAT_0206ba5c + (*(byte *)(param_1 + 0x47) - 1) * 0x94);
      iVar3 = FUN_0202a728(*(undefined4 *)(iVar4 + 0x6c));
      piVar1 = DAT_0206ba54;
      if ((iVar3 != 0) && (*(int *)(*(int *)(iVar4 + 0x6c) + 0x58) == *DAT_0206ba54)) {
        *(undefined1 *)(*DAT_0206ba54 + 0x14) = 3;
        *(undefined1 *)(*piVar1 + 0x17) = 2;
        *(int *)(param_1 + 0x58) = *piVar1;
        *(int *)(*piVar1 + 0x54) = param_1;
        FUN_02085fd0(param_1);
      }
      iVar3 = (*(byte *)(param_1 + 0x47) - 1) * 0x94;
      *(byte *)(DAT_0206ba60 + iVar3) = *(byte *)(DAT_0206ba60 + iVar3) & 0x7f;
      *(undefined1 *)(param_1 + 0x47) = 0;
      return;
    }
    return;
  }
  if (*(char *)(param_1 + 0x17) == '\0') {
    FUN_02089524();
  }
  else {
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
  }
  FUN_02085980(param_1);
  if ((*(byte *)(param_1 + 0x29) & 0x80) != 0) {
    FUN_020859e0(param_1,1);
    bVar2 = FUN_0208455c();
    *(byte *)(param_1 + 0x28) = (bVar2 & 0xf0) + 1;
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206ba80 @ 0206ba80
 * ================================================================ */

void FUN_0206ba80(int param_1)

{
  *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x58) = 0;
  FUN_02085fac(*(undefined4 *)(param_1 + 0x54));
  FUN_020859e0(param_1,0);
  *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
  return;
}

/* ================================================================
 * FUN_0206babc @ 0206babc
 * ================================================================ */

void FUN_0206babc(int param_1)

{
  if (*(short *)(param_1 + 0x3a) == 0) {
    FUN_0206bae4(param_1);
  }
  *(undefined1 *)(*(int *)(param_1 + 0x54) + 0x17) = 2;
  return;
}

/* ================================================================
 * FUN_0206bae4 @ 0206bae4
 * ================================================================ */

void FUN_0206bae4(int param_1)

{
  int iVar1;

  iVar1 = FUN_02085c28();
  if (iVar1 == 0) {
    FUN_02076a20(param_1,0xd,0);
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
  *(undefined1 *)(param_1 + 0x14) = 3;
  *(undefined1 *)(param_1 + 0x17) = 0x1e;
  return;
}

/* ================================================================
 * FUN_0206bb34 @ 0206bb34
 * ================================================================ */

void FUN_0206bb34(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;
  int iVar2;
  uint uVar3;

  FUN_0205031c(*(undefined4 *)(param_1 + 0x54),param_1,param_3,param_4,param_4);
  FUN_020859e0(param_1,1);
  cVar1 = *(char *)(param_1 + 0x17);
  *(uint *)(param_1 + 0x1c) =
       *(uint *)(param_1 + 0x1c) & 0xfffffffc | (uint)((*DAT_0206bbec & 1) != 0);
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  iVar2 = DAT_0206bbf0;
  if (cVar1 != '\0') {
    return;
  }
  *(int *)(*(int *)(param_1 + 0x54) + 0x58) = param_1;
  uVar3 = (uint)*(byte *)(*(int *)(param_1 + 0x54) + 0x52);
  *(uint *)(iVar2 + uVar3 * 4) =
       *(uint *)(iVar2 + uVar3 * 4) | *(uint *)(*(int *)(param_1 + 0x54) + 0x7c);
  FUN_02085fd0();
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(undefined1 *)(param_1 + 0x15) = 0;
  *(undefined1 *)(param_1 + 0x17) = 0xf;
  FUN_02085f80(0x22d);
  return;
}

/* ================================================================
 * FUN_0206bbf4 @ 0206bbf4
 * ================================================================ */

void FUN_0206bbf4(int param_1)

{
  uint uVar1;
  int iVar2;

  iVar2 = FUN_02076858(0x39,0,0);
  if (iVar2 == 0) {
    return;
  }
  uVar1 = (uint)(*(ushort *)(DAT_0206bc5c + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) >> 1);
  *(ushort *)(iVar2 + 0x32) = (ushort)((uVar1 << 0x1a) >> 0x16) | 8;
  *(ushort *)(iVar2 + 0x36) = (ushort)((int)(uVar1 & 0xfc0) >> 2) | 8;
  *(undefined2 *)(iVar2 + 0x50) = *(undefined2 *)(param_1 + 0x50);
  return;
}

/* ================================================================
 * FUN_0206bcb8 @ 0206bcb8
 * ================================================================ */

void FUN_0206bcb8(int param_1)

{
  char cVar1;

  FUN_02085980(param_1);
  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_0206bce4 @ 0206bce4
 * ================================================================ */

void FUN_0206bce4(int param_1)

{
  short sVar1;

  sVar1 = *(short *)(param_1 + 0x7e);
  if (sVar1 == 0) {
    (**(code **)(DAT_0206bd64 + (uint)*(byte *)(param_1 + 0x14) * 4))();
    FUN_02085d78(param_1);
    *(undefined2 *)(DAT_0206bd68 + (uint)*(byte *)(param_1 + 0x74) * 2) =
         *(undefined2 *)(param_1 + 0x7c);
    return;
  }
  if (sVar1 == 1) {
    (**(code **)(DAT_0206bd6c + (uint)*(byte *)(param_1 + 0x14) * 4))();
    return;
  }
  if (sVar1 != 2) {
    return;
  }
  if (*(char *)(param_1 + 0x14) != '\0') {
    return;
  }
  FUN_0206c18c(param_1);
  return;
}

/* ================================================================
 * FUN_0206bd70 @ 0206bd70
 * ================================================================ */

void FUN_0206bd70(int param_1)

{
  byte bVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  bool bVar6;

  iVar3 = FUN_0204bcc8();
  if (iVar3 == 0) {
    return;
  }
  iVar4 = FUN_0204bd00();
  if (iVar4 == 0) {
    return;
  }
  *(undefined1 *)(iVar3 + 8) = 6;
  *(undefined1 *)(iVar4 + 8) = 6;
  *(undefined1 *)(iVar3 + 9) = 0x40;
  *(undefined1 *)(iVar4 + 9) = 0x40;
  *(undefined1 *)(iVar3 + 10) = *(undefined1 *)(param_1 + 10);
  *(undefined1 *)(iVar4 + 10) = *(undefined1 *)(param_1 + 10);
  *(int *)(iVar3 + 0x54) = param_1;
  *(int *)(iVar4 + 0x54) = param_1;
  *(undefined2 *)(iVar3 + 0x7e) = 1;
  *(undefined2 *)(iVar4 + 0x7e) = 2;
  FUN_0205031c(param_1,iVar4);
  *(ushort *)(iVar4 + 0x50) = *(ushort *)(param_1 + 0x50) | 3;
  FUN_0204c178(6,iVar3);
  FUN_0204c178(6,iVar4);
  *(undefined1 *)(param_1 + 0x14) = 1;
  bVar6 = (*(byte *)(param_1 + 10) & 1) != 0;
  uVar5 = (uint)*(byte *)(param_1 + 10);
  if (bVar6) {
    uVar5 = DAT_0206beb4;
  }
  if (!bVar6) {
    uVar5 = DAT_0206beb8;
  }
  *(uint *)(param_1 + 0x48) = uVar5;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(undefined4 *)(param_1 + 0xc) = 0x80;
  iVar4 = (uint)*(byte *)(param_1 + 10) * 2;
  cVar2 = *(char *)(DAT_0206bebc + iVar4 + 1);
  *(short *)(param_1 + 0x32) = *(short *)(param_1 + 0x32) + (short)*(char *)(DAT_0206bebc + iVar4);
  *(short *)(param_1 + 0x36) = *(short *)(param_1 + 0x36) + (short)cVar2;
  *(undefined1 *)(param_1 + 0x1a) = 1;
  *(undefined2 *)(param_1 + 0x78) = *(undefined2 *)(param_1 + 0x32);
  *(undefined2 *)(param_1 + 0x7a) = *(undefined2 *)(param_1 + 0x36);
  FUN_0205031c(param_1,iVar3);
  iVar4 = DAT_0206bec0;
  *(undefined2 *)(iVar3 + 0x50) = *(undefined2 *)(param_1 + 0x50);
  bVar1 = *(byte *)(iVar4 + (uint)*(byte *)(param_1 + 0x17));
  *(undefined1 *)(param_1 + 0x74) = *(undefined1 *)(param_1 + 0xb);
  *(ushort *)(param_1 + 0x76) = (ushort)bVar1;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_0206bec4 @ 0206bec4
 * ================================================================ */

void FUN_0206bec4(int param_1)

{
  FUN_02089524();
  if (*(short *)(param_1 + 0x7c) == 0) {
    return;
  }
  *(char *)(param_1 + 0x2b) = *(char *)(param_1 + 10) << 3;
  FUN_0204fd40(param_1);
  FUN_0206c328(param_1);
  return;
}

/* ================================================================
 * FUN_0206befc @ 0206befc
 * ================================================================ */

void FUN_0206befc(int param_1)

{
  FUN_02050840();
  (**(code **)(DAT_0206bf24 + (*(byte *)(param_1 + 0x15) - 5) * 4))(param_1);
  return;
}

/* ================================================================
 * FUN_0206bf28 @ 0206bf28
 * ================================================================ */

void FUN_0206bf28(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  int iVar2;
  short sVar3;
  int iVar4;
  uint uVar5;

  iVar4 = *(int *)(DAT_0206c024 + (short)(ushort)*(byte *)(param_1 + 0xb) * 0x94);
  if (*(char *)(param_1 + 0x75) == '\0') {
    uVar5 = (uint)*(byte *)(iVar4 + 0x2a);
    if (uVar5 == (uint)*(byte *)(param_1 + 10) * 2) {
      iVar1 = (int)*(short *)(param_1 + 0x36) + (int)*(char *)(DAT_0206c028 + uVar5 + 1);
      sVar3 = *(short *)(param_1 + 0x32) + (short)*(char *)(DAT_0206c028 + uVar5);
      iVar2 = FUN_02083d30((int)sVar3,iVar1 * 0x10000 >> 0x10,*(undefined2 *)(iVar4 + 0x50),
                           (int)*(short *)(param_1 + 0x36),param_4);
      if ((iVar2 == 0) || (0x10 < (iVar2 - 0xfU & 0xff))) {
        *(short *)(iVar4 + 0x32) = sVar3;
        *(short *)(iVar4 + 0x36) = (short)iVar1;
      }
    }
    *(undefined1 *)(param_1 + 0x75) = 1;
    *(undefined1 *)(param_1 + 0x18) = 2;
  }
  if ((uint)*(byte *)(param_1 + 10) != (int)(uint)*(byte *)(iVar4 + 0x2a) >> 1) {
    return;
  }
  if (*(char *)(iVar4 + 0x29) == '\0') {
    return;
  }
  FUN_0206c1c8(param_1);
  if (*(char *)(param_1 + 0x75) != '\x01') {
    return;
  }
  *(undefined1 *)(param_1 + 0x75) = 2;
  FUN_02085f80(0xa4);
  return;
}

/* ================================================================
 * FUN_0206c068 @ 0206c068
 * ================================================================ */

void FUN_0206c068(int param_1)

{
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc;
  *(short *)(param_1 + 0x36) = *(short *)(param_1 + 0x36) + -1;
  FUN_020859e0(param_1,*(char *)(*(int *)(param_1 + 0x54) + 10) + '\b');
  FUN_0208ae78(param_1);
  return;
}

/* ================================================================
 * FUN_0206c0b0 @ 0206c0b0
 * ================================================================ */

void FUN_0206c0b0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  uint uVar1;
  byte bVar2;
  ushort uVar3;
  short sVar4;
  uint uVar5;
  int iVar6;

  iVar6 = *(int *)(param_1 + 0x54);
  if (*(ushort *)(iVar6 + 0x7c) < 8) {
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
    return;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  uVar3 = *(short *)(iVar6 + 0x7c) - 8U & 7;
  switch(*(undefined1 *)(param_1 + 10)) {
  case 0:
    sVar4 = (*(short *)(iVar6 + 0x36) - uVar3) + -0x10;
    break;
  case 1:
    sVar4 = *(short *)(iVar6 + 0x32) + uVar3 + 0x11;
    goto LAB_0206c14c;
  case 2:
    sVar4 = *(short *)(iVar6 + 0x36) + uVar3 + 0x10;
    break;
  case 3:
    sVar4 = (*(short *)(iVar6 + 0x32) - uVar3) + -0x11;
LAB_0206c14c:
    *(short *)(param_1 + 0x32) = sVar4;
  default:
    goto switchD_0206c0f4_default;
  }
  *(short *)(param_1 + 0x36) = sVar4;
switchD_0206c0f4_default:
  bVar2 = *(byte *)(param_1 + 0x2a);
  uVar5 = *(ushort *)(*(int *)(param_1 + 0x54) + 0x7c) - 8;
  uVar1 = (uVar5 & 0xfffff) >> 4;
  if (uVar1 == bVar2) {
    return;
  }
  *(char *)(param_1 + 0x2a) = (char)(uVar5 * 0x1000 >> 0x10);
  FUN_020859e0(param_1,(uint)*(byte *)(param_1 + 10) + uVar1 * 4 + 8 & 0xff,(uint)bVar2,uVar1,
               param_4);
  return;
}

/* ================================================================
 * FUN_0206c18c @ 0206c18c
 * ================================================================ */

void FUN_0206c18c(int param_1)

{
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  FUN_020859e0(param_1,*(char *)(param_1 + 10) + '\x04');
  FUN_0208ae78(param_1);
  return;
}

/* ================================================================
 * FUN_0206c1c8 @ 0206c1c8
 * ================================================================ */

void FUN_0206c1c8(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  undefined4 uVar5;
  int iVar6;
  uint uVar7;
  int unaff_r8;

  iVar6 = *(int *)(DAT_0206c320 + (short)(ushort)*(byte *)(param_1 + 0xb) * 0x94);
  uVar7 = (uint)*(byte *)(DAT_0206c31c + (uint)*(byte *)(param_1 + 10));
  if (*(ushort *)(param_1 + 0x76) <= *(ushort *)(param_1 + 0x7c)) goto LAB_0206c304;
  sVar2 = *(short *)(iVar6 + 0x32);
  sVar3 = *(short *)(iVar6 + 0x36);
  FUN_020852f8(iVar6,*(undefined4 *)(param_1 + 0xc),uVar7,DAT_0206c31c,param_4);
  sVar4 = *(short *)(iVar6 + 0x32);
  if ((sVar2 == sVar4) && (sVar3 == *(short *)(iVar6 + 0x36))) goto LAB_0206c304;
  switch(*(undefined1 *)(param_1 + 10)) {
  case 0:
    sVar4 = *(short *)(iVar6 + 0x36) - sVar3;
    break;
  case 1:
    sVar4 = sVar2 - sVar4;
    break;
  case 2:
    sVar4 = sVar3 - *(short *)(iVar6 + 0x36);
    break;
  case 3:
    sVar4 = sVar4 - sVar2;
    break;
  default:
    goto switchD_0206c238_default;
  }
  unaff_r8 = (int)sVar4;
switchD_0206c238_default:
  sVar2 = *(short *)(DAT_0206c324 + (uVar7 * 8 + 0x40) * 2);
  *(int *)(param_1 + 0x30) =
       ((unaff_r8 << 0x18) >> 0x10) * (int)*(short *)(DAT_0206c324 + uVar7 * 0x10) +
       *(int *)(param_1 + 0x30);
  *(int *)(param_1 + 0x34) = *(int *)(param_1 + 0x34) - (int)(short)(unaff_r8 << 8) * (int)sVar2;
  FUN_0206c328(param_1);
  if (*(ushort *)(param_1 + 0x7c) < *(ushort *)(param_1 + 0x76)) {
    cVar1 = *(char *)(param_1 + 0x18);
    *(char *)(param_1 + 0x18) = cVar1 + -1;
    if (cVar1 != '\x01') goto LAB_0206c304;
    uVar5 = 0x8b;
    *(undefined1 *)(param_1 + 0x18) = 8;
  }
  else {
    uVar5 = 0x12;
  }
  FUN_02085f80(uVar5);
LAB_0206c304:
  FUN_0208da4c();
  iVar6 = FUN_0208de54();
  if (iVar6 == 0) {
    *(undefined2 *)(param_1 + 0x72) = 600;
  }
  return;
}

/* ================================================================
 * FUN_0206c328 @ 0206c328
 * ================================================================ */

void FUN_0206c328(int param_1)

{
  switch(*(undefined1 *)(param_1 + 10)) {
  case 0:
    *(short *)(param_1 + 0x7c) = *(short *)(param_1 + 0x36) - *(short *)(param_1 + 0x7a);
    return;
  case 1:
    *(short *)(param_1 + 0x7c) = *(short *)(param_1 + 0x78) - *(short *)(param_1 + 0x32);
    return;
  case 2:
    *(short *)(param_1 + 0x7c) = *(short *)(param_1 + 0x7a) - *(short *)(param_1 + 0x36);
    return;
  case 3:
    *(short *)(param_1 + 0x7c) = *(short *)(param_1 + 0x32) - *(short *)(param_1 + 0x78);
    return;
  default:
    return;
  }
}

/* ================================================================
 * FUN_0206c398 @ 0206c398
 * ================================================================ */

void FUN_0206c398(int param_1)

{
  (**(code **)(DAT_0206c3b0 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206c3b4 @ 0206c3b4
 * ================================================================ */

void FUN_0206c3b4(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  byte bVar1;
  undefined1 uVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;
  char *pcVar6;
  undefined1 *puVar7;
  char local_18 [12];
  undefined4 uStack_c;

  bVar1 = *(byte *)(param_1 + 10);
  *(char *)(param_1 + 0x16) = (char)((int)(uint)bVar1 >> 3);
  *(byte *)(param_1 + 10) = bVar1 & 7;
  uStack_c = param_4;
  if ((bVar1 & 7) == 0) {
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
    FUN_02085fac();
    uVar2 = 3;
  }
  else {
    uVar2 = 1;
  }
  *(undefined1 *)(param_1 + 0x14) = uVar2;
  uVar3 = DAT_0206c4b8;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xffffffcf | 0x10;
  *(undefined1 *)(param_1 + 0x2e) = 4;
  *(short *)(param_1 + 0x3a) = *(short *)(param_1 + 0x3a) + -1;
  *(undefined4 *)(param_1 + 0x48) = uVar3;
  uVar3 = FUN_020460e8(*(undefined1 *)(param_1 + 0xb));
  *(undefined4 *)(param_1 + 0x60) = uVar3;
  *(ushort *)(param_1 + 100) = (ushort)*(byte *)(param_1 + 0x17);
  FUN_0208da4c();
  iVar4 = FUN_0208de54();
  if (iVar4 == 0) {
    iVar4 = 6;
    pcVar6 = local_18;
    puVar7 = DAT_0206c4bc;
    do {
      uVar2 = *puVar7;
      iVar4 = iVar4 + -1;
      pcVar6[1] = puVar7[1];
      *pcVar6 = uVar2;
      pcVar6 = pcVar6 + 2;
      puVar7 = puVar7 + 2;
    } while (iVar4 != 0);
    uVar5 = 0;
    do {
      if (local_18[uVar5] == *(char *)(param_1 + 0x17)) {
        *(short *)(param_1 + 100) = *(short *)(param_1 + 100) + 500;
        break;
      }
      uVar5 = uVar5 + 1;
    } while (uVar5 < 0xc);
  }
  FUN_020859e0(param_1,0);
  return;
}

/* ================================================================
 * FUN_0206c4c0 @ 0206c4c0
 * ================================================================ */

void FUN_0206c4c0(int param_1)

{
  code *UNRECOVERED_JUMPTABLE;

  UNRECOVERED_JUMPTABLE = DAT_0206c4dc;
  *(undefined1 *)(param_1 + 0x17) = 0x14;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
                    /* WARNING: Could not recover jumptable at 0x0206c4d8. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*UNRECOVERED_JUMPTABLE)();
  return;
}

/* ================================================================
 * FUN_0206c4e0 @ 0206c4e0
 * ================================================================ */

void FUN_0206c4e0(int param_1)

{
  uint uVar1;

  if (*(char *)(param_1 + 10) == '\x01') {
    uVar1 = 0;
    do {
      if (*(char *)(DAT_0206c550 + uVar1 + 0x5c) != '\0') {
        FUN_0206c4c0(param_1);
        return;
      }
      uVar1 = uVar1 + 1 & 0xff;
    } while (uVar1 < 4);
    return;
  }
  if (*(char *)(param_1 + 10) != '\x02') {
    return;
  }
  if (*(uint *)(param_1 + 0x60) !=
      (*(uint *)(param_1 + 0x60) & *(uint *)(DAT_0206c554 + (uint)*(byte *)(param_1 + 0x52) * 4))) {
    return;
  }
  FUN_0206c4c0(param_1);
  return;
}

/* ================================================================
 * FUN_0206c558 @ 0206c558
 * ================================================================ */

void FUN_0206c558(int param_1)

{
  char cVar1;

  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  FUN_02085fac();
  return;
}

/* ================================================================
 * FUN_0206c594 @ 0206c594
 * ================================================================ */

void FUN_0206c594(int param_1)

{
  int *piVar1;
  undefined4 uVar2;
  uint uVar3;
  char *pcVar4;
  int iVar5;
  uint uVar6;
  uint extraout_r1;
  int iVar7;
  bool bVar8;

  FUN_02085980(param_1);
  piVar1 = DAT_0206c6f8;
  if (*(char *)(param_1 + 0x47) != '\0') {
    uVar2 = FUN_0208da4c();
    uVar3 = FUN_0208de54();
    bVar8 = uVar3 == 0;
    if (bVar8) {
      uVar3 = (uint)*(byte *)(param_1 + 0x47);
    }
    if ((bVar8 && uVar3 == 2) && (pcVar4 = (char *)FUN_02093b5c(), *pcVar4 == '\0')) {
      *(undefined1 *)(DAT_0206c6e4 + (*(byte *)(param_1 + 0x47) - 1) * 0x94) = 0;
      *(undefined1 *)(param_1 + 0x47) = 0;
      return;
    }
    iVar7 = *(int *)(DAT_0206c6e8 + (*(byte *)(param_1 + 0x47) - 1) * 0x94);
    *(uint *)(DAT_0206c6ec + (uint)*(byte *)(param_1 + 0x52) * 4) =
         *(uint *)(DAT_0206c6ec + (uint)*(byte *)(param_1 + 0x52) * 4) |
         *(uint *)(DAT_0206c6f0 + (uint)*(byte *)(param_1 + 0x16) * 4);
    uVar3 = thunk_FUN_02086078(param_1,iVar7);
    if ((uVar3 & 0xf) != 0) {
      if (uVar3 < 0x10) {
        *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) | 4;
      }
      else {
        *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffb;
      }
    }
    if ((*DAT_0206c6f4 == *(char *)(iVar7 + 10)) || (iVar5 = FUN_0208de54(uVar2), iVar5 == 0)) {
      FUN_02049f0c(*(undefined2 *)(param_1 + 100));
      *(undefined1 *)(param_1 + 0x18) = 1;
    }
    *(undefined1 *)(param_1 + 0x47) = 0;
    *(undefined1 *)(iVar7 + 0x3d) = 0xf8;
    return;
  }
  if (*DAT_0206c6f8 == 0) {
    return;
  }
  uVar6 = (uint)*DAT_0206c6fc;
  bVar8 = uVar6 == 9;
  uVar3 = extraout_r1;
  if (bVar8) {
    uVar6 = (uint)*(ushort *)(param_1 + 100);
    uVar3 = DAT_0206c700;
  }
  if (bVar8 && uVar6 == uVar3) {
    FUN_02049f0c();
    *piVar1 = 0;
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206c720 @ 0206c720
 * ================================================================ */

void FUN_0206c720(int param_1)

{
  undefined2 *puVar1;

  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 1;
  *(undefined1 *)(param_1 + 0x17) = 0xfc;
  FUN_02085cf8();
  puVar1 = DAT_0206c7c8;
  DAT_0206c7c8[1] = 0x200;
  *puVar1 = puVar1[1];
  puVar1[2] = 0;
  FUN_0205063c(puVar1,param_1);
  *(undefined2 *)(param_1 + 0x3a) = 0x10;
  FUN_02029b38(0x1b);
  if (*(char *)(param_1 + 10) == '\0') {
    FUN_020859a0(param_1,0,3);
    FUN_02076fe0(0x19);
    return;
  }
  FUN_020859e0(param_1,0);
  return;
}

/* ================================================================
 * FUN_0206c7cc @ 0206c7cc
 * ================================================================ */

void FUN_0206c7cc(int param_1)

{
  undefined2 *puVar1;

  if (*(char *)(param_1 + 10) == '\0') {
    FUN_02085934(param_1,3);
  }
  else {
    FUN_02085980(param_1);
  }
  puVar1 = DAT_0206c84c;
  if (*(byte *)(param_1 + 0x17) != 0) {
    DAT_0206c84c[1] = *(byte *)(param_1 + 0x17) + 0x100;
    *puVar1 = puVar1[1];
    puVar1[2] = 0;
    FUN_0205063c(puVar1,param_1);
    *(short *)(param_1 + 0x3a) = (short)((int)(uint)*(byte *)(param_1 + 0x17) >> 4);
    *(byte *)(param_1 + 0x17) = *(byte *)(param_1 + 0x17) - 4;
    return;
  }
  *(undefined1 *)(param_1 + 0x17) = 0x5a;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  return;
}
