#include "DMBLog.h"
#include <dlt/dlt.h>


DLT_DECLARE_CONTEXT(ServiceDMBContext);
DLT_DECLARE_CONTEXT(mDltContext_log);

void initLogging( void )
{
    DLT_REGISTER_APP("SDRS", "TC SDR");
    DLT_REGISTER_CONTEXT(ServiceDMBContext, "SDRS", "TC SDR  Service for Logging.");
    DLT_REGISTER_CONTEXT(mDltContext_log, "SDRC", "TC SDR Service Standard Log Context");
}

#if 0
usr/include/dlt/dlt_types.h
/**
 * Definitions of DLT log level
 */
typedef enum
{
    DLT_LOG_DEFAULT =             -1,   /**< Default log level */
    DLT_LOG_OFF     =           0x00,   /**< Log level off */
    DLT_LOG_FATAL   =           0x01,   /**< fatal system error */
    DLT_LOG_ERROR   =           0x02,   /**< error with impact to correct functionality */
    DLT_LOG_WARN    =           0x03,   /**< warning, correct behaviour could not be ensured */
    DLT_LOG_INFO    =           0x04,   /**< informational */
    DLT_LOG_DEBUG   =           0x05,   /**< debug  */
    DLT_LOG_VERBOSE =           0x06,   /**< highest grade of information */
    DLT_LOG_MAX                         /**< maximum value, used for range check */
} DltLogLevelType;
#endif

void dlt_debug(char* in_log)
{
    DLT_LOG(ServiceDMBContext, DLT_LOG_WARN/*DLT_LOG_DEFAULT*/, DLT_STRING(in_log));
}
void vcrm_debug(char* in_log)
{
    DLT_LOG(mDltContext_log, DLT_LOG_WARN/*DLT_LOG_DEFAULT*/, DLT_STRING(in_log));
}
#ifdef __BOOT_TIME__
void dlt_log_func(char* func, char* str)
{
    DLT_LOG(ServiceDMBContext, DLT_LOG_DEBUG, DLT_CSTRING("test"), DLT_CSTRING("[ServiceDMB] "), DLT_CSTRING(func), DLT_CSTRING(" "), DLT_CSTRING(str));
}
#endif

void LBPrintf(const char *fmt, ...) 
{
    char buf[2048] = { 0, };
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    // printf("%s", buf);
    dlt_debug(buf);
}
