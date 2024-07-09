#include "m_pd.h"

/* ------------------------ lrshift~ ----------------------------- */

static t_class *lrshift_tilde_class;

typedef struct _lrshift_tilde
{
    t_object x_obj;
    int x_n;
} t_lrshift_tilde;

static t_int *leftshift_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out= (t_float *)(w[2]);
    int n = (int)(w[3]);
    int shift = (int)(w[4]);
    in += shift;
    n -= shift;
    while (n--)
    	*out++ = *in++;
    while (shift--)
    	*out++ = 0;
    return (w+5);
}

static t_int *rightshift_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out= (t_float *)(w[2]);
    int n = (int)(w[3]);
    int shift = (int)(w[4]);
    n -= shift;
    in -= shift;
    while (n--)
    	*--out = *--in;
    while (shift--)
    	*--out = 0;
    return (w+5);
}

static t_int *lrshift_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out= (t_float *)(w[2]);
    int n = (int)(w[3]);
	t_lrshift_tilde *x = (t_lrshift_tilde *)w[4];
	int shift = x->x_n;

    if (shift > n)
    	shift = n;
    if (shift < -n)
    	shift = -n;
    if (shift < 0)
	{
		shift = -shift;
		out += n;
		in += n;
		n -= shift;
	    in -= shift;
		while (n--)
	    	*--out = *--in;
	    while (shift--)
	    	*--out = 0;
	}
	else
	{
	    in += shift;
	    n -= shift;
	    while (n--)
	    	*out++ = *in++;
	    while (shift--)
	    	*out++ = 0;
	}
	return (w+5);
}

static void lrshift_tilde_dsp(t_lrshift_tilde *x, t_signal **sp)
{
    int n = sp[0]->s_n;
   	dsp_add(lrshift_perform, 4, sp[0]->s_vec, sp[1]->s_vec, n, x);

}

static void *lrshift_tilde_new(t_floatarg f)
{
    t_lrshift_tilde *x = (t_lrshift_tilde *)pd_new(lrshift_tilde_class);
    x->x_n = f;
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

static void lrshift_tilde_float(t_lrshift_tilde *x, t_floatarg f)
{
    x->x_n = f;
}

void lrshift_tilde_setup(void)
{
    lrshift_tilde_class = class_new(gensym("lrshift~"),
    	(t_newmethod)lrshift_tilde_new, 0, sizeof(t_lrshift_tilde), 0, 
	    A_DEFFLOAT, 0);
    class_addmethod(lrshift_tilde_class, nullfn, gensym("signal"), 0);
    class_addmethod(lrshift_tilde_class, (t_method)lrshift_tilde_dsp,
    	gensym("dsp"), 0);
	class_addfloat(lrshift_tilde_class, (t_method)lrshift_tilde_float);
}

