# LoopCable ESP32 ESP32<->Control protocol

## Fields 

### Current settings

| Field name | Description              | Min | Max | Unit | Line Format | 
| ---------- | ------------------------ | --- | --- | ---- | ----------- |
| CEND       | End Turning Point        | ??? | ??? | m    | 000         |
| CACC       | Acceleration             | ??? | ??? | m/s2 | 0.0         |
| CDEC       | Deceleration             | ??? | ??? | m/s2 | 0.0         |
| CTURN2     | Brake Time               | ??? | ??? | s    | 0.0         |


### Global settings

TODO: What is relation with Current Settings: END and TURN2

| Field name | Description              | Min | Max | Unit | Line Format  | 
| ---------- | ------------------------ | --- | --- | ---- | -----------  |
| GHOME      | Home Position            | ??? | ??? | m    | 000          |
| GEND       | End Limit                | ??? | ??? | m    | 000          |
| GTURN1     | Turn 1 Position          | ??? | ??? | m    | 000          |
| GTURN2     | Turn 2 Position          | ??? | ??? | m    | 000          |
| GMAXACC    | Maximal acceleration     | ??? | ??? | m/s2 | 0.0          |
| GMAXDEC    | Maximal deceleration     | ??? | ??? | m/s2 | 0.0          |
| GFMAX      | Maximal speed            | ??? | ??? | m/s  | 0.0          |
| GFMIN      | Minimal speed            | ??? | ??? | m/s  | 0.0          |
| GMAXTIME?  | Maximal time in auto     | ??? | ??? | s    | 0.0          |
| GMAXLAPS?  | Lap limit in auto        | ??? | ??? | -    | 00           |
| GSERVSPEED | Service mode speed limit | ??? | ??? | m/s  | 0.0          |


## Line format
* Communication speed is 19200 bps, format: 8N1


## Packet format
* First byte of packet is a start symbol "*", 0x2A
* Second byte is overall packet length, including start symbol and checksum
* Third byte is packet type - see below [Packet Types](#packet-types)
* Fourth and fifth bytes are sequence number - used for ack management
* N bytes - fixed-length payload based on packet type
* 2 bytes checksum [CRC](#checksum-calculation)


## Value representation
* Protocol-native fields - sequence number and CRC are in *BIG ENDIAN*, e.g. if the sequence number is 0x1234, byte 4
  will be 0x12 and byte 5 will be 0x34
* All other values are transmitted in BCD (Binary-coded decimal) - e.g. if END is "123", bytes on the line will be 
    1 - 0x31
    2 - 0x32
    3 - 0x33
* Decimal separator is not transmitted - 


## Packet types
| Packet code   | Description                           |
| ------------- | ------------------------------------- |
| 0x01          | All Settings (#packet-all-settings)   |  
| 0xFE          | Acknowledge (#packet-acknowledge)     |


### Packet All Settings
| Byte  | Value |   Description         |   Format                  |
| ----- | ----- | --------------------- | ------------------------- |
| 0x1   | 0x28  | Start symbol          | Always a star "*"         |
| 0x2   | 0x2A  | Packet Length         | Always 0x2A               |
| 0x3   | 0x01  | Packet Type           | Always 0x01               |
| 0x4   | 0xXX  | Sequence number       | 0x00 - 0xFF               |
| 0x5   | 0xXX  | Sequence number       | 0x00 - 0xFF               |
| 0x6   | 0xXX  | CEND - Digit 1 |  |
| 0x7   | 0xXX  | CEND - Digit 2 |  | 
| 0x8   | 0xXX  | CEND - Digit 3 |  |
| 0x9   | 0xXX  | CACC - Digit 1 |  |
| 0xA   | 0xXX  | CACC - Digit 2 |  |
| 0xB   | 0xXX  | CDEC - Digit 1 |  |
| 0xC   | 0xXX  | CDEC - Digit 2 |  |
| 0xD   | 0xXX  | CTURN2 - Digit 1 |  |
| 0xE   | 0xXX  | CTURN2 - Digit 2 |  |
| 0xF   | 0xXX  | GHOME - Digit 1  |  |
| 0x10  | 0xXX  | GHOME - Digit 2 |  |
| 0x11  | 0xXX  | GHOME - Digit 3 |  |
| 0x12  | 0xXX  | GEND - Digit 1 |  |
| 0x13  | 0xXX  | GEND - Digit 2 |  |
| 0x14  | 0xXX  | GEND - Digit 3  |  |
| 0x15  | 0xXX  | GTURN1 - Digit 1 |  |
| 0x16  | 0xXX  | GTURN1 - Digit 2 |  |
| 0x17  | 0xXX  | GTURN1 - Digit 3 |  |
| 0x18  | 0xXX  | GTURN2 - Digit 1 |  |
| 0x19  | 0xXX  | GTURN2 - Digit 2 |  |
| 0x1A  | 0xXX  | GTURN2 - Digit 3 |  |
| 0x1B  | 0xXX  | GMAXACC - Digit 1  |  |
| 0x1C  | 0xXX  | GMAXACC - Digit 2 |  |
| 0x1D  | 0xXX  | GMAXDEC - Digit 1 |  |
| 0x1E  | 0xXX  | GMAXDEC - Digit 2 |  |
| 0x1F  | 0xXX  | GFMAX - Digit 1 |  |
| 0x20  | 0xXX  | GFMAX - Digit 2 |  |
| 0x21  | 0xXX  | GFMIN - Digit 1 |  |
| 0x22  | 0xXX  | GFMIN - Digit 2 |  |
| 0x23  | 0xXX  | GMAXTIME - Digit 1 |  |
| 0x24  | 0xXX  | GMAXTIME - Digit 2 |  |
| 0x25  | 0xXX  | GMAXLAPS - Digit 1 |  |
| 0x26  | 0xXX  | GMAXLAPS - Digit 2 |  |
| 0x27  | 0xXX  | GSERVSPEED - Digit 1 |  |
| 0x28  | 0xXX  | GSERVSPEED - Digit 2 |  |
| 0x29  | 0xXX  | CRC - Byte 1 |  |
| 0x2A  | 0xXX  | CRC - Byte 2 |  |


# Packet Acknowledge
| Byte  | Value |   Description         |   Format                  |
| ----- | ----- | --------------------- | ------------------------- |
| 0x1   | 0x28  | Start symbol          | Always a star "*"         |
| 0x2   | 0x07  | Packet Length         | Always 0x07               |
| 0x3   | 0xFE  | Packet Type           | Always 0xFE               |
| 0x4   | 0xXX  | Sequence number       | 0x00 - 0xFF               |
| 0x5   | 0xXX  | Sequence number       | 0x00 - 0xFF               |
| 0x6   | 0xXX  | CRC - Byte 1          | 0x00 - 0xFF               |
| 0x7   | 0xXX  | CRC - Byte 2          | 0x00 - 0xFF               |








