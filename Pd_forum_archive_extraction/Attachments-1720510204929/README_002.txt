pd-0.34.3 with upsampling/downsampling support + enhanced multi-device support



INSTALL-instructions:
=====================
get a new vanilla pd-0.34.2-source
untar it to <mypath>/pd-0.34.2
run "patch -p1 -d <mypath>/pd-0.34.2 < pd-0.34-3-updown-multidev.diff"
changedir to <mypath>/pd-0.34.2/src
compile it with "make"
install it with "make install"
enjoy


upsampling/downsampling:
========================
have a look at the doc/3.audio.examples/74.up.downsampling.pd help-patch to see how it works


multi-device support:
=====================
note: this is linux/OSS only !!! (no Windof, no ALSA, no RME with old (2.2.x) drivers)
to get a great configuration, you should use soundcards that are somehow synched !
<blabla>
pd used to have multi-device support, but did strange things which i didn't expect.
for example, i am using 2 hammerfall-cards (1 real hammerfall (26 i/o) + 1 hammerfall light (18 i/o)
starting pd with "-outchannels 40" allocated 8 channels on the 1st card (the "real" hammerfall) and 26 channels on the 2nd card (the hammerfall light, which really has only 18 channels); the rest was abandoned.
thus i could only use 26 channels - no need for two cards, since this could be acquired with one single hammerfall too.
</blabla>

with this patch, pd does what i want:
have access to multiple soundcards in various configurations

added features = commandline-options:
-------------------------------------

"-soundindev ..." & "-soundoutdev ..."
lets you specify a list of dsp-devices to be used by pd; devices are separated by commas; numbering starts at 0 (for the first card) !
pe: "-soundindev 1"	uses the 2nd installed card as recording device
pe: "-soundoutdev 2,0" 	uses the 3rd and the 1st installed cards as playback devices

"-inchannels ..." & "-outchannels ..."
you can specify the number of channels for each device
pe: "-inchannels 2"		 have 2 i-channels (no real change)
pe: "-outchannels 2,4,8"	 use 2 channels on the 1st (specified) device, 4 o-channels on the 2nd (specified) device and 8 o-channels on the 3rd (specified) device 


examples:
---------
lets assume, you have a 1st sound-card providing 8 channels, a 2nd card with 16 channels and a last 3rd card with 2 channels
i only mention flags for either in or out devices (but it works for both types)
i assume that there are no other redefined flags (-in/outchannels, -soundin/outdev) used !

"" (no arguments)	: use 2 i/o-channels on the first installed card (this is default)

"-inchannels 26"	: use 8 ichannels on the 1st card (adc~ 1..8), 16 ichannels on the 2nd card (adc~ 9..24) and 2 ichannels on the 3rd card (adc~ 25..26): so the channels are spread over the hw-devices

"-inchannels 8,2"	: use 8 channels on the 1st card (adc~ 1..8) and 2 channels on the 2nd card 

"-outchannels 4,24"	: use 4 channels on the 1st card (dac~ 1..4) and up to 24 channels on the 2nd card (since the 2nd card only provides 16 channels, you will only have another 16 outputs (dac~ 5..20), summing up to 20 outputs)

"-soundindev 2"		: use 2 (default) ichannels on the 3rd card as adc~ 1..2

"-soundindev 0,1"	: use 2 (default) ichannels on the 1st card (adc~ 1..2) and 2 (default) ichannels on the 2nd card (adc~ 3..4)

"-soundoutdev 1,0 -outchannels 8,4": use 8 ochannels on the 2nd card (dac~1..8) and 4 ochannels on the first card (dac~9..12)

"-soundindev 0,1 -outchannels 32": use 16 channels on the 1st card (dac~1..16) and 8 channels on the 2nd card (dac~17..24); the remaining channels are lost. so the channels are spread over the specified hw-devices

"-soundindev 1 -outchannels 16,16": use 16 channels on the 2nd device and 2 (remaining) channels on the 3rd device

"-soundindev 1 -inchannels 22": use 8 channels on the 1st device, the rest is lost



