/* Public LTDMAIN smoke fixture: exercises CodeWarrior named sections. */

#pragma define_section LTDMAIN ".ltdmain" ".ltdmain.bss" abs32 RWX
#pragma section LTDMAIN begin

volatile unsigned long gLtdSmokeData = 0x4c54444dUL;
volatile unsigned long gLtdSmokeBss;

unsigned long FsaeLtdSmoke(unsigned long value);

unsigned long FsaeLtdSmoke(unsigned long value)
{
    gLtdSmokeBss = value ^ gLtdSmokeData;
    return gLtdSmokeBss;
}

#pragma section LTDMAIN end
