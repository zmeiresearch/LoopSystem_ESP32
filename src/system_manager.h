/*==============================================================================
  LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Multi-include guard
//==============================================================================
#ifndef INC_SYSTEM_MANAGER_H
#define INC_SYSTEM_MANAGER_H

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

//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================
eStatus   SystemManagerInit(void * params);
eStatus   SystemManagerTask();

#endif // INC_SYSTEM_MANAGER_H


