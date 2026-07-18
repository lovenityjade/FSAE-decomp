#include "game/arm9_middle_02070dd0_raw.h"

/*
 * Raw matching-first recovery for the 32 catalog entries beginning at
 * 0x02070dd0. These bodies remain address-named promotion candidates and are
 * not linked byte-matching evidence.
 *
 * Calls whose implicit register arguments were omitted by Ghidra are written
 * with the live recovered value so this mechanically faithful draft remains
 * valid C.
 */

extern undefined4 *DAT_0207136c;
extern uint DAT_02071370;
extern uint DAT_02071374;
extern uint DAT_02071378;
extern uint DAT_0207137c;
extern uint DAT_02071380;
extern uint DAT_02071384;
extern uint DAT_02071388;
extern uint DAT_0207138c;
extern uint DAT_02071390;
extern uint DAT_02071394;
extern uint DAT_02071398;
extern uint DAT_0207139c;
extern uint DAT_020719d0;
extern uint DAT_020719d4;
extern uint DAT_020719d8;
extern uint DAT_020719dc;
extern uint DAT_020719e0;
extern uint DAT_020719e4;
extern uint DAT_020719e8;
extern uint DAT_020719ec;
extern uint DAT_02071a68;
extern uint DAT_02071a6c;
extern uint DAT_02071adc;
extern uint DAT_02071afc;
extern uint DAT_02071b68;
extern uint DAT_02071be0;
extern uint DAT_02071be4;
extern uint DAT_02071be8;
extern uint DAT_02071c78;
extern uint DAT_02071f48;
extern uint DAT_02071f80;
extern uint DAT_02071f84;
extern uint DAT_020725cc;
extern uint DAT_020725fc;
extern uint DAT_02072600;
extern uint DAT_02072604;
extern uint DAT_02072634;
extern uint DAT_02072680;
extern uint DAT_02072700;
extern uint DAT_0207276c;
extern uint DAT_02072770;
extern uint DAT_0207285c;
extern uint DAT_020728f8;
extern uint DAT_02072918;
extern uint DAT_020729d0;
extern uint DAT_02072ba8;
extern uint DAT_02072be0;
extern uint DAT_02072c20;
extern uint DAT_02072c6c;
extern undefined2 *DAT_02072d70;
extern uint DAT_02072d74;
extern uint DAT_02072d78;
extern uint DAT_02072e40;
extern uint DAT_02072ee4;
extern uint DAT_02072ee8;
extern uint *DAT_02072eec;
extern uint DAT_02072ef0;
extern uint DAT_02072f54;
extern uint *DAT_02072f58;
extern uint *DAT_02072fc0;
extern uint DAT_02072fc4;
extern uint *DAT_02073034;
extern uint DAT_02073038;
extern undefined *PTR_DAT_02071c7c;
extern undefined *PTR_DAT_02071c80;
extern undefined *PTR_DAT_02071c84;
extern undefined *PTR_DAT_02071c88;
extern byte UNK_0207153c;

/* ================================================================
 * FUN_02070dd0 @ 02070dd0
 * ================================================================ */

void FUN_02070dd0(uint param_1)

