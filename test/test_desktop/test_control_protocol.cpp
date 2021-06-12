/*==============================================================================
   LoopSystem ESP32 - Control Protocol Unit Test
   
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include <control_protocol.h>
#include <unity.h>
#include <string.h>

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME        "ControlProtocolTest"

#define START_BYTE      '*'
#define STOP_BYTE        '\n'

//==============================================================================
//  Local types
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================


//==============================================================================
//  Local functions
//==============================================================================

static void setPacketStartByte(PacketAllValuesAscii * const packet)
{
    packet->startByte = 0x2a;
}

static void setPacketStopByte(PacketAllValuesAscii * const packet)
{
    packet->stopByte = 0x13;
}

static void setAsciiValuesValid(AllValuesAscii * const values)
{
    memcpy(&values->cEnd, "123", 3);
    memcpy(&values->cAcc, "72", 2);
    memcpy(&values->cDec, "81", 2);
    memcpy(&values->cTurn, "42", 2);
    memcpy(&values->gHome, "933", 3);
    memcpy(&values->gEnd, "178", 3);
    memcpy(&values->gTurn1, "981", 3);
    memcpy(&values->gTurn2, "189", 3);
    memcpy(&values->gMaxAcc, "12", 2);
    memcpy(&values->gMaxDec, "63", 2);
    memcpy(&values->gFMax, "97", 2);
    memcpy(&values->gFMin, "55", 2);
    memcpy(&values->gMaxTime, "12", 2);
    memcpy(&values->gMaxLaps, "47", 2);
    memcpy(&values->gServSpeed, "73", 2);

}

static void setPacketChecksum(PacketAllValuesAscii * const packet)
{
    uint16_t checksum = 0;
    for (int i = 0; i < sizeof(PacketAllValuesAscii) - 3; i++)
    {
        uint8_t * ptr = (uint8_t *)&packet[i];
        checksum = checksum + *ptr;
    }

    checksum = ~checksum;

    packet->checksum[0] = (uint8_t)((checksum & 0xFF00)>> 8 );
    packet->checksum[1] = (uint8_t)(checksum & 0xFF);
}


static void buildPacketValid(PacketAllValuesAscii * const packet)
{
    setPacketStartByte(packet);
    setPacketStopByte(packet);
    setAsciiValuesValid(&packet->values);
    setPacketChecksum(packet);
}

static void test_packet_to_values(void)
{
    PacketAllValuesAscii packet;
    AllValues values;

    memset(&packet, 0, sizeof(PacketAllValuesAscii));
    setPacketChecksum(&packet);

    buildPacketValid(&packet);

//eStatus PacketAllValuesAsciiToAllValues(const PacketAllValuesAscii * const packet, AllValues * const values);
    TEST_ASSERT_EQUAL(eOK, PacketAllValuesAsciiToAllValues(&packet, &values));
    TEST_ASSERT_EQUAL(values.cEnd, 123);
    TEST_ASSERT_EQUAL(values.cAcc, 72);
    TEST_ASSERT_EQUAL(values.cDec, 81);
    TEST_ASSERT_EQUAL(values.cTurn, 42);
    TEST_ASSERT_EQUAL(values.gHome, 933);
    TEST_ASSERT_EQUAL(values.gEnd, 178);
    TEST_ASSERT_EQUAL(values.gTurn1, 981);
    TEST_ASSERT_EQUAL(values.gTurn2, 189);
    TEST_ASSERT_EQUAL(values.gMaxAcc, 12);
    TEST_ASSERT_EQUAL(values.gMaxDec, 63);
    TEST_ASSERT_EQUAL(values.gFMax, 97);
    TEST_ASSERT_EQUAL(values.gFMin, 55);
    TEST_ASSERT_EQUAL(values.gMaxTime, 12);
    TEST_ASSERT_EQUAL(values.gMaxLaps, 47);
    TEST_ASSERT_EQUAL(values.gServSpeed, 73);

/* memcpy(&values->cEnd, "123", 3);
    memcpy(&values->cAcc, "72", 2);
    memcpy(&values->cDec, "81", 2);
    memcpy(&values->cTurn, "42", 2);
    memcpy(&values->gHome, "933", 3);
    memcpy(&values->gEnd, "178", 3);
    memcpy(&values->gTurn1, "981", 3);
    memcpy(&values->gTurn2, "189", 3);
    memcpy(&values->gMaxAcc, "12", 2);
    memcpy(&values->gMaxDec, "63", 2);
    memcpy(&values->gFMax, "97", 2);
    memcpy(&values->gFMin, "55", 2);
    memcpy(&values->gMaxTime, "12", 2);
    memcpy(&values->gMaxLaps, "47", 2);
    memcpy(&values->gServSpeed, "73", 2);
*/

}

