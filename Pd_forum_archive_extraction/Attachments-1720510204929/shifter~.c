/*	This MSP object was initiated by Brian K. Vogel at CNMAT.
 //	Some of the MAX/MSP setup code is from Matt Wright's windower~ external.
 //	Copyright (c) 1999.  The Regents of the University of California
 //	(Regents). All Rights Reserved.
 //
 //	This version was written by Tristan Jehan at MIT Media Lab.
 //	Copyright (c) 2000. Massachussets Institute of Technology.
 //	All Rights Reserved.
 //
 //	Windows adaptation Peter Castine. Copyright (c) 2006 with author.
 //  All rights reserved.
 */

#define SHIFTER_VERSION "1.00"
#define OUTPUT_BUFFER_DELAY 1000 // this should contain a few periods minimum
#define MINIMUM_PITCH 75.0 // won't do anything below this pitch
#define DEF_PITCH MINIMUM_PITCH // choose a default pitch
// PC: Changed name of macro from DEFAULT_PITCH to
// to DEF_PITCH to avoid a name conflict.
#define RES_ID 7050

#ifndef M_PI
// Conditional (and higher-precision) #define PC 28-Aug-06 (it's included in <math.h>)
#define M_PI 3.14159265358979323846264338327950288
#endif

#include <math.h>
//To compile it for Pd on Mac
# define PDMAC_VERSION

#ifdef WIN_VERSION

#include "ext.h"
#include "z_dsp.h"

#endif

#ifdef PDMAC_VERSION
#include "m_pd.h"
#endif

/******************************************************************************************
 *
 *	DllMain()
 *
 *	Windows voodoo for DLLs. Copied
 *
 ******************************************************************************************/
#ifdef WIN_VERSION

HINSTANCE	gDLLInstanceHdl = NIL;

BOOL WINAPI PShiftyDllEntry(HINSTANCE, DWORD, LPVOID);

BOOL WINAPI
PShiftyDllEntry(HINSTANCE iDLLInstance, DWORD iReason, LPVOID /*lpReserved*/) {
    CHAR	externalPath[MAX_PATH];

    // Perform actions based on the reason for calling.
    switch (iReason) {
    case DLL_PROCESS_ATTACH:
        // Initialize once for each new process.  Return FALSE to fail DLL load.
        gDLLInstanceHdl = iDLLInstance;

        // ?? I don't think the shifter~ object needs the GetModuleFileName() stuff
        //		OTOH, can it hurt??
        GetModuleFileName(iDLLInstance, externalPath, MAX_PATH);

        // Since the DLL_THREAD_ATTACH and DLL_THREAD_DETACH cases below don't actually
        // do anything, we don't need to receive those messages.
        // Tell Windows to optimize them out.
        DisableThreadLibraryCalls(iDLLInstance);
        break;

    case DLL_THREAD_ATTACH:
        // Do any thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do any thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:
        // Perform any necessary cleanup.
        break;
    }

    return TRUE;
}
#endif

void *shifter_class;
#ifdef PDMAC_VERSION
static t_class *proxy_class;
#endif

// Shifter Object and its variables
typedef struct _shifter {
#ifdef WIN_VERSION
    t_pxobject x_obj;
#endif
#ifdef PDMAC_VERSION
    t_object x_obj;
    t_float x_f;
#endif
#ifndef PDMAC_VERSION
    void *proxy[2];
    long inletNumber;
#endif
    float *inputRingBuf;
    int inputRingBufWritePos;
    float *outputRingBuf;
    int outputRingBufPitchMarkerPos;
    float pitchScaleFactor;
    float pitch;
    int ringBufSize;
    int outputDelay;
    int readPos;
    int inputPeriodLength;
    int samplesLeftInPeriod;
    int isUnvoiced; // 1 if unvoiced, 0 if voiced
    float minimumPitch;
    int minimumPitchSamps;
    int sampleRate; // sampling rate.
} t_shifter;

