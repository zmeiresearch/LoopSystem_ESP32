/*==============================================================================
   LoopSystem ESP32
   
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <HardwareSerial.h>
#include "driver/uart.h"    // for UART_PIN_NO_CHANGE

#include "config.h"
#include "control_serial.h"

#include <logger.h>
#include <control_protocol.h>

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


//==============================================================================
//  Local data
//==============================================================================
static HardwareSerial           serial(CONTROL_SERIAL_PORT);

static unsigned char            receiveBuffer[SERIAL_BUFFER_SIZE];
static size_t                   receiveIndex;
static ReceiveStatus            receiveStatus = eReceiveStatusIdle;
static PacketType               receivedPacketType = ePacketCount;
//==============================================================================
//  Local functions
//==============================================================================


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
        case ePacketModeNovice:
        case ePacketModeExpert:
        case ePacketModeAdvanced:
        case ePacketModeMaster:
            expectedValueCount = sizeof(ModeValuesAscii);
            break;
        default:
            expectedValueCount = 0;
            break;
    }

    return expectedValueCount;
}

void ProcessPacket(const unsigned char * const buffer, size_t bufferSize)
{
    Log(eLogDebug, CMP_NAME, "ProcessPacket: Processing: size: %d: %s", bufferSize, buffer);
    const PacketType packetType = (PacketType)buffer[1];

    const uint16_t receivedChecksum = ExtractChecksum(buffer, bufferSize);

    const uint16_t calculatedChecksum = CalculateChecksum(buffer, bufferSize);

    switch (packetType)
    {
        case ePacketGlobalValues:
        case ePacketModeNovice:
        case ePacketModeExpert:
        case ePacketModeAdvanced:
        case ePacketModeMaster:
        case ePacketCount:
        default:
            Log(eLogWarn, CMP_NAME, "ProcessPacket: Unexpected packet type: %02x", packetType);
            break;
    }
}

eStatus ControlSerialReceive()
{
    size_t expectedValueCount;
    // Process as much bytes as buffered
    while (serial.available())
    {
        unsigned char tmp = serial.read();
        Log(eLogDebug, CMP_NAME, "CSL: %d:%d:%02x", receiveStatus, receiveIndex, tmp);

        switch (receiveStatus)
        {
            case eReceiveStatusIdle:
            case eReceiveStatusError:
                // Always wait for a start byte
                if (START_BYTE == tmp)
                {
                    Log(eLogInfo, CMP_NAME, "ControlSerialLoop: Start Byte Received");
                    receiveIndex = 0;
                    receiveBuffer[receiveIndex++] = tmp;
                    receiveStatus = eReceiveStatusStartReceived;
                }
                break;
            
            case eReceiveStatusStartReceived:
                // Next comes the packet type
                if (((unsigned char)ePacketGlobalValues <= tmp) &&
                    (tmp < (unsigned char)ePacketCount))
                {
                    // valid packet type
                    Log(eLogInfo, CMP_NAME, "ControlSerialLoop: Valid Packet Type: 0x%02x", tmp);
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
                        Log(eLogInfo, CMP_NAME, "ControlSerialLoop: Values reception complete");
                    }
                }
                else
                {
                    // Unexpected non-BCD character
                    receiveStatus = eReceiveStatusError;
                    Log(eLogWarn, CMP_NAME, "ControlSerialLoop: Invalid character receiving values: 0x%02x", tmp);
                }
                break;

            case eReceiveStatusValuesReceived:
                // All values received, wait for checksum
                expectedValueCount = getValuesCount(receivedPacketType);
                if (eOK == IsHex(tmp))
                {
                    receiveBuffer[receiveIndex++] = tmp;
                    // + 6 for the Start byte and the Packet Type byte and the checksum
                    if (receiveIndex <= expectedValueCount + 6) 
                    {
                        receiveStatus = eReceiveStatusChecksumReceived;
                    }
                }
                else
                {
                    // Unexpected non-BCD character
                    receiveStatus = eReceiveStatusError;
                    Log(eLogWarn, CMP_NAME, "ControlSerialLoop: Invalid character receiving checksum: 0x%02x", tmp);
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

eStatus     ControlSerialTransmit()
{
    serial.write(REQUEST_GLOBAL_VALUES);
    return eOK; // Always return OK to continue looping
}