static void test_bcdToVal_u8(void)
{
    unsigned char in[2];
    uint8_t out;

    in[0] = 0x29;
    in[1] = 0x31;
    out = 0xab;
    TEST_ASSERT_EQUAL(eFAIL, BcdToVal(&in[0], &out));
    TEST_ASSERT_EQUAL(0xab, out);

    in[0] = '4';
    in[1] = '6';
    out = 0;
    TEST_ASSERT_EQUAL(eOK, BcdToVal(&in[0], &out));
    TEST_ASSERT_EQUAL(46, out);
}

static void test_bcdToVal_u16(void)
{
    unsigned char in[3];
    uint16_t out;

    in[0] = 0x31;
    in[1] = 0x32;
    in[2] = 0x3a;
    out = 0xabcd;
    TEST_ASSERT_EQUAL(eFAIL, BcdToVal(&in[0], &out));
    TEST_ASSERT_EQUAL(0xabcd, out);

    in[0] = '8';
    in[1] = '3';
    in[2] = '2';
    out = 0;
    TEST_ASSERT_EQUAL(eOK, BcdToVal(&in[0], &out));
    TEST_ASSERT_EQUAL(832, out);
}

static void test_valToBcd_u8(void)
{
    uint8_t in;
    unsigned char out[3];

    in = 101;
    out[0] = 0xde;
    out[1] = 0xad;
    out[2] = 0xc0;
    TEST_ASSERT_EQUAL(eFAIL, ValToBcd(in, &out[0]));
    TEST_ASSERT_EQUAL(0xde, out[0]);
    TEST_ASSERT_EQUAL(0xad, out[1]);
    TEST_ASSERT_EQUAL(0xc0, out[2]);

    in = 73;
    TEST_ASSERT_EQUAL(eOK, ValToBcd(in, &out[0]));
    TEST_ASSERT_EQUAL('7', out[0]);
    TEST_ASSERT_EQUAL('3', out[1]);
    TEST_ASSERT_EQUAL(0xc0, out[2]);
}

static void test_valToBcd_u16(void)
{
    uint16_t in;
    unsigned char out[4];

    in = 1001;
    out[0] = 0xde;
    out[1] = 0xad;
    out[2] = 0xc0;
    out[3] = 0xde;
    TEST_ASSERT_EQUAL(eFAIL, ValToBcd(in, &out[0]));
    TEST_ASSERT_EQUAL(0xde, out[0]);
    TEST_ASSERT_EQUAL(0xad, out[1]);
    TEST_ASSERT_EQUAL(0xc0, out[2]);
    TEST_ASSERT_EQUAL(0xde, out[3]);

    in = 739;
    TEST_ASSERT_EQUAL(eOK, ValToBcd(in, &out[0]));
    TEST_ASSERT_EQUAL('7', out[0]);
    TEST_ASSERT_EQUAL('3', out[1]);
    TEST_ASSERT_EQUAL('9', out[2]);
    TEST_ASSERT_EQUAL(0xde, out[3]);
}

//==============================================================================
//  Exported functions
//==============================================================================
int main(int argc, char ** argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_bcdToVal_u8);
    RUN_TEST(test_bcdToVal_u16);
    RUN_TEST(test_valToBcd_u8);
    RUN_TEST(test_valToBcd_u16);

    RUN_TEST(test_packet_to_values);
    
    UNITY_END();
    return 0;
}

