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
#define START_BYTE          0x2a
#define STOP_BYTE           0x0d

#define SPECIAL_CHECKSUM    0xdead

//==============================================================================
//  Exported types
//==============================================================================

typedef struct _ModeValues {
    uint8_t             mode;
    uint16_t            end;
    uint8_t             acc;
    uint8_t             dec;
    uint8_t             turn;
} ModeValues;

typedef struct _GlobalValues {
    uint16_t            home;
    uint16_t            end;
    uint16_t            turn1;
    uint16_t            turn2;
    uint8_t             maxAcc;
    uint8_t             maxDec;
    uint8_t             fMax;
    uint8_t             fMin;
    uint8_t             maxTime;
    uint8_t             maxLaps;
    uint8_t             servSpeed;
} GlobalValues;

#pragma pack(push, 1)

typedef struct _ModeValuesAscii {
    unsigned char       mode;
    unsigned char       end[3];
    unsigned char       acc[2];
    unsigned char       dec[2];
    unsigned char       turn[2];
} ModeValuesAscii;

typedef struct GlobalValuesAscii {
    unsigned char       home[3];
    unsigned char       end[3];
    unsigned char       turn1[3];
    unsigned char       turn2[3];
    unsigned char       maxAcc[2];
    unsigned char       maxDec[2];
    unsigned char       fMax[2];
    unsigned char       fMin[2];
    unsigned char       maxTime[2];
    unsigned char       maxLaps[2];
    unsigned char       servSpeed[2];
}  GlobalValuesAscii;

typedef struct _PacketModeValuesAscii {
    uint8_t             startByte;
    uint8_t             packetType;
    ModeValuesAscii     values;
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
