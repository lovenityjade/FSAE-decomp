#include "game/arm9_middle_020754ac_raw.h"

/*
 * Raw matching-first recovery for the 32 catalog entries beginning at
 * 0x020754ac. These bodies remain address-named promotion candidates and are
 * not linked byte-matching evidence.
 *
 * Calls whose implicit register arguments were omitted by Ghidra are written
 * with the live recovered values so this mechanically faithful draft remains
 * valid C.
 */

extern uint DAT_02075524;
extern uint DAT_020755a0;
extern uint DAT_020756d0;
extern uint *DAT_020756d4;
extern uint *DAT_02075788;
extern uint *DAT_020757f8;
extern uint *DAT_02075838;
extern uint DAT_02075974;
extern uint DAT_02075978;
extern uint *DAT_0207597c;
extern uint *DAT_02075980;
extern uint DAT_02075a58;
extern uint *DAT_02075a5c;
extern int *DAT_02075ae8;
extern uint DAT_02075b54;
extern byte *DAT_02075c0c;
extern uint DAT_02075c10;
extern byte *DAT_02075c94;
extern uint DAT_02075c98;
extern uint DAT_02075c9c;
extern byte *DAT_02075d44;
extern uint *DAT_02075d48;
extern uint DAT_02075d4c;
extern byte *DAT_02075e00;
extern uint *DAT_02075e04;
extern uint DAT_02075e08;
extern uint *DAT_02075ee8;
extern uint DAT_02075eec;
extern byte *DAT_02076038;
extern byte *DAT_0207603c;
extern uint DAT_02076040;
extern uint DAT_02076090;
extern uint DAT_020760f4;
extern uint *DAT_020760f8;
extern undefined4 *DAT_0207614c;
extern uint DAT_020761b0;
extern uint DAT_02076238;
extern undefined2 *DAT_020762f0;
extern uint DAT_02076494;
extern undefined2 *DAT_02076498;
extern undefined2 *DAT_02076534;

/* ================================================================
 * FUN_020754ac @ 020754ac
 * ================================================================ */

void FUN_020754ac(int param_1)

{
  FUN_02085980(param_1);
  *(short *)(param_1 + 0x7c) = *(short *)(param_1 + 0x7c) + -1;
  if (*(ushort *)(param_1 + 0x7c) == 0) {
    *(uint *)(*(int *)(param_1 + 0x54) + 0x60) =
         *(uint *)(*(int *)(param_1 + 0x54) + 0x60) ^
         *(uint *)(DAT_02075524 + (uint)*(byte *)(param_1 + 0xb) * 4);
    FUN_0204be88(param_1);
    return;
  }
  if (0x3b < *(ushort *)(param_1 + 0x7c)) {
    return;
  }
  *(uint *)(param_1 + 0x1c) =
       *(uint *)(param_1 + 0x1c) & 0xfffffffc | *(uint *)(param_1 + 0x1c) & 3 ^ 1;
  return;
}

/* ================================================================
 * FUN_02075528 @ 02075528
 * ================================================================ */

void FUN_02075528(int param_1,int param_2)

{
  *(undefined4 *)(param_2 + 0x30) = *(undefined4 *)(param_1 + 0x30);
  *(undefined4 *)(param_2 + 0x34) = *(undefined4 *)(param_1 + 0x34);
  return;
}

/* ================================================================
 * FUN_0207553c @ 0207553c
 * ================================================================ */

bool FUN_0207553c(int param_1)

{
  if ((*(ushort *)(param_1 + 0x36) & DAT_020755a0) != 0) {
    return true;
  }
  if (*(char *)(param_1 + 2) == '\0') {
    if ((*(char *)(param_1 + 0x3c) == '\0') && (*(char *)(*(int *)(param_1 + 0x28) + 0x45) != '\0'))
    {
      return (byte)(*(char *)(*(int *)(param_1 + 0x28) + 0x14) - 2U) < 2;
    }
    return true;
  }
  return true;
}

/* ================================================================
 * FUN_020755c0 @ 020755c0
 * ================================================================ */

