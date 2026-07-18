#include "game/arm9_middle_0206c850_raw.h"

/*
 * Raw matching-first recovery for the 32 catalog entries beginning at
 * 0x0206c850. These bodies remain address-named promotion candidates and are
 * not linked byte-matching evidence.
 *
 * Calls whose implicit r0 argument was omitted by Ghidra are written with the
 * owning param_1 so this mechanically faithful draft remains valid C.
 */

extern uint DAT_0206c890;
extern uint DAT_0206ca18;
extern uint DAT_0206ca1c;
extern uint *DAT_0206cb18;
extern uint DAT_0206ce9c;
extern uint DAT_0206cef4;
extern uint DAT_0206cf6c;
extern uint DAT_0206cf70;
extern byte *DAT_0206d048;
extern uint DAT_0206d148;
extern uint DAT_0206d14c;
extern uint DAT_0206d150;
extern uint DAT_0206d2cc;
extern uint DAT_0206d2d0;
extern uint DAT_0206d2d4;
extern uint DAT_0206d2d8;
extern uint DAT_0206d2dc;
extern uint DAT_0206d34c;
extern uint DAT_0206d3c8;
extern uint DAT_0206d3cc;
extern uint DAT_0206d414;
extern uint DAT_0206d4cc;
extern uint DAT_0206d580;
extern uint DAT_0206d5dc;
extern uint DAT_0206d60c;
extern uint DAT_0206d6c8;
extern uint DAT_0206d71c;
extern uint DAT_0206d79c;
extern uint DAT_0206d7d8;
extern uint PTR_DAT_0206d04c;
extern undefined *PTR_DAT_0206d050;
extern uint PTR_DAT_0206d05c;
extern uint PTR_PTR_0206d054;
extern uint PTR_u_X0YQ00000000YQX_0206d058;

/* ================================================================
 * FUN_0206c850 @ 0206c850
 * ================================================================ */

void FUN_0206c850(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  if (*(char *)(param_1 + 0x17) == '\0') {
    param_3 = 4;
    *(undefined1 *)(DAT_0206c890 + 0x6e) = 4;
  }
  else {
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
  }
  if (*(char *)(param_1 + 10) != '\0') {
    FUN_02085980(param_1);
    return;
  }
  FUN_02085934(param_1,3,param_3,param_4,param_4);
  return;
}

/* ================================================================
 * FUN_0206c8b0 @ 0206c8b0
 * ================================================================ */

void FUN_0206c8b0(int param_1)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  int iVar4;
  undefined4 uVar5;
  uint uVar6;
  uint uVar7;
  bool bVar8;

  *(ushort *)(param_1 + 0x66) = (ushort)*(byte *)(param_1 + 0x17) * 0x3c;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(undefined2 *)(param_1 + 100) = *(undefined2 *)(param_1 + 0x66);
  *(undefined1 *)(param_1 + 0x17) = 0x10;
  *(undefined1 *)(param_1 + 0x18) = 0;
  iVar4 = FUN_02056f30(*(undefined2 *)(param_1 + 0x50));
  uVar1 = *(ushort *)(DAT_0206ca18 + 0x18);
  uVar2 = *(ushort *)(DAT_0206ca18 + 0x1a);
  *(ushort *)(param_1 + 0x70) = (ushort)((int)(uint)*(byte *)(param_1 + 10) >> 4) & 3;
  uVar3 = *(ushort *)(DAT_0206ca1c + (uint)*(ushort *)(param_1 + 0x70) * 2);
  iVar4 = iVar4 + 0x2004 +
          (((int)((int)*(short *)(param_1 + 0x36) - (uint)uVar2) >> 4) << 6 |
          (int)((int)*(short *)(param_1 + 0x32) - (uint)uVar1) >> 4);
  *(int *)(param_1 + 0x74) = iVar4;
  bVar8 = (*(byte *)(param_1 + 10) & 8) == 0;
  if (bVar8) {
    iVar4 = 2;
  }
  *(uint *)(param_1 + 0xc) = (uint)uVar3;
  if (bVar8) {
    *(char *)(param_1 + 0x14) = (char)iVar4;
  }
  else {
    *(undefined1 *)(param_1 + 0x14) = 1;
    uVar5 = FUN_020460e8(*(undefined1 *)(param_1 + 0xb));
    *(undefined4 *)(param_1 + 0x60) = uVar5;
  }
  uVar6 = (uint)*(byte *)(param_1 + 10);
  uVar7 = uVar6 & 3;
  *(char *)(param_1 + 10) = (char)uVar7;
  *(byte *)(param_1 + 0x68) = (byte)((int)uVar6 >> 6) & 1;
  *(byte *)(param_1 + 0x72) = (byte)((int)uVar6 >> 2) & 1;
  *(char *)(param_1 + 0x2b) = (char)((uVar7 ^ 2) << 3);
  FUN_020859e0(param_1,uVar7 << 2);
  switch(*(undefined1 *)(param_1 + 10)) {
  case 0:
    break;
  case 1:
    goto LAB_0206c9f8;
  case 2:
    break;
  case 3:
LAB_0206c9f8:
    *(undefined2 *)(param_1 + 0x78) = *(undefined2 *)(param_1 + 0x32);
    *(short *)(param_1 + 0x7a) = *(short *)(param_1 + 0x36) + -0x10;
    *(undefined2 *)(param_1 + 0x7e) = 0x20;
    return;
  default:
    return;
  }
  *(short *)(param_1 + 0x78) = *(short *)(param_1 + 0x32) + -0x10;
  *(undefined2 *)(param_1 + 0x7a) = *(undefined2 *)(param_1 + 0x36);
  *(undefined2 *)(param_1 + 0x7c) = 0x20;
  return;
}

