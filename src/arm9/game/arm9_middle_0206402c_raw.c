#include "game/arm9_middle_0206402c_raw.h"

/*
 * Raw matching-first recovery for the 32 catalog entries beginning at
 * 0x0206402c. These bodies remain address-named promotion candidates and are
 * not linked byte-matching evidence.
 *
 * Calls whose implicit r0 argument was omitted by Ghidra are written with the
 * owning param_1 so this mechanically faithful draft remains valid C.
 */

extern uint DAT_0206411c;
extern uint DAT_02064120;
extern uint DAT_02064124;
extern uint DAT_02064128;
extern uint DAT_020644e4;
extern uint DAT_020645a0;
extern uint DAT_02064618;
extern uint DAT_0206468c;
extern uint DAT_020647c8;
extern uint DAT_020647cc;
extern uint DAT_02064810;
extern uint DAT_02064878;
extern uint DAT_0206487c;
extern byte *DAT_02064c58;
extern uint DAT_02064c5c;
extern uint DAT_02064da4;
extern uint DAT_02064da8;
extern uint DAT_02064dac;
extern uint DAT_02064db0;
extern uint DAT_02064db4;
extern uint DAT_02064db8;
extern uint DAT_02064e14;
extern uint DAT_02064e70;
extern uint DAT_02064fbc;
extern uint DAT_0206509c;
extern uint DAT_020654b0;
extern uint DAT_020654b4;
extern uint DAT_020654b8;
extern uint DAT_020654bc;
extern uint DAT_0206564c;
extern uint DAT_020656d0;
extern uint DAT_02065758;
extern uint DAT_0206575c;
extern uint DAT_02065760;
extern uint DAT_02065824;
extern uint DAT_02065828;
extern uint DAT_0206582c;
extern uint DAT_02065830;
extern uint DAT_020658b0;
extern uint DAT_0206595c;
extern uint DAT_02065960;
extern uint DAT_02065964;
extern uint DAT_02065968;
extern uint DAT_02065f54;
extern uint DAT_02065f58;

/* ================================================================
 * FUN_0206402c @ 0206402c
 * ================================================================ */

void FUN_0206402c(int param_1)

