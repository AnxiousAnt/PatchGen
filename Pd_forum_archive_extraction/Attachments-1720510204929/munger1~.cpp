/*
munger1~ (1.00 release)
a realtime multichannel granulator

a flext (cross-platform PD & Max/MSP) port of
the munger~ object from the PeRColate library (0.9 beta5)
http://www.music.columbia.edu/PeRColate/

Original PeRColate library by:

Dan Trueman http://www.music.princeton.edu/~dan/
R. Luke DuBois's http://www.lukedubois.com/

Flext port and additions by:
Ivica Ico Bukvic http://ico.bukvic.net
Ji-Sun Kim hideaway@vt.edu
http://disis.music.vt.edu
http://www.cctad.vt.edu

Released under GPL license
(whichever is the latest version--as of this release, version 2)
For more info on the GPL license please visit:
http://www.gnu.org/copyleft/gpl.html

Changelog:

3-3-2007 Ji-Sun
*Initial flext port
*Add Verbose message on the leftmost inlet

3-4-2007 Ico
*Redesigned verbose mode (3 modes, 0 off, 1 critical (default), 2 full)
*Added ability to name instances and thus associate post messages with specific instances
*Fixed bug in grate_var distribution
*Additional error and warning messages
*More transparent compiler check (for random() discrepancy)
*Increased maximum number of voices to 200 (that about chokes an AMD64 3000+)

3-6-2007 Ico
*Began implementing 0.9beta6 features (buffer, oneshot, note, stk)
*We should implement association with external buffer via another message in the first buffer (i.e. setextbuffer 0 or name)

3-9-2007 Ico
*Finished porting 0.9beta6 (works on Linux, needs to be tested on OSX and Win32 using Max/MSP)
*Added documentation for spatialize
*fixed bugs with multichannel diffusion
*fixed buffer crash bug (may need to test robustness of dissappearing buffers--it appears that Pd will crash no matter what if you
 connect to external buffer and then delete it in the middle of munger1~ trying to access it. Max/MSP appears to have some kind of implementation
 inside flext for this. May need to check more, but then again, who will ever want to delete buffers in the middle of performance?
*added discretepan option where each grain is sent only to one speaker
*tons of little bugfixes

3-10-07 Ji-Sun
*Set up proper MSVC compile environment
*Fixed oneshot MSVC bug

3-11-2007 Ico
*Further tests on MSVC Max/MSP
*Couple more bug fixes
*Are we done yet?

3-12-2007 Ico
*More bug fixes for cross-platform compatibility
*Cleaned-up float/long/int input parsing
*Added verbose 3 for calculating number of grains/second
*increased NUMVOICES to 500
*increased max channels to 24
*Fixed documentation
*/

#include <flext.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <flstk.h>
#include <ADSR.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0 with STK support
#endif

//MSVC doesn't know random(), while GCC's (at least on Linux) has rand() limit much higher
#ifndef __GNUC__
#define RANDOM (rand())
#else
#define RANDOM (random()%32767)
#endif

//#define TWOPI 6.283185307				 	
#define ONE_OVER_HALFRAND 0.00006103516 	// constant = 1. / 16384.0
#define ONE_OVER_MAXRAND 0.000030517578 	// 1 / 32768
#define NUMVOICES 500						//max number of voices
#define MINSPEED .001 						//minimum speed through buffer on playback
#define ENVSIZE 32
#define ONE_OVER_ENVSIZE .0078125
#define MINSIZE 64							// twice ENVSIZE. minimum grainsize in samples
#define RAND01 (((long)RANDOM * ONE_OVER_MAXRAND) //random numbers 0-1
#define RAND11 (((long)RANDOM - 16384.) * ONE_OVER_HALFRAND) //random numbers -1 to 1
#define WINLENGTH 1024
#define PITCHTABLESIZE 1000 					//max number of transpositions for the "scale" message
#define RECORDRAMP 1000
#define RECORDRAMP_INV 0.001
#define MAXCHANNELS 24

// useful define
#ifndef TWOPI
#define TWOPI		6.28318530717958647692
#endif  // TWOPI