{
  char cVar1;
  ushort uVar2;
  int iVar3;
  int iVar4;
  undefined1 uVar5;
  uint uVar6;
  uint uVar7;
  char *pcVar8;
  undefined4 *puVar9;
  byte bVar10;
  int iVar11;
  undefined4 *puVar12;
  byte *pbVar13;
  int iVar14;
  int iVar15;
  int iVar16;
  bool bVar17;

  puVar9 = DAT_0207136c;
  uVar7 = *(uint *)(param_1 + 0x54);
  iVar14 = *(int *)(param_1 + 100);
  if (*(char *)(uVar7 + 0x14) == '\x04') {
    uVar6 = 0;
    *(undefined4 *)(param_1 + 0x30) = *(undefined4 *)(uVar7 + 0x30);
    iVar11 = DAT_02071384;
    *(undefined4 *)(param_1 + 0x34) = *(undefined4 *)(uVar7 + 0x34);
    iVar16 = DAT_02071388;
    pbVar13 = (byte *)(param_1 + 0x17);
    *(short *)(param_1 + 0x36) = *(short *)(param_1 + 0x36) + *(short *)(uVar7 + 0x3a);
    iVar3 = DAT_0207138c;
    *(undefined1 *)(param_1 + 0x2a) = *(undefined1 *)(uVar7 + 0x2a);
    iVar4 = DAT_02071390;
    do {
      *(short *)(iVar14 + 0x32) =
           *(short *)(param_1 + 0x32) +
           (short)*(char *)(uVar6 + iVar3 + (uint)*(byte *)(param_1 + 0x2a) * 2);
      *(short *)(iVar14 + 0x36) =
           *(short *)(param_1 + 0x36) +
           (short)*(char *)(uVar6 + iVar4 + (uint)*(byte *)(param_1 + 0x2a) * 2);
      uVar7 = (uint)*(byte *)(param_1 + 0x2a);
      iVar15 = (uint)*(byte *)(iVar14 + 10) * 0x94 + iVar11;
      cVar1 = *(char *)(iVar15 + 2);
      if (uVar6 == 0) {
        uVar7 = uVar7 ^ 4;
      }
      bVar17 = cVar1 == '\0';
      if (bVar17) {
        cVar1 = *(char *)(iVar14 + 0x14);
      }
      if (bVar17 && cVar1 == '\x13') {
        bVar10 = *(byte *)(iVar15 + 0xd);
        if (((bVar10 & 0x80) == 0) && ((uint)bVar10 != uVar7 * 4)) {
          bVar10 = *(byte *)(iVar14 + 0x5e) & 0xfc | 2;
        }
        else {
          *(char *)(iVar14 + 0x2a) = (char)uVar7;
          *(undefined1 *)((uint)*(byte *)(iVar14 + 10) * 0x94 + iVar11 + 0x1a) = 0x80;
          *(ushort *)((uint)*(byte *)(iVar14 + 10) * 0x94 + iVar11 + 8) =
               (ushort)*pbVar13 + *(short *)(iVar16 + (uint)*(byte *)(iVar14 + 0x2a) * 2);
          bVar10 = *(byte *)(iVar14 + 0x5e) & 0xfc | 3;
        }
        *(byte *)(iVar14 + 0x5e) = bVar10;
      }
      uVar6 = uVar6 + 1 & 0xff;
      pbVar13 = (byte *)(param_1 + 0x18);
      iVar14 = *(int *)(param_1 + 0x68);
    } while (uVar6 < 2);
    if (*(char *)(param_1 + 0x17) == '\0') {
      if (((*(char *)(param_1 + 0x18) == '\0') ||
          (*(char *)(DAT_02071394 + (uint)*(byte *)(iVar14 + 10) * 0x94) != -0x80)) ||
         ((*(ushort *)(DAT_02071398 + (uint)*(byte *)(iVar14 + 10) * 2) & 1) == 0))
      goto LAB_020712c4;
      *(undefined1 *)(param_1 + 0x17) = 1;
      *(undefined1 *)(param_1 + 0x18) = 0;
    }
    else {
      uVar7 = (uint)*(byte *)(*(int *)(param_1 + 100) + 10);
      if ((*(char *)(DAT_02071394 + uVar7 * 0x94) != -0x80) ||
         ((*(ushort *)(DAT_02071398 + uVar7 * 2) & 1) == 0)) goto LAB_020712c4;
      *(undefined1 *)(param_1 + 0x17) = 0;
      *(undefined1 *)(param_1 + 0x18) = 1;
    }
    *(undefined2 *)(param_1 + 0x6e) = 0x10;
    *(undefined1 *)(param_1 + 0x6c) = 8;
    FUN_02085f80(0x97);
LAB_020712c4:
    cVar1 = *(char *)(param_1 + 0x6c);
    iVar14 = *(int *)(param_1 + 0x54);
    *(undefined1 *)(param_1 + 0x21) =
         *(undefined1 *)
          (((uint)*(byte *)(param_1 + 0x17) | (uint)*(byte *)(param_1 + 0x18) << 2) +
          DAT_0207139c + (uint)*(byte *)(param_1 + 0x2a) * 9);
    *(undefined1 *)(param_1 + 0x2f) = *(undefined1 *)(iVar14 + 0x2f);
    if (cVar1 == '\0') {
      return;
    }
    *(char *)(param_1 + 0x6c) = cVar1 + -1;
    if (cVar1 == '\x01') {
      pcVar8 = (char *)(param_1 + 0x17);
      if (*(byte *)(param_1 + 0x17) == 0) {
        pcVar8 = (char *)(param_1 + 0x18);
      }
      if (*pcVar8 != '\x01') {
        return;
      }
      *pcVar8 = '\x02';
      *(undefined1 *)(param_1 + 0x6c) = 0xf;
      *(undefined2 *)(param_1 + 0x6e) = 0x80;
      return;
    }
    *(uint *)(iVar14 + 0x6c) = *(int *)(iVar14 + 0x6c) + (uint)*(ushort *)(param_1 + 0x6e);
    if (0x200 < *(uint *)(*(int *)(param_1 + 0x54) + 0x6c)) {
      *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x6c) = 0x200;
    }
    return;
  }
  if (*(char *)(uVar7 + 0x14) != '\x05') {
    *(undefined1 *)(DAT_0207137c + (uint)*(byte *)(iVar14 + 10) * 0x94) = 0x81;
    *(undefined4 *)(iVar14 + 0x10) = 0x18000;
    *(undefined4 *)(iVar14 + 0xc) = 0x80;
    uVar5 = FUN_02071a70(param_1,iVar14,0);
    *(undefined1 *)(iVar14 + 0x2b) = uVar5;
    iVar11 = DAT_02071380;
    iVar16 = (uint)*(byte *)(iVar14 + 10) * 0x94;
    *(ushort *)(DAT_02071370 + iVar16) = *(ushort *)(DAT_02071370 + iVar16) & (ushort)DAT_02071374;
    uVar7 = (uint)*(byte *)(iVar14 + 10);
    puVar12 = (undefined4 *)(uVar7 * 0x94);
    *(undefined1 *)(iVar11 + (int)puVar12) = 1;
    cVar1 = *(char *)(param_1 + 0xb);
    if (cVar1 == '\0') {
LAB_02070ff4:
      if (cVar1 == '\0') {
        uVar7 = puVar9[(uint)*(byte *)(iVar14 + 10) * 0x25];
        puVar12 = puVar9 + (uint)*(byte *)(iVar14 + 10) * 0x25;
      }
      if (cVar1 == '\0' && uVar7 == param_1) goto LAB_02071010;
    }
    else {
      uVar7 = *(uint *)(param_1 + 0x54);
      puVar12 = DAT_0207136c + (uint)*(byte *)(iVar14 + 10) * 0x25;
      if (DAT_0207136c[(uint)*(byte *)(iVar14 + 10) * 0x25] != uVar7) goto LAB_02070ff4;
LAB_02071010:
      *puVar12 = 0;
    }
    iVar16 = *(int *)(param_1 + 0x68);
    *(undefined1 *)(DAT_0207137c + (uint)*(byte *)(iVar16 + 10) * 0x94) = 0x81;
    *(undefined4 *)(iVar16 + 0x10) = 0x18000;
    *(undefined4 *)(iVar16 + 0xc) = 0x80;
    uVar5 = FUN_02071a70(param_1,iVar16,1);
    *(undefined1 *)(iVar16 + 0x2b) = uVar5;
    iVar14 = DAT_02071380;
    iVar11 = (uint)*(byte *)(iVar16 + 10) * 0x94;
    *(ushort *)(DAT_02071370 + iVar11) = *(ushort *)(DAT_02071370 + iVar11) & 0xefff;
    uVar7 = (uint)*(byte *)(iVar16 + 10);
    *(undefined1 *)(iVar14 + uVar7 * 0x94) = 1;
    cVar1 = *(char *)(param_1 + 0xb);
    if (cVar1 == '\0') {
LAB_020710ac:
      puVar9 = DAT_0207136c;
      if (cVar1 == '\0') {
        uVar7 = DAT_0207136c[(uint)*(byte *)(iVar16 + 10) * 0x25];
        puVar9 = DAT_0207136c + (uint)*(byte *)(iVar16 + 10) * 0x25;
      }
      if (cVar1 != '\0' || uVar7 != param_1) goto LAB_020710d0;
    }
    else {
      uVar7 = *(uint *)(param_1 + 0x54);
      puVar9 = DAT_0207136c + (uint)*(byte *)(iVar16 + 10) * 0x25;
      if (DAT_0207136c[(uint)*(byte *)(iVar16 + 10) * 0x25] != uVar7) goto LAB_020710ac;
    }
    *puVar9 = 0;
    goto LAB_020710d0;
  }
  iVar11 = (uint)*(byte *)(iVar14 + 10) * 0x94;
  uVar6 = *(ushort *)(DAT_02071370 + iVar11) & DAT_02071374;
  *(short *)(DAT_02071370 + iVar11) = (short)uVar6;
  cVar1 = *(char *)(param_1 + 0xb);
  if (cVar1 == '\0') {
LAB_02070e44:
    puVar9 = DAT_0207136c;
    if (cVar1 == '\0') {
      uVar6 = DAT_0207136c[(uint)*(byte *)(iVar14 + 10) * 0x25];
      puVar9 = DAT_0207136c + (uint)*(byte *)(iVar14 + 10) * 0x25;
    }
    if (cVar1 == '\0' && uVar6 == param_1) goto LAB_02070e64;
  }
  else {
    uVar6 = DAT_0207136c[(uint)*(byte *)(iVar14 + 10) * 0x25];
    puVar9 = DAT_0207136c + (uint)*(byte *)(iVar14 + 10) * 0x25;
    if (uVar6 != uVar7) goto LAB_02070e44;
LAB_02070e64:
    *puVar9 = 0;
  }
  puVar9 = DAT_0207136c;
  if (*(char *)(iVar14 + 0x14) == '\n') {
    FUN_0208a954(iVar14);
  }
  else {
    uVar2 = (ushort)DAT_02071378;
    *(ushort *)(iVar14 + 0x32) = *(ushort *)(*(int *)(param_1 + 0x54) + 0x32) & uVar2 | 8;
    *(ushort *)(iVar14 + 0x36) = *(ushort *)(*(int *)(param_1 + 0x54) + 0x36) & uVar2 | 8;
  }
  iVar14 = *(int *)(param_1 + 0x68);
  puVar12 = (undefined4 *)((uint)*(byte *)(iVar14 + 10) * 0x94);
  uVar7 = *(ushort *)(DAT_02071370 + (int)puVar12) & DAT_02071374;
  *(short *)(DAT_02071370 + (int)puVar12) = (short)uVar7;
  cVar1 = *(char *)(param_1 + 0xb);
  if (cVar1 == '\0') {
LAB_02070f00:
    if (cVar1 == '\0') {
      uVar7 = puVar9[(uint)*(byte *)(iVar14 + 10) * 0x25];
      puVar12 = puVar9 + (uint)*(byte *)(iVar14 + 10) * 0x25;
    }
    if (cVar1 == '\0' && uVar7 == param_1) goto LAB_02070f1c;
  }
  else {
    uVar7 = *(uint *)(param_1 + 0x54);
    puVar12 = DAT_0207136c + (uint)*(byte *)(iVar14 + 10) * 0x25;
    if (DAT_0207136c[(uint)*(byte *)(iVar14 + 10) * 0x25] != uVar7) goto LAB_02070f00;
LAB_02070f1c:
    *puVar12 = 0;
  }
  if (*(char *)(iVar14 + 0x14) == '\n') {
    FUN_0208a954(iVar14);
  }
  else {
    uVar2 = (ushort)DAT_02071378;
    *(ushort *)(iVar14 + 0x32) = *(ushort *)(*(int *)(param_1 + 0x54) + 0x32) & uVar2 | 8;
    *(ushort *)(iVar14 + 0x36) = *(ushort *)(*(int *)(param_1 + 0x54) + 0x36) & uVar2 | 8;
  }
