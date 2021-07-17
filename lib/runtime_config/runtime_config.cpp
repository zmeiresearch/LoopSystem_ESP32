/*==============================================================================
   LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <Preferences.h>

#include "runtime_config.h"

#include "logger.h"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME    "RuntimeConfig"

//==============================================================================
//  Local types
//==============================================================================

//==============================================================================
//  Local function definitions
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================
static Preferences preferences;

//==============================================================================
//  Local functions
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================

void         ConfigInit()
{
    preferences.begin(CONFIG_NAMESPACE, false); 
}

void         Finalize()
{
    preferences.end();
}

bool         ConfigIsDefault()
{
    if (ConfigUpdateCount > 0) return false;
    return true;
}

uint32_t     ConfigVersion()
{
    return CONFIG_STRUCT_VERSION;
}

uint32_t     ConfigUpdateCount()
{
    return preferences.getUInt("UpdateCount", 0);
}

// Data starts here
String       ConfigWifiSSID()
{
    return preferences.getString("WifiSSID", "kyp");
}

String       ConfigWifiPassword()
{
    return preferences.getString("WifiPassword", "kyp");
}

void incrementUpdateCount()
{
    uint32_t updateCount = preferences.getUInt("UpdateCount", 0);
    updateCount++;
    preferences.putUInt("UpdateCount", updateCount);
}

bool        ConfigWriteWifiSSID(const String ssid)
{
    if (0 != preferences.putString("WifiSSID", ssid))
    {
        incrementUpdateCount();
        return true;
    }
    return false;
}

bool        ConfigWriteWifiPassword(const String password)
{
    if (0 != preferences.putString("WifiPassword", password))
    {
        incrementUpdateCount();
        return true;
    }
    return false;
}