class munger1
	:public flext_dsp
{
	//obligatory flext header (class name, base class name) 
	//featuring setup function to initialize some data
	FLEXT_HEADER_S(munger1, flext_dsp, setup) 

public:
	munger1(int argc, const t_atom *argv);
//memory allocation/free for recordbuf.
	void munger_alloc();
	void munger_free();
	
//window funcs
	void munger_setramp(short argc, t_atom *argv);
	void munger_sethanning( short argc, t_atom *argv);
//scale funcs
	void munger_scale(short argc, t_atom *argv);
	void munger_tempered(short argc, t_atom *argv);
	void munger_smooth(short argc, t_atom *argv);
	
//multichannel func
	void munger_spat(short argc, t_atom *argv);
	void munger_discretepan(short argc, t_atom *argv);

//note funcs
	float newNote(int whichVoice, int newNote);			//creates a new start position for a new note (oneshot grain)
	float newNoteSize(int whichVoice, int newNote);		//creates a size for a new note

//buffersize change
	void munger_bufsize(short argc, t_atom *argv);
//buffersize change (ms)
	void munger_bufsize_ms(short argc, t_atom *argv);

//set maximum number of voices possible
	void munger_maxvoices(short argc, t_atom *argv);
//set number of voices to actually use
	void munger_setvoices(short argc, t_atom *argv);
//set min grain size
	void munger_setminsize(short argc, t_atom *argv);
//turn on/off backwards grains 
	void munger_ambidirectional(short argc, t_atom *argv);
//turn on/off recording
	void munger_record(short argc, t_atom *argv);
//clear buffer
	void munger_clear(short argc, t_atom *argv);

//set overall gain and rand gain range
	void munger_gain(short argc, t_atom *argv);
	void munger_randgain(short argc, t_atom *argv);

//fix position for start of grain playback	
	void munger_setposition( short argc, t_atom *argv);

//post current parameter values
	void munger_poststate(short argc, t_atom *argv);
	void setpower( short argc, t_atom *argv);

//grain funcs
	int		findVoice();
	float	newSetup(int whichVoice);
	float	newSize(int whichOne);
	int		newDirection();
	float	envelope(int whichone, float sample);

//sample buffer funcs
	void	recordSamp(float sample);
	float	getSamp(double where);

	void munger_float(double f);
	void setverbose(short argc, t_atom *argv);

//note funcs
	void munger_note(short argc, t_atom *argv);
	void munger_oneshot(short argc, t_atom *argv);

//external buffer funcs
	void	munger_setbuffer(short argc, t_atom *argv);
	void	munger_clearbuffer();
	bool	munger_checkbuffer();
	float	getExternalSamp(double where);

protected:
// int n: length of signal vector. Loop over this for your signal processing.
// float *const *in, float *const *out: 
//          These are arrays of the signals in the objects signal inlets rsp.
//          oulets.
//	virtual void m_signal(int n, float *const *in, float *const *out);
	virtual void CbSignal();
	void m_float1(float v) { grate = v; tempgrate = v; grate_connected += 1;}
	void m_float2(float v) { grate_var = v; grate_var_connected += 1;}
	void m_float3(float v) { glen = v; glen_connected  += 1;}
	void m_float4(float v) { glen_var = v; glen_var_connected += 1;}
	void m_float5(float v) { gpitch = v; gpitch_connected += 1;}
	void m_float6(float v) { gpitch_var = v; gpitch_var_connected += 1;}
	void m_float7(float v) { gpan_spread = v; gpan_spread_connected += 1;}

	float	maxdelay;
//user controlled vars
    float grate;			//grain rate
	float tempgrate;		//grain rate assessed using variation	
    float grate_var;		//grain rate variation; percentage of grain rate
    float glen; 			//grain length
    float glen_var; 		
    float gpitch; 		 
    float gpitch_var;
    float gpan_spread;		//how much to spread the grains around center
				
	float	pitchTable[PITCHTABLESIZE]; 	//table of pitch values to draw from
	float	twelfth; //1/12
	float	semitone;
	short	smoothPitch;
	int		scale_len;
	
	float gain, randgain;
	float position; //playback position (0-1) (if ==-1, then random, which is default)
	
	float	buflen;
	float	maxsize, minsize;
	float	twothirdBufsize, onethirdBufsize;
	float	initbuflen;
	long	maxvoices;

	char	* name;
	int		verbose;
	int		graincounter;
	int		countsamples;

    //signals connected? or controls...
    short grate_connected;
    short grate_var_connected;
    short glen_connected;
    short glen_var_connected;
    short gpitch_connected;
    short gpitch_var_connected;
    short gpan_spread_connected;
    
    //window stuff
    short doHanning;
    float winTime[NUMVOICES], winRate[NUMVOICES];
    float winTable[WINLENGTH];
    float rampLength; //for simple linear ramp
    
    //voice parameters
    long	gvoiceSize[NUMVOICES];			//sample size
    double	gvoiceSpeed[NUMVOICES];		//1 = at pitch
    double	gvoiceCurrent[NUMVOICES];	//current sample position
    int		gvoiceDirection[NUMVOICES];		//1 = forward, -1 backwards
    int		gvoiceOn[NUMVOICES];			//currently playing? boolean
    long	gvoiceDone[NUMVOICES];			//how many samples already played from grain
    float	gvoiceLPan[NUMVOICES];
    float	gvoiceRPan[NUMVOICES];
    float	gvoiceRamp[NUMVOICES];
    float	gvoiceOneOverRamp[NUMVOICES];
    float	gvoiceGain[NUMVOICES];
    int		voices;
    float	gimme;

    ADSR	gvoiceADSR[NUMVOICES];
    int		gvoiceADSRon[NUMVOICES]; 		//set this to 1 if ADSR is desired instead of symmetrical ramp envelope
    float	gvoiceSpat[NUMVOICES][MAXCHANNELS];
    float	channelGain[MAXCHANNELS];
    float	channelGainSpread[MAXCHANNELS];
    float	notechannelGain[NUMVOICES][MAXCHANNELS];
    float	notechannelGainSpread[NUMVOICES][MAXCHANNELS];
    int		discretepan;
    
    //sample buffer
    float	*recordBuf;
    int		recordOn; //boolean
    int		recordRampVal; //ramp for when toggling record on and off
    int		rec_ramping; //-1 when ramping down, 1 when ramping up, 0 when not ramping. who's a ramp?
    long	recordCurrent;
    
    //other stuff
    long	time; 
    int		power;
    short	ambi;
	int		num_channels;

    float srate, one_over_srate;
    float srate_ms, one_over_srate_ms;

    //external record buffer vars
    t_symbol	*bufname;
    buffer	*l_buf; //t_buffer *l_buf;
    long	l_chan;
    short	externalBuffer;
//////
    //note and oneshot stuff
    short	oneshot;
    int		newnote;
    float	noteTransp[NUMVOICES], noteSize[NUMVOICES], notePan[NUMVOICES], noteGain[NUMVOICES];
    float	noteAttack[NUMVOICES], noteDecay[NUMVOICES], noteSustain[NUMVOICES], noteRelease[NUMVOICES];
    int		noteDirection[NUMVOICES];

private:
	//! Set up a method callback for a variable argument list
	//#define FLEXT_CALLBACK_V(M_FUN) 

	FLEXT_CALLBACK_V(munger_setvoices);
	FLEXT_CALLBACK_V(munger_maxvoices);
	FLEXT_CALLBACK_V(munger_setramp);
	FLEXT_CALLBACK_V(munger_scale);
	FLEXT_CALLBACK_V(munger_bufsize);
	FLEXT_CALLBACK_V(munger_bufsize_ms);
	FLEXT_CALLBACK_V(munger_setminsize);
	FLEXT_CALLBACK_V(setpower);
	FLEXT_CALLBACK_V(munger_record);
	FLEXT_CALLBACK_V(munger_ambidirectional);
	FLEXT_CALLBACK_V(munger_smooth);
	FLEXT_CALLBACK_V(munger_tempered);
	FLEXT_CALLBACK_V(munger_sethanning);
	FLEXT_CALLBACK_V(munger_randgain);
	FLEXT_CALLBACK_V(munger_setposition);
	FLEXT_CALLBACK_V(munger_gain);
	FLEXT_CALLBACK_V(munger_clear);
	FLEXT_CALLBACK_V(munger_poststate);
	FLEXT_CALLBACK_V(setverbose);
///Added. Mar. 6, 2007
	FLEXT_CALLBACK_V(munger_setbuffer);
	FLEXT_CALLBACK_V(munger_spat);
	FLEXT_CALLBACK_V(munger_discretepan);
	FLEXT_CALLBACK_V(munger_note);
	FLEXT_CALLBACK_V(munger_oneshot);

	FLEXT_CALLBACK_F(m_float1);
	FLEXT_CALLBACK_F(m_float2);
	FLEXT_CALLBACK_F(m_float3);
	FLEXT_CALLBACK_F(m_float4);
	FLEXT_CALLBACK_F(m_float5);
	FLEXT_CALLBACK_F(m_float6);
	FLEXT_CALLBACK_F(m_float7);

	static void setup (t_classid c)
	{
		
		FLEXT_CADDMETHOD_(c, 0, "verbose",setverbose);
		FLEXT_CADDMETHOD_(c, 0, "ramptime", munger_setramp);
		FLEXT_CADDMETHOD_(c, 0, "scale",munger_scale);
		FLEXT_CADDMETHOD_(c, 0, "delaylength",munger_bufsize);
		FLEXT_CADDMETHOD_(c, 0, "delaylength_ms",munger_bufsize_ms);
		FLEXT_CADDMETHOD_(c, 0, "minsize",munger_setminsize);
		FLEXT_CADDMETHOD_(c, 0, "power",setpower);
		FLEXT_CADDMETHOD_(c, 0, "record",munger_record);
		FLEXT_CADDMETHOD_(c, 0, "ambidirectional",munger_ambidirectional);
		FLEXT_CADDMETHOD_(c, 0, "smooth",munger_smooth);
		FLEXT_CADDMETHOD_(c, 0, "tempered",munger_tempered);
		FLEXT_CADDMETHOD_(c, 0, "hanning",munger_sethanning);
		FLEXT_CADDMETHOD_(c, 0, "rand_gain",munger_randgain);
		FLEXT_CADDMETHOD_(c, 0, "position",munger_setposition);
		FLEXT_CADDMETHOD_(c, 0, "gain",munger_gain);
		FLEXT_CADDMETHOD_(c, 0, "voices",munger_setvoices);
		FLEXT_CADDMETHOD_(c, 0, "maxvoices",munger_maxvoices);
		FLEXT_CADDMETHOD_(c, 0, "clear",munger_clear);
		FLEXT_CADDMETHOD_(c, 0, "state",munger_poststate);
///Added. Mar. 6, 2007
		FLEXT_CADDMETHOD_(c, 0, "buffer",munger_setbuffer);
		FLEXT_CADDMETHOD_(c, 0, "spatialize",munger_spat);
		FLEXT_CADDMETHOD_(c, 0, "note",munger_note);
		FLEXT_CADDMETHOD_(c, 0, "oneshot",munger_oneshot);
		FLEXT_CADDMETHOD_(c, 0, "discretepan", munger_discretepan);

		FLEXT_CADDMETHOD(c, 1, m_float1);
		FLEXT_CADDMETHOD(c, 2, m_float2);
		FLEXT_CADDMETHOD(c, 3, m_float3);
		FLEXT_CADDMETHOD(c, 4, m_float4);
		FLEXT_CADDMETHOD(c, 5, m_float5);
		FLEXT_CADDMETHOD(c, 6, m_float6);
		FLEXT_CADDMETHOD(c, 7, m_float7);
	}
};

