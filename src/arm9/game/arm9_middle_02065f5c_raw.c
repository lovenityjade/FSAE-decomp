#include "game/arm9_middle_02065f5c_raw.h"

/*
 * Raw matching-first recovery for the 32 catalog entries beginning at
 * 0x02065f5c. These bodies remain address-named promotion candidates and are
 * not linked byte-matching evidence.
 *
 * Calls whose implicit r0 argument was omitted by Ghidra are written with the
 * owning param_1 so this mechanically faithful draft remains valid C.
 */

extern uint DAT_0206601c;
extern uint DAT_0206617c;
extern uint DAT_02066180;
extern uint DAT_02066184;
extern uint DAT_02066188;
extern uint DAT_0206618c;
extern uint DAT_020662a4;
extern uint DAT_020662a8;
extern uint DAT_020662b0;
extern uint DAT_020662b4;
extern uint DAT_020663a4;
extern uint DAT_020663a8;
extern uint DAT_02066480;
extern uint DAT_02066484;
extern uint DAT_020664e0;
extern uint DAT_02066528;
extern uint DAT_020665d8;
extern uint DAT_02066628;
extern uint DAT_0206662c;
extern uint DAT_02066814;
extern uint DAT_02066908;
extern uint DAT_0206690c;
extern uint DAT_02067038;
extern uint DAT_0206703c;
extern uint DAT_02067768;
extern uint DAT_0206776c;
extern uint DAT_0206788c;
extern uint DAT_02067920;
extern uint DAT_02067a2c;
extern uint DAT_02067a30;
extern uint DAT_02067a34;
extern uint DAT_02067a38;
extern uint DAT_02067b38;
extern uint DAT_02067b3c;
extern uint DAT_02067b40;
extern uint *DAT_02067bd0;
extern uint DAT_02067bd4;
extern uint DAT_02067bd8;
extern uint DAT_02067c48;
extern uint DAT_02067cbc;
extern uint DAT_02067d5c;
extern uint DAT_02067d60;
extern uint DAT_02067d64;
extern byte *DAT_02067ecc;
extern uint DAT_02067f90;
extern uint PTR_DAT_020662b0;
extern uint PTR_DAT_020662b4;
extern uint PTR_PTR_020662ac;

/* ================================================================
 * FUN_02065f5c @ 02065f5c
 * ================================================================ */

void FUN_02065f5c(int param_1)

{
  undefined4 uVar1;

  *(short *)(param_1 + 0x76) = *(short *)(param_1 + 0x76) + -1;
  if (*(short *)(param_1 + 0x76) != 0) {
    return;
  }
  *(ushort *)(param_1 + 0x76) = (ushort)*(byte *)(param_1 + 0x17);
  uVar1 = (undefined4)FUN_02066020(param_1,*(undefined4 *)(param_1 + 0x78));
  *(undefined4 *)(param_1 + 0x78) = uVar1;
  return;
}

/* ================================================================
 * FUN_02065f94 @ 02065f94
 * ================================================================ */

void FUN_02065f94(int param_1)

{
  int iVar1;
  uint uVar2;

  uVar2 = *(uint *)(param_1 + 0x70);
  if (uVar2 != 0) {
    if (uVar2 == (uVar2 & *(uint *)(DAT_0206601c + (uint)*(byte *)(param_1 + 0x52) * 4))) {
      *(undefined4 *)(param_1 + 0x70) = 0;
      *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + -1;
      *(ushort *)(param_1 + 0x76) = (ushort)*(byte *)(param_1 + 0x17);
      return;
    }
    return;
  }
  if (*(short *)(param_1 + 0x74) == 0) {
    iVar1 = FUN_02084f98(param_1,*(undefined2 *)(param_1 + 0x7c),*(undefined2 *)(param_1 + 0x7e));
    if (iVar1 != 0) {
      *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + -1;
      *(ushort *)(param_1 + 0x76) = (ushort)*(byte *)(param_1 + 0x17);
    }
    return;
  }
  *(short *)(param_1 + 0x74) = *(short *)(param_1 + 0x74) + -1;
  return;
}

/* ================================================================
 * FUN_02066020 @ 02066020
 * ================================================================ */

ushort * FUN_02066020(int param_1,ushort *param_2)

