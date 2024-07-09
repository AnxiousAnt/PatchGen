#include "m_pd.h"
#include <stdio.h>

static float bhrir_l[25][50][256];
static float bhrir_r[25][50][256];


static void readfile(void)
{
t_int az, el, i;
	
FILE *fid;
fid = fopen("hrir.bin", "r");               
for(az=0; az<=24; az++)
{
   for(el=0; el<=49; el++)  /*takes from hrir.bin all hrtfs and arrays them*/
   {                                          
	   fread(bhrir_l[az][el], sizeof(float), 200, fid);
           	for(i=200; i<=255; i++){bhrir_l[az][el][i]=0;}  /*adds 56 zeroes*/
	   fread(bhrir_r[az][el], sizeof(float), 200, fid);
           	for(i=200; i<=255; i++){bhrir_r[az][el][i]=0;}

   }
}

}




static t_class *overlap_tilde_class;

typedef struct _overlap_tilde {
  t_object  x_obj;
  t_sample f_overlap;
  t_sample f;
  t_int apple;
  t_float secondfloat;
  t_float firstfloat;
} t_overlap_tilde;

t_int *overlap_tilde_perform(t_int *w)
{
  t_overlap_tilde *x = (t_overlap_tilde *)(w[1]);
  t_float  *in1 =    (t_float *)(w[2]);
  t_float  *in2 =    (t_float *)(w[3]);
  t_float  *out1 =    (t_float *)(w[4]);
  t_float  *out2 =    (t_float *)(w[5]);
  int         n =           (int)(w[6]);
  int         i;
 
  t_int     read;
  t_int     azi;
  t_int     ele;
  t_int	    readpos;

      azi = (x->firstfloat - 1);
      ele = (x->secondfloat - 1);

	
   for(readpos=0; readpos<256; readpos++)
   {
	   *(out2++) = bhrir_l[azi][ele][readpos];
	   *(out1++) = bhrir_r[azi][ele][readpos];
   }


	   
  return (w+7);
} // endperform

void overlap_tilde_dsp(t_overlap_tilde *x, t_signal **sp)
{
  dsp_add(overlap_tilde_perform, 6, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}


static void overlap_first(t_overlap_tilde *x, t_floatarg f)
{
	x->firstfloat = f;
	post("firstfloat is: %f", x->firstfloat);
}

static void overlap_bound(t_overlap_tilde *x, t_floatarg f)
{
	x->secondfloat = f;
	post("secondfloat is: %f", x->secondfloat);
}


void *overlap_tilde_new(t_floatarg f)
{
  t_overlap_tilde *x = (t_overlap_tilde *)pd_new(overlap_tilde_class);

  x->f_overlap = f;
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("first"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("bound"));
  outlet_new(&x->x_obj, &s_float);
  outlet_new(&x->x_obj, &s_float);	/* second outlet for stereo */

  
  return (void *)x;
}

void overlap_tilde_setup(void) {
  overlap_tilde_class = class_new(gensym("overlap~"),
        (t_newmethod)overlap_tilde_new,
        0, sizeof(t_overlap_tilde),
        CLASS_DEFAULT, 
        A_DEFFLOAT, 0);

  class_addmethod(overlap_tilde_class,(t_method)overlap_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(overlap_tilde_class, (t_method)overlap_first, gensym("first"), A_FLOAT, 0);	
  class_addmethod(overlap_tilde_class, (t_method)overlap_bound, gensym("bound"), A_FLOAT, 0);
  CLASS_MAINSIGNALIN(overlap_tilde_class, t_overlap_tilde, f);
}