//FLEXT_NEW_V("munger1~", munger1) //with parameter
FLEXT_NEW_DSP_V("munger1~", munger1) //with parameter
//FLEXT_NEW_DSP("munger1~", munger1) //without parameter

munger1::munger1(int argc, const t_atom *argv):maxdelay(3000.), num_channels(2) //argc: argument #, argv:  argument value
	{
		float arg1;
		int i,j;
		int arg2;

		if(argc)
		{
			arg1 = GetAFloat(argv[0]);
			if (arg1 >= 100.) maxdelay = arg1;
			if(argc > 1)
			{
				arg2 = GetAInt(argv[1]);
				if (arg2 < 2) num_channels = 2;
				else num_channels = arg2;
				if (num_channels > MAXCHANNELS) num_channels = MAXCHANNELS;
				post ("munger: number channels = %d", num_channels);
			}

			if (argc > 2)
			{
				name = new char[sizeof (argv[2])+1];
				GetAString(argv[2], name, sizeof (argv[2])+1);
				if (name[0] == '_')
				//if underscore is used to widen object in pd
				{
					delete [] name;
					name = NULL;
				}
			}
		}

		if (!name)
		{
			name = new char[0];
			name = "";
		}

		verbose = 1;
		post("munger1~ %s: maxdelay = %d milliseconds", name, (long)maxdelay);


		AddInSignal("This is Inlet 1"); //e.g. gated signal.		
		AddInFloat("This is Inlet 2"); //float: grain separation
		AddInFloat("This is Inlet 3"); //float: grain rate variation
		AddInFloat("This is Inlet 4"); //float: grain size
		AddInFloat("This is Inlet 5"); //float: grain size variation
		AddInFloat("This is Inlet 6"); //float: grain pitch
		AddInFloat("This is Inlet 7"); //float: grain pitch variation
		AddInFloat("This is Inlet 8"); //float: stereo spread
		
        for( int n = 0; n < num_channels; n++)
		{
			char tmp[32];
			sprintf(tmp, "Output signal channel : %d", n);
			AddOutSignal(tmp); //dry signal
		}
		grate_connected = 0;
		grate_var_connected = 0;
		glen_connected = 0;
		glen_var_connected = 0;
		gpitch_connected = 0;
		gpitch_var_connected = 0;
		gpan_spread_connected = 0;

		srate = Samplerate(); //current sample rate, flext function
		one_over_srate = 1./srate;
		srate_ms = srate/1000.;
		one_over_srate_ms = 1./srate_ms;
		if (recordBuf)
			delete [] recordBuf;
		initbuflen = (float)(maxdelay +50. )* 44.1;
		buflen = initbuflen;
		maxsize = buflen / 3;
		twothirdBufsize = maxsize * 2;
		onethirdBufsize = maxsize;
		minsize = MINSIZE;
		voices = 10;
		gain = 1.1;
		randgain = 0.;
		munger_alloc();
		twelfth = 1./12;
		semitone  = pow(2., 1./12.);
		smoothPitch = 1;
		scale_len = PITCHTABLESIZE;

		grate = 1.;
		tempgrate = 1.;
		grate_var = 0.;
		glen = 1.;
		glen_var = 0.;
		gpitch = 1.;
		gpitch_var = 0.;
		gpan_spread = 0.;
		time = 0;
		position = -1.;
		gimme = 0.;
		power = 1;
		ambi = 0;
		maxvoices = 20;
//Added. Mar. 5. 2007
		oneshot = 0;
		newnote = 0;
     

		for (i = 0; i < NUMVOICES; i++)
		{
			gvoiceSize[i] = 1000;
			gvoiceSpeed[i] = 1.;
			gvoiceCurrent[i] = 0.;
			gvoiceDirection[i] = 1;
			gvoiceOn[i] = 0;
			gvoiceDone[i] = 0;
			gvoiceRPan[i] = .5;
			gvoiceLPan[i] = .5;
			gvoiceGain[i] = 1.;
//Added. Mar. 5. 2007
    		gvoiceADSRon[i] = 0;
    		for(j=0;j<MAXCHANNELS;j++) {
    			gvoiceSpat[i][j] = 0.;
    			notechannelGain[i][j] = 0.;
    			notechannelGainSpread[i][j] = 0.;
    		}
	    	
    		//note and oneshot inits
			noteTransp[i] = 0.;
			noteSize[i] = 100.;
			notePan[i] = 0.5;
			noteGain[i] = 1.;
			noteAttack[i] = 20.;
			noteDecay[i] = 50.;
			noteSustain[i] = 0.3;
			noteRelease[i] = 200.;
		}
//Added. Mar. 5. 2007
		for(i=0;i<MAXCHANNELS;i++) {
			channelGain[i] = 0.;
    			channelGainSpread[i] = 0.;
	    	}

		doHanning = 0; // init hanning window
		for(i = 0; i < WINLENGTH; i++)
		{
			winTable[i] = 0.5 + 0.5*cos(TWOPI * i/WINLENGTH + .5*TWOPI);
		}
		for(i=0; i<PITCHTABLESIZE; i++) 
		{
			pitchTable[i] = 0.;
		}	
		rampLength = 256.;

		//sample buffer
		for( i = 0; i < initbuflen; i++)
			recordBuf[i] = 0;
		recordOn = 1; //boolean
		recordCurrent = 0;
		recordRampVal = 0;
		rec_ramping = 0;

		externalBuffer = 0; //use internal buffer by default
		l_chan = 0; //is there any other choice?
		discretepan = 0; //off by default

		srand(54); //0.54?


	}
