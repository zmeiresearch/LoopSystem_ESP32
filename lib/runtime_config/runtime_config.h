/*==============================================================================
  LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Multi-include guard
//==============================================================================
#ifndef INC_RUNTIME_CONFIG_H
#define INC_RUNTIME_CONFIG_H

//==============================================================================
//  Includes
//==============================================================================
#include "WString.h"
#include <sstream>
#include <string>
#include <iostream>
#include <Preferences.h>

#include "globals.h"
#include "logger.h"

//==============================================================================
//  Defines
//==============================================================================
#define CONFIG_STRUCT_VERSION   1
#define CONFIG_NAMESPACE        "loop32"


//==============================================================================
//  Exported types
//==============================================================================
template <typename T> T stringTo(const std::string &inVal) {
    std::istringstream ss(inVal);
    T outVal;
    ss >> outVal;
    return outVal;
}

template < typename T > std::string toString( const T& n )
{
    std::ostringstream stm ;
    stm << n ;
    return stm.str() ;
}

template<typename T>
class ConfigValue {
    private:
        std::string     _name;
        std::string     _defaultValue;
        Preferences&    _preferences;
    private:
        void incrementUpdateCount() { _preferences.putUInt("UpdateCount", _preferences.getUInt("UpdateCount", 0) + 1); };
    public:
        ConfigValue(std::string name, T defaultValue, Preferences & preferences): 
            _name(name), _defaultValue(toString(defaultValue)), _preferences(preferences) 
            {};
        T Get() {
            return stringTo<T>(toString(_preferences.getString(_name.c_str(), String(_defaultValue.c_str())).c_str()));
        };
        bool Set(T val) {
            Log(eLogInfo, "RuntimeConfig", "ConfigValue: %s: Set: %s", _name.c_str(), std::string(val).c_str());
            if (0 != _preferences.putString(_name.c_str(), std::string(val).c_str())) {
                incrementUpdateCount();
                return true;
            }
            Log(eLogError, "RuntimeConfig", "ConfigValue: Error setting %s!", _name.c_str());
            return false;
        };
        bool IsDefault();
};

#define DECLARE_CONFIG_VARIABLE(type, val, prefs)  ConfigValue <type> val = ConfigValue <type>(#val, CONFIG_DEFAULT_##val, prefs)

class RuntimeConfig {
    private:
        Preferences prefs;

    public:
        RuntimeConfig(): prefs(Preferences()) {};
        void Init();
        void Finalize();
    public:
        DECLARE_CONFIG_VARIABLE(std::string, WifiSSID, prefs);
        DECLARE_CONFIG_VARIABLE(std::string, WifiPassword, prefs);
        
        DECLARE_CONFIG_VARIABLE(bool,        VpnEnabled, prefs);
        DECLARE_CONFIG_VARIABLE(std::string, VpnLocalAddress, prefs);
        DECLARE_CONFIG_VARIABLE(std::string, VpnLocalNetmask, prefs);
        DECLARE_CONFIG_VARIABLE(std::string, VpnGateway, prefs);
        DECLARE_CONFIG_VARIABLE(int,         VpnClientPort, prefs);
        DECLARE_CONFIG_VARIABLE(std::string, VpnClientPrivateKey, prefs);
        DECLARE_CONFIG_VARIABLE(int,         VpnPeerPort, prefs);
        DECLARE_CONFIG_VARIABLE(std::string, VpnPeerAddress, prefs);
        DECLARE_CONFIG_VARIABLE(std::string, VpnPeerPublicKey, prefs);
        
};


//==============================================================================
//  Exported data
//==============================================================================

extern class RuntimeConfig RuntimeConfigInst;

//==============================================================================
//  Exported functions
//==============================================================================

#endif // INC_RUNTIME_CONFIG_H
