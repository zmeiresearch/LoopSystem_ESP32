/*==============================================================================
   LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <Arduino.h>
#include "config.h"
#include "logger.h"

#include <WString.h>
#include <pgmspace.h>

#include "webserver.h"


//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME                "Main"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE    0
#else
#define ARDUINO_RUNNING_CORE    1
#endif

#define   INIT_DO_HIGH(x)       { pinMode((x), OUTPUT); digitalWrite((x), HIGH); }
#define   INIT_DO_LOW(x)        { pinMode((x), OUTPUT); digitalWrite((x), LOW); }


//==============================================================================
//  Local types
//==============================================================================
// Function pointers for the pseudo-modules. Each module gets a separate task,
// in which 1) It's Init function is executed; 2) If init is OK, the Loop
// function is called  periodically until it returns != OK, after which, the
// task is deleted. Loop can also not return at all, but in this case, it needs
// to call delay/yield on it's own
typedef eStatus (*ModuleInitFn)(void * params);
typedef eStatus (*ModuleLoopFn)();

typedef struct _Module
{
    const char *    Name;
    ModuleInitFn    Init;
    ModuleLoopFn    Loop;
    uint32_t        Period;
    void *          Params;
    uint32_t        StackSize;
    uint8_t         Priority;   // Priority, with 3 (configMAX_PRIORITIES - 1)
                                // being the highest, and 0 being the lowest.
} Module;


//==============================================================================
//  Local function definitions
//==============================================================================


//==============================================================================
//  Local data
//==============================================================================
const Module Modules[] = {
    { "Logger",     LogInit,        LogLoop,    LOG_TASK_PERIOD,    NULL,   4096,   2 },
//    { "Blink",      NULL,           blinkLoop,  0,                  NULL,   4096,   1 },
    { "Webserver",  WebserverInit,  NULL,       5,                  NULL,   4096,   1 }
};


//==============================================================================
//  Local functions
//==============================================================================

static void setupHardware()
{
}

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

static eStatus startModules()
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

//==============================================================================
//  Exported functions
//==============================================================================

void setup()
{
    eStatus retVal = eOK;

    setupHardware();

    LogSetMinLevel(eLogInfo);

    retVal = startModules();
}

// All tasks already started during setup()
void loop()
{

}



