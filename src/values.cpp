/*==============================================================================
   LoopSystem ESP32
   
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include "config.h"
#if defined(DEMO_MODE)
#include <Arduino.h>
#endif // DEMO_MODE

#include <control_protocol.h>

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME    "Values"

//==============================================================================
//  Local types
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================

//==============================================================================
//  Exported data
//==============================================================================
GlobalValues    gGlobalValues;
ModeValues      gModeValues[MODE_COUNT];
CurrentStatus   gStatus;


//==============================================================================
//  Local functions
//==============================================================================


//==============================================================================
//  Exported functions
//==============================================================================
#if defined(DEMO_MODE)
eStatus DemoModeInit(void * params)
{
    gModeValues[0].acc = 1;
    gModeValues[0].dec = 2;
    gModeValues[0].end = 34;
    gModeValues[0].turn = 156;

    gModeValues[1].acc = 5;
    gModeValues[1].dec = 3;
    gModeValues[1].end = 12;
    gModeValues[1].turn = 48;

    gModeValues[2].acc = 8;
    gModeValues[2].dec = 1;
    gModeValues[2].end = 77;
    gModeValues[2].turn = 39;

    gModeValues[3].acc = 44;
    gModeValues[3].dec = 22;
    gModeValues[3].end = 11;
    gModeValues[3].turn = 33;

    gGlobalValues.home = 15;
    gGlobalValues.end = 155;
    gGlobalValues.turn1 = 25;
    gGlobalValues.turn2 = 138;
    gGlobalValues.maxAcc = 12;
    gGlobalValues.maxDec = 88;
    gGlobalValues.maxSpeed = 666;
    gGlobalValues.homingSpeed = 33;
    gGlobalValues.maxTime = 128;
    gGlobalValues.maxLaps = 13;
    gGlobalValues.servSpeed = 2;

    return eOK;
}

eStatus DemoModeLoop()
{
    int mode = random(0,4);

    gModeValues[mode].acc += random(-1, 1);
    gModeValues[mode].dec += random(-1, 1);
    gModeValues[mode].turn += random(-1, 1);
    gModeValues[mode].end += random(-1, 1);


    gGlobalValues.home += random(-1, 1);
    gGlobalValues.end += random(-1, 1);
    gGlobalValues.turn1 += random(-1, 1);
    gGlobalValues.turn2 += random(-1, 1);
    gGlobalValues.maxAcc += random(-1, 1);
    gGlobalValues.maxDec += random(-1, 1);
    gGlobalValues.maxSpeed += random(-1, 1);
    gGlobalValues.homingSpeed += random(-1, 1);
    gGlobalValues.maxTime += random(-1, 1);
    gGlobalValues.maxLaps += random(-1, 1);
    gGlobalValues.servSpeed += random(-1, 1);


    return eOK;
}
#endif // DEMO_MODE