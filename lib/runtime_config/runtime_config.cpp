/*==============================================================================
   LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include "StreamUtils.h"

#include "config.h"
#include "runtime_config.h"
#include "logger.h"

//==============================================================================
//  Defines
//==============================================================================
#define CONFIG_UPDATE_COUNT_KEY "ConfigUpdateCount"
#define CONFIG_VERSION_KEY      "ConfigStructVersion"

#define CMP_NAME                "RuntimeConfig"

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
//  Exported data
//==============================================================================

class RuntimeConfig Config;

//==============================================================================
//  Exported functions
//==============================================================================

void RuntimeConfig::Init()
{
    Log(eLogInfo, CMP_NAME, "RuntimeConfig::Init");
    EEPROM.begin(512);
    EepromStream eepromStream(0, CONFIG_JSON_SIZE);
    deserializeJson(_json, eepromStream);

    uint32_t storedConfigVersion = _json[CONFIG_VERSION_KEY].as<uint32_t>();

    if ((uint32_t)CONFIG_STRUCT_VERSION != storedConfigVersion)
    {
        doUpgrade(storedConfigVersion, (uint32_t)CONFIG_STRUCT_VERSION);
    }
}

void RuntimeConfig::Finalize()
{
    Log(eLogInfo, CMP_NAME, "RuntimeConfig::Finalize");
}

void RuntimeConfig::DumpConfig(DynamicJsonDocument const & doc)
{
    String tmp;
    serializeJson(doc, tmp);
    Log(eLogInfo, CMP_NAME, "DumpConfig: %s", tmp.c_str());
}

void RuntimeConfig::commit()
{
    Log(eLogInfo, CMP_NAME, "RuntimeConfig::commit");
    _json[CONFIG_UPDATE_COUNT_KEY] = _json[CONFIG_UPDATE_COUNT_KEY].as<uint32_t>() + 1;
    EepromStream eepromStream(0, CONFIG_JSON_SIZE);
    serializeJson(_json, eepromStream);
    eepromStream.flush();
}

void RuntimeConfig::doUpgrade(uint32_t previousVersion, uint32_t currentVersion)
{
    Log(eLogInfo, CMP_NAME, "RuntimeConfig::doUpgrade: from %d to %d", previousVersion, currentVersion);

    for (uint32_t i = previousVersion; i<=currentVersion; i++)
    {
        switch (i)
        {
            case 1:
                WifiSSID.SetDefault();
                WifiPassword.SetDefault();
                break;
            case 2:
                // Added VPN parameters
                VpnEnabled.SetDefault();
                VpnLocalAddress.SetDefault();
                VpnLocalNetmask.SetDefault();
                VpnGateway.SetDefault();
                VpnClientPort.SetDefault();
                VpnClientPrivateKey.SetDefault();
                VpnPeerPort.SetDefault();
                VpnPeerAddress.SetDefault();
                VpnPeerPublicKey.SetDefault();
                break;
            default:
                break;
        }
    }


    _json[CONFIG_VERSION_KEY] = currentVersion;
    commit();
}