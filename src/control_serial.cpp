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
typedef enum _receiveStatus
{
    eReceiveStatusIdle,
    eReceiveStatusStartReceived,
    eReceiveStatusTypeReceived,
    eReceiveStatusValuesReceived,
    eReceiveStatusChecksumReceived,
    eReceiveStatusError,
    eReceiveStatusCount
} receiveStatus;


//==============================================================================
//  Local data
//==============================================================================
static HardwareSerial           serial(CONTROL_SERIAL_PORT);

static unsigned char            receiveBuffer[SERIAL_BUFFER_SIZE];
static size_t                   receiveIndex;
static receiveStatus            receiveStatus = eReceiveStatusIdle;
static ePacketType              receivePacketType = ePacketTypeCount;
//==============================================================================
//  Local functions
//==============================================================================


//==============================================================================
//  Exported functions
//==============================================================================

eStatus ControlSerialInit(void * param)
{
    Log(eLogInfo, CMP_NAME, "ControlSerialInit");

    writePtr = (uint8_t *)&inPacket;
    status = eReceiveIdle;

    serial.begin(CONTROL_SERIAL_BAUD, CONTROL_SERIAL_FORMAT, CONTROL_SERIAL_RX_PIN, CONTROL_SERIAL_TX_PIN); 

    return eOK;
}

size_t getValuesCount(ePacketType packetType)
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

eStatus ControlSerialLoop()
{
    // Process as much bytes as buffered
    while (serial.available())
    {
        unsigned char tmp = serial.read();
        Log(eLogDebug, CMP_NAME, "CSL: %d:%d:%02x:%08x:%08x", total, status, tmp, writePtr, (uint8_t *)&(inPacket.stopByte));
    }

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
                receivedPacketType = tmp;
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
            const size_t expectedValueCount = getValuesCount(receivedPacketType);
            if (receiveIndex <= expectedValueCount + 2) // + 2 for the Start byte and the Packet Type byte
            {
                if (isBcd(tmp))
                {
                    receiveBuffer[receiveIndex++] = tmp;
                }
                else
                {
                    // Unexpected non-BCD character
                    receiveStatus = eReceiveStatusError;
                    Log(eLogWarn, CMP_NAME, "ControlSerialLoop: Invalid character receiving values: 0x%02x", tmp);
                }
            }
            else
            {
                receiveStatus = eReceiveStatusValuesReceived;
            }
            break;

        case eReceiveStatusValuesReceived:
            // All values received, wait for checksum
            const size_t expectedValueCount = getValuesCount(receivedPacketType);
            if (receiveIndex <= expectedValueCount + 6) // + 6 for the Start byte and the Packet Type byte and the checksum
            {
                if (isHex(tmp))
                {
                    receiveBuffer[receiveIndex++] = tmp;
                }
                else
                {
                    // Unexpected non-BCD character
                    receiveStatus = eReceiveStatusError;
                    Log(eLogWarn, CMP_NAME, "ControlSerialLoop: Invalid character receiving checksum: 0x%02x", tmp);
                }
            }
            else
            {
                receiveStatus = eReceiveStatusChecksumReceived;
            }
 
            break;

        case eReceiveStatusChecksumReceived:
            // Everything received, wait for stop byte
            if (STOP_BYTE == tmp) 
            {
                Log(eLogInfo, CMP_NAME, "ControlSerialLoop: ProcessingPacket");
                ProcessPacket(&receiveBuffer, receiveIndex);
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

    return eOK; // Always return OK to continue looping
}


eStatus ControlSerialLoop()
{
    while(serial.available())
    {
        uint8_t tmp = serial.read();
        total++;

    //    Log(eLogDebug, CMP_NAME, "CSL: %d:%d:%02x:%08x:%08x", total, status, tmp, writePtr, (uint8_t *)&(inPacket.stopByte));

        switch (status)
        {
            case eReceiveIdle:
            case eReceiveError:
                if (START_BYTE == tmp)
                {
                    Log(eLogInfo, CMP_NAME, "ControlSerialLoop: Start Byte Received");
                    writePtr = (uint8_t *)&inPacket;
                    *writePtr++ = tmp;
                    status = eReceiveInProgress;
                }
                break;
            case eReceiveInProgress:
                *writePtr = tmp;
                if (STOP_BYTE == tmp)
                {
                    if (writePtr == (uint8_t*)&(inPacket.stopByte))
                    {
                        Log(eLogInfo, CMP_NAME, "ControlSerialLoop: processing packet");
                        PacketAllValuesAsciiToAllValues(&inPacket, &values);
                        status = eReceiveIdle;
                    }
                    else
                    {
                        Log(eLogError, CMP_NAME, "ControlSerialLoop: Unexpected stop byte received!");
                        status = eReceiveError;
                    }
                }
                else
                {
                    if (writePtr > (uint8_t*)&(inPacket.stopByte))
                    {
                        Log(eLogError, CMP_NAME, "ControlSerialLoop: receive overflow!");
                        status = eReceiveError;
                    }
                }

                if (eReceiveInProgress == status)
                {
                    writePtr++;
                }
                break;
        }
    }

    return eOK; // Always return OK to continue looping

}