/* ================================================================
 * FUN_0206ca20 @ 0206ca20
 * ================================================================ */

void FUN_0206ca20(int param_1)

{
  char cVar1;
  int iVar2;

  iVar2 = FUN_0206cea0(param_1);
  cVar1 = '\0';
  if (iVar2 != 0) {
    cVar1 = *(char *)(param_1 + 0x68);
    *(undefined1 *)(param_1 + 0x14) = 2;
  }
  if (iVar2 != 0 && cVar1 != '\0') {
    *(undefined4 *)(param_1 + 0x60) = 0;
  }
  return;
}

/* ================================================================
 * FUN_0206ca4c @ 0206ca4c
 * ================================================================ */

void FUN_0206ca4c(int param_1)

{
  char cVar1;
  ushort uVar2;

  if (*(short *)(param_1 + 100) == 0) {
    cVar1 = *(char *)(param_1 + 0x17);
    *(char *)(param_1 + 0x17) = cVar1 + -1;
    if (cVar1 == '\x01') {
      *(undefined1 *)(param_1 + 0x17) = 0x10;
      if ((ushort)*(byte *)(param_1 + 0x18) < *(ushort *)(param_1 + 0x70)) {
        uVar2 = *(byte *)(param_1 + 0x18) + 1;
        *(char *)(param_1 + 0x18) = (char)uVar2;
        FUN_020859e0(param_1,(uVar2 & 0xff) + (ushort)*(byte *)(param_1 + 10) * 4 & 0xff);
      }
      else {
        *(undefined1 *)(param_1 + 0x14) = 3;
        *(char *)(param_1 + 0x18) = (char)*(ushort *)(param_1 + 0x70);
      }
    }
    FUN_02085980(param_1);
    return;
  }
  *(short *)(param_1 + 100) = *(short *)(param_1 + 100) + -1;
  return;
}

/* ================================================================
 * FUN_0206cac8 @ 0206cac8
 * ================================================================ */

void FUN_0206cac8(int param_1)

{
  int iVar1;

  if ((*DAT_0206cb18 & 0x3f) == 0) {
    FUN_0206cef8(param_1);
  }
  if ((*(int *)(param_1 + 0x60) != 0) && (iVar1 = FUN_0206cea0(param_1), iVar1 == 0)) {
    *(undefined1 *)(param_1 + 0x14) = 4;
  }
  FUN_0206cc9c(param_1);
  FUN_02085980(param_1);
  return;
}

/* ================================================================
 * FUN_0206cb1c @ 0206cb1c
 * ================================================================ */

void FUN_0206cb1c(int param_1)

{
  char cVar1;

  cVar1 = *(char *)(param_1 + 0x17);
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  if (cVar1 == '\x01') {
    cVar1 = *(char *)(param_1 + 0x18);
    *(undefined1 *)(param_1 + 0x17) = 0x10;
    if (cVar1 == '\0') {
      *(undefined1 *)(param_1 + 0x14) = 1;
      *(undefined2 *)(param_1 + 100) = *(undefined2 *)(param_1 + 0x66);
    }
    else {
      *(char *)(param_1 + 0x18) = cVar1 + -1;
      FUN_020859e0(param_1,cVar1 + -1 + *(char *)(param_1 + 10) * '\x04');
    }
  }
  FUN_02085980(param_1);
  return;
}

