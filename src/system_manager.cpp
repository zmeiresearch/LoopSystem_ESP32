/*==============================================================================
   LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include "system_manager.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include "SPIFFS.h"

#include "config.h"
#include "logger.h"

#include "webserver.h"
#include "runtime_config.h"
#include "system.h"
#include "wireguard_vpn.h"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME        "SysMgr"

//==============================================================================
//  Local types
//==============================================================================

//==============================================================================
//  Local function definitions
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================
static bool         spiffsInitialized = false;
static uint32_t     spiffsRetryCount = 0;

static bool         wifiConnected = false;
static bool         wifiApStarted = false;

static DNSServer    dnsServer;

static uint32_t     wifiApConnectStartTime = 0;
static uint32_t     wifiClientConnectStartTime = 0;


//==============================================================================
//  Local functions
//==============================================================================
static void setupmDNS()
{
    // Advertise through MDNS
    if (!MDNS.begin(WIFI_DEVICE_NAME)) 
    {
        Log(eLogWarn, CMP_NAME, "SystemManagerTask: Error setting up MDNS responder!");
    } 
    else
    {
        Log(eLogInfo, CMP_NAME, "SystemManagerTask: mDNS responder started");
        MDNS.addService("http", "tcp", 80);
		//MDNS.addServiceTxt("http", "tcp", "PATH", "/config");
    }
}

static void wifiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Log(eLogWarn, CMP_NAME, "wifiDicsonnected, retrying!");
    WiFi.begin(Config.WifiSSID.Get().c_str(), Config.WifiPassword.Get().c_str());
}

static eStatus connectWifi(const char * const ssid, const char * const password)
{
    eStatus retVal = eFAIL;

    Log(eLogInfo, CMP_NAME, "connectWifi: Connecting to %s", ssid);

    WiFi.mode(WIFI_STA);
    WiFi.onEvent(wifiDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);

    // IVA: TODO: handle static IP address!
    //WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(WIFI_DEVICE_NAME);
    WiFi.begin(ssid, password);

    uint32_t startTime = xTaskGetTickCount();
    // Wait for connection
    while ( (WiFi.status() != WL_CONNECTED) &&
            (( (xTaskGetTickCount() - startTime) * portTICK_PERIOD_MS) < WIFI_DEFAULT_CONNECT_TIME))
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        Log(eLogInfo, CMP_NAME, "connectWifi: Connecting...");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        retVal = eOK;
        Log(eLogInfo, CMP_NAME, "connectWifi: Connected to %s, IP address: %s", ssid, WiFi.localIP().toString().c_str());
    }
    else
    {
        retVal = eFAIL;
        Log(eLogWarn, CMP_NAME, "SystemManagerTask: Unable to connect to %s", ssid);
        WiFi.mode(WIFI_OFF);
    }

    return retVal;
}

static eStatus startAP(const char * const ssid, const char * const password)
{
    
    IPAddress apIp, apNetmask;
    apIp.fromString(WIFI_AP_ADDRESS);
    apNetmask.fromString(WIFI_AP_NETMASK);

    Log(eLogInfo, CMP_NAME, "startAP: Setting up AP ssid:%s, IP address:%s", ssid, apIp.toString().c_str());

    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
	WiFi.softAP(ssid, password, WIFI_AP_CHANNEL);
    SystemSleepMs(1000);
    WiFi.softAPConfig(apIp, apIp, apNetmask);

	// Ensure we don't poison the client DNS cache
	dnsServer.setTTL(0);
	dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer.start(53, "*", apIp);							// 53 is port for DNS server

    return eOK; // Not sure how this could fail
}

//==============================================================================
//  Exported functions
//==============================================================================
eStatus SystemManagerInit(void * params)
{
    Log(eLogInfo, CMP_NAME, "SystemManagerInit: Build ID:%s", SystemGetBuildId());

    Config.Init();

    return eOK;
}

eStatus SystemManagerTask()
{

    if (!spiffsInitialized && spiffsRetryCount < SPIFFS_RETRY_LIMIT)
    {
        if (SPIFFS.begin())
        {
            Log(eLogInfo, CMP_NAME, "SystemManagerTask: SPIFFS initialization success");
            spiffsInitialized = true;
        }
        else
        {
            Log(eLogCrit, CMP_NAME, "SystemManagerTask: SPIFFS intialization failure!");
            spiffsRetryCount++;
        }
    }

    if (!wifiConnected && !wifiApStarted)
    {
        Log(eLogInfo, CMP_NAME, "SystemManagerTask: Connecting to configured Wifi: %s", Config.WifiSSID.Get().c_str());

        if ( 0 == wifiClientConnectStartTime)
        {
            wifiClientConnectStartTime = SystemGetTimeMs();
        }

        if (eOK == connectWifi(Config.WifiSSID.Get().c_str(), Config.WifiPassword.Get().c_str()))
        {
            wifiConnected = true;
            WebserverInit();
            if (Config.VpnEnabled.Get())
            {
                WireguardSetup();
            }

            setupmDNS();
        }
        else
        {
            if (SystemElapsedTimeMs(wifiClientConnectStartTime) < WIFI_WAIT_FOR_AP_TIME)
            {
                Log(eLogInfo, CMP_NAME, "SystemManagerTask: Unable to connect to wifi, retrying");
            }
            else
            {
                if (eOK == startAP(WIFI_AP_SSID, WIFI_AP_PASSWORD))
                {
                    Log(eLogInfo, CMP_NAME, "SystemManagerTask: Started configuration AP %s", WIFI_AP_SSID);
                    wifiApStarted = true;
                    WebserverInit();
                    wifiApConnectStartTime = SystemGetTimeMs();
                }
                else
                {
                    Log(eLogError, CMP_NAME, "SystemManagerTask: Unable to start configuration AP!");
                    SystemRestart();
                }
            }
        }
    }

    if (wifiApStarted)
    {
        dnsServer.processNextRequest();
        if (SystemElapsedTimeMs(wifiApConnectStartTime) > WIFI_WAIT_FOR_CONFIG_TIME)
        {
            Log(eLogError, CMP_NAME, "SystemManagerTask: Configuration time in AP mode exceeded, restarting!");
            SystemRestart();
        }
    }

    // always loop
    return eOK;
}
