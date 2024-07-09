/* Copyright (c) 2015 Claude Heiland-Allen
* Heavily based on Pure-data source code for [cpole~] and [cputime]
* Tested only on GNU/Linux/Debian/Wheezy, no WIN32 or OSX here... */

/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"

#include <math.h>

#ifdef _WIN32
#include <wtypes.h>
#endif

#include <time.h>

/* -------------------------- cputime_tilde ------------------------------ */

static t_class *cputime_tilde_class;

typedef struct _cputime_tilde
{
    t_object x_obj;
    t_float x_f;
} t_cputime_tilde;

static t_sample cputime_tilde_seconds(void)
{
#ifndef _WIN32
    /* wrap-around every minute to avoid precision loss with float32 */
    double minutes = clock() / (double) CLOCKS_PER_SEC / 60.0;
    minutes -= floor(minutes);
    return 1000.0 * 60.0 * minutes;
#else
    FILETIME ignorethis, ignorethat;
    LARGE_INTEGER usertime, kerneltime;
    BOOL retval;
    retval = GetProcessTimes(GetCurrentProcess(), &ignorethis, &ignorethat,
        (FILETIME *)&kerneltime, (FILETIME *)&usertime);
    if (retval)
    {
        t_sample t = (0.0001 * kerneltime.QuadPart + usertime.QuadPart);
        return t;
    }
    else return 0;
#endif
}

static void *cputime_tilde_new(void)
{
    t_cputime_tilde *x = (t_cputime_tilde *)pd_new(cputime_tilde_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return (x);
}

static t_int *cputime_tilde_perform(t_int *w)
{
    t_sample *in1 = *(t_sample **)(w+1), *in2 = *(t_sample **)(w+2),
        *out1 = *(t_sample **)(w+3), *out2 = *(t_sample **)(w+4);
    t_int n = *(t_int *)(w+5);
    t_sample s = cputime_tilde_seconds();
    while (n--)
    {   
      /* handle wrap-around */
      t_sample d = s - *in1++;
      *out1++ = d < 0 ? d + 60000.0 : d;
      *out2++ = 0;
    }
    return (w + 6); 
    (void) in2;
}

static void cputime_tilde_dsp(t_cputime_tilde *x, t_signal **sp)
{
    dsp_add(cputime_tilde_perform,
      5, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
    (void) x;
}

extern void cputime_tilde_setup(void)
{
    cputime_tilde_class = class_new(gensym("cputime~"), (t_newmethod)cputime_tilde_new, 0,
        sizeof(t_cputime_tilde), 0, 0);
    CLASS_MAINSIGNALIN(cputime_tilde_class, t_cputime_tilde, x_f);
    class_addmethod(cputime_tilde_class, (t_method)cputime_tilde_dsp, gensym("dsp"), 0);
}