float munger1::newNote(int whichVoice, int newNote)
{
	float newPosition;
	int i, temp;
	buffer *b = l_buf;
	
	gvoiceSize[whichVoice] 		= (long)newNoteSize(whichVoice, newNote);
	//gvoiceDirection[whichVoice] 	= newDirection(x);	 
	gvoiceDirection[whichVoice]	= noteDirection[newNote];
	
	if(num_channels == 2) {
		//gvoiceLPan[whichVoice] 		= ((float)rand() - 16384.) * ONE_OVER_MAXRAND * gpan_spread + 0.5;
		//gvoiceRPan[whichVoice]		= 1. - gvoiceLPan[whichVoice];
		//make equal power panning....
		//gvoiceLPan[whichVoice] 		= powf(gvoiceLPan[whichVoice], 0.5);
		//gvoiceRPan[whichVoice] 		= powf(gvoiceRPan[whichVoice], 0.5);
		gvoiceRPan[whichVoice] 		= powf(notePan[newNote], 0.5);
		gvoiceLPan[whichVoice] 		= powf((1. - notePan[newNote]), 0.5);
	}
	else {
		if(notePan[newNote] == -1.) {
			for(i=0;i<num_channels;i++) {
				notechannelGain[whichVoice][i] 		= 1.;	
				notechannelGainSpread[whichVoice][i] = 0.;
			}
		} else {
			for(i=0;i<num_channels;i++) {
				notechannelGain[whichVoice][i] 		= 0.;	//initialize all to 0.
				notechannelGainSpread[whichVoice][i] = 0.;
			}
			temp = (int)notePan[newNote];
			if(temp>=num_channels) temp=0;
			notechannelGain[whichVoice][temp] = 1.;			//update the one we want
		}
		for(i=0;i<num_channels;i++) {
			gvoiceSpat[whichVoice][i] = notechannelGain[whichVoice][i] + ((long)(RANDOM) - 16384.) * ONE_OVER_HALFRAND * notechannelGainSpread[whichVoice][i];
		}
	}

	gvoiceOn[whichVoice] 		= 1;
	gvoiceDone[whichVoice]		= 0;
	gvoiceGain[whichVoice]		= noteGain[newNote];
	
	gvoiceADSRon[whichVoice]		= 1;
	//post("adsr %f %f %f %f", noteAttack[newNote], noteDecay[newNote], noteSustain[newNote], noteRelease[newNote]);
	gvoiceADSR[whichVoice].setAllTimes(noteAttack[newNote]/1000., noteDecay[newNote]/1000., noteSustain[newNote], noteRelease[newNote]/1000.);
	gvoiceADSR[whichVoice].keyOn();
	
/*** set start point; tricky, cause of moving buffer, variable playback rates, backwards/forwards, etc.... ***/

    if(!externalBuffer) {
		// 1. random positioning and moving buffer (default)
		if(position == -1. && recordOn == 1) { 
			if(gvoiceDirection[whichVoice] == 1) {//going forward			
				if(gvoiceSpeed[whichVoice] > 1.) 
					newPosition = recordCurrent - onethirdBufsize - (long)(RANDOM) * ONE_OVER_MAXRAND * onethirdBufsize;
				else
					newPosition = recordCurrent - (long)(RANDOM) * ONE_OVER_MAXRAND * onethirdBufsize;//was 2/3rds
			}
			
			else //going backwards
				newPosition = recordCurrent - (long)(RANDOM) * ONE_OVER_MAXRAND * onethirdBufsize;
		}
		
		// 2. fixed positioning and moving buffer	
		else if (position >= 0. && recordOn == 1) {
			if(gvoiceDirection[whichVoice] == 1) {//going forward			
				if(gvoiceSpeed[whichVoice] > 1.) 
					//newPosition = recordCurrent - onethirdBufsize - position * onethirdBufsize;
					//this will follow more closely...
					newPosition = recordCurrent - gvoiceSize[whichVoice]*gvoiceSpeed[whichVoice] - position * onethirdBufsize;
					
				else
					newPosition = recordCurrent - position * onethirdBufsize;//was 2/3rds
			}
			
			else //going backwards
				newPosition = recordCurrent - position * onethirdBufsize;
		}
		
		// 3. random positioning and fixed buffer	
		else if (position == -1. && recordOn == 0) {
			if(gvoiceDirection[whichVoice] == 1) {//going forward			
				newPosition = recordCurrent - onethirdBufsize - (long)(RANDOM) * ONE_OVER_MAXRAND * onethirdBufsize;
			}	
			else //going backwards
				newPosition = recordCurrent - (long)(RANDOM) * ONE_OVER_MAXRAND * onethirdBufsize;
		}
		
		// 4. fixed positioning and fixed buffer	
		else if (position >= 0. && recordOn == 0) {
			if(gvoiceDirection[whichVoice] == 1) {//going forward			
				newPosition = recordCurrent - onethirdBufsize - position * onethirdBufsize;
			}	
			else //going backwards
				newPosition = recordCurrent - position * onethirdBufsize;
		}
	}
	else {
		if (position == -1.) {
			newPosition = (long)(RANDOM) * ONE_OVER_MAXRAND * b->Frames();
		}
		else  if (position >= 0.) newPosition = position * b->Frames();
	}
	
	return newPosition;
	
}	

//creates a size for a new grain
//actual number of samples PLAYED, regardless of pitch
//might be shorter for higher pitches and long grains, to avoid collisions with recordCurrent
//size given now in milliseconds!
//for oneshot notes, this will also scale the ADSR and make it smaller, if the grainSpeed is high
float munger1::newNoteSize(int whichOne, int newNote)
{
	float newsize, temp, temp2, pitchExponent;
	
	//set grain pitch
	pitchExponent = noteTransp[newNote];
	gvoiceSpeed[whichOne] = gpitch * pow(semitone, pitchExponent);
	
	if(gvoiceSpeed[whichOne] < MINSPEED) gvoiceSpeed[whichOne] = MINSPEED;
	newsize = srate_ms*(noteSize[newNote]);
	//if(newsize > maxsize) newsize = maxsize;
	if(newsize*gvoiceSpeed[whichOne] > maxsize) {
		temp2 = maxsize/gvoiceSpeed[whichOne]; //newsize
		temp = temp2/newsize;
		noteAttack[newNote] *= temp;
		noteDecay[newNote] *= temp;
		noteRelease[newNote] *= temp;
		newsize = temp2;
	}
	//if(newsize < minsize) newsize = minsize;
	return newsize;

}


void munger1::munger_spat(short argc, t_atom *argv)
{
	int i, j;
	
	if (argc)
	{
		for (i=j=0; i < (argc - 1); i+=2)
		{
			channelGain[j] = GetAFloat(argv[i]);
			channelGainSpread[j] = GetAFloat(argv[i+1]);
			if(verbose > 1) post("munger1~ %s: channel gain %d = %f, spread = %f", name, j, channelGain[j], channelGainSpread[j]);
			j++;
		}
	}
}

