/****************************************************************************************************
* rfiduino.cpp - RFIDuino Library CPP File
*
* RFIDuino Library v1.3 by TrossenRobotics / RobotGeek / Dangerous Things
*
* This Library enables a Geekduino/Arduino to work with the RobotGeek RFIDuino Shield.
* Using the shield and this library a user can detect a card, read/decode the card,
* compare the read to previous reads, and interact with the onboard buzzer.
*
* Links
*      http://dngr.us/rfiduino-shield
*
****************************************************************************************************/
#include "Arduino.h"
#include "RFIDuino.h"

/**
 * Assume we are using 1.2 as those are the only ones for sale.
 *
 * @param float [version=1.2]
 */
RFIDuino::RFIDuino(float version) {
    if (version == (float) 1.1) {
    // original RFIDuino (4-pin antenna, no printed version number)
        // User Accessible Output
        buzzer = 5;
        led1 = 3;
        led2 = 2;

        // RFID related pins
        demodOut = 8;
        shd = 7;
        mod = 6;
        rdyClk = 4;
    } else {
    // RFIDuino 1.2 (2-pin antenna, "Rev 1.2" printed version number)
        // User Accessible Output
        buzzer = 5;
        led1 = 8;
        led2 = 4;

        // RFID related pins
        demodOut = 3;
        shd = 7;
        mod = 6;
        rdyClk = 2;
    }

    // set pin modes on RFID pins
    pinMode(mod, OUTPUT);
    pinMode(shd, OUTPUT);
    pinMode(demodOut, INPUT);
    pinMode(rdyClk, INPUT);

    // set pins for User Accessible Output
    pinMode(buzzer, OUTPUT);
    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);

    // set user outputs to LOW
    digitalWrite(buzzer, LOW);
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);

    // set shd and MOD low to prepare for reading
    digitalWrite(shd, LOW);
    digitalWrite(mod, LOW);
    sleep = digitalRead(shd);
}

/**
 * Manchester decode. Supply the function an array to store the tags ID in
 *
 * @param buf
 * @return
 */
bool RFIDuino::decodeTag(unsigned char *buf) {
    unsigned char i = 0;
    unsigned short timeCount;
    unsigned char timeOutFlag = 0;
    unsigned char row, col;
    unsigned char row_parity;
    unsigned char col_parity[5];
    unsigned char dat;
    unsigned char j;
    unsigned long start = millis();

    while (1) {
        // Give up between attempts, never mid-frame. Two EM4100 frames at the slowest
        // carrier (100 kHz) take ~82 ms, so any tag in the field gets a full read first.
        if (millis() - start >= DECODE_LIMIT_MS) {
            return false;
        }
        timeCount = 0;

        // watch for demodOut to go low
        while (0 == digitalRead(demodOut)) {
            // if we pass TIMEOUT milliseconds, break out of the loop
            if(timeCount >= TIMEOUT) {
                break;
            } else {
                timeCount++;
            }
        }

        if (timeCount >= 600) {
          return false;
        }
        timeCount = 0;

        delayMicroseconds(RFIDuino::delayVal);
        if(digitalRead(demodOut)) {
            // 9 header bits
            for(i = 0; i < 8; i++) {
                timeCount = 0; // restart counting
                // while DEMOD out is high
                while(1 == digitalRead(demodOut)) {
                  if(timeCount == TIMEOUT) {
                    timeOutFlag = 1;
                    break;
                  } else {
                    timeCount++;
                  }
                }

                if(timeOutFlag) {
                  break;
                } else {
                  delayMicroseconds(RFIDuino::delayVal);
                  if( 0 == digitalRead(demodOut)) {
                    break;
                  }
                }
            }

            if(timeOutFlag) {
                timeOutFlag = 0;
                return false;
            }

            // Receive the data
            if(i == 8) {
                timeOutFlag = 0;
                timeCount = 0;

                while(1 == digitalRead(demodOut)) {
                    if(timeCount == TIMEOUT) {
                        timeOutFlag = 1;
                        break;
                    } else {
                        timeCount++;
                    }

                    if(timeOutFlag) {
                        timeOutFlag = 0;
                        return false;
                    }
                }

                col_parity[0] = col_parity[1] = col_parity[2] = col_parity[3] = col_parity[4] = 0;
                for(row = 0; row < 11; row++) {
                    row_parity = 0;
                    j = row >> 1;

                    for(col = 0, row_parity = 0; col < 5; col++) {
                        delayMicroseconds(RFIDuino::delayVal);
                        if(digitalRead(demodOut)) {
                            dat = 1;
                        } else {
                            dat = 0;
                        }

                        if(col < 4 && row < 10) {
                            buf[j] <<= 1;
                            buf[j] |= dat;
                        }

                        row_parity += dat;
                        col_parity[col] += dat;
                        timeCount = 0;
                        while(digitalRead(demodOut) == dat) {
                            if(timeCount == TIMEOUT) {
                            timeOutFlag = 1;
                            break;
                            } else {
                            timeCount++;
                            }
                        }
                        if(timeOutFlag) {
                            break;
                        }
                    }

                    if(row < 10) {
                        // Row parity
                        if((row_parity & 0x01) || timeOutFlag) {
                            timeOutFlag = 1;
                            break;
                        }
                    }
                }

                // Column parity
                if( timeOutFlag || (col_parity[0] & 0x01) || (col_parity[1] & 0x01) || (col_parity[2] & 0x01) || (col_parity[3] & 0x01) ) {
                    timeOutFlag = 0;
                    return false;
                } else {
                    return true;
                }
            }
            return false;
        }
    }
};

