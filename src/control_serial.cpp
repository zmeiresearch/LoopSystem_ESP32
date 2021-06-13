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
    while(serial.available())
    {
        uint8_t tmp = serial.read();
        switch (status)
        {
            case eReceiveIdle:
            case eReceiveError:
                if (START_BYTE == tmp)
                {
                    writePtr = (uint8_t *)&inPacket;
                    *writePtr++ = tmp;
                    status = eReceiveInProgress;
                }
                break;
            case eReceiveInProgress:
                if (writePtr > (uint8_t *)&(inPacket.stopByte))
                {
                    status = eReceiveError;
                    Log(eLogError, CMP_NAME, "ControlSerialLoop: receive overflow!");
                }
                else
                {
                    if ( (writePtr == (uint8_t*)&(inPacket.stopByte)) &&
                         (STOP_BYTE == tmp))
                    {
                        *writePtr++ = tmp;
                        Log(eLogInfo, CMP_NAME, "ControlSerialLoop: processing packet");
                        PacketAllValuesAsciiToAllValues(&inPacket, &values);
                        status = eReceiveIdle;
                    }

                    *writePtr++ = tmp;
                }
                break;
        }
    }

    return eOK; // Always return OK to continue looping

}
