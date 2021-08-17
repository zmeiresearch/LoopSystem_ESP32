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
#define CONFIG_STRUCT_VERSION   1
#define CONFIG_JSON_SIZE        2048
#define CONFIG_UPDATE_COUNT_KEY "ConfigUpdateCount"

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
        bool Set(T val) {
            _config.json()[_key] = val;
            _config.commit();
            return true;
        };
};

#define DECLARE_CONFIG_VARIABLE(type, name)  RuntimeConfigVariable <type> name = RuntimeConfigVariable <type>(#name, CONFIG_DEFAULT_##name, *this)

class RuntimeConfig : RuntimeConfigBase
{
    private:
        //Preferences             _prefs;
        DynamicJsonDocument*    _json;

    private:
        void doUpgrade(uint32_t previousVersion, uint32_t currentVersion);

    public: // RuntimeConfigBase interface
        JsonDocument& json() { return *_json; };
        void commit();

    public:
        RuntimeConfig() {_json = new DynamicJsonDocument(CONFIG_JSON_SIZE); };
        ~RuntimeConfig() { delete _json; };
        void Init();
        void Finalize();
    
    public: // Configuration variables
        DECLARE_CONFIG_VARIABLE(String, WifiSSID);
        DECLARE_CONFIG_VARIABLE(String, WifiPassword);
        DECLARE_CONFIG_VARIABLE(bool,   VpnEnabled);
};


//==============================================================================
//  Exported data
//==============================================================================
extern class RuntimeConfig Config;

//==============================================================================
//  Exported functions
//==============================================================================

#endif // INC_RUNTIME_CONFIG_H
