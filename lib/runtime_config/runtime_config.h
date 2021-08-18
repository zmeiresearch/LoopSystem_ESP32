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
#include "ArduinoJson.h"

#include "globals.h"

//==============================================================================
//  Defines
//==============================================================================
#define CONFIG_JSON_SIZE        2048


//==============================================================================
//  Exported types
//==============================================================================

class RuntimeConfigBase {
    public:
        virtual JsonDocument& json() = 0;
        virtual void commit() = 0;
};

template<typename T> 
class RuntimeConfigVariable 
{
    private:
        T                   _default;
        RuntimeConfigBase&  _config;
        String              _key;
    public:
        RuntimeConfigVariable(const char * key, T defaultVal, RuntimeConfigBase& config): 
            _default(defaultVal), _config(config), _key(key) {};
        T Get() { 
            if (_config.json()[_key].isNull()) {
                return _default;
            } else {
                return _config.json()[_key].as<T>();
            }
        };
        void Set(T val) {
            _config.json()[_key] = val;
            _config.commit();
        };
        void SetDefault() { _config.json()[_key] = _default;};
};

#define DECLARE_CONFIG_VARIABLE(type, name)  RuntimeConfigVariable <type> name = RuntimeConfigVariable <type>(#name, CONFIG_DEFAULT_##name, *this)

class RuntimeConfig : RuntimeConfigBase
{
    private:
        DynamicJsonDocument    _json = DynamicJsonDocument(CONFIG_JSON_SIZE);

    private:
        void doUpgrade(uint32_t previousVersion, uint32_t currentVersion);

    public: // RuntimeConfigBase interface
        JsonDocument& json() { return _json; };
        void commit();

    public:
        RuntimeConfig(){};
        void Init();
        void Finalize();
        void DumpConfig(DynamicJsonDocument const & doc);
        void DumpConfig() { DumpConfig(_json);};
    
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
extern class RuntimeConfig Config;

//==============================================================================
//  Exported functions
//==============================================================================

#endif // INC_RUNTIME_CONFIG_H
