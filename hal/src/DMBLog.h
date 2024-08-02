#ifndef DMBLOG_H
#define DMBLOG_H
#include <stdio.h>
#include <stdarg.h>

extern void initLogging( void );
extern void dlt_debug(char* in_log);
extern void vcrm_debug(char* in_log);
extern void dlt_log_func(char* func, char* str);
#define HS_LOG( logText )    dlt_debug(logText)
#define LS_LOG( logText )    dlt_debug(logText)
#define VCRM_LOG( logText )    vcrm_debug(logText)
#define BOOT_LOG(func, str)  dlt_log_func(func, str);

void LBPrintf(const char *fmt, ...);
#define LB_PRINTF(fmt, ...) LBPrintf((fmt), ##__VA_ARGS__)

#endif // DMBLOG_H
