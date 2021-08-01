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

#include "webserver.h"

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
//  Local function declarations
//==============================================================================
static eStatus  queueForTransmit(const char * const buffer, size_t size);
static size_t   getValuesByteCount(PacketType packetType);


//==============================================================================
//  Local functions
//==============================================================================
static eStatus queueForTransmit(const char * const buffer, size_t size)
{
    eStatus retVal = eBUSY;

    do
    {
        if ((sendStatus == eSendStatusIdle) ||
            (sendStatus == eSendStatusComplete))
        {
            if (size <= SERIAL_BUFFER_SIZE)
            {
                memcpy(sendBuffer, buffer, size);
                sendCount = size;
                sendStatus = eSendStatusQueued;
                retVal = eOK;
            }
            else
            {
                retVal = eOVERFLOW;
            }
        }
        
        if (eBUSY == retVal)
        {
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
    } while (eBUSY == retVal);

    return retVal;
}

static size_t getValuesByteCount(PacketType packetType)
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

//==============================================================================
//  Global Values handling
//==============================================================================
void SerialRequestGlobalValues()
{
    Log(eLogInfo, CMP_NAME, "SerialRequestGlobalValues");
    queueForTransmit(REQUEST_GLOBAL_VALUES, 3);
}

static void serialReceiveGlobalValueAscii(PacketGlobalValuesAscii const * const packet)
{
    GlobalValues values;
    Log(eLogInfo, CMP_NAME, "serialReceiveGlobalValueAscii");
    values.home = TenByteBcdToUint32((const char *)&packet->values.home[0]);
    values.maxEnd = TenByteBcdToUint32((const char *)&packet->values.maxEnd[0]);
    values.maxTurn1 = TenByteBcdToUint32((const char *)&packet->values.maxTurn1[0]);
    values.minTurn2 = TenByteBcdToUint32((const char *)&packet->values.minTurn2[0]);
    values.maxAcc = FiveByteBcdToUint32((const char *)&packet->values.maxAcc[0]);
    values.maxDec = FiveByteBcdToUint32((const char *)&packet->values.maxDec[0]);
    values.maxSpeed = FiveByteBcdToUint32((const char *)&packet->values.maxSpeed[0]);
    values.homingSpeed = FiveByteBcdToUint32((const char *)&packet->values.homingSpeed[0]);
    values.maxTime = FiveByteBcdToUint32((const char *)&packet->values.maxTime[0]);
    values.maxLaps = FiveByteBcdToUint32((const char *)&packet->values.maxLaps[0]);
    values.servSpeed = FiveByteBcdToUint32((const char *)&packet->values.servSpeed[0]);

    DumpGlobalValues(&values);

    PushGlobalValues(values);
}

void SerialSendGlobalValues(const GlobalValues * const values)
{
    PacketGlobalValuesAscii packet;

    packet.startByte = START_BYTE_WRITE;
    packet.packetType = ePacketGlobalValues;

    // All lines print out a null terminator, but it gets overwritten by the next one
    snprintf((char *)&packet.values.home[0], 11, "%010u", values->home);
    snprintf((char *)&packet.values.maxEnd[0], 11, "%010u", values->maxEnd);
    snprintf((char *)&packet.values.maxTurn1[0], 11, "%010u", values->maxTurn1);
    snprintf((char *)&packet.values.minTurn2[0], 11, "%010u", values->minTurn2);
    snprintf((char *)&packet.values.maxAcc[0], 6, "%05u", values->maxAcc);
    snprintf((char *)&packet.values.maxDec[0], 6, "%05u", values->maxDec);
    snprintf((char *)&packet.values.maxSpeed[0], 6, "%05u", values->maxSpeed);
    snprintf((char *)&packet.values.homingSpeed[0], 6, "%05u", values->homingSpeed);
    snprintf((char *)&packet.values.maxTime[0], 6, "%05u", values->maxTime);
    snprintf((char *)&packet.values.maxLaps[0], 6, "%05u", values->maxLaps);
    snprintf((char *)&packet.values.servSpeed[0], 6, "%05u", values->servSpeed);

    uint16_t checksum = CalculateChecksum((const unsigned char *)&packet, sizeof(PacketGlobalValuesAscii));

    snprintf((char*)&packet.checksum[0], 5, "%04X", checksum);
    packet.stopByte = STOP_BYTE;

    Log(eLogInfo, CMP_NAME, "SerialSendGlobalValues: %s", &packet);

    queueForTransmit((const char *)&packet, sizeof(PacketGlobalValuesAscii));
}

//==============================================================================
//  ModeValues Handling
//==============================================================================
void SerialRequestModeValues(Modes mode)
{
    Log(eLogInfo, CMP_NAME, "SerialRequestModeValues: %d", mode);
    switch (mode)
    {
        case eModeNovice:
            queueForTransmit(REQUEST_MODE_NOVICE, 3);
            break;
        case eModeAdvanced:
            queueForTransmit(REQUEST_MODE_ADVANCED, 3);
            break;
        case eModeExpert:
            queueForTransmit(REQUEST_MODE_EXPERT, 3);
            break;
        case eModeMaster:
            queueForTransmit(REQUEST_MODE_MASTER, 3);
            break;        
        default:
            Log(eLogWarn, CMP_NAME, "SerialRequestModeValues: invalid mode 0x%02x", mode);
            break;
    }
}

static void serialReceiveModeValuesAscii(PacketModeValuesAscii const * const modePacket)
{
    uint8_t mode = (uint8_t)modePacket->values.mode - 0x30;
    ModeValues values;

    if ((eModeNovice <= mode) && (mode <= eModeMaster))
    {
        values.mode = (Modes)mode;
        values.speed = FiveByteBcdToUint32((const char *)&modePacket->values.speed[0]);
        values.turn1 = TenByteBcdToUint32((const char *)&modePacket->values.turn1[0]);
        values.turn2 = TenByteBcdToUint32((const char *)&modePacket->values.turn2[0]);
        values.brakeTime = FiveByteBcdToUint32((const char *)&modePacket->values.brakeTime[0]);
        values.acc = FiveByteBcdToUint32((const char *)&modePacket->values.acc[0]);
        values.dec = FiveByteBcdToUint32((const char *)&modePacket->values.dec[0]);
        Log(eLogInfo, CMP_NAME, "serialReceiveModeValuesAscii: Updated mode 0x%02x", mode);
        DumpModeValues(&values);
        PushModeValues(values);
    }
    else 
    {
        Log(eLogWarn, CMP_NAME, "serialReceiveModeValuesAscii: invalid mode: 0x%02x", mode);
    }
}

void SerialSendModeValues(const ModeValues * const modeValues)
{
    PacketModeValuesAscii packet;

    packet.startByte = START_BYTE_WRITE;
    packet.packetType = ePacketModeValues;

    // All lines print out a null terminator, but it gets overwritten by the next one
    snprintf((char *)&packet.values.mode, 2, "%01u", modeValues->mode);
    snprintf((char *)&packet.values.speed[0], 6, "%05u", modeValues->speed);
    snprintf((char *)&packet.values.turn1[0], 11, "%010u", modeValues->turn1);
    snprintf((char *)&packet.values.turn2[0], 11, "%010u", modeValues->turn2);
    snprintf((char *)&packet.values.brakeTime[0], 6, "%05u", modeValues->brakeTime);
    snprintf((char *)&packet.values.acc[0], 6, "%05u", modeValues->acc);
    snprintf((char *)&packet.values.dec[0], 6, "%05u", modeValues->dec);

    uint16_t checksum = CalculateChecksum((const unsigned char *)&packet, sizeof(PacketModeValuesAscii));

    snprintf((char*)&packet.checksum[0], 5, "%04X", checksum);
    packet.stopByte = STOP_BYTE;

    Log(eLogInfo, CMP_NAME, "SerialSendModeValues: %s", &packet);

    queueForTransmit((const char *)&packet, sizeof(PacketModeValuesAscii));
}

//==============================================================================
//  Status handling
//==============================================================================
static void serialReceiveStatusAscii(PacketStatusAscii const * const statusPacket)
{
    Log(eLogDebug, CMP_NAME, "serialReceiveParseStatusAscii");
    CurrentStatus status;

    uint8_t mode = (uint8_t)statusPacket->status.mode - 0x30;

    status.mode = (Modes)mode;
    status.completedLaps = FiveByteBcdToUint32((const char *)&statusPacket->status.completedLaps[0]);
    status.position = TenByteBcdToUint32((const char *)&statusPacket->status.position[0]);
    status.systemStatus = statusPacket->status.systemStatus;
    //DumpStatus(&status);
    PushStatus(status);
}

//==============================================================================
//  Packet reception dispatcher
//==============================================================================
static void receivePacket(const unsigned char * const buffer, size_t bufferSize)
{
    Log(eLogDebug, CMP_NAME, "receivePacket: Processing: size: %d: %s", bufferSize, buffer);
    const PacketType packetType = (PacketType)buffer[1];

    const uint16_t receivedChecksum = ExtractChecksum(buffer, bufferSize);
    const uint16_t calculatedChecksum = CalculateChecksum(buffer, bufferSize);

    if (receivedChecksum == calculatedChecksum)
    {
        //Log(eLogInfo, CMP_NAME, "receivePacket: Good packet received, type: %02x", packetType);

        switch (packetType)
        {
            case ePacketGlobalValues:
                serialReceiveGlobalValueAscii((PacketGlobalValuesAscii const * const)buffer);
                break;
            case ePacketModeValues:
                serialReceiveModeValuesAscii((PacketModeValuesAscii const * const )buffer);
                break;
            case ePacketStatus:
                serialReceiveStatusAscii((PacketStatusAscii const * const )buffer);
                break;
            case ePacketCount:
            default:
                Log(eLogWarn, CMP_NAME, "receivePacket: Unexpected packet type: %02x", packetType);
                break;
        }
    }
    else
    {
        Log(eLogWarn, CMP_NAME, "receivePacket: bad checksum: received: 0x%04x, calculated: 0x%04x", receivedChecksum, calculatedChecksum);
    }
}

//==============================================================================
//  Task interface
//==============================================================================
eStatus ControlSerialInit(void * param)
{
    Log(eLogInfo, CMP_NAME, "ControlSerialInit");

    serial.begin(CONTROL_SERIAL_BAUD, CONTROL_SERIAL_FORMAT, CONTROL_SERIAL_RX_PIN, CONTROL_SERIAL_TX_PIN); 

    return eOK;
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
                expectedValueCount = getValuesByteCount(receivedPacketType);
                if (eOK == IsBcdOrMinus(tmp))
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
                expectedValueCount = getValuesByteCount(receivedPacketType);
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
                    //Log(eLogInfo, CMP_NAME, "ControlSerialLoop: ProcessingPacket");
                    receivePacket(&receiveBuffer[0], receiveIndex);
                    receiveStatus = eReceiveStatusIdle;
                }
                else
                {
                    receiveStatus = eReceiveStatusError;
                    Log(eLogWarn, CMP_NAME, "ControlSerialLoop: Unexpected character 0x%02x while waiting for stop byte", tmp);
                    receiveBuffer[receiveIndex++] = tmp;
                    receiveBuffer[receiveIndex] = 0;
                    Log(eLogWarn, CMP_NAME, "%s", receiveBuffer);
                }

                break;
        
            default:
                receiveStatus = eReceiveStatusError;
                break;
        }
    }

    return eOK; // Always return OK to continue looping
}

eStatus ControlSerialTransmit()
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
