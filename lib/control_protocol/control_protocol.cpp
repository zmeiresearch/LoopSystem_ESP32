/*==============================================================================
   LoopSystem ESP32
   
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
#define TMP_BUFFER_SIZE 16

//==============================================================================
//  Local types
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================
static unsigned char tmp[TMP_BUFFER_SIZE];


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

uint16_t ExtractChecksum(const unsigned char * packet, const int packetLength)
{
    memset(&tmp, 0, TMP_BUFFER_SIZE);
    memcpy(&tmp, &packet[packetLength-3], 4);   // 4 bytes checksum + stop/null
    uint16_t fromPacket = strtoul((const char *)&tmp, NULL, 16);
    
    Log(eLogDebug, CMP_NAME, "ExtractChecksum: got: 0x%04x", fromPacket);

    return fromPacket;
}

uint16_t CalculateChecksum(const unsigned char * packet, const int packetLength)
{
    uint16_t calculated = 0;

    for (size_t i = 0; i < packetLength - 5; i++) 
    {
        calculated = calculated + (uint8_t)packet[i];
    }

    calculated = (uint16_t)~calculated;

    Log(eLogDebug, CMP_NAME, "CalculateChecksum: calculated: 0x%04x", calculated);

    return calculated;
}
/*
static void writeChecksum(const unsigned char * packet, const int packetLength)
{
    uint16_t * checksum = (uint16_t *)&packet[packetLength - 3];

    for (size_t i = 0; i < packetLength - 3; i++)
    {
        *checksum = *checksum + packet[i];
    }

    *checksum = (uint16_t)(~(*checksum));

    snprintf((char *)&tmp, TMP_BUFFER_SIZE, "%04x", checksum);
    Log(eLogDebug, CMP_NAME, "writeChecksum: calulcated: %s", tmp);
    
    memcpy((void *)&packet[packetLength-5], tmp, 4);    // 4 charactes checksum, just before the end of the packet

}

static void setStartStop(uint8_t * const packet, const size_t packetLength)
{
    packet[0] = START_BYTE;
    packet[packetLength - 1] = STOP_BYTE;
}
*/



//==============================================================================
//  Exported functions
//==============================================================================

void DumpModeValues(const ModeValues * const values)
{
    Log(eLogInfo, CMP_NAME, "End:%d",           values->end);
    Log(eLogInfo, CMP_NAME, "Acc:%d",           values->acc);
    Log(eLogInfo, CMP_NAME, "Dec:%d",           values->dec);
    Log(eLogInfo, CMP_NAME, "Turn:%d",          values->turn);
}

void DumpGlobalValues(const GlobalValues * const values)
{
    Log(eLogInfo, CMP_NAME, "gHome:%d",         values->home);
    Log(eLogInfo, CMP_NAME, "gEnd:%d",          values->end);
    Log(eLogInfo, CMP_NAME, "gTurn1:%d",        values->turn1);
    Log(eLogInfo, CMP_NAME, "gTurn2:%d",        values->turn2);
    Log(eLogInfo, CMP_NAME, "gMaxAcc:%d",       values->maxAcc);
    Log(eLogInfo, CMP_NAME, "gMaxDec:%d",       values->maxDec);
    Log(eLogInfo, CMP_NAME, "gFMax:%d",         values->maxSpeed);
    Log(eLogInfo, CMP_NAME, "gFMin:%d",         values->homingSpeed);
    Log(eLogInfo, CMP_NAME, "gMaxTime:%d",      values->maxTime);
    Log(eLogInfo, CMP_NAME, "gMaxLaps:%d",      values->maxLaps);
    Log(eLogInfo, CMP_NAME, "gServSpeed:%d",    values->servSpeed);
}

eStatus IsBcd(const char val)
{
    if (val < '0' || val > '9') return eFAIL;

    return eOK;
}

eStatus IsHex(const char val)
{
    if (eOK == IsBcd(val)) return eOK;
    if ( ('a' <= val ) && (val <= 'f')) return eOK;
    if ( ('A' <= val ) && (val <= 'F')) return eOK;
    return eFAIL;
}


/*eStatus CheckModeValuesAscii(const ModeValuesAscii * const packet)
{
    uint8_t     tmp_u8;
    uint16_t    tmp_u16;

    eStatus retVal = eOK;

    if (eOK == retVal) retVal = BcdToVal(packet->end, &tmp_u16);
    if (eOK == retVal) retVal = BcdToVal(packet->acc, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->dec, &tmp_u16);
    if (eOK == retVal) retVal = BcdToVal(packet->turn, &tmp_u8);

    return retVal;
}

eStatus CheckGlobalValuesAscii(const AllValuesAscii * const packet)
{
    uint8_t     tmp_u8;
    uint16_t    tmp_u16;

    eStatus retVal = eOK;
    if (eOK == retVal) retVal = BcdToVal(packet->home, &tmp_u16);
    if (eOK == retVal) retVal = BcdToVal(packet->end, &tmp_u16);
    if (eOK == retVal) retVal = BcdToVal(packet->turn1, &tmp_u16);
    if (eOK == retVal) retVal = BcdToVal(packet->turn2, &tmp_u16);
    if (eOK == retVal) retVal = BcdToVal(packet->maxAcc, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->maxDec, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->fMax, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->fMin, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->maxTime, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->maxLaps, &tmp_u8);
    if (eOK == retVal) retVal = BcdToVal(packet->servSpeed, &tmp_u8);

    return retVal;     

}

eStatus CheckPacketModeValuesAscii(const PacketModeValuesAscii * const packet)
{
    eStatus retVal = eOK;

    retVal = checkStartByte((const uint8_t * const)packet);
    
    if (eOK != retVal)
    {
        Log(eLogWarn, CMP_NAME, "CheckPacketModeValuesAscii: error in start byte!");
    }

    if (eOK == retVal)
    {
        retVal = checkChecksum((const uint8_t * const)packet, sizeof(PacketAllValuesAscii));

        if (eOK != retVal)
        {
            Log(eLogWarn, CMP_NAME, "CheckPacketModeValuesAscii: Checksum error!");
        }
    }

    if (eOK == retVal)
    {
        retVal = CheckAllValuesAscii(&packet->values);

        if (eOK != retVal)
        {
            Log(eLogWarn, CMP_NAME, "CheckPacketModeValuesAscii: Invalid values detected!");
        }
    }

    return retVal;
}


eStatus CheckPacketGlobalValuesAscii(const PacketAllValuesAscii * const packet)
{
}
*/

/*eStatus ModeValuesToPacketModeValuesAscii(ePacketType const type, const ModeValues * const values, PacketModeValuesAscii * const packet)
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
        Log(eLogInfo, CMP_NAME, "PacketAllValuesAsciiToAllValues: got good packet, processing!");
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

        DumpAllValues(values);
    }

    return retVal;
}*/

