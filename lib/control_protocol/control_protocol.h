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

// Change if bigger packet appears
#define SERIAL_BUFFER_SIZE  sizeof(PacketGlobalValuesAscii) + 0x1

//==============================================================================
//  Exported types
//==============================================================================

typedef enum _PacketType {
    ePacketGlobalValues = 0x41,
    ePacketModeNovice = 0x42,
    ePacketModeExpert = 0x43,
    ePacketModeAdvanced = 0x44,
    ePacketModeMaster = 0x45,
    ePacketCount = 0x46
} PacketType;

typedef struct _ModeValues {
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
    unsigned char       startByte;
    unsigned char       packetType;
    ModeValuesAscii     values;
    unsigned char       checksum[4];
    unsigned char       stopByte;
} PacketAllValuesAscii;

typedef struct _PacketGlobalValuesAscii {
    unsigned char       startByte;
    unsigned char       packetType;
    GlobalValuesAscii   values;
    unsigned char       checksum[4];
    unsigned char       stopByte;
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

eStatus CheckModeValuesAscii(const AllValuesAscii * const packet);
eStatus CheckGlobalValuesAscii(const AllValuesAscii * const packet);
eStatus CheckPacketModeValuesAscii(const PacketAllValuesAscii * const packet);
eStatus CheckPacketGlobalValuesAscii(const PacketAllValuesAscii * const packet);

eStatus ModeValuesToPacketAllValuesAscii(const AllValues * const values, PacketAllValuesAscii * const packet);
eStatus GlobalValuesToPacketAllValuesAscii(const AllValues * const values, PacketAllValuesAscii * const packet);
eStatus PacketModeValuesAsciiToAllValues(const PacketAllValuesAscii * const packet, AllValues * const values);
eStatus PacketGlobalValuesAsciiToAllValues(const PacketAllValuesAscii * const packet, AllValues * const values);

#endif // INC_CONTROL_PROTOCOL_H
