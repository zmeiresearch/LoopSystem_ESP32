/*==============================================================================
  LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Multi-include guard
//==============================================================================
#ifndef INC_MODULE_HOST_H
#define INC_MODULE_HOST_H

//==============================================================================
//  Includes
//==============================================================================
#include "globals.h"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  Exported types
//==============================================================================
// Function pointers for the pseudo-modules. Each module gets a separate task,
// in which 1) It's Init function is executed; 2) If init is OK, the Loop
// function is called  periodically until it returns != OK, after which, the
// task is deleted. Loop can also not return at all, but in this case, it needs
// to call delay/yield on it's own
typedef eStatus (*ModuleInitFn)(void * params);
typedef eStatus (*ModuleLoopFn)();

typedef struct _Module
{
    const char *    Name;
    ModuleInitFn    Init;
    ModuleLoopFn    Loop;
    uint32_t        Period;
    void *          Params;
    uint32_t        StackSize;
    uint8_t         Priority;   // Priority, with 3 (configMAX_PRIORITIES - 1)
                                // being the highest, and 0 being the lowest.
} Module;

//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================
eStatus StartModules();

#endif // INC_MODULE_HOST_H
