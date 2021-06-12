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

static void setStartByte(PacketAllValues * const packet)
{
    packet->startByte = 0x2a;
}

static void setStopByte(PacketAllValues * const packet)
{
    packet->stopByte = 0x13;
}

static void setChecksum(PacketAllValues * const packet)
{
    uint16_t checksum = 0;
    for (int i = 0; i < sizeof(PacketAllValues) - 3; i++)
    {
        checksum = checksum + ((uint8_t*)&packet)[i];
    }

    checksum = ~checksum;

    packet->checksum[0] = (uint8_t)((checksum & 0xFF)>> 8 );
    packet->checksum[1] = (uint8_t)(checksum & 0xFF);
}

static void setDefaultValues(PacketAllValues * const packet)
{
}

static void test_checkPacketAllValues_BadStartByte(void)
{
    PacketAllValues packet;
    memset(&packet, 0, sizeof(PacketAllValues));
    
    packet.startByte = 0x22;
    TEST_ASSERT_EQUAL(eFAIL, CheckPacketAllValues(&packet));
}

static void test_checkPacketAllValues_BadChecksum(void)
{
    PacketAllValues packet;
    memset(&packet, 0, sizeof(PacketAllValues));
    
    packet.startByte = 0x2a;
    TEST_ASSERT_EQUAL(eFAIL, CheckPacketAllValues(&packet));
}

static void test_checkPacketAllValues_BadValues(void)
{
    PacketAllValues packet;
    memset(&packet, 0, sizeof(PacketAllValues));
    
    packet.startByte = 0x2a;
    packet.checksum[0] = 0xff;
    packet.checksum[1] = 0xd5;
    TEST_ASSERT_EQUAL(eOK, CheckPacketAllValues(&packet));
}

static void test_bcdToVal_u8(void)
{
    char in[2];
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
    char in[3];
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


//==============================================================================
//  Exported functions
//==============================================================================
int main(int argc, char ** argv)
{
    UNITY_BEGIN();

//    RUN_TEST(test_checkPacketAllValues_BadStartByte);

//    RUN_TEST(test_checkPacketAllValues_BadChecksum);

  //  RUN_TEST(test_checkPacketAllValues_GoodChecksum);

    RUN_TEST(test_bcdToVal_u8);
    RUN_TEST(test_bcdToVal_u16);
    
    UNITY_END();
    return 0;
}