/**
 * Use the tone() function to play an 'error' sound, a single tone repeated 3 times
 */
void RFIDuino::errorSound() {
  tone(buzzer, 1300, 500);
  delay(250);
  noTone(buzzer);
  delay(250);

  tone(buzzer, 1300, 1000);
  delay(250);
  noTone(buzzer);
  delay(250);

  tone(buzzer, 1300, 1000);
  delay(250);
  noTone(buzzer);
  delay(250);

};

/**
 * Use the tone() function to play an 'success' sound, 3 ascending tones
 */
void RFIDuino::successSound() {
  tone(buzzer, 3500, 1000);
  delay(165);
  noTone(buzzer);

  tone(buzzer, 4000, 1000);
  delay(165);
  noTone(buzzer);

  tone(buzzer, 4500, 1000);
  delay(165);
  noTone(buzzer);
  delay(165);
};

/**
 * Function to compare 2 byte arrays. Returns true if the two arrays match.
 *
 * @param tagData1
 * @param tagData2
 * @return
 */
bool RFIDuino::compareTagData(byte *tagData1, byte *tagData2) {
    for (int j = 0; j < 5; j++) {
        if (tagData1[j] != tagData2[j]) {
            return false; // if any of the ID numbers are not the same, return a false
        }
    }

    return true;  // all id numbers have been verified
}

/**
 * Function to transfer one byte array to a secondary byte array.
 *
 * @param tagData - Source
 * @param tagDataBuffer - Destination
 */
void RFIDuino::transferToBuffer(byte *tagData, byte *tagDataBuffer) {
    for (int j = 0; j < 5; j++) {
        tagDataBuffer[j] = tagData[j];
    }
}

/**
 * Returns true if a tag is being scanned
 *
 * @param tagData
 * @return
 */
bool RFIDuino::scanForTag(byte *tagData) {
    static byte tagDataBuffer[5];       // previous good read. 'static' so it survives between calls
    static bool haveFirstRead = false;  // true once tagDataBuffer holds a read to compare against

    if (!decodeTag(tagData)) return false;

    if (haveFirstRead && compareTagData(tagData, tagDataBuffer)) {
        haveFirstRead = false;          // verified; the next tag needs two fresh reads
        return true;
    }

    transferToBuffer(tagData, tagDataBuffer);  // first read, or a mismatch: this read becomes the new reference
    haveFirstRead = true;
    return false;
}

/**
 * Set the EM4095 to sleep (true) or awake (false) using the shd pin
 *
 * @param state
 */
void RFIDuino::setSleep(bool state) {
    if (state != RFIDuino::sleep) {
        digitalWrite(shd, state);
        if (!state) {
            delay(35); // give the EM4095 time to wake up
        }

        RFIDuino::sleep = digitalRead(shd);
    }
}

bool RFIDuino::getSleep() {
    return RFIDuino::sleep;
}
