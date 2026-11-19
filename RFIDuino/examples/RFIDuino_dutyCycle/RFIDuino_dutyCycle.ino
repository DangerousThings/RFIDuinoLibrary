/***************************************************************************
 * RFIDuino Duty Cycle
 *
 * Saves power by letting the RFIDuino's reader chip sleep between listening
 * periods. Time is split into slots (myRFIDuino.slotMs, 160 ms by default):
 * the reader is awake for one slot, then asleep for the rest of the cycle.
 *
 *   setDutyCycle(0.25) -> awake 1 slot in 4 (160 ms on, 480 ms off)
 *   setDutyCycle(1.0)  -> always on (the default)
 *
 * Keep calling scanForTag() as usual; it does the switching. It never puts
 * the reader to sleep in the middle of reading a tag.
 *
 * The red LED is on while the reader is awake, so you can watch the cycle.
 * A tag held near the antenna is read once per cycle. A tag swiped past
 * quickly may be missed while the reader is asleep: lower duty cycles save
 * more power but react more slowly.
 *
 * If tags at the edge of range stop reading, raise myRFIDuino.slotMs.
 *
 * The RFIDuino Shield is designed to be used with 125khz EM4100 family tags.
 *
 * Links
 *    RFIDuino Shield Product Page
 *      http://dngr.us/rfiduino-shield
 *
 ***************************************************************************/

#include <RFIDuino.h> //include the RFIDuino Library

#define SERIAL_PORT Serial      //Serial port definition for Geekduino, Arduino Uno, and most Arduino Boards
//#define SERIAL_PORT Serial1   //Serial port defintion for Arduino Leonardo and other ATmega32u4 based boards

/***************
* HARDWARE VERSION - MAKE SURE YOU HAVE SET THE CORRECT HARDWARE VERSION BELOW
* Version 1.1 has a 4-pin antenna port and no version marking
* Version 1.2 has a 2-pin antenna port and is marked 'Rev 1.2'
*****************/
//RFIDuino myRFIDuino(1.1);     //initialize an RFIDuino object for hardware version 1.1
RFIDuino myRFIDuino(1.2);   //initialize an RFIDuino object for hardware version 1.2

byte tagData[5]; //Holds the ID numbers from the tag

void setup()
{
  SERIAL_PORT.begin(9600);
  SERIAL_PORT.println("RFIDuino Duty Cycle Example. Hold a tag near the antenna.");

  myRFIDuino.setDutyCycle(0.25);   //awake 1 slot in 4
}

void loop()
{
  //red LED shows when the reader is awake
  digitalWrite(myRFIDuino.led1, myRFIDuino.getSleep() ? LOW : HIGH);

  if(myRFIDuino.scanForTag(tagData) == true)
  {
    SERIAL_PORT.print("RFID Tag ID:");
    for(int n=0;n<5;n++)
    {
      SERIAL_PORT.print(tagData[n],DEC);
      if(n<4)
      {
        SERIAL_PORT.print(",");
      }
    }
    SERIAL_PORT.print("\n\r");

    digitalWrite(myRFIDuino.led2,HIGH);     //short green flash and beep
    digitalWrite(myRFIDuino.buzzer, HIGH);
    delay(100);
    digitalWrite(myRFIDuino.buzzer, LOW);
    digitalWrite(myRFIDuino.led2,LOW);
  }
}
