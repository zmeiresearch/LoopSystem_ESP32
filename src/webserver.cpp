/*==============================================================================
   LoopSystem ESP32
   
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <string>

#include <SPIFFS.h>
#include <Update.h>
#include <ArduinoJson.h>
#include "ESPAsyncWebServer.h"

#include "config.h"
#include "globals.h"
#include "logger.h"
#include "values.h"
#include "control_serial.h"
#include "system.h"

#include "runtime_config.h"

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

static Modes modeFromString(String modeStr)
{
    Modes mode = eModeCount;
    if (0 == modeStr.compareTo("novice"))
    {
        mode = eModeNovice;
    } 
    else if (0 == modeStr.compareTo("expert"))
    {
        mode = eModeExpert;
    } 
    else if (0 == modeStr.compareTo("advanced"))
    {
        mode = eModeAdvanced;
    }
    else if (0 == modeStr.compareTo("master"))
    {
        mode = eModeMaster;
    }

    return mode;
}

static void getModeValues(AsyncWebServerRequest *request)
{
    const int paramCount = request->params();

    Log(eLogInfo, CMP_NAME, "getModeValues: got %d params", paramCount);

    for (int i = 0; i < paramCount; i++)
    {
        AsyncWebParameter* p = request->getParam(i);
        Log(eLogInfo, CMP_NAME, "getModeValues: id: %s:%s", p->name().c_str(), p->value().c_str());

        if (0 == p->name().compareTo("mode"))
        {
            Modes mode =  modeFromString(p->value());
            
            if (eModeCount == mode)
            {
                Log(eLogWarn, CMP_NAME, "getModeValues: Unknown mode!");
            }
            else
            {

                SendPacketRequestModeValues(mode);

                vTaskDelay(500/portTICK_PERIOD_MS);

                AsyncResponseStream *response = request->beginResponseStream("application/json");
                DynamicJsonDocument json(1024);
                json["speed"] = String(gModeValues[mode].speed);
                json["turn1"] = String(gModeValues[mode].turn1);
                json["turn2"] = String(gModeValues[mode].turn2);
                json["brakeTime"] = String(gModeValues[mode].brakeTime);
                json["acc"] = String(gModeValues[mode].acc);
                json["dec"] = String(gModeValues[mode].dec);
                serializeJson(json, *response);
                request->send(response);
            }
        }
    }
}


static void postModeValues(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
    Log(eLogInfo, CMP_NAME, "postModeValues: Processing %d bytes: %s", len, data);

    //DynamicJsonDocument doc(total);
    DynamicJsonDocument json(1024);
    auto resultError = deserializeJson(json, (const char *) data, len);

    //Log(eLogWarn, CMP_NAME, "postModeValues: json[values]:%s", json["values"]);

    if (resultError)
    {
        Log(eLogWarn, CMP_NAME, "postModeValues: Deserialization error: %s", resultError.c_str());
    }
    else
    {
        const char * modeStr = json["mode"];

        if (modeStr)
        {
            Modes mode = modeFromString(String(modeStr));

            Log(eLogInfo, CMP_NAME, "posModeValues: got mode %d", mode);

            if (eModeCount != mode)
            {
                if (json.containsKey("speed")) {
                    gModeValues[mode].speed = json["speed"].as<uint32_t>();
                }
                else
                {
                    Log(eLogWarn, CMP_NAME, "postModeValues: no speed!");
                }

                if (json.containsKey("turn1")) {
                    gModeValues[mode].turn1 = json["turn1"].as<uint32_t>();
                }
                else
                {
                    Log(eLogWarn, CMP_NAME, "postModeValues: no turn1!");
                }

                if (json.containsKey("turn2")) {
                    gModeValues[mode].turn2 = json["turn2"].as<uint32_t>();
                }
                else
                {
                    Log(eLogWarn, CMP_NAME, "postModeValues: no turn2!");
                }

                if (json.containsKey("brakeTime")) {
                    gModeValues[mode].brakeTime = json["brakeTime"].as<uint32_t>();
                }
                else
                {
                    Log(eLogWarn, CMP_NAME, "postModeValues: no brakeTime!");
                }

                if (json.containsKey("acc")) {
                    gModeValues[mode].acc = json["acc"].as<uint32_t>();
                }
                else
                {
                    Log(eLogWarn, CMP_NAME, "postModeValues: no acc!");
                }

                if (json.containsKey("dec")) {
                    gModeValues[mode].dec = json["dec"].as<uint32_t>();
                }
                else
                {
                    Log(eLogWarn, CMP_NAME, "postModeValues: no dec!");
                }

                

                Log(eLogInfo, CMP_NAME, "postModeValues: updated mode %s", modeStr);
                DumpModeValues(&gModeValues[mode]);

                SendPacketModeValuesAscii(mode);
            }
        }
    }
}

static void getGlobalValues(AsyncWebServerRequest *request)
{
    SendPacketRequestGlobalValues();

    vTaskDelay(500/portTICK_PERIOD_MS);

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument json(1024);
    json["home"] = String(gGlobalValues.home);
    json["maxEnd"] = String(gGlobalValues.maxEnd);
    json["maxTurn1"] = String(gGlobalValues.maxTurn1);
    json["minTurn2"] = String(gGlobalValues.minTurn2);
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

static void getStatus(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument json(1024);
    json["mode"] = String(gStatus.mode);
    json["completedLaps"] = String(gStatus.completedLaps);
    json["position"] = String(gStatus.position);
    json["systemStatus"] = String(gStatus.systemStatus - 0x30);
    serializeJson(json, *response);
    request->send(response);
}

static void postGlobalValues(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{

    Log(eLogInfo, CMP_NAME, "postGlobalValues: Processing %d bytes: %s", len, data);

    //DynamicJsonDocument doc(total);
    DynamicJsonDocument json(1024);
    auto resultError = deserializeJson(json, (const char *) data, len);

    if (resultError)
    {
        Log(eLogWarn, CMP_NAME, "postGlobalValues: Deserialization error: %s", resultError.c_str());
    }
    else
    {
        const char* home = json["values"]["home"];
        if (home) {
            gGlobalValues.home = json["values"]["home"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no home!");
        }

        const char* maxEnd = json["values"]["maxEnd"];
        if (maxEnd) {
            gGlobalValues.maxEnd = json["values"]["maxEnd"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxEnd!");
        }

        const char* maxTurn1 = json["values"]["maxTurn1"];
        if (maxTurn1) {
            gGlobalValues.maxTurn1 = json["values"]["maxTurn1"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxTurn1!");
        }


        const char* minTurn2 = json["values"]["minTurn2"];
        if (minTurn2) {
            gGlobalValues.minTurn2 = json["values"]["minTurn2"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no minTurn2!");
        }


        const char* maxAcc = json["values"]["maxAcc"];
        if (maxAcc) {
            gGlobalValues.maxAcc = json["values"]["maxAcc"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxAcc!");
        }


        const char* maxDec = json["values"]["maxDec"];
        if (maxDec) {
            gGlobalValues.maxDec = json["values"]["maxDec"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxDec!");
        }


        const char* maxSpeed = json["values"]["maxSpeed"];
        if (maxSpeed) {
            gGlobalValues.maxSpeed = json["values"]["maxSpeed"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxSpeed!");
        }


        const char* homingSpeed = json["values"]["homingSpeed"];
        if (homingSpeed) {
            gGlobalValues.homingSpeed = json["values"]["homingSpeed"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no homingSpeed!");
        }


        const char* maxTime = json["values"]["maxTime"];
        if (maxTime) {
            gGlobalValues.maxTime = json["values"]["maxTime"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxTime!");
        }

        const char* maxLaps = json["values"]["maxLaps"];
        if (maxLaps) {
            gGlobalValues.maxLaps = json["values"]["maxLaps"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxLaps!");
        }

        const char* servSpeed = json["values"]["servSpeed"];
        if (servSpeed) {
            gGlobalValues.servSpeed = json["values"]["servSpeed"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no servSpeed!");
        }
    }       

    SendPacketGlobalValuesAscii();
}

static eStatus getConfig(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument json(1024);
    json["wifi"]["ssid"] = String(ConfigWifiSSID());
    json["wifi"]["password"] = String(ConfigWifiPassword());
    serializeJson(json, *response);
    request->send(response);
}

static eStatus postConfig(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
    eStatus retVal = eOK;

    Log(eLogInfo, CMP_NAME, "postConfig: Processing %d bytes: %s", len, data);

    //DynamicJsonDocument doc(total);
    DynamicJsonDocument json(1024);
    auto resultError = deserializeJson(json, (const char *) data, len);

    if (resultError)
    {
        Log(eLogWarn, CMP_NAME, "postConfig: Deserialization error: %s", resultError.c_str());
    }
    else
    {
        const char* ssid = json["wifi"]["ssid"];
        const char* password = json["wifi"]["password"];
        if ((ssid) && (password))
        {
            Log(eLogWarn, CMP_NAME, "postConfig: Setting wifi parameters: %s, %s", ssid, password);
            ConfigWriteWifiSSID(json["wifi"]["ssid"].as<String>());
            ConfigWriteWifiPassword(json["wifi"]["password"].as<String>());
            SystemRestart();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postConfig: incomplete wifi configuration!");
        } 
    }

    return retVal;
}


//==============================================================================
//  Exported functions
//==============================================================================

// Initialize update webserver
eStatus WebserverInit()
{
    Log(eLogInfo, CMP_NAME, "Webserver init called");
    
    // Web root 
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html", "text/html");
        response->addHeader("Connection", "close");
        request->send(response);

    });

    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
        //Log(eLogDebug, CMP_NAME, "get modeValues received");
        getStatus(request);
    });

    server.on("/modeValues", HTTP_GET, [](AsyncWebServerRequest *request){
        //Log(eLogDebug, CMP_NAME, "get modeValues received");
        getModeValues(request);
    });

    server.on("/globalValues", HTTP_GET, [](AsyncWebServerRequest *request){
        //Log(eLogDebug, CMP_NAME, "get globalValues received");
        getGlobalValues(request);
    });

    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
        //Log(eLogDebug, CMP_NAME, "get config received");
        getConfig(request);
    });

    server.on("/modeValues", HTTP_POST,
    [](AsyncWebServerRequest * request){
        Log(eLogDebug, CMP_NAME, "post modeValues received");
        },
        NULL,
        [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
            Log(eLogDebug, CMP_NAME, "post modeValues body handling");
            postModeValues(request, data, len, index, total);
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
        });

    server.on("/globalValues", HTTP_POST,
    [](AsyncWebServerRequest * request){
            Log(eLogDebug, CMP_NAME, "post globalValues received");
        },
        NULL,
        [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
            Log(eLogDebug, CMP_NAME, "post globalValues body handling");
            postGlobalValues(request, data, len, index, total);
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
        });

    server.on("/config", HTTP_POST,
    [](AsyncWebServerRequest * request){
            Log(eLogDebug, CMP_NAME, "post config received");
        },
        NULL,
        [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
            Log(eLogDebug, CMP_NAME, "post config body handling");
            postConfig(request, data, len, index, total);
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
        });

    // Firmware update handler
    server.on("/updateFirmware", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (!Update.hasError()) {
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
            response->addHeader("Connection", "close");
            request->send(response);
            SystemRestart();
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
            SystemRestart();
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