{
  int iVar1;
  int iVar2;

  FUN_02084228();
  iVar2 = DAT_0206411c;
  if ((*(uint *)(param_1 + 0x60) & 1) != 0) {
    iVar1 = FUN_02063fd8(DAT_0206411c);
    if (iVar1 == 0) {
      *(uint *)(param_1 + 0x60) = *(uint *)(param_1 + 0x60) ^ 1;
    }
    else {
      FUN_02083f60(param_1,*(undefined4 *)(iVar2 + 0x28));
    }
  }
  if ((*(uint *)(param_1 + 0x60) & 2) != 0) {
    iVar2 = FUN_02063fd8(DAT_02064120);
    if (iVar2 == 0) {
      *(uint *)(param_1 + 0x60) = *(uint *)(param_1 + 0x60) ^ 2;
    }
    else {
      FUN_02083f60(param_1,*(undefined4 *)(DAT_0206411c + 0xbc));
    }
  }
  if ((*(uint *)(param_1 + 0x60) & 4) != 0) {
    iVar2 = FUN_02063fd8(DAT_02064124);
    if (iVar2 == 0) {
      *(uint *)(param_1 + 0x60) = *(uint *)(param_1 + 0x60) ^ 4;
    }
    else {
      FUN_02083f60(param_1,*(undefined4 *)(DAT_0206411c + 0x150));
    }
  }
  if ((*(uint *)(param_1 + 0x60) & 8) != 0) {
    iVar2 = FUN_02063fd8(DAT_02064128);
    if (iVar2 != 0) {
      FUN_02083f60(param_1,*(undefined4 *)(DAT_0206411c + 0x1e4));
      return;
    }
    *(uint *)(param_1 + 0x60) = *(uint *)(param_1 + 0x60) ^ 8;
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206412c @ 0206412c
 * ================================================================ */

void FUN_0206412c(int param_1)

{
  int iVar1;

  iVar1 = FUN_02083ccc(param_1,0xffffffeb,0xffffffe7);
  if ((iVar1 != 0x43 && iVar1 != 0x21) && (0xb < (iVar1 - 0x14U & 0xffff))) {
    *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 2;
  }
  iVar1 = FUN_02083ccc(param_1,0xfffffffb,0xffffffe7);
  if ((iVar1 != 0x43 && iVar1 != 0x21) && (0xb < (iVar1 - 0x14U & 0xffff))) {
    *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 2;
  }
  iVar1 = FUN_02083ccc(param_1,4,0xffffffe7);
  if ((iVar1 != 0x43 && iVar1 != 0x21) && (0xb < (iVar1 - 0x14U & 0xffff))) {
    *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 4;
  }
  iVar1 = FUN_02083ccc(param_1,0x14,0xffffffe7);
  if (iVar1 != 0x43 && iVar1 != 0x21) {
    if (0xb < (iVar1 - 0x14U & 0xffff)) {
      *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 4;
    }
    return;
  }
  return;
}

/* ================================================================
 * FUN_02064214 @ 02064214
 * ================================================================ */

void FUN_02064214(int param_1)

{
  int iVar1;

  iVar1 = FUN_02083ccc(param_1,0xffffffeb,0x18);
  if ((iVar1 != 0x43 && iVar1 != 0x21) && (0xb < (iVar1 - 0x14U & 0xffff))) {
    *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 0x20;
  }
  iVar1 = FUN_02083ccc(param_1,0xfffffffb,0x18);
  if ((iVar1 != 0x43 && iVar1 != 0x21) && (0xb < (iVar1 - 0x14U & 0xffff))) {
    *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 0x20;
  }
  iVar1 = FUN_02083ccc(param_1,4,0x18);
  if ((iVar1 != 0x43 && iVar1 != 0x21) && (0xb < (iVar1 - 0x14U & 0xffff))) {
    *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 0x40;
  }
  iVar1 = FUN_02083ccc(param_1,0x14,0x18);
  if (iVar1 != 0x43 && iVar1 != 0x21) {
    if (0xb < (iVar1 - 0x14U & 0xffff)) {
      *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 0x40;
    }
    return;
  }
  return;
}

/* ================================================================
 * FUN_020642fc @ 020642fc
 * ================================================================ */

void FUN_020642fc(int param_1)

{
  int iVar1;

  iVar1 = FUN_02083ccc(param_1,0x18,0xffffffeb);
  if ((iVar1 != 0x43 && iVar1 != 0x21) && (0xb < (iVar1 - 0x14U & 0xffff))) {
    *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 0x2000;
  }
  iVar1 = FUN_02083ccc(param_1,0x18,0xfffffffb);
  if ((iVar1 != 0x43 && iVar1 != 0x21) && (0xb < (iVar1 - 0x14U & 0xffff))) {
    *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 0x2000;
  }
  iVar1 = FUN_02083ccc(param_1,0x18,4);
  if ((iVar1 != 0x43 && iVar1 != 0x21) && (0xb < (iVar1 - 0x14U & 0xffff))) {
    *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 0x4000;
  }
  iVar1 = FUN_02083ccc(param_1,0x18,0x14);
  if (iVar1 != 0x43 && iVar1 != 0x21) {
    if (0xb < (iVar1 - 0x14U & 0xffff)) {
      *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 0x4000;
    }
    return;
  }
  return;
}

/* ================================================================
 * FUN_020643e4 @ 020643e4
 * ================================================================ */

void FUN_020643e4(int param_1)

{
  int iVar1;

  iVar1 = FUN_02083ccc(param_1,0xffffffe7,0xffffffeb);
  if ((iVar1 != 0x43 && iVar1 != 0x21) && (0xb < (iVar1 - 0x14U & 0xffff))) {
    *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 0x200;
  }
  iVar1 = FUN_02083ccc(param_1,0xffffffe7,0xfffffffb);
  if ((iVar1 != 0x43 && iVar1 != 0x21) && (0xb < (iVar1 - 0x14U & 0xffff))) {
    *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 0x200;
  }
  iVar1 = FUN_02083ccc(param_1,0xffffffe7,4);
  if ((iVar1 != 0x43 && iVar1 != 0x21) && (0xb < (iVar1 - 0x14U & 0xffff))) {
    *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 0x400;
  }
  iVar1 = FUN_02083ccc(param_1,0xffffffe7,0x14);
  if (iVar1 != 0x43 && iVar1 != 0x21) {
    if (0xb < (iVar1 - 0x14U & 0xffff)) {
      *(ushort *)(param_1 + 0x2c) = *(ushort *)(param_1 + 0x2c) | 0x400;
    }
    return;
  }
  return;
}

/* ================================================================
 * FUN_020644cc @ 020644cc
 * ================================================================ */

void FUN_020644cc(int param_1)

{
  (**(code **)(DAT_020644e4 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_020644e8 @ 020644e8
 * ================================================================ */

void FUN_020644e8(int param_1)

{
  undefined1 uVar1;
  undefined4 uVar2;
  int iVar3;

  *(byte *)(param_1 + 0x2a) = *(byte *)(param_1 + 0xb) & 2;
  FUN_020859e0();
  if ((*(byte *)(param_1 + 0xb) & 1) == 0) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0xff;
  }
  *(undefined1 *)(param_1 + 0x78) = uVar1;
  iVar3 = (int)(uint)*(byte *)(param_1 + 0xb) >> 4;
  *(char *)(param_1 + 0xb) = (char)iVar3;
  if (iVar3 != 0) {
    uVar2 = FUN_020460e8(*(undefined1 *)(param_1 + 10));
    *(undefined4 *)(param_1 + 0x60) = uVar2;
  }
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc;
  iVar3 = FUN_02056f30(*(undefined2 *)(param_1 + 0x50));
  *(int *)(param_1 + 0x7c) =
       iVar3 + 4 +
       (((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_020645a0 + 0x1a)) >> 4) << 6
       | (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_020645a0 + 0x18)) >> 4) * 2;
  FUN_02064770(param_1);
  return;
}

/* ================================================================
 * FUN_020645a4 @ 020645a4
 * ================================================================ */

void FUN_020645a4(int param_1)

{
  if (*(uint *)(param_1 + 0x60) ==
      (*(uint *)(param_1 + 0x60) & *(uint *)(DAT_02064618 + (uint)*(byte *)(param_1 + 0x52) * 4))) {
    if (*(char *)(param_1 + 0xb) == '\x01') {
      *(undefined4 *)(param_1 + 0x60) = 0;
    }
    *(undefined1 *)(param_1 + 0x14) = 2;
    *(undefined1 *)(param_1 + 0x18) = 8;
    FUN_020647d0(param_1);
    *(byte *)(param_1 + 0x2a) = *(char *)(param_1 + 0x2a) + *(char *)(param_1 + 0x78) & 3;
    FUN_020859e0(param_1);
    FUN_02085f80(0x57);
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206461c @ 0206461c
 * ================================================================ */

void FUN_0206461c(int param_1)

{
  int iVar1;
  char cVar2;

  cVar2 = *(char *)(param_1 + 0x18) + -1;
  *(char *)(param_1 + 0x18) = cVar2;
  iVar1 = DAT_0206468c;
  if (cVar2 != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 3;
  *(undefined1 *)(param_1 + 0x18) = *(undefined1 *)(param_1 + 0x17);
  *(ushort *)(param_1 + 0x76) =
       (ushort)*(undefined4 *)(param_1 + 0x60) &
       (ushort)*(undefined4 *)(iVar1 + (uint)*(byte *)(param_1 + 0x52) * 4);
  *(byte *)(param_1 + 0x2a) = *(char *)(param_1 + 0x2a) + *(char *)(param_1 + 0x78) & 3;
  FUN_020859e0();
  FUN_02064770(param_1);
  FUN_02085f80(0x57);
  return;
}

/* ================================================================
 * FUN_02064770 @ 02064770
 * ================================================================ */

void FUN_02064770(int param_1)

{
  int iVar1;
  undefined2 uVar2;
  undefined2 *puVar3;
  uint uVar4;

  puVar3 = *(undefined2 **)(param_1 + 0x7c);
  uVar4 = *(byte *)(param_1 + 0x2a) & 0xfffffffe;
  iVar1 = (int)(char)*(undefined2 *)(DAT_020647c8 + uVar4);
  uVar2 = *(undefined2 *)(DAT_020647cc + uVar4);
  *(undefined2 *)(param_1 + 0x70) = puVar3[iVar1];
  puVar3[iVar1] = uVar2;
  *puVar3 = uVar2;
  *(undefined2 *)(param_1 + 0x74) = puVar3[-iVar1];
  puVar3[-iVar1] = uVar2;
  return;
}

/* ================================================================
 * FUN_020647d0 @ 020647d0
 * ================================================================ */

void FUN_020647d0(int param_1)

{
  int iVar1;
  int iVar2;

  iVar2 = *(int *)(param_1 + 0x7c);
  iVar1 = (int)(char)*(undefined2 *)(DAT_02064810 + (*(byte *)(param_1 + 0x2a) & 0xfffffffe));
  *(undefined2 *)(iVar2 + iVar1 * 2) = *(undefined2 *)(param_1 + 0x70);
  *(undefined2 *)(iVar2 + iVar1 * -2) = *(undefined2 *)(param_1 + 0x74);
  return;
}

/* ================================================================
 * FUN_02064814 @ 02064814
 * ================================================================ */

bool FUN_02064814(int param_1)

{
  int iVar1;
  short sVar2;
  short *psVar3;
  uint uVar4;

  psVar3 = *(short **)(param_1 + 0x7c);
  uVar4 = *(byte *)(param_1 + 0x2a) & 0xfffffffe;
  iVar1 = (int)(char)*(undefined2 *)(DAT_02064878 + uVar4);
  sVar2 = *(short *)(DAT_0206487c + uVar4);
  if (sVar2 != psVar3[iVar1]) {
    return true;
  }
  if (sVar2 == *psVar3) {
    return sVar2 != psVar3[-iVar1];
  }
  return true;
}

/* ================================================================
 * FUN_02064bf4 @ 02064bf4
 * ================================================================ */

void FUN_02064bf4(int param_1)

{
  uint uVar1;

  if (*(int *)(param_1 + 0x70) != 0) {
    uVar1 = *(int *)(param_1 + 0x70) - 1;
    *(uint *)(param_1 + 0x70) = uVar1;
    if (uVar1 == 0) {
      FUN_0204be88();
      return;
    }
    if (uVar1 < 0x10) {
      *(uint *)(param_1 + 0x1c) =
           *(uint *)(param_1 + 0x1c) & 0xfffffffc | (int)(*DAT_02064c58 & 2) >> 1;
    }
  }
  (**(code **)(DAT_02064c5c + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_02064c60 @ 02064c60
 * ================================================================ */

void FUN_02064c60(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  ushort uVar1;
  ushort *puVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  undefined4 uVar6;
  int iVar7;
  bool bVar8;

  uVar6 = *(undefined4 *)(DAT_02064da4 + (uint)*(byte *)(param_1 + 10) * 4);
  iVar5 = *(byte *)(param_1 + 0x14) + 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc | 1;
  *(undefined1 *)(param_1 + 0x2e) = 0;
  *(undefined4 *)(param_1 + 0x48) = uVar6;
  *(undefined4 *)(param_1 + 0xc) = 0x100;
  *(char *)(param_1 + 0x14) = (char)iVar5;
  puVar2 = (ushort *)
           FUN_0204d634(*(undefined1 *)(param_1 + 0x52),*(undefined1 *)(param_1 + 0xb),iVar5,0x100,
                        param_4);
  *(ushort **)(param_1 + 0x78) = puVar2;
  *(ushort **)(param_1 + 0x6c) = puVar2;
  *(undefined4 *)(param_1 + 0x70) = 0;
  bVar8 = *puVar2 == DAT_02064da8;
  uVar4 = DAT_02064da8;
  if (bVar8) {
    uVar4 = (uint)puVar2[1];
    *(ushort **)(param_1 + 0x78) = puVar2 + 2;
    *(ushort **)(param_1 + 0x6c) = puVar2 + 2;
  }
  if (bVar8) {
    *(uint *)(param_1 + 0x70) = uVar4;
  }
  if (*(char *)(param_1 + 0x17) == '\0') {
    uVar6 = 0;
  }
  else {
    uVar6 = FUN_020460e8();
  }
  iVar5 = DAT_02064dac;
  *(undefined4 *)(param_1 + 0x60) = uVar6;
  uVar4 = (uint)*(byte *)(iVar5 + 1);
  if (uVar4 < 5) {
    uVar1 = *(ushort *)(DAT_02064db8 + (uVar4 * 2 + 1) * 2);
    uVar3 = *(uint *)(param_1 + 0x1c);
    iVar7 = DAT_02064db8;
  }
  else {
    uVar3 = *(uint *)(param_1 + 0x1c);
    iVar5 = *(int *)(*(int *)(DAT_02064db0 + uVar4 * 4) + (uint)*(byte *)(iVar5 + 0x48) * 8);
    uVar1 = *(ushort *)(DAT_02064db4 + ((uint)*(byte *)(iVar5 + 1) * 2 + 1) * 2);
    iVar7 = DAT_02064db4;
  }
  *(uint *)(param_1 + 0x1c) = uVar3 & 0xff00ffff | (uVar1 & 0xf) << 0x14 | (uVar1 & 0xf) << 0x10;
  FUN_02081088(param_1,*(undefined2 *)(iVar7 + (uint)*(byte *)(iVar5 + 1) * 4));
  FUN_02064fc0(param_1);
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10));
  return;
}

/* ================================================================
 * FUN_02064dbc @ 02064dbc
 * ================================================================ */

void FUN_02064dbc(int param_1)

{
  if (*(uint *)(param_1 + 0x60) !=
      (*(uint *)(param_1 + 0x60) & *(uint *)(DAT_02064e14 + (uint)*(byte *)(param_1 + 0x52) * 4))) {
    return;
  }
  *(undefined4 *)(param_1 + 0x60) = 0;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  if (*(char *)(param_1 + 9) == 's') {
    return;
  }
  FUN_020768d8();
  return;
}

/* ================================================================
 * FUN_02064e18 @ 02064e18
 * ================================================================ */

void FUN_02064e18(int param_1)

{
  int iVar1;
  undefined4 uVar2;

  if (*(byte *)(param_1 + 10) < 9) {
    FUN_02084228();
  }
  else {
    FUN_02084200();
    uVar2 = *(undefined4 *)(param_1 + 0x60);
    iVar1 = FUN_02093b5c();
    *(byte *)(iVar1 + 0xd) = *(byte *)(iVar1 + 0xd) | (byte)uVar2;
  }
  (**(code **)(DAT_02064e70 + (uint)*(byte *)(param_1 + 0x15) * 4))(param_1);
  *(undefined4 *)(param_1 + 0x60) = 0;
  return;
}

/* ================================================================
 * FUN_02064e74 @ 02064e74
 * ================================================================ */

void FUN_02064e74(int param_1)

{
  FUN_020650f4(param_1);
  if ((int)(uint)*(ushort *)(param_1 + 0x74) < (int)*(short *)(param_1 + 0x36)) {
    return;
  }
  *(ushort *)(param_1 + 0x36) = *(ushort *)(param_1 + 0x74);
  FUN_02064fc0(param_1);
  return;
}

/* ================================================================
 * FUN_02064ea0 @ 02064ea0
 * ================================================================ */

void FUN_02064ea0(int param_1)

{
  FUN_020650f4(param_1);
  if ((int)*(short *)(param_1 + 0x32) < (int)(uint)*(ushort *)(param_1 + 0x74)) {
    return;
  }
  *(ushort *)(param_1 + 0x32) = *(ushort *)(param_1 + 0x74);
  FUN_02064fc0(param_1);
  return;
}

/* ================================================================
 * FUN_02064ecc @ 02064ecc
 * ================================================================ */

void FUN_02064ecc(int param_1)

{
  FUN_020650f4(param_1);
  if ((int)*(short *)(param_1 + 0x36) < (int)(uint)*(ushort *)(param_1 + 0x74)) {
    return;
  }
  *(ushort *)(param_1 + 0x36) = *(ushort *)(param_1 + 0x74);
  FUN_02064fc0(param_1);
  return;
}

/* ================================================================
 * FUN_02064ef8 @ 02064ef8
 * ================================================================ */

void FUN_02064ef8(int param_1)

{
  FUN_020650f4(param_1);
  if ((int)(uint)*(ushort *)(param_1 + 0x74) < (int)*(short *)(param_1 + 0x32)) {
    return;
  }
  *(ushort *)(param_1 + 0x32) = *(ushort *)(param_1 + 0x74);
  FUN_02064fc0(param_1);
  return;
}

/* ================================================================
 * FUN_02064f24 @ 02064f24
 * ================================================================ */

void FUN_02064f24(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;

  iVar3 = *(int *)(param_1 + 0x68);
  *(int *)(param_1 + 0x68) = iVar3 + -1;
  if (iVar3 == 0) {
    FUN_02064fc0(param_1);
  }
  uVar4 = *(uint *)(param_1 + 0x60) & 0xf;
  FUN_0208da4c();
  iVar1 = FUN_0208de54();
  iVar3 = DAT_02064fbc;
  if (iVar1 == 0) {
    if (uVar4 != 0) {
      uVar5 = 0;
      do {
        if (((uVar4 & 1 << (uVar5 & 0xff)) != 0) &&
           (iVar1 = FUN_020650a0(uVar5 * 0x94 + iVar3), iVar1 == 0)) {
          uVar4 = uVar4 & ~(1 << (uVar5 & 0xff)) & 0xffU;
        }
        uVar5 = uVar5 + 1;
      } while (uVar5 < 2);
      uVar2 = FUN_02093b5c();
      FUN_02099704(uVar2,param_1,uVar4,0,0);
      return;
    }
    return;
  }
  return;
}

/* ================================================================
 * FUN_02064fc0 @ 02064fc0
 * ================================================================ */

void FUN_02064fc0(int param_1)

{
  int iVar1;
  ushort uVar2;
  uint uVar3;
  short sVar4;
  ushort *puVar5;
  ushort *puVar6;

  uVar3 = DAT_0206509c;
  if (**(ushort **)(param_1 + 0x78) == DAT_0206509c) {
    if (*(char *)(param_1 + 9) == 't') {
      *(undefined1 *)(param_1 + 0x15) = 4;
      *(uint *)(param_1 + 0x68) = uVar3;
      return;
    }
    *(undefined4 *)(param_1 + 0x78) = *(undefined4 *)(param_1 + 0x6c);
  }
  puVar5 = *(ushort **)(param_1 + 0x78);
  uVar2 = *puVar5;
  if ((uVar2 & 0x8000) != 0) {
    *(undefined1 *)(param_1 + 0x15) = 4;
    *(uint *)(param_1 + 0x68) = (uint)puVar5[1];
    goto LAB_0206508c;
  }
  iVar1 = (int)(uVar2 & 0xff) >> 3;
  *(char *)(param_1 + 0x15) = (char)iVar1;
  *(char *)(param_1 + 0x2b) = (char)uVar2;
  puVar6 = puVar5 + 1;
  switch(iVar1) {
  case 0:
    sVar4 = *(short *)(param_1 + 0x36);
    break;
  case 1:
    sVar4 = *(short *)(param_1 + 0x32);
    goto LAB_02065054;
  case 2:
    sVar4 = *(short *)(param_1 + 0x36);
LAB_02065054:
    sVar4 = sVar4 + puVar5[1];
    goto LAB_0206507c;
  case 3:
    sVar4 = *(short *)(param_1 + 0x32);
    break;
  default:
    goto switchD_02065030_default;
  }
  sVar4 = sVar4 - puVar5[1];
LAB_0206507c:
  puVar6 = puVar5 + 2;
  *(short *)(param_1 + 0x74) = sVar4;
switchD_02065030_default:
  if (*puVar6 != 0) {
    *(uint *)(param_1 + 0xc) = (uint)*puVar6;
  }
LAB_0206508c:
  *(int *)(param_1 + 0x78) = *(int *)(param_1 + 0x78) + 8;
  return;
}

/* ================================================================
 * FUN_020650a0 @ 020650a0
 * ================================================================ */

bool FUN_020650a0(int param_1)

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

/* ================================================================
 * FUN_020650f4 @ 020650f4
 * ================================================================ */

void FUN_020650f4(int param_1)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;

  iVar3 = DAT_020654b0;
  uVar7 = *(uint *)(param_1 + 0x60);
  uVar1 = (int)(uint)*(byte *)(param_1 + 0x2b) >> 3;
  uVar8 = uVar7 & 0xf;
  iVar9 = 0;
  *(undefined2 *)(param_1 + 0x7c) = *(undefined2 *)(param_1 + 0x32);
  *(undefined2 *)(param_1 + 0x7e) = *(undefined2 *)(param_1 + 0x36);
  switch(uVar1) {
  case 0:
    FUN_0204fd40();
    uVar2 = (uint)*(ushort *)(param_1 + 0x7e) - (int)*(short *)(param_1 + 0x36) & 0xff;
    iVar9 = uVar2 << 8;
    if (uVar2 == 0) goto switchD_02065130_default;
    if (((uVar7 & 1) != 0) && (iVar4 = FUN_020650a0(iVar3), iVar4 != 0)) {
      FUN_020852f8(*(undefined4 *)(iVar3 + 0x28),iVar9,0);
    }
    if (((uVar7 & 2) != 0) && (iVar4 = FUN_020650a0(DAT_020654b4), iVar4 != 0)) {
      FUN_020852f8(*(undefined4 *)(iVar3 + 0xbc),iVar9,0);
    }
    if (((uVar7 & 4) != 0) && (iVar4 = FUN_020650a0(DAT_020654b8), iVar4 != 0)) {
      FUN_020852f8(*(undefined4 *)(iVar3 + 0x150),iVar9,0);
    }
    if (((uVar7 & 8) == 0) || (iVar4 = FUN_020650a0(DAT_020654bc), iVar4 == 0))
    goto switchD_02065130_default;
    uVar5 = *(undefined4 *)(iVar3 + 0x1e4);
    uVar6 = 0;
    break;
  case 1:
    FUN_0204fd40();
    uVar2 = (int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(param_1 + 0x7c) & 0xff;
    iVar9 = uVar2 << 8;
    if (uVar2 == 0) goto switchD_02065130_default;
    if (((uVar7 & 1) != 0) && (iVar4 = FUN_020650a0(iVar3), iVar4 != 0)) {
      FUN_020852f8(*(undefined4 *)(iVar3 + 0x28),iVar9,8);
    }
    if (((uVar7 & 2) != 0) && (iVar4 = FUN_020650a0(DAT_020654b4), iVar4 != 0)) {
      FUN_020852f8(*(undefined4 *)(iVar3 + 0xbc),iVar9,8);
    }
    if (((uVar7 & 4) != 0) && (iVar4 = FUN_020650a0(DAT_020654b8), iVar4 != 0)) {
      FUN_020852f8(*(undefined4 *)(iVar3 + 0x150),iVar9,8);
    }
    if (((uVar7 & 8) == 0) || (iVar4 = FUN_020650a0(DAT_020654bc), iVar4 == 0))
    goto switchD_02065130_default;
    uVar5 = *(undefined4 *)(iVar3 + 0x1e4);
    uVar6 = 8;
    break;
  case 2:
    FUN_0204fd40();
    uVar2 = (int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(param_1 + 0x7e) & 0xff;
    iVar9 = uVar2 << 8;
    if (uVar2 == 0) goto switchD_02065130_default;
    if (((uVar7 & 1) != 0) && (iVar4 = FUN_020650a0(iVar3), iVar4 != 0)) {
      FUN_020852f8(*(undefined4 *)(iVar3 + 0x28),iVar9,0x10);
    }
    if (((uVar7 & 2) != 0) && (iVar4 = FUN_020650a0(DAT_020654b4), iVar4 != 0)) {
      FUN_020852f8(*(undefined4 *)(iVar3 + 0xbc),iVar9,0x10);
    }
    if (((uVar7 & 4) != 0) && (iVar4 = FUN_020650a0(DAT_020654b8), iVar4 != 0)) {
      FUN_020852f8(*(undefined4 *)(iVar3 + 0x150),iVar9,0x10);
    }
    if (((uVar7 & 8) == 0) || (iVar4 = FUN_020650a0(DAT_020654bc), iVar4 == 0))
    goto switchD_02065130_default;
    uVar5 = *(undefined4 *)(iVar3 + 0x1e4);
    uVar6 = 0x10;
    break;
  case 3:
    FUN_0204fd40();
    uVar2 = (uint)*(ushort *)(param_1 + 0x7c) - (int)*(short *)(param_1 + 0x32) & 0xff;
    iVar9 = uVar2 << 8;
    if (uVar2 == 0) goto switchD_02065130_default;
    if (((uVar7 & 1) != 0) && (iVar4 = FUN_020650a0(iVar3), iVar4 != 0)) {
      FUN_020852f8(*(undefined4 *)(iVar3 + 0x28),iVar9,0x18);
    }
    if (((uVar7 & 2) != 0) && (iVar4 = FUN_020650a0(DAT_020654b4), iVar4 != 0)) {
      FUN_020852f8(*(undefined4 *)(iVar3 + 0xbc),iVar9,0x18);
    }
    if (((uVar7 & 4) != 0) && (iVar4 = FUN_020650a0(DAT_020654b8), iVar4 != 0)) {
      FUN_020852f8(*(undefined4 *)(iVar3 + 0x150),iVar9,0x18);
    }
    if (((uVar7 & 8) == 0) || (iVar4 = FUN_020650a0(DAT_020654bc), iVar4 == 0))
    goto switchD_02065130_default;
    uVar5 = *(undefined4 *)(iVar3 + 0x1e4);
    uVar6 = 0x18;
    break;
  default:
    goto switchD_02065130_default;
  }
  FUN_020852f8(uVar5,iVar9,uVar6);
switchD_02065130_default:
  FUN_0208da4c();
  iVar4 = FUN_0208de54();
  if (iVar4 != 0) {
    return;
  }
  if (uVar8 == 0) {
    return;
  }
  uVar7 = 0;
  do {
    if (((uVar8 & 1 << (uVar7 & 0xff)) != 0) &&
       (iVar4 = FUN_020650a0(uVar7 * 0x94 + iVar3), iVar4 == 0)) {
      uVar8 = uVar8 & ~(1 << (uVar7 & 0xff)) & 0xffU;
    }
    uVar7 = uVar7 + 1;
  } while (uVar7 < 2);
  uVar5 = FUN_02093b5c();
  FUN_02099704(uVar5,param_1,uVar8,iVar9,(uVar1 & 3) << 3);
  return;
}

/* ================================================================
 * FUN_02065588 @ 02065588
 * ================================================================ */

void FUN_02065588(int param_1,int param_2,undefined4 param_3,undefined4 param_4)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;

  uVar6 = 3;
  if (param_2 == 5) {
    FUN_0208da4c();
    iVar2 = FUN_0208de54();
    if (iVar2 == 0) {
      uVar6 = 4;
    }
  }
  iVar3 = (uVar6 >> 1) * 0x10 + (uVar6 & 1 ^ 1) * -8;
  iVar2 = iVar3;
  uVar1 = uVar6;
  uVar5 = uVar6;
  iVar4 = iVar3;
  if (uVar6 == 0) {
    return;
  }
  do {
    for (; uVar1 != 0; uVar1 = uVar1 - 1 & 0xff) {
      FUN_0208a240(param_1,param_2,*(short *)(param_1 + 0x32) + iVar2 & 0xffff,
                   *(short *)(param_1 + 0x36) + iVar4 & 0xffff,param_4);
      iVar2 = iVar2 + -0x10;
    }
    uVar5 = uVar5 - 1 & 0xff;
    iVar2 = iVar3;
    uVar1 = uVar6;
    iVar4 = iVar4 + -0x10;
  } while (uVar5 != 0);
  return;
}

/* ================================================================
 * FUN_02065634 @ 02065634
 * ================================================================ */

void FUN_02065634(int param_1)

{
  (**(code **)(DAT_0206564c + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_02065650 @ 02065650
 * ================================================================ */

void FUN_02065650(int param_1)

{
  byte bVar1;
  int iVar2;
  undefined4 uVar3;

  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  iVar2 = FUN_02056f30(*(undefined2 *)(param_1 + 0x50));
  bVar1 = *(byte *)(param_1 + 10);
  *(int *)(param_1 + 0x7c) =
       iVar2 + 4 +
       (((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_020656d0 + 0x1a)) >> 4) << 6
       | (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_020656d0 + 0x18)) >> 4) * 2;
  *(byte *)(param_1 + 0x18) = bVar1 & 0x80;
  *(byte *)(param_1 + 10) = bVar1 & 0x1f;
  if ((bVar1 & 0x80) == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = FUN_020460e8(*(undefined1 *)(param_1 + 0xb));
  }
  *(undefined4 *)(param_1 + 0x60) = uVar3;
  return;
}

/* ================================================================
 * FUN_020656d4 @ 020656d4
 * ================================================================ */

void FUN_020656d4(int param_1)

{
  int iVar1;

  iVar1 = FUN_02056f30(*(undefined2 *)(param_1 + 0x50));
  if (*(ushort *)(iVar1 + (uint)**(ushort **)(param_1 + 0x7c) * 2 + 0x5004) != DAT_02065758) {
    return;
  }
  *(uint *)(DAT_0206575c + (uint)*(byte *)(param_1 + 0x52) * 4) =
       *(uint *)(DAT_0206575c + (uint)*(byte *)(param_1 + 0x52) * 4) |
       *(uint *)(DAT_02065760 + (uint)*(byte *)(param_1 + 10) * 4);
  if (*(byte *)(param_1 + 0x17) != 0) {
    *(ushort *)(param_1 + 0x78) = (ushort)*(byte *)(param_1 + 0x17) * 0x1e;
    *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02065764 @ 02065764
 * ================================================================ */

void FUN_02065764(int param_1)

{
  if (*(char *)(param_1 + 0x18) != '\0') {
    if (*(uint *)(param_1 + 0x60) ==
        (*(uint *)(param_1 + 0x60) & *(uint *)(DAT_02065824 + (uint)*(byte *)(param_1 + 0x52) * 4)))
    {
      FUN_0204be88();
      return;
    }
  }
  *(short *)(param_1 + 0x78) = *(short *)(param_1 + 0x78) + -1;
  if (*(short *)(param_1 + 0x78) != 0) {
    return;
  }
  FUN_02021c28(DAT_0206582c,
               (((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_02065828 + 0x1a)) >>
                4) << 6 |
               (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_02065828 + 0x18)) >> 4)
               & 0xffff,*(undefined2 *)(param_1 + 0x50));
  *(uint *)(DAT_02065824 + (uint)*(byte *)(param_1 + 0x52) * 4) =
       *(uint *)(DAT_02065824 + (uint)*(byte *)(param_1 + 0x52) * 4) &
       ~*(uint *)(DAT_02065830 + (uint)*(byte *)(param_1 + 10) * 4);
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + -1;
  return;
}

/* ================================================================
 * FUN_02065850 @ 02065850
 * ================================================================ */

void FUN_02065850(int param_1)

{
  int iVar1;
  uint uVar2;

  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  iVar1 = FUN_02056f30(*(undefined2 *)(param_1 + 0x50));
  uVar2 = ((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_020658b0 + 0x1a)) >> 4) <<
          6 | (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_020658b0 + 0x18)) >> 4;
  *(uint *)(param_1 + 0x7c) = iVar1 + 4 + uVar2 * 2;
  *(undefined2 *)(param_1 + 0x78) = *(undefined2 *)(iVar1 + 4 + uVar2 * 2);
  return;
}

/* ================================================================
 * FUN_020658dc @ 020658dc
 * ================================================================ */

void FUN_020658dc(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  uint uVar3;

  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  uVar3 = (uint)*(byte *)(param_1 + 0x52);
  *(uint *)(DAT_0206595c + uVar3 * 4) =
       *(uint *)(DAT_0206595c + uVar3 * 4) |
       *(uint *)(DAT_02065960 + (uint)*(byte *)(param_1 + 10) * 4);
  if (*(char *)(param_1 + 0xb) != '\0') {
    uVar2 = FUN_0204d634(uVar3);
    FUN_0204602c(uVar2,*(ushort *)(DAT_02065964 + uVar3 * 4) & 0xff,
                 *(ushort *)(DAT_02065968 + uVar3 * 4) & 0xff,uVar3);
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02065eb0 @ 02065eb0
 * ================================================================ */

void FUN_02065eb0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  short sVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;

  *(ushort *)(param_1 + 0x7c) =
       (ushort)((uint)((int)*(short *)(param_1 + 0x32) << 0x1a) >> 0x17) + 8;
  iVar4 = (uint)*(byte *)(param_1 + 0x52) * 4;
  sVar1 = *(short *)(DAT_02065f54 + iVar4);
  *(ushort *)(param_1 + 0x7e) =
       (ushort)((uint)((int)*(short *)(param_1 + 0x36) << 0x1a) >> 0x17) + 8;
  iVar2 = DAT_02065f58;
  *(short *)(param_1 + 0x32) =
       *(short *)(param_1 + 0x7c) +
       sVar1 + (short)((int)((int)*(short *)(param_1 + 0x32) & 0xfc0U) >> 2);
  iVar4 = (uint)*(ushort *)(param_1 + 0x7e) +
          (uint)*(ushort *)(iVar2 + iVar4) + ((int)((int)*(short *)(param_1 + 0x36) & 0xfc0U) >> 2);
  *(short *)(param_1 + 0x36) = (short)iVar4;
  uVar3 = FUN_0204d634((uint)*(byte *)(param_1 + 0x52),*(undefined1 *)(param_1 + 10),iVar4,
                       (uint)*(ushort *)(param_1 + 0x7e),param_4);
  *(undefined4 *)(param_1 + 0x78) = uVar3;
  *(undefined4 *)(param_1 + 0x70) = 0;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined2 *)(param_1 + 0x76) = 1;
  FUN_0208ae30(param_1);
  return;
}
