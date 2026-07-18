#include "game/arm9_middle_02067f94_raw.h"

/*
 * Raw matching-first recovery for the 32 catalog entries beginning at
 * 0x02067f94. These bodies remain address-named promotion candidates and are
 * not linked byte-matching evidence.
 *
 * Calls whose implicit r0 argument was omitted by Ghidra are written with the
 * owning param_1 so this mechanically faithful draft remains valid C.
 */

extern byte *DAT_0206828c;
extern byte *DAT_0206835c;
extern uint DAT_02068424;
extern uint DAT_02068490;
extern uint DAT_020685ec;
extern uint DAT_020685f0;
extern uint DAT_020685f4;
extern uint DAT_0206863c;
extern byte *DAT_02068708;
extern uint DAT_0206870c;
extern byte *DAT_02068710;
extern char *DAT_020687cc;
extern byte *DAT_020687d0;
extern uint DAT_020687d4;
extern byte *DAT_020687d8;
extern uint DAT_020688f0;
extern uint DAT_02068978;
extern uint DAT_02068e14;
extern byte *DAT_02068f44;
extern uint DAT_02068f48;
extern uint DAT_02068f4c;
extern uint DAT_02068f50;
extern uint DAT_02068f54;
extern uint DAT_02069028;
extern uint DAT_0206902c;
extern uint DAT_02069098;
extern uint DAT_0206909c;
extern uint DAT_02069260;
extern uint DAT_02069264;
extern uint DAT_02069268;
extern uint DAT_0206926c;
extern uint DAT_02069308;
extern uint DAT_02069370;
extern uint DAT_02069374;
extern uint DAT_020693f0;
extern uint DAT_020693f4;
extern uint DAT_020693f8;
extern uint PTR_DAT_02068bcc;
extern undefined *PTR_DAT_02068bd0;
extern undefined *PTR_DAT_02068bd4;

/* ================================================================
 * FUN_02067f94 @ 02067f94
 * ================================================================ */

void FUN_02067f94(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x40002002;
  *(undefined2 *)(param_1 + 0x32) = 0x108;
  *(undefined2 *)(param_1 + 0x36) = 0x118;
  *(undefined4 *)(param_1 + 0xc) = 0xc00;
  *(undefined1 *)(param_1 + 0x2b) = 0x1d;
  FUN_020859e0(param_1,*(undefined1 *)(param_1 + 10),0x1d,0xc00,param_4);
  return;
}

/* ================================================================
 * FUN_02067ff0 @ 02067ff0
 * ================================================================ */

void FUN_02067ff0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  FUN_0204fd40();
  if (0x60 < *(short *)(param_1 + 0x36)) {
    return;
  }
  *(undefined2 *)(param_1 + 0x36) = 0x60;
  FUN_02076858(0x28,5,1,0x60,param_4);
  *(undefined1 *)(param_1 + 0x17) = 0x3a;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  FUN_02076fe0(0x223);
  return;
}

/* ================================================================
 * FUN_02068044 @ 02068044
 * ================================================================ */

void FUN_02068044(int param_1)

{
  byte bVar1;
  char cVar2;
  undefined4 uVar3;

  bVar1 = *(byte *)(param_1 + 0x17);
  if (bVar1 < 0x2f) {
    if (bVar1 < 0x2e) {
      if (bVar1 != 0x26) goto LAB_020680c0;
      uVar3 = 3;
    }
    else {
      uVar3 = 2;
    }
  }
  else if (bVar1 < 0x36) {
    if (bVar1 != 0x35) goto LAB_020680c0;
    uVar3 = 1;
  }
  else {
    if (bVar1 != 0x3a) goto LAB_020680c0;
    uVar3 = 0;
  }
  FUN_02076858(0x28,10,uVar3);
LAB_020680c0:
  cVar2 = *(char *)(param_1 + 0x17);
  *(char *)(param_1 + 0x17) = cVar2 + -1;
  if (cVar2 == '\0') {
    *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
    *(undefined1 *)(param_1 + 0x2b) = 8;
    *(undefined4 *)(param_1 + 0xc) = 0x200;
    return;
  }
  return;
}

/* ================================================================
 * FUN_020680f4 @ 020680f4
 * ================================================================ */

void FUN_020680f4(int param_1)

{
  int iVar1;

  FUN_0204fd40();
  iVar1 = *(int *)(param_1 + 0xc) + -0x10;
  *(int *)(param_1 + 0xc) = iVar1;
  if (iVar1 < 1) {
    *(undefined1 *)(param_1 + 0x17) = 0;
    *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  }
  return;
}

