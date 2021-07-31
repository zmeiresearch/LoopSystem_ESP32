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

// Dump request data to log - requires special handling as the data is not null-terminated
static void dumpRequestData(const char * const requestName, const uint8_t * const data, const size_t len)
{
    char *  tmp = (char *)malloc(len+1);
    memcpy(tmp, data, len);
    tmp[len] = 0;

    Log(eLogInfo, CMP_NAME, "%s: Processing %d bytes: %s", requestName, len, tmp);

    free(tmp);
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
    dumpRequestData("postModeValues", data, len);

    DynamicJsonDocument json(1024);
    auto resultError = deserializeJson(json, (const char *) data, len);

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

    dumpRequestData("postGlobalValues", data, len);

    //DynamicJsonDocument doc(total);
    DynamicJsonDocument json(1024);
    auto resultError = deserializeJson(json, (const char *) data, len);

    if (resultError)
    {
        Log(eLogWarn, CMP_NAME, "postGlobalValues: Deserialization error: %s", resultError.c_str());
    }
    else
    {
        if (json.containsKey("home")) {
            gGlobalValues.home = json["home"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no home!");
        }

        if (json.containsKey("maxEnd")) {
            gGlobalValues.maxEnd = json["maxEnd"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxEnd!");
        }

        if (json.containsKey("maxTurn1")) {
            gGlobalValues.maxTurn1 = json["maxTurn1"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxTurn1!");
        }

        if (json.containsKey("minTurn2")) {
            gGlobalValues.minTurn2 = json["minTurn2"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no minTurn2!");
        }

        if (json.containsKey("maxAcc")) {
            gGlobalValues.maxAcc = json["maxAcc"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxAcc!");
        }

        if (json.containsKey("maxDec")) {
            gGlobalValues.maxDec = json["maxDec"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxDec!");
        }

        if (json.containsKey("maxSpeed")) {
            gGlobalValues.maxSpeed = json["maxSpeed"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxSpeed!");
        }

        if (json.containsKey("homingSpeed")) {
            gGlobalValues.homingSpeed = json["homingSpeed"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no homingSpeed!");
        }

        if (json.containsKey("maxTime")) {
            gGlobalValues.maxTime = json["maxTime"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxTime!");
        }

        if (json.containsKey("maxLaps")) {
            gGlobalValues.maxLaps = json["maxLaps"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no maxLaps!");
        }

        if (json.containsKey("servSpeed")) {
            gGlobalValues.servSpeed = json["servSpeed"].as<uint32_t>();
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "postGlobalValues: no servSpeed!");
        }
    }       

    SendPacketGlobalValuesAscii();
}

static void getConfig(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument json(1024);
    json["wifi"]["ssid"] = String(ConfigWifiSSID());
    json["wifi"]["password"] = String(ConfigWifiPassword());
    serializeJson(json, *response);
    request->send(response);
}

static void postConfig(AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
{
    dumpRequestData("postConfig", data, len);

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
        getStatus(request);
    });

    server.on("/modeValues", HTTP_GET, [](AsyncWebServerRequest *request){
        getModeValues(request);
    });

    server.on("/globalValues", HTTP_GET, [](AsyncWebServerRequest *request){
        getGlobalValues(request);
    });

    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
        getConfig(request);
    });

    server.on("/modeValues", HTTP_POST,
    [](AsyncWebServerRequest * request){
        },
        NULL,
        [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
            postModeValues(request, data, len, index, total);
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
            request->send(response);
        });

    server.on("/globalValues", HTTP_POST,
    [](AsyncWebServerRequest * request){
        },
        NULL,
        [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
            postGlobalValues(request, data, len, index, total);
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
            request->send(response);
        });

    server.on("/config", HTTP_POST,
    [](AsyncWebServerRequest * request){
        },
        NULL,
        [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
            postConfig(request, data, len, index, total);
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
            request->send(response);
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
