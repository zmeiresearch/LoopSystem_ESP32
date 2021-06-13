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
#include "logger.h"
#else
#include "mock_logger.h"
#endif


#include "control_protocol.h"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME        "ControlProtocol"

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
        *checksum = *checksum + packet[i];
    }

    *checksum = (uint16_t)(~(*checksum));

    Log(eLogDebug, CMP_NAME, "writeChecksum: calulcated: %#04x", *checksum);

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

eStatus CheckAllValuesAscii(const AllValuesAscii * const packet)
{
    uint8_t     tmp_u8;
    uint16_t    tmp_u16;

    eStatus retVal = BcdToVal(packet->cEnd, &tmp_u16);
    if (eOK == retVal) retVal = BcdToVal(packet->cAcc, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->cDec, &tmp_u16);
    if (eOK == retVal) retVal = BcdToVal(packet->cTurn, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->gHome, &tmp_u16);
    if (eOK == retVal) retVal = BcdToVal(packet->gEnd, &tmp_u16);
    if (eOK == retVal) retVal = BcdToVal(packet->gTurn1, &tmp_u16);
    if (eOK == retVal) retVal = BcdToVal(packet->gTurn2, &tmp_u16);
    if (eOK == retVal) retVal = BcdToVal(packet->gMaxAcc, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->gMaxDec, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->gFMax, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->gFMin, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->gMaxTime, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->gMaxLaps, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->gServSpeed, &tmp_u8);

    return retVal;     
}

eStatus CheckPacketAllValuesAscii(const PacketAllValuesAscii * const packet)
{
    eStatus retVal = eOK;

    retVal = checkStartByte((const uint8_t * const)packet);
    
    if (eOK != retVal)
    {
        Log(eLogWarn, CMP_NAME, "CheckPacketAllValues: error in start byte!");
    }

    if (eOK == retVal)
    {
        retVal = checkChecksum((const uint8_t * const)packet, sizeof(PacketAllValuesAscii));

        if (eOK != retVal)
        {
            Log(eLogWarn, CMP_NAME, "CheckPacketAllValues: Checksum error!");
        }
    }

    if (eOK == retVal)
    {
        retVal = CheckAllValuesAscii(&packet->values);

        if (eOK != retVal)
        {
            Log(eLogWarn, CMP_NAME, "CheckPacketAllValues: Invalid values detected!");
        }
    }

    return retVal;
}

eStatus AllValuesToPacketAllValuesAscii(const AllValues * const values, PacketAllValuesAscii * const packet)
{
    eStatus retVal = eOK;

    if (eOK == retVal) retVal = ValToBcd(values->cEnd,      packet->values.cEnd);
    if (eOK == retVal) retVal = ValToBcd(values->cAcc,      packet->values.cAcc);
    if (eOK == retVal) retVal = ValToBcd(values->cDec,      packet->values.cDec);
    if (eOK == retVal) retVal = ValToBcd(values->cTurn,     packet->values.cTurn);
    if (eOK == retVal) retVal = ValToBcd(values->gHome,     packet->values.gHome);
    if (eOK == retVal) retVal = ValToBcd(values->gEnd,      packet->values.gEnd);
    if (eOK == retVal) retVal = ValToBcd(values->gTurn1,    packet->values.gTurn1);
    if (eOK == retVal) retVal = ValToBcd(values->gTurn2,    packet->values.gTurn2);
    if (eOK == retVal) retVal = ValToBcd(values->gMaxAcc,   packet->values.gMaxAcc);
    if (eOK == retVal) retVal = ValToBcd(values->gMaxDec,   packet->values.gMaxDec);
    if (eOK == retVal) retVal = ValToBcd(values->gFMax,     packet->values.gFMax);
    if (eOK == retVal) retVal = ValToBcd(values->gFMin,     packet->values.gFMin);
    if (eOK == retVal) retVal = ValToBcd(values->gMaxTime,  packet->values.gMaxTime);
    if (eOK == retVal) retVal = ValToBcd(values->gMaxLaps,  packet->values.gMaxLaps);
    if (eOK == retVal) retVal = ValToBcd(values->gServSpeed,packet->values.gServSpeed);
    
    if (eOK == retVal) 
    {
        setStartStop((uint8_t * const)packet, sizeof(PacketAllValuesAscii));
        writeChecksum((uint8_t * const)packet, sizeof(PacketAllValuesAscii));
    }

    return retVal;
}

eStatus PacketAllValuesAsciiToAllValues(const PacketAllValuesAscii * const packet, AllValues * const values)
{
    eStatus retVal = eOK;

    retVal = CheckPacketAllValuesAscii(packet);

    if (eOK == retVal)
    {
        BcdToVal(packet->values.cEnd,       &values->cEnd); 
        BcdToVal(packet->values.cAcc,       &values->cAcc);
        BcdToVal(packet->values.cDec,       &values->cDec);
        BcdToVal(packet->values.cTurn,      &values->cTurn);
        BcdToVal(packet->values.gHome,      &values->gHome);
        BcdToVal(packet->values.gEnd,       &values->gEnd);
        BcdToVal(packet->values.gTurn1,     &values->gTurn1);
        BcdToVal(packet->values.gTurn2,     &values->gTurn2);
        BcdToVal(packet->values.gMaxAcc,    &values->gMaxAcc);
        BcdToVal(packet->values.gMaxDec,    &values->gMaxDec);
        BcdToVal(packet->values.gFMax,      &values->gFMax);
        BcdToVal(packet->values.gFMin,      &values->gFMin);
        BcdToVal(packet->values.gMaxTime,   &values->gMaxTime);
        BcdToVal(packet->values.gMaxLaps,   &values->gMaxLaps);
        BcdToVal(packet->values.gServSpeed, &values->gServSpeed);
    }

    return retVal;
}

