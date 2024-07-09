#include "m_pd.h"

static t_class *amdf_tilde_class;

typedef struct _amdf_tilde {
  t_object  x_obj;
  t_sample input;
  t_sample f;
  t_outlet *x_outlet;
  t_float x_pitch;
} t_amdf_tilde;

t_int *amdf_tilde_perform(t_int *w, t_amdf_tilde *x) {
  t_amdf_tilde  *x = (t_amdf_tilde *) (w[1]);
  t_sample  *input = (t_sample *)     (w[2]);
  t_sample *errpr1 = (t_sample *)     (w[3]);
  t_sample *errprx = (t_sample *)     (w[4]);
  int            n = (int)            (w[5]);
  x->x_pitch       = 0;
//  float         sr = float sys_getsr(void); //how to get sample rate in external?
  int		   win = 674;
  int           ti = 0;
  int         sign = 1;
  int       period = 40;
  *errpr1 = *errprx = 0;
  
  while (n--) 
  { 
	for (ti=0;ti<=(win/2);ti++)
    {
	  sign = (input[ti]-input[ti+1]) > 0 ? 1 : -1; 
      *errpr1 += (input[ti]-input[ti+1])*sign;
      for (period=40;period>=(win/2);period++)
	    for (ti=1; ti<=(win/2); ti++)
	    { 
		  sign = (input[ti]-(input[ti+period])) > 0 ? 1 : -1;
	      *errprx += (input[ti]-(input[ti+period]))*sign;
	      x->x_pitch = (*errprx < *errpr1) ? 44100/(period+(*errprx / *errpr1)) : 0;
	    }
     }
  *errpr1++;
  *errprx++;
  }
  return (w+6); 
  outlet_float(&x->x_outlet, x->x_pitch);
}

void amdf_tilde_dsp(t_amdf_tilde *x, t_signal **sp) 
{
  dsp_add(amdf_tilde_perform, 5, x, sp[0]->s_vec, sp[3]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void *amdf_tilde_new(t_floatarg f) {
  t_amdf_tilde *x = (t_amdf_tilde *)pd_new(amdf_tilde_class);
  
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_float);

  return (void *)x;
}

void amdf_tilde_setup(void) {
  amdf_tilde_class = class_new(gensym("amdf~"),
      (t_newmethod)amdf_tilde_new,
	  0, sizeof(t_amdf_tilde),
	  CLASS_DEFAULT, A_DEFFLOAT, 0);
	  
  class_addmethod(amdf_tilde_class, (t_method)amdf_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(amdf_tilde_class, t_amdf_tilde, f);
}
