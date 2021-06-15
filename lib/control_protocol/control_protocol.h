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
#define START_BYTE              0x2a        // '*'
#define STOP_BYTE               0x0d        // Carriage return

#define REQUEST_GLOBAL_VALUES   "#0\r"

// Change if bigger packet appears
#define SERIAL_BUFFER_SIZE      sizeof(PacketGlobalValuesAscii) + 0x1

#define MODE_COUNT              4

//==============================================================================
//  Exported types
//==============================================================================

typedef enum _PacketType {
    ePacketGlobalValues = 0x30,
    ePacketModeNovice = 0x31,
    ePacketModeExpert = 0x32,
    ePacketModeAdvanced = 0x33,
    ePacketModeMaster = 0x34,
    ePacketCount = 0x35
} PacketType;

typedef struct _ModeValues {
    uint32_t            end;
    uint32_t            acc;
    uint32_t            dec;
    uint32_t            turn;
} ModeValues;

typedef struct _GlobalValues {
    uint32_t            home;
    uint32_t            end;
    uint32_t            turn1;
    uint32_t            turn2;
    uint32_t            maxAcc;
    uint32_t            maxDec;
    uint32_t            maxSpeed;
    uint32_t            homingSpeed;
    uint32_t            maxTime;
    uint32_t            maxLaps;
    uint32_t            servSpeed;
} GlobalValues;

#pragma pack(push, 1)

typedef struct _ModeValuesAscii {
    unsigned char       end[10];
    unsigned char       acc[10];
    unsigned char       dec[10];
    unsigned char       turn[10];
} ModeValuesAscii;

typedef struct GlobalValuesAscii {
    unsigned char       home[10];
    unsigned char       end[10];
    unsigned char       turn1[10];
    unsigned char       turn2[10];
    unsigned char       maxAcc[5];
    unsigned char       maxDec[5];
    unsigned char       maxSpeed[5];
    unsigned char       homingSpeed[5];
    unsigned char       maxTime[5];
    unsigned char       maxLaps[5];
    unsigned char       servSpeed[5];
}  GlobalValuesAscii;

typedef struct _PacketModeValuesAscii {
    unsigned char       startByte;
    unsigned char       packetType;
    ModeValuesAscii     values;
    unsigned char       checksum[4];
    unsigned char       stopByte;
} PacketModeValuesAscii;

typedef struct _PacketGlobalValuesAscii {
    unsigned char       startByte;
    unsigned char       packetType;
    GlobalValuesAscii   values;
    unsigned char       checksum[4];
    unsigned char       stopByte;
} PacketGlobalValuesAscii;


#pragma pack(pop)


//==============================================================================
//  Exported data
//==============================================================================

//==============================================================================
//  Exported functions
//==============================================================================
//eStatus BcdToVal(const unsigned char * const bcd, uint8_t * const outVal);
//eStatus BcdToVal(const unsigned char * const bcd, uint16_t * const outVal);
//eStatus ValToBcd(const uint8_t val, unsigned char * const outBuf);
//eStatus ValToBcd(const uint16_t val, unsigned char * const outBuf);

eStatus IsBcd(const char val);
eStatus IsHex(const char val);

uint16_t ExtractChecksum(const unsigned char * packet, const int packetLength);
uint16_t CalculateChecksum(const unsigned char * packet, const int packetLength);


//eStatus CheckModeValuesAscii(const AllValuesAscii * const packet);
//eStatus CheckGlobalValuesAscii(const AllValuesAscii * const packet);
//eStatus CheckPacketModeValuesAscii(const PacketAllValuesAscii * const packet);
//eStatus CheckPacketGlobalValuesAscii(const PacketAllValuesAscii * const packet);

//eStatus ModeValuesToPacketAllValuesAscii(const AllValues * const values, PacketAllValuesAscii * const packet);
//eStatus GlobalValuesToPacketAllValuesAscii(const AllValues * const values, PacketAllValuesAscii * const packet);
//eStatus PacketModeValuesAsciiToAllValues(const PacketAllValuesAscii * const packet, AllValues * const values);
//eStatus PacketGlobalValuesAsciiToAllValues(const PacketAllValuesAscii * const packet, AllValues * const values);

#endif // INC_CONTROL_PROTOCOL_H