LAB_020710d0:
  *(undefined4 *)(param_1 + 0x68) = 0;
  *(undefined4 *)(param_1 + 100) = 0;
  *(undefined1 *)(param_1 + 0x14) = 2;
  FUN_020709fc(param_1);
  return;
}

/* ================================================================
 * FUN_020713a0 @ 020713a0
 * ================================================================ */

void FUN_020713a0(int param_1)

{
  char cVar1;
  char cVar2;
  bool bVar3;
  byte bVar4;
  undefined1 uVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  undefined4 uVar9;
  uint uVar10;
  int iVar11;
  uint uVar12;

  iVar6 = FUN_02056f30(*(undefined2 *)(param_1 + 0x50));
  bVar3 = false;
  bVar4 = 0;
  for (iVar11 = DAT_020719d0; (bVar4 < 4 && (*(int *)(iVar11 + 0x28) != 0)); iVar11 = iVar11 + 0x94)
  {
    if (((*(byte *)(iVar11 + 2) & 0x80) == 0) && ((*(ushort *)(iVar11 + 0x36) & 0x1000) == 0)) {
      FUN_02085e08(param_1);
    }
    bVar4 = bVar4 + 1;
  }
  uVar7 = *(uint *)(param_1 + 0x6c);
  *(uint *)(param_1 + 0x7c) = *(int *)(param_1 + 0x7c) + uVar7;
  if (uVar7 != 0) {
    *(undefined4 *)(param_1 + 0xc) = 0x100;
    uVar7 = (uVar7 >> 8) + 1 & 0xff;
    if (uVar7 != 0) {
      uVar10 = DAT_020719d4 - 3;
      uVar12 = DAT_020719d4 - 2;
      do {
        *(char *)(param_1 + 0x2b) = *(char *)(param_1 + 0x2a) << 2;
        if (uVar7 == 1) {
          *(uint *)(param_1 + 0xc) = *(uint *)(param_1 + 0x6c) & 0xff;
        }
        if ((*(ushort *)(param_1 + 0x74) & 0x4000) != 0) {
          *(ushort *)
           (iVar6 + (((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_020719d8 + 0x1a)
                           ) >> 4) << 6 |
                    (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_020719d8 + 0x18))
                    >> 4) * 2 + 4) = *(ushort *)(param_1 + 0x74);
          *(undefined2 *)(param_1 + 0x74) = 0;
        }
        FUN_0204fd40(param_1);
        uVar8 = FUN_02083c84(param_1);
        if (uVar8 != *(ushort *)(param_1 + 0x74)) {
          *(short *)(param_1 + 0x74) = (short)uVar8;
          if (uVar8 == uVar10 || uVar8 == uVar12) {
            *(short *)(iVar6 + (((int)((int)*(short *)(param_1 + 0x36) -
                                      (uint)*(ushort *)(DAT_020719d8 + 0x1a)) >> 4) << 6 |
                               (int)((int)*(short *)(param_1 + 0x32) -
                                    (uint)*(ushort *)(DAT_020719d8 + 0x18)) >> 4) * 2 + 4) =
                 (short)DAT_020719d4;
          }
          else {
            iVar11 = FUN_02083cdc(param_1);
            if (iVar11 == 0x2e) {
              bVar3 = true;
            }
            if (iVar11 < 0x61) {
              if (iVar11 < 0x60) {
                if (iVar11 < 0x3a) {
                    /* WARNING: Could not recover jumptable at 0x02071534. Too many branches */
                    /* WARNING: Treating indirect jump as call */
                  if (-1 < iVar11 + -0x2e) {
                    (*(code *)(&UNK_0207153c + (iVar11 + -0x2e) * 4))();
                    return;
                  }
LAB_02071688:
                  iVar11 = FUN_02083cdc(param_1);
                  if (iVar11 == 0x21) {
                    *(undefined1 *)(param_1 + 0x14) = 5;
                    FUN_020859e0(param_1,8);
                    iVar11 = DAT_020719dc;
                    iVar6 = *(int *)(param_1 + 0x58);
                    *(undefined1 *)
                     (DAT_020719dc + (short)(ushort)*(byte *)(*(int *)(iVar6 + 100) + 10) * 0x94) =
                         0x11;
                    *(undefined1 *)
                     (iVar11 + (short)(ushort)*(byte *)(*(int *)(iVar6 + 0x68) + 10) * 0x94) = 0x11;
                    FUN_0202954c();
                    return;
                  }
                  if (iVar11 == 0x43) {
                    *(undefined1 *)(param_1 + 0x14) = 5;
                    FUN_020859e0(param_1,8);
                    iVar11 = DAT_020719dc;
                    iVar6 = *(int *)(param_1 + 0x58);
                    *(undefined1 *)
                     (DAT_020719dc + (short)(ushort)*(byte *)(*(int *)(iVar6 + 100) + 10) * 0x94) =
                         3;
                    *(undefined1 *)
                     (iVar11 + (short)(ushort)*(byte *)(*(int *)(iVar6 + 0x68) + 10) * 0x94) = 3;
                    FUN_0202954c();
                    return;
                  }
                  *(undefined2 *)(param_1 + 0x32) = *(undefined2 *)(param_1 + 0x70);
                  *(undefined2 *)(param_1 + 0x36) = *(undefined2 *)(param_1 + 0x72);
                  *(undefined1 *)(param_1 + 0x2a) = *(undefined1 *)(param_1 + 0x78);
                  *(undefined4 *)(param_1 + 0x7c) = 0;
                  return;
                }
                if (iVar11 != 0x5e) goto LAB_02071688;
              }
            }
            else if (iVar11 < 99) {
              if (iVar11 != 0x62) goto LAB_02071688;
            }
            else if (iVar11 != 100) goto LAB_02071688;
          }
        }
        uVar7 = uVar7 - 1 & 0xff;
      } while (uVar7 != 0);
    }
  }
  bVar4 = *(byte *)(param_1 + 0x2a);
  if ((bVar4 & 1) != 0) goto switchD_0207188c_default;
  iVar11 = 0;
  uVar5 = FUN_02083ccc(param_1,(int)*(char *)(DAT_020719e0 + (uint)bVar4),
                       (int)*(char *)(DAT_020719e0 + bVar4 + 1));
  switch(uVar5) {
  case 0x5d:
    if (!bVar3) {
      uVar9 = 0;
LAB_02071818:
      iVar11 = FUN_02076858(0x54,uVar9,0);
    }
    break;
  case 0x5e:
    break;
  case 0x5f:
    if (!bVar3) {
      uVar9 = 2;
      goto LAB_02071818;
    }
    break;
  case 0x60:
    break;
  case 0x61:
    if (!bVar3) {
      uVar9 = 3;
      goto LAB_02071818;
    }
    break;
  case 0x62:
    break;
  case 99:
    if (!bVar3) {
      uVar9 = 1;
      goto LAB_02071818;
    }
  }
  iVar6 = DAT_020719e0;
  if (iVar11 != 0) {
    *(short *)(iVar11 + 0x32) =
         *(short *)(param_1 + 0x32) +
         (short)*(char *)(DAT_020719e0 + (uint)*(byte *)(param_1 + 0x2a));
    *(short *)(iVar11 + 0x36) =
         *(short *)(param_1 + 0x36) + (short)*(char *)(iVar6 + *(byte *)(param_1 + 0x2a) + 1);
  }
  uVar7 = *(byte *)(param_1 + 0x2a) ^ 4;
  cVar1 = *(char *)(DAT_020719e0 + uVar7);
  cVar2 = *(char *)(DAT_020719e0 + uVar7 + 1);
  uVar5 = FUN_02083ccc(param_1,(int)cVar1,(int)cVar2);
  switch(uVar5) {
  case 0x5e:
    iVar11 = FUN_02076858(0x54,4,0);
    goto joined_r0x020718e0;
  case 0x5f:
    break;
  case 0x60:
    iVar11 = FUN_02076858(0x54,6,0);
joined_r0x020718e0:
    if (iVar11 != 0) {
LAB_0207191c:
      *(short *)(iVar11 + 0x32) = *(short *)(param_1 + 0x32) + (short)cVar1;
      *(short *)(iVar11 + 0x36) = *(short *)(param_1 + 0x36) + (short)cVar2;
    }
    break;
  case 0x61:
    break;
  case 0x62:
    iVar11 = FUN_02076858(0x54,7,0);
    goto joined_r0x020718fc;
  case 99:
    break;
  case 100:
    iVar11 = FUN_02076858(0x54,5,0);
joined_r0x020718fc:
    if (iVar11 == 0) break;
    goto LAB_0207191c;
  }
