/*==============================================================================
   LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <Arduino.h>
#include <WString.h>
#include <pgmspace.h>

#include "config.h"
#include "logger.h"

#include "values.h"

#include "module_host.h"

#include "webserver.h"
#include <control_protocol.h>
#include <control_serial.h>
#include "system_manager.h"


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

//==============================================================================
//  Local function definitions
//==============================================================================
static void     setupHardware();
static eStatus  keepaliveLoop();

//==============================================================================
//  Local data
//==============================================================================
const Module modules[] = {
    { "Logger",         LogInit,            LogLoop,                LOG_TASK_PERIOD,    NULL,   8192,   2 },
    { "SystemManager",  SystemManagerInit,  SystemManagerTask,      100,                NULL,   4096,   1 },
    { "Keepalive",      NULL,               keepaliveLoop,          10000,              NULL,   4096,   0 },
    { "SerialReceive",  ControlSerialInit,  ControlSerialReceive,   5,                  NULL,   4096,   2 },
    { "SerialTransmit", ControlSerialInit,  ControlSerialTransmit,  5,                  NULL,   4096,   2 },
    { "SerialRefresh",  NULL,               ControlRefreshTask,     2000,               NULL,   4096,   2 },
    //{ "Webserver",      WebserverInit,      NULL,                   0,                  NULL,   4096,   1 }
};

//==============================================================================
//  Local functions
//==============================================================================
static void  setupHardware()
{
    // Nothing to do
}

static eStatus keepaliveLoop()
{
    Log(eLogWarn, CMP_NAME, "Still alive!");
    //DumpStatus(&gStatus);
    return eOK;
}

//==============================================================================
//  Exported functions
//==============================================================================

void setup()
{
    setupHardware();

    StartModules(&modules[0], ARRAY_SIZE(modules));

    LogSetMinLevel(eLogDebug);

}

// All tasks already started during setup()
void loop()
{

}