/* ================================================================
 * FUN_0206cb84 @ 0206cb84
 * ================================================================ */

void FUN_0206cb84(int param_1)

{
  short sVar1;
  char *pcVar2;
  char *pcVar3;

  pcVar3 = *(char **)(param_1 + 0x74);
  sVar1 = (short)pcVar3;
  switch(*(undefined1 *)(param_1 + 10)) {
  case 0:
    pcVar2 = pcVar3;
    break;
  case 1:
    do {
      pcVar3 = pcVar3 + -1;
      if (*pcVar3 == -1) break;
    } while (0x1e < (byte)(*pcVar3 - 1U));
    *(short *)(param_1 + 0x7c) = (sVar1 - (short)pcVar3) * 0x10 + -8;
    *(char *)(param_1 + 0x73) = (char)((int)(uint)*(ushort *)(param_1 + 0x7c) >> 4);
    return;
  case 2:
    pcVar2 = pcVar3;
    do {
      pcVar2 = pcVar2 + -0x40;
      if (*pcVar2 == -1) break;
    } while (0x1e < (byte)(*pcVar2 - 1U));
    *(short *)(param_1 + 0x7e) = (short)((uint)((int)pcVar3 - (int)pcVar2) >> 2) + -8;
    *(char *)(param_1 + 0x73) = (char)((int)(uint)*(ushort *)(param_1 + 0x7e) >> 4);
    return;
  case 3:
    do {
      pcVar3 = pcVar3 + 1;
      if (*pcVar3 == -1) break;
    } while (0x1e < (byte)(*pcVar3 - 1U));
    *(short *)(param_1 + 0x7c) = ((short)pcVar3 - sVar1) * 0x10 + -8;
    *(char *)(param_1 + 0x73) = (char)((int)(uint)*(ushort *)(param_1 + 0x7c) >> 4);
    return;
  default:
    return;
  }
  do {
    pcVar2 = pcVar2 + 0x40;
    if (*pcVar2 == -1) break;
  } while (0x1e < (byte)(*pcVar2 - 1U));
  *(short *)(param_1 + 0x7e) = (short)((uint)((int)pcVar2 - (int)pcVar3) >> 2) + -8;
  *(char *)(param_1 + 0x73) = (char)((int)(uint)*(ushort *)(param_1 + 0x7e) >> 4);
  return;
}

/* ================================================================
 * FUN_0206cc9c @ 0206cc9c
 * ================================================================ */

