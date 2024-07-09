#include "m_pd.h"
#include "math.h"

static t_class *maxval_tilde_class;

typedef struct _maxval_tilde {
  t_object x_obj;
  t_sample x_f;
  t_int pos;
  t_float val;
  t_outlet *pos_out;
  t_outlet *val_out;
} t_maxval_tilde;

t_int *maxval_tilde_perform(t_int *w)
{
  t_maxval_tilde *x = (t_maxval_tilde *)(w[1]);
  t_sample *sig = (t_sample *)(w[2]);
  t_int size = (t_int)(w[3]);
  t_int i;
  t_float temp;
  temp=0;
  x->pos=0;
  for(i=0; i < size; i++)
  {
    if (fabsf(sig[i]) > temp)
    {
      temp=fabsf(sig[i]);
      x->pos=i;
    }
  }
  x->val=sig[x->pos];
  outlet_float(x->pos_out, (float) x->pos);
  outlet_float(x->val_out, (float) x->val);
  return(w+4);

}

//       And then comes the stuff that I'm much indebted for.  Thanks.  *Yoink*

void maxval_tilde_dsp(t_maxval_tilde *x, t_signal **sp)
{
  dsp_add(maxval_tilde_perform, 3, x,
          sp[0]->s_vec, sp[0]->s_n);
}

void *maxval_tilde_new()
{
  t_maxval_tilde *x = (t_maxval_tilde *)pd_new(maxval_tilde_class);
  x->pos=0;
  x->val=0;
  x->pos_out = outlet_new(&x->x_obj, &s_float);
  x->val_out = outlet_new(&x->x_obj, &s_float);
  return (void *)x;
}

void maxval_tilde_setup(void) {
  maxval_tilde_class = class_new(gensym("maxval~"), (t_newmethod)maxval_tilde_new, 0, sizeof(t_maxval_tilde), CLASS_DEFAULT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(maxval_tilde_class, (t_method)maxval_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(maxval_tilde_class, t_maxval_tilde, x_f);
}

