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
#define START_BYTE      '*'
#define STOP_BYTE        '\n'


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
    unsigned char       cEnd[3];
    unsigned char       cAcc[2];
    unsigned char       cDec[2];
    unsigned char       cTurn[2];
    unsigned char       gHome[3];
    unsigned char       gEnd[3];
    unsigned char       gTurn1[3];
    unsigned char       gTurn2[3];
    unsigned char       gMaxAcc[2];
    unsigned char       gMaxDec[2];
    unsigned char       gFMax[2];
    unsigned char       gFMin[2];
    unsigned char       gMaxTime[2];
    unsigned char       gMaxLaps[2];
    unsigned char       gServSpeed[2];
}  AllValuesAscii;

typedef struct _PacketAllValuesAscii {
    uint8_t             startByte;
    uint8_t             packetType;
    AllValuesAscii      values;
    uint8_t             checksum[2];
    uint8_t             stopByte;
} PacketAllValuesAscii;

#pragma pack(pop)


//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================
eStatus BcdToVal(const unsigned char * const bcd, uint8_t * const outVal);
eStatus BcdToVal(const unsigned char * const bcd, uint16_t * const outVal);
eStatus ValToBcd(const uint8_t val, unsigned char * const outBuf);
eStatus ValToBcd(const uint16_t val, unsigned char * const outBuf);

eStatus CheckAllValuesAscii(const AllValuesAscii * const packet);
eStatus CheckPacketAllValuesAscii(const PacketAllValuesAscii * const packet);

eStatus AllValuesToPacketAllValuesAscii(const AllValues * const values, PacketAllValuesAscii * const packet);
eStatus PacketAllValuesAsciiToAllValues(const PacketAllValuesAscii * const packet, AllValues * const values);

#endif // INC_CONTROL_PROTOCOL_H
