#!/bin/bash
echo "######## batch off ########"
time pd -noprefs -noaudio -nogui -stderr -r 48000 -open "test2.pd" -send "RENDER 50 batch-off.wav"
hexdump -C batch-off.wav | head
echo "######## batch on  ########"
time pd -noprefs -batch          -stderr -r 48000 -open "test2.pd" -send "RENDER 50 batch-on.wav"
hexdump -C batch-on.wav  | head
diff -s batch-off.wav batch-on.wav
