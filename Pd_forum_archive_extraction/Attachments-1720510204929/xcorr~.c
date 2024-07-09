#include "m_pd.h"
#include "math.h"
#include "stdlib.h"
#include "fftw3.h"
#include <alloca.h>


static t_class *xcorr_tilde_class;

typedef struct _xcorr_tilde {
  t_object x_obj;
  t_sample x_f;
} t_xcorr_tilde;

t_int *xcorr_tilde_perform(t_int *w)
{
  t_xcorr_tilde *x = (t_xcorr_tilde *)(w[1]);

  t_sample *sig1 = (t_sample *)(w[2]);
  t_sample *sig2 = (t_sample *)(w[3]);
  t_sample *out  = (t_sample *)(w[4]);
  fftwf_plan plan;
  long int size  = (long int) w[5];
  long int k     = size/2;
  float *expsig1 = NULL;
  float *revsig2 = NULL;
  float temp, temp2;
  long int i=0;
  int well_defined=1;
  int qtr, thrqtr;

// The two signals are created, nonzero on 0 to N/4 and 3N/4 to N
// This will be revised

  expsig1=(float *) alloca(size*sizeof(float));
  revsig2=(float *) alloca(size*sizeof(float));
  qtr = size/4;
  thrqtr = 3*size/4;
  for (i=0; i < qtr ; i++)
  {
    expsig1[i]=sig1[i];
    revsig2[i]=0;
  }
  for (i=qtr; i < thrqtr ; i++)
  {
    expsig1[i]=sig1[i];
    revsig2[i]=sig2[size-i];
  }
  for (i=thrqtr; i < size ; i++)
  {
    expsig1[i]=sig1[i];
    revsig2[i]=0;
  }

  plan=fftwf_plan_r2r_1d(size, expsig1, expsig1, FFTW_R2HC, 0);
  fftwf_execute(plan);
  plan=fftwf_plan_r2r_1d(size, revsig2, revsig2, FFTW_R2HC, 0);
  fftwf_execute(plan);
  expsig1[0]*=revsig2[0];
  expsig1[k]*=revsig2[k];
  for(i=1; i < k; i++)
  {
    temp=expsig1[i];
    temp2=expsig1[size-i];
    expsig1[i]=temp*revsig2[i]-temp2*revsig2[size-i];
    expsig1[size-i]=temp*revsig2[size-i]+temp2*revsig2[i];
  }
  plan=fftwf_plan_r2r_1d(size, expsig1, expsig1, FFTW_HC2R, 0);
  fftwf_execute(plan);
  
  for(i=0; i < size; i++)
  {
    out[i]=expsig1[i];
  }
}
  return(w+6);

}

void xcorr_tilde_dsp(t_xcorr_tilde *x, t_signal **sp)
{
  dsp_add(xcorr_tilde_perform, 5, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void *xcorr_tilde_new(void)
{
  t_xcorr_tilde *x = (t_xcorr_tilde *)pd_new(xcorr_tilde_class);

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

  outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void xcorr_tilde_setup(void) {
  xcorr_tilde_class = class_new(gensym("xcorr~"), (t_newmethod)xcorr_tilde_new, 0, sizeof(t_xcorr_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
  class_addmethod(xcorr_tilde_class, (t_method)xcorr_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(xcorr_tilde_class, t_xcorr_tilde, x_f);
}

