#!/bin/sh
# Host tests for RFIDuino's scanForTag() and duty-cycle logic. Needs g++. Usage: test/run.sh
# The library's decodeTag() call in scanForTag() is swapped for a scripted fakeDecode() via sed;
# if that call is reworded, the grep below stops the run instead of testing the wrong thing.
set -e
here=$(cd "$(dirname "$0")" && pwd)
lib="$here/../RFIDuino"
out="$here/build"    # inside the repo (git-ignored), not a guessable shared /tmp path; overwritten each run
mkdir -p "$out"
cp "$lib/RFIDuino.h" "$out/"
sed 's/decodeTag(tagData)/fakeDecode(tagData)/' "$lib/RFIDuino.cpp" > "$out/lib.cpp"
grep -q 'fakeDecode(tagData)' "$out/lib.cpp" || { echo "run.sh: decodeTag(tagData) not found in RFIDuino.cpp; update the sed"; exit 1; }
for t in test_scan test_duty; do
  g++ -std=gnu++11 -Wall -Wextra -Werror -I"$out" -I"$here" "$here/$t.cpp" -o "$out/$t"
  "$out/$t"
done
