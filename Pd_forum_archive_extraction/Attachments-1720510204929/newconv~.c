#include "m_pd.h"
#include <stdio.h>
#include "fftr4.h"

/* this part declares the 3-dimensional array of hrtf data in memory */

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
static float aleftout[256]

	  = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};



static float arightout[256]

	  = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};




static t_class *newconv_tilde_class;

typedef struct _newconv_tilde {
  t_object  x_obj;
  t_sample f_newconv;
  t_sample f;
  t_float secondfloat;
  t_float firstfloat;
} t_newconv_tilde;

static t_int *newconv_tilde_perform(t_int *w)
{
  t_newconv_tilde *x = (t_newconv_tilde *)(w[1]);
  t_float  *in1 =    (t_float *)(w[2]);
  t_float  *in2 =    (t_float *)(w[3]);
  t_float  *out1 =    (t_float *)(w[4]);
  t_float  *out2 =    (t_float *)(w[5]);

  int         n =           (int)(w[6]);

  t_int         set, read;
  t_int         azi;
  t_int         ele;
 

   while (n--)
   {
     
     for(set=0; set<=255; set++) 
     {
	     aleftout[set]=(*in1++);  /*brings in 256 samples from input*/
	     arightout[set]=(*in2++);
     }
  
                azi = x->firstfloat-1;
		ele = x->secondfloat-1;
     
     fftr4_256(aleftout);
     fftr4_256(bhrir_l[azi][ele]);
     fftr4_mul256(aleftout,bhrir_l[azi][ele]);
     fftr4_scale256(aleftout);
     fftr4_un256(aleftout);

     fftr4_256(arightout);
     fftr4_256(bhrir_r[azi][ele]);
     fftr4_mul256(arightout,bhrir_r[azi][ele]);
     fftr4_scale256(arightout);
     fftr4_un256(arightout);

     for(read=0; read<=255; read++)
     {
	     *out1++ = aleftout[read];
	     *out2++ = arightout[read];
     }  
   }
  
/*
   while (n--)
   {
      	
	t_float lsum = 0.0;
	t_float rsum = 0.0;


	for(i=199; i>0; i--)
      	{
		lout[i]=lout[i-1];         
	    rout[i]=rout[i-1];	
	}
		
		lout[0]=(*in1++);      
        rout[0]=(*in2++);

	for(i=0; i<200; i++)
	{	                                		
                azi = x->firstfloat;
		ele = x->secondfloat;
                 
		
		lsum+=hrir_l[17][17][i]*lout[i];     
		lsum+=hrir_r[17][17][i]*lout[i];
	}


	   *out1++ = lsum;
	   *out2++ = rsum;
  

   }        
  
*/
   
  /*t_sample f_newconv = (x->f_newconv<0)?0.0:(x->f_newconv>1)?1.0:x->f_newconv;

    while (n--) *out++ = (*in1++)*(1-f_newconv)+(*in2++)*f_newconv;*/

  return (w+7);
}

static void newconv_tilde_dsp(t_newconv_tilde *x, t_signal **sp)
{
  dsp_add(newconv_tilde_perform, 6, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

static void newconv_first(t_newconv_tilde *x, t_floatarg f)
{
	x->firstfloat = f;
	post("firstfloat is: %f", x->firstfloat);
}

static void newconv_bound(t_newconv_tilde *x, t_floatarg f)
{
	x->secondfloat = f;
	post("secondfloat is: %f", x->secondfloat);
}

static void *newconv_tilde_new(t_floatarg f)
{
  t_newconv_tilde *x = (t_newconv_tilde *)pd_new(newconv_tilde_class);

  x->f_newconv = f;
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  /*floatinlet_new (&x->x_obj, &x->f_newconv); */	/* accessing x->x_newconv gives the (last) value of this input (float) */
	/* just another float inlet: */
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("first"));

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("bound"));
  outlet_new(&x->x_obj, &s_signal);

  outlet_new(&x->x_obj, &s_signal);	/* second outlet for stereo */


  return (void *)x;
}

void newconv_tilde_setup(void) {
  newconv_tilde_class = class_new(gensym("newconv~"),
        (t_newmethod)newconv_tilde_new,
        0, sizeof(t_newconv_tilde),
        CLASS_DEFAULT, 
        A_DEFFLOAT, 0);

  class_addmethod(newconv_tilde_class, (t_method)newconv_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(newconv_tilde_class, (t_method)newconv_first, gensym("first"), A_FLOAT, 0);	
  class_addmethod(newconv_tilde_class, (t_method)newconv_bound, gensym("bound"), A_FLOAT, 0);	/* calls a function newconv_bound when a float arrives */
  CLASS_MAINSIGNALIN(newconv_tilde_class, t_newconv_tilde, f);
}

