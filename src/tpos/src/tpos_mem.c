

#include "tpos_mem.h"
#include "tpos.h"
#include "tpos_service.h"

extern void tpos_clrTaskWdt(void);

// clear task dog..
void   ClrTaskWdt(void)
{
    tpos_clrTaskWdt();
}

// time delay, unit 1ms..
void DelayNmSec(INT32U delay)
{
    #ifndef __SOFT_SIMULATOR__
    tpos_sleep(delay);
    #else
     Sleep(delay);
    #endif
}


