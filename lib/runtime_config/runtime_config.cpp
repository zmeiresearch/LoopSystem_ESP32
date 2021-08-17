/*==============================================================================
   LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================


#include "config.h"

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

//==============================================================================
//  Local functions
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================

//==============================================================================
//  Exported data
//==============================================================================
class RuntimeConfig RuntimeConfigInst;

void RuntimeConfig::Init()
{
    // IVA: TODO: Handle updates!
    prefs.begin(CONFIG_NAMESPACE, false); 
}

void RuntimeConfig::Finalize()
{
    prefs.end();
}



/*uint32_t     ConfigVersion()
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
    return preferences.getString("WifiSSID", WIFI_DEFAULT_SSID);
}

String       ConfigWifiPassword()
{
    return preferences.getString("WifiPassword", WIFI_DEFAULT_PASSWORD);
}



bool  ConfigWriteWifiSSID(const String ssid)
{
    Log(eLogInfo, CMP_NAME, "ConfigWriteWifiSSID: %s", ssid.c_str());
    if (0 != preferences.putString("WifiSSID", ssid.c_str()))
    {
        incrementUpdateCount();
        return true;
    }
    Log(eLogError, CMP_NAME "ConfigWriteWifiSSID: error setting SSID!");
    return false;
}

bool        ConfigWriteWifiPassword(const String password)
{
    Log(eLogInfo, CMP_NAME, "ConfigWriteWifiPassword: %s", password.c_str());
    if (0 != preferences.putString("WifiPassword", password.c_str()))
    {
        incrementUpdateCount();
        return true;
    }
    Log(eLogError, CMP_NAME "ConfigWriteWifiPassword: error setting Password!");
    return false;
}*/