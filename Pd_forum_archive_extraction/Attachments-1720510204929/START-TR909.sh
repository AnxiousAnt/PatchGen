#!/bin/bash
#
#
#
pd_patch="tr909.pd"
pd_install="/usr/bin"
pd_audio="-jack -r 44100 -audiobuf 80 -channels 2"
pd_midi="-nomidi -nogui"
pd_options="-font 10"
pd_path="-path /home/muranyia/pd-externals/"
#
processing_patch="./source/tr909.pde"
processing_install="~/Download/processing-1.1"
#
#
echo "/// TR-909 EMULATION ///"
echo "/// Raul Diaz [2008] ///"
echo "    loading ............"
echo "........................"
#
#
eval "$processing_install/processing $processing_patch & $pd_install/pd $pd_audio $pd_midi $pd_options $pd_path $pd_patch"
#
set +v exit 0
