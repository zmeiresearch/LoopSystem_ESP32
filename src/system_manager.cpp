/*==============================================================================
   LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include "system_manager.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include "SPIFFS.h"

#include "config.h"
#include "logger.h"

#include "wifi_manager.h"
#include "runtime_config.h"
#include "system.h"
#include "vpn_manager.h"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME        "SysMgr"

//==============================================================================
//  Local types
//==============================================================================

//==============================================================================
//  Local function definitions
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================
static bool         spiffsInitialized = false;
static uint32_t     spiffsRetryCount = 0;

//==============================================================================
//  Local functions
//==============================================================================
static void tryInitSpiffs()
{
    if (!spiffsInitialized && spiffsRetryCount < SPIFFS_RETRY_LIMIT)
    {
        if (SPIFFS.begin())
        {
            Log(eLogInfo, CMP_NAME, "tryInitSpiffs: SPIFFS initialization success");
            spiffsInitialized = true;
        }
        else
        {
            Log(eLogCrit, CMP_NAME, "tryInitSpiffs: SPIFFS intialization failure!");
            spiffsRetryCount++;
        }
    }
}

//==============================================================================
//  Exported functions
//==============================================================================
eStatus SystemManagerInit(void * params)
{
    Log(eLogInfo, CMP_NAME, "SystemManagerInit: Build ID:%s", SystemGetBuildId());

    Config.Init();
    VpnManagerInit(NULL);

    return eOK;
}

eStatus SystemManagerTask()
{
    tryInitSpiffs();
    WifiManagerDoWork();

    // always loop
    return eOK;
}