void FUN_0206cc9c(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int unaff_r4;
  int unaff_r5;
  bool bVar5;

  FUN_0206cb84(param_1);
  iVar2 = DAT_0206ce9c;
  switch(*(undefined1 *)(param_1 + 10)) {
  case 0:
    goto LAB_0206ccc8;
  case 1:
    unaff_r5 = (int)*(short *)(param_1 + 0x7a);
    unaff_r4 = (int)(((uint)*(ushort *)(param_1 + 0x78) - (uint)*(ushort *)(param_1 + 0x7c)) *
                    0x10000) >> 0x10;
    break;
  case 2:
    unaff_r4 = (int)*(short *)(param_1 + 0x78);
    unaff_r5 = (int)(((uint)*(ushort *)(param_1 + 0x7a) - (uint)*(ushort *)(param_1 + 0x7e)) *
                    0x10000) >> 0x10;
    break;
  case 3:
LAB_0206ccc8:
    unaff_r4 = (int)*(short *)(param_1 + 0x78);
    unaff_r5 = (int)*(short *)(param_1 + 0x7a);
  }
  iVar1 = FUN_02086120(*(undefined4 *)(DAT_0206ce9c + 0x28));
  if ((iVar1 != 0) &&
     (iVar2 = *(int *)(iVar2 + 0x28),
     (int)(uint)(*(ushort *)(iVar2 + 0x50) & *(ushort *)(param_1 + 0x50)) >> 0xe != 0)) {
    uVar4 = (uint)*(ushort *)(param_1 + 0x7c);
    uVar3 = *(short *)(iVar2 + 0x32) - unaff_r4;
    bVar5 = (uVar3 & 0xffff) <= uVar4;
    if (bVar5) {
      uVar4 = (uint)*(ushort *)(param_1 + 0x7e);
      uVar3 = *(short *)(iVar2 + 0x36) - unaff_r5;
    }
    if (bVar5 && (uVar3 & 0xffff) <= uVar4) {
      FUN_020852f8(iVar2,*(undefined4 *)(param_1 + 0xc),*(undefined1 *)(param_1 + 0x2b));
    }
  }
  iVar2 = DAT_0206ce9c;
  iVar1 = FUN_02086120(*(undefined4 *)(DAT_0206ce9c + 0xbc));
  if ((iVar1 != 0) &&
     (iVar2 = *(int *)(iVar2 + 0xbc),
     (int)(uint)(*(ushort *)(iVar2 + 0x50) & *(ushort *)(param_1 + 0x50)) >> 0xe != 0)) {
    uVar4 = (uint)*(ushort *)(param_1 + 0x7c);
    uVar3 = *(short *)(iVar2 + 0x32) - unaff_r4;
    bVar5 = (uVar3 & 0xffff) <= uVar4;
    if (bVar5) {
      uVar4 = (uint)*(ushort *)(param_1 + 0x7e);
      uVar3 = *(short *)(iVar2 + 0x36) - unaff_r5;
    }
    if (bVar5 && (uVar3 & 0xffff) <= uVar4) {
      FUN_020852f8(iVar2,*(undefined4 *)(param_1 + 0xc),*(undefined1 *)(param_1 + 0x2b));
    }
  }
  iVar2 = DAT_0206ce9c;
  iVar1 = FUN_02086120(*(undefined4 *)(DAT_0206ce9c + 0x150));
  if ((iVar1 != 0) &&
     (iVar2 = *(int *)(iVar2 + 0x150),
     (int)(uint)(*(ushort *)(iVar2 + 0x50) & *(ushort *)(param_1 + 0x50)) >> 0xe != 0)) {
    uVar4 = (uint)*(ushort *)(param_1 + 0x7c);
    uVar3 = *(short *)(iVar2 + 0x32) - unaff_r4;
    bVar5 = (uVar3 & 0xffff) <= uVar4;
    if (bVar5) {
      uVar4 = (uint)*(ushort *)(param_1 + 0x7e);
      uVar3 = *(short *)(iVar2 + 0x36) - unaff_r5;
    }
    if (bVar5 && (uVar3 & 0xffff) <= uVar4) {
      FUN_020852f8(iVar2,*(undefined4 *)(param_1 + 0xc),*(undefined1 *)(param_1 + 0x2b));
    }
  }
  iVar2 = DAT_0206ce9c;
  iVar1 = FUN_02086120(*(undefined4 *)(DAT_0206ce9c + 0x1e4));
  if (iVar1 == 0) {
    return;
  }
  iVar2 = *(int *)(iVar2 + 0x1e4);
  if ((int)(uint)(*(ushort *)(iVar2 + 0x50) & *(ushort *)(param_1 + 0x50)) >> 0xe == 0) {
    return;
  }
  uVar4 = (uint)*(ushort *)(param_1 + 0x7c);
  uVar3 = *(short *)(iVar2 + 0x32) - unaff_r4;
  bVar5 = (uVar3 & 0xffff) <= uVar4;
  if (bVar5) {
    uVar4 = (uint)*(ushort *)(param_1 + 0x7e);
    uVar3 = *(short *)(iVar2 + 0x36) - unaff_r5;
  }
  if (!bVar5 || uVar4 < (uVar3 & 0xffff)) {
    return;
  }
  FUN_020852f8(iVar2,*(undefined4 *)(param_1 + 0xc),*(undefined1 *)(param_1 + 0x2b));
  return;
}

/* ================================================================
 * FUN_0206cea0 @ 0206cea0
 * ================================================================ */

undefined4 FUN_0206cea0(int param_1)

{
  if (*(char *)(param_1 + 0x72) == '\0') {
    if (*(uint *)(param_1 + 0x60) ==
        (*(uint *)(param_1 + 0x60) & *(uint *)(DAT_0206cef4 + (uint)*(byte *)(param_1 + 0x52) * 4)))
    {
      return 1;
    }
  }
  else if (*(uint *)(param_1 + 0x60) !=
           (*(uint *)(param_1 + 0x60) &
           *(uint *)(DAT_0206cef4 + (uint)*(byte *)(param_1 + 0x52) * 4))) {
    return 1;
  }
  return 0;
}