/* ================================================================
 * FUN_02068128 @ 02068128
 * ================================================================ */

void FUN_02068128(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;
  int iVar2;

  cVar1 = *(char *)(param_1 + 0x17);
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  if (cVar1 != '\0') {
    return;
  }
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined1 *)(param_1 + 0x2b) = 0x18;
  *(undefined4 *)(param_1 + 0xc) = 0x800;
  iVar2 = FUN_02076858(0x28,2,0,0x800,param_4);
  *(undefined2 *)(iVar2 + 0x32) = *(undefined2 *)(param_1 + 0x32);
  *(undefined2 *)(iVar2 + 0x36) = *(undefined2 *)(param_1 + 0x36);
  iVar2 = FUN_02076858(0x28,2,1);
  *(undefined2 *)(iVar2 + 0x32) = *(undefined2 *)(param_1 + 0x32);
  *(undefined2 *)(iVar2 + 0x36) = *(undefined2 *)(param_1 + 0x36);
  iVar2 = FUN_02076858(0x28,2,2);
  *(undefined2 *)(iVar2 + 0x32) = *(undefined2 *)(param_1 + 0x32);
  *(undefined2 *)(iVar2 + 0x36) = *(undefined2 *)(param_1 + 0x36);
  FUN_02076858(0x28,3,0);
  FUN_02076fe0(0x209);
  return;
}

/* ================================================================
 * FUN_020681ec @ 020681ec
 * ================================================================ */

void FUN_020681ec(int param_1)

{
  FUN_0204fd40();
  if (0x4f < *(short *)(param_1 + 0x32)) {
    return;
  }
  *(undefined2 *)(param_1 + 0x32) = 0x50;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined1 *)(param_1 + 0x17) = 5;
  return;
}

/* ================================================================
 * FUN_02068224 @ 02068224
 * ================================================================ */

void FUN_02068224(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;

  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  FUN_0204625c(2,8,param_3,param_4,param_4);
  FUN_02076858(0x28,0,0);
  FUN_0204be88(param_1);
  *DAT_0206828c = *DAT_0206828c + '\x01';
  FUN_02076fe0(0x20a);
  FUN_02076fe0(0x1b2);
  return;
}

/* ================================================================
 * FUN_020682ac @ 020682ac
 * ================================================================ */

void FUN_020682ac(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  uint uVar1;
  int iVar2;
  uint uVar3;

  uVar3 = (uint)*(byte *)(param_1 + 0xb);
  iVar2 = *(byte *)(param_1 + 0x14) + 1;
  uVar1 = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 2 | (uVar3 + 1 & 3) << 0xe | 0x40002000;
  *(uint *)(param_1 + 0xc) = (uVar3 * -2 + 6) * 0x100;
  *(undefined1 *)(param_1 + 0x2b) = 0x18;
  *(char *)(param_1 + 0x14) = (char)iVar2;
  *(uint *)(param_1 + 0x1c) = uVar1;
  FUN_020859e0(param_1,uVar3 + 0xc & 0xff,uVar1,iVar2,param_4);
  return;
}

/* ================================================================
 * FUN_02068314 @ 02068314
 * ================================================================ */

void FUN_02068314(int param_1)

