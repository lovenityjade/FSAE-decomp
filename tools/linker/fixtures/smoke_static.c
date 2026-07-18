/* Public linker smoke fixture: no SDK declarations or game code. */

volatile unsigned long gLinkerSmokeValue = 0x46534145UL;

void TwlMain(void);
void _start(void);
void _start_AutoloadDoneCallback(void);
unsigned long FsaeLtdSmoke(unsigned long value);

void *const _start_ModuleParams[9] = { 0 };
void *const _start_LtdModuleParams[6] = { 0 };

void TwlMain(void)
{
    gLinkerSmokeValue = FsaeLtdSmoke(gLinkerSmokeValue ^ 0x10203040UL);
}

void _start(void)
{
    TwlMain();
}

void _start_AutoloadDoneCallback(void)
{
}
