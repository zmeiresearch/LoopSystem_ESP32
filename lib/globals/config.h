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
#define     WIFI_AP_SSID            "loopsystem-esp32-config"
#define     WIFI_AP_PASSWORD        "looploop"
#define     WIFI_AP_ADDRESS         "192.168.99.1"
#define     WIFI_AP_NETMASK         "255.255.255.0"
#define     WIFI_AP_CHANNEL         9

// Name the board will advertise once connected to a network
#define     WIFI_DEVICE_NAME        "loopsystem-esp32"

// Time in milliseconds to wait for WiFi to connect in client mode
#define     WIFI_DEFAULT_CONNECT_TIME   30

// Number of retries to mount SPIFFS. Most of the time if it fails once, it will 
// fail forever
#define     SPIFFS_RETRY_LIMIT      5

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
