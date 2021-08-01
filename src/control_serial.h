/*==============================================================================
   LoopSystem ESP32
   
  ============================================================================*/


//==============================================================================
//  Multi-include guard
//==============================================================================
#ifndef INC_CONTROL_SERIAL_H
#define INC_CONTROL_SERIAL_H

//==============================================================================
//  Includes
//==============================================================================

#include <Arduino.h>
#include "globals.h"
#include "values.h"

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
eStatus     ControlSerialInit(void * param);
eStatus     ControlSerialReceive();
eStatus     ControlSerialTransmit();
eStatus     ControlRefreshTask();
void        SendPacketGlobalValuesAscii(const GlobalValues * const values);
void        SendPacketModeValuesAscii(const ModeValues * const modeValues);
void        SendPacketRequestModeValues(Modes mode);
void        SendPacketRequestGlobalValues();

#endif // INC_CONTROL_SERIAL_H