{
  char cVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;

  uVar5 = (uint)*param_2;
  iVar6 = (int)uVar5 >> 0xc;
  if (iVar6 == 0xe) {
    param_2 = param_2 + -(uVar5 & 0xfff);
    uVar5 = (uint)*param_2;
    iVar6 = (int)uVar5 >> 0xc;
  }
  switch(iVar6) {
  case 10:
    *(ushort *)(param_1 + 0x76) = ((ushort)uVar5 & (ushort)DAT_02066184) * 0x1e;
    break;
  case 0xb:
    uVar4 = FUN_020460e8(uVar5 & 0xff);
    cVar1 = *(char *)(param_1 + 0x14);
    *(undefined4 *)(param_1 + 0x70) = uVar4;
    goto LAB_02066118;
  case 0xc:
    *(uint *)(DAT_02066188 + (uint)*(byte *)(param_1 + 0x52) * 4) =
         *(uint *)(DAT_02066188 + (uint)*(byte *)(param_1 + 0x52) * 4) |
         *(uint *)(DAT_0206618c + (uVar5 & 0x1f) * 4);
    break;
  case 0xd:
    cVar1 = *(char *)(param_1 + 0x14);
    *(ushort *)(param_1 + 0x74) = ((ushort)uVar5 & (ushort)DAT_02066184) * 0x3c;
LAB_02066118:
    *(char *)(param_1 + 0x14) = cVar1 + '\x01';
    break;
  case 0xe:
  default:
    iVar3 = FUN_0204bd80(10);
    if (iVar3 != 0) {
      FUN_0205031c(param_1,iVar3);
      iVar2 = DAT_02066180;
      *(ushort *)(iVar3 + 0x32) =
           (ushort)(((uint)*param_2 << 0x1a) >> 0x16) + 8 +
           *(short *)(DAT_0206617c + (uint)*(byte *)(param_1 + 0x52) * 4);
      *(short *)(iVar3 + 0x36) =
           (short)((int)(*param_2 & 0xfc0) >> 2) + 8 +
           *(short *)(iVar2 + (uint)*(byte *)(param_1 + 0x52) * 4);
      *(char *)(iVar3 + 10) = (char)iVar6;
    }
    break;
  case 0xf:
    FUN_0204be88(param_1);
    return (ushort *)0x0;
  }
  return param_2 + 1;
}

/* ================================================================
 * FUN_020661ac @ 020661ac
 * ================================================================ */

void FUN_020661ac(int param_1)

{
  uint uVar1;
  undefined4 uVar2;
  char cVar3;
  uint uVar4;
  bool bVar5;

  uVar2 = DAT_020662a4;
  uVar4 = (int)(*(byte *)(param_1 + 0xb) & 0xe0) >> 5;
  uVar1 = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  cVar3 = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = uVar1;
  bVar5 = uVar4 < 4;
  if (bVar5) {
    uVar1 = 9;
  }
  *(char *)(param_1 + 0x14) = cVar3;
  if (bVar5) {
    cVar3 = '\x03';
  }
  *(undefined1 *)(param_1 + 0x3c) = 9;
  if (bVar5) {
    *(byte *)(param_1 + 0x3c) = (byte)uVar1 | 0x80;
  }
  *(char *)(param_1 + 0x2a) = (char)uVar4;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc;
  *(undefined1 *)(param_1 + 0x40) = 0x38;
  *(undefined1 *)(param_1 + 0x3f) = 1;
  *(undefined4 *)(param_1 + 0x48) = uVar2;
  *(undefined2 *)(param_1 + 0x5c) = 0xaa;
  *(byte *)(param_1 + 0xb) = *(byte *)(param_1 + 0xb) & 0x1f;
  if (bVar5) {
    *(char *)(param_1 + 0x14) = cVar3;
  }
  uVar2 = FUN_020460e8(*(undefined1 *)(param_1 + 10));
  *(undefined4 *)(param_1 + 0x60) = uVar2;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 0x2a));
  uVar4 = (uint)*(byte *)(DAT_020662a8 + 1);
  if (uVar4 < 5) {
    FUN_02081088(param_1,*(undefined2 *)(PTR_DAT_020662b4 + uVar4 * 2));
    return;
  }
  FUN_02081088(param_1,*(undefined2 *)
                        (PTR_DAT_020662b0 +
                        (uint)*(byte *)(*(int *)(*(int *)(PTR_PTR_020662ac + uVar4 * 4) +
                                                (uint)*(byte *)(DAT_020662a8 + 0x48) * 8) + 1) * 2))
  ;
  return;
}

/* ================================================================
 * FUN_020662e4 @ 020662e4
 * ================================================================ */

void FUN_020662e4(int param_1)

{
  char cVar1;

  FUN_02085980();
  if ((*(byte *)(param_1 + 0x29) & 0x80) == 0) {
    return;
  }
  cVar1 = *(char *)(param_1 + 0x2a);
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(byte *)(param_1 + 0x3c) = *(byte *)(param_1 + 0x3c) | 0x80;
  *(char *)(param_1 + 0x2a) = cVar1 + -4;
  FUN_020859e0(param_1,cVar1 + -4);
  return;
}

/* ================================================================
 * FUN_02066330 @ 02066330
 * ================================================================ */

void FUN_02066330(int param_1)

