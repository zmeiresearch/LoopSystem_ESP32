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
#define START_BYTE_READ         0x2a        // '*'
#define START_BYTE_WRITE        0x25        // %
#define STOP_BYTE               0x0d        // Carriage return

#define REQUEST_GLOBAL_VALUES   "#1\r"
#define REQUEST_MODE_NOVICE     "#2\r"
#define REQUEST_MODE_EXPERT     "#3\r"
#define REQUEST_MODE_ADVANCED   "#4\r"
#define REQUEST_MODE_MASTER     "#5\r"



// Change if bigger packet appears
#define SERIAL_BUFFER_SIZE      sizeof(PacketGlobalValuesAscii) + 0x1

#define MODE_COUNT              4

//==============================================================================
//  Exported types
//==============================================================================

typedef enum _PacketType {
    ePacketStatus = 0x30,
    ePacketGlobalValues = 0x31,
    ePacketMode = 0x32,
    ePacketCount = 0x36
} PacketType;

typedef struct _ModeValues {
    uint32_t            turn1;
    uint32_t            acc;
    uint32_t            dec;
    uint32_t            turn2;
} ModeValues;

typedef struct _GlobalValues {
    uint32_t            home;
    uint32_t            maxEnd;
    uint32_t            maxTurn1;
    uint32_t            minTurn2;
    uint32_t            maxAcc;
    uint32_t            maxDec;
    uint32_t            maxSpeed;
    uint32_t            homingSpeed;
    uint32_t            maxTime;
    uint32_t            maxLaps;
    uint32_t            servSpeed;
} GlobalValues;

typedef struct _CurrentStatus {
    uint8_t             mode;
    uint32_t            completedLaps;
    uint32_t            position;
    uint32_t            systemStatus;
} CurrentStatus;

#pragma pack(push, 1)

typedef struct _ModeValuesAscii {
    unsigned char       mode;
    unsigned char       turn1[10];
    unsigned char       acc[10];
    unsigned char       dec[10];
    unsigned char       turn2[10];
} ModeValuesAscii;

typedef struct _GlobalValuesAscii {
    unsigned char       home[10];
    unsigned char       maxEnd[10];
    unsigned char       maxTurn1[10];
    unsigned char       minTurn2[10];
    unsigned char       maxAcc[5];
    unsigned char       maxDec[5];
    unsigned char       maxSpeed[5];
    unsigned char       homingSpeed[5];
    unsigned char       maxTime[5];
    unsigned char       maxLaps[5];
    unsigned char       servSpeed[5];
}  GlobalValuesAscii;

typedef struct _StatusAscii {
    unsigned char       mode;
    unsigned char       completedLaps[5];
    unsigned char       position[10];
    unsigned char       systemStatus;
} StatusAscii;

typedef struct _PacketModeValuesAscii {
    unsigned char       startByte;
    unsigned char       packetType;
    ModeValuesAscii     values;
    unsigned char       checksum[4];
    unsigned char       stopByte;
} PacketModeValuesAscii;

typedef struct _PacketStatusAscii {
    unsigned char       startByte;
    unsigned char       packetType;
    StatusAscii         status;
    unsigned char       checksum[4];
    unsigned char       stopByte;
} PacketStatusAscii;

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
void DumpModeValues(const ModeValues * const values);
void DumpGlobalValues(const GlobalValues * const values);
void DumpStatus( const CurrentStatus * const status);

//eStatus BcdToVal(const unsigned char * const bcd, uint8_t * const outVal);
//eStatus BcdToVal(const unsigned char * const bcd, uint16_t * const outVal);
//eStatus ValToBcd(const uint8_t val, unsigned char * const outBuf);
//eStatus ValToBcd(const uint16_t val, unsigned char * const outBuf);

eStatus IsBcd(const char val);
eStatus IsHex(const char val);

uint16_t ExtractChecksum(const unsigned char * packet, const int packetLength);
uint16_t CalculateChecksum(const unsigned char * packet, const int packetLength);

uint32_t TenByteBcdToUint32(const char * const data);
uint32_t FiveByteBcdToUint32(const char * const data);


//eStatus CheckModeValuesAscii(const AllValuesAscii * const packet);
//eStatus CheckGlobalValuesAscii(const AllValuesAscii * const packet);
//eStatus CheckPacketModeValuesAscii(const PacketAllValuesAscii * const packet);
//eStatus CheckPacketGlobalValuesAscii(const PacketAllValuesAscii * const packet);

//eStatus ModeValuesToPacketAllValuesAscii(const AllValues * const values, PacketAllValuesAscii * const packet);
//eStatus GlobalValuesToPacketAllValuesAscii(const AllValues * const values, PacketAllValuesAscii * const packet);
//eStatus PacketModeValuesAsciiToAllValues(const PacketAllValuesAscii * const packet, AllValues * const values);
//eStatus PacketGlobalValuesAsciiToAllValues(const PacketAllValuesAscii * const packet, AllValues * const values);

#endif // INC_CONTROL_PROTOCOL_H
