#!/bin/bash
for pd in "${@}"
do
  "${pd}" -version
  timeout 1 "${pd}" -nrt -nogui -nosound -open samplerate~-test.pd -send "pd quit"
  timeout 1 "${pd}" -nrt -nogui -nosound -open onload.pd -send "onload pd open samplerate~-test.pd . ; onload pd quit"
done
