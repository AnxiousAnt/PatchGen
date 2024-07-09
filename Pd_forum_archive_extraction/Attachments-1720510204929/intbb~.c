#include "m_pd.h"
#include "math.h"
#include "stdlib.h"
#include <alloca.h>


static t_class *intbb_tilde_class;

typedef struct _intbb_tilde {
  t_object x_obj;
  t_sample x_f;
  t_sample *integration;
  t_int is_new;
  t_int size;
} t_intbb_tilde;

t_int *intbb_tilde_perform(t_int *w)
{
  t_intbb_tilde *x = (t_intbb_tilde *)(w[1]);

  t_sample *in = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int) w[4];
  int i;
  x->size = n;

  if (x->is_new)
  {
    x->integration=calloc(n,sizeof(float));
    x->is_new=0;
  }

    
  if (x->integration != NULL)
    for(i=0; i < n; i++)
    {
      out[i]=in[i]+.999*x->integration[i];
      x->integration[i]=out[i];
    }
  return(w+5);

}

void intbb_tilde_bang(t_intbb_tilde *x)
{
  int i;
  if (x->integration != NULL)
    for(i=0; i < x->size; i++)
      x->integration[i]=0;
}


//       And then comes the stuff that I'm much indebted for.  Thanks.  *Yoink*

void intbb_tilde_dsp(t_intbb_tilde *x, t_signal **sp)
{
  dsp_add(intbb_tilde_perform, 4, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void *intbb_tilde_new(void)
{
  t_intbb_tilde *x = (t_intbb_tilde *)pd_new(intbb_tilde_class);
  x->integration = NULL;
  x->is_new=1;
  outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void intbb_tilde_setup(void) {
  intbb_tilde_class = class_new(gensym("intbb~"), (t_newmethod)intbb_tilde_new, 0, sizeof(t_intbb_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
  class_addmethod(intbb_tilde_class, (t_method)intbb_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(intbb_tilde_class, t_intbb_tilde, x_f);
  class_addbang  (intbb_tilde_class, intbb_tilde_bang);
}