{
  int iVar1;
  int iVar2;

  iVar2 = DAT_020663a8;
  iVar1 = DAT_020663a4;
  if (*(char *)(param_1 + 0x41) != -0x6b) {
    return;
  }
  if ((*(byte *)(param_1 + 0x2a) & 3) !=
      (int)(*(byte *)(*(int *)(param_1 + 0x4c) + 0x2b) + 4 & 0x18) >> 3) {
    return;
  }
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(byte *)(param_1 + 0x3c) = *(byte *)(param_1 + 0x3c) & 0x7f;
  *(uint *)(iVar2 + (uint)*(byte *)(param_1 + 0x52) * 4) =
       *(uint *)(iVar2 + (uint)*(byte *)(param_1 + 0x52) * 4) |
       *(uint *)(iVar1 + (uint)*(byte *)(param_1 + 0xb) * 4);
  FUN_02085f80(0x17);
  return;
}

/* ================================================================
 * FUN_020663ac @ 020663ac
 * ================================================================ */

void FUN_020663ac(int param_1)

{
  byte bVar1;
  int iVar2;

  FUN_02085980();
  if ((*(byte *)(param_1 + 0x29) & 0x80) != 0) {
    *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
    FUN_0208da4c();
    iVar2 = FUN_0208de54();
    bVar1 = *(byte *)(param_1 + 0x17);
    if (iVar2 == 0) {
      if ((bVar1 == 0) || (0x3b < bVar1)) {
        *(ushort *)(param_1 + 0x7c) = (ushort)bVar1 * 3;
      }
      else {
        *(undefined2 *)(param_1 + 0x7c) = 0xb4;
      }
    }
    else {
      *(ushort *)(param_1 + 0x7c) = (ushort)bVar1 << 1;
    }
    FUN_020859e0(param_1,*(char *)(param_1 + 0x2a) + '\x04');
    return;
  }
  return;
}

/* ================================================================
 * FUN_02066424 @ 02066424
 * ================================================================ */

void FUN_02066424(int param_1)

{
  int iVar1;
  uint uVar2;

  if (*(char *)(param_1 + 0x17) == '\0') {
    return;
  }
  *(short *)(param_1 + 0x7c) = *(short *)(param_1 + 0x7c) + -1;
  iVar1 = DAT_02066484;
  if (*(short *)(param_1 + 0x7c) != 0) {
    return;
  }
  uVar2 = *(uint *)(DAT_02066480 + (uint)*(byte *)(param_1 + 0xb) * 4);
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(iVar1 + (uint)*(byte *)(param_1 + 0x52) * 4) =
       *(uint *)(iVar1 + (uint)*(byte *)(param_1 + 0x52) * 4) & ~uVar2;
  return;
}

/* ================================================================
 * FUN_02066488 @ 02066488
 * ================================================================ */

void FUN_02066488(int param_1)

{
  FUN_02085980();
  if ((*(byte *)(param_1 + 0x29) & 0x80) == 0) {
    return;
  }
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + -3;
  *(byte *)(param_1 + 0x3c) = *(byte *)(param_1 + 0x3c) | 0x80;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 0x2a));
  return;
}

/* ================================================================
 * FUN_020664c8 @ 020664c8
 * ================================================================ */

void FUN_020664c8(int param_1)

