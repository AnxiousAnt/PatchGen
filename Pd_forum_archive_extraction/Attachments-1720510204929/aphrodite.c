/* APHRODITE: frequency independent mobility OYMB   by Chris Penrose and Kenkyuukai input */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include <pv.h>
#include <helper.h>

int srate = 44100;
int currentFrame = 0;
int D = 0;

typedef struct _FreqLoops {

  int		startFrame,
		loopCount,
		memberCount,
  		*members;

  float		soundPhase,
		currentCount,
		increment;
} FreqLoops;


typedef struct _Kiev {

  int		on;
  int		frameCount;
  float		atten;
} Kiev;


int main(argc, argv)
int argc; char **argv;
{

  int		N = 1024,
		N2,
		Nw = 0,
		Nw2,
		loopCount = 0,
		memberBand = 0,
		I = 0,
		N_Frames = 0,

/* allows process to run for over 72 days
   when N=1024, D=128 and R=44100  */

    		endFrame = INT_MAX,	
		fileEnd,
		i,j,k,
		in,
		on, 
		obank = 0,
		outbufsize = 16384,
		hippoIndex = 0,
		*bitshuffle,
		recalculate = 0,
		mouse = 0,
		fastRefresh,
		currentIndex = 0,
		recycle = 0,
		maxMembers,
		isatty();

  long		seed = 0;

  float		memSize = .5,
		groupSize = 3.,
		fundamental,
		refreshRate = .05,
		prob = .1,
		duration = 10.,
		refreshIndex = 0.,
		attenBand = 1,
		attenBase = -1,
		onCountBase = 3.,
  		offCountBase = 5.,
		durationBand = 1.,
		magnitude = 4.,
		timeAdjust = 1.,
		magIndex,
		interpIndex,
		fframes,
		startScalar = 0.,
    		durationScalar = .5,
		startConstant = 0.,
    		durationConstant = .5,
		P = 0.,
		scalar = 1.,
		*cortex,
		*Hwin,
		*Wanal,
		*Wsyn,
		*input,
		*winput,
		*buffer,
		*channel,
		*output,
		*trigland,
		**hippocamp;

  char		ch;

  void		sayonara();

  FreqLoops	*loopinfo;

  Kiev		*gates;

  if (argc < 2)
    usage(1);


  while( (ch= crack( argc, argv,
  	"R|N|M|I|i|d|p|P|a|v|w|W|l|L|o|O|A|b|B|p|g|e|k|K|x|X|H|T|mtfrch",
		     0  )) != 0 ) {

    switch(ch) {
    	case 'R':	srate = atoi(arg_option);
			break;
	case 'N':	N = atoi(arg_option);
			break;
	case 'M':	Nw = atoi(arg_option);
			break;
	case 'I':	I = atoi(arg_option);
			break;
	case 'a':	scalar = atof(arg_option);
	    		break;
	case 'v':	magnitude = atof(arg_option);
	    		break;
	case 'd':	duration = atof(arg_option);
	  		break;
	case 'l':	startScalar = atof(arg_option);
	    		break;
	case 'L':	durationScalar = atof(arg_option);
	    		break;
	case 'k':	startConstant = atof(arg_option);
	    		break;
	case 'K':	durationConstant = atof(arg_option);
	    		break;
	case 'T':	timeAdjust = atof(arg_option);
	    		break;
	case 'e':	refreshRate = atof(arg_option);
	    		break;
	case 'H':	memSize = atof(arg_option);
	    		break;
	case 'o':	onCountBase = atof(arg_option);
	    		break;
	case 'O':	offCountBase = atof(arg_option);
	    		break;
	case 'B':	durationBand = atof(arg_option);
	    		break;
	case 'A':	attenBase = atof(arg_option);
	    		break;
	case 'b':	attenBand = atof(arg_option);
	    		break;
	case 'g':	groupSize = atof(arg_option);
	    		break;
        case 'x':	memberBand = atoi(arg_option);
	    		break;
        case 'X':	seed = atoi(arg_option);
	    		break;
	case 'p':	prob = atof(arg_option);
	    		break;
    	case 'r':	recalculate = 1;
	  		break;
	case 'm':	mouse = 1;
	    		break;
	case 'c':	recycle = 1;
	    		break;
	case 'h':	usage(1);
    }
  }

  if (duration == 0.) {
      fprintf(stderr,
	      "You must specify a duration\n");
      exit(0);
  }

  /* trap control-c signal, remap it to our sayonara() function */

  else {

    signal(SIGINT, sayonara);
  }

  /* use time of day to seed random number generator if no seed present */

  if (seed == 0) {

    struct timeval      tp;
    struct timezone     tzp;

    gettimeofday(&tp, &tzp);

    srrand((int) tp.tv_usec);

    fprintf(stderr,"random seed: %d\n", (int) tp.tv_usec);
  }

  else
    srrand(seed);

  if (Nw == 0)
    Nw = N;

  if (I == 0)
    I = Nw / 8;

  if (D == 0)
    D = I;

  /* convert duration units (seconds) into frame units */

  if (duration != 0.)
    endFrame = (unsigned long) ( (duration * ( (float) srate /
					       (float) I )) + .5 );

  /* calculate the size of our analysis memory, memSize is given in units 
   of seconds */

  N_Frames = (int) ((memSize * ((float) srate / (float) I)) + .5);

  fframes = (float) (N_Frames - 1);

  /* make sure that the startScale and startConstant add up to 1.0 or less */

  if (startScalar + startConstant > 1.0) {
    startScalar /= (startScalar + startConstant);
    startConstant /= (startScalar + startConstant);
  }

  if (startScalar < 0.)
    startScalar = 0.;

  if (startConstant < 0.)
    startConstant = 0.;

  /* make sure that the durationScale and durationConstant
     add up to 1.0 or less */

  if (durationScalar + durationConstant > 1.0) {
    durationScalar /= (durationScalar + durationConstant);
    durationConstant /= (durationScalar + durationConstant);
  }

  if (durationScalar < 0.)
    durationScalar = 1. / fframes;

  if (durationConstant < 0.)
    durationConstant = 0.;

  /* how to support refresh rates where the rate is potentially < 1 < ??? */
  /* do we use a conditional to discriminate? or is that somehow unnecessary */
  
  if (refreshRate > 1.) {
    fastRefresh = 1;
    refreshIndex = refreshRate;
  }

  else {

    /* use inverse of refreshIndex if it is greater than 1. */

    refreshRate = 1. / refreshRate;
    fastRefresh = 0;
  }

  /* filter gate parameter translation */ 

  onCountBase = (onCountBase *
		 ((float) srate / (float) I));

  offCountBase = (offCountBase *
		 ((float) srate / (float) I));
  
  durationBand = (durationBand *
		 ((float) srate / (float) I));

  myPI = 4. * atan(1.);
  myTWOPI = 8. * atan(1.);
  obank = P != 0.;
  N2 = N>>1;
  Nw2 = Nw>>1;

  scalar *= (32767. / (float) N2);

  Wanal = (float *) space( Nw, sizeof(float) );
  Wsyn = (float *) space( Nw, sizeof(float) );
  Hwin = (float *) space( Nw, sizeof(float) );
  winput = (float *) space( Nw, sizeof(float) );
  buffer = (float *) space( N, sizeof(float) );	   
  channel = (float *) space( N+2, sizeof(float) );

  /* allocate and clear i/o buffers */

  input = (float *) zerospace( Nw, sizeof(float) );
  output = (float *) zerospace( Nw, sizeof(float) );


  cortex = (float *) space( N+2, sizeof(float) );

  gates = (Kiev *) space( N2+1, sizeof(Kiev) );

  loopinfo = (FreqLoops *) space( N2+1, sizeof( FreqLoops ) );

  fundamental = (float) srate / (float) N;

  maxMembers = (( (int) (((float) srate / 2) 
		/ fundamental) ) - 
		( (int) (( ((float) srate / 2)
		/ groupSize) / fundamental) )) + 1;

  for ( i=0; i < N2+1; i++ )
    (loopinfo+i)->members = (int *) space( maxMembers, sizeof( int ) );


  hippocamp = (float **) space(N_Frames+1, sizeof(float *));

  for ( i=0; i <= N_Frames; i++ )
    *(hippocamp+i) = (float *) space(N+2, sizeof(float));

/* FFT cosine funk */
  trigland = (float *) space( N2, sizeof(float) ); 
    
/* FFT bit shuffle */
  bitshuffle = (int *) space( 3+(int)sqrt((float) (N>>1)), sizeof(int) );
  
  /* initialize FFT needs */
    
  init_rdft( N, bitshuffle, trigland );

/* generate our hamming windows for analysis (not necessary in unite)
   and resynthesis */

  makewindows( Hwin, Wanal, Wsyn, Nw, N, I, obank );


  /* load in delta functions */

  in = -Nw;
  on = in;

  fileEnd = 0;

/* read initial sound and process */

  for( i = 0; i < N_Frames; i++ ) {

    if ( fread( *(hippocamp+i), sizeof(float), N+2, stdin ) == 0 ) {

      fileEnd = 1;
      N_Frames = i;
      fframes = N_Frames - 1;
      break;
    }
  }

/* build our loop groups */



  while (currentIndex < N2) {

    int 	currentCount;

    float	currentFrequency = fundamental * (currentIndex + 1);

    /* first, determine how many members will be in the current group */
   
    currentCount = ((int) ((currentFrequency * groupSize)
			   / fundamental)) - currentIndex;
 
    if ( currentCount > N2 - currentIndex )
      currentCount = N2 - currentIndex;

    (loopinfo+loopCount)->memberCount = currentCount;

    for ( i=0; i < currentCount; i++ ) {

      *(((loopinfo+loopCount)->members)+i) = currentIndex + i; 
    }

    currentIndex += currentCount;
    loopCount++;
  }

/* initialize our loop starting and ending points */

  for ( i=0; i <= loopCount; i++ ) {
    
    (loopinfo+i)->increment = 1.;

    (loopinfo+i)->startFrame = (int) ( ((prand() * startScalar)
					+ startConstant)
				      * (float) N_Frames );

    (loopinfo+i)->loopCount = (int) ( ((prand() * durationScalar)
		+ durationConstant) * (float) N_Frames );

    (loopinfo+i)->soundPhase = (loopinfo+i)->startFrame;

    (gates+i)->on = (prand() < prob);

    if ( (gates+i)->on )
      (gates+i)->frameCount = (int) ( ( onCountBase + 
		(rrand() * durationBand) ) + .5 );

    else {
      (gates+i)->frameCount = (int) ( ( offCountBase + 
		(rrand() * durationBand) ) + .5 );
      (gates+i)->atten = pow( 10., ((attenBase +
		( attenBand * rrand() )) * .05) );
    }
  }  

  fprintf(stderr,"aphrodite has seduced the data\n");


/* perform synthesis */

  magIndex = 1.;
  interpIndex = durationScalar;

  while (currentFrame <= endFrame) {

    on += I;

    magIndex = timeAdjust;

/* GENERATE WAVEFORM PLEASE */

/* interpolate delta functions and apply current deltas */

    for ( i=0; i < loopCount; i++ ) {

      float	currentPhase = (loopinfo+i)->soundPhase;

      for ( j=0; j < (loopinfo+i)->memberCount; j++ ) {
      
	int	index = *(((loopinfo+i)->members)+j);

	/* determine current spectral frame */

	if ( (gates+i)->on) {
	  
	  if ( (gates+i)->frameCount > 0 ) {

	    *(cortex+(index<<1)) = 
	      *(*(hippocamp + (int) ( currentPhase ))
		+ (index<<1));
	  }

	  else {

	    *(cortex+(index<<1)) = 
	      *(*(hippocamp + (int) ( currentPhase ))
		+ (index<<1)) * ( (1. + (gates+i)->atten) / 2. );
	  }
	}

	else {
	  
	  if ( (gates+i)->frameCount > 0 ) {
	    
	    *(cortex+(index<<1)) = 
	      *(*(hippocamp + (int) ( currentPhase ))
		+ (index<<1)) * (gates+i)->atten;
	  }
	  
	  else {

	    *(cortex+(index<<1)) = 
	      *(*(hippocamp + (int) ( currentPhase ))
		+ (index<<1)) * ( (1. + (gates+i)->atten) / 2. );
	  }
	}
	
	/* supply our phase */

	*(cortex+(index<<1)+1) = *(*(hippocamp + (int) ( currentPhase ))
			+ (index<<1) + 1);
      }

      if ( (gates+i)->on) {

	if ( (gates+i)->frameCount > 0 ) {

	  (gates+i)->frameCount = ((gates+i)->frameCount) - 1;
	}

	else {
	    
	  (gates+i)->on = 0;
	  (gates+i)->frameCount = (int) ( ( offCountBase + 
					    (rrand() * durationBand) ) + .5 );

	  (gates+i)->atten = pow( 10., ((attenBase +
					( attenBand * rrand() )) * .05) );
	}
      }

      else {

	if ( (gates+i)->frameCount > 0 ) {

	  (gates+i)->frameCount = ((gates+i)->frameCount) - 1;
	}

	else {

	  (gates+i)->on = 1;
	  (gates+i)->frameCount = (int) ( ( onCountBase +
					    (rrand() * durationBand) ) + .5 );
	}
      }

      currentPhase += ((loopinfo+i)->increment * magIndex);

      (loopinfo+i)->currentCount += ((loopinfo+i)->increment * magIndex);

      while ( currentPhase > fframes )
	currentPhase -= fframes;

      while ( currentPhase < 0. )
	currentPhase += fframes;

      if ( (loopinfo+i)->currentCount >= (float) (loopinfo+i)->loopCount ) {
	
	if (recalculate) {
	  (loopinfo+i)->startFrame = prand() * startScalar * (float) N_Frames;
	  (loopinfo+i)->loopCount = (prand() * interpIndex * (float) N_Frames);

	  if ( (loopinfo+i)->loopCount == 0 )
	    (loopinfo+i)->loopCount = 1;
	}

	(loopinfo+i)->currentCount = 0.;
	(loopinfo+i)->soundPhase = (loopinfo+i)->startFrame;
      }

      else 
	(loopinfo+i)->soundPhase = currentPhase;
    }

/* treat nyquist frequency as dc */

    *(cortex+N) = *(*(hippocamp + (int) ( (loopinfo)->soundPhase + .5 )));
    *(cortex+N+1) = *(*(hippocamp + 
			(int) ( (loopinfo)->soundPhase + .5 )) + 1);

/* resynthesize please */

    unconvert( cortex, buffer, N2, I, srate );
    rdft( N, -1, buffer, bitshuffle, trigland );
    overlapadd( buffer, N, Wsyn, output, Nw, on );
    
    fwrite( output, I, sizeof(float), stdout );

/* read new analysis frame(s) if necessary */

    if (!fileEnd) {

      if (fastRefresh) {

	int	usedIndex = (int) refreshIndex;
	
	/* read new frames */

	for( j = 0; j < usedIndex; i++ ) {

	  fprintf(stderr,"read!\n");

	  if ( fread( *(hippocamp+hippoIndex),
		      sizeof(float), N+2, stdin ) < N+2 ) {

	    fprintf(stderr,"end!\n");
	    fileEnd = 1;
	  }

	  hippoIndex++;

	  hippoIndex %= N_Frames;
	}

	/* in the fastRefresh case, refreshIndex will accumulate floating-
	   point discrepancies, and here be incremented for the next
	   iteration, but it will also be adjusted by usedIndex:  how many
	   frames were actually loaded */

	refreshIndex += (refreshRate - (float) usedIndex);
      }

      else {

	if (refreshIndex >= refreshRate) {

	  /* read a new frame and update our indices */

	  if ( fread( *(hippocamp+hippoIndex), sizeof(float), N+2, stdin )
	       		== 0 ) {
	    fileEnd = 1;
	  }

	  hippoIndex++;
	  hippoIndex %= N_Frames;
	  refreshIndex -= refreshRate;
	}

	refreshIndex += 1.;
      }
    }

    for ( k = 0; k < Nw - I; k++ )
      output[k] = output[k+I];

    for ( k = Nw - I; k < Nw; k++ )
      output[k] = 0.;

    currentFrame++;
  }

  fprintf(stderr,
	    "\n\nframes: %d srate: %d N: %d Nw: %d I: %d playbufsize: %d\n\n",
	    currentFrame, srate, N, Nw, I, outbufsize);

  fflush(stdout);

/* bye bye */
  
  fprintf(stderr,"Sayonara...\n");

  exit(0);
}

