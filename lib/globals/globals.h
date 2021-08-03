/*==============================================================================
   LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Multi-include guard
//==============================================================================
#ifndef INC_GLOBALS_H
#define INC_GLOBALS_H

//==============================================================================
//  Multi-include guard
//==============================================================================
#include <stdint.h>


//==============================================================================
//  Defines/Macros
//==============================================================================
#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))

#define DIV_ROUND_CLOSEST(n, d) ((((n) < 0) ^ ((d) < 0)) ? (((n) - (d)/2)/(d)) : (((n) + (d)/2)/(d)))

// GCC-ism below. Make it a proper function if compilation fails 
#define WAIT_FOR(cond, timeout) ({                                          \
        uint32_t startTime = PortGetTime(); bool condState = (cond);        \
        while (!condState && ((PortGetTime() - startTime) < (timeout)))     \
        { PortSleepMs(5); condState = (cond);}                              \
        (condState) ? eOK : eTIMEOUT;                                       \
    })

//==============================================================================
// Peripherals
//==============================================================================


//==============================================================================
//  Exported types
//==============================================================================
typedef enum _eStatus
{
    eOK,
    eDONE,
    eFAIL,
    eINVALIDARG,
    eUNSUPPORTED,
    eBUSY,
    eNOTINITIALIZED,
    eOUTOFMEMORY,
    eTIMEOUT,
    eOVERFLOW,
    eStatusCount
} eStatus;

//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================

#endif // INC_GLOBALS_H