{
  (**(code **)(DAT_020664e0 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_020664e4 @ 020664e4
 * ================================================================ */

void FUN_020664e4(int param_1)

{
  undefined4 uVar1;
  byte bVar2;

  uVar1 = DAT_02066528;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined4 *)(param_1 + 0x48) = uVar1;
  *(undefined4 *)(param_1 + 0x10) = 0x1c000;
  bVar2 = FUN_0208455c();
  *(byte *)(param_1 + 0x2b) = bVar2 & 0x1f;
  *(undefined4 *)(param_1 + 0xc) = 0x100;
  *(undefined1 *)(param_1 + 0x17) = 10;
  return;
}

/* ================================================================
 * FUN_020665c0 @ 020665c0
 * ================================================================ */

void FUN_020665c0(int param_1)

{
  (**(code **)(DAT_020665d8 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_020665dc @ 020665dc
 * ================================================================ */

void FUN_020665dc(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;

  uVar1 = DAT_0206662c;
  uVar2 = *(undefined4 *)(DAT_02066628 + (uint)*(byte *)(param_1 + 10) * 4);
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined1 *)(param_1 + 0x45) = 100;
  *(undefined1 *)(param_1 + 0x3c) = 0x89;
  *(undefined1 *)(param_1 + 0x40) = 0x38;
  *(undefined1 *)(param_1 + 0x3f) = 0x3e;
  *(undefined4 *)(param_1 + 0x48) = uVar2;
  *(short *)(param_1 + 0x5c) = (short)uVar1;
  return;
}

/* ================================================================
 * FUN_02066630 @ 02066630
 * ================================================================ */

void FUN_02066630(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  if (*(char *)(param_1 + 0x7c) != '\0') {
    *(char *)(param_1 + 0x7c) = *(char *)(param_1 + 0x7c) + -1;
  }
  if (*(char *)(param_1 + 0x7d) != '\0') {
    *(char *)(param_1 + 0x7d) = *(char *)(param_1 + 0x7d) + -1;
  }
  if (*(char *)(param_1 + 0x7e) != '\0') {
    *(char *)(param_1 + 0x7e) = *(char *)(param_1 + 0x7e) + -1;
  }
  if (*(char *)(param_1 + 0x7f) != '\0') {
    *(char *)(param_1 + 0x7f) = *(char *)(param_1 + 0x7f) + -1;
  }
  if (*(char *)(param_1 + 0x18) != '\0') {
    *(char *)(param_1 + 0x18) = *(char *)(param_1 + 0x18) + -1;
  }
  if ((*(byte *)(param_1 + 0x41) & 0x80) != 0) {
    switch(*(undefined1 *)(*(int *)(param_1 + 0x4c) + 10)) {
    case 0:
      goto LAB_020666b4;
    case 1:
      if (*(char *)(param_1 + 0x7d) != '\0') {
        return;
      }
      FUN_02066818(param_1,param_1 + 0x7d,param_3,param_4,param_4);
      return;
    case 2:
      if (*(char *)(param_1 + 0x7e) != '\0') {
        return;
      }
      FUN_02066818(param_1,param_1 + 0x7e,param_3,param_4,param_4);
      return;
    case 3:
      if (*(char *)(param_1 + 0x7f) != '\0') {
        return;
      }
      FUN_02066818(param_1,param_1 + 0x7f,param_3,param_4,param_4);
      return;
    default:
      if (*(char *)(param_1 + 0x18) != '\0') {
        return;
      }
      FUN_02066818(param_1,param_1 + 0x18,param_3,param_4,param_4);
      return;
    }
  }
  return;
LAB_020666b4:
  if (*(char *)(param_1 + 0x7c) != '\0') {
    return;
  }
  FUN_02066818(param_1,param_1 + 0x7c,param_3,param_4,param_4);
  return;
}

/* ================================================================
 * FUN_0206672c @ 0206672c
 * ================================================================ */

void FUN_0206672c(int param_1)

{
  char cVar1;
  byte bVar2;

  cVar1 = *(char *)(param_1 + 0x17);
  bVar2 = cVar1 + 1;
  *(byte *)(param_1 + 0x17) = bVar2;
  if (*(byte *)(param_1 + 0x18) <= (byte)(cVar1 + 1U)) {
    FUN_02067040(param_1);
    FUN_0205747c(8,0);
    FUN_0204be88(param_1);
    return;
  }
  if ((bVar2 & 1) == 0) {
    return;
  }
  FUN_02067770(param_1);
  return;
}

/* ================================================================
 * FUN_0206677c @ 0206677c
 * ================================================================ */

void FUN_0206677c(int param_1)

{
  byte bVar1;
  char cVar2;
  ushort uVar3;
  int iVar4;
  undefined4 uVar5;
  uint uVar6;
  short extraout_r1;

  iVar4 = FUN_02076858(0xf,0xf,0);
  if (iVar4 == 0) {
    return;
  }
  bVar1 = *(byte *)(param_1 + 10);
  uVar5 = FUN_0208455c();
  uVar3 = FUN_0208455c();
  FUN_0200db54(uVar5,bVar1 + 2);
  cVar2 = *(char *)(DAT_02066814 + (uint)bVar1);
  uVar6 = FUN_0208455c();
  FUN_02050390(param_1,iVar4,
               (int)(short)((short)cVar2 + extraout_r1 * 0x10 + (uVar3 & 7) + -4) << 0x10,
               ((int)(((uVar6 & 0x1f) - 0x10) * 0x10000) >> 0x10) << 0x10);
  return;
}

/* ================================================================
 * FUN_02066818 @ 02066818
 * ================================================================ */

void FUN_02066818(int param_1,undefined1 *param_2)

{
  byte bVar1;
  int iVar2;
  uint uVar3;

  uVar3 = (uint)*(byte *)(*(int *)(param_1 + 0x4c) + 0x40);
  if (0x23 < uVar3) {
    return;
  }
  bVar1 = *(byte *)(DAT_02066908 + uVar3);
  if (bVar1 != 0) {
    *param_2 = 8;
    FUN_0208da4c();
    iVar2 = FUN_0208de54();
    if (((iVar2 == 0) && (iVar2 = FUN_02093b5c(), *(char *)(iVar2 + 1) != '\0')) &&
       (*(char *)(*(int *)(DAT_0206690c + 0x28) + 0x21) ==
        *(char *)(*(int *)(DAT_0206690c + 0xbc) + 0x21))) {
      bVar1 = bVar1 << 1;
    }
    if (*(byte *)(param_1 + 0x45) < bVar1) {
      bVar1 = 0;
    }
    else {
      bVar1 = *(byte *)(param_1 + 0x45) - bVar1;
    }
    *(byte *)(param_1 + 0x45) = bVar1;
    if (bVar1 == 0) {
      *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
      *(undefined1 *)(param_1 + 0x17) = 0;
      *(char *)(param_1 + 0x18) = (*(char *)(param_1 + 10) + '\x01') * '\x02';
      *(byte *)(param_1 + 0x3c) = *(byte *)(param_1 + 0x3c) & 0x7f;
      return;
    }
    if ((*(int *)(param_1 + 0x60) == 0) && (bVar1 < 0x32)) {
      FUN_02066910(param_1);
      *(undefined4 *)(param_1 + 0x60) = 1;
    }
    FUN_0206677c(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02066910 @ 02066910
 * ================================================================ */

void FUN_02066910(int param_1)

{
  uint uVar1;
  uint uVar2;
  undefined2 *puVar3;

  puVar3 = *(undefined2 **)(DAT_02067038 + (uint)*(byte *)(param_1 + 10) * 4);
  switch((uint)*(byte *)(param_1 + 10)) {
  case 0:
    uVar2 = ((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_0206703c + 0x1a)) >> 4)
            << 6 | (int)((*(short *)(param_1 + 0x32) + -8) - (uint)*(ushort *)(DAT_0206703c + 0x18))
                   >> 4;
    uVar1 = uVar2 & 0xffff;
    FUN_02021c28(*puVar3,uVar1 - 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[1],uVar1 - 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[2],uVar1 - 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[3],uVar1 - 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[4],uVar1 - 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[5],uVar2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[6],uVar1 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[7],uVar1 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[8],uVar1 + 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[9],uVar1 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[10],uVar1 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xb],uVar1 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    return;
  case 1:
    uVar2 = ((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_0206703c + 0x1a)) >> 4)
            << 6 | (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_0206703c + 0x18))
                   >> 4;
    uVar1 = uVar2 & 0xffff;
    FUN_02021c28(*puVar3,uVar1 - 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[1],uVar1 - 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[2],uVar1 - 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[3],uVar1 - 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[4],uVar1 - 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[5],uVar1 - 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[6],uVar1 - 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[7],uVar2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[8],uVar1 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[9],uVar1 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[10],uVar1 + 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xb],uVar1 + 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xc],uVar1 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xd],uVar1 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xe],uVar1 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    return;
  case 2:
    uVar2 = ((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_0206703c + 0x1a)) >> 4)
            << 6 | (int)((*(short *)(param_1 + 0x32) + -8) - (uint)*(ushort *)(DAT_0206703c + 0x18))
                   >> 4;
    uVar1 = uVar2 & 0xffff;
    FUN_02021c28(*puVar3,uVar1 - 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[1],uVar1 - 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[2],uVar1 - 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[3],uVar1 - 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[4],uVar1 - 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[5],uVar1 - 0x3d & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[6],uVar1 - 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[7],uVar1 - 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[8],uVar2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[9],uVar1 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[10],uVar1 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xb],uVar1 + 3 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xc],uVar1 + 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xd],uVar1 + 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xe],uVar1 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xf],uVar1 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x10],uVar1 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x11],uVar1 + 0x43 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    return;
  case 3:
    uVar2 = ((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_0206703c + 0x1a)) >> 4)
            << 6 | (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_0206703c + 0x18))
                   >> 4;
    uVar1 = uVar2 & 0xffff;
    FUN_02021c28(*puVar3,uVar1 - 0x43 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[1],uVar1 - 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[2],uVar1 - 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[3],uVar1 - 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[4],uVar1 - 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[5],uVar1 - 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[6],uVar1 - 0x3d & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[7],uVar1 - 3 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[8],uVar1 - 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[9],uVar1 - 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[10],uVar2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xb],uVar1 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xc],uVar1 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xd],uVar1 + 3 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xe],uVar1 + 0x3d & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xf],uVar1 + 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x10],uVar1 + 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x11],uVar1 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x12],uVar1 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x13],uVar1 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x14],uVar1 + 0x43 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    return;
  default:
    return;
  }
}

