#include "game/arm9_middle_02061818_raw.h"

/*
 * Raw matching-first recovery for catalog entries 0x02061818..0x02063fff.
 * The 31 bodies remain deliberately address-named until semantic review.
 * They are promotion candidates, not linked byte-matching evidence.
 *
 * Calls whose implicit r0 argument was omitted by Ghidra are written with the
 * owning param_1 so this mechanically faithful draft remains valid C.
 */

extern uint DAT_02061b7c;
extern uint DAT_02061b80;
extern uint DAT_02061b84;
extern uint DAT_02061c00;
extern uint DAT_02061c60;
extern uint DAT_02061cb0;
extern uint DAT_02061db8;
extern uint DAT_02061dbc;
extern uint DAT_02061dc0;
extern uint DAT_02061dfc;
extern uint DAT_02061e00;
extern uint DAT_020623b4;
extern uint DAT_020623b8;
extern uint DAT_020623bc;
extern uint DAT_020623c0;
extern uint DAT_020623c4;
extern uint DAT_02062b9c;
extern uint DAT_02062ba0;
extern uint DAT_02062ba4;
extern uint DAT_02062ba8;
extern uint DAT_02062bac;
extern uint DAT_02062bb0;
extern uint DAT_02062bb4;
extern uint DAT_02062ce0;
extern uint DAT_02062d7c;
extern uint DAT_02062d80;
extern uint DAT_02062d84;
extern uint DAT_02062d88;
extern uint DAT_02062d8c;
extern uint DAT_02062f14;
extern uint DAT_02062f18;
extern uint DAT_02062f1c;
extern uint DAT_02062f20;
extern uint DAT_02062fc8;
extern byte *DAT_02063050;
extern byte *DAT_02063054;
extern uint DAT_02063058;
extern uint DAT_020630ac;
extern uint DAT_020630c8;
extern uint DAT_020633fc;
extern uint DAT_020635b4;
extern uint DAT_020635b8;
extern uint DAT_0206366c;
extern uint DAT_02063670;
extern undefined2 *DAT_02063674;
extern uint DAT_020637b8;
extern uint DAT_020637bc;
extern uint DAT_020637c0;
extern undefined2 *DAT_020637c4;
extern uint DAT_02063864;
extern uint DAT_02063868;
extern uint DAT_0206386c;
extern uint DAT_02063994;
extern uint DAT_02063998;
extern uint DAT_0206399c;
extern uint DAT_020639a0;
extern uint DAT_020639a4;
extern uint DAT_02063aa4;
extern uint DAT_02063aa8;
extern uint DAT_02063aac;
extern uint DAT_02063fd4;

/* ================================================================
 * FUN_02061818 @ 02061818
 * ================================================================ */

void FUN_02061818(int param_1)