void sayonara(int loin) {

  fprintf(stderr,"\nseconds: %.3f frames: %d\n", (float) currentFrame / 
	((float) srate / (float) D), currentFrame);

  fprintf(stderr,"Sayonara...\n");

  fflush(stdout);

  exit(loin);
}

int usage(meow)
int meow;
{
    fprintf(stderr,
	"aphrodite:  guaranteed spectral mobility OYMB\n"
	"%% aphrodite   [flags] < pv-analysis-file > integer_index_data\n"
	"	N:	fft length [1024]\n"
	"	R:	sampling rate [44100]\n"
	"	M:	window size in samples [N]\n"
	"	I:	interpolation factor in samples [M/8]\n"
	"	i:	initial position function table [undefined]\n"
	"	d:	duration of output [10. seconds]\n"
	"	F:	input frame count [undefined]\n"
	"	a:	amplitude multiplier [1.]\n"
	"	v:	velocity magnitude adjustment [4.]\n"
	"	l:	loop start time multiplier (0-1) [1.]\n"
	"	L:	loop duration multiplier (0-1) [.5]\n"
	"	k:	loop start constant offset (0-1) [0.]\n"
	"	K:	loop duration constant offset (0-1) [.5]\n"
	"	o:	duration of band sans attenuation [3 sec.]\n"
	"	O:	duration of band with attenuation [5 sec.]\n"
	"	B:	duration bandwidth [1 sec.]\n"
	"	A:	attenuation [-1 dB]\n"
	"	b:	attenuation bandwidth [1 dB]\n"
	"	p:	percentage of bands on at start [.25]\n"
	"	e:	refresh rate of input [.05 sec/sec]\n"
	"	H:	source memory size [.5 sec.]\n"
	"	g:	size of frequency grouping [2.]\n"
	"	x:	frequency band in group bandwidth\n"
	"	X:	random number seed [usec time of day]\n"
	"	T:	time evolution adjustor [1.]\n"
	"	r:	recalculate loop times\n"
	"	c:	cycle input analysis file\n"
	"	h:	this gleeful bubble of joy\n"
	);

    exit(meow);
}
