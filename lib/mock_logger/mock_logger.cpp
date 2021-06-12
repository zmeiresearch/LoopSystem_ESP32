/*==============================================================================
   LoopSystem ESP32

  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <stdio.h>
#include <stdarg.h>
#include <unity.h>

#include "mock_logger.h"

//==============================================================================
//  Defines
//==============================================================================
#define LOG_BUFFER_SIZE     (size_t)1024

//==============================================================================
//  Local types
//==============================================================================


//==============================================================================
//  Local data
//==============================================================================
static char         logBuffer[LOG_BUFFER_SIZE] = { 0 };
static int          writePtr = 0;

//==============================================================================
//  Local functions
//==============================================================================
static size_t getFree() 
{
    return (LOG_BUFFER_SIZE - writePtr);
}


static const char getLevelChar(const eLogLevel level)
{
    const char chars[eLogLevelCount+1] = {
        'D',    // []ebug
        'I',    // []nfo
        'W',    // []arning
        'E',    // []rror
        'C',    // []itical
        'X'     // Unknown
    };

    if ((level >= 0) && (level < eLogLevelCount))
    {
        return chars[level];
    }
    else 
    {
        return chars[eLogLevelCount];
    }
}

//==============================================================================
//  Exported functions
//==============================================================================
void Log(const eLogLevel level, const char * const component, ...)
{
    int i;
    writePtr = 0;

    // first print the time
    i = snprintf((char *)&logBuffer[writePtr], getFree(),
            "%c|%s|", getLevelChar(level), component);

    if (i < 0)
    {
        exit(-1);
    } 
    else 
    {
        writePtr += i;
        // if that went well, append the actual message as well
        va_list args;
        va_start(args, component);
        const char * fmt = va_arg(args, char *);
        
        int i = vsnprintf((char *)&logBuffer[writePtr], getFree(), fmt, args);
        va_end(args);
    
        TEST_MESSAGE(logBuffer);
    }
}