{
  int iVar1;

  FUN_0204fd40();
  iVar1 = (*(byte *)(param_1 + 0xb) + 1) * 0x1a + 0x50;
  if (*(short *)(param_1 + 0x32) < iVar1) {
    *(short *)(param_1 + 0x32) = (short)iVar1;
  }
  if (*DAT_0206835c == '\0') {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_0206837c @ 0206837c
 * ================================================================ */

void FUN_0206837c(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x2002;
  *(undefined1 *)(param_1 + 0x2b) = 0x1d;
  *(undefined4 *)(param_1 + 0xc) = 0x100;
  *(undefined2 *)(param_1 + 0x32) = 8;
  *(undefined2 *)(param_1 + 0x36) = 0x10;
  FUN_020859e0(param_1,*(char *)(param_1 + 0xb) + '\x04',0x10,8,param_4);
  return;
}

/* ================================================================
 * FUN_020683dc @ 020683dc
 * ================================================================ */

void FUN_020683dc(int param_1)

{
  FUN_0204fd40();
  FUN_02085980(param_1);
  if ((*(byte *)(param_1 + 0x29) & 0x80) == 0) {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_0206840c @ 0206840c
 * ================================================================ */

void FUN_0206840c(int param_1)

{
  (**(code **)(DAT_02068424 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_02068428 @ 02068428
 * ================================================================ */

void FUN_02068428(int param_1)

{
  byte bVar1;
  byte bVar2;

  bVar1 = *(byte *)(DAT_02068490 + (uint)*(byte *)(param_1 + 0xb) * 2);
  bVar2 = *(byte *)(DAT_02068490 + (uint)*(byte *)(param_1 + 0xb) * 2 + 1);
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x2002;
  *(ushort *)(param_1 + 0x32) = (ushort)bVar1;
  *(ushort *)(param_1 + 0x36) = (ushort)bVar2;
  *(undefined1 *)(param_1 + 0x18) = 0;
  *(undefined1 *)(param_1 + 0x17) = 0;
  *(undefined2 *)(param_1 + 100) = 0;
  FUN_020859e0(param_1,2);
  return;
}

/* ================================================================
 * FUN_02068494 @ 02068494
 * ================================================================ */

void FUN_02068494(int param_1)

{
  byte bVar1;
  ushort uVar2;
  int iVar3;
  uint uVar4;
  undefined4 uVar5;
  uint uVar6;
  uint uVar7;

  FUN_02085980();
  uVar2 = *(ushort *)(param_1 + 100);
  uVar4 = (uint)*(short *)(DAT_020685ec + ((int)(uint)uVar2 >> 4) * 4);
  uVar7 = (uint)((ulonglong)uVar4 * 0xfffc0000);
  uVar6 = (uint)((ulonglong)uVar4 * 0xfffa6000);
  bVar1 = *(byte *)(DAT_020685f0 + (uint)*(byte *)(param_1 + 0xb) * 2 + 1);
  *(ushort *)(param_1 + 0x32) =
       (ushort)*(byte *)(DAT_020685f0 + (uint)*(byte *)(param_1 + 0xb) * 2) +
       (short)((int)(uVar7 + 0x800 >> 0xc |
                    (((int)((ulonglong)uVar4 * 0xfffc0000 >> 0x20) - uVar4) +
                    (uint)(0xfffff7ff < uVar7)) * 0x100000) >> 0xc);
  *(ushort *)(param_1 + 0x36) =
       (ushort)bVar1 +
       (short)((int)(uVar6 + 0x800 >> 0xc |
                    (((int)((ulonglong)uVar4 * 0xfffa6000 >> 0x20) - uVar4) +
                    (uint)(0xfffff7ff < uVar6)) * 0x100000) >> 0xc);
  if (*(char *)(param_1 + 0x17) == '\0') {
    *(ushort *)(param_1 + 100) = uVar2 + 0x33;
    if (0x4ff < *(ushort *)(param_1 + 100)) {
      *(undefined1 *)(param_1 + 0x17) = 1;
    }
  }
  else if ((*(char *)(param_1 + 0x17) == '\x01') &&
          (*(ushort *)(param_1 + 100) = uVar2 + 0x25c, 0x3fff < *(ushort *)(param_1 + 100))) {
    *(undefined2 *)(param_1 + 100) = 0x4000;
    *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
    *(undefined1 *)(param_1 + 0x17) = 10;
  }
  iVar3 = DAT_020685f4;
  if (*(char *)(param_1 + 0x18) == '\x1e') {
    *(undefined4 *)(DAT_020685f4 + 0xc) = 0xffffffe5;
    uVar5 = 3;
  }
  else {
    if (*(char *)(param_1 + 0x18) != '(') goto LAB_020685dc;
    *(undefined4 *)(DAT_020685f4 + 0xc) = 0xffffffe5;
    uVar5 = 5;
  }
  *(undefined1 *)(iVar3 + 7) = 0;
  FUN_0204625c(uVar5,0xc);
LAB_020685dc:
  *(char *)(param_1 + 0x18) = *(char *)(param_1 + 0x18) + '\x01';
  return;
}

/* ================================================================
 * FUN_020685f8 @ 020685f8
 * ================================================================ */

void FUN_020685f8(int param_1)

{
  char cVar1;

  FUN_02085980();
  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02068624 @ 02068624
 * ================================================================ */

void FUN_02068624(int param_1)

{
  (**(code **)(DAT_0206863c + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_02068640 @ 02068640
 * ================================================================ */

void FUN_02068640(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  uint uVar1;

  uVar1 = *(uint *)(param_1 + 0x1c) & 0x3ff03f3f | 0x72080;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined2 *)(param_1 + 0x32) = 0x100;
  *(undefined2 *)(param_1 + 0x36) = 0xc0;
  *(undefined1 *)(param_1 + 0x1b) = 0x6b;
  *(undefined1 *)(param_1 + 0x20) = 0x80;
  *(uint *)(param_1 + 0x1c) = uVar1;
  FUN_020859e0(param_1,0,uVar1,0x80,param_4);
  return;
}

/* ================================================================
 * FUN_020686a0 @ 020686a0
 * ================================================================ */

void FUN_020686a0(int param_1)

{
  uint uVar1;

  if (*DAT_02068708 == '\0') {
    if (((*(byte *)(DAT_0206870c + 8) & 0x80) == 0) || (*DAT_02068710 != '\x04')) {
      uVar1 = 0;
    }
    else {
      uVar1 = 2;
    }
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | uVar1;
    FUN_02085980();
    return;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
  return;
}

/* ================================================================
 * FUN_02068714 @ 02068714
 * ================================================================ */

void FUN_02068714(int param_1)

{
  char *pcVar1;

  pcVar1 = DAT_020687cc;
  if (*(char *)(param_1 + 0x14) == '\0') {
    *(undefined2 *)(param_1 + 0x32) = 0x100;
    *(undefined2 *)(param_1 + 0x36) = 0xc0;
    *(undefined1 *)(param_1 + 0x1b) = 0x4e;
    *(undefined1 *)(param_1 + 0x20) = 0xa0;
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3ff03f3f | 0x70080;
    *(undefined1 *)(param_1 + 0x14) = 1;
    return;
  }
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc;
  if (*pcVar1 == '\x04') {
    if ((*DAT_020687d0 == '\0') && ((*(byte *)(DAT_020687d4 + 8) & 0x80) != 0)) {
      return;
    }
    if ((*DAT_020687d8 & 0x10) != 0) {
      *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 2;
    }
    return;
  }
  return;
}

/* ================================================================
 * FUN_0206884c @ 0206884c
 * ================================================================ */

void FUN_0206884c(int param_1)

{
  char cVar1;

  if (*(char *)(param_1 + 0x14) == '\0') {
    *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x40002002;
    *(undefined2 *)(param_1 + 0x32) = 0x80;
    *(undefined2 *)(param_1 + 0x36) = 0x60;
    *(undefined1 *)(param_1 + 0x17) = 8;
    *(undefined1 *)(param_1 + 0x14) = 1;
    FUN_020859e0(param_1,*(char *)(param_1 + 0xb) + '\b');
    return;
  }
  if (*(char *)(param_1 + 0x14) != '\x01') {
    return;
  }
  cVar1 = *(char *)(param_1 + 0x17);
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  if (cVar1 != '\x01') {
    return;
  }
  FUN_0204be88();
  return;
}

/* ================================================================
 * FUN_020688d8 @ 020688d8
 * ================================================================ */

void FUN_020688d8(int param_1)

{
  (**(code **)(DAT_020688f0 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_020688f4 @ 020688f4
 * ================================================================ */

void FUN_020688f4(int param_1)

{
  char cVar1;

  cVar1 = '2' - *(char *)(DAT_02068978 + (uint)*(byte *)(param_1 + 0xb));
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x6002;
  *(undefined2 *)(param_1 + 0x32) = 0x9b;
  *(undefined2 *)(param_1 + 0x36) = 0x6c;
  *(char *)(param_1 + 0x18) = cVar1;
  *(char *)(param_1 + 0x17) = cVar1;
  *(undefined2 *)(param_1 + 100) = 0;
  *(undefined2 *)(param_1 + 0x66) = 0;
  *(undefined2 *)(param_1 + 0x68) = 0;
  *(undefined2 *)(param_1 + 0x6a) = 0;
  *(undefined1 *)(param_1 + 0x6c) = 0;
  FUN_020859e0(param_1,*(byte *)(param_1 + 0xb) + 4 & 0xff);
  return;
}

/* ================================================================
 * FUN_0206897c @ 0206897c
 * ================================================================ */

void FUN_0206897c(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  ushort uVar2;
  longlong lVar3;
  longlong lVar4;
  short sVar5;
  char cVar6;
  undefined *puVar7;
  uint uVar8;
  uint uVar9;
  int unaff_r4;
  int unaff_r5;
  uint unaff_r6;
  uint uVar10;
  uint uVar11;

  FUN_02085980();
  uVar10 = (uint)*(short *)(PTR_DAT_02068bcc + ((int)(uint)*(ushort *)(param_1 + 100) >> 4) * 4);
  uVar11 = (uint)((ulonglong)uVar10 * 0xfffbe000);
  uVar8 = (uint)((ulonglong)uVar10 * 0xfffa5000);
  switch(*(undefined1 *)(param_1 + 0xb)) {
  case 0:
    break;
  case 1:
    unaff_r6 = (uint)*(ushort *)(param_1 + 0x66);
    iVar1 = (int)(uint)*(ushort *)(param_1 + 0x68) >> 4;
    unaff_r4 = -(int)*(short *)(PTR_DAT_02068bcc + iVar1 * 4);
    unaff_r5 = -(int)*(short *)(PTR_DAT_02068bcc + (iVar1 * 2 + 1) * 2);
    goto switchD_020689f4_default;
  case 2:
    uVar2 = *(ushort *)(param_1 + 0x66);
    puVar7 = PTR_DAT_02068bd0;
    goto LAB_02068a54;
  case 3:
    break;
  default:
    goto switchD_020689f4_default;
  }
  uVar2 = *(ushort *)(param_1 + 0x68);
  puVar7 = PTR_DAT_02068bd4;
LAB_02068a54:
  unaff_r6 = (uint)uVar2;
  unaff_r4 = (int)*(short *)(puVar7 + 100);
  unaff_r5 = (int)*(short *)(puVar7 + 0x66);
switchD_020689f4_default:
  uVar9 = *(short *)(PTR_DAT_02068bcc + ((int)unaff_r6 >> 4) * 4) * 0x20000;
  uVar9 = uVar9 + 0x800 >> 0xc |
          (((uint)(int)*(short *)(PTR_DAT_02068bcc + ((int)unaff_r6 >> 4) * 4) >> 0xf) +
          (uint)(0xfffff7ff < uVar9)) * 0x100000;
  lVar3 = (longlong)unaff_r4 * (longlong)(int)uVar9 + 0x800;
  lVar4 = (longlong)unaff_r5 * (longlong)(int)uVar9 + 0x800;
  uVar9 = (uint)lVar4 >> 0xc | (int)((ulonglong)lVar4 >> 0x20) * 0x100000;
  *(short *)(param_1 + 0x32) =
       (short)((int)((uVar11 + 0x800 >> 0xc |
                     (((int)((ulonglong)uVar10 * 0xfffbe000 >> 0x20) - uVar10) +
                     (uint)(0xfffff7ff < uVar11)) * 0x100000) +
                    ((uint)lVar3 >> 0xc | (int)((ulonglong)lVar3 >> 0x20) * 0x100000)) >> 0xc) +
       0x9b;
  *(short *)(param_1 + 0x36) =
       (short)((int)((uVar8 + 0x800 >> 0xc |
                     (((int)((ulonglong)uVar10 * 0xfffa5000 >> 0x20) - uVar10) +
                     (uint)(0xfffff7ff < uVar8)) * 0x100000) + uVar9) >> 0xc) + 0x6c;
  if (*(char *)(param_1 + 0x6c) == '\0') {
    sVar5 = FUN_0200d948(0x1300,*(byte *)(param_1 + 0x18) >> 1,0,uVar9,param_4);
    *(short *)(param_1 + 100) = *(short *)(param_1 + 100) + sVar5;
    if (0x12ff < *(ushort *)(param_1 + 100)) {
      *(char *)(param_1 + 0x6c) = *(char *)(param_1 + 0x6c) + '\x01';
    }
  }
  else {
    sVar5 = FUN_0200d948(0x2d00,*(byte *)(param_1 + 0x18) >> 1,*(char *)(param_1 + 0x6c),uVar9,
                         param_4);
    *(short *)(param_1 + 100) = *(short *)(param_1 + 100) + sVar5;
    if (0x3fff < *(ushort *)(param_1 + 100)) {
      FUN_0204be88(param_1);
    }
  }
  sVar5 = FUN_0200d948(0x8000,*(undefined1 *)(param_1 + 0x18));
  *(short *)(param_1 + 0x66) = *(short *)(param_1 + 0x66) + sVar5;
  if (0x7fff < *(ushort *)(param_1 + 0x66)) {
    *(undefined2 *)(param_1 + 0x66) = 0x8000;
  }
  sVar5 = FUN_0200d948(0x10000,*(undefined1 *)(param_1 + 0x18));
  *(short *)(param_1 + 0x68) = *(short *)(param_1 + 0x68) + sVar5;
  sVar5 = FUN_0200d948(0x20000,*(undefined1 *)(param_1 + 0x18));
  cVar6 = *(char *)(param_1 + 0x17) + -1;
  sVar5 = *(short *)(param_1 + 0x6a) + sVar5;
  *(short *)(param_1 + 0x6a) = sVar5;
  if (cVar6 == '\0') {
    sVar5 = 0;
  }
  *(char *)(param_1 + 0x17) = cVar6;
  if (cVar6 == '\0') {
    *(short *)(param_1 + 0x68) = sVar5;
    *(short *)(param_1 + 0x6a) = sVar5;
  }
  return;
}

/* ================================================================
 * FUN_02068d58 @ 02068d58
 * ================================================================ */

undefined4 FUN_02068d58(int param_1,undefined4 param_2,uint param_3)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;

  uVar1 = FUN_0208455c();
  if ((uVar1 & 0x3f) < 2) {
    iVar2 = FUN_0204bd80(9);
    if (iVar2 != 0) {
      FUN_0205031c(param_1,iVar2);
      *(undefined1 *)(iVar2 + 10) = *(undefined1 *)(param_1 + 10);
      if (param_3 < 0x25) {
        uVar3 = 0x4f;
        uVar4 = 0x4000;
      }
      else {
        uVar3 = 0x78;
        uVar4 = 0x8000;
      }
      FUN_02021a94(uVar3,param_2,uVar4);
    }
    return 1;
  }
  return 0;
}

/* ================================================================
 * FUN_02068dd0 @ 02068dd0
 * ================================================================ */

void FUN_02068dd0(int param_1)

{
  char cVar1;

  cVar1 = *(char *)(param_1 + 0x18);
  if ((cVar1 != '\0') && (*(char *)(param_1 + 0x18) = cVar1 + -1, cVar1 == '\x01')) {
    FUN_020859e0(param_1,*(undefined1 *)(param_1 + 0x2a));
  }
  (**(code **)(DAT_02068e14 + (uint)*(byte *)(param_1 + 0x14) * 4))(param_1);
  return;
}

/* ================================================================
 * FUN_02068e18 @ 02068e18
 * ================================================================ */

void FUN_02068e18(int param_1)

{
  char *pcVar1;
  undefined4 uVar2;
  uint uVar3;
  uint uVar4;

  if ((*(byte *)(param_1 + 0xb) & 0x7f) == 3) {
    *(byte *)(param_1 + 0xb) = *(byte *)(param_1 + 0xb) & 0x80 | *DAT_02068f44 - 2U;
  }
  uVar2 = DAT_02068f48;
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  *(byte *)(param_1 + 0x5e) = *(byte *)(param_1 + 0x5e) & 0xfc;
  *(undefined4 *)(param_1 + 0x48) = uVar2;
  FUN_0208ae78(param_1);
  FUN_020859e0(param_1,0);
  pcVar1 = DAT_02068f44;
  uVar4 = *(byte *)(param_1 + 0xb) & 3;
  *(short *)(param_1 + 0x7c) = (short)((int)(uint)*(byte *)(param_1 + 0xb) >> 7);
  *(char *)(param_1 + 0xb) = (char)uVar4;
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xffff3fff | 0x8000;
  if (pcVar1[2] != '\0') {
    FUN_02081088(param_1,*(undefined2 *)(DAT_02068f4c + (uVar4 + 0xc) * 2));
    return;
  }
  uVar3 = (uint)(byte)pcVar1[1];
  if (4 < uVar3) {
    FUN_02081088(param_1,*(undefined2 *)
                          (DAT_02068f54 +
                          (uVar4 + (uint)*(byte *)(*(int *)(*(int *)(DAT_02068f50 + uVar3 * 4) +
                                                           (uint)(byte)pcVar1[0x48] * 8) + 1) * 3) *
                          2));
    return;
  }
  FUN_02081088(param_1,*(undefined2 *)(DAT_02068f4c + (uVar4 + uVar3 * 3) * 2));
  return;
}

/* ================================================================
 * FUN_02068f58 @ 02068f58
 * ================================================================ */

void FUN_02068f58(int param_1)

{
  uint uVar1;
  uint uVar2;

  uVar1 = FUN_020690a0(param_1);
  if ((int)uVar1 < (int)(*(byte *)(param_1 + 0xb) + 2)) {
    if (*(byte *)(param_1 + 0x2a) < uVar1) {
      uVar2 = uVar1;
      if (*(byte *)(param_1 + 0xb) + 1 == uVar1) {
        *(undefined1 *)(param_1 + 0x18) = 4;
        uVar2 = uVar1 + 1 & 0xff;
      }
      FUN_020859e0(param_1,uVar2);
      FUN_02085f80(0x12);
    }
    else if (uVar1 < *(byte *)(param_1 + 0x2a)) {
      FUN_020859e0(param_1,uVar1);
    }
    *(char *)(param_1 + 0x2a) = (char)uVar1;
    return;
  }
  *(uint *)(DAT_02069028 + (uint)*(byte *)(param_1 + 0x52) * 4) =
       *(uint *)(DAT_02069028 + (uint)*(byte *)(param_1 + 0x52) * 4) |
       *(uint *)(DAT_0206902c + (uint)*(byte *)(param_1 + 10) * 4);
  *(undefined1 *)(param_1 + 0x14) = 2;
  *(undefined1 *)(param_1 + 0x18) = 0;
  *(undefined2 *)(param_1 + 0x3a) = 0;
  *(undefined1 *)(param_1 + 0x2a) = 4;
  FUN_020859e0(param_1);
  FUN_02085f80(0x71);
  return;
}

/* ================================================================
 * FUN_02069030 @ 02069030
 * ================================================================ */

void FUN_02069030(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;

  if (*(short *)(param_1 + 0x7c) == 0) {
    return;
  }
  uVar3 = FUN_020690a0(param_1);
  iVar2 = DAT_0206909c;
  iVar1 = DAT_02069098;
  if ((int)(*(byte *)(param_1 + 0xb) + 2) <= (int)uVar3) {
    return;
  }
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(char *)(param_1 + 0x2a) = (char)uVar3;
  *(uint *)(iVar2 + (uint)*(byte *)(param_1 + 0x52) * 4) =
       *(uint *)(iVar2 + (uint)*(byte *)(param_1 + 0x52) * 4) ^
       *(uint *)(iVar1 + (uint)*(byte *)(param_1 + 10) * 4);
  FUN_020859e0(param_1,uVar3 & 0xff);
  return;
}

/* ================================================================
 * FUN_020690a0 @ 020690a0
 * ================================================================ */

uint FUN_020690a0(int param_1)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;

  uVar2 = FUN_02084110();
  uVar3 = FUN_02084600();
  iVar4 = DAT_02069264;
  if ((uVar2 & 1) != 0) {
    *(undefined1 *)(DAT_02069264 + 0x3f) =
         *(undefined1 *)(DAT_02069260 + (uint)*(byte *)(param_1 + 0x2a));
    if ((*(ushort *)(iVar4 + 0x36) & 0x90) == 0) {
      iVar4 = *(int *)(*(int *)(iVar4 + 0x28) + 0x6c);
      if (((iVar4 != 0) && (iVar4 = *(int *)(iVar4 + 0x58), *(char *)(iVar4 + 8) == '\x01')) &&
         ((uVar2 & *(uint *)(DAT_02069268 + (uint)*(byte *)(iVar4 + 10) * 4)) == 0)) {
        uVar3 = uVar3 + 1 & 0xff;
      }
    }
    else {
      uVar3 = uVar3 - 1 & 0xff;
    }
  }
  iVar4 = DAT_02069264;
  if ((uVar2 & 2) != 0) {
    *(undefined1 *)(DAT_02069264 + 0xd3) =
         *(undefined1 *)(DAT_02069260 + (uint)*(byte *)(param_1 + 0x2a));
    if ((*(ushort *)(iVar4 + 0xca) & 0x90) == 0) {
      iVar4 = *(int *)(*(int *)(iVar4 + 0xbc) + 0x6c);
      if (((iVar4 != 0) && (iVar4 = *(int *)(iVar4 + 0x58), *(char *)(iVar4 + 8) == '\x01')) &&
         ((uVar2 & *(uint *)(DAT_02069268 + (uint)*(byte *)(iVar4 + 10) * 4)) == 0)) {
        uVar3 = uVar3 + 1 & 0xff;
      }
    }
    else {
      uVar3 = uVar3 - 1 & 0xff;
    }
  }
  iVar1 = DAT_0206926c;
  iVar4 = DAT_02069264;
  if ((uVar2 & 4) != 0) {
    *(undefined1 *)(DAT_02069264 + 0x167) =
         *(undefined1 *)(DAT_02069260 + (uint)*(byte *)(param_1 + 0x2a));
    if ((*(ushort *)(iVar1 + 0x5e) & 0x90) == 0) {
      iVar4 = *(int *)(*(int *)(iVar4 + 0x150) + 0x6c);
      if (((iVar4 != 0) && (iVar4 = *(int *)(iVar4 + 0x58), *(char *)(iVar4 + 8) == '\x01')) &&
         ((uVar2 & *(uint *)(DAT_02069268 + (uint)*(byte *)(iVar4 + 10) * 4)) == 0)) {
        uVar3 = uVar3 + 1 & 0xff;
      }
    }
    else {
      uVar3 = uVar3 - 1 & 0xff;
    }
  }
  iVar1 = DAT_0206926c;
  iVar4 = DAT_02069264;
  if ((uVar2 & 8) == 0) {
    return uVar3;
  }
  *(undefined1 *)(DAT_02069264 + 0x1fb) =
       *(undefined1 *)(DAT_02069260 + (uint)*(byte *)(param_1 + 0x2a));
  if ((*(ushort *)(iVar1 + 0xf2) & 0x90) != 0) {
    return uVar3 - 1 & 0xff;
  }
  iVar4 = *(int *)(*(int *)(iVar4 + 0x1e4) + 0x6c);
  if (iVar4 != 0) {
    iVar4 = *(int *)(iVar4 + 0x58);
    if (*(char *)(iVar4 + 8) == '\x01') {
      if ((uVar2 & *(uint *)(DAT_02069268 + (uint)*(byte *)(iVar4 + 10) * 4)) == 0) {
        uVar3 = uVar3 + 1 & 0xff;
      }
      return uVar3;
    }
    return uVar3;
  }
  return uVar3;
}

/* ================================================================
 * FUN_020692a8 @ 020692a8
 * ================================================================ */

void FUN_020692a8(int param_1)

{
  int iVar1;
  int iVar2;

  iVar1 = DAT_02069308;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0x3fff3ffc | 0x80008002;
  iVar2 = iVar1 + (uint)*(byte *)(param_1 + 10) * 6;
  *(undefined2 *)(param_1 + 0x32) = *(undefined2 *)(iVar1 + (uint)*(byte *)(param_1 + 10) * 6);
  *(undefined2 *)(param_1 + 0x36) = *(undefined2 *)(iVar2 + 2);
  *(char *)(param_1 + 0x21) = (char)*(undefined2 *)(iVar2 + 4);
  return;
}

/* ================================================================
 * FUN_0206930c @ 0206930c
 * ================================================================ */

void FUN_0206930c(int param_1)

{
  int iVar1;

  iVar1 = DAT_02069374;
  if (*(byte *)(DAT_02069370 + 8) < 3) {
    return;
  }
  if (*(char *)(DAT_02069370 + 2) != *(char *)(param_1 + 0xb)) {
    return;
  }
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffff3ff | 0x400;
  *(undefined2 *)(iVar1 + 0x66) = 0x3f40;
  *(undefined2 *)(iVar1 + 0x68) = 0x10;
  *(undefined1 *)(param_1 + 0x17) = 0x10;
  FUN_02076fe0(0x7f);
  return;
}

/* ================================================================
 * FUN_02069378 @ 02069378
 * ================================================================ */

void FUN_02069378(int param_1)

{
  byte bVar1;
  byte bVar2;
  int iVar3;
  char cVar4;

  *(ushort *)(DAT_020693f0 + 0x68) =
       (ushort)*(byte *)(param_1 + 0x17) | (0x10 - (ushort)*(byte *)(param_1 + 0x17)) * 0x100;
  cVar4 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar4;
  iVar3 = DAT_020693f8;
  if (cVar4 != '\0') {
    return;
  }
  bVar1 = *(byte *)(DAT_020693f4 + (uint)*(byte *)(param_1 + 0xb) * 2);
  bVar2 = *(byte *)(DAT_020693f4 + (uint)*(byte *)(param_1 + 0xb) * 2 + 1);
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffff3fc;
  *(ushort *)(param_1 + 0x32) = (ushort)bVar1;
  *(ushort *)(param_1 + 0x36) = (ushort)bVar2;
  *(undefined1 *)(iVar3 + 8) = 4;
  return;
}
