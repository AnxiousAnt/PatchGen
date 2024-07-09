<CsoundSynthesizer>
<CsOptions>

</CsOptions>
<CsInstruments>
sr=44100
ksmps=100
nchnls=2


	instr 1

	ihold
iamp	=	p5*200						;convert velocity to amp
inote	=	p4						;MIDI note number
ifreq	=	(2^((inote-69)/12))*440				;MIDI to cps

koff	invalue	"off"						;note number receiving 0 velocity from MIDI
knote	=	inote						;k-rate note number for checking against koff
kdens1	=	ifreq						;k-rate frequency


	if (knote != koff) goto stayon					;if the note number of this note is not the one receiving 0 velocity, stay on

	turnoff							;if this note is receiving 0 velocity, turn off

stayon:

kenv	expsegr	.001, .075, iamp, .01, iamp*.9, .5, .001			;envelope

kcps1	invalue	"kcps1"						;control parameters for grain3
kphs1	invalue	"kphs1"
kgdur1	invalue	"kgdur1"
kcps2	invalue	"kcps2"
kcps3	invalue	"kcps3"

asig1	grain3	kcps1, kphs1, 0.005, 0, kgdur1, kdens1, 10000, 1, 2, 0, 0
asig2	grain3	kcps1, kphs1, 0.0051, 0, kgdur1, kdens1, 10000, 1, 2, 0, 0
;asig3	grain3	kcps2, kphs1, 0.005, 0, kgdur1, kdens1, 10000, 1, 2, 0, 0
;asig4	grain3	kcps2, kphs1, 0.0051, 0, kgdur1, kdens1, 10000, 1, 2, 0, 0
;asig5	grain3	kcps3, kphs1, 0.005, 0, kgdur1, kdens1, 10000, 1, 2, 0, 0
;asig6	grain3	kcps3, kphs1, 0.0051, 0, kgdur1, kdens1, 10000, 1, 2, 0, 0


;asiglu	=	kenv*(asig1+asig3+asig5)/3
;asigru	=	kenv*(asig2+asig4+asig6)/3
asiglu	=	kenv*asig1
asigru	=	kenv*asig2


asigl	compress	asiglu, asiglu, 0, 80, 85, 4, .007, .002, .04
asigr	compress	asigru, asigru, 0, 80, 85, 4, .007, .002, .04


outs	asigl, asigr

	endin

</CsInstruments>
<CsScore>
;f1 0 8192 10 1 .25 .1 0 0 0 0 0 0 2
f1 0 131073 1 "C:\Csound5\sounds\aceventura-takecare.wav" 0 1 0		;Ace Ventura saying "Take care now, bye-bye then." replace with your choice of soundfile.
f2 0 8192 7 0 1024 1 6144 1 1024 0
f3 0 8192 10 1
f4 600 4 10 1

</CsScore>
</CsoundSynthesizer>