switchD_0207188c_default:
  uVar9 = DAT_020719e4;
  if ((*(byte *)(param_1 + 0x2a) & 0xfb) == 0) {
    uVar9 = DAT_020719e8;
  }
  *(undefined4 *)(param_1 + 0x48) = uVar9;
  if ((*(byte *)(param_1 + 0x2a) & 1) == 0) {
    if ((*(byte *)(param_1 + 0x2a) & 2) == 0) {
      *(ushort *)(param_1 + 0x32) = (*(ushort *)(param_1 + 0x32) & (ushort)DAT_020719ec) + 8;
    }
    else {
      *(ushort *)(param_1 + 0x36) = (*(ushort *)(param_1 + 0x36) & (ushort)DAT_020719ec) + 8;
    }
  }
  if ((*(int *)(param_1 + 0x6c) != 0) &&
     (iVar11 = *(int *)(param_1 + 0x6c) + -0x20, *(int *)(param_1 + 0x6c) = iVar11, iVar11 < 0)) {
    *(undefined4 *)(param_1 + 0x6c) = 0;
  }
  if (*(char *)(param_1 + 0x23) != *(char *)(param_1 + 0x2a)) {
    FUN_020859e0(param_1);
  }
  if (0x1fff < *(uint *)(param_1 + 0x7c)) {
    FUN_02085f80(0x72);
    *(undefined4 *)(param_1 + 0x7c) = 0;
  }
  FUN_02071ae0(param_1);
  return;
}