/* ================================================================
 * FUN_02067040 @ 02067040
 * ================================================================ */

void FUN_02067040(int param_1)

{
  uint uVar1;
  uint uVar2;
  undefined2 *puVar3;

  puVar3 = *(undefined2 **)(DAT_02067768 + (uint)*(byte *)(param_1 + 10) * 4);
  switch((uint)*(byte *)(param_1 + 10)) {
  case 0:
    uVar2 = ((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_0206776c + 0x1a)) >> 4)
            << 6 | (int)((*(short *)(param_1 + 0x32) + -8) - (uint)*(ushort *)(DAT_0206776c + 0x18))
                   >> 4;
    uVar1 = uVar2 & 0xffff;
    FUN_02021c28(*puVar3,uVar1 - 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[1],uVar1 - 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[2],uVar1 - 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[3],uVar1 - 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[4],uVar1 - 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[5],uVar2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[6],uVar1 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[7],uVar1 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[8],uVar1 + 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[9],uVar1 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[10],uVar1 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xb],uVar1 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    return;
  case 1:
    uVar2 = ((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_0206776c + 0x1a)) >> 4)
            << 6 | (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_0206776c + 0x18))
                   >> 4;
    uVar1 = uVar2 & 0xffff;
    FUN_02021c28(*puVar3,uVar1 - 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[1],uVar1 - 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[2],uVar1 - 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[3],uVar1 - 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[4],uVar1 - 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[5],uVar1 - 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[6],uVar1 - 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[7],uVar2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[8],uVar1 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[9],uVar1 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[10],uVar1 + 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xb],uVar1 + 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xc],uVar1 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xd],uVar1 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xe],uVar1 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    return;
  case 2:
    uVar2 = ((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_0206776c + 0x1a)) >> 4)
            << 6 | (int)((*(short *)(param_1 + 0x32) + -8) - (uint)*(ushort *)(DAT_0206776c + 0x18))
                   >> 4;
    uVar1 = uVar2 & 0xffff;
    FUN_02021c28(*puVar3,uVar1 - 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[1],uVar1 - 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[2],uVar1 - 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[3],uVar1 - 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[4],uVar1 - 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[5],uVar1 - 0x3d & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[6],uVar1 - 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[7],uVar1 - 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[8],uVar2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[9],uVar1 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[10],uVar1 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xb],uVar1 + 3 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xc],uVar1 + 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xd],uVar1 + 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xe],uVar1 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xf],uVar1 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x10],uVar1 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x11],uVar1 + 0x43 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    return;
  case 3:
    uVar2 = ((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_0206776c + 0x1a)) >> 4)
            << 6 | (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_0206776c + 0x18))
                   >> 4;
    uVar1 = uVar2 & 0xffff;
    FUN_02021c28(*puVar3,uVar1 - 0x43 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[1],uVar1 - 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[2],uVar1 - 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[3],uVar1 - 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[4],uVar1 - 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[5],uVar1 - 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[6],uVar1 - 0x3d & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[7],uVar1 - 3 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[8],uVar1 - 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[9],uVar1 - 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[10],uVar2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xb],uVar1 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xc],uVar1 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xd],uVar1 + 3 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xe],uVar1 + 0x3d & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0xf],uVar1 + 0x3e & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x10],uVar1 + 0x3f & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x11],uVar1 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x12],uVar1 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x13],uVar1 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(puVar3[0x14],uVar1 + 0x43 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    return;
  default:
    return;
  }
}