/* ================================================================
 * FUN_0206cef8 @ 0206cef8
 * ================================================================ */

void FUN_0206cef8(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  int iVar2;

  iVar2 = FUN_02076858(0x4a,0,0,param_4,param_4);
  if (iVar2 == 0) {
    return;
  }
  iVar1 = (uint)*(byte *)(param_1 + 10) * 2;
  FUN_02050390(param_1,iVar2,(int)*(char *)(DAT_0206cf6c + iVar1) << 0x10,
               (int)*(char *)(DAT_0206cf6c + iVar1 + 1) << 0x10);
  iVar1 = DAT_0206cf70;
  *(byte *)(iVar2 + 0x2b) = (*(byte *)(param_1 + 10) ^ 2) << 3;
  *(uint *)(iVar2 + 0xc) = (uint)*(ushort *)(iVar1 + (uint)*(byte *)(param_1 + 0x18) * 2);
  return;
}

/* ================================================================
 * FUN_0206cf90 @ 0206cf90
 * ================================================================ */

void FUN_0206cf90(int param_1)

{
  byte *pbVar1;
  undefined *puVar2;
  uint uVar3;

  puVar2 = PTR_DAT_0206d050;
  pbVar1 = DAT_0206d048;
  if (((uint)*(byte *)(param_1 + 0xb) & *(uint *)(PTR_DAT_0206d04c + (*DAT_0206d048 - 1) * 4)) != 0)
  {
    FUN_0204be88(param_1);
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(undefined4 *)(param_1 + 0xc) = 0x80;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(undefined **)(param_1 + 0x48) = puVar2;
  FUN_0206d350(param_1);
  FUN_020859e0(param_1,0);
  uVar3 = (uint)pbVar1[1];
  if (4 < uVar3) {
    FUN_02081088(param_1,*(undefined2 *)
                          (PTR_u_X0YQ00000000YQX_0206d058 +
                          (uint)*(byte *)(*(int *)(*(int *)(PTR_PTR_0206d054 + uVar3 * 4) +
                                                  (uint)pbVar1[0x48] * 8) + 1) * 2));
    return;
  }
  FUN_02081088(param_1,*(undefined2 *)(PTR_DAT_0206d05c + uVar3 * 2));
  return;
}

/* ================================================================
 * FUN_0206d060 @ 0206d060
 * ================================================================ */

void FUN_0206d060(int param_1)

{
  int iVar1;

  iVar1 = FUN_02085ccc();
  if (iVar1 != 0) {
    return;
  }
  iVar1 = FUN_02083ca8(*(undefined2 *)(param_1 + 0x7c),*(undefined2 *)(param_1 + 0x50));
  if (iVar1 == DAT_0206d148) {
    return;
  }
  if (3 < (iVar1 + DAT_0206d14c & 0xffffU)) {
    iVar1 = FUN_02076858(0xf,4,0);
    if (iVar1 != 0) {
      FUN_0205031c(param_1);
    }
    FUN_0204be88(param_1);
    return;
  }
  *(byte *)(param_1 + 0x2b) = (byte)((uint)((iVar1 + DAT_0206d14c) * 0x40000000) >> 0x1b);
  *(undefined1 *)(param_1 + 0x14) = 2;
  *(undefined1 *)(param_1 + 0x17) = 0x20;
  FUN_02085f80(0x15);
  FUN_0206d3d0(param_1);
  iVar1 = FUN_02083ca8(*(short *)(param_1 + 0x7c) +
                       *(short *)(DAT_0206d150 + ((int)(uint)*(byte *)(param_1 + 0x2b) >> 3) * 2),
                       *(undefined2 *)(param_1 + 0x50));
  if ((iVar1 != 0x148) && (iVar1 != 0x14a)) {
    return;
  }
  *(undefined2 *)(param_1 + 0x3a) = 0xfffe;
  return;
}

/* ================================================================
 * FUN_0206d154 @ 0206d154
 * ================================================================ */

void FUN_0206d154(int param_1)

{
  short sVar1;
  short sVar2;
  undefined4 uVar3;
  char cVar4;
  uint uVar5;
  int iVar6;

  FUN_0204fd40();
  uVar5 = FUN_020862a8(param_1);
  if (uVar5 != 0) {
    if ((uVar5 & 1) != 0) {
      *(undefined1 *)(*(int *)(DAT_0206d2cc + 0x28) + 0x41) = 0xb8;
      FUN_02089988();
    }
    uVar3 = DAT_0206d2d0;
    if ((uVar5 & 2) != 0) {
      *(undefined1 *)(*(int *)(DAT_0206d2cc + 0xbc) + 0x41) = 0xb8;
      FUN_02089988(uVar3);
    }
    uVar3 = DAT_0206d2d4;
    if ((uVar5 & 4) != 0) {
      *(undefined1 *)(*(int *)(DAT_0206d2cc + 0x150) + 0x41) = 0xb8;
      FUN_02089988(uVar3);
    }
    uVar3 = DAT_0206d2d8;
    if ((uVar5 & 8) != 0) {
      *(undefined1 *)(*(int *)(DAT_0206d2cc + 0x1e4) + 0x41) = 0xb8;
      FUN_02089988(uVar3);
    }
  }
  cVar4 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar4;
  if (cVar4 != '\0') {
    return;
  }
  iVar6 = FUN_02085ccc(param_1);
  if (iVar6 != 0) {
    return;
  }
  *(undefined2 *)(param_1 + 0x3a) = 0;
  iVar6 = FUN_02083d28(param_1);
  if (iVar6 != 0) {
    iVar6 = FUN_02076858(0xf,4,0);
    if (iVar6 != 0) {
      FUN_0205031c(param_1);
    }
    iVar6 = (int)(uint)*(byte *)(param_1 + 0x2b) >> 2;
    sVar1 = *(short *)(DAT_0206d2dc + iVar6 * 2);
    sVar2 = *(short *)(DAT_0206d2dc + (iVar6 + 1) * 2);
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
    *(short *)(param_1 + 0x32) = *(short *)(param_1 + 0x32) + sVar1;
    *(short *)(param_1 + 0x36) = *(short *)(param_1 + 0x36) + sVar2;
    FUN_0206d350(param_1);
    FUN_0208366c(0,*(undefined2 *)(param_1 + 0x7c),*(undefined2 *)(param_1 + 0x50));
    *(undefined1 *)(param_1 + 0x14) = 3;
    *(undefined1 *)(param_1 + 0x17) = 0x3c;
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 1;
  FUN_0206d350(param_1);
  return;
}

/* ================================================================
 * FUN_0206d2e0 @ 0206d2e0
 * ================================================================ */

void FUN_0206d2e0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 uVar1;
  char cVar2;

  cVar2 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar2;
  uVar1 = DAT_0206d34c;
  if (cVar2 != '\0') {
    *(uint *)(param_1 + 0x1c) =
         *(uint *)(param_1 + 0x1c) & 0xfffffffc | *(uint *)(param_1 + 0x1c) & 3 ^ 1;
    return;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(undefined1 *)(param_1 + 0x14) = 1;
  FUN_0208366c(uVar1,*(undefined2 *)(param_1 + 0x7c),*(undefined2 *)(param_1 + 0x50),param_1,param_4
              );
  return;
}

/* ================================================================
 * FUN_0206d350 @ 0206d350
 * ================================================================ */

void FUN_0206d350(int param_1)

{
  undefined1 uVar1;
  undefined2 uVar2;

  *(ushort *)(param_1 + 0x7c) =
       (ushort)(((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_0206d3c8 + 0x1a)) >>
                4) << 6) |
       (ushort)((int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_0206d3c8 + 0x18)) >> 4
               );
  uVar2 = FUN_0208367c(*(undefined2 *)(param_1 + 0x7c),*(undefined2 *)(param_1 + 0x50));
  *(undefined2 *)(param_1 + 0x78) = uVar2;
  uVar1 = FUN_02083d4c(*(undefined2 *)(param_1 + 0x7c),*(undefined2 *)(param_1 + 0x50));
  *(undefined1 *)(param_1 + 0x7a) = uVar1;
  FUN_0208366c(DAT_0206d3cc,*(undefined2 *)(param_1 + 0x7c),*(undefined2 *)(param_1 + 0x50));
  FUN_0208365c(0xf,*(undefined2 *)(param_1 + 0x7c),*(undefined2 *)(param_1 + 0x50));
  return;
}

