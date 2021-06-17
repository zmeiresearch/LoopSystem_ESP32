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

//==============================================================================
//  Defines
//==============================================================================

// In demo mode, a specific task is started that periodically changes mode and
// global values
//#define DEMO_MODE   1


// Wifi configuration
#define WIFI_SSID                   "gr5-c3_leaf"
#define WIFI_PASSWORD               "AASSDDFFqwer11223344!!"
#define WIFI_DEVICE_NAME            "loopsystem-esp32"

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


