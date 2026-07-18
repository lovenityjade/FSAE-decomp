#ifndef GAME_ARM9_MIDDLE_02061818_RAW_H
#define GAME_ARM9_MIDDLE_02061818_RAW_H

#include <stdint.h>

typedef uint8_t byte;
typedef uint8_t undefined1;
typedef uint16_t undefined2;
typedef uint32_t undefined4;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef int bool;
typedef void code();

#ifndef true
#define true 1
#define false 0
#endif

/*
 * Intentionally unspecified parameter lists preserve the raw decompiler's
 * register-recovery uncertainty while keeping every recovered body parseable.
 */
void FUN_02061818();
void FUN_02061b88();
void FUN_02061be8();
void FUN_02061c04();
void FUN_02061c64();
void FUN_02061cb4();
void FUN_02061cf4();
void FUN_02061d0c();
bool FUN_02061dc4();
void FUN_02061e04();
void FUN_02061e44();
void FUN_020623c8();
void FUN_02062bd4();
void FUN_02062ce4();
void FUN_02062d90();
void FUN_02062dc4();
void FUN_02062e30();
void FUN_02062f24(int param_1, undefined1 param_2);
void FUN_02062fcc();
void FUN_0206305c();
void FUN_020630b0();
void FUN_020633bc();
void FUN_02063450();
void FUN_020635d8();
void FUN_02063678();
void FUN_020637c8();
void FUN_020637f4();
void FUN_02063870();
void FUN_020639a8();
void FUN_02063bac();
bool FUN_02063fd8();

#endif