void munger1::munger_note(short argc, t_atom *argv)
{
	if (oneshot)
	{
		int i, temp;

		if(argc < 8)
		{
			post("munger1~ %s: need 8 args -- transposition, gain, pan, attkT, decayT, susLevel, relT, direction [-1/1]", name);
			return;
		}
	
		newnote++;
	
		if(newnote > voices)
		{
			if(verbose > 0) post("munger1~ %s: too many notes amadeus.", name);
			return;
		}

		noteTransp[newnote] = GetAFloat(argv[0]);
		noteGain[newnote] = GetAFloat(argv[1]);	
		notePan[newnote] = GetAFloat(argv[2]);
		noteAttack[newnote] = GetAFloat(argv[3]);
		noteDecay[newnote] = GetAFloat(argv[4]);
		noteSustain[newnote] = GetAFloat(argv[5]);
		noteRelease[newnote] = GetAFloat(argv[6]);
		noteDirection[newnote] = GetAInt(argv[7]);	
	
		//Stk ADSR bug?
		if (noteSustain[newnote] <= 0.001) noteSustain[newnote] = 0.001;

		noteSize[newnote] = noteAttack[newnote] + noteDecay[newnote] + noteRelease[newnote];
 	}
}

//turn oneshot mode on/off. in oneshot mode, the internal granular voice allocation method goes away
//	so the munger will be silent, except when it receives "note" messages
void munger1::munger_oneshot(short argc, t_atom *argv)
{
	int temp;

	if (argc)
	{
		temp = GetAInt(argv[0]);
		oneshot = temp;
    		if(verbose > 1) post("munger1~ %s: setting oneshot: %d", name, temp);
	}
}


//external buffer stuff
bool munger1::munger_checkbuffer()
{
    if (!l_buf) {
    //if(!l_buf || !l_buf->Ok() || !l_buf->Valid()) {
        post("munger1~ %s: error: no valid buffer defined", name);
        // return zero length
        return false;
    }
    else if (!l_buf->Valid())
    {
	post("munger1~ %s: buffer mysteriously dissapeared...", name);
	munger_clearbuffer();
	return false;
    }
    else if (l_buf->Update())
    {
            // buffer parameters have been updated
            if(l_buf->Valid()) {
                if(verbose > 1) post("munger1~ %s: updated buffer reference",name);
                return true;
            }
            else {
                post("munger1~ %s: error: buffer has become invalid",name);
		munger_clearbuffer();
                return false;
            }
    }
    else return true;
}

void munger1::munger_clearbuffer()
{
    	if (l_buf) {
		delete l_buf;
		l_buf = NULL;
		bufname = NULL;
		externalBuffer = 0;
		if(verbose > 1) post("munger1~ %s: external buffer deleted.", name);
	}
}

void munger1::munger_setbuffer(short argc, t_atom *argv)
{
	if(argc == 0) {
    		// argument list is empty
    		// clear existing buffer
    		if (l_buf) munger_clearbuffer();
	}
	else if(argc == 1 && IsSymbol(argv[0])) {
        	// one symbol given as argument
        	// clear existing buffer
		if (l_buf) munger_clearbuffer();
        	// save buffer name
        	bufname = GetSymbol(argv[0]);
        	// make new reference to system buffer object
        	l_buf = new buffer(bufname);
        	if(!l_buf->Ok()) {
            		if(verbose > 0) post("munger1~ %s: error: buffer %s is currently not valid!", name, GetAString(argv[0]));
        	}
		else
		{
			if(verbose > 1) post("munger1~ %s: successfully associated with the %s buffer.", name, GetAString(argv[0]));
			externalBuffer = 1;
			l_chan = 0;
		}
    	}
    	else {
        	// invalid argument list, leave buffer as is but issue error message to console
        	if(verbose > 0) post("munger1~ %s: error: message argument must be a string.", name);
		if (l_buf) munger_clearbuffer();
    	}
}

void munger1::setverbose(short argc, t_atom *argv)
{
	int temp;

	if (argc)
	{
		temp = GetAInt(argv[0]);
		if (temp < 0) temp = 0;
		if (temp > 3) temp = 3;
		verbose = temp;
		post("munger1~ %s: setting verbose: %d", name, temp);
		if (verbose < 3)
		{
			graincounter = 0;
			countsamples = 0;
		}
	}
}

//grain funcs
float munger1::envelope(int whichone, float sample)
{
	long done = gvoiceDone[whichone];
	long tail = gvoiceSize[whichone] - gvoiceDone[whichone];
	
	if(done < gvoiceRamp[whichone]) sample *= (done*gvoiceOneOverRamp[whichone]);
	else if(tail < gvoiceRamp[whichone]) sample *= (tail*gvoiceOneOverRamp[whichone]);
	
	return sample;
}
//tries to find an available voice; return -1 if no voices available
int munger1::findVoice()
{
	int i = 0, foundOne = -1;
	while(foundOne < 0 && i < voices ) {
		if (!gvoiceOn[i]) foundOne = i;
		i++;
	}
	return foundOne;
}

