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
static bool             initialized = false;
static AsyncWebServer   server(80);
static AsyncWebSocket   socket("/ws");

//==============================================================================
//  Local function definitions
//==============================================================================
static void notFoundResponse(AsyncWebServerRequest *request);

static void doUpdate(int updatingWhat, AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
static void updateRequestFn(AsyncWebServerRequest *request);
static void firmwareUploadFn(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
static void spiffsUploadFn(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

static eStatus webscocketSendJsonAll(const JsonDocument & json);
//==============================================================================
//  Local functions
//==============================================================================

// 404 default handler
static void notFoundResponse(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

//==============================================================================
//  OTA Update
//==============================================================================
// The actual update function
static void doUpdate(int updatingWhat, AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    // handle upload and update
    if (!index)
    {
        Log(eLogInfo, CMP_NAME, "doUpdate: Update %s", filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, updatingWhat/*U_FLASH/U_SPIFFS*/))
        { 
            Log(eLogCrit, CMP_NAME, "doUpdate: Error updating!");
            //start with max available size
            Update.printError(Serial);
        }
    }

    // flashing firmware to ESP
    if (len)
    {
        Update.write(data, len);
    }

    if (final)
    {
        if (Update.end(true))
        { 
            //true to set the size to the current progress
            Log(eLogInfo, CMP_NAME, "doUpdate: Update Success: %ub written, rebooting...", index+len);
        }
        else
        {
            Log(eLogCrit, CMP_NAME, "doUpdate: Error updating!");
            Update.printError(Serial);
        }
    }
}

static void updateRequestFn(AsyncWebServerRequest *request)
{
    if (!Update.hasError()) {
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
            response->addHeader("Connection", "close");
            request->send(response);
            SystemRestart();
        } else {
            AsyncWebServerResponse *response = request->beginResponse(500, "text/plain", "ERROR");
            response->addHeader("Connection", "close");
            request->send(response);
        }
}

// Firmware upload
static void firmwareUploadFn(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    doUpdate(U_FLASH, request, filename, index, data, len, final);
}

// SPIFFS upload
static void spiffsUploadFn(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    doUpdate(U_SPIFFS, request, filename, index, data, len, final);
}

//==============================================================================
//  Helper functions
//==============================================================================

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

template<typename T>eStatus setVarFromJson(const JsonObject &json, const char * const key, T * const var )
{
    eStatus retVal = eFAIL;
     if (json.containsKey(key)) {
        *var = json[key].as<T>();
        retVal = eOK;
    }
    else
    {
        Log(eLogWarn, CMP_NAME, "setVariableFromJson: no %s", key);
    }
    return retVal;
}

template<typename T>eStatus setConfigVarFromJson(const JsonObject &json, const char * const key, RuntimeConfigVariable<T>& configVar)
{
    eStatus retVal = eFAIL;

    if (json.containsKey(key))
    {
        if (configVar.Get() != json[key].as<T>())
        {
            Log(eLogInfo, CMP_NAME, "setConfigFromJson: setting %s", key);
            configVar.Set(json[key].as<T>());
            retVal = eOK;
        }
        else
        {
            retVal = eNOCHANGE;
        }
    }

    return retVal;
}

//==============================================================================
//  Get/Post mode values
//==============================================================================
static eStatus requestModeValues(const JsonObject & json)
{
    eStatus retVal = eFAIL;
    Log(eLogInfo, CMP_NAME, "requestModeValues");
    if (json.containsKey("mode"))
    {
        Modes mode = modeFromString(json["mode"]);
        if (mode != eModeCount)
        {
            SerialRequestModeValues(mode);
            retVal = eOK;
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "requestModeValues: Invalid mode: %s", json["mode"].as<String>().c_str());
        }
    }
    else
    {
        Log(eLogWarn, CMP_NAME, "requestModeValues: no mode!");
    }
    
    return retVal;
}

eStatus PushModeValues(const ModeValues & values)
{
    Log(eLogInfo, CMP_NAME, "PushModeValues");

    DynamicJsonDocument json(256);
    json["type"] = String("ModeValues");
    json["data"]["mode"] = String(values.mode);
    json["data"]["speed"] = String(values.speed);
    json["data"]["turn1"] = String(values.turn1);
    json["data"]["turn2"] = String(values.turn2);
    json["data"]["brakeTime"] = String(values.brakeTime);
    json["data"]["acc"] = String(values.acc);
    json["data"]["dec"] = String(values.dec);
    return webscocketSendJsonAll(json);    
}

static eStatus receiveModeValues(const JsonObject &json)
{
    eStatus retVal = eOK;
    Log(eLogInfo, CMP_NAME, "receiveModeValues");

    if (json.containsKey("mode"))
    {
        Modes mode = modeFromString(json["mode"]);
        ModeValues values;

        if (eModeCount != mode)
        {
            values.mode = mode;
            retVal = (eStatus)(retVal | setVarFromJson(json, "speed",       &values.speed ));
            retVal = (eStatus)(retVal | setVarFromJson(json, "turn1",       &values.turn1));
            retVal = (eStatus)(retVal | setVarFromJson(json, "turn2",       &values.turn2));
            retVal = (eStatus)(retVal | setVarFromJson(json, "brakeTime",   &values.brakeTime));
            retVal = (eStatus)(retVal | setVarFromJson(json, "acc",         &values.acc));
            retVal = (eStatus)(retVal | setVarFromJson(json, "dec",         &values.dec));

            if (eOK == retVal) 
            {
                Log(eLogInfo, CMP_NAME, "receiveModeValues: updated mode %s", json["mode"].as<String>().c_str());
            }
            else 
            {
                Log(eLogWarn, CMP_NAME, "receiveModeValues: got status %d while extracting data!", retVal);
            }
            DumpModeValues(&values);

            SerialSendModeValues(&values);
            retVal = eOK;
        }
        else 
        {
            Log(eLogWarn, CMP_NAME, "receiveModeValues: got invalid mode %s", json["mode"].as<String>().c_str());
        }
    }
    else
    {
        Log(eLogWarn, CMP_NAME, "receiveModeValues: no mode!");
    }

    return retVal;
}

//==============================================================================
//  Get/Post global values
//==============================================================================
static eStatus requestGlobalValues()
{
    Log(eLogInfo, CMP_NAME, "requestGlobalValues");
    SerialRequestGlobalValues();
    return eOK;
}

eStatus PushGlobalValues(const GlobalValues & values)
{
    Log(eLogInfo, CMP_NAME, "PushGlobalValues");
    DynamicJsonDocument json(1024);
    json["type"] = String("GlobalValues");
    json["data"]["home"] = String(values.home);
    json["data"]["maxEnd"] = String(values.maxEnd);
    json["data"]["maxTurn1"] = String(values.maxTurn1);
    json["data"]["minTurn2"] = String(values.minTurn2);
    json["data"]["maxAcc"] = String(values.maxAcc);
    json["data"]["maxDec"] = String(values.maxDec);
    json["data"]["maxSpeed"] = String(values.maxSpeed);
    json["data"]["homingSpeed"] = String(values.homingSpeed);
    json["data"]["maxTime"] = String(values.maxTime);
    json["data"]["maxLaps"] = String(values.maxLaps);
    json["data"]["servSpeed"] = String(values.servSpeed);
    return webscocketSendJsonAll(json);
}

static eStatus receiveGlobalValues(const JsonObject &json)
{
    eStatus retVal = eOK;
    Log(eLogInfo, CMP_NAME, "receiveGlobalValues");
    GlobalValues values;
    retVal = (eStatus)(retVal | setVarFromJson(json, "home",        &values.home));
    retVal = (eStatus)(retVal | setVarFromJson(json, "maxEnd",      &values.maxEnd));
    retVal = (eStatus)(retVal | setVarFromJson(json, "maxTurn1",    &values.maxTurn1));
    retVal = (eStatus)(retVal | setVarFromJson(json, "minTurn2",    &values.minTurn2 ));
    retVal = (eStatus)(retVal | setVarFromJson(json, "maxAcc",      &values.maxAcc));
    retVal = (eStatus)(retVal | setVarFromJson(json, "maxDec",      &values.maxDec));
    retVal = (eStatus)(retVal | setVarFromJson(json, "maxSpeed",    &values.maxSpeed));
    retVal = (eStatus)(retVal | setVarFromJson(json, "homingSpeed", &values.homingSpeed));
    retVal = (eStatus)(retVal | setVarFromJson(json, "maxTime",     &values.maxTime));
    retVal = (eStatus)(retVal | setVarFromJson(json, "maxLaps",     &values.maxLaps));
    retVal = (eStatus)(retVal | setVarFromJson(json, "servSpeed",   &values.servSpeed));

    SerialSendGlobalValues(&values);

    return retVal;
}

//==============================================================================
//  Get system status
//==============================================================================
eStatus pushSystemStatus()
{
    String tmp;
    DynamicJsonDocument json(512);
    
    json["type"] = String("SystemStatus");
    json["data"]["wifi"]["ssid"] = Config.WifiSSID.Get();
    SystemGetWifiStatus(tmp);
    json["data"]["wifi"]["status"] = tmp;
    json["data"]["system"]["buildId"] = String(SystemGetBuildId());
    json["data"]["system"]["buildTime"] = String(SystemGetBuildTime());
    
    SystemGetMemoryInfo(tmp);
    json["data"]["system"]["memoryStats"] = tmp;

    return webscocketSendJsonAll(json);
}

//==============================================================================
//  Get/Post system configuration
//==============================================================================
static eStatus pushConfig()
{
    DynamicJsonDocument json(1024);
    json["type"] = String("Config");
    json["data"] = Config.json().as<JsonObject>();

    return webscocketSendJsonAll(json);
}

static eStatus receiveConfig(const JsonObject &json)
{
    eStatus retVal = eFAIL;
    bool systemRestart = false;
    bool vpnRestart = false;

    if (eOK == setConfigVarFromJson(json, "WifiSSID",           Config.WifiSSID)) systemRestart = true;
    if (eOK == setConfigVarFromJson(json, "WifiPassword",       Config.WifiPassword)) systemRestart = true;

    if (eOK == setConfigVarFromJson(json, "VpnEnabled",         Config.VpnEnabled)) vpnRestart = true;
    if (eOK == setConfigVarFromJson(json, "VpnLocalAddress",    Config.VpnLocalAddress)) vpnRestart = true;
    if (eOK == setConfigVarFromJson(json, "VpnLocalNetmask",    Config.VpnLocalNetmask)) vpnRestart = true;
    if (eOK == setConfigVarFromJson(json, "VpnGateway",         Config.VpnGateway)) vpnRestart = true;
    if (eOK == setConfigVarFromJson(json, "VpnClientPort",      Config.VpnClientPort)) vpnRestart = true;
    if (eOK == setConfigVarFromJson(json, "VpnClientPrivateKey",Config.VpnClientPrivateKey)) vpnRestart = true;
    if (eOK == setConfigVarFromJson(json, "VpnPeerPort",        Config.VpnPeerPort)) vpnRestart = true;
    if (eOK == setConfigVarFromJson(json, "VpnPeerAddress",     Config.VpnPeerAddress)) vpnRestart = true;
    if (eOK == setConfigVarFromJson(json, "VpnPeerPublicKey",   Config.VpnPeerPublicKey)) vpnRestart = true;
    
    if (systemRestart)
    {
        SystemRestart();
    }

    if (vpnRestart)
    {
        //VpnRestart();
    }

    return retVal;
}

//==============================================================================
//  Get device status
//==============================================================================
eStatus PushStatus(const CurrentStatus & status)
{
    DynamicJsonDocument json(256);
    json["type"] = String("Status");
    json["data"]["mode"] = String(status.mode);
    json["data"]["completedLaps"] = String(status.completedLaps);
    json["data"]["position"] = String(status.position);
    json["data"]["systemStatus"] = String(status.systemStatus - 0x30);

    return webscocketSendJsonAll(json);
}

//==============================================================================
//  Websocket handling
//==============================================================================
static eStatus webscocketSendJsonAll(const JsonDocument & json)
{
    eStatus retVal = eFAIL;
    size_t len = measureJson(json);
    AsyncWebSocketMessageBuffer * buffer = socket.makeBuffer(len);
    if (buffer) 
    {
        if (len == serializeJson(json,buffer->get(), len))
        {
            socket.textAll(buffer);
            retVal = eOK;
        }
        else
        {
            Log(eLogWarn, CMP_NAME, "webscocketSendJsonAll: Error serializing, ended up with: %s", buffer->get());
        }        
    }
    return retVal;
}

static eStatus  websocketHandleMessage(AsyncWebSocketClient * client, void *arg, uint8_t *data, size_t len) 
{
    eStatus retVal = eFAIL;

    dumpRequestData("websocketHandleMessage", data, len);

    AwsFrameInfo *info = (AwsFrameInfo*)arg;

    // We expect all incomming data to be in one chunk (small) and Text - JSON
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
    {
        DynamicJsonDocument json(1024);
        auto resultError = deserializeJson(json, (const char *) data, len);

        if (resultError) 
        {
            Log(eLogWarn, CMP_NAME, "websocketHandleMessage: Deserialization error: %s", resultError.c_str());
        }
        else
        {
            const char *action = json["action"];
            const JsonObject data = json["data"];
            if (!action)
            {
                Log(eLogWarn, CMP_NAME, "websocketHandleMessage: No action!");
            }
            else
            {
                if (0 == strcmp(action, "getConfig"))
                {
                    retVal = pushConfig();
                }
                else if (0 == strcmp(action, "getSystemStatus"))
                {
                    retVal = pushSystemStatus();
                }
                else if ( 0 == strcmp(action, "setConfig"))
                {
                    retVal = receiveConfig(data);
                }
                else if ( 0 == strcmp(action, "getGlobalValues"))
                {
                    retVal = requestGlobalValues();
                }
                else if ( 0 == strcmp(action, "setGlobalValues"))
                {
                    retVal = receiveGlobalValues(data);
                }
                else if (0 == strcmp(action, "getModeValues"))
                {
                    retVal = requestModeValues(data);
                }
                else if (0 == strcmp(action, "setModeValues"))
                {
                    retVal = receiveModeValues(data);
                }
                else 
                {
                    Log(eLogWarn, CMP_NAME, "websocketHandleMessage: Unknown action %s", action);
                }
            }
        }
    }
    else
    {
        Log(eLogWarn, "websocketHandleMessage: Unexpected data!");
        dumpRequestData("websocketHandleMessage", data, len);
    }

    return retVal;
}


static void websocketOnEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, 
        AwsEventType type, void * arg, uint8_t *data, size_t len)
{
    // None of this is really needed
    if(type == WS_EVT_CONNECT)
    {
        //client connected
        Log(eLogDebug, CMP_NAME, "ws[%s][%u] connect\n", server->url(), client->id());
        client->ping();
    } 
    else if(type == WS_EVT_DISCONNECT)
    {
        //client disconnected
        Log(eLogDebug, CMP_NAME, "ws[%s][%u] disconnect: %u\n", server->url(), client->id());
    } 
    else if(type == WS_EVT_ERROR)
    {
        //error was received from the other end
        Log(eLogDebug, CMP_NAME, "ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    } 
    else if(type == WS_EVT_PONG)
    {
        //pong message was received (in response to a ping request maybe)
        Log(eLogDebug, CMP_NAME, "ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
    }
    else if(type == WS_EVT_DATA)
    {
        websocketHandleMessage(client, arg, data, len);
    }
}

//==============================================================================
//  Exported functions
//==============================================================================

eStatus WebserverCloseSockets()
{
    if (initialized)
    {
        socket.closeAll();
    }
    return eOK;
}

// Initialize webserver
eStatus WebserverInit()
{
    Log(eLogInfo, CMP_NAME, "Webserver init called");
    
    // Web root 
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html", "text/html");
        response->addHeader("Connection", "close");
        request->send(response);
    });

    // OTA Update Handlers
    server.on("/updateFirmware", HTTP_POST, updateRequestFn, firmwareUploadFn);
    server.on("/updateSpiffs", HTTP_POST, updateRequestFn, spiffsUploadFn);

    // serve static files directly
    server.serveStatic("/", SPIFFS, "/");

    // Websocket 
    socket.onEvent(websocketOnEvent);
    server.addHandler(&socket);

    // Everything else - 404
    server.onNotFound(notFoundResponse);
   
    server.begin();

    initialized = true;

    return eOK;
}

// Cleans-up websockets to avoid exhaustion
eStatus WebserverTask()
{
    if (initialized)
    {
        socket.cleanupClients();
    }
    
    return eOK;
}
