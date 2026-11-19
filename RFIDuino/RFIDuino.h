/****************************************************************************************************
* rfiduino.h - RFIDuino Library Header File
*
* RFIDuino Library v1.3 by TrossenRobotics / RobotGeek / Dangerous Things
*
*	This Library enables a Geekduino/Arduino to work with the RobotGeek RFIDuino Shield.
*	Using the sheild and this library a user can detect a card, read/decode the card,
*	compare the read to previous reads, and intearct with the inbord buzzer.
*
* Links
*    RFIDuino Shield Product Page
*      http://dngr.us/rfiduino-shield
*
* Header file for the RFIDuino Library
*
****************************************************************************************************/

#include "Arduino.h"

#ifndef RFIDuino_h                                                  		// load header if we have not defined RFIDuino_h
#define RFIDuino_h                          

#define DECODE_LIMIT_MS 100                                         		// max time decodeTag() keeps looking for a frame
#define WAKE_MS     35                                              		// EM4095 Tset max: wake-up until reads are valid
#define MIN_SLOT_MS 100                                             		// shortest duty-cycle slot: wake-up + a confirmed read at 125 kHz
#define TIMEOUT     1000                                            		// standard timeout for manchester decode

class RFIDuino {
	public:

		RFIDuino(float version = 1.2);
		
		// Manchester decode: microseconds from a signal edge to the bit sample. Must land in the second half
		// of a bit, ~0.6-0.75 of it (384 = 3/4 bit at 125 kHz). Tune if your antenna doesn't resonate at 125 kHz.
		unsigned int delayVal = 320;

		// i/o
		void errorSound(); 					        // play an error sound to the buzzer
		void successSound(); 				                // play a success sound ot the buzzer

		// low level decode functions
		bool decodeTag(unsigned char *buf);	                        // low level tag decode
		void transferToBuffer(byte *tagData, byte *tagDataBuffer);	// transfer data from one array to another
		bool compareTagData(byte *tagData1, byte *tagData2);		// compare 2 byte arrays

		// higher level tag scanning / reporting
		bool scanForTag(byte *tagData);

		// Duty cycling, to save power: the EM4095 is awake for 1 slot, then asleep for the rest of the cycle.
		// 1.0 = always on (default); 0.25 = awake 1 slot in 4. scanForTag() does the switching and never
		// sleeps mid-read or between the two reads of a pair. Don't call setSleep() while below 1.0.
		void setDutyCycle(float dutyCycle);

		// Duty-cycle slot in ms: wake-up (35 ms) + listening. The default covers a confirmed read at 100-150 kHz;
		// raise it if reads fail at the edge of range. Values below MIN_SLOT_MS are raised to it.
		unsigned int slotMs = 160;

		// Sleep getter and setter
		void setSleep(bool state);                                  	// Change shd pin state
		bool getSleep();

		// user output pin values, variables as they will change depending on which hardware is used
		int buzzer;
		int led1;
		int led2;

	private:

		// pins to connect Arduino to the EM4095 chip, variables as they will change depending on which hardware is used
		int demodOut;
		int shd;
		int mod;
		int rdyClk;

		// Cached sleep pin state
		bool sleep;

		// scanForTag() pair verification
		byte tagDataBuffer[5];				// previous good read
		bool haveFirstRead = false;			// true once tagDataBuffer holds a read to compare against

		// duty cycling
		unsigned int sleepSixteenths = 0;		// sleep length in 1/16 slots; 0 = duty cycling off. Integer so float math stays in setDutyCycle()
		unsigned long phaseStart = 0;			// millis() when the chip last woke or went to sleep
		unsigned long sleepFor = 0;			// length of the current sleep, ms
		void sleepNow();

};

#endif
