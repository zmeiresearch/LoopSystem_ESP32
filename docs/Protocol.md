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
* Communication speed is 19200 bps, format: 8N2


## Packet format
* First byte of packet is a start symbol `*`, 0x2A
* Second byte is packet type - see below [Packet Types](#packet-types)
* N bytes - fixed-length payload based on packet type
* 2 bytes [checksum](#checksum-calculation)
* Stop symbol - Carriage return, 0x13

## Value representation
* Protocol-native fields - checksum is in *LITTLE ENDIAN*, e.g. if the checksum is 0x1234, first transmitted byte will
  be 0x34 and second will be 0x12
* All other values are transmitted in BCD (Binary-coded decimal) - e.g. if END is "123", bytes on the line will be 
    1 - 0x31
    2 - 0x32
    3 - 0x33
* Decimal separator is not transmitted 


## Packet types
| Packet code   | Description                           |
| ------------- | ------------------------------------- |
| 0x01          | [All Settings](#packet-all-values)    |
| 0xFE          | [Acknowledge](#packet-acknowledge)    |


### Packet All Values
| Byte  | Value |   Description         |   Format                  |
| ----- | ----- | --------------------- | ------------------------- |
| 0x1   | 0x28  | Start symbol          | Always a star `*`         |
| 0x2   | 0x01  | Packet Type           | Always 0x01               |
| 0x3   | 0xXX  | CEND - Digit 1        |                           |
| 0x4   | 0xXX  | CEND - Digit 2        |                           | 
| 0x5   | 0xXX  | CEND - Digit 3        |                           |
| 0x6   | 0xXX  | CACC - Digit 1        |                           |
| 0x7   | 0xXX  | CACC - Digit 2        |                           |
| 0x8   | 0xXX  | CDEC - Digit 1        |                           |
| 0x9   | 0xXX  | CDEC - Digit 2        |                           |
| 0xA   | 0xXX  | CTURN2 - Digit 1      |                           |
| 0xB   | 0xXX  | CTURN2 - Digit 2      |                           |
| 0xC   | 0xXX  | GHOME - Digit 1       |                           |
| 0xD   | 0xXX  | GHOME - Digit 2       |                           |
| 0xE   | 0xXX  | GHOME - Digit 3       |                           |
| 0xF   | 0xXX  | GEND - Digit 1        |                           |
| 0x10  | 0xXX  | GEND - Digit 2        |                           |
| 0x11  | 0xXX  | GEND - Digit 3        |                           |
| 0x12  | 0xXX  | GTURN1 - Digit 1      |                           |
| 0x13  | 0xXX  | GTURN1 - Digit 2      |                           |
| 0x14  | 0xXX  | GTURN1 - Digit 3      |                           |
| 0x15  | 0xXX  | GTURN2 - Digit 1      |                           |
| 0x16  | 0xXX  | GTURN2 - Digit 2      |                           |
| 0x17  | 0xXX  | GTURN2 - Digit 3      |                           |
| 0x18  | 0xXX  | GMAXACC - Digit 1     |                           |
| 0x19  | 0xXX  | GMAXACC - Digit 2     |                           |
| 0x1A  | 0xXX  | GMAXDEC - Digit 1     |                           |
| 0x1B  | 0xXX  | GMAXDEC - Digit 2     |                           |
| 0x1C  | 0xXX  | GFMAX - Digit 1       |                           |
| 0x1D  | 0xXX  | GFMAX - Digit 2       |                           |
| 0x1E  | 0xXX  | GFMIN - Digit 1       |                           |
| 0x1F  | 0xXX  | GFMIN - Digit 2       |                           |
| 0x20  | 0xXX  | GMAXTIME - Digit 1    |                           |
| 0x21  | 0xXX  | GMAXTIME - Digit 2    |                           |
| 0x22  | 0xXX  | GMAXLAPS - Digit 1    |                           |
| 0x23  | 0xXX  | GMAXLAPS - Digit 2    |                           |
| 0x24  | 0xXX  | GSERVSPEED - Digit 1  |                           |
| 0x25  | 0xXX  | GSERVSPEED - Digit 2  |                           |
| 0x26  | 0xXX  | CRC - Byte 1          |                           |
| 0x27  | 0xXX  | CRC - Byte 2          |                           |
| 0x28  | 0x13  | End symbol            | Always CR                 |


### Packet Acknowledge
| Byte  | Value |   Description         |   Format                  |
| ----- | ----- | --------------------- | ------------------------- |
| 0x1   | 0x28  | Start symbol          | Always a star `*`         |
| 0x2   | 0xFE  | Packet Type           | Always 0xFE               |
| 0x3   | 0xXX  | CRC - Byte 1          | 0x00 - 0xFF               |
| 0x4   | 0xXX  | CRC - Byte 2          | 0x00 - 0xFF               |
| 0x5   | 0x13  | End Symbol            | Always CR                 |