//creates a new (random) start position for a new grain, returns beginning start sample
//sets up size and direction
//max grain size is BUFLENGTH / 3, to avoid recording into grains while they are playing
float munger1::newSetup(int whichVoice)
{
	float newPosition;
	buffer *b = l_buf;
	int i, tmpdiscretepan;
	
	gvoiceSize[whichVoice] 		= (long)newSize(whichVoice);
	gvoiceDirection[whichVoice] 	= newDirection();
	if( num_channels == 2)
	{
		gvoiceLPan[whichVoice] 		= ((long)(RANDOM) - 16384.) * ONE_OVER_MAXRAND * gpan_spread + 0.5;
		gvoiceRPan[whichVoice]		= 1. - gvoiceLPan[whichVoice];
		//make equal power panning....
		gvoiceLPan[whichVoice] 		= powf(gvoiceLPan[whichVoice], 0.5);
		gvoiceRPan[whichVoice] 		= powf(gvoiceRPan[whichVoice], 0.5);
	}
	else if (discretepan)
	{
		tmpdiscretepan = (int)((long)(RANDOM) * ONE_OVER_MAXRAND * ((float)num_channels + 0.99));
		for(i=0;i<num_channels;i++) {
			if (i == tmpdiscretepan) gvoiceSpat[whichVoice][i] = channelGain[i] + ((long)(RANDOM) - 16384.) * ONE_OVER_HALFRAND * channelGainSpread[i];
			else gvoiceSpat[whichVoice][i] = 0.;
		}			
	}
	else
	{
		for(i = 0; i < num_channels; i++)
			gvoiceSpat[whichVoice][i] = channelGain[i] + ((long)(RANDOM) - 16384.) * ONE_OVER_HALFRAND * channelGainSpread[i];
	}
	gvoiceOn[whichVoice] 		= 1;
	gvoiceDone[whichVoice]		= 0;
	gvoiceGain[whichVoice]		= gain + ((long)(RANDOM) - 16384.) * ONE_OVER_HALFRAND * randgain;

	gvoiceADSRon[whichVoice]		= 0;
	
	if(gvoiceSize[whichVoice] < 2.*rampLength) {
		gvoiceRamp[whichVoice] = .5 * gvoiceSize[whichVoice];
		if(gvoiceRamp[whichVoice] <= 0.) gvoiceRamp[whichVoice] = 1.;
		gvoiceOneOverRamp[whichVoice] = 1./gvoiceRamp[whichVoice];
	}
	else {
		gvoiceRamp[whichVoice] = rampLength;
		if(gvoiceRamp[whichVoice] <= 0.) gvoiceRamp[whichVoice] = 1.;
		gvoiceOneOverRamp[whichVoice] = 1./gvoiceRamp[whichVoice];
	}
	
	
/*** set start point; tricky, cause of moving buffer, variable playback rates, backwards/forwards, etc.... ***/

    if(!externalBuffer) {
	// 1. random positioning and moving buffer (default)
	if(position == -1. && recordOn == 1) { 
		if(gvoiceDirection[whichVoice] == 1) {//going forward			
			if(gvoiceSpeed[whichVoice] > 1.) 
				newPosition = recordCurrent - onethirdBufsize - (long)RANDOM * ONE_OVER_MAXRAND * onethirdBufsize;
			else
				newPosition = recordCurrent - (long)RANDOM * ONE_OVER_MAXRAND * onethirdBufsize;//was 2/3rds
		}
		
		else //going backwards
			newPosition = recordCurrent - (long)RANDOM * ONE_OVER_MAXRAND * onethirdBufsize;
	}
	
	// 2. fixed positioning and moving buffer	
	else if (position >= 0. && recordOn == 1) {
		if(gvoiceDirection[whichVoice] == 1) {//going forward			
			if(gvoiceSpeed[whichVoice] > 1.) 
				//newPosition = recordCurrent - onethirdBufsize - position * onethirdBufsize;
				//this will follow more closely...
				newPosition = recordCurrent - gvoiceSize[whichVoice]*gvoiceSpeed[whichVoice] - position * onethirdBufsize;
				
			else
				newPosition = recordCurrent - position * onethirdBufsize;//was 2/3rds
		}
		
		else //going backwards
			newPosition = recordCurrent - position * onethirdBufsize;
	}
	
	// 3. random positioning and fixed buffer	
	else if (position == -1. && recordOn == 0) {
		if(gvoiceDirection[whichVoice] == 1) {//going forward			
			newPosition = recordCurrent - onethirdBufsize - (long)RANDOM * ONE_OVER_MAXRAND * onethirdBufsize;
		}	
		else //going backwards
			newPosition = recordCurrent - (long)RANDOM * ONE_OVER_MAXRAND * onethirdBufsize;
	}
	
	// 4. fixed positioning and fixed buffer	
	else if (position >= 0. && recordOn == 0) {
		if(gvoiceDirection[whichVoice] == 1) {//going forward			
			newPosition = recordCurrent - onethirdBufsize - position * onethirdBufsize;
		}	
		else //going backwards
			newPosition = recordCurrent - position * onethirdBufsize;
	}
    }
    else {
	if (position == -1.) {
		newPosition = (float)(RANDOM * ONE_OVER_MAXRAND * (float)(b->Frames()));
	}
	else if (position >= 0.) newPosition = position * (float)b->Frames();
    }
	
    return newPosition;
	
}
//creates a size for a new grain
//actual number of samples PLAYED, regardless of pitch
//might be shorter for higher pitches and long grains, to avoid collisions with recordCurrent
//size given now in milliseconds!
float munger1::newSize(int whichOne)
{
	float newsize, temp;
	int pitchChoice, pitchExponent;
	
	//set grain pitch
	if(smoothPitch == 1) gvoiceSpeed[whichOne] = gpitch + ((long)RANDOM - 16384.) * ONE_OVER_HALFRAND*gpitch_var;
	else {
		//temp = (long)RANDOM * ONE_OVER_MAXRAND * gpitch_var * (float)PITCHTABLESIZE;
		temp = (long)RANDOM * ONE_OVER_MAXRAND * gpitch_var * (float)scale_len;
		pitchChoice = (int) temp;
		if(pitchChoice > PITCHTABLESIZE) pitchChoice = PITCHTABLESIZE;
		if(pitchChoice < 0) pitchChoice = 0;
		pitchExponent = (int)pitchTable[pitchChoice];
		gvoiceSpeed[whichOne] = gpitch * pow(semitone, pitchExponent);
	}
	
	if(gvoiceSpeed[whichOne] < MINSPEED) gvoiceSpeed[whichOne] = MINSPEED;
	newsize = srate_ms*(glen + ((long)RANDOM - 16384.) * ONE_OVER_HALFRAND * glen_var);
	if(newsize > maxsize) newsize = maxsize;
	if(newsize*gvoiceSpeed[whichOne] > maxsize) 
		newsize = maxsize/gvoiceSpeed[whichOne];
	if(newsize < minsize) newsize = minsize;
	return newsize;

}	
int munger1::newDirection()
{
//-1 == always backwards
//0  == backwards and forwards (default)
//1  == only forwards
	int dir;
	if(ambi == 0) {
		dir = RANDOM- 16384;
		if (dir < 0) dir = -1;
		else dir = 1;
	} 
	else 
		if(ambi == -1) dir = -1;
	else
		dir = 1;

	return dir;
}

//buffer funcs
void munger1::recordSamp(float sample)
{
	if(recordCurrent >= buflen) recordCurrent = 0;
	
	if(recordOn) {
		if(rec_ramping == 0) recordBuf[recordCurrent++] = sample; //add sample
		else { //ramp up or down if turning on/off recording
			recordBuf[recordCurrent++] = sample * RECORDRAMP_INV * (float)recordRampVal;
			recordRampVal += rec_ramping;
			if(recordRampVal <= 0) {
				rec_ramping = 0;
				recordOn = 0;
			}
			if(recordRampVal >= RECORDRAMP) rec_ramping = 0;
		}
	}
}

float munger1::getSamp(double where)
{
	double alpha, om_alpha, output;
	long first;
	
	while(where < 0.) where += buflen;
	while(where >= buflen) where -= buflen;
	
	first = (long)where;
	
	alpha = where - first;
	om_alpha = 1. - alpha;
	
	output = recordBuf[first++] * om_alpha;
	if(first <  buflen) {
		output += recordBuf[first] * alpha;
	}
	else {
		output += recordBuf[0] * alpha;
	}
	
	return (float)output;	
}

float munger1::getExternalSamp(double where)
{
	double alpha, om_alpha, output;
	long first;
	
	buffer *b = l_buf;
	float *tab;
	double frames, sampNum, nc;
	
	if (!b) return 0.;

	tab = b->Data();
	frames = (double)b->Frames();
	nc = (double)b->Channels(); 		//== buffer~ framesize...

	if (where < 0.) where = 0.;
	else if (where >= frames) where = 0.;
	
	sampNum = where*nc;
	
	first = (long)sampNum;
	
	alpha = sampNum - first;
	om_alpha = 1. - alpha;
	
	output = tab[first] * om_alpha;
	first += (long)nc;
	output += tab[first] * alpha;
	
	return (float)output;	
}

void munger1::munger_setramp(short argc, t_atom *argv)
{
	doHanning = 0;

	if (argc)
	{
		rampLength = srate_ms * GetAInt(argv[0]); 
		if(rampLength <= 0.) rampLength = 1.;
		if (verbose > 1) post("munger1~ %s: setting ramp to: %d ms", name, rampLength);
	}
}

