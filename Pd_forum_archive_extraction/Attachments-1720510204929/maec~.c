/*
 * 
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "m_pd.h"
#include "speex/speex_echo.h"




/* pd's samplerate */
float fs;
unsigned int frame_size = 64;	// Pd block size;
unsigned int filter_length = 4410; // 100ms @ 44.1Kz

static t_class *maec_class;

typedef struct _maec
{
	t_object x_obj;
	t_float f;
      
} t_maec;


spx_int16_t *input_frame;
spx_int16_t *echo_frame;
spx_int16_t *output_frame;
spx_int16_t *deco_in, *deco_out;


SpeexEchoState *echo_state;
SpeexDecorrState *deco_state;




static t_int *maec_perform(t_int *w)
{

	/* Achtung !!!
	 * inlet and outlet buffer share the same memory !!!
	 */ 

	int n, i, j;

	n = (int)(w[15]);

	t_maec *obj = (t_maec *)(w[1]);
	t_float *in = (t_float *)(w[2]);
	t_float *echo = (t_float *)(w[3]);

	t_float *in1 = (t_float *)(w[4]);
	t_float *in2 = (t_float *)(w[5]);
	t_float *in3 = (t_float *)(w[6]);
	t_float *in4 = (t_float *)(w[7]);
	t_float *in5 = (t_float *)(w[8]);

	t_float *out = (t_float *)(w[9]);
	t_float *out1 = (t_float *)(w[10]);
	t_float *out2 = (t_float *)(w[11]);
	t_float *out3 = (t_float *)(w[12]);
	t_float *out4 = (t_float *)(w[13]);
	t_float *out5 = (t_float *)(w[14]);

/*
	for (i = 0, j = 0 ; i<n; i++) {
		deco_in[j++] = (spx_int16_t) floorf(0x7fff * in1[i] + 0.5);
		deco_in[j++] = (spx_int16_t) floorf(0x7fff * in2[i] + 0.5);
		deco_in[j++] = (spx_int16_t) floorf(0x7fff * in3[i] + 0.5);
		deco_in[j++] = (spx_int16_t) floorf(0x7fff * in4[i] + 0.5);
		deco_in[j++] = (spx_int16_t) floorf(0x7fff * in5[i] + 0.5);

//		echo_frame[i] =(spx_int16_t) floorf(0x7fff/5.0 * (in1[i]+in2[i]+in3[i]+in4[i]+in5[i] +0.5));
	}

	speex_decorrelate(deco_state, deco_in, deco_out, 50);

	for (i=0, j=0; i<n; i++){
		echo_frame[i] =(spx_int16_t) floorf((deco_out[j]+deco_out[j+1]+deco_out[j+2]+deco_out[j+3]+deco_out[j+4])/5.0) +0.5;
		j+=5;
	}*/

	for (i = 0; i<n ; i++) {
		input_frame[i] = (spx_int16_t) (floorf((32767.0 * in[i]) + 0.5));
		echo_frame[i] = (spx_int16_t) (floorf((32767.0 * echo[i]) + 0.5));
	}
	speex_echo_cancellation(echo_state, input_frame, echo_frame, output_frame);

	for (i = 0; i<n; i++) {
		out[i] = (t_float) output_frame[i] / (float) 0x7fff;
	}
	
/*	for (i = 0, j = 0; i<n; i++) {
		out1[i] =  (t_float) deco_out[j++] / (float) 0x7fff;
		out2[i] =  (t_float) deco_out[j++] / (float) 0x7fff;
		out3[i] =  (t_float) deco_out[j++] / (float) 0x7fff;
		out4[i] =  (t_float) deco_out[j++] / (float) 0x7fff;
		out5[i] =  (t_float) deco_out[j++] / (float) 0x7fff;
	//	out5[i] = (t_float) echo_frame[i]/ (float) 0x7fff;
  
	}*/

	return (w+16);
}


static void maec_dsp(t_maec *x, t_signal **sp)
{
		dsp_add(maec_perform, 15, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec,
			 	sp[5]->s_vec, sp[6]->s_vec, sp[7]->s_vec, sp[8]->s_vec, sp[9]->s_vec, sp[10]->s_vec, sp[11]->s_vec, 
				sp[12]->s_vec, sp[0]->s_n);
}

static void *maec_new(t_symbol *s, int argc, t_atom *argv)
{
	t_maec *x = (t_maec *)pd_new(maec_class);

	inlet_new(&x->x_obj, &x->x_obj.ob_pd,  gensym("signal"),  gensym("signal"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd,  gensym("signal"),  gensym("signal"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd,  gensym("signal"),  gensym("signal"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd,  gensym("signal"),  gensym("signal"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd,  gensym("signal"),  gensym("signal"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd,  gensym("signal"),  gensym("signal"));

	outlet_new(&x->x_obj, gensym("signal"));
	outlet_new(&x->x_obj, gensym("signal"));
	outlet_new(&x->x_obj, gensym("signal"));
	outlet_new(&x->x_obj, gensym("signal"));
	outlet_new(&x->x_obj, gensym("signal"));
	outlet_new(&x->x_obj, gensym("signal"));

	return (x);
}

void maec_length(t_maec *x, t_float f)
{
	speex_echo_state_reset(echo_state);
	deco_state = speex_decorrelate_new(fs, 5, frame_size);
	echo_state = speex_echo_state_init_mc(frame_size, f, 1, 1);
	speex_echo_ctl(echo_state, SPEEX_ECHO_SET_SAMPLING_RATE, &fs);

	post("%f/n", f);
}


void maec_tilde_setup(void)
{


	fs = 44100; // sys_getsr();
	maec_class = class_new(gensym("maec~"), (t_newmethod)maec_new, 0, sizeof(t_maec), 0, A_GIMME, 0);
	CLASS_MAINSIGNALIN(maec_class, t_maec, f);

	class_addmethod(maec_class, (t_method)maec_dsp, gensym("dsp"), 0);
	class_addfloat(maec_class, (t_method)maec_length);

	input_frame = getbytes(frame_size * sizeof(spx_int16_t)); 

	echo_frame = getbytes(frame_size * sizeof(spx_int16_t));
	output_frame = getbytes(frame_size * sizeof(spx_int16_t));
	deco_in = getbytes(frame_size * 5 * sizeof(spx_int16_t));
	deco_out = getbytes(frame_size * 5 * sizeof(spx_int16_t));

	
	deco_state = speex_decorrelate_new(fs, 5, frame_size);
	echo_state = speex_echo_state_init_mc(frame_size, filter_length, 1, 1);
	speex_echo_ctl(echo_state, SPEEX_ECHO_SET_SAMPLING_RATE, &fs);

}
	

