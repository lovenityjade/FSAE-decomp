#ifndef GAME_ARM9_MIDDLE_02070DD0_RAW_H
#define GAME_ARM9_MIDDLE_02070DD0_RAW_H

#include <stdint.h>

typedef uint8_t byte;
typedef int8_t sbyte;
typedef uint8_t undefined;
typedef uint8_t undefined1;
typedef uint16_t undefined2;
typedef uint32_t undefined4;
typedef uint64_t undefined8;
typedef int64_t longlong;
typedef uint64_t ulonglong;
typedef uint16_t ushort;
typedef uint32_t uint;
typedef int bool;
typedef void code();

#ifndef true
#define true 1
#define false 0
#endif

/* Raw declarations intentionally retain unspecified parameter lists. */
void FUN_02070dd0();
void FUN_020713a0();
void FUN_020719f0();
int FUN_02071a70();
void FUN_02071ae0();
void FUN_02071ae4();
void FUN_02071b00();
void FUN_02071b6c();
void FUN_02071bec();
void FUN_02071c8c();
void FUN_02071f4c();
void FUN_02072554();
void FUN_020725d0(int, undefined1, undefined1);
void FUN_02072608();
void FUN_02072638();
void FUN_02072684();
void FUN_02072704();
void FUN_020727e0();
void FUN_020728a8();
void FUN_02072900();
void FUN_0207294c();
void FUN_02072b38();
void FUN_02072bc8();
void FUN_02072be4();
void FUN_02072c24();
void FUN_02072c8c();
void FUN_02072d7c();
void FUN_02072ddc();
void FUN_02072e44();
void FUN_02072ef4();
void FUN_02072f5c();
void FUN_02072fc8();

#endif