{
  undefined4 uVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  undefined2 uVar5;
  uint uVar6;

  uVar4 = (uint)*(byte *)(param_1 + 10);
  uVar6 = ((int)(((int)*(short *)(param_1 + 0x36) + uVar4 * -8) -
                (uint)*(ushort *)(DAT_02061b7c + 0x1a)) >> 4) << 6 |
          (int)(((int)*(short *)(param_1 + 0x32) + uVar4 * -8) -
               (uint)*(ushort *)(DAT_02061b7c + 0x18)) >> 4;
  uVar2 = uVar6 & 0xffff;
  if (uVar4 == 1) {
    FUN_02021c28(*(uint *)(param_1 + 0x60) & 0xffff,uVar6 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x140,uVar2 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x141,uVar2 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    uVar2 = uVar2 + 0x41;
    uVar5 = *(undefined2 *)(param_1 + 0x50);
  }
  else {
    if (uVar4 == 2) {
      FUN_02021c28(*(uint *)(param_1 + 0x60) & 0xffff,uVar6 & 0xffff,*(undefined2 *)(param_1 + 0x50)
                  );
      FUN_02021c28(DAT_02061b80,uVar2 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x140,uVar2 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x145,uVar2 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x147,uVar2 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x146,uVar2 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x141,uVar2 + 0x80 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      FUN_02021c28(0x144,uVar2 + 0x81 & 0xffff,*(undefined2 *)(param_1 + 0x50));
      uVar1 = 0x142;
      uVar2 = uVar2 + 0x82 & 0xffff;
      uVar5 = *(undefined2 *)(param_1 + 0x50);
      goto LAB_02061b6c;
    }
    if (uVar4 != 3) {
      iVar3 = FUN_02085ccc();
      if (iVar3 != 0) {
        return;
      }
      uVar5 = *(undefined2 *)(param_1 + 0x50);
      uVar1 = DAT_02061b84;
      goto LAB_02061b6c;
    }
    FUN_02021c28(*(uint *)(param_1 + 0x60) & 0xffff,uVar6 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    uVar1 = DAT_02061b80;
    FUN_02021c28(DAT_02061b80,uVar2 + 1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(uVar1,uVar2 + 2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x140,uVar2 + 3 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x145,uVar2 + 0x40 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x147,uVar2 + 0x41 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x147,uVar2 + 0x42 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x146,uVar2 + 0x43 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x145,uVar2 + 0x80 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x147,uVar2 + 0x81 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x147,uVar2 + 0x82 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x146,uVar2 + 0x83 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x141,uVar2 + 0xc0 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x144,uVar2 + 0xc1 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    FUN_02021c28(0x144,uVar2 + 0xc2 & 0xffff,*(undefined2 *)(param_1 + 0x50));
    uVar2 = uVar2 + 0xc3;
    uVar5 = *(undefined2 *)(param_1 + 0x50);
  }
  uVar1 = 0x142;
  uVar2 = uVar2 & 0xffff;
LAB_02061b6c:
  FUN_02021c28(uVar1,uVar2,uVar5);
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02061b88 @ 02061b88
 * ================================================================ */

void FUN_02061b88(undefined4 param_1,int param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  uint uVar2;

  uVar2 = (uint)*(ushort *)(param_2 + 0x50);
  iVar1 = (int)uVar2 >> 0xe;
  if (iVar1 == 2) {
    if ((*(byte *)(param_2 + 0x2b) & 8) != 0) {
      FUN_02021c28(0x7a,param_1,uVar2,2,param_4);
      return;
    }
    FUN_02021c28(0x79,param_1,uVar2,2,param_4);
    return;
  }
  if ((*(byte *)(param_2 + 0x2b) & 8) != 0) {
    FUN_02021c28(0x51,param_1,uVar2,iVar1,param_4);
    return;
  }
  FUN_02021c28(0x50,param_1,uVar2,iVar1,param_4);
  return;
}

/* ================================================================
 * FUN_02061be8 @ 02061be8
 * ================================================================ */

void FUN_02061be8(int param_1)

{
  (**(code **)(DAT_02061c00 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_02061c04 @ 02061c04
 * ================================================================ */

void FUN_02061c04(int param_1)

{
  undefined4 uVar1;

  *(undefined1 *)(param_1 + 0x14) = 1;
  *(undefined1 *)(param_1 + 0x17) = 0x18;
  uVar1 = FUN_020460e8(*(undefined1 *)(param_1 + 10));
  *(undefined4 *)(param_1 + 0x60) = uVar1;
  *(ushort *)(param_1 + 0x78) =
       (ushort)(((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_02061c60 + 0x1a)) >>
                4) << 6) |
       (ushort)((int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_02061c60 + 0x18)) >> 4
               );
  FUN_02061e04(param_1);
  return;
}

/* ================================================================
 * FUN_02061c64 @ 02061c64
 * ================================================================ */

void FUN_02061c64(int param_1)

{
  if ((*(byte *)(param_1 + 0xb) & 0x80) != 0) {
    *(undefined1 *)(param_1 + 0x14) = 2;
    FUN_02061d0c(param_1);
    return;
  }
  if (*(uint *)(param_1 + 0x60) !=
      (*(uint *)(param_1 + 0x60) & *(uint *)(DAT_02061cb0 + (uint)*(byte *)(param_1 + 0x52) * 4))) {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 2;
  FUN_02061d0c(param_1);
  return;
}

/* ================================================================
 * FUN_02061cb4 @ 02061cb4
 * ================================================================ */

void FUN_02061cb4(int param_1)

{
  char cVar1;

  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 == '\0') {
    *(undefined1 *)(param_1 + 0x14) = 3;
    return;
  }
  if (cVar1 == '\f') {
    FUN_02061e44(param_1);
    FUN_02085f98(param_1,0x11);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02061cf4 @ 02061cf4
 * ================================================================ */

void FUN_02061cf4(undefined4 param_1)

{
  FUN_020623c8(param_1);
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02061d0c @ 02061d0c
 * ================================================================ */

void FUN_02061d0c(int param_1)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  ushort uVar4;
  short sVar5;
  short sVar6;
  ushort uVar7;
  ushort *puVar8;
  uint uVar9;

  uVar9 = *(byte *)(param_1 + 0xb) & 7;
  *(short *)(param_1 + 0x7c) = (short)DAT_02061db8;
  if (3 < uVar9) {
    return;
  }
  sVar5 = *(short *)(DAT_02061dbc + uVar9 * 2);
  puVar8 = (ushort *)(DAT_02061dc0 + uVar9 * 8);
  sVar6 = *(short *)(param_1 + 0x78) + sVar5 * 3;
  uVar1 = puVar8[1];
  uVar2 = puVar8[2];
  uVar3 = puVar8[3];
  uVar4 = *puVar8;
  uVar7 = 0;
  while( true ) {
    uVar9 = FUN_02083ca8(sVar6,*(undefined2 *)(param_1 + 0x50));
    if (((uVar4 == uVar9 || uVar1 == uVar9) || uVar2 == uVar9) || uVar3 == uVar9) {
      *(short *)(param_1 + 0x7c) = sVar6;
    }
    if (((uVar4 == uVar9 || uVar1 == uVar9) || uVar2 == uVar9) || uVar3 == uVar9) break;
    uVar7 = uVar7 + 1;
    sVar6 = sVar6 + sVar5;
    if (7 < uVar7) {
      return;
    }
  }
  return;
}

/* ================================================================
 * FUN_02061dc4 @ 02061dc4
 * ================================================================ */

bool FUN_02061dc4(int param_1)

{
  return *(char *)(DAT_02061e00 +
                  ((uint)*(ushort *)(param_1 + 0x78) +
                   (int)*(short *)(DAT_02061dfc + (*(byte *)(param_1 + 0xb) & 3) * 2) & 0xffff)) !=
         '\x18';
}

/* ================================================================
 * FUN_02061e04 @ 02061e04
 * ================================================================ */

void FUN_02061e04(int param_1)

{
  ushort uVar1;
  int iVar2;

  uVar1 = FUN_02061dc4(param_1);
  *(ushort *)(param_1 + 0x7e) = *(ushort *)(param_1 + 0x7e) | uVar1;
  if ((*(byte *)(param_1 + 0xb) & 4) == 0) {
    iVar2 = FUN_02083cdc(param_1);
    if (iVar2 == 0x5c) {
      *(ushort *)(param_1 + 0x7e) = *(ushort *)(param_1 + 0x7e) | 2;
    }
    return;
  }
  return;
}

/* ================================================================
 * FUN_02061e44 @ 02061e44
 * ================================================================ */

void FUN_02061e44(int param_1)

{
  short sVar1;
  ushort uVar2;
  undefined2 *puVar3;
  uint uVar4;

  if (*(byte *)(DAT_020623b4 + 1) == 6) {
    uVar2 = *(byte *)(param_1 + 0xb) & 3;
    puVar3 = (undefined2 *)
             (*(int *)(DAT_020623bc +
                      (uint)*(byte *)(*(int *)(*(int *)(DAT_020623b8 + 0x18) +
                                              (uint)*(byte *)(DAT_020623b4 + 0x48) * 8) + 1) * 4) +
             ((uint)*(ushort *)(param_1 + 0x7e) * 0x18 + (short)uVar2 * 6) * 2);
  }
  else {
    uVar2 = *(byte *)(param_1 + 0xb) & 3;
    puVar3 = (undefined2 *)
             (*(int *)(DAT_020623c0 + (uint)*(byte *)(DAT_020623b4 + 1) * 4) +
             ((uint)*(ushort *)(param_1 + 0x7e) * 0x18 + (short)uVar2 * 6) * 2);
  }
  sVar1 = *(short *)(param_1 + 0x78);
  switch(uVar2) {
  case 0:
    break;
  case 1:
    FUN_02021c28(*puVar3,sVar1 + -0x40,0x4000);
    FUN_02021c28(puVar3[1],sVar1 + -0x3f,0x8000);
    FUN_02021c28(puVar3[2],sVar1,0x4000);
    FUN_02021c28(puVar3[3],sVar1 + 1,0x8000);
    FUN_02021c28(puVar3[4],sVar1 + 0x40,0x4000);
    FUN_02021c28(puVar3[5],sVar1 + 0x41,0x8000);
    uVar4 = (uint)*(ushort *)(param_1 + 0x7c);
    if (uVar4 != DAT_020623c4) {
      FUN_02021c28(puVar3[0xc],uVar4 - 0x41 & 0xffff,0x8000);
      FUN_02021c28(puVar3[0xd],uVar4 - 0x40 & 0xffff,0x4000);
      FUN_02021c28(puVar3[0xe],uVar4 - 1 & 0xffff,0x8000);
      FUN_02021c28(puVar3[0xf],uVar4,0x4000);
      FUN_02021c28(puVar3[0x10],uVar4 + 0x3f & 0xffff,0x8000);
      FUN_02021c28(puVar3[0x11],uVar4 + 0x40 & 0xffff,0x4000);
      return;
    }
    return;
  case 2:
    FUN_02021c28(*puVar3,sVar1 + -1,0x4000);
    FUN_02021c28(puVar3[1],sVar1,0x4000);
    FUN_02021c28(puVar3[2],sVar1 + 1,0x4000);
    FUN_02021c28(puVar3[3],sVar1 + 0x3f,0x8000);
    FUN_02021c28(puVar3[4],sVar1 + 0x40,0x8000);
    FUN_02021c28(puVar3[5],sVar1 + 0x41,0x8000);
    uVar4 = (uint)*(ushort *)(param_1 + 0x7c);
    if (uVar4 != DAT_020623c4) {
      FUN_02021c28(puVar3[-0xc],uVar4 - 0x41 & 0xffff,0x8000);
      FUN_02021c28(puVar3[-0xb],uVar4 - 0x40 & 0xffff,0x8000);
      FUN_02021c28(puVar3[-10],uVar4 - 0x3f & 0xffff,0x8000);
      FUN_02021c28(puVar3[-9],uVar4 - 1 & 0xffff,0x4000);
      FUN_02021c28(puVar3[-8],uVar4,0x4000);
      FUN_02021c28(puVar3[-7],uVar4 + 1 & 0xffff,0x4000);
      return;
    }
    return;
  case 3:
    FUN_02021c28(*puVar3,sVar1 + -0x41,0x8000);
    FUN_02021c28(puVar3[1],sVar1 + -0x40,0x4000);
    FUN_02021c28(puVar3[2],sVar1 + -1,0x8000);
    FUN_02021c28(puVar3[3],sVar1,0x4000);
    FUN_02021c28(puVar3[4],sVar1 + 0x3f,0x8000);
    FUN_02021c28(puVar3[5],sVar1 + 0x40,0x4000);
    uVar4 = (uint)*(ushort *)(param_1 + 0x7c);
    if (uVar4 != DAT_020623c4) {
      FUN_02021c28(puVar3[-0xc],uVar4 - 0x40 & 0xffff,0x4000);
      FUN_02021c28(puVar3[-0xb],uVar4 - 0x3f & 0xffff,0x8000);
      FUN_02021c28(puVar3[-10],uVar4,0x4000);
      FUN_02021c28(puVar3[-9],uVar4 + 1 & 0xffff,0x8000);
      FUN_02021c28(puVar3[-8],uVar4 + 0x40 & 0xffff,0x4000);
      FUN_02021c28(puVar3[-7],uVar4 + 0x41 & 0xffff,0x8000);
      return;
    }
    return;
  default:
    return;
  }
  FUN_02021c28(*puVar3,sVar1 + -0x41,0x8000);
  FUN_02021c28(puVar3[1],sVar1 + -0x40,0x8000);
  FUN_02021c28(puVar3[2],sVar1 + -0x3f,0x8000);
  FUN_02021c28(puVar3[3],sVar1 + -1,0x4000);
  FUN_02021c28(puVar3[4],sVar1,0x4000);
  FUN_02021c28(puVar3[5],sVar1 + 1,0x4000);
  uVar4 = (uint)*(ushort *)(param_1 + 0x7c);
  if (uVar4 != DAT_020623c4) {
    FUN_02021c28(puVar3[0xc],uVar4 - 1 & 0xffff,0x4000);
    FUN_02021c28(puVar3[0xd],uVar4,0x4000);
    FUN_02021c28(puVar3[0xe],uVar4 + 1 & 0xffff,0x4000);
    FUN_02021c28(puVar3[0xf],uVar4 + 0x3f & 0xffff,0x8000);
    FUN_02021c28(puVar3[0x10],uVar4 + 0x40 & 0xffff,0x8000);
    FUN_02021c28(puVar3[0x11],uVar4 + 0x41 & 0xffff,0x8000);
    return;
  }
  return;
}

/* ================================================================
 * FUN_020623c8 @ 020623c8
 * ================================================================ */

void FUN_020623c8(int param_1)

{
  byte bVar1;
  short sVar2;
  uint uVar3;
  int iVar4;
  byte bVar5;
  undefined2 *puVar6;

  uVar3 = (uint)*(byte *)(DAT_02062b9c + 1);
  if (uVar3 == 6) {
    bVar5 = *(byte *)(param_1 + 0xb);
    iVar4 = *(int *)(*(int *)(DAT_02062ba0 + 0x18) + (uint)*(byte *)(DAT_02062b9c + 0x48) * 8);
    if ((bVar5 & 4) == 0) {
      bVar1 = *(byte *)(iVar4 + 1);
      iVar4 = DAT_02062ba8;
    }
    else {
      bVar1 = *(byte *)(iVar4 + 1);
      iVar4 = DAT_02062ba4;
    }
    puVar6 = (undefined2 *)
             (*(int *)(iVar4 + (uint)bVar1 * 4) +
             ((uint)*(ushort *)(param_1 + 0x7e) * 0x18 + (short)(bVar5 & 3) * 6) * 2);
  }
  else {
    bVar5 = *(byte *)(param_1 + 0xb);
    iVar4 = (short)(bVar5 & 3) * 6;
    if ((bVar5 & 4) == 0) {
      puVar6 = (undefined2 *)
               (*(int *)(DAT_02062bb0 + uVar3 * 4) +
               ((uint)*(ushort *)(param_1 + 0x7e) * 0x18 + iVar4) * 2);
    }
    else {
      puVar6 = (undefined2 *)
               (*(int *)(DAT_02062bac + uVar3 * 4) +
               ((uint)*(ushort *)(param_1 + 0x7e) * 0x18 + iVar4) * 2);
    }
  }
  sVar2 = *(short *)(param_1 + 0x78);
  switch(bVar5 & 7) {
  case 0:
    break;
  case 1:
    FUN_02021c28(*puVar6,sVar2 + -0x40,0x4000);
    FUN_02021c28(puVar6[1],sVar2 + -0x3f,0x8000);
    FUN_02021c28(puVar6[2],sVar2,0x4000);
    FUN_02021c28(puVar6[3],sVar2 + 1,0x8000);
    FUN_02021c28(puVar6[4],sVar2 + 0x40,0x4000);
    FUN_02021c28(puVar6[5],sVar2 + 0x41,0x8000);
    uVar3 = (uint)*(ushort *)(param_1 + 0x7c);
    if (uVar3 == DAT_02062bb4) {
      return;
    }
    FUN_02021c28(puVar6[0xc],uVar3 - 0x41 & 0xffff,0x8000);
    FUN_02021c28(puVar6[0xd],uVar3 - 0x40 & 0xffff,0x4000);
    FUN_02021c28(puVar6[0xe],uVar3 - 1 & 0xffff,0x8000);
    FUN_02021c28(puVar6[0xf],uVar3,0x4000);
    FUN_02021c28(puVar6[0x10],uVar3 + 0x3f & 0xffff,0x8000);
    FUN_02021c28(puVar6[0x11],uVar3 + 0x40 & 0xffff,0x4000);
    return;
  case 2:
    FUN_02021c28(*puVar6,sVar2 + -1,0x4000);
    FUN_02021c28(puVar6[1],sVar2,0x4000);
    FUN_02021c28(puVar6[2],sVar2 + 1,0x4000);
    FUN_02021c28(puVar6[3],sVar2 + 0x3f,0x8000);
    FUN_02021c28(puVar6[4],sVar2 + 0x40,0x8000);
    FUN_02021c28(puVar6[5],sVar2 + 0x41,0x8000);
    uVar3 = (uint)*(ushort *)(param_1 + 0x7c);
    if (uVar3 == DAT_02062bb4) {
      return;
    }
    FUN_02021c28(puVar6[-0xc],uVar3 - 0x41 & 0xffff,0x8000);
    FUN_02021c28(puVar6[-0xb],uVar3 - 0x40 & 0xffff,0x8000);
    FUN_02021c28(puVar6[-10],uVar3 - 0x3f & 0xffff,0x8000);
    FUN_02021c28(puVar6[-9],uVar3 - 1 & 0xffff,0x4000);
    FUN_02021c28(puVar6[-8],uVar3,0x4000);
    FUN_02021c28(puVar6[-7],uVar3 + 1 & 0xffff,0x4000);
    return;
  case 3:
    FUN_02021c28(*puVar6,sVar2 + -0x41,0x8000);
    FUN_02021c28(puVar6[1],sVar2 + -0x40,0x4000);
    FUN_02021c28(puVar6[2],sVar2 + -1,0x8000);
    FUN_02021c28(puVar6[3],sVar2,0x4000);
    FUN_02021c28(puVar6[4],sVar2 + 0x3f,0x8000);
    FUN_02021c28(puVar6[5],sVar2 + 0x40,0x4000);
    uVar3 = (uint)*(ushort *)(param_1 + 0x7c);
    if (uVar3 == DAT_02062bb4) {
      return;
    }
    FUN_02021c28(puVar6[-0xc],uVar3 - 0x40 & 0xffff,0x4000);
    FUN_02021c28(puVar6[-0xb],uVar3 - 0x3f & 0xffff,0x8000);
    FUN_02021c28(puVar6[-10],uVar3,0x4000);
    FUN_02021c28(puVar6[-9],uVar3 + 1 & 0xffff,0x8000);
    FUN_02021c28(puVar6[-8],uVar3 + 0x40 & 0xffff,0x4000);
    FUN_02021c28(puVar6[-7],uVar3 + 0x41 & 0xffff,0x8000);
    return;
  case 4:
    FUN_02021c28(*puVar6,sVar2 + -0x41,0x8000);
    FUN_02021c28(puVar6[1],sVar2 + -0x40,0x8000);
    FUN_02021c28(puVar6[2],sVar2 + -0x3f,0x8000);
    FUN_02021c28(puVar6[3],sVar2 + -1,0x4000);
    FUN_02021c28(puVar6[4],sVar2,0x4000);
    FUN_02021c28(puVar6[5],sVar2 + 1,0x4000);
    return;
  case 5:
    FUN_02021c28(*puVar6,sVar2 + -0x40,0x4000);
    FUN_02021c28(puVar6[1],sVar2 + -0x3f,0x8000);
    FUN_02021c28(puVar6[2],sVar2,0x4000);
    FUN_02021c28(puVar6[3],sVar2 + 1,0x8000);
    FUN_02021c28(puVar6[4],sVar2 + 0x40,0x4000);
    FUN_02021c28(puVar6[5],sVar2 + 0x41,0x8000);
    return;
  case 6:
    FUN_02021c28(*puVar6,sVar2 + -1,0x4000);
    FUN_02021c28(puVar6[1],sVar2,0x4000);
    FUN_02021c28(puVar6[2],sVar2 + 1,0x4000);
    FUN_02021c28(puVar6[3],sVar2 + 0x3f,0x8000);
    FUN_02021c28(puVar6[4],sVar2 + 0x40,0x8000);
    FUN_02021c28(puVar6[5],sVar2 + 0x41,0x8000);
    return;
  case 7:
    FUN_02021c28(*puVar6,sVar2 + -0x41,0x8000);
    FUN_02021c28(puVar6[1],sVar2 + -0x40,0x4000);
    FUN_02021c28(puVar6[2],sVar2 + -1,0x8000);
    FUN_02021c28(puVar6[3],sVar2,0x4000);
    FUN_02021c28(puVar6[4],sVar2 + 0x3f,0x8000);
    FUN_02021c28(puVar6[5],sVar2 + 0x40,0x4000);
    return;
  default:
    return;
  }
  FUN_02021c28(*puVar6,sVar2 + -0x41,0x8000);
  FUN_02021c28(puVar6[1],sVar2 + -0x40,0x8000);
  FUN_02021c28(puVar6[2],sVar2 + -0x3f,0x8000);
  FUN_02021c28(puVar6[3],sVar2 + -1,0x4000);
  FUN_02021c28(puVar6[4],sVar2,0x4000);
  FUN_02021c28(puVar6[5],sVar2 + 1,0x4000);
  uVar3 = (uint)*(ushort *)(param_1 + 0x7c);
  if (uVar3 == DAT_02062bb4) {
    return;
  }
  FUN_02021c28(puVar6[0xc],uVar3 - 1 & 0xffff,0x4000);
  FUN_02021c28(puVar6[0xd],uVar3,0x4000);
  FUN_02021c28(puVar6[0xe],uVar3 + 1 & 0xffff,0x4000);
  FUN_02021c28(puVar6[0xf],uVar3 + 0x3f & 0xffff,0x8000);
  FUN_02021c28(puVar6[0x10],uVar3 + 0x40 & 0xffff,0x8000);
  FUN_02021c28(puVar6[0x11],uVar3 + 0x41 & 0xffff,0x8000);
  return;
}

/* ================================================================
 * FUN_02062bd4 @ 02062bd4
 * ================================================================ */

void FUN_02062bd4(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  short sVar1;
  uint uVar2;
  ushort uVar3;
  ushort *puVar4;
  undefined4 uVar5;
  uint uVar6;

  sVar1 = *(short *)(param_1 + 0x32);
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(char *)(param_1 + 0x16) = (char)sVar1;
  *(short *)(param_1 + 0x32) = (short)(sVar1 * 0x3c);
  puVar4 = (ushort *)
           FUN_0204d634(*(undefined1 *)(param_1 + 0x52),*(ushort *)(param_1 + 0x36) & 0xff,
                        sVar1 * 0x3c,(int)sVar1,param_4);
  *(ushort **)(param_1 + 0x70) = puVar4;
  uVar2 = DAT_02062ce0;
  uVar6 = (uint)*puVar4;
  uVar3 = 0;
  if (uVar6 != 0xffff) {
    do {
      if (((uVar6 >> 0xd) + 0xfffe & 0xffff) < 2) {
        FUN_0204e49c((puVar4[3] & 0xfc0) + ((int)(puVar4[2] & 0xfc0) >> 6),(puVar4[2] & 0x3f) + 1,
                     (puVar4[3] & 0x3f) + 1);
      }
      puVar4 = puVar4 + 4;
      uVar6 = (uint)*puVar4;
      uVar3 = uVar3 + 1;
    } while (uVar6 != uVar2);
  }
  if ((*(byte *)(param_1 + 0x17) == 0) || (uVar3 < *(byte *)(param_1 + 0x17))) {
    *(char *)(param_1 + 0x17) = (char)uVar3;
  }
  if (*(char *)(param_1 + 10) == -1) {
    *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
    return;
  }
  uVar5 = FUN_020460e8();
  *(undefined4 *)(param_1 + 0x68) = uVar5;
  return;
}

/* ================================================================
 * FUN_02062ce4 @ 02062ce4
 * ================================================================ */

void FUN_02062ce4(int param_1)

{
  undefined4 uVar1;

  if (*(uint *)(param_1 + 0x68) ==
      (*(uint *)(param_1 + 0x68) & *(uint *)(DAT_02062d7c + (uint)*(byte *)(param_1 + 0x52) * 4))) {
    if (*(char *)(param_1 + 9) != 'p' && *(char *)(param_1 + 9) != 'q') {
      FUN_02076fe0(DAT_02062d80);
      FUN_02076fe0(DAT_02062d84);
    }
    uVar1 = DAT_02062d88;
    if (*(char *)(param_1 + 0x16) != '\0') {
      if (*(char *)(param_1 + 9) != 'p' && *(char *)(param_1 + 9) != 'q') {
        FUN_02076fe0(DAT_02062d88);
        *(short *)(DAT_02062d8c + 0x26) = (short)uVar1;
      }
      if (*(char *)(param_1 + 9) == '\t') {
        *(char *)(DAT_02062d8c + 0x52) = *(char *)(DAT_02062d8c + 0x52) + '\x01';
      }
    }
    *(undefined1 *)(param_1 + 0x14) = 2;
    return;
  }
  return;
}

/* ================================================================
 * FUN_02062d90 @ 02062d90
 * ================================================================ */

void FUN_02062d90(int param_1)

{
  uint uVar1;

  uVar1 = FUN_02084600(*(undefined2 *)(param_1 + 100));
  if (uVar1 < *(byte *)(param_1 + 0x17)) {
    FUN_02062f24(param_1,0);
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 3;
  return;
}

/* ================================================================
 * FUN_02062dc4 @ 02062dc4
 * ================================================================ */

void FUN_02062dc4(int param_1)

{
  uint uVar1;

  uVar1 = FUN_02084600(*(undefined2 *)(param_1 + 100));
  if (uVar1 < *(byte *)(param_1 + 0x17)) {
    FUN_02062f24(param_1,0x5a);
  }
  if (*(short *)(param_1 + 0x32) != 0) {
    *(short *)(param_1 + 0x32) = *(short *)(param_1 + 0x32) + -1;
    if (*(short *)(param_1 + 0x32) == 0) {
      *(undefined1 *)(param_1 + 0x14) = 4;
    }
    return;
  }
  return;
}

/* ================================================================
 * FUN_02062e30 @ 02062e30
 * ================================================================ */

void FUN_02062e30(int param_1)

{
  char cVar1;
  uint uVar2;
  uint uVar3;
  bool bVar4;

  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  if (*(byte *)(param_1 + 0xb) != 0xff) {
    *(uint *)(DAT_02062f14 + (uint)*(byte *)(param_1 + 0x52) * 4) =
         *(uint *)(DAT_02062f14 + (uint)*(byte *)(param_1 + 0x52) * 4) |
         *(uint *)(DAT_02062f18 + (*(byte *)(param_1 + 0xb) & 0x1f) * 4);
  }
  if (((*(byte *)(param_1 + 0xb) & 0x80) == 0) &&
     (*(char *)(param_1 + 9) != 'o' && *(char *)(param_1 + 9) != 'p')) {
    FUN_02076fe0(0x10);
  }
  uVar2 = DAT_02062f1c;
  if (*(char *)(param_1 + 0x16) != '\0') {
    if ((*(char *)(DAT_02062f1c + 0x52) != '\0') && (*(char *)(param_1 + 9) == '\t')) {
      *(char *)(DAT_02062f1c + 0x52) = *(char *)(DAT_02062f1c + 0x52) + -1;
    }
    uVar3 = (uint)*(byte *)(param_1 + 9);
    if (uVar3 == 9) {
      uVar2 = (uint)*(byte *)(DAT_02062f1c + 0x52);
    }
    if (uVar3 != 9 || uVar2 != 0) {
      bVar4 = uVar3 != 0x71;
      if (!bVar4) {
        uVar3 = (uint)*(ushort *)(DAT_02062f1c + 0x26);
        uVar2 = DAT_02062f20;
      }
      if (bVar4 || uVar3 != uVar2) goto LAB_02062f08;
    }
    *(undefined2 *)(DAT_02062f1c + 0x26) = 0;
    FUN_020497e4();
  }
LAB_02062f08:
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02062f24 @ 02062f24
 * ================================================================ */

void FUN_02062f24(int param_1,undefined1 param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;

  iVar2 = FUN_02084634(*(undefined2 *)(param_1 + 100));
  iVar3 = FUN_0204bcc8();
  if (iVar3 == 0) {
    return;
  }
  iVar4 = FUN_02085024(iVar3,*(int *)(param_1 + 0x70) + iVar2 * 8,*(undefined1 *)(param_1 + 0x52));
  iVar1 = DAT_02062fc8;
  if (iVar4 == 0) {
    return;
  }
  *(undefined1 *)(iVar3 + 0x70) = *(undefined1 *)(iVar3 + 9);
  uVar5 = *(undefined4 *)(iVar1 + iVar2 * 4);
  *(undefined1 *)(iVar3 + 0x71) = *(undefined1 *)(iVar3 + 10);
  *(undefined1 *)(iVar3 + 0x72) = *(undefined1 *)(iVar3 + 0xb);
  *(int *)(iVar3 + 0x60) = param_1;
  *(char *)(iVar3 + 0x68) = (char)iVar2 + '\x01';
  *(undefined1 *)(iVar3 + 0x52) = *(undefined1 *)(param_1 + 0x52);
  *(undefined1 *)(iVar3 + 8) = 6;
  *(undefined1 *)(iVar3 + 9) = 10;
  *(undefined1 *)(iVar3 + 0x18) = param_2;
  *(short *)(param_1 + 0x62) = *(short *)(param_1 + 0x62) + 1;
  *(ushort *)(param_1 + 100) = *(ushort *)(param_1 + 100) | (ushort)uVar5;
  return;
}

/* ================================================================
 * FUN_02062fcc @ 02062fcc
 * ================================================================ */

void FUN_02062fcc(void)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;

  iVar1 = DAT_02063058;
  if (*DAT_02063050 != '\0') {
    uVar4 = 0;
    if ((*DAT_02063054 & 0x1f) == 0) {
      *(byte *)(DAT_02063058 + 0x6c) = *(char *)(DAT_02063058 + 0x6c) + 1U & 3;
    }
    do {
      iVar3 = iVar1 + (*(byte *)(iVar1 + 0x6c) + uVar4 & 3) * 4;
      iVar2 = *(int *)(iVar3 + 0x70);
      if (iVar2 != 0) {
        *(uint *)(iVar2 + 0x1c) = *(uint *)(iVar2 + 0x1c) & 0xfffffffc | 2;
        FUN_020857c8(*(undefined4 *)(iVar3 + 0x70));
      }
      uVar4 = uVar4 + 1 & 0xff;
    } while (uVar4 < 4);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206305c @ 0206305c
 * ================================================================ */

void FUN_0206305c(int param_1)

{
  int iVar1;

  if (*(char *)(param_1 + 10) == '\0') {
    FUN_020630b0(param_1);
    return;
  }
  FUN_020633bc(param_1);
  iVar1 = param_1;
  if ((*(uint *)(param_1 + 0x1c) & 3) == 0) {
    iVar1 = 0;
  }
  *(int *)(DAT_020630ac + (uint)*(byte *)(param_1 + 0xb) * 4) = iVar1;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
  return;
}

/* ================================================================
 * FUN_020630b0 @ 020630b0
 * ================================================================ */

void FUN_020630b0(int param_1)

{
  (**(code **)(DAT_020630c8 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_020633bc @ 020633bc
 * ================================================================ */

void FUN_020633bc(int param_1)

{
  (**(code **)(DAT_020633fc + (uint)*(byte *)(param_1 + 0x14) * 4))();
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 3;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3fff | 0x40000000;
  return;
}

/* ================================================================
 * FUN_02063450 @ 02063450
 * ================================================================ */

void FUN_02063450(int param_1)

{
  byte bVar1;
  short sVar2;
  short sVar3;
  int iVar4;
  char *pcVar5;
  short sVar6;
  undefined2 uVar7;
  uint uVar8;

  FUN_0208da4c();
  iVar4 = FUN_0208de54();
  if ((iVar4 == 0) && (pcVar5 = (char *)FUN_02093b5c(), *(char *)(param_1 + 0xb) == *pcVar5)) {
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
    return;
  }
  iVar4 = *(int *)(param_1 + 0x58);
  bVar1 = *(byte *)(*(int *)(param_1 + 0x54) + (uint)*(byte *)(param_1 + 0xb) + 0x68);
  uVar8 = (uint)bVar1;
  if (uVar8 == 0xff) {
    uVar8 = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
    goto LAB_020635a4;
  }
  if ((bVar1 & 1) != 0) {
    sVar6 = *(short *)(DAT_020635b4 + (uVar8 & 0xe) * 2);
    sVar3 = *(short *)(DAT_020635b4 + ((uVar8 & 0xe) + 1) * 2);
    goto LAB_0206350c;
  }
  switch(uVar8) {
  case 0:
    sVar6 = *(short *)(iVar4 + 0x32);
    sVar2 = *(short *)(DAT_020635b8 + 0x20);
    sVar3 = 6;
    goto LAB_02063508;
  case 1:
    break;
  case 2:
    uVar7 = 0xfa;
    goto LAB_02063534;
  case 3:
    break;
  case 4:
    sVar6 = *(short *)(iVar4 + 0x32);
    sVar2 = *(short *)(DAT_020635b8 + 0x20);
    sVar3 = 0xba;
LAB_02063508:
    sVar6 = sVar6 - sVar2;
LAB_0206350c:
    *(short *)(param_1 + 0x32) = sVar6;
LAB_02063548:
    *(short *)(param_1 + 0x36) = sVar3;
    break;
  case 5:
    break;
  case 6:
    uVar7 = 6;
LAB_02063534:
    *(undefined2 *)(param_1 + 0x32) = uVar7;
    sVar3 = *(short *)(iVar4 + 0x36) - *(short *)(DAT_020635b8 + 0x22);
    goto LAB_02063548;
  }
  if (*(short *)(param_1 + 0x36) < 6) {
    *(undefined2 *)(param_1 + 0x36) = 6;
  }
  if (0xba < *(short *)(param_1 + 0x36)) {
    *(undefined2 *)(param_1 + 0x36) = 0xba;
  }
  if (*(short *)(param_1 + 0x32) < 6) {
    *(undefined2 *)(param_1 + 0x32) = 6;
  }
  if (0xfa < *(short *)(param_1 + 0x32)) {
    *(undefined2 *)(param_1 + 0x32) = 0xfa;
  }
  uVar8 = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 2;
LAB_020635a4:
  *(uint *)(param_1 + 0x1c) = uVar8;
  FUN_02085980(param_1);
  return;
}

/* ================================================================
 * FUN_020635d8 @ 020635d8
 * ================================================================ */

void FUN_020635d8(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined2 *puVar3;

  uVar2 = DAT_02063670;
  uVar1 = DAT_0206366c;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(undefined1 *)(param_1 + 0x2e) = 0;
  *(undefined1 *)(param_1 + 0x3c) = 0xd8;
  *(undefined1 *)(param_1 + 0x46) = 0xf4;
  *(undefined1 *)(param_1 + 0x40) = 0x35;
  *(undefined1 *)(param_1 + 0x3f) = 0xd;
  *(undefined4 *)(param_1 + 0x48) = uVar1;
  *(short *)(param_1 + 0x5c) = (short)uVar2;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  FUN_02085cf8();
  FUN_020859e0(param_1,0);
  puVar3 = DAT_02063674;
  *DAT_02063674 = 0x100;
  puVar3[1] = 0x100;
  puVar3[2] = 0;
  FUN_0205063c(puVar3,param_1);
  FUN_02084180(param_1);
  return;
}

/* ================================================================
 * FUN_02063678 @ 02063678
 * ================================================================ */

void FUN_02063678(int param_1)

{
  char cVar1;
  undefined2 uVar2;
  undefined2 *puVar3;
  uint uVar4;
  int iVar5;

  if ((*(byte *)(param_1 + 0x41) & 0x80) != 0) {
    if ((*(byte *)(param_1 + 0x3c) & 0x80) == 0) {
      *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
      *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 3;
      FUN_02085d34();
      FUN_020859e0(param_1,1);
      FUN_02085f80(0x56);
      return;
    }
    iVar5 = *(int *)(param_1 + 0x4c);
    if ((iVar5 != 0) && (*(char *)(iVar5 + 8) == '\x01')) {
      iVar5 = (short)(ushort)*(byte *)(iVar5 + 10) * 0x94;
      if (((*(byte *)(DAT_020637b8 + iVar5 + 0x8b) & 0x7f) >> 4 != 0) &&
         (*(char *)(DAT_020637bc + iVar5) == '\0')) goto LAB_0206372c;
    }
    FUN_02085f80(0x5b);
    *(undefined1 *)(param_1 + 0x17) = 0x40;
  }
LAB_0206372c:
  cVar1 = *(char *)(param_1 + 0x17);
  if (cVar1 == '\0') {
    return;
  }
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  uVar4 = *(byte *)(param_1 + 0x18) + 1;
  *(char *)(param_1 + 0x18) = (char)uVar4;
  puVar3 = DAT_020637c4;
  if (cVar1 != '\x01') {
    uVar4 = uVar4 & 0xe;
    uVar2 = *(undefined2 *)(DAT_020637c0 + (uVar4 + 1) * 2);
    *DAT_020637c4 = *(undefined2 *)(DAT_020637c0 + uVar4 * 2);
    puVar3[1] = uVar2;
    puVar3[2] = 0;
    FUN_0205063c(puVar3,param_1);
    return;
  }
  *DAT_020637c4 = 0x100;
  puVar3[1] = 0x100;
  puVar3[2] = 0;
  FUN_0205063c(puVar3,param_1);
  return;
}

/* ================================================================
 * FUN_020637c8 @ 020637c8
 * ================================================================ */

void FUN_020637c8(int param_1)

{
  FUN_02085980();
  if ((*(byte *)(param_1 + 0x29) & 0x80) == 0) {
    return;
  }
  FUN_020841b0(param_1);
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_020637f4 @ 020637f4
 * ================================================================ */

void FUN_020637f4(int param_1)

{
  if ((*(char *)(param_1 + 0x74) != '\0') &&
     ((*(uint *)(DAT_02063868 + (*(byte *)(param_1 + 10) & 0x1f) * 4) &
      *(uint *)(DAT_02063864 + (uint)*(byte *)(param_1 + 0x52) * 4)) == 0)) {
    *(undefined1 *)(param_1 + 0x74) = 0;
  }
  if (*(char *)(param_1 + 0x14) != '\x04') {
    FUN_0206402c(param_1);
  }
  (**(code **)(DAT_0206386c + (uint)*(byte *)(param_1 + 0x14) * 4))(param_1);
  *(undefined4 *)(param_1 + 0x60) = 0;
  return;
}

/* ================================================================
 * FUN_02063870 @ 02063870
 * ================================================================ */

void FUN_02063870(int param_1)

{
  ushort uVar1;
  undefined4 uVar2;
  char cVar3;
  uint uVar4;
  int iVar5;
  uint uVar6;
  int iVar7;

  if (*(char *)(param_1 + 0x17) == '\0') {
    cVar3 = *(char *)(param_1 + 0x14) + '\x01';
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  }
  else {
    cVar3 = '\x04';
  }
  *(char *)(param_1 + 0x14) = cVar3;
  iVar5 = DAT_02063998;
  uVar2 = DAT_02063994;
  *(undefined1 *)(param_1 + 0x17) = 0x1e;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 1;
  *(undefined1 *)(param_1 + 0x2e) = 0;
  *(undefined4 *)(param_1 + 0x48) = uVar2;
  *(uint *)(param_1 + 0xc) = (*(byte *)(param_1 + 0xb) + 1) * 0x40;
  *(undefined2 *)(param_1 + 0x7a) = *(undefined2 *)(param_1 + 0x36);
  *(undefined2 *)(param_1 + 0x78) = *(undefined2 *)(param_1 + 0x32);
  uVar6 = (uint)*(byte *)(iVar5 + 1);
  if (uVar6 < 5) {
    uVar1 = *(ushort *)(DAT_020639a4 + (uVar6 * 2 + 1) * 2);
    uVar4 = *(uint *)(param_1 + 0x1c);
    iVar7 = DAT_020639a4;
  }
  else {
    uVar4 = *(uint *)(param_1 + 0x1c);
    iVar5 = *(int *)(*(int *)(DAT_0206399c + uVar6 * 4) + (uint)*(byte *)(iVar5 + 0x48) * 8);
    uVar1 = *(ushort *)(DAT_020639a0 + ((uint)*(byte *)(iVar5 + 1) * 2 + 1) * 2);
    iVar7 = DAT_020639a0;
  }
  *(uint *)(param_1 + 0x1c) = uVar4 & 0xff00ffff | (uVar1 & 0xf) << 0x14 | (uVar1 & 0xf) << 0x10;
  FUN_02081088(param_1,*(undefined2 *)(iVar7 + (uint)*(byte *)(iVar5 + 1) * 4));
  FUN_020859e0(param_1,0);
  return;
}

/* ================================================================
 * FUN_020639a8 @ 020639a8
 * ================================================================ */

void FUN_020639a8(int param_1)

{
  uint uVar1;
  char cVar2;

  if ((((*(byte *)(param_1 + 10) & 0x80) != 0) && (*(char *)(param_1 + 0x74) == '\0')) &&
     (((int)*(short *)(param_1 + 0x32) != (uint)*(ushort *)(param_1 + 0x78) ||
      ((int)*(short *)(param_1 + 0x36) != (uint)*(ushort *)(param_1 + 0x7a))))) {
    if (((*(uint *)(DAT_02063aa8 + (*(byte *)(param_1 + 10) & 0x1f) * 4) &
         *(uint *)(DAT_02063aa4 + (uint)*(byte *)(param_1 + 0x52) * 4)) != 0) &&
       ((*(uint *)(param_1 + 0x60) & 0xf) == 0)) {
      *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
      *(undefined1 *)(param_1 + 0x17) = 0x2d;
      *(undefined1 *)(param_1 + 0x74) = 1;
      FUN_020859e0(param_1);
      return;
    }
  }
  uVar1 = (*(uint *)(param_1 + 0x60) & 0xf0) >> 4;
  cVar2 = *(char *)(DAT_02063aac + uVar1);
  *(char *)(param_1 + 0x2b) = cVar2;
  if (cVar2 != -1) {
    FUN_02063bac(param_1);
  }
  if ((uVar1 != *(byte *)(param_1 + 0x75)) && ((uVar1 & (uVar1 ^ *(byte *)(param_1 + 0x75))) != 0))
  {
    FUN_02085f80(0x12);
  }
  *(char *)(param_1 + 0x75) = (char)uVar1;
  FUN_020859e0(param_1,uVar1 + 1);
  return;
}

/* ================================================================
 * FUN_02063bac @ 02063bac
 * ================================================================ */

void FUN_02063bac(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  uint uVar8;
  uint uVar9;

  iVar4 = DAT_02063fd4;
  uVar6 = FUN_02056f30(*(undefined2 *)(param_1 + 0x50));
  uVar8 = *(uint *)(param_1 + 0x60);
  uVar9 = uVar8 & 0xf;
  *(short *)(param_1 + 0x7c) = *(short *)(param_1 + 0x32);
  *(short *)(param_1 + 0x7e) = *(short *)(param_1 + 0x36);
  *(undefined2 *)(param_1 + 0x2c) = 0;
  if (uVar9 == 0) {
    return;
  }
  iVar2 = (int)(uint)*(byte *)(param_1 + 0x2b) >> 2;
  switch(iVar2) {
  case 0:
    goto LAB_02063c28;
  case 1:
LAB_02063c28:
    FUN_0206412c(param_1,uVar6,(int)*(short *)(param_1 + 0x36),(int)*(short *)(param_1 + 0x32),
                 param_4);
    FUN_0208530c(param_1,*(undefined4 *)(param_1 + 0xc),0,*(undefined2 *)(param_1 + 0x2c));
    uVar3 = (uint)*(ushort *)(param_1 + 0x7e) - (int)*(short *)(param_1 + 0x36) & 0xff;
    iVar1 = uVar3 << 8;
    if (uVar3 != 0) {
      if ((uVar8 & 1) != 0) {
        FUN_020852f8(*(undefined4 *)(iVar4 + 0x28),iVar1,0);
      }
      if ((uVar8 & 2) != 0) {
        FUN_020852f8(*(undefined4 *)(iVar4 + 0xbc),iVar1,0);
      }
      if ((uVar8 & 4) != 0) {
        FUN_020852f8(*(undefined4 *)(iVar4 + 0x150),iVar1,0);
      }
      if ((uVar8 & 8) != 0) {
        FUN_020852f8(*(undefined4 *)(iVar4 + 0x1e4),iVar1,0);
      }
    }
    uVar7 = FUN_02093b5c();
    FUN_02099704(uVar7,param_1,uVar9,iVar1,0);
    if (iVar2 != 1) {
      return;
    }
    break;
  case 2:
    break;
  case 3:
    break;
  case 4:
    goto LAB_02063da0;
  case 5:
    goto LAB_02063da0;
  case 6:
    goto LAB_02063e5c;
  case 7:
    goto LAB_02063e5c;
  default:
    return;
  }
  FUN_020642fc(param_1,uVar6);
  FUN_0208530c(param_1,*(undefined4 *)(param_1 + 0xc),8,*(undefined2 *)(param_1 + 0x2c));
  uVar3 = (int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(param_1 + 0x7c) & 0xff;
  iVar1 = uVar3 << 8;
  if (uVar3 != 0) {
    if ((uVar8 & 1) != 0) {
      FUN_020852f8(*(undefined4 *)(iVar4 + 0x28),iVar1,8);
    }
    if ((uVar8 & 2) != 0) {
      FUN_020852f8(*(undefined4 *)(iVar4 + 0xbc),iVar1,8);
    }
    if ((uVar8 & 4) != 0) {
      FUN_020852f8(*(undefined4 *)(iVar4 + 0x150),iVar1,8);
    }
    if ((uVar8 & 8) != 0) {
      FUN_020852f8(*(undefined4 *)(iVar4 + 0x1e4),iVar1,8);
    }
  }
  uVar7 = FUN_02093b5c();
  FUN_02099704(uVar7,param_1,uVar9,iVar1,8);
  if (iVar2 != 3) {
    return;
  }
LAB_02063da0:
  FUN_02064214(param_1,uVar6);
  FUN_0208530c(param_1,*(undefined4 *)(param_1 + 0xc),0x10,*(undefined2 *)(param_1 + 0x2c));
  uVar3 = (int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(param_1 + 0x7e) & 0xff;
  iVar1 = uVar3 << 8;
  if (uVar3 != 0) {
    if ((uVar8 & 1) != 0) {
      FUN_020852f8(*(undefined4 *)(iVar4 + 0x28),iVar1,0x10);
    }
    if ((uVar8 & 2) != 0) {
      FUN_020852f8(*(undefined4 *)(iVar4 + 0xbc),iVar1,0x10);
    }
    if ((uVar8 & 4) != 0) {
      FUN_020852f8(*(undefined4 *)(iVar4 + 0x150),iVar1,0x10);
    }
    if ((uVar8 & 8) != 0) {
      FUN_020852f8(*(undefined4 *)(iVar4 + 0x1e4),iVar1,0x10);
    }
  }
  uVar7 = FUN_02093b5c();
  FUN_02099704(uVar7,param_1,uVar9,iVar1,0x10);
  if (iVar2 != 5) {
    return;
  }
LAB_02063e5c:
  FUN_020643e4(param_1,uVar6);
  FUN_0208530c(param_1,*(undefined4 *)(param_1 + 0xc),0x18,*(undefined2 *)(param_1 + 0x2c));
  iVar5 = DAT_02063fd4;
  uVar3 = (uint)*(ushort *)(param_1 + 0x7c) - (int)*(short *)(param_1 + 0x32) & 0xff;
  iVar1 = uVar3 << 8;
  if (uVar3 != 0) {
    if ((uVar8 & 1) != 0) {
      FUN_020852f8(*(undefined4 *)(DAT_02063fd4 + 0x28),iVar1,0x18);
    }
    if ((uVar8 & 2) != 0) {
      FUN_020852f8(*(undefined4 *)(iVar5 + 0xbc),iVar1,0x18);
    }
    if ((uVar8 & 4) != 0) {
      FUN_020852f8(*(undefined4 *)(iVar5 + 0x150),iVar1,0x18);
    }
    if ((uVar8 & 8) != 0) {
      FUN_020852f8(*(undefined4 *)(iVar5 + 0x1e4),iVar1,0x18);
    }
  }
  uVar7 = FUN_02093b5c();
  FUN_02099704(uVar7,param_1,uVar9,iVar1,0x18);
  if (iVar2 == 7) {
    FUN_0206412c(param_1,uVar6);
    FUN_0208530c(param_1,*(undefined4 *)(param_1 + 0xc),0,*(undefined2 *)(param_1 + 0x2c));
    uVar3 = (uint)*(ushort *)(param_1 + 0x7e) - (int)*(short *)(param_1 + 0x36) & 0xff;
    iVar2 = uVar3 << 8;
    if (uVar3 != 0) {
      if ((uVar8 & 1) != 0) {
        FUN_020852f8(*(undefined4 *)(iVar4 + 0x28),iVar2,0);
      }
      if ((uVar8 & 2) != 0) {
        FUN_020852f8(*(undefined4 *)(iVar4 + 0xbc),iVar2,0);
      }
      if ((uVar8 & 4) != 0) {
        FUN_020852f8(*(undefined4 *)(iVar4 + 0x150),iVar2,0);
      }
      if ((uVar8 & 8) != 0) {
        FUN_020852f8(*(undefined4 *)(iVar4 + 0x1e4),iVar2,0);
      }
    }
    uVar6 = FUN_02093b5c();
    FUN_02099704(uVar6,param_1,uVar9,iVar2,0);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02063fd8 @ 02063fd8
 * ================================================================ */

bool FUN_02063fd8(int param_1)

{
  int iVar1;

  if ((*(byte *)(param_1 + 0x8b) & 0x7f) >> 4 != 0) {
    return true;
  }
  if (*(short *)(param_1 + 8) != 0x1ac) {
    if (*(char *)(param_1 + 2) == '\0' || *(char *)(param_1 + 2) == '%') {
      iVar1 = FUN_020860dc(*(undefined4 *)(param_1 + 0x28));
      return iVar1 != 0;
    }
    return false;
  }
  return true;
}