// Declarations
#ifndef PDMAC_VERSION
void main(void);
void shifter_dsp(t_shifter *x, t_signal **sp, short *count);
void *shifter_new(Symbol *s, short argc, Atom *argv);
void shifter_assist(t_shifter *x, void *b, long m, long a, char *s);
void shifter_float(t_shifter *x, double f);
void shifter_freq_float(t_shifter *x, double f);
void shifter_freq_int(t_shifter *x, long n);
void shifter_fact_float(t_shifter *x, double f);
void shifter_fact_int(t_shifter *x, long n);
void ResizeBufferAndAtomList(t_shifter *x);
void shifter_int(t_shifter *x, long n);
#endif
void shifter_free(t_shifter *x);
t_int *shifter_perform(t_int *w);

// Main function
#ifndef PDMAC_VERSION
void main(void) {
#ifdef _WINDOWS_	// PC: Conditional compilation to handle the funky copyright char x-platform
    const char kCRightGlyph[] = "(c)";
#else
    const char kCRightGlyph[] = "©";
#endif
    post("shifter~ object version " SHIFTER_VERSION " by Tristan Jehan (MIT) and Brian Vogel (CNMAT)");
    post("copyright %s 1999 Regents of the University of California", kCRightGlyph);
    post("copyright %s 2000 Massachussets Institute of Technology", kCRightGlyph);
#ifdef _WINDOWS_
    // Could hardcode copyright glyph, but if we change to use the Windows encoding for this
    // character, it will be easier to change if we do it like this:
    post("Windows port copyright %s 2006 Peter Castine", kCRightGlyph);
#endif
    post(" ");

    setup( (Messlist **)&shifter_class, (method)shifter_new, (method)shifter_free, (short)sizeof(t_shifter),0L, A_GIMME,0);
    addmess((method)shifter_dsp, "dsp", A_CANT, 0);
    addmess((method)shifter_assist,"assist",A_CANT,0);
    addfloat((method)shifter_float);
    addint((method)shifter_int);
    dsp_initclass();
#ifdef _WINDOWS_
#else
    rescopy('STR#',RES_ID);		// But the shifter~.rsrc Tristan sent was empty anyway??
#endif
}
#endif

#ifndef PDMAC_VERSION
// Assist function used to label inlets and outlets
void shifter_assist(t_shifter* x, void* b, long m, long a, char* s) {
#ifdef _WINDOWS_
#pragma unused (x, b)
    // PC: J'ai *quelle* honte. On ne peux pas faire l'assistance comme ça.
    char* ss;

    if (m == 1) switch (a) {
        default:
            ss = "signal (yer tune)";
            break;
        case 1:
            ss = "float (reference pitch)";
            break;
        case 2:
            ss = "int (semitones) or float (frequency)";
            break;
        }
    else ss = "signal (pshifted tune)";

    strcpy(s, ss);
#else		// TJ/BKV original
    assist_string(RES_ID,m,a,1,4,s);
#endif
}
#endif