/* ================================================================
 * FUN_020719f0 @ 020719f0
 * ================================================================ */

void FUN_020719f0(int param_1)

{
  byte bVar1;
  undefined2 uVar2;
  uint uVar3;
  bool bVar4;

  FUN_02085980(param_1);
  if ((*(byte *)(param_1 + 0x29) & 0x80) == 0) {
    return;
  }
  bVar1 = *(byte *)(param_1 + 0x78);
  *(byte *)(param_1 + 0x2a) = bVar1;
  bVar4 = (bVar1 & 0xfb) != 0;
  uVar3 = (uint)bVar1;
  if (bVar4) {
    uVar3 = DAT_02071a68;
  }
  if (!bVar4) {
    uVar3 = DAT_02071a6c;
  }
  *(undefined2 *)(param_1 + 0x36) = *(undefined2 *)(param_1 + 0x72);
  *(uint *)(param_1 + 0x48) = uVar3;
  *(undefined1 *)(param_1 + 0x18) = 0xff;
  *(undefined1 *)(param_1 + 0x17) = 0xff;
  *(undefined1 *)(param_1 + 0x23) = 0xff;
  *(undefined1 *)(param_1 + 0x14) = 1;
  *(undefined4 *)(param_1 + 0x7c) = 0;
  *(undefined2 *)(param_1 + 0x32) = *(undefined2 *)(param_1 + 0x70);
  *(undefined4 *)(param_1 + 0x6c) = 0;
  uVar2 = FUN_02083c84(param_1);
  *(undefined2 *)(param_1 + 0x74) = uVar2;
  return;
}

/* ================================================================
 * FUN_02071a70 @ 02071a70
 * ================================================================ */

int FUN_02071a70(int param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  uint uVar2;
  int iVar3;

  iVar1 = DAT_02071adc;
  uVar2 = (uint)*(byte *)(param_1 + 0x2a);
  if (param_3 == 0) {
    uVar2 = uVar2 ^ 4;
  }
  uVar2 = uVar2 + 2 & 6;
  iVar3 = FUN_02083d18(param_2,(int)*(char *)(DAT_02071adc + uVar2),
                       (int)*(char *)(DAT_02071adc + uVar2 + 1));
  while (iVar3 != 0) {
    uVar2 = uVar2 + 2 & 6;
    iVar3 = FUN_02083d18(param_2,(int)*(char *)(iVar1 + uVar2),(int)*(char *)(iVar1 + uVar2 + 1));
  }
  return uVar2 << 2;
}

/* ================================================================
 * FUN_02071ae0 @ 02071ae0
 * ================================================================ */

void FUN_02071ae0(void)

{
  return;
}

/* ================================================================
 * FUN_02071ae4 @ 02071ae4
 * ================================================================ */

void FUN_02071ae4(int param_1)