/* ================================================================
 * FUN_02067770 @ 02067770
 * ================================================================ */

void FUN_02067770(int param_1)

{
  int iVar1;

  iVar1 = FUN_02076858(0xf,2,0);
  if (iVar1 != 0) {
    *(ushort *)(iVar1 + 0x32) =
         *(short *)(param_1 + 0x32) -
         (ushort)*(byte *)(DAT_0206788c + ((int)(uint)*(byte *)(param_1 + 0x17) >> 1));
    *(short *)(iVar1 + 0x36) = *(short *)(param_1 + 0x36) + -0x14;
    *(undefined2 *)(iVar1 + 0x50) = *(undefined2 *)(param_1 + 0x50);
  }
  iVar1 = FUN_02076858(0xf,2,0);
  if (iVar1 != 0) {
    *(ushort *)(iVar1 + 0x32) =
         *(short *)(param_1 + 0x32) +
         (ushort)*(byte *)(DAT_0206788c + ((int)(uint)*(byte *)(param_1 + 0x17) >> 1));
    *(short *)(iVar1 + 0x36) = *(short *)(param_1 + 0x36) + -0x14;
    *(undefined2 *)(iVar1 + 0x50) = *(undefined2 *)(param_1 + 0x50);
  }
  iVar1 = FUN_02076858(0xf,2,0);
  if (iVar1 != 0) {
    *(ushort *)(iVar1 + 0x32) =
         *(short *)(param_1 + 0x32) -
         (ushort)*(byte *)(DAT_0206788c + ((int)(uint)*(byte *)(param_1 + 0x17) >> 1));
    *(short *)(iVar1 + 0x36) = *(short *)(param_1 + 0x36) + 0x14;
    *(undefined2 *)(iVar1 + 0x50) = *(undefined2 *)(param_1 + 0x50);
  }
  iVar1 = FUN_02076858(0xf,2,0);
  if (iVar1 == 0) {
    return;
  }
  *(ushort *)(iVar1 + 0x32) =
       *(short *)(param_1 + 0x32) +
       (ushort)*(byte *)(DAT_0206788c + ((int)(uint)*(byte *)(param_1 + 0x17) >> 1));
  *(short *)(iVar1 + 0x36) = *(short *)(param_1 + 0x36) + 0x14;
  *(undefined2 *)(iVar1 + 0x50) = *(undefined2 *)(param_1 + 0x50);
  return;
}

/* ================================================================
 * FUN_020678e4 @ 020678e4
 * ================================================================ */

void FUN_020678e4(int param_1)

