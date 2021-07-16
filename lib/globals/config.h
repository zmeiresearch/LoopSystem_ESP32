/*==============================================================================
  LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Multi-include guard
//==============================================================================
#ifndef INC_CONFIG_H
#define INC_CONFIG_H

//==============================================================================
//  Includes
//==============================================================================
#include "config_local.h"

//==============================================================================
//  Defines
//==============================================================================

// Board will first try connecting to the WiFi network that is in run-time
// config. If that fails for some time, a hotspot with default credentials will 
// be started to allow configuration.
#define     WIFI_AP_SSID          "loopsystem-esp32-config"
#define     WIFI_AP_PASSWORD      "looploop"

// Name the board will advertise once connected to a network
#define     WIFI_DEVICE_NAME      "loopsystem-esp32"

// Log configuration
#define     LOG_TASK_PERIOD         5   //  in ms
#define     LOG_LEVEL_DEFAULT       eLogInfo
#define     LOG_BUFFER_SIZE         8192
#define     LOG_SOCKET_PORT         81
#define     LOG_SOCKET_PATH         "/log"

// Control UART
#define     CONTROL_SERIAL_PORT     2
#define     CONTROL_SERIAL_BAUD     19200
#define     CONTROL_SERIAL_FORMAT   SERIAL_8N2
#define     CONTROL_SERIAL_RX_PIN   16
#define     CONTROL_SERIAL_TX_PIN   17

//==============================================================================
//  Exported types
//==============================================================================

//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================


#endif // INC_CONFIG_H
