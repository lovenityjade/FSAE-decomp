/*
 * Repository-owned ARM9i layout material.
 *
 * No opaque target or SDK byte sequence is included here.  These four sections
 * emit only structural zero fill and the documented autoload fields.  Official
 * code comes from external TwlSDK archives selected by config/build/arm9i.json.
 */

    .syntax unified
    .arch armv5te

    .section .arm9i.prefix,"a",%progbits
    .balign 4
    .global gFsaeArm9iFilePrefix
    .type gFsaeArm9iFilePrefix,%object
gFsaeArm9iFilePrefix:
    .space 4, 0
    .size gFsaeArm9iFilePrefix, .-gFsaeArm9iFilePrefix

    .section .ltdmain.start_padding,"a",%progbits
    .balign 4
    .global gFsaeArm9iStartPadding
    .type gFsaeArm9iStartPadding,%object
gFsaeArm9iStartPadding:
    .space 32, 0
    .size gFsaeArm9iStartPadding, .-gFsaeArm9iStartPadding

    .section .ltdmain.end_padding,"a",%progbits
    .balign 4
    .global gFsaeArm9iEndPadding
    .type gFsaeArm9iEndPadding,%object
gFsaeArm9iEndPadding:
    .space 20, 0
    .size gFsaeArm9iEndPadding, .-gFsaeArm9iEndPadding

    .section .arm9i.autoload,"a",%progbits
    .balign 4
    .global gFsaeArm9iAutoload
    .type gFsaeArm9iAutoload,%object
gFsaeArm9iAutoload:
    .word 0x021870c0
    .word 0x00004780
    .word 0x021870c0
    .word 0x00034420
    .size gFsaeArm9iAutoload, .-gFsaeArm9iAutoload
