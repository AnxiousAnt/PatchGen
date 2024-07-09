#include "m_pd.h"
#include "math.h"
#include "stdlib.h"

static t_class *maxm_tilde_class;

typedef struct _maxm_tilde {
  t_object x_obj;
  t_sample x_f;
  long int pos;
  t_sample val;
  long int min;
  long int max;
  float floatin1, floatin2;
  t_outlet *pos_out;
  t_outlet *val_out;
} t_maxm_tilde;

t_int *maxm_tilde_perform(t_int *w)
{
  t_maxm_tilde *x = (t_maxm_tilde *)(w[1]);
  t_sample *sig = (t_sample *)(w[2]);
  long int size = (long int)(w[3]);
  x->min=(long int) x->floatin1;
  x->max=(long int) x->floatin2;
  long int i,j;
  if (x->max < x->min)
  {
    j=x->max;
    x->max=x->min;
    x->min=j;
  }
  if ((x->min==0) && (x->max==0))
    x->max=size-1;
  i=((abs(x->max) > abs(x->min)) ? abs(x->max) : abs(x->min));
  j=( ( abs(x->max) > abs(x->min) ) ? ((x->max > 0) ? x->max : (size-x->max) ): ( (x->min > 0) ? x->min : (size - x->min)));
  if (i>=size)
    i=size-1;
  if ((j < 0) || (j >= size))
    j=size-1;
  x->val=sig[j];

  if ((x->min < 0) && (x->max > 0))
  {
    for( ; i>0 ; i--)
    {
      if (i <= (-1*x->min))
        if (sig[size-i] >= x->val)
        {
          x->val=sig[size-i];
          x->pos=-1*i;
        }
      if (i <= x->max)
        if (sig[i] >= x->val)
        {
          x->val=sig[i];
          x->pos=i;
        }
    }
    if (sig[0] >= x->val)
    {
      x->val=sig[0];
      x->pos=0;
    }
  }
  else if ((x->min >= 0) && (x->max >= 0))
  for( ; i>=0 ; i--)
    if ((x->min <= i) && (i <= x->max))
      if (sig[i] >= x->val)
      {
        x->val=sig[i];
        x->pos=i;
      }
  else if ((x->min <= 0) && (x->max <= 0))
  {
  for( ; i>0 ; i--)
    if (((-1*x->min) >= i) && (i >= (-1*x->max)))
      if (sig[size-i] >= x->val)
      {
        x->val=sig[size-i];
        x->pos= -1*i;
      }
  if (((-1*x->min) >= 0) && (0 >= (-1*x->max)))
    if (sig[0] >= x->val)
      {
        x->val=sig[0];
        x->pos= 0;
      }
  }
  else
  {
    x->val=0;
    x->pos=0;
  }
  outlet_float(x->pos_out, (float) x->pos);
  outlet_float(x->val_out, (float) x->val);
  return(w+4);

}

//       And then comes the stuff that I'm much indebted for.  Thanks.  *Yoink*

void maxm_tilde_dsp(t_maxm_tilde *x, t_signal **sp)
{
  dsp_add(maxm_tilde_perform, 3, x,
          sp[0]->s_vec, sp[0]->s_n);
}

void *maxm_tilde_new(t_floatarg low, t_floatarg high)
{
  t_maxm_tilde *x = (t_maxm_tilde *)pd_new(maxm_tilde_class);
  floatinlet_new(&x->x_obj, &x->floatin1);
  floatinlet_new(&x->x_obj, &x->floatin2);
  x->floatin1=low;
  x->floatin2=high;
  x->pos=0;
  x->val=0;
  x->min=(long int) low;
  x->max=(long int) high;
	x->pos_out = outlet_new(&x->x_obj, &s_float);
	x->val_out = outlet_new(&x->x_obj, &s_float);
  return (void *)x;
}

void maxm_tilde_setup(void) {
  maxm_tilde_class = class_new(gensym("maxm~"), (t_newmethod)maxm_tilde_new, 0, sizeof(t_maxm_tilde), CLASS_DEFAULT, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(maxm_tilde_class, (t_method)maxm_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(maxm_tilde_class, t_maxm_tilde, x_f);
}

