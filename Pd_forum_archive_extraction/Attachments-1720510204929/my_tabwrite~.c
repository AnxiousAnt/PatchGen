#include <m_pd.h>

/* ------------------------- my_tabwrite~ -------------------------- */

static t_class *my_tabwrite_tilde_class;

typedef struct my_tabwrite_tilde
{
  t_object x_obj;
  t_outlet *x_bangout;
  int x_phase;
  int x_nsampsintab;
  float *x_vec;
  t_symbol *x_arrayname;
  t_clock *x_clock;
  float x_f;
} t_my_tabwrite_tilde;

static void my_tabwrite_tilde_tick(t_my_tabwrite_tilde *x);

static void *my_tabwrite_tilde_new(t_symbol *s)
{
  t_my_tabwrite_tilde *x = (t_my_tabwrite_tilde *)pd_new(my_tabwrite_tilde_class);
  x->x_clock = clock_new(x, (t_method)my_tabwrite_tilde_tick);
  x->x_phase = 0x7fffffff;
  x->x_arrayname = s;
  x->x_f = 0;
  x->x_bangout = outlet_new(&x->x_obj, &s_bang);
  return (x);
}

static t_int *my_tabwrite_tilde_perform(t_int *w)
{
  t_my_tabwrite_tilde *x = (t_my_tabwrite_tilde *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  int n = (int)(w[3]), phase = x->x_phase, endphase = x->x_nsampsintab;
  if (!x->x_vec) goto bad;
  
  if (endphase > phase)
    {
      int nxfer = endphase - phase;
      float *fp = x->x_vec + phase;
      if (nxfer > n) nxfer = n;
      phase += nxfer;
      while (nxfer--)
	{
	  float f = *in++;
	  /* bash NANs and underflow/overflow hazards to zero */
	  if (!((f > 1.0e-20f && f < 1.0e20f) || (f < -1e-20f && f > -1e20)))
	    f = 0;
	  *fp++ = f;
    	}
      if (phase >= endphase)
    	{
	  clock_delay(x->x_clock, 0);
	  phase = 0x7fffffff;
    	}
      x->x_phase = phase;
    }
 bad:
  return (w+4);
}

void my_tabwrite_tilde_set(t_my_tabwrite_tilde *x, t_symbol *s)
{
  t_garray *a;
  
  x->x_arrayname = s;
  if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
      if (*s->s_name) pd_error(x, "tabwrite~: %s: no such array",
			       x->x_arrayname->s_name);
      x->x_vec = 0;
    }
    else if (!garray_getfloatarray(a, &x->x_nsampsintab, &x->x_vec))
      {
    	error("%s: bad template for tabwrite~", x->x_arrayname->s_name);
    	x->x_vec = 0;
      }
  else garray_usedindsp(a);
}

static void my_tabwrite_tilde_dsp(t_my_tabwrite_tilde *x, t_signal **sp)
{
  my_tabwrite_tilde_set(x, x->x_arrayname);
  dsp_add(my_tabwrite_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}

static void my_tabwrite_tilde_bang(t_my_tabwrite_tilde *x)
{
  x->x_phase = 0;
}

static void my_tabwrite_tilde_stop(t_my_tabwrite_tilde *x)
{
  if (x->x_phase != 0x7fffffff)
    {
      my_tabwrite_tilde_tick(x);
      x->x_phase = 0x7fffffff;
    }
}

static void my_tabwrite_tilde_tick(t_my_tabwrite_tilde *x)
{
  t_garray *a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class);
  if (!a) bug("tabwrite_tilde_tick");
  else garray_redraw(a);
  outlet_bang(x->x_bangout);
}

static void my_tabwrite_tilde_free(t_my_tabwrite_tilde *x)
{
  clock_free(x->x_clock);
}

void my_tabwrite_tilde_setup(void)
{
  my_tabwrite_tilde_class = class_new(gensym("my_tabwrite~"),
				   (t_newmethod)my_tabwrite_tilde_new, (t_method)my_tabwrite_tilde_free,
    	sizeof(t_my_tabwrite_tilde), 0, A_DEFSYM, 0);
  CLASS_MAINSIGNALIN(my_tabwrite_tilde_class, t_my_tabwrite_tilde, x_f);
  class_addmethod(my_tabwrite_tilde_class, (t_method)my_tabwrite_tilde_dsp,
		  gensym("dsp"), 0);
  class_addmethod(my_tabwrite_tilde_class, (t_method)my_tabwrite_tilde_set,
		  gensym("set"), A_SYMBOL, 0);
  class_addmethod(my_tabwrite_tilde_class, (t_method)my_tabwrite_tilde_stop,
		  gensym("stop"), 0);
  class_addbang(my_tabwrite_tilde_class, my_tabwrite_tilde_bang);
}

