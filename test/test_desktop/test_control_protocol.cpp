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

static void setStartByte(PacketAllValuesAscii * const packet)
{
    packet->startByte = 0x2a;
}

static void setStopByte(PacketAllValuesAscii * const packet)
{
    packet->stopByte = 0x13;
}

static void setChecksum(PacketAllValuesAscii * const packet)
{
    uint16_t checksum = 0;
    for (int i = 0; i < sizeof(PacketAllValuesAscii) - 3; i++)
    {
        checksum = checksum + ((uint8_t*)&packet)[i];
    }

    checksum = ~checksum;

    packet->checksum[0] = (uint8_t)((checksum & 0xFF)>> 8 );
    packet->checksum[1] = (uint8_t)(checksum & 0xFF);
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
    
    UNITY_END();
    return 0;
}

