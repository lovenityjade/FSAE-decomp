#include "game/arm9_middle_0207303c_raw.h"

/*
 * Raw matching-first recovery for the 32 catalog entries beginning at
 * 0x0207303c. These bodies remain address-named promotion candidates and are
 * not linked byte-matching evidence.
 *
 * Calls whose implicit register arguments were omitted by Ghidra are written
 * with the live recovered values so this mechanically faithful draft remains
 * valid C.
 */

extern uint DAT_020730ac;
extern uint DAT_020730dc;
extern uint DAT_02073188;
extern uint DAT_020732e4;
extern uint DAT_020732e8;
extern uint DAT_020732ec;
extern uint DAT_020732f0;
extern uint DAT_0207338c;
extern uint DAT_02073490;
extern undefined4 *DAT_02073494;
extern uint DAT_02073498;
extern uint DAT_0207349c;
extern uint DAT_020734a0;
extern uint *DAT_0207357c;
extern uint DAT_02073580;
extern uint DAT_02073584;
extern uint *DAT_0207363c;
extern uint *DAT_0207377c;
extern int *DAT_02073820;
extern uint DAT_02073824;
extern undefined2 *DAT_02073828;
extern uint *DAT_02073888;
extern undefined2 *DAT_0207388c;
extern uint DAT_020738ac;
extern uint *DAT_02073a48;
extern undefined2 *DAT_02073b44;
extern uint *DAT_02073bb4;
extern undefined2 *DAT_02073bb8;
extern uint DAT_02073c18;
extern uint DAT_02073c58;
extern uint *DAT_02073cac;
extern uint DAT_02073d38;
extern uint *DAT_02073da4;
extern uint DAT_02073e58;
extern uint DAT_02073e5c;
extern uint DAT_02073ef4;
extern uint DAT_02073ef8;

/* ================================================================
 * FUN_0207303c @ 0207303c
 * ================================================================ */

void FUN_0207303c(int param_1)

