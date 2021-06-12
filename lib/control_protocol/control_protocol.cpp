/*==============================================================================
   LoopSystem ESP32
   
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <stdlib.h>

#include "globals.h"
#include "config.h"
#include "logger.h"
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
#pragma pack(push, 1)

typedef struct _PacketAllValues {
    uint8_t             startByte;
    uint8_t             packetType;
    uint8_t             cEnd[3];
    uint8_t             cAcc[2];
    uint8_t             cDec[2];
    uint8_t             cTurn[2];
    uint8_t             gHome[3];
    uint8_t             gEnd[3];
    uint8_t             gTurn1[3];
    uint8_t             gTurn2[3];
    uint8_t             gMaxAcc[2];
    uint8_t             gMaxDec[2];
    uint8_t             gFMax[2];
    uint8_t             gFMin[2];
    uint8_t             gMaxTime[2];
    uint8_t             gMaxLaps[2];
    uint8_t             gServSpeed[2];
    uint8_t             checksum[2];
    uint8_t             stopByte;
} PacketAllValues;

#pragma pack(pop)

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

    uint16_t fromPacket = (uint16_t)*(&packet[packetLength-3]);
    uint16_t calculated = 0;

    // two bytes CRC + stop byte
    for (size_t i; i < packetLength - 3; i++) 
    {
        calculated += packet[i];
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