{
  (**(code **)(DAT_02071afc + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_02071b00 @ 02071b00
 * ================================================================ */

void FUN_02071b00(int param_1)

{
  int iVar1;
  char cVar2;

  *(undefined1 *)(param_1 + 0x14) = 1;
  *(undefined1 *)(param_1 + 0x17) = 4;
  cVar2 = FUN_02061dc4(param_1);
  iVar1 = DAT_02071b68;
  *(char *)(param_1 + 0xb) = cVar2 * '\x18';
  FUN_0208366c(0,(((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(iVar1 + 0x1a)) >> 4) <<
                  6 | (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(iVar1 + 0x18)) >> 4)
                 & 0xffff,*(undefined2 *)(param_1 + 0x50));
  return;
}

/* ================================================================
 * FUN_02071b6c @ 02071b6c
 * ================================================================ */

void FUN_02071b6c(int param_1)

{
  char cVar1;
  int iVar2;

  cVar1 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar1;
  if (cVar1 != '\0') {
    return;
  }
  *(undefined1 *)(param_1 + 0x17) = 4;
  *(undefined1 *)(param_1 + 0x14) = 2;
  iVar2 = DAT_02071be4;
  if (*(char *)(DAT_02071be0 + 1) != '\x01') {
    iVar2 = DAT_02071be8;
  }
  FUN_02071c8c(param_1,iVar2 + ((*(byte *)(param_1 + 10) & 3) * 6 + (uint)*(byte *)(param_1 + 0xb))
                               * 2);
  FUN_02085f98(param_1,0x11);
  return;
}

/* ================================================================
 * FUN_02071bec @ 02071bec
 * ================================================================ */

void FUN_02071bec(int param_1)

{
  byte bVar1;
  char cVar2;
  int iVar3;
  undefined *puVar4;
  bool bVar5;

  cVar2 = *(char *)(param_1 + 0x17) + -1;
  *(char *)(param_1 + 0x17) = cVar2;
  if (cVar2 != '\0') {
    return;
  }
  bVar1 = *(byte *)(param_1 + 10);
  if (*(char *)(DAT_02071c78 + 1) == '\x01') {
    bVar5 = (bVar1 & 4) == 0;
    iVar3 = (bVar1 & 3) * 6 + (uint)*(byte *)(param_1 + 0xb);
    puVar4 = PTR_DAT_02071c80;
    if (!bVar5) {
      puVar4 = PTR_DAT_02071c7c + iVar3 * 2;
    }
  }
  else {
    bVar5 = (bVar1 & 4) == 0;
    iVar3 = (bVar1 & 3) * 6 + (uint)*(byte *)(param_1 + 0xb);
    puVar4 = PTR_DAT_02071c88;
    if (!bVar5) {
      puVar4 = PTR_DAT_02071c84 + iVar3 * 2;
    }
  }
  if (bVar5) {
    puVar4 = puVar4 + iVar3 * 2;
  }
  FUN_02071c8c(param_1,puVar4);
  FUN_0204be88(param_1);
  return;
}

/* ================================================================
 * FUN_02071c8c @ 02071c8c
 * ================================================================ */

void FUN_02071c8c(int param_1,undefined2 *param_2,undefined4 param_3,undefined4 param_4)

{
  uint uVar1;
  byte bVar2;
  uint uVar3;

  bVar2 = *(byte *)(param_1 + 10);
  uVar3 = ((int)((int)*(short *)(param_1 + 0x36) - (uint)*(ushort *)(DAT_02071f48 + 0x1a)) >> 4) <<
          6 | (int)((int)*(short *)(param_1 + 0x32) - (uint)*(ushort *)(DAT_02071f48 + 0x18)) >> 4;
  uVar1 = uVar3 & 0xffff;
  switch(bVar2 & 3) {
  case 0:
    FUN_02021c28(*param_2,uVar1 - 0x41 & 0xffff,0x8000,bVar2,param_4);
    FUN_02021c28(param_2[1],uVar1 - 0x40 & 0xffff,0x8000);
    FUN_02021c28(param_2[2],uVar1 - 0x3f & 0xffff,0x8000);
    FUN_02021c28(param_2[3],uVar1 - 1 & 0xffff,0x4000);
    FUN_02021c28(param_2[4],uVar3 & 0xffff,0x4000);
    FUN_02021c28(param_2[5],uVar1 + 1 & 0xffff,0x4000);
    return;
  case 1:
    FUN_02021c28(*param_2,uVar1 - 0x40 & 0xffff,0x4000,bVar2,param_4);
    FUN_02021c28(param_2[1],uVar1 - 0x3f & 0xffff,0x8000);
    FUN_02021c28(param_2[2],uVar3 & 0xffff,0x4000);
    FUN_02021c28(param_2[3],uVar1 + 1 & 0xffff,0x8000);
    FUN_02021c28(param_2[4],uVar1 + 0x40 & 0xffff,0x4000);
    FUN_02021c28(param_2[5],uVar1 + 0x41 & 0xffff,0x8000);
    return;
  case 2:
    FUN_02021c28(*param_2,uVar1 - 1 & 0xffff,0x4000,bVar2,param_4);
    FUN_02021c28(param_2[1],uVar3 & 0xffff,0x4000);
    FUN_02021c28(param_2[2],uVar1 + 1 & 0xffff,0x4000);
    FUN_02021c28(param_2[3],uVar1 + 0x3f & 0xffff,0x8000);
    FUN_02021c28(param_2[4],uVar1 + 0x40 & 0xffff,0x8000);
    FUN_02021c28(param_2[5],uVar1 + 0x41 & 0xffff,0x8000);
    return;
  case 3:
    FUN_02021c28(*param_2,uVar1 - 0x41 & 0xffff,0x8000,bVar2,param_4);
    FUN_02021c28(param_2[1],uVar1 - 0x40 & 0xffff,0x4000);
    FUN_02021c28(param_2[2],uVar1 - 1 & 0xffff,0x8000);
    FUN_02021c28(param_2[3],uVar3 & 0xffff,0x4000);
    FUN_02021c28(param_2[4],uVar1 + 0x3f & 0xffff,0x8000);
    FUN_02021c28(param_2[5],uVar1 + 0x40 & 0xffff,0x4000);
    return;
  default:
    return;
  }
}

/* ================================================================
 * FUN_02071f4c @ 02071f4c
 * ================================================================ */

void FUN_02071f4c(int param_1)

{
  if (*(char *)(param_1 + 10) != '\0') {
    (**(code **)(DAT_02071f80 + (uint)*(byte *)(param_1 + 0x14) * 4))();
    return;
  }
  (**(code **)(DAT_02071f84 + (uint)*(byte *)(param_1 + 0x14) * 4))();
  return;
}

/* ================================================================
 * FUN_02072554 @ 02072554
 * ================================================================ */

void FUN_02072554(int param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;

  iVar3 = FUN_02076858(0x56,param_1,0);
  if (iVar3 == 0) {
    return;
  }
  uVar1 = param_1 + 7U & 0xf;
  *(char *)(iVar3 + 0x20) = (char)(param_1 << 5);
  *(undefined1 *)(iVar3 + 0x2a) = 4;
  iVar2 = DAT_020725cc;
  *(uint *)(iVar3 + 0x1c) =
       *(uint *)(iVar3 + 0x1c) & 0xff003fff | uVar1 << 0x10 | uVar1 << 0x14 | 0x8000;
  *(int *)(iVar2 + param_1 * 0x94) = iVar3;
  return;
}

/* ================================================================
 * FUN_020725d0 @ 020725d0
 * ================================================================ */

void FUN_020725d0(int param_1,undefined1 param_2,undefined1 param_3)

{
  int iVar1;
  int iVar2;

  iVar1 = DAT_02072600;
  param_1 = param_1 * 0x94;
  *(undefined1 *)(DAT_020725fc + param_1) = 3;
  iVar2 = DAT_02072604;
  *(undefined1 *)(iVar1 + param_1) = param_2;
  *(undefined1 *)(iVar2 + param_1) = param_3;
  return;
}

/* ================================================================
 * FUN_02072608 @ 02072608
 * ================================================================ */

void FUN_02072608(int param_1)

{
  FUN_02072638(param_1);
  (**(code **)(DAT_02072634 + (uint)*(byte *)(param_1 + 0x14) * 4))(param_1);
  FUN_02072684(param_1);
  return;
}

/* ================================================================
 * FUN_02072638 @ 02072638
 * ================================================================ */

void FUN_02072638(int param_1)

{
  char cVar1;
  int iVar2;
  bool bVar3;

  cVar1 = -0x6c;
  iVar2 = (uint)*(byte *)(param_1 + 10) * 0x94 + DAT_02072680;
  bVar3 = *(char *)(param_1 + 0x14) != '\0';
  if (bVar3) {
    cVar1 = *(char *)(iVar2 + 0xc);
  }
  if (bVar3 && cVar1 != '\0') {
    *(char *)(param_1 + 0x14) = cVar1;
    *(undefined1 *)(param_1 + 0x15) = 0;
    *(undefined1 *)(param_1 + 0x17) = *(undefined1 *)(iVar2 + 0x38);
    *(undefined1 *)(param_1 + 0x18) = *(undefined1 *)(iVar2 + 0x39);
    *(undefined1 *)(iVar2 + 0xc) = 0;
    return;
  }
  return;
}

/* ================================================================
 * FUN_02072684 @ 02072684
 * ================================================================ */

void FUN_02072684(int param_1)

{
  ushort uVar1;
  uint uVar2;
  uint uVar3;
  bool bVar4;

  uVar1 = *(ushort *)((uint)*(byte *)(param_1 + 10) * 0x94 + DAT_02072700 + 8);
  uVar2 = uVar1 & 0xff;
  *(uint *)(param_1 + 0x1c) =
       *(uint *)(param_1 + 0x1c) & 0xfffffffb | (uint)(4 < *(byte *)(param_1 + 0x2a)) << 2;
  bVar4 = uVar2 < 0xa8;
  if (bVar4) {
    uVar2 = uVar2 + (*(byte *)(param_1 + 0x2a) >> 1);
  }
  uVar3 = (uint)*(byte *)(param_1 + 0x1b);
  if (bVar4) {
    uVar2 = uVar2 & 0xff;
  }
  bVar4 = (int)(uint)uVar1 >> 8 == uVar3;
  if (bVar4) {
    uVar3 = (uint)*(byte *)(param_1 + 0x23);
  }
  if (!bVar4 || uVar2 != uVar3) {
    *(char *)(param_1 + 0x1b) = (char)(uVar1 >> 8);
    FUN_020859a0(param_1);
    return;
  }
  return;
}

/* ================================================================
 * FUN_02072704 @ 02072704
 * ================================================================ */

void FUN_02072704(int param_1)

{
  int iVar1;
  int iVar2;

  iVar1 = DAT_0207276c;
  *(char *)(param_1 + 0x14) = *(char *)(param_1 + 0x14) + '\x01';
  *(undefined1 *)(param_1 + 0x1b) = 0;
  if (*(char *)(iVar1 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) != '\0') {
    FUN_02072638(param_1);
  }
  iVar2 = DAT_02072770;
  iVar1 = (short)(ushort)*(byte *)(param_1 + 10) * 0x94;
  *(undefined1 *)(param_1 + 0x23) = 0xff;
  if (*(short *)(iVar2 + iVar1) == 0) {
    *(undefined2 *)(iVar2 + iVar1) = 0;
  }
  return;
}

/* ================================================================
 * FUN_020727e0 @ 020727e0
 * ================================================================ */

void FUN_020727e0(int param_1)

{
  char cVar1;

  if (*(char *)(param_1 + 0x17) != -1) {
    if (*(char *)(param_1 + 0x18) == '\0') {
      *(undefined4 *)(param_1 + 0xc) = 0x100;
      *(char *)(param_1 + 0x2b) = *(char *)(param_1 + 0x2a) << 2;
      FUN_0204fd40();
      cVar1 = *(char *)(param_1 + 0x17) + -1;
      *(char *)(param_1 + 0x17) = cVar1;
      if (cVar1 == '\0') {
        *(undefined1 *)(DAT_0207285c + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) = 2;
      }
    }
    else {
      *(char *)(param_1 + 0x18) = *(char *)(param_1 + 0x18) + -1;
    }
  }
  FUN_02085934(param_1,*(undefined1 *)(param_1 + 0x1b));
  return;
}

/* ================================================================
 * FUN_020728a8 @ 020728a8
 * ================================================================ */

void FUN_020728a8(int param_1,undefined4 param_2,int param_3,undefined4 param_4)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;

  cVar1 = *(char *)(param_1 + 0x17);
  uVar3 = param_4;
  if ((cVar1 != -1) &&
     (*(char *)(param_1 + 0x17) = cVar1 + -1, iVar2 = DAT_020728f8, cVar1 == '\x01')) {
    uVar3 = 2;
    *(undefined1 *)(DAT_020728f8 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) = 2;
    param_3 = iVar2;
  }
  if ((*(byte *)(param_1 + 0x29) & 0x80) != 0) {
    return;
  }
  FUN_02085934(param_1,*(undefined1 *)(param_1 + 0x1b),param_3,uVar3,param_4);
  return;
}

/* ================================================================
 * FUN_02072900 @ 02072900
 * ================================================================ */

void FUN_02072900(int param_1)

{
  (**(code **)(DAT_02072918 + (uint)*(byte *)(param_1 + 0x15) * 4))();
  return;
}

/* ================================================================
 * FUN_0207294c @ 0207294c
 * ================================================================ */

void FUN_0207294c(int param_1)

{
  byte bVar1;

  if (*(char *)(DAT_020729d0 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) != '\0') {
    bVar1 = *(byte *)(param_1 + 0x29);
    while ((bVar1 & 0x80) == 0) {
      FUN_02085980(param_1);
      bVar1 = *(byte *)(param_1 + 0x29);
    }
    FUN_02085934(param_1,*(undefined1 *)(param_1 + 0x1b));
    *(undefined1 *)(DAT_020729d0 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) = 0;
    return;
  }
  if ((*(byte *)(param_1 + 0x29) & 0x80) == 0) {
    FUN_02085934(param_1,*(undefined1 *)(param_1 + 0x1b));
    return;
  }
  return;
}

/* ================================================================
 * FUN_02072b38 @ 02072b38
 * ================================================================ */

void FUN_02072b38(int param_1)

{
  int iVar1;

  FUN_0204fd40();
  FUN_02085934(param_1,*(undefined1 *)(param_1 + 0x1b));
  *(int *)(param_1 + 0xc) = *(int *)(param_1 + 0xc) + -0x14;
  iVar1 = DAT_02072ba8;
  if (*(short *)(param_1 + 0x32) < 0xb9) {
    return;
  }
  *(undefined2 *)(param_1 + 0x32) = 0xb8;
  *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
  *(undefined2 *)(iVar1 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) = 0x1af;
  *(undefined1 *)(param_1 + 0x2a) = 2;
  FUN_02076fe0(0x1fc);
  return;
}

/* ================================================================
 * FUN_02072bc8 @ 02072bc8
 * ================================================================ */

void FUN_02072bc8(int param_1)

{
  (**(code **)(DAT_02072be0 + (uint)*(byte *)(param_1 + 0x15) * 4))();
  return;
}

/* ================================================================
 * FUN_02072be4 @ 02072be4
 * ================================================================ */

void FUN_02072be4(int param_1)

{
  int iVar1;

  iVar1 = DAT_02072c20;
  *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
  *(undefined4 *)(param_1 + 0x10) = 0x20000;
  *(undefined2 *)(iVar1 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) = 0xc;
  FUN_02076fe0(0xd);
  return;
}

/* ================================================================
 * FUN_02072c24 @ 02072c24
 * ================================================================ */

void FUN_02072c24(int param_1)

{
  int iVar1;

  FUN_02085934(param_1,*(undefined1 *)(param_1 + 0x1b));
  iVar1 = FUN_020856cc(param_1,0x4000);
  if (iVar1 != 0) {
    return;
  }
  FUN_02076fe0(0xe);
  *(undefined1 *)(DAT_02072c6c + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) = 2;
  return;
}

/* ================================================================
 * FUN_02072c8c @ 02072c8c
 * ================================================================ */

void FUN_02072c8c(int param_1)

{
  uint uVar1;
  undefined2 *puVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;

  uVar3 = DAT_02072d74;
  puVar2 = DAT_02072d70;
  if (*(byte *)(param_1 + 10) == 0) {
    *(undefined1 *)(param_1 + 0x2a) = 0;
    puVar2 = DAT_02072d70;
    *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
    *(undefined1 *)(param_1 + 0x2b) = 0x1e;
    *(undefined4 *)(param_1 + 0xc) = 0x140;
    *(undefined1 *)(param_1 + 0x17) = 0x1e;
    *puVar2 = 4;
  }
  else {
    *(undefined1 *)(param_1 + 0x15) = 4;
    *(undefined1 *)(param_1 + 0x2a) = 4;
    puVar2[(short)(ushort)*(byte *)(param_1 + 10) * 0x4a] = (short)uVar3;
    iVar4 = DAT_02072d78;
    uVar5 = *(uint *)(param_1 + 0x1c);
    uVar1 = *(uint *)(*(int *)(DAT_02072d78 + 0x28) + 0x1c) & 0xc0000000;
    *(uint *)(param_1 + 0x1c) = uVar5 & 0x3fffffff | uVar1;
    *(uint *)(param_1 + 0x1c) =
         uVar5 & 0x3fff3ffc | uVar1 | *(uint *)(*(int *)(iVar4 + 0x28) + 0x1c) & 0xc000;
  }
  *(undefined1 *)(param_1 + 0x2e) = 3;
  *(byte *)(param_1 + 0x5e) =
       *(byte *)(param_1 + 0x5e) & 0xf0 | 3U - *(char *)(param_1 + 10) & 3 | 4;
  return;
}

/* ================================================================
 * FUN_02072d7c @ 02072d7c
 * ================================================================ */

void FUN_02072d7c(int param_1)

{
  if (*(char *)(param_1 + 0x17) != '\0') {
    *(char *)(param_1 + 0x17) = *(char *)(param_1 + 0x17) + -1;
    return;
  }
  FUN_02085934(param_1,*(undefined1 *)(param_1 + 0x1b));
  FUN_0204fd40(param_1);
  if (*(short *)(param_1 + 0x36) < 0xe2) {
    *(undefined2 *)(param_1 + 0x36) = 0xe2;
    *(undefined1 *)(param_1 + 0x2b) = 0x18;
    *(undefined1 *)(param_1 + 0x2a) = 6;
    *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
    return;
  }
  return;
}

/* ================================================================
 * FUN_02072ddc @ 02072ddc
 * ================================================================ */

void FUN_02072ddc(int param_1)

{
  int iVar1;

  FUN_02085934(param_1,*(undefined1 *)(param_1 + 0x1b));
  FUN_0204fd40(param_1);
  iVar1 = DAT_02072e40;
  if (0x7f < *(short *)(param_1 + 0x32)) {
    return;
  }
  *(undefined2 *)(param_1 + 0x32) = 0x80;
  *(undefined1 *)(param_1 + 0x2a) = 4;
  *(undefined2 *)(iVar1 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) = 0;
  *(undefined1 *)(param_1 + 0x17) = 0x3c;
  *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
  return;
}

/* ================================================================
 * FUN_02072e44 @ 02072e44
 * ================================================================ */

void FUN_02072e44(int param_1)

{
  char cVar1;

  cVar1 = *(char *)(param_1 + 0x17);
  if (cVar1 == '\0') {
    if ((*(byte *)(param_1 + 0x29) & 0x80) == 0) {
      FUN_02085934(param_1,*(undefined1 *)(param_1 + 0x1b));
      return;
    }
    FUN_02076fe0(DAT_02072ef0);
    FUN_02076fe0(0x20);
    *DAT_02072eec = *DAT_02072eec | 4;
    *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
    return;
  }
  *(char *)(param_1 + 0x17) = cVar1 + -1;
  if (cVar1 != '\x01') {
    return;
  }
  *(short *)(DAT_02072ee8 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) = (short)DAT_02072ee4;
  FUN_02076fe0(0x228);
  *DAT_02072eec = *DAT_02072eec | 2;
  return;
}

/* ================================================================
 * FUN_02072ef4 @ 02072ef4
 * ================================================================ */

void FUN_02072ef4(int param_1)

{
  int iVar1;

  if (*(char *)(param_1 + 10) != '\0') {
    if ((*(byte *)(param_1 + 0x29) & 0x80) == 0) {
      FUN_02085934(param_1,*(undefined1 *)(param_1 + 0x1b));
    }
    iVar1 = DAT_02072f54;
    *(undefined2 *)(param_1 + 0x32) = *(undefined2 *)(*(int *)(DAT_02072f54 + 0x28) + 0x32);
    *(undefined2 *)(param_1 + 0x36) = *(undefined2 *)(*(int *)(iVar1 + 0x28) + 0x36);
  }
  if ((*DAT_02072f58 & 8) != 0) {
    *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
  }
  return;
}

/* ================================================================
 * FUN_02072f5c @ 02072f5c
 * ================================================================ */

void FUN_02072f5c(int param_1)

{
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  FUN_020506d8(param_1);
  if ((*DAT_02072fc0 & 0x10) == 0) {
    return;
  }
  *(undefined2 *)(DAT_02072fc4 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) = 0;
  *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
  *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) & 0xfffffffc | 1;
  return;
}

/* ================================================================
 * FUN_02072fc8 @ 02072fc8
 * ================================================================ */

void FUN_02072fc8(int param_1)

{
  int iVar1;
  char cVar2;

  if ((*DAT_02073034 & 0x20) == 0) {
    return;
  }
  cVar2 = '\x06';
  if (1 < *(byte *)(param_1 + 10)) {
    cVar2 = '\x02';
  }
  *(char *)(param_1 + 0x2a) = cVar2;
  iVar1 = DAT_02073038;
  *(char *)(param_1 + 0x2b) = cVar2 << 2;
  *(undefined4 *)(param_1 + 0xc) = 0x100;
  *(undefined1 *)(param_1 + 0x17) = 0xc;
  *(char *)(param_1 + 0x15) = *(char *)(param_1 + 0x15) + '\x01';
  *(undefined2 *)(iVar1 + (short)(ushort)*(byte *)(param_1 + 10) * 0x94) = 4;
  return;
}