void munger1::munger_scale(short argc, t_atom *argv)
{
	int i,j;
	
	if (verbose > 1) post("munger1~ %s: loading scale from input list", name);
	smoothPitch = 0;
	
	for(i=0;i<PITCHTABLESIZE;i++) pitchTable[i] = 0.;
	if (argc > PITCHTABLESIZE) argc = PITCHTABLESIZE;
	for (i=0; i < argc; i++)
	{
		pitchTable[i] = GetAFloat(argv[i]);
	}
	
	scale_len = argc;
	
	i = 0;

	//wrap input list through all of pitchTable
	for (j=argc; j < PITCHTABLESIZE; j++) {
		pitchTable[j] = pitchTable[i++];
		if (i >= argc) i = 0;
	}
		
}

void munger1::munger_bufsize(short argc, t_atom *argv)
{
	float temp;
	
	if (argc)
	{
		temp = srate * GetAFloat(argv[0]);
		if(temp < 20.*(float)MINSIZE)
		{
			temp = 20.*(float)MINSIZE;
			if (verbose > 0) post("munger1~ %s error: delaylength too small!", name);
		}
		//if(temp > (float)BUFLENGTH) temp = (float)BUFLENGTH;
		if(temp > initbuflen)
		{
			temp = initbuflen;
			if (verbose > 0) post("munger1~ %s error: delaylength too large!", name);
		}
		//buflen = temp;
		//maxsize = buflen / 3.;
		maxsize = temp / 3.;
    		twothirdBufsize = maxsize * 2.;
    		onethirdBufsize = maxsize; 
    		if (verbose > 1) post("munger1~ %s: setting delaylength to: %f seconds", name, temp/srate);

	}
}
void munger1::munger_bufsize_ms(short argc, t_atom *argv)
{
	float temp;

	if (argc)
	{
		temp = srate_ms * GetAFloat(argv[0]);
		if(temp < 20.*(float)MINSIZE)
		{
			temp = 20.*(float)MINSIZE;
			if (verbose > 0) post("munger1~ %s error: delaylength_ms too small!", name);
		}
		//if(temp > (float)BUFLENGTH) temp = (float)BUFLENGTH;
		if(temp > initbuflen)
		{
			temp = initbuflen;
			if (verbose > 0) post("munger1~ %s error: delaylength_ms too large!", name);
		}
		//buflen = temp;
		//maxsize = buflen / 3.;
		maxsize = temp / 3.;
    		twothirdBufsize = maxsize * 2.;
    		onethirdBufsize = maxsize; 
    		if (verbose > 1) post("munger1~ %s: setting delaylength to: %d milliseconds", name, (long)(temp/srate_ms));
	}
}
void munger1::munger_setminsize(short argc, t_atom *argv)
{
	float temp;

	if (argc)
	{
		temp = srate_ms * GetAFloat(argv[0]);
		if(temp < (float)MINSIZE)
		{
			temp = (float)MINSIZE;
			if (verbose > 0) post("munger1~ %s error: delaylength too small!", name);
		}
		if(temp >= initbuflen) {
			temp = (float)MINSIZE;
			if (verbose > 0) post("munger1~ %s error: minsize too large!", name);
		}
		minsize = temp;
    		if (verbose > 1) post("munger1~ %s: setting min grain size to: %f ms", name, (minsize/srate_ms));

	}
}

void munger1::munger_discretepan(short argc, t_atom *argv)
{
	int temp;

	if (argc)
	{
		temp = GetAInt(argv[0]);
		if(temp < 0)
		{
			if (verbose > 0) post("munger1~ %s error: discretepan can be only 0 or 1!", name);
			temp = 0;
		}
		if(temp > 1)
		{
			if (verbose > 0) post("munger1~ %s error: error: discretepan can be only 0 or 1!", name);
			temp = 1;
		}
		discretepan = temp;

	}
}

void munger1::munger_setvoices(short argc, t_atom *argv)
{
	int temp;

	if (argc)
	{
		temp = GetAInt(argv[0]);
		if(temp < 0)
		{
			if (verbose > 0) post("munger1~ %s error: voices has to be between 0 and maxvoices (currently %d)!", name, maxvoices);
			temp = 0;
		}
		if(temp > maxvoices)
		{
			if (verbose > 0) post("munger1~ %s error: voices has to be between 0 and maxvoices (currently %d)!", name, maxvoices);
			temp = maxvoices;
		}
		voices = temp;
    		if (verbose > 1) post("munger1~ %s: setting voices to: %d ", name, voices);

	}
}

void munger1::munger_maxvoices(short argc, t_atom *argv)
{
	int temp;

	if (argc)
	{
		temp = GetAInt(argv[0]);
		if(temp < 0)
		{
			if (verbose > 0) post("munger1~ %s error: maxvoices cannot be less than 0!", name);
			temp = 0;
		}
		if(temp > NUMVOICES) temp = NUMVOICES;
		maxvoices = temp;
    		if (verbose > 1) post("munger1~ %s: setting max voices to: %d ", name, maxvoices);
	}
}

void munger1::setpower(short argc, t_atom *argv)
{
	int temp;

	if (argc)
	{
		temp = GetAInt(argv[0]);
		power = temp;
    		post("munger1~ %s: setting power: %d", name, temp);
	}
}
void munger1::munger_record(short argc, t_atom *argv)
{
	int temp;

	if (argc)
	{
		temp = GetAInt(argv[0]);
		//recordOn = temp;
		if (!temp)
		{
			recordRampVal = RECORDRAMP;
			rec_ramping = -1;
		}
		else
		{
			recordOn = 1;
			recordRampVal = 0;
			rec_ramping = 1;
		}
    		if (verbose > 1) post("munger1~ %s: setting record: %d", name, temp);
	}
}

void munger1::munger_ambidirectional(short argc, t_atom *argv)
{
	int temp;

	if (argc)
	{
		temp = GetAInt(argv[0]);
		ambi = temp;
    		if (verbose > 1) post("munger1~ %s: setting ambidirectional: %d", name, temp);
	}
}

void munger1::munger_gain(short argc, t_atom *argv)
{
	float temp;

	if (argc)
	{
		temp = GetAFloat(argv[0]);
    		if (verbose > 1) post("munger1~ %s: setting gain to: %f ", name, temp);
    		gain = temp;
	}
}

void munger1::munger_setposition(short argc, t_atom *argv)
{
	float temp;

	if (argc)
	{
		temp = GetAFloat(argv[0]);
		if (temp > 1.) temp = 1.;
    		if (temp < 0.) temp = -1.;
    		if (verbose > 1) post("munger1~ %s: setting position to: %f ", name, temp);
    		position = temp;
	}
}

void munger1::munger_randgain(short argc, t_atom *argv)
{
	float temp;

	if (argc)
	{
		temp = GetAFloat(argv[0]);
    		if (verbose > 1) post("munger1~ %s: setting rand_gain to: %f ", name, temp);
    		randgain = temp;
	}
}

void munger1::munger_sethanning(short argc, t_atom *argv)
{
	if (verbose > 1) post("munger1~ %s: hanning window is busted", name);
	doHanning = 1;
}

