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
    eReceiveIdle,
    eReceiveInProgress,
    eReceiveError,
    eReceiveCount
} receiveStatus;


//==============================================================================
//  Local data
//==============================================================================
static HardwareSerial           serial(CONTROL_SERIAL_PORT);
static PacketAllValuesAscii     inPacket;
static PacketAllValuesAscii     outPacket;
static AllValues                values;

static uint8_t *                writePtr;
static receiveStatus            status;

//==============================================================================
//  Local functions
//==============================================================================


//==============================================================================
//  Exported functions
//==============================================================================

size_t ControlSerialGetWriteSize()
{
    return serial.availableForWrite();
}

size_t ControlSerialWrite(const uint8_t * const buffer, const size_t toSend)
{
    return serial.write(buffer, toSend);
}

eStatus ControlSerialInit(void * param)
{
    Log(eLogInfo, CMP_NAME, "ControlSerialInit");

    writePtr = (uint8_t *)&inPacket;
    status = eReceiveIdle;

    serial.begin(CONTROL_SERIAL_BAUD, CONTROL_SERIAL_FORMAT, CONTROL_SERIAL_RX_PIN, CONTROL_SERIAL_TX_PIN); 

    return eOK;
}

eStatus ControlSerialLoop()
{
    static int total = 0;
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
