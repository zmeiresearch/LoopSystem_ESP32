//==============================================================================
// Wireguard VPN Client demo for LwIP/ESP32     
//==============================================================================

//==============================================================================
//  Includes
//==============================================================================
extern "C" {
#include "wireguardif.h"
#include "wireguard.h"
}

#include "vpn_manager.h"
#include "logger.h"
#include "runtime_config.h"
#include "wifi_manager.h"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME "VpnMgr"

//==============================================================================
//  Local types
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================
static bool started = false;

static struct netif wg_netif = {0};
static uint8_t peer_index = WIREGUARDIF_INVALID_INDEX;

//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Local functions
//==============================================================================
static eStatus setIpAddrFromString(const String& inString, ip_addr_t * const outAddr )
{
    if (!ipaddr_aton(inString.c_str(), outAddr))
    {
        Log(eLogWarn, CMP_NAME, "setIpAddrFromString: Error parsing %s", inString.c_str());
        return eFAIL;
    }
    return eOK;
}


static eStatus wireguardConnect()
{
    Log(eLogInfo, CMP_NAME, "wireguardConnect");
    eStatus retVal = eOK;
    struct wireguardif_init_data wg;
    struct wireguardif_peer peer;
    ip_addr_t local_address;
    ip_addr_t local_netmask;
    ip_addr_t gateway_address;
    ip_addr_t peer_address;

    if (eOK == retVal) retVal = setIpAddrFromString(Config.VpnLocalAddress(),   &local_address);
    if (eOK == retVal) retVal = setIpAddrFromString(Config.VpnLocalNetmask(),   &local_netmask);
    if (eOK == retVal) retVal = setIpAddrFromString(Config.VpnGateway(),        &gateway_address);
    if (eOK == retVal) retVal = setIpAddrFromString(Config.VpnPeerAddress(),    &peer_address);

    if (eOK == retVal)
    {
        // Setup the WireGuard device structure
        wg.private_key = Config.VpnClientPrivateKey().c_str();
        wg.listen_port = Config.VpnClientPort();
        wg.bind_netif = NULL; // NB! not working on ESP32 even if set!

        // Register the new WireGuard network interface with lwIP
        if (NULL == netif_add(&wg_netif, ip_2_ip4(&local_address), ip_2_ip4(&local_netmask), ip_2_ip4(&gateway_address), &wg, &wireguardif_init, &ip_input)) 
        {
            Log(eLogWarn, CMP_NAME, "wireguardConnect: error adding netif!");
            retVal = eFAIL;
        }
        else
        {
            // Mark the interface as administratively up, link up flag is set automatically when peer connects
            netif_set_up(&wg_netif);
        }
    }

    if (eOK == retVal)
    {
        // Initialise the first WireGuard peer structure
        wireguardif_peer_init(&peer);
        peer.public_key = Config.VpnPeerPublicKey().c_str();
        peer.endport_port = Config.VpnPeerPort();
        peer.preshared_key = NULL;
        // Allow all IPs through tunnel
        IP_ADDR4(&peer.allowed_ip, 0, 0, 0, 0);
        IP_ADDR4(&peer.allowed_mask, 0, 0, 0, 0);

        ip_addr_set(&peer.endpoint_ip, &peer_address);

        // Register the new WireGuard peer with the netwok interface
        if (ERR_OK != wireguardif_add_peer(&wg_netif, &peer, &peer_index))
        {
            Log(eLogWarn, CMP_NAME, "wireguardConnect: error adding peer!");
            retVal = eFAIL;
        }
    }

    if (eOK == retVal)
    {
        if ((peer_index != WIREGUARDIF_INVALID_INDEX) && !ip_addr_isany(&peer.endpoint_ip))
        {
            // Start outbound connection to peer
            if (ERR_OK == wireguardif_connect(&wg_netif, peer_index))
            {
                Log(eLogInfo, CMP_NAME, "wireguardConnect: Connected!");
            }
            else
            {
                Log(eLogWarn, CMP_NAME, "wireguardConnect: error connecting to peer!");
                retVal = eFAIL;
            }
        }
    }

    return retVal;
}

static eStatus wireguardDisconnect()
{
    err_t wg_retVal =  wireguardif_disconnect(&wg_netif, peer_index);

    if (ERR_OK == wg_retVal)
    {
        wg_retVal = wireguardif_remove_peer(&wg_netif, peer_index);
        Log(eLogInfo, CMP_NAME, "wireguardDisconnect: Got %d removing peer");
    }
    else
    {
        Log(eLogWarn, CMP_NAME, "wireguardDisconnect: Error %d disconnecting peer!", wg_retVal);
    }

    return eOK;
}

//==============================================================================
//  Exported functions
//==============================================================================
eStatus VpnManagerInit(void * params)
{
    Log(eLogInfo, CMP_NAME, "VpnManagerInit");
    return eOK;
}

eStatus VpnRestart()
{
    Log(eLogInfo, CMP_NAME, "VpnRestart");
    
    // Disconnect if connected. Will retry connection on next task loop 
    // if conditions are right. Nothing to do otherwise
    if (started)
    {
        wireguardDisconnect();
        started = false;
    } 
    return eOK;
}

eStatus VpnManagerTask()
{
    Log(eLogDebug, CMP_NAME, "VpnManagerTask: enabled: %d, started: %d, wifi: %d", Config.VpnEnabled(), started, WifiIsConnected());
    if (!started && Config.VpnEnabled())
    {
        if (WifiIsConnected())
        {
            if (eOK == wireguardConnect())
            {
                started = true;
            }
        }
    }

    if (started && (!Config.VpnEnabled() || !WifiIsConnected()))
    {
        wireguardDisconnect();
        started = false;
    }

    return eOK; // continue looping
}