#ifndef PDMAC_VERSION
void *shifter_new(Symbol *s, short argc, Atom *argv) {
#pragma unused(s)				// Stop compiler warnings
#endif
#ifdef PDMAC_VERSION
    void *shifter_new(t_symbol *s, int argc, t_atom *argv) {
#endif
        t_shifter *x;
        float min_pitch = MINIMUM_PITCH;
        int delay_samps = OUTPUT_BUFFER_DELAY;
        int i;

        if (argc == 0) {
            min_pitch = MINIMUM_PITCH;
            delay_samps = OUTPUT_BUFFER_DELAY;
        } else if (argc == 1) { // one argument
            if (argv[0].a_type == A_FLOAT) {
                min_pitch = (float)argv[0].a_w.w_float;
            }
#ifndef PDMAC_VERSION
            else if (argv[0].a_type == A_LONG) {
                min_pitch = (float)argv[0].a_w.w_long;
            }
#endif
            else {
                goto argerror;
            }
        } else if (argc == 2) { // two arguments
            if (argv[0].a_type == A_FLOAT) {
                min_pitch = (float)argv[0].a_w.w_float;
            }
#ifndef PDMAC_VERSION
            else if (argv[0].a_type == A_LONG) {
                min_pitch = (float)argv[0].a_w.w_long;
            }
#endif
            else {
                goto argerror;
            }
            if (argv[1].a_type == A_FLOAT) {
                delay_samps = (int)argv[1].a_w.w_float;
            }
#ifndef PDMAC_VERSION
            else if (argv[1].a_type == A_LONG) {
                delay_samps = (int)argv[1].a_w.w_long;
            }
#endif
            else {
                goto argerror;
            }
        } else {
            goto argerror;
        }

        if ((min_pitch <= 0) || (delay_samps <= 0)) {
            error("shifter~: Minimum pitch and Delay samples must be positive!");
            goto argerror;
        }

#ifndef PDMAC_VERSION
        x = (t_shifter *)newobject(shifter_class);
#endif
#ifdef PDMAC_VERSION
        x = (t_shifter *)pd_new(shifter_class);
#endif
#ifndef PDMAC_VERSION
        dsp_setup((t_pxobject *)x,1);  // the object has 3 signal inlets: audio in, pitch scale factor and pitch in
        x->proxy[1] = proxy_new(x,2L,&x->inletNumber);
        x->proxy[0] = proxy_new(x,1L,&x->inletNumber);
        outlet_new((t_object *)x, "signal"); // add a signal outlet.
#endif

#ifdef PDMAC_VERSION
        floatinlet_new(&x->x_obj,&x->pitchScaleFactor);
        floatinlet_new(&x->x_obj,&x->pitch);
        outlet_new(&x->x_obj, gensym("signal"));
#endif
        x->sampleRate = sys_getsr(); // Set the sampling rate.
        x->outputDelay = delay_samps;
        x->ringBufSize = (int)(2*delay_samps);
        x->minimumPitch = min_pitch;
        x->minimumPitchSamps = (int)(x->sampleRate/x->minimumPitch);
        x->inputRingBuf = 0;
        x->inputRingBufWritePos = 0;
        x->pitchScaleFactor = 1;
        x->pitch = DEF_PITCH;

        // Allocate memory for the input circular buffer.
        x->inputRingBuf = (float *)t_getbytes((long) (x->ringBufSize * sizeof(float)));

        post("--- Shifter~ ---");
        post("  Minimum pitch %.2f Hz", min_pitch);
        post("  Window size %ld", delay_samps);
        post("  Latency = %ld samples", x->minimumPitchSamps + delay_samps);
        post(" ");

        // Check if the allocation suceeded.
        if (x->inputRingBuf == NULL) {
#ifndef PDMAC_VERSION
            ouchstring("shifter~: out of memory!");
#endif
#ifdef PDMAC_VERSION
            error("shifter~: out of memory!");
#endif
            x->ringBufSize = 0;
            return 0;
        }

        // Initiallize the input circular buffer to a zero value in each element.
        for (i=0; i<x->ringBufSize; i++) {
            x->inputRingBuf[i] = 0.0f;
        }

        x->outputRingBuf = 0;
        x->outputRingBufPitchMarkerPos = 0;

        // Allocate memory for the output circular buffer.
        x->outputRingBuf = (float *)t_getbytes((long) (x->ringBufSize * sizeof(float)));

        // Check if the allocation suceeded.
        if (x->outputRingBuf == NULL) {
#ifndef PDMAC_VERSION
            ouchstring("shifter~: out of memory!");
#endif
#ifdef PDMAC_VERSION
            error("shifter~: out of memory!");
#endif
            x->ringBufSize = 0;
            return 0;
        }

        // Initiallize the output circular buffer to a zero value in each element.
        for (i=0; i<x->ringBufSize; i++) {
            x->outputRingBuf[i] = 0.0f;
        }

        x->readPos = (x->inputRingBufWritePos - x->outputDelay + x->ringBufSize) % x->ringBufSize;
        x->samplesLeftInPeriod = 0;
        x->inputPeriodLength = 100;
        x->isUnvoiced = 1;
        return x;

argerror:
        error("shifter~ usage: shifter~ [Minimum input pitch] [Window size]");
        post("If only 1 argument, it sets the minimum pitch");
        post("If no argument, the minimum pitch default is %f Hz and the window size default is %ld",
             (float)MINIMUM_PITCH, (long)OUTPUT_BUFFER_DELAY);
        post(" ");
        return 0;
    }


    // Free memory
    void shifter_free(t_shifter *x) {

        if (x->ringBufSize != 0) {
            t_freebytes(x->inputRingBuf, (long) (x->ringBufSize * sizeof(float)));
        }
        if (x->ringBufSize != 0) {
            t_freebytes(x->outputRingBuf, (long) (x->ringBufSize * sizeof(float)));
        }
#ifndef PDMAC_VERSION
        freeobject(x->proxy[0]);
        dsp_free(&(x->x_obj));
#endif
    }

#ifndef PDMAC_VERSION
    void shifter_dsp(t_shifter *x, t_signal **sp, short *count) {
        if (count[0]) dsp_add(shifter_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
    }
#endif

#ifdef PDMAC_VERSION
    static void shifter_dsp(t_shifter *x, t_signal **sp) {
        dsp_add(shifter_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
    }
#endif


    // Manage Inlets
#ifndef PDMAC_VERSION
    void  shifter_int(t_shifter *x, long n) {
        switch (x->inletNumber) {
        case 1: // middle inlet
            shifter_freq_int(x,n);
            break;
        case 2: // right inlet
            shifter_fact_int(x,n);
            break;
        }
    }
    void  shifter_float(t_shifter *x, double f) {
        switch (x->inletNumber) {
        case 1: // middle inlet
            shifter_freq_float(x,f);
            break;
        case 2: // right inlet
            shifter_fact_float(x,f);
            break;
        }
    }
    void shifter_freq_float(t_shifter *x, double f) {
        x->pitch = f;
    }
    void shifter_freq_int(t_shifter *x, long n) {
        x->pitch = (t_float) n;
    }
    void shifter_fact_float(t_shifter *x, double f) {
        x->pitchScaleFactor = f;
    }
    void shifter_fact_int(t_shifter *x, long n) {

        float octave;
        int note;

        if (n<0) {
            octave = 1.0f/powf(2.0f,-((n+1)/12)+1);
            note = (12-(-n % 12)) % 12;
        } else {
            octave = powf(2.0f,(n/12));
            note = n % 12;
        }

        // pitch shift by 1/2 tones
        switch (note) {
        case 0:
            x->pitchScaleFactor = 1.0f * octave;
            break; 					// no change
        case 1:
            x->pitchScaleFactor = 1.06666666666666666667f * octave;
            break; 	// 16/15
        case 2:
            x->pitchScaleFactor = 1.125f * octave;
            break; 					// 9/8
        case 3:
            x->pitchScaleFactor = 1.2f * octave;
            break; 					// 6/5
        case 4:
            x->pitchScaleFactor = 1.25f * octave;
            break; 					// 5/4
        case 5:
            x->pitchScaleFactor = 1.33333333333333333333f * octave;
            break;	// 4/3
        case 6:
            x->pitchScaleFactor = 1.4f * octave;
            break; 					// 7/5
        case 7:
            x->pitchScaleFactor = 1.5f * octave;
            break; 					// 3/2
        case 8:
            x->pitchScaleFactor = 1.6f* octave;
            break; 						// 8/5
        case 9:
            x->pitchScaleFactor = 1.66666666666666666667f * octave;
            break; 	// 5/3
        case 10:
            x->pitchScaleFactor = 1.8f * octave;
            break; 					// 9/5
        case 11:
            x->pitchScaleFactor = 1.875f * octave;
            break; 					// 15/8
        }
    }
#endif


    // Perform function
    t_int *shifter_perform(t_int *w) {

        t_float *in = (t_float *)(w[1]);
        t_float *out = (t_float *)(w[2]);
        t_shifter *x = (t_shifter *)(w[3]);
        t_int size = w[4];

        float periodRatio;
        float correctedPitchScale;
        float correctedPitchIn;
        float windowVal;
        int olaIndex;
        int outLag;
        int inHalfAway;

        while (size > 0) {

            x->inputRingBuf[x->inputRingBufWritePos] = (*in);

            if (x->samplesLeftInPeriod == 0) {
                outLag = 1;
                inHalfAway = (x->inputRingBufWritePos + x->ringBufSize/2) % x->ringBufSize;

                if (inHalfAway < (x->ringBufSize/2)) {
                    if ((x->outputRingBufPitchMarkerPos < inHalfAway) || (x->outputRingBufPitchMarkerPos > x->inputRingBufWritePos))
                        outLag = 0;
                } else {
                    if ((x->outputRingBufPitchMarkerPos > x->inputRingBufWritePos) && (x->outputRingBufPitchMarkerPos < inHalfAway))
                        outLag = 0;
                }

                while (outLag == 1) {
                    if ((x->pitchScaleFactor <= 0.1) || (x->pitchScaleFactor > 6.0) || (x->isUnvoiced == 1)) {
                        correctedPitchScale = 1.0f;
                    } else {
                        correctedPitchScale = x->pitchScaleFactor;
                    }

                    periodRatio = 1.0f/(correctedPitchScale);
                    x->outputRingBufPitchMarkerPos = (int)(x->outputRingBufPitchMarkerPos + (int)(x->inputPeriodLength*periodRatio)) % x->ringBufSize;
                    for (olaIndex = -x->inputPeriodLength; olaIndex <= x->inputPeriodLength; ++olaIndex) {
                        windowVal = (1 + cos(M_PI*olaIndex/(float)x->inputPeriodLength))*0.5f;
                        x->outputRingBuf [(olaIndex + x->outputRingBufPitchMarkerPos + x->ringBufSize) % x->ringBufSize] +=
                            windowVal * x->inputRingBuf [(olaIndex + x->inputRingBufWritePos - x->minimumPitchSamps + x->ringBufSize) % x->ringBufSize];
                    }
                    outLag = 1;
                    inHalfAway = (x->inputRingBufWritePos + x->ringBufSize/2) % x->ringBufSize;
                    if (inHalfAway < (x->ringBufSize/2)) {
                        if ((x->outputRingBufPitchMarkerPos < inHalfAway) || (x->outputRingBufPitchMarkerPos > x->inputRingBufWritePos))
                            outLag = 0;
                    } else {
                        if ((x->outputRingBufPitchMarkerPos > x->inputRingBufWritePos) && (x->outputRingBufPitchMarkerPos <= inHalfAway))
                            outLag = 0;
                    }
                }

                if (x->pitch <= x->minimumPitch) {
                    correctedPitchIn = DEF_PITCH;
                    x->isUnvoiced = 1;
                } else {
                    correctedPitchIn = x->pitch;
                    x->isUnvoiced = 0;
                }
                x->inputPeriodLength = (int)((1.0f/correctedPitchIn)*(float)x->sampleRate);
                x->samplesLeftInPeriod = x->inputPeriodLength;
            }

            --x->samplesLeftInPeriod;

            *out = x->outputRingBuf[x->readPos];

            x->outputRingBuf[x->readPos] = 0;
            x->inputRingBufWritePos++;
            x->inputRingBufWritePos %= x->ringBufSize;
            x->readPos++;
            x->readPos %= x->ringBufSize;

            in++;
            out++;
            size--;
        }

        return (w+5);
    }
#ifdef PDMAC_VERSION
    void shifter_tilde_setup(void) {
        const char kCRightGlyph[] = "©";
        shifter_class = class_new(gensym("shifter~"),
                                  (t_newmethod)shifter_new,
                                  (t_method)shifter_free,
                                  sizeof(t_shifter), 0,
                                  A_GIMME, 0);
        class_addmethod(shifter_class, (t_method)shifter_dsp, gensym("dsp"), 0);
        CLASS_MAINSIGNALIN(shifter_class, t_shifter,x_f);
        t_symbol *symbol = gensym("shifter~.pd");
        class_sethelpsymbol(shifter_class, symbol);
        post("shifter~ object version " SHIFTER_VERSION " by Tristan Jehan (MIT) and Brian Vogel (CNMAT)");
        post("copyright %s 1999 Regents of the University of California", kCRightGlyph);
        post("copyright %s 2000 Massachussets Institute of Technology", kCRightGlyph);
        post("Mac port copyright %s 2008 Julian Villegas", kCRightGlyph);
        post(" ");
    }
#endif