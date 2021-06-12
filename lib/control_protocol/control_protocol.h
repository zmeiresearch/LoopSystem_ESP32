/*==============================================================================
   LoopSystem ESP32
   
  ============================================================================*/


//==============================================================================
//  Multi-include guard
//==============================================================================
#ifndef INC_CONTROL_PROTOCOL_H
#define INC_CONTROL_PROTOCOL_H

//==============================================================================
//  Includes
//==============================================================================
#include "globals.h"

//==============================================================================
//  Defines
//==============================================================================

//==============================================================================
//  Exported types
//==============================================================================

typedef struct _AllValues {
    uint16_t            cEnd;
    uint8_t             cAcc;
    uint8_t             cDec;
    uint8_t             cTurn;
    uint16_t            gHome;
    uint16_t            gEnd;
    uint16_t            gTurn1;
    uint16_t            gTurn2;
    uint8_t             gMaxAcc;
    uint8_t             gMaxDec;
    uint8_t             gFMax;
    uint8_t             gFMin;
    uint8_t             gMaxTime;
    uint8_t             gMaxLaps;
    uint8_t             gServSpeed;
}  AllValues;

#pragma pack(push, 1)

typedef struct _AllValuesAscii {
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
}  AllValuesAscii;

typedef struct _PacketAllValues {
    uint8_t             startByte;
    uint8_t             packetType;
    AllValuesAscii      values;
    uint8_t             checksum[2];
    uint8_t             stopByte;
} PacketAllValues;

#pragma pack(pop)


//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================
eStatus CheckPacketAllValues(const PacketAllValues * const packet);
eStatus BcdToVal(const char * const bcd, uint8_t * const outVal);
eStatus BcdToVal(const char * const bcd, uint16_t * const outVal);
eStatus ValToBcd(const uint8_t val, char * const outBuf);
eStatus ValToBcd(const uint16_t val, char * const outBuf);


#endif // INC_CONTROL_PROTOCOL_H