/* ================================================================
 * FUN_0206d3d0 @ 0206d3d0
 * ================================================================ */

void FUN_0206d3d0(int param_1)

{
  FUN_0208366c(*(undefined2 *)(param_1 + 0x78),*(undefined2 *)(param_1 + 0x7c),
               *(undefined2 *)(param_1 + 0x50));
  FUN_0208365c(*(undefined1 *)(param_1 + 0x7a),*(undefined2 *)(param_1 + 0x7c),
               *(undefined2 *)(param_1 + 0x50));
  return;
}

/* ================================================================
 * FUN_0206d3fc @ 0206d3fc
 * ================================================================ */

void FUN_0206d3fc(int param_1)

{
  (**(code **)(DAT_0206d414 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206d418 @ 0206d418
 * ================================================================ */

void FUN_0206d418(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;

  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  iVar1 = DAT_0206d4cc;
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(undefined1 *)(param_1 + 0x17) = 0x3c;
  *(ushort *)(param_1 + 0x7c) =
       (ushort)(((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(iVar1 + 0x1a)) >> 4) << 6
               ) |
       (ushort)((int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(iVar1 + 0x18)) >> 4);
  if ((int)(uint)*(ushort *)(param_1 + 0x50) >> 0xe == 2) {
    uVar4 = 0x8000;
    uVar3 = 0x8000;
    uVar2 = 0x58;
  }
  else {
    uVar4 = 0x4000;
    uVar3 = 0x4000;
    uVar2 = 0x2f;
  }
  FUN_02021c28(uVar2,*(undefined2 *)(param_1 + 0x7c),uVar3);
  FUN_0208366c(1,*(undefined2 *)(param_1 + 0x7c),uVar4);
  FUN_020859e0(param_1,*(byte *)(param_1 + 10) & 3);
  FUN_02085f80(0x88);
  return;
}

/* ================================================================
 * FUN_0206d4d0 @ 0206d4d0
 * ================================================================ */

void FUN_0206d4d0(int param_1)

{
  char cVar1;
  int iVar2;

  FUN_02085980(param_1);
  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  iVar2 = FUN_02083ca8(*(undefined2 *)(param_1 + 0x7c),*(undefined2 *)(param_1 + 0x50));
  if (iVar2 != 0x2f && iVar2 != 0x58) {
    *(undefined1 *)(param_1 + 0x14) = 2;
    FUN_020859e0(param_1,4);
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 3;
  *(undefined1 *)(param_1 + 0x17) = 0x1e;
  return;
}

/* ================================================================
 * FUN_0206d530 @ 0206d530
 * ================================================================ */

void FUN_0206d530(int param_1)

{
  FUN_02085980(param_1);
  if ((*(byte *)(param_1 + 0x29) & 0x80) != 0) {
    if (4 < *(byte *)(param_1 + 10)) {
      *(undefined1 *)(param_1 + 10) = 4;
    }
    FUN_02021c28(*(undefined2 *)(DAT_0206d580 + (uint)*(byte *)(param_1 + 10) * 2),
                 *(undefined2 *)(param_1 + 0x7c),*(undefined2 *)(param_1 + 0x50));
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206d584 @ 0206d584
 * ================================================================ */

void FUN_0206d584(int param_1)

{
  char cVar1;

  cVar1 = *(char *)(param_1 + 0x17);
  *(uint *)(param_1 + 0x1c) =
       *(uint *)(param_1 + 0x1c) & 0xfffffffc | *(uint *)(param_1 + 0x1c) & 3 ^ 1;
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  if (cVar1 != '\x01') {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_0206d5c4 @ 0206d5c4
 * ================================================================ */

void FUN_0206d5c4(int param_1)

{
  (**(code **)(DAT_0206d5dc + (uint)*(byte *)(param_1 + 10) * 4))();
  return;
}

/* ================================================================
 * FUN_0206d5e0 @ 0206d5e0
 * ================================================================ */

void FUN_0206d5e0(int param_1)

{
  undefined2 uVar1;

  uVar1 = *(undefined2 *)(DAT_0206d60c + ((uint)*(byte *)(param_1 + 10) * 2 + 1) * 2);
  *(undefined2 *)(param_1 + 0x32) =
       *(undefined2 *)(DAT_0206d60c + (uint)*(byte *)(param_1 + 10) * 4);
  *(undefined2 *)(param_1 + 0x36) = uVar1;
  return;
}

/* ================================================================
 * FUN_0206d62c @ 0206d62c
 * ================================================================ */

void FUN_0206d62c(int param_1)

{
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x40000002;
  *(undefined1 *)(param_1 + 0x17) = 0x1e;
  FUN_0206d5e0(param_1);
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_0206d678 @ 0206d678
 * ================================================================ */

void FUN_0206d678(int param_1)

{
  if (*(char *)(param_1 + 0x17) != '\0') {
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
    return;
  }
  *(short *)(param_1 + 0x32) = *(short *)(param_1 + 0x32) + -2;
  if (*(short *)(param_1 + 0x32) < -0x77) {
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206d6b0 @ 0206d6b0
 * ================================================================ */

void FUN_0206d6b0(int param_1)

{
  (**(code **)(DAT_0206d6c8 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206d6cc @ 0206d6cc
 * ================================================================ */

void FUN_0206d6cc(int param_1)

{
  if (*(char *)(param_1 + 0x17) != '\0') {
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
    return;
  }
  *(short *)(param_1 + 0x32) = *(short *)(param_1 + 0x32) + 2;
  if (0x197 < *(short *)(param_1 + 0x32)) {
    FUN_0204be88(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206d704 @ 0206d704
 * ================================================================ */

void FUN_0206d704(int param_1)

{
  (**(code **)(DAT_0206d71c + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_0206d720 @ 0206d720
 * ================================================================ */

void FUN_0206d720(int param_1)

{
  uint uVar1;
  bool bVar2;

  uVar1 = (uint)*(byte *)(param_1 + 10);
  bVar2 = uVar1 != 4;
  if (bVar2) {
    uVar1 = *(uint *)(param_1 + 0x1c) & 0x3ff033fc | 0x80058001;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3ff03ffc | 0x80058001;
  if (bVar2) {
    *(uint *)(param_1 + 0x1c) = uVar1 | 0x400;
  }
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  FUN_0206d5e0(param_1);
  *(short *)(param_1 + 0x32) = *(short *)(param_1 + 0x32) + *(short *)(DAT_0206d79c + 0x26);
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_0206d7a0 @ 0206d7a0
 * ================================================================ */

void FUN_0206d7a0(int param_1)

{
  int iVar1;

  FUN_0206d5e0(param_1);
  *(short *)(param_1 + 0x32) = *(short *)(param_1 + 0x32) + *(short *)(DAT_0206d7d8 + 0x26);
  iVar1 = FUN_0205b6f0();
  if (iVar1 == 0) {
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffff3ff;
  }
  return;
}

/* ================================================================
 * FUN_0206d7dc @ 0206d7dc
 * ================================================================ */

void FUN_0206d7dc(int param_1)

{
  byte bVar1;
  uint uVar2;

  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(undefined1 *)(param_1 + 0x1b) = 0x4e;
    *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 3;
    *(undefined1 *)(param_1 + 0x20) = 0x80;
    *(short *)(param_1 + 0x32) = *(short *)(param_1 + 0x32) + 8;
    *(short *)(param_1 + 0x36) = *(short *)(param_1 + 0x36) + 8;
    *(undefined1 *)(param_1 + 0x17) = 0;
    *(undefined1 *)(param_1 + 0x18) = 0;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc;
  }
  uVar2 = *(uint *)(param_1 + 0x1c);
  *(uint *)(param_1 + 0x1c) = uVar2 & 0xfff0ffff;
  uVar2 = uVar2 & 0xfff0fffc;
  if (*(char *)(param_1 + 0x18) != '\0') {
    uVar2 = uVar2 | 2;
  }
  *(uint *)(param_1 + 0x1c) = uVar2;
  bVar1 = *(char *)(param_1 + 0x17) + 1;
  *(byte *)(param_1 + 0x17) = bVar1;
  if (0x10 < bVar1) {
    *(undefined1 *)(param_1 + 0x17) = 0;
    *(byte *)(param_1 + 0x18) = *(byte *)(param_1 + 0x18) ^ 1;
  }
  return;
}

/* ================================================================
 * FUN_0206d898 @ 0206d898
 * ================================================================ */

void FUN_0206d898(int param_1)

{
  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 1;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x40004002;
    FUN_0206d5e0(param_1);
    FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
    return;
  }
  FUN_02085980(param_1);
  return;
}