void FUN_020755c0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  uint uVar1;
  byte bVar2;

  bVar2 = *(byte *)(param_1 + 0x5e) & 0xfc;
  uVar1 = *(uint *)(param_1 + 0x1c) & 0xffff3ffc | 0xc001;
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(byte *)(param_1 + 0x5e) = bVar2;
  *(uint *)(param_1 + 0x1c) = uVar1;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10),uVar1,bVar2,param_4);
  return;
}

/* ================================================================
 * FUN_0207562c @ 0207562c
 * ================================================================ */

void FUN_0207562c(int param_1)

{
  byte bVar1;

  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x40004001;
    *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 3;
    FUN_020859e0(param_1);
  }
  FUN_02085980(param_1);
  if ((*(byte *)(param_1 + 0x29) & 1) != 0) {
    *(undefined1 *)(param_1 + 0x29) = 0;
    bVar1 = *(byte *)(param_1 + 0x17) ^ 1;
    *(byte *)(param_1 + 0x17) = bVar1;
    if (bVar1 != 0) {
      FUN_02076fe0(DAT_020756d0);
    }
  }
  if ((*DAT_020756d4 & 1) == 0) {
    return;
  }
  FUN_0204be88(param_1);
  return;
}
/* ================================================================
 * FUN_020756f4 @ 020756f4
 * ================================================================ */

void FUN_020756f4(int param_1)

{
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(undefined1 *)(param_1 + 0x18) = *(undefined1 *)(param_1 + 0xb);
  *(undefined4 *)(param_1 + 0xc) = 0x280;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 3;
  FUN_020859e0(param_1,3);
  return;
}

/* ================================================================
 * FUN_0207573c @ 0207573c
 * ================================================================ */

void FUN_0207573c(int param_1)

{
  FUN_02085980(param_1);
  if ((*DAT_02075788 & 2) == 0) {
    return;
  }
  FUN_0204fd40(param_1);
  if (0x104 < *(short *)(param_1 + 0x36)) {
    return;
  }
  *(undefined2 *)(param_1 + 0x36) = 0x104;
  *(undefined1 *)(param_1 + 0x14) = 2;
  *(undefined1 *)(param_1 + 0x17) = 0x1e;
  return;
}

/* ================================================================
 * FUN_0207578c @ 0207578c
 * ================================================================ */

void FUN_0207578c(int param_1)

{
  char cVar1;
  int iVar2;

  FUN_02085980(param_1);
  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 3;
  *DAT_020757f8 = 4;
  iVar2 = (int)*(short *)(param_1 + 0x32);
  if (0x1f8 < iVar2) {
    *(undefined1 *)(param_1 + 0x2b) = 0x18;
    *(int *)(param_1 + 0xc) = (iVar2 + -0x1f8) * 2;
    return;
  }
  *(undefined1 *)(param_1 + 0x2b) = 8;
  *(int *)(param_1 + 0xc) = (0x1f8 - iVar2) * 2;
  return;
}

/* ================================================================
 * FUN_020757fc @ 020757fc
 * ================================================================ */

void FUN_020757fc(int param_1)

