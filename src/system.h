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
#define SYSTEM_CPU_MONITOR_PERIOD (1000)

#if !defined(BUILD_REV)
#define BUILD_REV                 "Unknown"
#endif  // BUILD_REV

//==============================================================================
//  Function-like macros
//==============================================================================
#define SystemGetBuildId()      (BUILD_REV)
#define SystemGetBuildTime()    (__TIME__ ", " __DATE__ )

#define SystemGetTimeMs()       (xTaskGetTickCount() * portTICK_PERIOD_MS)
#define SystemGetTimeuS()       (esp_timer_get_time())
// No need to round in the most common case of 1 tick/ms
#define SystemSleepMs(x)        (vTaskDelay((x)/portTICK_PERIOD_MS))
#define SystemElapsedTimeMs(x)  (SystemGetTimeMs() - (x))
#define SystemElapsedTimeuS(x)  (SystemGetTimeuS() - (x))

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
void    SystemGetMemoryInfo(String & memInfo);
void    SystemGetWifiStatus(String & wifiStatus);

#endif // INC_SYSTEM_H

