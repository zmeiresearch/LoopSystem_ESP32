/*==============================================================================
   LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include "system.h"

#include "SPIFFS.h"

#include "config.h"

#include "runtime_config.h"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME        "System"

//==============================================================================
//  Local types
//==============================================================================

//==============================================================================
//  Local function definitions
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================


//==============================================================================
//  Local functions
//==============================================================================


//==============================================================================
//  Exported functions
//==============================================================================
void SystemRestart()
{
    ConfigFinalize();
    SPIFFS.end();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ESP.restart();
}