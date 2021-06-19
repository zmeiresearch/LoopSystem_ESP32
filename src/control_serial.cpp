/*==============================================================================
   LoopSystem ESP32
   
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <HardwareSerial.h>
#include "driver/uart.h"    // for UART_PIN_NO_CHANGE

#include <logger.h>
#include <control_protocol.h>

#include "config.h"
#include "control_serial.h"

#include "values.h"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME    "ControlSerial"

//==============================================================================
//  Local types
//==============================================================================
typedef enum _ReceiveStatus
{
    eReceiveStatusIdle,
    eReceiveStatusStartReceived,
    eReceiveStatusTypeReceived,
    eReceiveStatusValuesReceived,
    eReceiveStatusChecksumReceived,
    eReceiveStatusError,
    eReceiveStatusCount
} ReceiveStatus;

typedef enum _SendStatus
{
    eSendStatusIdle,
    eSendStatusQueued,
    eSendStatusInProgress,
    eSendStatusComplete,
    eSendStatusError
} SendStatus;

//==============================================================================
//  Local data
//==============================================================================
static HardwareSerial           serial(CONTROL_SERIAL_PORT);

static unsigned char            sendBuffer[SERIAL_BUFFER_SIZE];
static size_t                   sendCount;
static SendStatus               sendStatus = eSendStatusIdle;

static unsigned char            receiveBuffer[SERIAL_BUFFER_SIZE];
static size_t                   receiveIndex;
static ReceiveStatus            receiveStatus = eReceiveStatusIdle;
static PacketType               receivedPacketType = ePacketCount;
//==============================================================================
//  Local functions
//==============================================================================
static eStatus queueForTransmit(const char * const buffer, size_t size);

//==============================================================================
//  Exported functions
//==============================================================================

eStatus ControlSerialInit(void * param)
{
    Log(eLogInfo, CMP_NAME, "ControlSerialInit");

    serial.begin(CONTROL_SERIAL_BAUD, CONTROL_SERIAL_FORMAT, CONTROL_SERIAL_RX_PIN, CONTROL_SERIAL_TX_PIN); 

    return eOK;
}

size_t getValuesCount(PacketType packetType)
{
    size_t expectedValueCount;
    switch (packetType)
    {
        case ePacketGlobalValues:
            expectedValueCount = sizeof(GlobalValuesAscii);
            break;
        case ePacketModeValues:
            expectedValueCount = sizeof(ModeValuesAscii);
            break;
        case ePacketStatus:
            expectedValueCount = sizeof(StatusAscii);
            break;
        default:
            expectedValueCount = 0;
            break;
    }

    return expectedValueCount;
}

void ParsePacketGlobalValueAscii(PacketGlobalValuesAscii const * const packet)
{
    Log(eLogInfo, CMP_NAME, "ParsePacketGlobalValueAscii");
    gGlobalValues.home = TenByteBcdToUint32((const char *)&packet->values.home[0]);
    gGlobalValues.maxEnd = TenByteBcdToUint32((const char *)&packet->values.maxEnd[0]);
    gGlobalValues.maxTurn1 = TenByteBcdToUint32((const char *)&packet->values.maxTurn1[0]);
    gGlobalValues.minTurn2 = TenByteBcdToUint32((const char *)&packet->values.minTurn2[0]);
    gGlobalValues.maxAcc = FiveByteBcdToUint32((const char *)&packet->values.maxAcc[0]);
    gGlobalValues.maxDec = FiveByteBcdToUint32((const char *)&packet->values.maxDec[0]);
    gGlobalValues.maxSpeed = FiveByteBcdToUint32((const char *)&packet->values.maxSpeed[0]);
    gGlobalValues.homingSpeed = FiveByteBcdToUint32((const char *)&packet->values.homingSpeed[0]);
    gGlobalValues.maxTime = FiveByteBcdToUint32((const char *)&packet->values.maxTime[0]);
    gGlobalValues.maxLaps = FiveByteBcdToUint32((const char *)&packet->values.maxLaps[0]);
    gGlobalValues.servSpeed = FiveByteBcdToUint32((const char *)&packet->values.servSpeed[0]);

    DumpGlobalValues(&gGlobalValues);
}

void SendPacketGlobalValuesAscii(void)
{
    PacketGlobalValuesAscii packet;

    packet.startByte = START_BYTE_WRITE;
    packet.packetType = ePacketGlobalValues;

    // All lines print out a null terminator, but it gets overwritten by the next one
    snprintf((char *)&packet.values.home[0], 11, "%010u", gGlobalValues.home);
    snprintf((char *)&packet.values.maxEnd[0], 11, "%010u", gGlobalValues.maxEnd);
    snprintf((char *)&packet.values.maxTurn1[0], 11, "%010u", gGlobalValues.maxTurn1);
    snprintf((char *)&packet.values.minTurn2[0], 11, "%010u", gGlobalValues.minTurn2);
    snprintf((char *)&packet.values.maxAcc[0], 6, "%05u", gGlobalValues.maxAcc);
    snprintf((char *)&packet.values.maxDec[0], 6, "%05u", gGlobalValues.maxDec);
    snprintf((char *)&packet.values.maxSpeed[0], 6, "%05u", gGlobalValues.maxSpeed);
    snprintf((char *)&packet.values.homingSpeed[0], 6, "%05u", gGlobalValues.homingSpeed);
    snprintf((char *)&packet.values.maxTime[0], 6, "%05u", gGlobalValues.maxTime);
    snprintf((char *)&packet.values.maxLaps[0], 6, "%05u", gGlobalValues.maxLaps);
    snprintf((char *)&packet.values.servSpeed[0], 6, "%05u", gGlobalValues.servSpeed);

    uint16_t checksum = CalculateChecksum((const unsigned char *)&packet, sizeof(PacketGlobalValuesAscii));

    snprintf((char*)&packet.checksum[0], 5, "%04X", checksum);
    packet.stopByte = STOP_BYTE;

    while (eOK != queueForTransmit((const char *)&packet, sizeof(PacketGlobalValuesAscii)))
    {
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void SendPacketModeValuesAscii(Modes mode)
{
    PacketModeValuesAscii packet;

    packet.startByte = START_BYTE_WRITE;
    packet.packetType = ePacketModeValues;

    // All lines print out a null terminator, but it gets overwritten by the next one
    snprintf((char *)&packet.values.mode, 2, "%01u", mode);
    snprintf((char *)&packet.values.speed[0], 6, "%05u", gModeValues[mode].speed);
    snprintf((char *)&packet.values.turn1[0], 11, "%010u", gModeValues[mode].turn1);
    snprintf((char *)&packet.values.turn2[0], 11, "%010u", gModeValues[mode].turn2);
    snprintf((char *)&packet.values.brakeTime[0], 6, "%05u", gModeValues[mode].brakeTime);
    snprintf((char *)&packet.values.acc[0], 6, "%05u", gModeValues[mode].acc);
    snprintf((char *)&packet.values.dec[0], 6, "%05u", gModeValues[mode].dec);

    uint16_t checksum = CalculateChecksum((const unsigned char *)&packet, sizeof(PacketModeValuesAscii));

    snprintf((char*)&packet.checksum[0], 5, "%04X", checksum);
    packet.stopByte = STOP_BYTE;

    Log(eLogDebug, CMP_NAME, "SendPacketModeValuesAscii: %s", &packet);

    while (eOK != queueForTransmit((const char *)&packet, sizeof(PacketModeValuesAscii)))
    {
        vTaskDelay(100/portTICK_PERIOD_MS);
    }

    vTaskDelay(500/portTICK_PERIOD_MS);

    while (eOK !=  queueForTransmit(REQUEST_MODE_NOVICE, 3))
    {
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
}

void        SendPacketRequestModeValues(Modes mode)
{
    switch (mode)
    {
        case eModeNovice:
            while (eOK !=  queueForTransmit(REQUEST_MODE_NOVICE, 3))
            {
                vTaskDelay(100/portTICK_PERIOD_MS);
            }
            break;
        case eModeAdvanced:
            while (eOK !=  queueForTransmit(REQUEST_MODE_ADVANCED, 3))
            {
                vTaskDelay(100/portTICK_PERIOD_MS);
            }
            break;
        
        case eModeExpert:
            while (eOK !=  queueForTransmit(REQUEST_MODE_EXPERT, 3))
            {
                vTaskDelay(100/portTICK_PERIOD_MS);
            }
            break;
        
        case eModeMaster:
            while (eOK  !=  queueForTransmit(REQUEST_MODE_MASTER, 3))
            {
                vTaskDelay(100/portTICK_PERIOD_MS);
            }
            break;
        
        default:
            Log(eLogWarn, CMP_NAME, "SendPacketRequestModeValues: invalid mode 0x%02x", mode);
            break;
    }

    

}

void ParsePacketModeValuesAscii(PacketModeValuesAscii const * const modePacket)
{
    uint8_t mode = (uint8_t)modePacket->values.mode - 0x30;

    if ((eModeNovice <= mode) && (mode <= eModeMaster))
    {
        gModeValues[mode].speed = FiveByteBcdToUint32((const char *)&modePacket->values.speed[0]);
        gModeValues[mode].turn1 = TenByteBcdToUint32((const char *)&modePacket->values.turn1[0]);
        gModeValues[mode].turn2 = TenByteBcdToUint32((const char *)&modePacket->values.turn2[0]);
        gModeValues[mode].brakeTime = FiveByteBcdToUint32((const char *)&modePacket->values.brakeTime[0]);
        gModeValues[mode].acc = FiveByteBcdToUint32((const char *)&modePacket->values.acc[0]);
        gModeValues[mode].dec = FiveByteBcdToUint32((const char *)&modePacket->values.dec[0]);
        Log(eLogDebug, CMP_NAME, "ParsePacketModeValuesAscii: Updated mode 0x%02x", mode);
    }
    else 
    {
        Log(eLogWarn, CMP_NAME, "ParsePacketModeValuesAscii: invalid mode: 0x%02x", mode);
    }
}

void ParsePacketStatusAscii(PacketStatusAscii const * const statusPacket)
{
    Log(eLogDebug, CMP_NAME, "ParseStatus");

    uint8_t mode = (uint8_t)statusPacket->status.mode - 0x30;

    if ((eModeNovice <= mode) && (mode <= eModeMaster))
    {
        gStatus.mode = mode;
    }
    else 
    {
        Log(eLogWarn, CMP_NAME, "ParseStatus: invalid mode: 0x%02x", statusPacket->status.mode);
    }

    gStatus.completedLaps = FiveByteBcdToUint32((const char *)&statusPacket->status.completedLaps[0]);
    gStatus.position = TenByteBcdToUint32((const char *)&statusPacket->status.position[0]);
    gStatus.systemStatus = statusPacket->status.systemStatus;
}


void ProcessPacket(const unsigned char * const buffer, size_t bufferSize)
{
    Log(eLogDebug, CMP_NAME, "ProcessPacket: Processing: size: %d: %s", bufferSize, buffer);
    const PacketType packetType = (PacketType)buffer[1];

    const uint16_t receivedChecksum = ExtractChecksum(buffer, bufferSize);
    const uint16_t calculatedChecksum = CalculateChecksum(buffer, bufferSize);

    if (receivedChecksum == calculatedChecksum)
    {
        //Log(eLogInfo, CMP_NAME, "ProcessPacket: Good packet received, type: %02x", packetType);

        switch (packetType)
        {
            case ePacketGlobalValues:
                ParsePacketGlobalValueAscii((PacketGlobalValuesAscii const * const)buffer);
            case ePacketModeValues:
                ParsePacketModeValuesAscii((PacketModeValuesAscii const * const )buffer);
            case ePacketStatus:
                ParsePacketStatusAscii((PacketStatusAscii const * const )buffer);
                break;
            case ePacketCount:
            default:
                Log(eLogWarn, CMP_NAME, "ProcessPacket: Unexpected packet type: %02x", packetType);
                break;
        }
    }
    else
    {
        Log(eLogWarn, CMP_NAME, "ProcessPacket: bad checksum: received: 0x%04x, calculated: 0x%04x", receivedChecksum, calculatedChecksum);
    }
}

eStatus ControlSerialReceive()
{
    size_t expectedValueCount;
    // Process as much bytes as buffered
    while (serial.available())
    {
        unsigned char tmp = serial.read();
        //Log(eLogDebug, CMP_NAME, "CSL: %d:%d:%02x", receiveStatus, receiveIndex, tmp);

        switch (receiveStatus)
        {
            case eReceiveStatusIdle:
            case eReceiveStatusError:
                // Always wait for a start byte
                if (START_BYTE_READ == tmp)
                {
                    //Log(eLogInfo, CMP_NAME, "ControlSerialLoop: Start Byte Received");
                    receiveIndex = 0;
                    receiveBuffer[receiveIndex++] = tmp;
                    receiveStatus = eReceiveStatusStartReceived;
                }
                break;
            
            case eReceiveStatusStartReceived:
                // Next comes the packet type
                if (((unsigned char)ePacketStatus <= tmp) &&
                    (tmp < (unsigned char)ePacketCount))
                {
                    // valid packet type
                    //Log(eLogInfo, CMP_NAME, "ControlSerialLoop: Valid Packet Type: 0x%02x", tmp);
                    receiveBuffer[receiveIndex++] = tmp;
                    receivedPacketType = (PacketType)tmp;
                    receiveStatus = eReceiveStatusTypeReceived;
                }
                else 
                {
                    receiveStatus = eReceiveStatusError;
                    Log(eLogWarn, CMP_NAME, "ControlSerialLoop: Unknown packet: 0x%02x", tmp);

                }
                break;
            
            case eReceiveStatusTypeReceived:
                // Packet type received, wait for all values to arrive
                expectedValueCount = getValuesCount(receivedPacketType);
                if (eOK == IsBcd(tmp))
                {
                    receiveBuffer[receiveIndex++] = tmp;
                    // + 2 for the Start byte and the Packet Type byte
                    if (receiveIndex == expectedValueCount + 2)
                    {
                        receiveStatus = eReceiveStatusValuesReceived;
                        //Log(eLogInfo, CMP_NAME, "ControlSerialLoop: Values reception complete");
                    }
                }
                else
                {
                    // Unexpected non-BCD character
                    receiveStatus = eReceiveStatusError;
                    Log(eLogWarn, CMP_NAME, "ControlSerialLoop: Invalid character receiving values: 0x%02x", tmp);
                    receiveBuffer[receiveIndex++] = tmp;
                    receiveBuffer[receiveIndex] = 0;
                    Log(eLogWarn, CMP_NAME, "%s", receiveBuffer);
                }
                break;

            case eReceiveStatusValuesReceived:
                // All values received, wait for checksum
                expectedValueCount = getValuesCount(receivedPacketType);
                if (eOK == IsHex(tmp))
                {
                    receiveBuffer[receiveIndex++] = tmp;
                    // + 6 for the Start byte and the Packet Type byte and the checksum
                    if (receiveIndex == expectedValueCount + 6) 
                    {
                        receiveStatus = eReceiveStatusChecksumReceived;
                        //Log(eLogInfo, CMP_NAME, "ControlSerialLoop: Checksum reception complete");
                    }
                }
                else
                {
                    // Unexpected non-BCD character
                    receiveStatus = eReceiveStatusError;
                    Log(eLogWarn, CMP_NAME, "ControlSerialLoop: Invalid character receiving checksum: 0x%02x", tmp);
                    receiveBuffer[receiveIndex++] = tmp;
                    receiveBuffer[receiveIndex] = 0;
                    Log(eLogWarn, CMP_NAME, "%s", receiveBuffer);
                }
                break;

            case eReceiveStatusChecksumReceived:
                // Everything received, wait for stop byte
                if (STOP_BYTE == tmp) 
                {
                    receiveBuffer[receiveIndex] = 0; // NULL - terminate the buffer
                    Log(eLogInfo, CMP_NAME, "ControlSerialLoop: ProcessingPacket");
                    ProcessPacket(&receiveBuffer[0], receiveIndex);
                    receiveStatus = eReceiveStatusIdle;
                }
                else
                {
                    receiveStatus = eReceiveStatusError;
                    Log(eLogWarn, CMP_NAME, "ControlSerialLoop: Unexpected character 0x%02x while waiting for stop byte", tmp);
                }

                break;
        
            default:
                receiveStatus = eReceiveStatusError;
                break;
        }
    }

    return eOK; // Always return OK to continue looping
}

static eStatus queueForTransmit(const char * const buffer, size_t size)
{
    eStatus retVal = eOK;
    if ((sendStatus == eSendStatusIdle) ||
        (sendStatus == eSendStatusComplete))
    {
        if (size <= SERIAL_BUFFER_SIZE)
        {
            memcpy(sendBuffer, buffer, size);
            sendCount = size;
            sendStatus = eSendStatusQueued;
        }
        else
        {
            retVal = eOVERFLOW;
        }
    }
    else 
    {
        retVal = eBUSY;
    }
    return retVal;
}

static int toSend = 0;
eStatus     ControlRefreshTask()
{
    switch (toSend)
    {
        case 0:
            //if (eOK == queueForTransmit(REQUEST_GLOBAL_VALUES, 3))
            {
                toSend = 1;
            }
            break;
        case 1:
            //if (eOK == queueForTransmit(REQUEST_MODE_NOVICE, 3))
            {
                toSend = 2;
            }
            break;
        case 2:
            //if (eOK == queueForTransmit(REQUEST_MODE_EXPERT, 3))
            {
                toSend = 3;
            }
            break;
        case 3:
            //if (eOK == queueForTransmit(REQUEST_MODE_ADVANCED, 3))
            {
                toSend = 4;
            }
            break;
        case 4:
            //if (eOK == queueForTransmit(REQUEST_MODE_MASTER, 3))
            {
                toSend = 5;
            }
            break;
        default:
            toSend = 0;
            break;
    }

    return eOK; // Always return OK to continue looping

}

eStatus     ControlSerialTransmit()
{
    if (eSendStatusQueued == sendStatus)
    {
        sendStatus = eSendStatusInProgress;
        serial.write(&sendBuffer[0], sendCount);
        sendBuffer[sendCount] = 0;
        Log(eLogDebug, CMP_NAME, "Wrote: %s", &sendBuffer);
    }
    sendStatus = eSendStatusIdle;

    return eOK;    
}
