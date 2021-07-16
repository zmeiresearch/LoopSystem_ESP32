/*==============================================================================
   LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <Arduino.h>

#include "module_host.h"

#include "logger.h"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME                "ModuleHost"

//==============================================================================
//  Local types
//==============================================================================


//==============================================================================
//  Local function definitions
//==============================================================================
static void     moduleHost(void * param);

//==============================================================================
//  Local data
//==============================================================================

//==============================================================================
//  Local functions
//==============================================================================
static void moduleHost(void * param)
{
    eStatus retVal = eOK;
    const Module * const module = (const Module * const)param;

    if (NULL != module->Init)
    {
        retVal = module->Init(module->Params);
    }

    if (NULL != module->Loop)
    {
        while (eOK == retVal)
        {
            retVal = module->Loop();
            if (0 != module->Period)
            {
                vTaskDelay(module->Period / portTICK_PERIOD_MS);
            }
        }
    }
    // FreeRTOS will reclaim resources
    vTaskDelete(NULL);
}

//==============================================================================
//  Exported functions
//==============================================================================
eStatus StartModules()
{
    eStatus retVal = eOK;
    TaskHandle_t    taskHandle;

    for (int i = 0; i < ARRAY_SIZE(Modules); i++)
    {
        xTaskCreate(moduleHost,
                Modules[i].Name,
                Modules[i].StackSize,   // Stack size
                (void*)&Modules[i],
                Modules[i].Priority,
                &taskHandle);
        if (taskHandle == NULL)
        {
            Log(eLogCrit, CMP_NAME, "startModules: Error creating task for %s", Modules[i].Name);
            // Logger may not be working at that time!
            Serial.begin(115200);
            Serial.print("startModules: Error creating task for");
            Serial.println(Modules[i].Name);

            retVal = eFAIL;
        }
    }
    return retVal;
}