void munger1::munger_tempered(short argc, t_atom *argv)
{
	int i;
	if (verbose > 1) post("munger1~ %s: doing tempered scale", name);
	smoothPitch = 0;
	scale_len = 100;
	for(i=0; i<scale_len-1; i += 2) {
		pitchTable[i] = 0.5*i;
		pitchTable[i+1] = -0.5*i;
	}
	scale_len = PITCHTABLESIZE;
}
void munger1::munger_smooth(short argc, t_atom *argv)
{
	if (verbose > 1) post("munger1~ %s: doing smooth scale", name);
	smoothPitch = 1;
}

void munger1::munger_alloc()
{
	//recordBuf = t_getbytes(BUFLENGTH * sizeof(float));
	//recordBuf = (float *)t_getbytes((size_t)buflen * sizeof(float));
	recordBuf = new float[buflen];
	if (!recordBuf)
	{
		error("munger1~ %s: out of memory", name);
		return;
	}
}

void munger1::munger_clear(short argc, t_atom *argv)
{
	long i;
	for(i=0; i<initbuflen; i++) recordBuf[i] = 0.;
}

void munger1::munger_free()
{
	//recordBuf = (float*)sysmem_newptr(buflen * sizeof(float));
	if( recordBuf)
		//t_freebytes(recordBuf, (size_t)initbuflen * sizeof(float));
		delete [] recordBuf;
	if( !recordBuf)
	{
		error("munger1~ %s: out of memory", name);
		return;
	}
}

//efine our DSP function. It gets this arguments:
// 
// int n: length of signal vector. Loop over this for your signal processing.
// float *const *in, float *const *out: 
//          These are arrays of the signals in the objects signal inlets rsp.
//          oulets. 

//void munger1::m_signal(int n, float *const *in, float *const *out_m)
void munger1::CbSignal()
{
	//ins1 holds the signal vector ofthe first inlet, index 0
	//post("m_signal: n=%d", n);

	int n = Blocksize();
	const float *ins1 = InSig(0);
		
	float outsamp[MAXCHANNELS], samp;
	int newvoice, i, j;
	float *out[MAXCHANNELS] ;

	for (i=0;i<num_channels;i++) {
		out[i] = (float *)(OutSig(i));
	}  

	//bypass stuff
	//if(thisHdr()->z_disabled) goto out;
		
	if(gpan_spread > 1.) gpan_spread = 1.;
	if(gpan_spread < 0.) gpan_spread = 0.;
	
	if(!power) {
		while(n--) {
			for(i = 0; i < num_channels; i++) {
				*out[i]++ = 0.;
			}
		}
	}

	else {
		while(n--) {
			if (verbose > 2)
			{
				countsamples++;
				if (countsamples >= ((int)srate - 1))
				{
					post("munger1~ %s: outputting %d grains per second", name, graincounter);
					graincounter = 0;
					countsamples = 0;
				}
			}
			for( i = 0; i < num_channels; i++) outsamp[i] = 0.;
			//record a sample
			//if(recordOn) recordSamp(x, *in++);
			recordSamp(*ins1++);

			//grab a note if requested; works in oneshot mode or otherwise
			if (oneshot)
			{
				while(newnote > 0)
				{
					newvoice = findVoice();
					if(newvoice >= 0)
					{
						gvoiceCurrent[newvoice] = newNote(newvoice, newnote);
					}
					newnote--;
				}
			}
			//find a voice if it's time (high resolution). ignore if in "oneshot" mode
			else
			{
				if(time++ >= (long)gimme) {
					time = 0;
					newvoice = findVoice();
					if(newvoice >= 0) {
						gvoiceCurrent[newvoice] = newSetup(newvoice);
					}
					tempgrate = grate + ((long)RANDOM - 16384.) * ONE_OVER_HALFRAND * grate_var;
					gimme = srate_ms * tempgrate; //grate is actually time-distance between grains
				}
			}
			time++;
			//mix 'em, pan 'em
			for(i=0; i< maxvoices; i++) {
				if(gvoiceOn[i]) {
					//get a sample, envelope it
					if(externalBuffer) samp = getExternalSamp(gvoiceCurrent[i]);
					else samp = getSamp(gvoiceCurrent[i]);
					if (!gvoiceADSRon[i]) samp = envelope(i, samp) * gvoiceGain[i];
					else samp = samp * gvoiceADSR[i].tick() * gvoiceGain[i]; //ADSR_ADRtick->computeSample()
					//post("tick %d: %f", i, gvoiceADSR[i].tick());
					//else samp = samp * ADSR_ADRtick(gvoiceADSR[i]) * gvoiceGain[i];
					//samp = envelope(i, samp) * gvoiceGain[i];
					//pan it
					if(num_channels == 2) {
						outsamp[0] += samp * gvoiceLPan[i];
						outsamp[1] += samp * gvoiceRPan[i];
					}
					else { //multichannel subroutine
						for(j=0;j<num_channels;j++) {
							outsamp[j] += samp * gvoiceSpat[i][j];
						}
					}
					
					//see if grain is done after jumping to next sample point
					gvoiceCurrent[i] += (double)gvoiceDirection[i] * (double)gvoiceSpeed[i];
					if (!gvoiceADSRon[i]) {
						if(gvoiceDone[i] >= gvoiceSize[i] || ++gvoiceDone[i] >= gvoiceSize[i])
						{
							gvoiceOn[i] = 0;
						}
					}
					else {
						if(gvoiceADSR[i].getState() == gvoiceADSR[i].SUSTAIN) gvoiceADSR[i].keyOff();
						if(++gvoiceDone[i] >= gvoiceSize[i] || gvoiceADSR[i].getState() == gvoiceADSR[i].DONE) 
						{
							gvoiceOn[i] = 0;
						}
					}
					if (!gvoiceOn[i]) graincounter++;
				}
			}
			for(i=0;i<num_channels;i++) {
				*out[i]++ = outsamp[i];
			}
			//power = 0;
		}
	}
}

void munger1::munger_poststate(short argc, t_atom *argv)
{
	post (">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	post ("***CURRENT munger1~ %s PARAMETER VALUES***:", name);
	post ("all times in milliseconds");
	post ("grain spacing = %f", grate);	
    	post ("grain spacing variation = %f", grate_var);
    	post ("grain length = %f", glen);
    	post ("grain length variation = %f", glen_var);				 
    	post ("grain transposition multiplier = %f", gpitch);
    	post ("grain transposition multiplier variation = %f", gpitch_var);
    	post ("panning spread = %f", gpan_spread);
	post ("grain gain = %f", gain);
	post ("grain gain variation = %f", randgain);
	post ("playback position (-1 = random) = %f", position);
	post ("grain playback direction (0 = both) = %d", ambi);
	post ("buffer length = %f", buflen * one_over_srate_ms);
	post ("max grain size = %f", maxsize * one_over_srate_ms);
	post ("min grain size = %f", minsize * one_over_srate_ms);
	post ("max number of voices = %ld", maxvoices);
	post ("current number of voices = %d", voices);
	post ("grain envelope (ramp) length = %f", rampLength * one_over_srate_ms);
	post ("recording? = %d", recordOn);
	post ("power = %d", power);
	post ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
}
