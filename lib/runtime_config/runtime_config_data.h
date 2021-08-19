/*==============================================================================
  LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Multi-include guard
//==============================================================================
#ifndef INC_RUNTIME_CONFIG_DATA_H
#define INC_RUNTIME_CONFIG_DATA_H

//==============================================================================
//  Includes
//==============================================================================

#include "runtime_config_base.h"

//==============================================================================
//  Defines
//==============================================================================
#define CONFIG_JSON_SIZE        2048

//==============================================================================
//  Exported types
//==============================================================================

class RuntimeConfigData : public RuntimeConfigBase
{
    public: // Configuration variables
        DECLARE_CONFIG_VARIABLE(String,     WifiSSID);
        DECLARE_CONFIG_VARIABLE(String,     WifiPassword);

        DECLARE_CONFIG_VARIABLE(bool,       VpnEnabled);
        DECLARE_CONFIG_VARIABLE(String,     VpnLocalAddress);
        DECLARE_CONFIG_VARIABLE(String,     VpnLocalNetmask);
        DECLARE_CONFIG_VARIABLE(String,     VpnGateway);
        DECLARE_CONFIG_VARIABLE(uint16_t,   VpnClientPort);
        DECLARE_CONFIG_VARIABLE(String,     VpnClientPrivateKey);
        DECLARE_CONFIG_VARIABLE(uint16_t,   VpnPeerPort);
        DECLARE_CONFIG_VARIABLE(String,     VpnPeerAddress);
        DECLARE_CONFIG_VARIABLE(String,     VpnPeerPublicKey);
#define CONFIG_STRUCT_VERSION   2   // Here so that it's easier to remember it on every bump!
};


//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================

#endif // INC_RUNTIME_CONFIG_DATA_H
