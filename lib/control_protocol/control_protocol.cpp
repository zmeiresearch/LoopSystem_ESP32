/*==============================================================================
   LoopSystem ESP32
   
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <stdlib.h>

#include "globals.h"
#include "config.h"

#ifndef MOCK_LOGGER
#else
#include "mock_logger.h"
#endif


#include "control_protocol.h"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME        "ControlProtocol"

#define START_BYTE      '*'
#define STOP_BYTE        '\n'

//==============================================================================
//  Local types
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================


//==============================================================================
//  Local functions
//==============================================================================

static eStatus checkStartByte(const uint8_t * const packet)
{
    eStatus retVal = eFAIL;

    uint8_t fromPacket = (uint8_t)packet[0];

    Log(eLogDebug, CMP_NAME, "checkStartByte: expected: %#02x, got: %#02x", START_BYTE, fromPacket);

    if (START_BYTE == fromPacket) 
    {
        retVal = eOK;
    }

    return retVal;
}

static eStatus checkChecksum(const uint8_t * const packet, const size_t packetLength)
{
    eStatus retVal = eFAIL;

    uint16_t fromPacket = ((uint16_t)packet[packetLength-3] << 8) + packet[packetLength-2];
    uint16_t calculated = 0;

    // two bytes CRC + stop byte
    for (size_t i = 0; i < packetLength - 3; i++) 
    {
        calculated = calculated + (uint16_t)packet[i];
    }

    calculated = ~calculated;

    Log(eLogDebug, CMP_NAME, "CheckChecksum: packetLength: %d, calculated: %#04x, got: %#04x", 
            packetLength, calculated, fromPacket);

    if (calculated == fromPacket)
    {
        retVal = eOK;
    }

    return retVal;
}

static void writeChecksum(const uint8_t * packet, const size_t packetLength)
{
    uint16_t * checksum = (uint16_t *)&packet[packetLength - 3];

    for (size_t i = 0; i < packetLength - 3; i++)
    {
        *checksum += packet[i];
    }

    *checksum = ~(*checksum);

    Log(eLogDebug, CMP_NAME, "writeChecksum: calulcated: %#04x", checksum);

}

static void setStartStop(uint8_t * const packet, const size_t packetLength)
{
    packet[0] = START_BYTE;
    packet[packetLength - 1] = STOP_BYTE;
}

static eStatus isBcd(const char val)
{
    if (val < '0' || val > '9') return eFAIL;

    return eOK;
}

//==============================================================================
//  Exported functions
//==============================================================================

eStatus CheckPacketAllValues(const PacketAllValues * const packet)
{
    eStatus retVal = eOK;

    retVal = checkStartByte((const uint8_t * const)packet);
    
    if (eOK != retVal)
    {
        Log(eLogWarn, CMP_NAME, "CheckPacketAllValues: error in start byte!");
    }

    if (eOK == retVal)
    {
        retVal = checkChecksum((const uint8_t * const)packet, sizeof(PacketAllValues));

        if (eOK != retVal)
        {
            Log(eLogWarn, CMP_NAME, "CheckPacketAllValues: Checksum error!");
        }
    }

    return retVal;
}

eStatus BcdToVal(const unsigned char * const bcd, uint8_t * const outVal)
{
    eStatus retVal = eFAIL;

    if (eOK == isBcd(bcd[0]) && eOK == isBcd(bcd[1]))
    {
        *outVal = ((uint8_t)bcd[0] - (uint8_t)0x30) * 10 + ((uint8_t)bcd[1] - (uint8_t)0x30);
        retVal = eOK;
    }
    return retVal;
}

eStatus BcdToVal(const unsigned char * const bcd, uint16_t * const outVal)
{
    eStatus retVal = eFAIL;

    if (eOK == isBcd(bcd[0]) && eOK == isBcd(bcd[1]) && eOK == isBcd(bcd[2]))
    {
        *outVal = ((uint8_t)bcd[0] - (uint8_t)0x30) * 100 + ((uint8_t)bcd[1] - (uint8_t)0x30) * 10 + ((uint8_t)bcd[2] - (uint8_t)0x30);
        retVal = eOK;
    }
    return retVal;

}

eStatus ValToBcd(const uint8_t val, unsigned char * const outBuf)
{
    eStatus retVal = eFAIL;
    if (val < 100)
    {
        outBuf[0] = (uint8_t)val/10 + 0x30;
        outBuf[1] = (uint8_t)val % 10 + 0x30;
        retVal = eOK;
    }
    return retVal;
}

eStatus ValToBcd(const uint16_t val, unsigned char * const outBuf)
{
    eStatus retVal = eFAIL;
    if (val < 1000)
    {
        outBuf[0] = (uint8_t)(val/100) + 0x30;
        outBuf[1] = (uint8_t)((val % 100) / 10) + 0x30;
        outBuf[2] = (uint8_t)(val % 10) + 0x30;
        retVal = eOK;
    }
    return retVal;
}