{
  char cVar1;

  FUN_02085934(param_1,*(undefined1 *)(param_1 + 0x1b));
  FUN_0204fd40(param_1);
  cVar1 = *(char *)(param_1 + 0x17);
  if ((cVar1 != '\0') && (*(char *)(param_1 + 0x17) = cVar1 + -1, cVar1 == '\x01')) {
    *(undefined1 *)(param_1 + 0x2a) = 4;
    *(undefined1 *)(param_1 + 0x2b) = 0x10;
  }
  if (0x13c < *(short *)(param_1 + 0x36)) {
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02073094 @ 02073094
 * ================================================================ */

void FUN_02073094(int param_1)

{
  (**(code **)(DAT_020730ac + (uint)*(byte *)(param_1 + 0x15) * 4))();
  return;
}

/* ================================================================
 * FUN_020730b0 @ 020730b0
 * ================================================================ */

void FUN_020730b0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  FUN_0205069c(param_1,DAT_020730dc,0x78,param_4,param_4);
  *(undefined1 *)(param_1 + 0x17) = 0x78;
  *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
  return;
}

/* ================================================================
 * FUN_020730e0 @ 020730e0
 * ================================================================ */

void FUN_020730e0(int param_1)

{
  uint uVar1;
  int iVar2;
  char cVar3;
  uint uVar4;

  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  FUN_020506d8(param_1);
  cVar3 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar3;
  if (cVar3 != '\0') {
    return;
  }
  *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
  *(undefined1 *)(param_1 + 0x17) = 0x3c;
  iVar2 = DAT_02073188;
  if (*(char *)(param_1 + 10) == '\0') {
    uVar4 = *(uint *)(param_1 + 0x1c);
    *(uint *)(param_1 + 0x1c) = uVar4 & 0xfffffffc | 1;
    uVar1 = *(byte *)(iVar2 + 0x53) + 7 & 0xf;
    *(uint *)(param_1 + 0x1c) = uVar4 & 0xff00fffc | 1 | uVar1 << 0x10 | uVar1 << 0x14;
    FUN_02076fe0(0x208);
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_020731f0 @ 020731f0
 * ================================================================ */

void FUN_020731f0(int param_1)

{
  uint uVar1;
  undefined2 uVar2;
  byte bVar3;
  int iVar4;
  uint uVar5;

  iVar4 = DAT_020732e8;
  uVar5 = *(uint *)(param_1 + 0x1c);
  uVar1 = (*(byte *)(DAT_020732e4 + (uint)*(byte *)(param_1 + 10)) & 0xf) << 0x10;
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc;
  *(uint *)(param_1 + 0x1c) = uVar5 & 0xfff0ffff | uVar1;
  if (*(char *)(iVar4 + 1) == '\x02') {
    *(uint *)(param_1 + 0x1c) = uVar5 & 0xfff0fffc | uVar1 | 1;
    *(short *)(param_1 + 0x32) = *(short *)(param_1 + 0x32) + 4;
    *(short *)(param_1 + 0x36) = *(short *)(param_1 + 0x36) + -2;
  }
  FUN_020859e0(param_1,0);
  bVar3 = FUN_0208455c();
  *(byte *)(param_1 + 0x28) = (bVar3 & 0xf0) + 1;
  iVar4 = FUN_02056f30(*(undefined2 *)(param_1 + 0x50));
  uVar2 = *(undefined2 *)(DAT_020732f0 + (uint)*(byte *)(param_1 + 10) * 2);
  uVar1 = (((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_020732ec + 0x1a)) >> 4) <<
           6 | (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_020732ec + 0x18)) >> 4)
          & 0xffff;
  *(uint *)(param_1 + 0x7c) = iVar4 + 4 + uVar1 * 2;
  *(undefined2 *)(param_1 + 0x78) = uVar2;
  FUN_0208366c(uVar2,uVar1,*(undefined2 *)(param_1 + 0x50));
  return;
}

/* ================================================================
 * FUN_020732f4 @ 020732f4
 * ================================================================ */

void FUN_020732f4(int param_1)

{
  byte bVar1;
  int iVar2;

  if (*(short *)(param_1 + 0x78) != **(short **)(param_1 + 0x7c)) {
    iVar2 = FUN_020769d4(param_1,*(undefined1 *)(DAT_0207338c + (uint)*(byte *)(param_1 + 10)));
    if (iVar2 != 0) {
      *(undefined1 *)(iVar2 + 0xb) = 4;
      *(short *)(iVar2 + 0x36) = *(short *)(iVar2 + 0x36) + 8;
      *(short *)(iVar2 + 0x3a) = *(short *)(iVar2 + 0x3a) + -8;
      *(undefined1 *)(iVar2 + 0x2b) = 0x90;
    }
    FUN_0204be88(param_1);
    return;
  }
  FUN_02085980(param_1);
  if ((*(byte *)(param_1 + 0x29) & 0x80) == 0) {
    return;
  }
  FUN_020859e0(param_1,0);
  bVar1 = FUN_0208455c();
  *(byte *)(param_1 + 0x28) = (bVar1 & 0xf0) + 1;
  return;
}

/* ================================================================
 * FUN_020733c8 @ 020733c8
 * ================================================================ */

void FUN_020733c8(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 *puVar1;
  int iVar2;

  FUN_020742cc(param_1);
  iVar2 = DAT_02073490;
  *(undefined1 *)(DAT_02073490 + 0x30) = 0xff;
  *(undefined1 *)(iVar2 + 0x31) = 0xff;
  *(undefined1 *)(iVar2 + 0x32) = 0;
  *(undefined1 *)(iVar2 + 0x34) = 0xff;
  *(undefined1 *)(iVar2 + 0x35) = 0xff;
  *(undefined1 *)(iVar2 + 0x36) = 0;
  puVar1 = DAT_02073494;
  *(undefined1 *)(param_1 + 0x17) = 0;
  *(undefined2 *)(param_1 + 0x7c) = 0x122;
  *puVar1 = 0;
  if (*(char *)(param_1 + 10) == '\v') {
    FUN_02029b38(*(undefined2 *)(DAT_0207349c + (uint)*(byte *)(DAT_02073498 + 1) * 2));
    return;
  }
  iVar2 = FUN_02081b64();
  if (iVar2 != 0) {
    iVar2 = FUN_02076858(0x58,0xb,0);
    if (iVar2 == 0) {
      return;
    }
    FUN_02050390(param_1,iVar2,0,0,param_4);
    FUN_0204be88(param_1);
    return;
  }
  FUN_02029b38(*(undefined2 *)(DAT_020734a0 + (uint)*(byte *)(DAT_02073498 + 1) * 2));
  return;
}

/* ================================================================
 * FUN_020734a4 @ 020734a4
 * ================================================================ */

void FUN_020734a4(int param_1)

{
  int iVar1;
  uint uVar2;

  if ((*DAT_0207357c & 0x10) == 0) {
    return;
  }
  if (*(short *)(param_1 + 0x7c) != 0) {
    *(short *)(param_1 + 0x7c) = *(short *)(param_1 + 0x7c) + -1;
  }
  uVar2 = (uint)*(ushort *)(param_1 + 0x7c);
  if (uVar2 < 0xab) {
    if (uVar2 < 0xaa) {
      if (uVar2 < 0x83) {
        if (uVar2 < 0x82) {
          if (uVar2 == 0) {
            *(undefined1 *)(param_1 + 0x14) = 2;
          }
          return;
        }
      }
      else if (uVar2 != 0x96) {
        return;
      }
    }
  }
  else if (uVar2 < 0x105) {
    if (0x103 < uVar2) {
      return;
    }
    if (uVar2 != 0xd2) {
      return;
    }
  }
  else if (uVar2 != DAT_02073580) {
    return;
  }
  iVar1 = FUN_02076858(0x58,6,0);
  if (iVar1 != 0) {
    FUN_02050390(param_1,iVar1,
                 (int)*(short *)(DAT_02073584 + (uint)*(byte *)(param_1 + 0x17) * 2) << 0x10,
                 (int)*(short *)(DAT_02073584 + (*(byte *)(param_1 + 0x17) + 1) * 2) << 0x10);
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + '\x02';
    return;
  }
  return;
}

/* ================================================================
 * FUN_02073588 @ 02073588
 * ================================================================ */

void FUN_02073588(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;

  iVar1 = FUN_02076858(0x58,7,0,param_4,param_4);
  if (iVar1 == 0) {
    return;
  }
  FUN_02050390(param_1,iVar1,0,0x80000);
  *(undefined1 *)(param_1 + 0x14) = 3;
  return;
}

/* ================================================================
 * FUN_020735c8 @ 020735c8
 * ================================================================ */

void FUN_020735c8(int param_1)

{
  int iVar1;

  if ((*DAT_0207363c & 1) == 0) {
    return;
  }
  iVar1 = FUN_02076858(0x58,2,0);
  if (iVar1 == 0) {
    return;
  }
  FUN_0205031c(param_1);
  FUN_0204625c(2,4);
  FUN_02076fe0(0x218);
  *(undefined1 *)(param_1 + 0x14) = 4;
  *(undefined1 *)(param_1 + 0x17) = 0x3c;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  return;
}

/* ================================================================
 * FUN_02073640 @ 02073640
 * ================================================================ */

void FUN_02073640(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;

  FUN_02085980(param_1);
  if (*(char *)(param_1 + 0x17) != '\0') {
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
    return;
  }
  iVar1 = FUN_02081b64();
  if (iVar1 == 0) {
    iVar1 = FUN_02076858(0x58,1,0);
    if (iVar1 != 0) {
      FUN_02050390(param_1,iVar1,0,0xffec0000,param_4);
      *(undefined1 *)(param_1 + 0x14) = 5;
      *(undefined1 *)(param_1 + 0x17) = 0x78;
      *(undefined1 *)(param_1 + 0x18) = 0;
      return;
    }
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 5;
  *(undefined1 *)(param_1 + 0x17) = 0x78;
  *(undefined1 *)(param_1 + 0x18) = 0;
  return;
}

/* ================================================================
 * FUN_020736cc @ 020736cc
 * ================================================================ */

void FUN_020736cc(int param_1)

{
  int iVar1;

  if (*(char *)(param_1 + 0x17) == '\0') {
    if (((*(char *)(param_1 + 0x18) == '\0') && ((*DAT_0207377c & 0x40) != 0)) &&
       (iVar1 = FUN_02076858(0x58,9,0), iVar1 != 0)) {
      FUN_02050390(param_1,iVar1,0,0xffb40000);
      *(int *)(iVar1 + 0x54) = param_1;
      *(undefined1 *)(param_1 + 0x18) = 1;
    }
    if ((*(short *)(param_1 + 0x7c) == 0) && (iVar1 = FUN_02081b64(), iVar1 != 0)) {
      *DAT_0207377c = *DAT_0207377c | 0x20;
      *(undefined2 *)(param_1 + 0x7c) = 1;
    }
  }
  else {
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
  }
  FUN_02085980(param_1);
  return;
}

/* ================================================================
 * FUN_0207379c @ 0207379c
 * ================================================================ */

void FUN_0207379c(int param_1)

{
  int *piVar1;
  undefined2 *puVar2;

  FUN_020742cc(param_1);
  piVar1 = DAT_02073820;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 1;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffff3fc | 0x401;
  *(undefined2 *)(*piVar1 + 0x66) = 0xf40;
  *(short *)(*piVar1 + 0x68) = (short)DAT_02073824;
  FUN_02085cf8(param_1);
  puVar2 = DAT_02073828;
  *(undefined4 *)(param_1 + 0xc) = 0x400;
  *puVar2 = 0x400;
  puVar2[1] = 0x100;
  puVar2[2] = 0;
  FUN_0205063c(puVar2,param_1);
  return;
}

/* ================================================================
 * FUN_0207382c @ 0207382c
 * ================================================================ */

void FUN_0207382c(int param_1)

{
  undefined2 *puVar1;
  int iVar2;

  iVar2 = *(int *)(param_1 + 0xc) + -0x20;
  *(int *)(param_1 + 0xc) = iVar2;
  puVar1 = DAT_0207388c;
  if (iVar2 == 0x100) {
    *(undefined1 *)(param_1 + 0x14) = 2;
    FUN_02085d34(param_1);
    *DAT_02073888 = *DAT_02073888 | 0x20;
    return;
  }
  *DAT_0207388c = (short)iVar2;
  puVar1[1] = 0x100;
  puVar1[2] = 0;
  FUN_0205063c(puVar1,param_1);
  return;
}

/* ================================================================
 * FUN_02073894 @ 02073894
 * ================================================================ */

void FUN_02073894(int param_1)

{
  (**(code **)(DAT_020738ac + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_020738b0 @ 020738b0
 * ================================================================ */

void FUN_020738b0(int param_1)

{
  FUN_020742cc(param_1);
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc;
  return;
}

/* ================================================================
 * FUN_02073904 @ 02073904
 * ================================================================ */

void FUN_02073904(int param_1)

{
  int iVar1;

  iVar1 = FUN_02076858(0x58,4,0);
  if (iVar1 != 0) {
    FUN_0205031c(param_1,iVar1);
    *(int *)(iVar1 + 0x54) = param_1;
    FUN_020742cc(param_1);
    *(undefined1 *)(param_1 + 0x18) = 0;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  }
  iVar1 = FUN_02081b64();
  if (iVar1 == 0) {
    return;
  }
  *(uint *)(param_1 + 0x1c) =
       *(uint *)(param_1 + 0x1c) & 0xfff0ffff |
       (((*(uint *)(param_1 + 0x1c) & 0xfffff) >> 0x10) + 1 & 0xf) << 0x10;
  return;
}

/* ================================================================
 * FUN_02073984 @ 02073984
 * ================================================================ */

void FUN_02073984(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  short sVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;

  FUN_02085980(param_1);
  *(int *)(param_1 + 0x38) = *(int *)(param_1 + 0x38) + -0x8000;
  sVar1 = *(short *)(param_1 + 0x3a);
  if (sVar1 == -0x14) {
    *(undefined1 *)(param_1 + 0x14) = 2;
    iVar2 = FUN_02081b64();
    if (iVar2 == 0) {
      uVar3 = 0x214;
    }
    else {
      uVar3 = 0x23a;
    }
    FUN_02076fe0(uVar3);
    return;
  }
  uVar4 = 0xfffffff6;
  if (sVar1 == -10) {
    uVar4 = (uint)*(byte *)(param_1 + 0x18);
  }
  if (sVar1 == -10 && uVar4 == 0) {
    iVar2 = FUN_02076858(0x58,5,0);
    if (iVar2 != 0) {
      FUN_02050390(param_1,iVar2,0,0x40000,param_4);
      *(undefined1 *)(param_1 + 0x18) = 1;
      return;
    }
    return;
  }
  return;
}

/* ================================================================
 * FUN_02073a18 @ 02073a18
 * ================================================================ */

void FUN_02073a18(undefined4 param_1)

{
  FUN_02085980(param_1);
  FUN_020742e4(param_1);
  if ((*DAT_02073a48 & 1) == 0) {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02073a68 @ 02073a68
 * ================================================================ */

void FUN_02073a68(int param_1)

{
  int iVar1;

  FUN_020742cc(param_1);
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  iVar1 = FUN_02081b64();
  if (iVar1 == 0) {
    return;
  }
  *(uint *)(param_1 + 0x1c) =
       *(uint *)(param_1 + 0x1c) & 0xfff0ffff |
       (((*(uint *)(param_1 + 0x1c) & 0xfffff) >> 0x10) + 1 & 0xf) << 0x10;
  return;
}

/* ================================================================
 * FUN_02073ac0 @ 02073ac0
 * ================================================================ */

void FUN_02073ac0(int param_1)

{
  undefined2 *puVar1;

  FUN_0205031c(*(undefined4 *)(param_1 + 0x54),param_1);
  if (*(short *)(param_1 + 0x3a) != -0x14) {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 2;
  *(undefined1 *)(param_1 + 0x17) = 0x5a;
  FUN_02085cf8(param_1);
  puVar1 = DAT_02073b44;
  *(uint *)(param_1 + 0x1c) =
       *(uint *)(param_1 + 0x1c) & 0xfffffcff | ((*(uint *)(param_1 + 0x1c) & 0x3ff) >> 8 | 2) << 8;
  *(undefined4 *)(param_1 + 0xc) = 0x1000000;
  *puVar1 = 0x100;
  puVar1[1] = 0x100;
  puVar1[2] = 0;
  FUN_0205063c(puVar1,param_1);
  return;
}

/* ================================================================
 * FUN_02073b48 @ 02073b48
 * ================================================================ */

void FUN_02073b48(int param_1)

{
  undefined2 *puVar1;
  char cVar2;
  int iVar3;

  cVar2 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar2;
  puVar1 = DAT_02073bb8;
  if (cVar2 == '\0') {
    *DAT_02073bb4 = *DAT_02073bb4 | 1;
    *(undefined1 *)(param_1 + 0x14) = 3;
    FUN_02085d34(param_1);
    return;
  }
  iVar3 = *(int *)(param_1 + 0xc) + -0x18000;
  *(int *)(param_1 + 0xc) = iVar3;
  *puVar1 = 0x100;
  puVar1[1] = (short)((uint)iVar3 >> 0x10);
  puVar1[2] = 0;
  FUN_0205063c(puVar1,param_1);
  return;
}

/* ================================================================
 * FUN_02073bd8 @ 02073bd8
 * ================================================================ */

void FUN_02073bd8(int param_1)

{
  undefined4 uVar1;

  FUN_020742cc(param_1);
  uVar1 = DAT_02073c18;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(undefined2 *)(param_1 + 0x3a) = 0;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 1;
  FUN_02076fe0(uVar1);
  return;
}

/* ================================================================
 * FUN_02073c40 @ 02073c40
 * ================================================================ */

void FUN_02073c40(int param_1)

{
  (**(code **)(DAT_02073c58 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_02073c5c @ 02073c5c
 * ================================================================ */

void FUN_02073c5c(int param_1)

{
  FUN_020742cc(param_1);
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc;
  return;
}

/* ================================================================
 * FUN_02073c88 @ 02073c88
 * ================================================================ */

void FUN_02073c88(void)

{
  if ((*DAT_02073cac & 2) != 0) {
    FUN_0204be88();
    return;
  }
  FUN_02085980();
  return;
}

/* ================================================================
 * FUN_02073ccc @ 02073ccc
 * ================================================================ */

void FUN_02073ccc(int param_1)

{
  int iVar1;

  FUN_020742cc(param_1);
  *(undefined1 *)(param_1 + 0x17) = 0x78;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 1;
  iVar1 = FUN_02081b64();
  if (iVar1 != 0) {
    *(uint *)(param_1 + 0x1c) =
         *(uint *)(param_1 + 0x1c) & 0xfff0ffff |
         (((*(uint *)(param_1 + 0x1c) & 0xfffff) >> 0x10) + 1 & 0xf) << 0x10;
  }
  FUN_02076fe0(DAT_02073d38);
  return;
}

/* ================================================================
 * FUN_02073d3c @ 02073d3c
 * ================================================================ */

void FUN_02073d3c(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;

  FUN_02085980(param_1);
  if (*(char *)(param_1 + 0x17) != '\0') {
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
    return;
  }
  iVar1 = FUN_02076858(0x58,3,0);
  if (iVar1 != 0) {
    FUN_02050390(param_1,iVar1,0,0xfff80000,param_4);
    *DAT_02073da4 = *DAT_02073da4 | 2;
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02073dc4 @ 02073dc4
 * ================================================================ */

void FUN_02073dc4(int param_1)

{
  FUN_020742cc(param_1);
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 1;
  return;
}

/* ================================================================
 * FUN_02073df4 @ 02073df4
 * ================================================================ */

void FUN_02073df4(int param_1)

{
  FUN_02085980(param_1);
  if ((*(byte *)(param_1 + 0x29) & 0x80) == 0) {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02073e18 @ 02073e18
 * ================================================================ */

void FUN_02073e18(int param_1)

{
  if (*(char *)(param_1 + 0xb) == '\0') {
    (**(code **)(DAT_02073e58 + (uint)*(byte *)(param_1 + 0x14) * 4))();
    return;
  }
  (**(code **)(DAT_02073e5c + (uint)*(byte *)(param_1 + 0x14) * 4))();
  FUN_02085980(param_1);
  return;
}

/* ================================================================
 * FUN_02073e60 @ 02073e60
 * ================================================================ */

void FUN_02073e60(int param_1)

{
  uint uVar1;

  FUN_020742cc(param_1);
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 3;
  *(undefined4 *)(param_1 + 0xc) = 0x80;
  *(undefined1 *)(param_1 + 0x2b) = 0x10;
  uVar1 = FUN_02049020();
  if (uVar1 == 3) {
    uVar1 = *(byte *)(DAT_02073ef4 + 1) + 3 & 0xff;
  }
  uVar1 = *(ushort *)(DAT_02073ef8 + uVar1 * 2) & 0xf;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xff00ffff | uVar1 << 0x14 | uVar1 << 0x10
  ;
  return;
}