{
  FUN_02085980(param_1);
  if (*(int *)(param_1 + 0xc) != 0) {
    FUN_0204fd40(param_1);
  }
  if ((*DAT_02075838 & 8) == 0) {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_0207583c @ 0207583c
 * ================================================================ */

void FUN_0207583c(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  uint *puVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  bool bVar7;

  puVar3 = DAT_0207597c;
  cVar1 = *(char *)(param_1 + 0x14);
  if (cVar1 == '\0') {
    uVar5 = *(uint *)(param_1 + 0x1c);
    uVar4 = (uint)*(byte *)(param_1 + 0xb);
    bVar7 = uVar4 == 0;
    *(uint *)(param_1 + 0x1c) = uVar5 & 0xfffffffc | 1;
    if (bVar7) {
      uVar4 = uVar5 & 0xffff3ffc | 1;
    }
    *(undefined1 *)(param_1 + 0x14) = 1;
    iVar6 = DAT_02075974;
    if (bVar7) {
      *(uint *)(param_1 + 0x1c) = uVar4;
    }
    else {
      *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 3;
      *(uint *)(param_1 + 0x1c) = uVar5 & 0xffff33fc | 0x8401;
      uVar2 = DAT_02075978;
      *(undefined2 *)(iVar6 + 0x66) = 0xf40;
      *(short *)(iVar6 + 0x68) = (short)uVar2;
    }
    FUN_020859e0(param_1,*(undefined1 *)(param_1 + 0xb));
  }
  else if (cVar1 == '\x01') {
    if ((*DAT_0207597c & 1) != 0) {
      if (*(char *)(param_1 + 0xb) == '\0') {
        FUN_02076fe0(0x21c);
      }
      *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
    }
  }
  else if ((cVar1 == '\x02') &&
          (*(uint *)(param_1 + 0x1c) =
                *(uint *)(param_1 + 0x1c) & 0xfffffffc | (uint)((*DAT_02075980 & 4) == 0),
          (*puVar3 & 2) != 0)) {
    FUN_0204be88(param_1);
  }
  iVar6 = *(int *)(param_1 + 0x54);
  *(undefined4 *)(param_1 + 0x30) = *(undefined4 *)(iVar6 + 0x30);
  *(undefined4 *)(param_1 + 0x34) = *(undefined4 *)(iVar6 + 0x34);
  *(undefined4 *)(param_1 + 0x38) = *(undefined4 *)(iVar6 + 0x38);
  FUN_02085980(param_1);
  return;
}

/* ================================================================
 * FUN_02075984 @ 02075984
 * ================================================================ */

void FUN_02075984(int param_1)

{
  char cVar1;
  byte bVar2;
  bool bVar3;

  if (*(char *)(param_1 + 0x14) == '\0') {
    bVar2 = *(byte *)(param_1 + 0xb);
    *(undefined1 *)(param_1 + 0x17) = 0;
    bVar3 = bVar2 == 0;
    if (bVar3) {
      bVar2 = 3;
    }
    if (!bVar3) {
      bVar2 = 2;
    }
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xffff3ffc | 0xc001;
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | bVar2 & 3;
    FUN_020859e0(param_1,*(undefined1 *)(param_1 + 0xb));
  }
  FUN_02085980(param_1);
  cVar1 = *(char *)(param_1 + 0xb);
  bVar3 = cVar1 == '\0';
  if (bVar3) {
    cVar1 = *(char *)(param_1 + 0x29);
  }
  if (bVar3 && cVar1 == '\x01') {
    *(undefined1 *)(param_1 + 0x29) = 0;
    if (*(char *)(param_1 + 0x17) == '\0') {
      FUN_02076fe0(DAT_02075a58);
    }
    *(byte *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + 1U & 3;
  }
  if ((*DAT_02075a5c & 8) != 0) {
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02075a9c @ 02075a9c
 * ================================================================ */

void FUN_02075a9c(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int *piVar1;

  piVar1 = DAT_02075ae8;
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(undefined4 *)(param_1 + 0xc) = 0x180;
  *(undefined1 *)(param_1 + 0x2b) = 0x10;
  *(undefined2 *)(param_1 + 0x32) = 0x1f8;
  *(undefined2 *)(param_1 + 0x36) = 0x148;
  *piVar1 = param_1;
  FUN_020859e0(param_1,0,piVar1,0x1f8,param_4);
  FUN_02076620(param_1);
  return;
}

/* ================================================================
 * FUN_02075b04 @ 02075b04
 * ================================================================ */

void FUN_02075b04(int param_1)

{
  int iVar1;
  int iVar2;
  bool bVar3;

  iVar1 = *(short *)(DAT_02075b54 + 0x20) + 0x80;
  bVar3 = *(short *)(param_1 + 0x32) == iVar1;
  iVar2 = DAT_02075b54;
  if (bVar3) {
    iVar2 = (int)*(short *)(param_1 + 0x36);
    iVar1 = *(short *)(DAT_02075b54 + 0x22) + 0x60;
  }
  if (bVar3 && iVar2 == iVar1) {
    FUN_0204625c(2,2);
    *(undefined1 *)(param_1 + 0x14) = 3;
    *(undefined1 *)(param_1 + 0x17) = 0xf8;
    return;
  }
  return;
}

/* ================================================================
 * FUN_02075b58 @ 02075b58
 * ================================================================ */

void FUN_02075b58(int param_1)

{
  if (*(char *)(param_1 + 0x17) != '\0') {
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
    return;
  }
  FUN_0204fd40();
  if (0x1a7 < *(short *)(param_1 + 0x36)) {
    *(undefined2 *)(param_1 + 0x36) = 0x1a8;
    *(undefined1 *)(param_1 + 0x14) = 4;
    *(undefined1 *)(param_1 + 0x17) = 0x1e;
    return;
  }
  return;
}

/* ================================================================
 * FUN_02075ba0 @ 02075ba0
 * ================================================================ */

void FUN_02075ba0(int param_1)

{
  byte *pbVar1;
  int iVar2;
  char cVar3;
  uint uVar4;

  cVar3 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar3;
  pbVar1 = DAT_02075c0c;
  if (cVar3 != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 5;
  *(undefined1 *)(param_1 + 0x17) = 0x3c;
  iVar2 = DAT_02075c10;
  uVar4 = 0;
  if (*pbVar1 != 0) {
    do {
      *(undefined1 *)(*(int *)(uVar4 * 0x94 + iVar2 + 0x28) + 0x2a) = 4;
      uVar4 = uVar4 + 1 & 0xff;
    } while (uVar4 < *pbVar1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02075c14 @ 02075c14
 * ================================================================ */

void FUN_02075c14(int param_1)

{
  byte bVar1;
  byte *pbVar2;
  int iVar3;
  undefined4 uVar4;
  char cVar5;
  int iVar6;
  uint uVar7;

  cVar5 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar5;
  pbVar2 = DAT_02075c94;
  if (cVar5 != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 6;
  *(undefined1 *)(param_1 + 0x17) = 0x10;
  uVar4 = DAT_02075c9c;
  iVar3 = DAT_02075c98;
  bVar1 = *pbVar2;
  uVar7 = 0;
  if (bVar1 != 0) {
    do {
      iVar6 = uVar7 * 0x94 + iVar3;
      *(undefined1 *)(iVar6 + 0xc) = 0xf;
      *(short *)(iVar6 + 8) = (short)uVar4;
      uVar7 = uVar7 + 1 & 0xff;
      *(char *)(iVar6 + 0x38) = *(char *)(param_1 + 0x17) + -0x6a;
      *(undefined1 *)(iVar6 + 0x39) = 1;
    } while (uVar7 < bVar1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02075ca0 @ 02075ca0
 * ================================================================ */

void FUN_02075ca0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  byte bVar1;
  byte *pbVar2;
  int iVar3;
  int iVar4;
  char cVar5;
  uint uVar6;

  cVar5 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar5;
  pbVar2 = DAT_02075d44;
  if (cVar5 != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 7;
  *(undefined1 *)(param_1 + 0x17) = 0x96;
  bVar1 = *pbVar2;
  uVar6 = 0;
  *DAT_02075d48 = 0;
  iVar3 = DAT_02075d4c;
  if (bVar1 != 0) {
    do {
      iVar4 = FUN_02076858(0x68,0,0);
      if (iVar4 != 0) {
        FUN_02050390(*(undefined4 *)(uVar6 * 0x94 + iVar3 + 0x28),iVar4,0xa0000,0xfff00000,param_4);
      }
      uVar6 = uVar6 + 1 & 0xff;
    } while (uVar6 < *pbVar2);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02075d50 @ 02075d50
 * ================================================================ */

void FUN_02075d50(int param_1)

{
  byte bVar1;
  byte *pbVar2;
  int iVar3;
  int iVar4;
  char cVar5;
  uint uVar6;

  cVar5 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar5;
  pbVar2 = DAT_02075e00;
  if (cVar5 == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 8;
    *(undefined1 *)(param_1 + 0x17) = 0x3c;
    bVar1 = *pbVar2;
    *DAT_02075e04 = 1;
    iVar3 = DAT_02075e08;
    uVar6 = 0;
    if (bVar1 != 0) {
      do {
        iVar4 = FUN_02076858(0x68,1,uVar6);
        if (iVar4 != 0) {
          FUN_02050390(*(undefined4 *)(uVar6 * 0x94 + iVar3 + 0x28),iVar4,0xa0000,0xfff00000);
        }
        uVar6 = uVar6 + 1 & 0xff;
      } while (uVar6 < *pbVar2);
    }
    FUN_02076fe0(0x21c);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02075e44 @ 02075e44
 * ================================================================ */

void FUN_02075e44(int param_1)

{
  FUN_0204fd40();
  if (0x118 < *(short *)(param_1 + 0x36)) {
    return;
  }
  *(undefined2 *)(param_1 + 0x36) = 0x118;
  *(undefined1 *)(param_1 + 0x14) = 10;
  *(undefined1 *)(param_1 + 0x17) = 100;
  return;
}

/* ================================================================
 * FUN_02075e78 @ 02075e78
 * ================================================================ */

void FUN_02075e78(int param_1)

{
  uint *puVar1;
  undefined4 uVar2;

  puVar1 = DAT_02075ee8;
  if ((*DAT_02075ee8 & 4) == 0) {
    return;
  }
  if (*(char *)(param_1 + 0x17) == '\0') {
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xffff3ffc | 0x4001;
    *(undefined1 *)(param_1 + 0x14) = 0xb;
    *(undefined1 *)(param_1 + 0x17) = 0x78;
    uVar2 = DAT_02075eec;
    *puVar1 = 8;
    FUN_02076fe0(uVar2);
    FUN_0204625c(2,4);
    return;
  }
  *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
  return;
}

/* ================================================================
 * FUN_02075ef0 @ 02075ef0
 * ================================================================ */

void FUN_02075ef0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;
  byte *pbVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;

  FUN_02085980(param_1);
  cVar1 = *(char *)(param_1 + 0x17);
  if (cVar1 == '\0') {
    if (*(char *)(param_1 + 0x15) == '\0') {
      if ((*DAT_02076038 & 0x7f) != 6) {
        return;
      }
      FUN_02049eb8();
      *(undefined1 *)(param_1 + 0x15) = 1;
      return;
    }
    uVar4 = 0;
    uVar5 = (uint)*DAT_0207603c;
    if (uVar5 != 0) {
      do {
        if ((*(ushort *)(DAT_02076040 + uVar4 * 2) & 0x1000) != 0) {
          return;
        }
        uVar4 = uVar4 + 1 & 0xff;
      } while (uVar4 < uVar5);
    }
    iVar3 = FUN_02076858(0x69,1,0,uVar5,param_4);
    if (iVar3 == 0) {
      *(undefined1 *)(param_1 + 0x17) = 1;
      return;
    }
    FUN_02050390(param_1,iVar3,0,0x300000);
    *(int *)(param_1 + 0x58) = iVar3;
    FUN_0204be88(*(undefined4 *)(param_1 + 0x54));
    FUN_02076fe0(0xb0);
    *(undefined1 *)(param_1 + 0x14) = 0xc;
    return;
  }
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  if (cVar1 != '\x01') {
    return;
  }
  *(undefined1 *)(param_1 + 0x15) = 0;
  FUN_02049f0c(500);
  pbVar2 = DAT_02076038;
  DAT_02076038[3] = DAT_02076038[3] & 0xe0 | 1;
  pbVar2[4] = pbVar2[4] & 0xe0 | 7;
  iVar3 = FUN_02076858(0x68,0,0,pbVar2,param_4);
  if (iVar3 == 0) {
    return;
  }
  FUN_02050390(param_1,iVar3,0,0xffdc0000);
  return;
}

/* ================================================================
 * FUN_02076044 @ 02076044
 * ================================================================ */

void FUN_02076044(int param_1)

{
  FUN_02085980(param_1);
  if (*(char *)(*(int *)(param_1 + 0x58) + 0x45) != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 0xd;
  *(undefined1 *)(param_1 + 0x17) = 8;
  FUN_02076fe0(0x21f);
  *(undefined1 *)(DAT_02076090 + 7) = 0x16;
  FUN_020463f0(0x100);
  return;
}

/* ================================================================
 * FUN_02076094 @ 02076094
 * ================================================================ */

void FUN_02076094(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;

  cVar1 = *(char *)(param_1 + 0x17);
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  if (cVar1 == '\x06') {
    FUN_02021ec4(DAT_020760f4,1);
    *DAT_020760f8 = 0;
    return;
  }
  if (cVar1 != '\x01') {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 0xe;
  *(undefined1 *)(param_1 + 0x17) = 0x7c;
  FUN_0204625c(2,4,0xe,param_4,param_4);
  return;
}

/* ================================================================
 * FUN_020760fc @ 020760fc
 * ================================================================ */

void FUN_020760fc(int param_1)

{
  undefined4 *puVar1;
  char cVar2;

  FUN_02085980(param_1);
  cVar2 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar2;
  puVar1 = DAT_0207614c;
  if (cVar2 != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 0xf;
  *(undefined1 *)(param_1 + 0x17) = 0x5a;
  *(undefined4 *)(param_1 + 0xc) = 0x400;
  *(undefined1 *)(param_1 + 0x2b) = 0x10;
  *puVar1 = 1;
  return;
}

/* ================================================================
 * FUN_02076150 @ 02076150
 * ================================================================ */

void FUN_02076150(int param_1)

{
  FUN_02085980(param_1);
  if (*(char *)(param_1 + 0x17) != '\0') {
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
    return;
  }
  FUN_0204fd40(param_1);
  if (DAT_020761b0 <= *(short *)(param_1 + 0x36)) {
    *(undefined2 *)(param_1 + 0x36) = 0x1f8;
    *(undefined1 *)(param_1 + 0x14) = 0x10;
    FUN_020859e0(param_1,1);
    FUN_02076fe0(0x226);
    return;
  }
  return;
}

/* ================================================================
 * FUN_020761b4 @ 020761b4
 * ================================================================ */

void FUN_020761b4(int param_1)

{
  FUN_02085980(param_1);
  *(undefined1 *)(param_1 + 0x14) = 0x11;
  *(undefined1 *)(param_1 + 0x17) = 0x78;
  FUN_020859e0(param_1,2);
  return;
}

/* ================================================================
 * FUN_020761e0 @ 020761e0
 * ================================================================ */

void FUN_020761e0(int param_1)

{
  char cVar1;

  FUN_02085980(param_1);
  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  FUN_0204625c(1,8);
  FUN_02082ee0(9);
  FUN_020827a8(10);
  return;
}

/* ================================================================
 * FUN_02076220 @ 02076220
 * ================================================================ */

void FUN_02076220(int param_1)

{
  (**(code **)(DAT_02076238 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0207623c @ 0207623c
 * ================================================================ */

void FUN_0207623c(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined2 *puVar1;
  int iVar2;

  iVar2 = FUN_02076858(0x69,2,0,param_4,param_4);
  if (iVar2 == 0) {
    return;
  }
  *(int *)(iVar2 + 0x54) = param_1;
  FUN_0205031c(param_1);
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(undefined1 *)(param_1 + 0x17) = 8;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(undefined1 *)(param_1 + 0x45) = 0xff;
  FUN_020859e0(param_1,0);
  FUN_02085cf8(param_1);
  puVar1 = DAT_020762f0;
  *(uint *)(param_1 + 0x1c) =
       *(uint *)(param_1 + 0x1c) & 0xfffffcff | ((*(uint *)(param_1 + 0x1c) & 0x3ff) >> 8 | 2) << 8;
  *(undefined2 *)(param_1 + 0x7c) = 0x100;
  *(undefined2 *)(param_1 + 0x7e) = 0x100;
  *puVar1 = 0x100;
  puVar1[1] = 0x100;
  puVar1[2] = 0;
  FUN_0205063c(puVar1,param_1);
  return;
}

/* ================================================================
 * FUN_020762f4 @ 020762f4
 * ================================================================ */

void FUN_020762f4(int param_1)

{
  char cVar1;
  int iVar2;

  FUN_02085980(param_1);
  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 2;
  *(undefined1 *)(param_1 + 0x17) = 0xf0;
  *(undefined2 *)(param_1 + 0x78) = 0x78;
  iVar2 = FUN_02076858(0x69,3,0);
  if (iVar2 == 0) {
    return;
  }
  *(undefined1 *)(iVar2 + 0x17) = *(undefined1 *)(param_1 + 0x17);
  *(int *)(iVar2 + 0x54) = param_1;
  FUN_0205031c(param_1);
  return;
}

/* ================================================================
 * FUN_0207635c @ 0207635c
 * ================================================================ */

void FUN_0207635c(int param_1)

{
  char cVar1;
  undefined2 *puVar2;

  cVar1 = *(char *)(param_1 + 0x17);
  if (cVar1 != '\0') {
    *(char *)(param_1 + 0x17) = cVar1 + -1;
    if (cVar1 == '\x01') {
      FUN_02076fe0(0xb1);
      FUN_02076fe0(0x81);
    }
    if ((*(byte *)(param_1 + 0x17) & 1) != 0) {
      *(short *)(param_1 + 0x36) =
           *(short *)(param_1 + 0x36) +
           (short)*(char *)(DAT_02076494 + ((int)(uint)*(byte *)(param_1 + 0x17) >> 1 & 1U));
    }
    if ((*(byte *)(param_1 + 0x17) & 0xf) == 0) {
      *(short *)(param_1 + 0x7e) = *(short *)(param_1 + 0x7e) + -1;
    }
    puVar2 = DAT_02076498;
    *DAT_02076498 = *(undefined2 *)(param_1 + 0x7c);
    puVar2[1] = *(undefined2 *)(param_1 + 0x7e);
    puVar2[2] = 0;
    FUN_0205063c(puVar2,param_1);
    return;
  }
  *(short *)(param_1 + 0x78) = *(short *)(param_1 + 0x78) + -1;
  if (*(ushort *)(param_1 + 0x78) == 0) {
    FUN_02085d34(param_1);
    FUN_0204be88(param_1);
    return;
  }
  if (*(ushort *)(param_1 + 0x78) < 0x1e) {
    *(uint *)(param_1 + 0x1c) =
         *(uint *)(param_1 + 0x1c) & 0xfffffffc | *(uint *)(param_1 + 0x1c) & 3 ^ 1;
  }
  *(short *)(param_1 + 0x7c) = *(short *)(param_1 + 0x7c) + 8;
  *(short *)(param_1 + 0x7e) = *(short *)(param_1 + 0x7e) + -1;
  puVar2 = DAT_02076498;
  *(int *)(param_1 + 0x34) = *(int *)(param_1 + 0x34) + -0x1999;
  *puVar2 = *(undefined2 *)(param_1 + 0x7c);
  puVar2[1] = *(undefined2 *)(param_1 + 0x7e);
  puVar2[2] = 0;
  FUN_0205063c(puVar2,param_1);
  return;
}

/* ================================================================
 * FUN_0207649c @ 0207649c
 * ================================================================ */

void FUN_0207649c(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined2 *puVar1;

  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
    FUN_020859e0(param_1);
    FUN_02085cf8(param_1);
  }
  if (*(char *)(*(int *)(param_1 + 0x54) + 0x45) != '\0') {
    FUN_02050390(*(int *)(param_1 + 0x54),param_1,0,1,param_4);
    puVar1 = DAT_02076534;
    *DAT_02076534 = *(undefined2 *)(*(int *)(param_1 + 0x54) + 0x7c);
    puVar1[1] = 0x100;
    puVar1[2] = 0;
    FUN_0205063c(puVar1,param_1);
    return;
  }
  FUN_02085d34(param_1);
  FUN_0204be88(param_1);
  return;
}