{
  undefined4 uVar1;

  uVar1 = DAT_02067920;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined1 *)(param_1 + 0x3c) = 0x89;
  *(undefined1 *)(param_1 + 0x40) = 0x38;
  *(undefined1 *)(param_1 + 0x3f) = 0x45;
  *(undefined4 *)(param_1 + 0x48) = uVar1;
  *(undefined2 *)(param_1 + 0x5c) = 0xaa;
  return;
}

/* ================================================================
 * FUN_02067924 @ 02067924
 * ================================================================ */

void FUN_02067924(int param_1)

{
  uint uVar1;
  ushort uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;

  iVar5 = DAT_02067a34;
  iVar4 = DAT_02067a30;
  iVar3 = DAT_02067a2c;
  if (*(char *)(param_1 + 0x41) != -0x6b) {
    return;
  }
  if ((*(char *)(*(int *)(param_1 + 0x4c) + 0x2b) + 4U & 0x18) != 0) {
    return;
  }
  *(byte *)(param_1 + 0x3c) = *(byte *)(param_1 + 0x3c) & 0x7f;
  iVar6 = DAT_02067a38;
  uVar2 = *(ushort *)(DAT_02067a38 + 0x18);
  *(uint *)(iVar5 + (uint)*(byte *)(param_1 + 0x52) * 4) =
       *(uint *)(iVar5 + (uint)*(byte *)(param_1 + 0x52) * 4) |
       *(uint *)(iVar4 + (uint)*(byte *)(param_1 + 10) * 4);
  uVar7 = ((int)((*(short *)(param_1 + 0x36) + -8) - (uint)*(ushort *)(iVar6 + 0x1a)) >> 4) << 6 |
          (int)((*(short *)(param_1 + 0x32) + -8) - (uint)uVar2) >> 4;
  uVar1 = uVar7 & 0xffff;
  FUN_02021c28(iVar3,uVar7 & 0xffff,*(undefined2 *)(param_1 + 0x50));
  FUN_02021c28(iVar3 + 1,uVar1 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
  FUN_02021c28(0x154,uVar1 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
  FUN_02021c28(0x155,uVar1 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
  FUN_02085f98(param_1,0x17);
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02067a60 @ 02067a60
 * ================================================================ */

void FUN_02067a60(int param_1)

{
  uint uVar1;
  int iVar2;
  uint uVar3;

  iVar2 = DAT_02067b40;
  if (*(uint *)(param_1 + 0x60) !=
      (*(uint *)(param_1 + 0x60) & *(uint *)(DAT_02067b38 + (uint)*(byte *)(param_1 + 0x52) * 4))) {
    return;
  }
  uVar3 = ((int)((*(short *)(param_1 + 0x36) + -8) - (uint)*(ushort *)(DAT_02067b3c + 0x1a)) >> 4)
          << 6 | (int)((*(short *)(param_1 + 0x32) + -8) - (uint)*(ushort *)(DAT_02067b3c + 0x18))
                 >> 4;
  uVar1 = uVar3 & 0xffff;
  FUN_02021c28(DAT_02067b40,uVar3 & 0xffff,*(undefined2 *)(param_1 + 0x50));
  FUN_02021c28(iVar2 + 1,uVar1 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
  FUN_02021c28(0x150,uVar1 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
  FUN_02021c28(0x151,uVar1 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
  FUN_02085f98(param_1,0x17);
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02067b60 @ 02067b60
 * ================================================================ */

void FUN_02067b60(int param_1)

{
  uint *puVar1;
  int iVar2;

  puVar1 = DAT_02067bd0;
  *(undefined1 *)(param_1 + 0x14) = 1;
  if ((*puVar1 & 1 << (uint)*(byte *)(param_1 + 0xb)) == 0) {
    return;
  }
  puVar1[4] = puVar1[4] | 1 << (uint)*(byte *)(param_1 + 0xb);
  FUN_020768d8();
  iVar2 = FUN_02084600(puVar1[4] &
                       *(uint *)(DAT_02067bd4 + (uint)(*(byte *)(param_1 + 0xb) >> 2) * 4));
  FUN_020769d4(param_1,*(undefined1 *)(DAT_02067bd8 + iVar2));
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02067bf8 @ 02067bf8
 * ================================================================ */

void FUN_02067bf8(int param_1)

{
  int iVar1;
  undefined1 uVar2;

  iVar1 = DAT_02067c48;
  *(undefined1 *)(param_1 + 0x14) = 1;
  uVar2 = FUN_02083d4c((((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(iVar1 + 0x1a)) >>
                        4) << 6 |
                       (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(iVar1 + 0x18)) >> 4
                       ) & 0xffff,*(undefined2 *)(param_1 + 0x50));
  *(undefined1 *)(param_1 + 0x18) = uVar2;
  return;
}

/* ================================================================
 * FUN_02067c4c @ 02067c4c
 * ================================================================ */

void FUN_02067c4c(int param_1)

{
  byte bVar1;
  uint uVar2;

  uVar2 = FUN_02083d4c((((int)((int)*(short *)(param_1 + 0x36) -
                              (uint)*(ushort *)(DAT_02067cbc + 0x1a)) >> 4) << 6 |
                       (int)((int)*(short *)(param_1 + 0x32) -
                            (uint)*(ushort *)(DAT_02067cbc + 0x18)) >> 4) & 0xffff,
                       *(undefined2 *)(param_1 + 0x50));
  if (*(byte *)(param_1 + 0x18) != uVar2) {
    if (uVar2 == 0xf) {
      bVar1 = *(byte *)(param_1 + 0xb);
    }
    else {
      bVar1 = *(byte *)(param_1 + 0x17);
    }
    *(undefined1 *)(param_1 + 0x14) = 2;
    *(undefined1 *)(param_1 + 0x16) = 0;
    *(uint *)(param_1 + 0x60) = (uint)bVar1;
    return;
  }
  return;
}

/* ================================================================
 * FUN_02067cc0 @ 02067cc0
 * ================================================================ */

void FUN_02067cc0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  char cVar2;
  undefined1 uVar3;
  undefined4 uVar4;
  uint uVar5;

  cVar2 = *(char *)(param_1 + 0x16) + -1;
  *(char *)(param_1 + 0x16) = cVar2;
  iVar1 = DAT_02067d5c;
  if (cVar2 != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 1;
  uVar5 = (uint)*(ushort *)(iVar1 + 0x18);
  uVar3 = FUN_02083d4c((((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(iVar1 + 0x1a)) >>
                        4) << 6 | (int)((int)*(short *)(param_1 + 0x32) - uVar5) >> 4) & 0xffff,
                       *(undefined2 *)(param_1 + 0x50),(int)*(short *)(param_1 + 0x36),uVar5,param_4
                      );
  uVar5 = (uint)*(byte *)(param_1 + 0x52);
  *(undefined1 *)(param_1 + 0x18) = uVar3;
  uVar4 = FUN_0204d634(uVar5,*(uint *)(param_1 + 0x60) & 0xff);
  FUN_0204602c(uVar4,*(ushort *)(DAT_02067d60 + uVar5 * 4) & 0xff,
               *(ushort *)(DAT_02067d64 + uVar5 * 4) & 0xff,uVar5);
  return;
}

/* ================================================================
 * FUN_02067da0 @ 02067da0
 * ================================================================ */

void FUN_02067da0(int param_1)

{
  int iVar1;

  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 2;
  iVar1 = 2;
  if (*(char *)(param_1 + 10) != '\0') {
    iVar1 = 1;
  }
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | iVar1 << 0xe | 0x40002000;
  *(undefined1 *)(param_1 + 0x17) = 7;
  *(undefined2 *)(param_1 + 0x32) = 0x80;
  *(undefined2 *)(param_1 + 0x36) = 0x60;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_02067e50 @ 02067e50
 * ================================================================ */

void FUN_02067e50(int param_1)

{
  ushort uVar1;
  uint uVar2;
  bool bVar3;

  if (*DAT_02067ecc != '\x03') {
    return;
  }
  uVar2 = FUN_0208455c();
  if ((uVar2 & 0x7f) == 0) {
    uVar2 = FUN_0208455c();
    FUN_02076858(0x28,9,uVar2 & 3);
  }
  uVar1 = (ushort)*(byte *)(param_1 + 0x18);
  bVar3 = uVar1 == 0;
  if (bVar3) {
    *(short *)(param_1 + 0x7c) = *(short *)(param_1 + 0x7c) + -1;
    uVar1 = *(ushort *)(param_1 + 0x7c);
  }
  if (bVar3 && uVar1 == 0) {
    FUN_02076858(0x28,5,0);
    uVar1 = FUN_0208455c();
    *(ushort *)(param_1 + 0x7c) = (uVar1 & 0xff) + 0x46;
    return;
  }
  return;
}

/* ================================================================
 * FUN_02067eec @ 02067eec
 * ================================================================ */

void FUN_02067eec(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x40002002;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined2 *)(param_1 + 0x32) = 0x100;
  *(undefined2 *)(param_1 + 0x36) = 0x60;
  *(undefined1 *)(param_1 + 0x2b) = 0x18;
  *(undefined4 *)(param_1 + 0xc) = 0x810;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10),0x810,0x18,param_4);
  return;
}

/* ================================================================
 * FUN_02067f48 @ 02067f48
 * ================================================================ */

void FUN_02067f48(int param_1)

{
  FUN_0204fd40();
  if (*(short *)(param_1 + 0x32) < 0x80) {
    *(undefined2 *)(param_1 + 0x32) = 0x80;
    *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  }
  return;
}

/* ================================================================
 * FUN_02067f78 @ 02067f78
 * ================================================================ */

void FUN_02067f78(int param_1)

{
  (**(code **)(DAT_02067f90 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}
