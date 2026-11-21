// Minimal Arduino stand-in for host tests: a settable clock and a pin-state array.
#pragma once
#include <stdint.h>
typedef uint8_t byte; typedef bool boolean;
#define INPUT 0
#define OUTPUT 1
#define LOW 0
#define HIGH 1
#define max(a,b) ((a)>(b)?(a):(b))
extern unsigned long fakeNow;        // ms; tests set it, delay() advances it
extern int pinState[32];
inline void pinMode(int,int){}
inline void digitalWrite(int p,int v){ pinState[p] = v; }
inline int digitalRead(int p){ return pinState[p]; }
inline unsigned long millis(){ return fakeNow; }
inline void delayMicroseconds(unsigned){}
inline void delay(unsigned long ms){ fakeNow += ms; }
inline void tone(int,unsigned,unsigned long=0){} inline void noTone(int){}
