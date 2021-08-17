/*==============================================================================
   LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"

#include "SPIFFS.h"
#include "WiFi.h"

#include "system.h"
#include "config.h"
#include "runtime_config.h"
#include "webserver.h"

#include "logger.h"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME        "System"

//==============================================================================
//  Local types
//==============================================================================


//==============================================================================
//  Local function definitions
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================

//==============================================================================
//  Local functions
//==============================================================================


//==============================================================================
//  Exported functions
//==============================================================================
void SystemRestart()
{
    Log(eLogWarn, CMP_NAME, "SystemRestart called!");
    WebserverCloseSockets();
    Config.Finalize();
    SPIFFS.end();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ESP.restart();
}

void SystemGetMemoryInfo(String & memInfo)
{
    memInfo = String("Free: ");
    memInfo += String(esp_get_free_heap_size()/1024);
    memInfo += String("kB, Min Free: ");
    memInfo += String(esp_get_minimum_free_heap_size()/1024);
    memInfo += String("kB");
}

void SystemGetWifiStatus(String & wifiStatus)
{
    switch(WiFi.status())
    {
        case WL_IDLE_STATUS:
            wifiStatus = String("Idle");
            break;
        case WL_NO_SSID_AVAIL:
            wifiStatus = String("No SSID available");
            break;
        case WL_SCAN_COMPLETED:
            wifiStatus = String("Scan completed");
            break;
        case WL_CONNECTED:
            wifiStatus = String("Connected");
            break;
        case WL_CONNECT_FAILED:
            wifiStatus = String("Failed");
            break;
        case WL_CONNECTION_LOST:
            wifiStatus = String("Connection lost");
            break;
        case WL_DISCONNECTED:
            wifiStatus = String("Disconnected");
            break;
        default:
            wifiStatus = String("Unknown");
            break;
    }


}
