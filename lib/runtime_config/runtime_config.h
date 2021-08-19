/*==============================================================================
  LoopSystem ESP32
  
  DO NOT EDIT THIS FILE - runtime_config_data.h is probably what you're looking 
  for!
  ============================================================================*/

//==============================================================================
//  Multi-include guard
//==============================================================================
#ifndef INC_RUNTIME_CONFIG_H
#define INC_RUNTIME_CONFIG_H

//==============================================================================
//  Includes
//==============================================================================

#include "runtime_config_data.h"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  Exported types
//==============================================================================

class RuntimeConfig : public RuntimeConfigData
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
};

//==============================================================================
//  Exported data
//==============================================================================
extern class RuntimeConfig Config;

//==============================================================================
//  Exported functions
//==============================================================================

#endif // INC_RUNTIME_CONFIG_H
