#include <cassert>
#include <cstring>
#include <cstdio>
unsigned long fakeNow = 0; int pinState[32];
static int nextRead, calls;          // what the fake decoder returns next; how often it was called
static bool fakeDecode(unsigned char *buf) {
    calls++; if (!nextRead) return false;
    memset(buf, 0, 5); buf[0] = nextRead; return true;
}
#include "lib.cpp"
const int SHD = 7;
RFIDuino r;
// scan at absolute time t with the given read; report result and whether the decoder ran
static bool scan(unsigned long t, int read, bool expectDecode) {
    fakeNow = t; nextRead = read; int before = calls; byte d[5];
    bool res = r.scanForTag(d);
    assert((calls != before) == expectDecode);
    return res;
}
int main() {
    // slot 160 ms, 25% -> awake 160, asleep 480
    fakeNow = 1000; r.setDutyCycle(0.25);
    assert(pinState[SHD] == LOW);
    assert(!scan(1010, 'A', false));                // first 35 ms: no reads
    assert(!scan(1040, 'A', true));                 // first of pair
    assert( scan(1050, 'A', true));                 // verified
    assert(!scan(1160, 'A', false));                // slot over, nothing pending -> sleep
    assert(pinState[SHD] == HIGH && r.getSleep());
    assert(!scan(1160 + 479, 'A', false) && pinState[SHD] == HIGH);   // still asleep
    assert(!scan(1640, 'A', false) && pinState[SHD] == LOW);          // wakes at +480
    assert(!scan(1674, 'A', false));                // still inside 35 ms
    assert(!scan(1675, 'A', true));                 // first of pair
    assert( scan(1800, 'A', true));                 // slot over but pair pending: one more attempt, completes it
    assert(pinState[SHD] == HIGH);                  // ...then sleeps
    // pending pair discarded when the extra attempt fails
    assert(!scan(2280, 0, false) && pinState[SHD] == LOW);   // wake
    assert(!scan(2315, 'B', true));                 // first of pair
    assert(!scan(2440, 0, true) && pinState[SHD] == HIGH);   // extra attempt fails -> sleep
    assert(!scan(2920, 0, false));                  // wake
    assert(!scan(2955, 'B', true));                 // old B was discarded: this is a new first read
    assert( scan(2960, 'B', true));
    // too-small slot is raised to MIN_SLOT_MS (100); sleep = 100 * 3
    r.slotMs = 50;
    assert(!scan(3055, 'C', false) && pinState[SHD] == HIGH);   // slot ran from 2920: 135 >= 100 -> sleep
    assert(!scan(3354, 0, false) && pinState[SHD] == HIGH);
    assert(!scan(3355, 0, false) && pinState[SHD] == LOW);
    r.slotMs = 160;
    // ignored / always-on
    r.setDutyCycle(0);                              // ignored: still cycling
    assert(!scan(3360, 'D', false));                // inside 35 ms of wake
    r.setDutyCycle(1.0);
    assert(pinState[SHD] == LOW && !r.getSleep());
    assert(!scan(3361, 'D', true));
    assert( scan(3362, 'D', true));                 // normal pair, no timing
    // disabling while asleep wakes the chip
    fakeNow = 4000; r.setDutyCycle(0.5);
    assert(!scan(4200, 0, false) && pinState[SHD] == HIGH);
    r.setDutyCycle(1.0);
    assert(pinState[SHD] == LOW && !r.getSleep());
    puts("all duty-cycle checks passed");
}
