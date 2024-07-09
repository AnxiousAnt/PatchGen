#include "m_pd.h"

typedef struct wrap_overshoot_tilde
{
    t_object x_obj;
    t_float x_f;
  int overShoot, shootFlag, k_i;
  t_float token, storeLast;
  t_sample f_s;
  t_clock *x_clock;

} t_wrap_overshoot_tilde;

t_class *wrap_overshoot_tilde_class;

void wrap_overshoot_tilde_tick(t_wrap_overshoot_tilde *x)
{
  x->shootFlag = 1;


}

static void *wrap_overshoot_tilde_new(void)
{
    t_wrap_overshoot_tilde *x = (t_wrap_overshoot_tilde *)pd_new(wrap_overshoot_tilde_class);
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    x->overShoot = 0;
    x->shootFlag = 0;
    x->f_s = 0;
    x->k_i = 0;
    x->storeLast = 0;
    x->x_clock = clock_new(x, (t_method)wrap_overshoot_tilde_tick);
    return (x);
}

static t_int *wrap_overshoot_tilde_perform(t_int *w, t_wrap_overshoot_tilde *x)
{
    t_sample *in = *(t_sample **)(w+1), *out = *(t_sample **)(w+2);
    t_int n = *(t_int *)(w+3);
    /*    if(x->overShoot > 0)
      {
	x->shootFlag = 1;
	//	x->overShoot = 0;
      }
    else if(x->overShoot == 0)
      {
	x->shootFlag = 1;
	}*/
    
	// else if(x->overShoot == 0) x->shootFlag = 0;
    while(n--)
    {
        x->f_s = *in++;
        x->k_i = x->f_s;
	if (x->storeLast < 1 && (x->f_s - x->k_i) >= 1)
	  {
	    //	    *out++ = x->f_s - x->k_i;
	    clock_delay(x->x_clock, 0);
	  }
	if(x->shootFlag == 1)
	  {
	    *out++ = x->f_s - x->k_i;
	  }
	else if (x->f_s > 0) *out++ = x->f_s - x->k_i;
        else *out++ = x->f_s - (x->k_i - 1);
	x->storeLast = (float)(x->f_s - x->k_i);
    }
    if(x->shootFlag == 1) x->shootFlag = 0;
    return (w + 4);
}

static void wrap_overshoot_tilde_dsp(t_wrap_overshoot_tilde *x, t_signal **sp)
{
    dsp_add(wrap_overshoot_tilde_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void wrap_overshoot_tilde_setup(void)
{
  wrap_overshoot_tilde_class = class_new(gensym("wrap_overshoot~"), 
  (t_newmethod)wrap_overshoot_tilde_new, 
  0, sizeof(t_wrap_overshoot_tilde),
  CLASS_DEFAULT, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(wrap_overshoot_tilde_class, t_wrap_overshoot_tilde, x_f);
    class_addmethod(wrap_overshoot_tilde_class, (t_method)wrap_overshoot_tilde_dsp,
        gensym("dsp"), A_CANT, 0);
}
