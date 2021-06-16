/*==============================================================================
   LoopSystem ESP32
   
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <string>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"

#include "config.h"
#include "globals.h"
#include "logger.h"
#include "values.h"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME            "Webserver"

//==============================================================================
//  Local types
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================
static AsyncWebServer server(80);

//==============================================================================
//  Local function definitions
//==============================================================================
static void notFoundResponse(AsyncWebServerRequest *request);
static void firmwareUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
static void spiffsUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);


//==============================================================================
//  Local functions
//==============================================================================

// 404 default handler
static void notFoundResponse(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

// IVA: TODO: Merge firmwareUpload and spiffsUpload - the two differ only in U_FLASH/U_SPIFFS
// Firmware upload
static void firmwareUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    // handle upload and update
    if (!index)
    {
        Log(eLogInfo, CMP_NAME, "firmwareUpload: Update %s", filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH))
        { 
            Log(eLogCrit, CMP_NAME, "firmwareUpload: Error updating!");
            //start with max available size
            Update.printError(Serial);
        }
    }

    /* flashing firmware to ESP*/
    if (len)
    {
        Update.write(data, len);
    }

    if (final)
    {
        if (Update.end(true))
        { 
            //true to set the size to the current progress
            Log(eLogInfo, CMP_NAME, "firmwareUpload: Update Success: %ub written\nRebooting...", index+len);
        }
        else
        {
            Log(eLogCrit, CMP_NAME, "formwareUpload: Error updating!");
            Update.printError(Serial);
        }
    }
}

// SPIFFS upload
static void spiffsUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    if (!index)
    {
        Log(eLogInfo, CMP_NAME, "spiffsUpload: Update %s", filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS))
        { 
            Log(eLogCrit, CMP_NAME, "spiffsUpload: Error updating!");
            Update.printError(Serial);
        }
    }

    if (len)
    {
        Update.write(data, len);
    }

    if (final)
    {
        if (Update.end(true))
        {    
            Log(eLogInfo, CMP_NAME, "spiffsUpload: Update Success: %ub written\nRebooting...", index+len);
        }
        else
        {
            Log(eLogCrit, CMP_NAME, "spiffsUpload: Error updating!");
            Update.printError(Serial);
        }
    }
}

static void getModeValues(AsyncWebServerRequest *request)
{
    const int paramCount = request->params();

    Log(eLogWarn, CMP_NAME, "getModeValues: got %d params", paramCount);

    for (int i = 0; i < paramCount; i++)
    {
        AsyncWebParameter* p = request->getParam(i);
        Log(eLogInfo, CMP_NAME, "getModeValues: id: %s:%s", p->name().c_str(), p->value().c_str());

        if (0 == p->name().compareTo("mode"))
        {
            int mode = -1;

            if (0 == p->value().compareTo("novice"))
            {
                mode = 0;
            } 
            else if (0 == p->value().compareTo("expert"))
            {
                mode = 1;
            } 
            else if (0 == p->value().compareTo("advanced"))
            {
                mode = 2;
            }
            else if (0 == p->value().compareTo("master"))
            {
                mode = 3;
            }
            
            if (-1 == mode)
            {
                Log(eLogWarn, CMP_NAME, "getModeValues: Unknown mode!");
            }
            else
            {
                AsyncResponseStream *response = request->beginResponseStream("application/json");
                DynamicJsonDocument json(1024);
                json["end"] = String(gModeValues[mode].end);
                json["acc"] = String(gModeValues[mode].acc);
                json["dec"] = String(gModeValues[mode].dec);
                json["turn"] = String(gModeValues[mode].turn);
                serializeJson(json, *response);
                request->send(response);
            }
        }
    }
}

static void getGlobalValues(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument json(1024);
    json["home"] = String(gGlobalValues.home);
    json["end"] = String(gGlobalValues.end);
    json["turn1"] = String(gGlobalValues.turn1);
    json["turn2"] = String(gGlobalValues.turn2);
    json["maxAcc"] = String(gGlobalValues.maxAcc);
    json["maxDec"] = String(gGlobalValues.maxDec);
    json["maxSpeed"] = String(gGlobalValues.maxSpeed);
    json["homingSpeed"] = String(gGlobalValues.homingSpeed);
    json["maxTime"] = String(gGlobalValues.maxTime);
    json["maxLaps"] = String(gGlobalValues.maxLaps);
    json["servSpeed"] = String(gGlobalValues.servSpeed);
    serializeJson(json, *response);
    request->send(response);
}

//==============================================================================
//  Exported functions
//==============================================================================

// Initialize update webserver
eStatus WebserverInit(void * params) 
{
    // IVA: TODO: Move SPIFFS and Wifi intialization outside!
    if (!SPIFFS.begin())
    {
        Log(eLogCrit, CMP_NAME, "WebserverInit: An Error has occurred while mounting SPIFFS");
        return eFAIL;
    }

    // Connect to WiFi network
    Log(eLogInfo, CMP_NAME, "WebserverInit: Connecting to Wifi: %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) 
    {
        // IVA: TODO: Use FreeRTOS primitives!
        vTaskDelay(500 / portTICK_PERIOD_MS);
        Log(eLogInfo, CMP_NAME, ".");
    }
  
    Log(eLogInfo, CMP_NAME, "WebserverInit: Connected to %s", WIFI_SSID);
    Log(eLogInfo, CMP_NAME, "IP address: %s", WiFi.localIP().toString().c_str());

    // Advertise through MDNS
    if (!MDNS.begin(WIFI_DEVICE_NAME)) 
    {
        Log(eLogWarn, CMP_NAME, "WebserverInit: Error setting up MDNS responder!");
    } 
    else
    {
        Log(eLogInfo, CMP_NAME, "mDNS responder started");
    }
    
    // Web root 
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html", "text/html");
        response->addHeader("Connection", "close");
        request->send(response);

    });

    server.on("/modeValues", HTTP_GET, [](AsyncWebServerRequest *request){
        Log(eLogDebug, CMP_NAME, "modeValues received");
        getModeValues(request);
    });

    server.on("/globalValues", HTTP_GET, [](AsyncWebServerRequest *request){
        Log(eLogDebug, CMP_NAME, "modeValues received");
        getGlobalValues(request);
    });

    // Firmware update handler
    server.on("/updateFirmware", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (!Update.hasError()) {
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
            response->addHeader("Connection", "close");
            request->send(response);
            ESP.restart();
        } else {
            AsyncWebServerResponse *response = request->beginResponse(500, "text/plain", "ERROR");
            response->addHeader("Connection", "close");
            request->send(response);
        } }, firmwareUpload);

    // SPIFFS update handler
    server.on("/updateSpiffs", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (!Update.hasError()) {
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
            response->addHeader("Connection", "close");
            request->send(response);
            ESP.restart();
        } else {
            AsyncWebServerResponse *response = request->beginResponse(500, "text/plain", "ERROR");
            response->addHeader("Connection", "close");
            request->send(response);
        } }, spiffsUpload);

    // serve static files directly
    server.serveStatic("/", SPIFFS, "/");


    // Everything else - 404
    server.onNotFound(notFoundResponse);
   
    server.begin();

    return eOK;
}

