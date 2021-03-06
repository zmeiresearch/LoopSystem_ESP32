/*==============================================================================
  LoopSystem ESP32

  DO NOT EDIT THIS FILE - runtime_config_data.h is probably what you're looking 
  for!
  ============================================================================*/

//==============================================================================
//  Multi-include guard
//==============================================================================
#ifndef INC_RUNTIME_CONFIG_BASE_H
#define INC_RUNTIME_CONFIG_BASE_H

//==============================================================================
//  Includes
//==============================================================================

#include "WString.h"
#include "ArduinoJson.h"

#include "globals.h"

//==============================================================================
//  Defines
//==============================================================================

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
        T operator()() { return Get(); };
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

//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================

#endif // INC_RUNTIME_CONFIG_BASE_H
