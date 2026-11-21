#include <cassert>
#include <cstring>
#include <cstdio>
unsigned long fakeNow = 0; int pinState[32];
// scripted reads: 0 = decode fails, otherwise the tag's first byte
static const int *script; static int pos;
static bool fakeDecode(unsigned char *buf) {
    int v = script[pos++]; if (!v) return false;
    memset(buf, 0, 5); buf[0] = v; return true;
}
#include "lib.cpp"
static bool run(RFIDuino &r, const int *s, int n) {   // true iff only the last call returns true
    script = s; pos = 0; byte d[5];
    for (int i = 0; i < n; i++) if (r.scanForTag(d) != (i == n - 1)) return false;
    return true;
}
int main() {
    RFIDuino r;
    int a[] = {'A','A'};           assert(run(r, a, 2));
    int b[] = {'A','B','B'};       assert(run(r, b, 3));   // the old code got stuck here
    int c[] = {0,'A',0,'A'};       assert(run(r, c, 4));
    int d[] = {'A','B','C','D','D'}; assert(run(r, d, 5));
    int e[] = {'C','C'};           assert(run(r, e, 2));   // still works after all of the above
    puts("all scanForTag checks passed");
}
