<CsoundSynthesizer>
<CsOptions>

</CsOptions>
<CsInstruments>
sr=48000
ksmps=10
nchnls=1

	instr 2


kfc	invalue	"freq"
kq	invalue	"q"
kamp	invalue	"gain"
kfilter	invalue	"filter"

arawl	in

	if kfilter == 1 goto rezzy
	if kfilter == 2 goto butlp
	if kfilter == 3 goto butbp
	if kfilter == 4 goto res
	if kfilter == 5 goto lp2
	if kfilter == 6 goto ton

raw:
afilt	=	arawl
	goto skip
rezzy:
afilt	rezzy	arawl, kfc, kq
	goto skip
butlp:
afilt	butterlp	arawl, kfc
	goto skip
butbp:
afilt	butterbp	arawl, kfc, kq
	goto skip
res:
afilt	reson	arawl, kfc, kq
	goto skip
lp2:
afilt	lowpass2	arawl, kfc, kq
	goto skip
ton:
afilt	tone	arawl, kfc


skip:

aout	=	afilt*kamp




	out	aout
	endin


</CsInstruments>
<CsScore>
f1 3600 16 10 1

i2 0 3600


</CsScore>
</CsoundSynthesizer>