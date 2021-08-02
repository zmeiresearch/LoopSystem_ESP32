/*==============================================================================
  LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Multi-include guard
//==============================================================================
#ifndef INC_SYSTEM_H
#define INC_SYSTEM_H

//==============================================================================
//  Includes
//==============================================================================
#include "globals.h"
#include "WString.h"

//==============================================================================
//  Defines
//==============================================================================
#if !defined(BUILD_REV)
#define BUILD_REV               "Unknown"
#endif  // BUILD_REV

//==============================================================================
//  Function-like macros
//==============================================================================
#define SystemGetBuildId()      (BUILD_REV)
#define SystemGetBuildTime()    (__TIME__ " ; " __DATE__ )

#define SystemGetTimeMs()       (xTaskGetTickCount() * portTICK_PERIOD_MS)
#define SystemSleepMs(x)        (vTaskDelay((x)/portTICK_PERIOD_MS))
#define SystemElapsedTimeMs(x)  (SystemGetTimeMs() - (x))

//==============================================================================
//  Exported types
//==============================================================================

//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================
void    SystemRestart();

#endif // INC_SYSTEM_H

