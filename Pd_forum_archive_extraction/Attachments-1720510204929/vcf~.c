#include <math.h>
#include <m_pd.h>

typedef struct vcfctl
{
    double c_re;
    double c_im;
    double c_q;
    double c_isr;
} t_vcfctl;

typedef struct sigvcf
{
    t_object x_obj;
    t_vcfctl x_cspace;
    t_vcfctl *x_ctl;
    float x_f;
} t_sigvcf;

t_class *sigvcf_class;

static void *sigvcf_new(t_floatarg q)
{
    t_sigvcf *x = (t_sigvcf *)pd_new(sigvcf_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));
    outlet_new(&x->x_obj, gensym("signal"));
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_ctl = &x->x_cspace;
    x->x_cspace.c_re = 0;
    x->x_cspace.c_im = 0;
    x->x_cspace.c_q = q;
    x->x_cspace.c_isr = 0;
    x->x_f = 0;
    return (x);
}

static void sigvcf_ft1(t_sigvcf *x, t_floatarg f)
{
    x->x_ctl->c_q = (f > 0 ? f : 0.f);
}

static t_int *sigvcf_perform(t_int *w)
{
    float *in1 = (float *)(w[1]);
    float *in2 = (float *)(w[2]);
    float *out1 = (float *)(w[3]);
    float *out2 = (float *)(w[4]);
    t_vcfctl *c = (t_vcfctl *)(w[5]);
    int n = (t_int)(w[6]);
    int i;
    double re = c->c_re, re2;
    double im = c->c_im;
    double q = c->c_q;
    double qinv = (q > 0? 1.0f/q : 0);
    double ampcorrect = 2.0f - 2.0f / (q + 2.0f);
    double isr = c->c_isr;
    double coefr, coefi;
    double f1;
    for (i = 0; i < n; i++)
    {
        double cf, r, oneminusr;
        cf = *in2++ * isr;
        if (cf < 0) cf = 0;
        r = (qinv > 0 ? 1 - cf * qinv : 0);
        if (r < 0) r = 0;
        oneminusr = 1.0f - r;
        coefr = r * cos(cf);
        coefi = r * sin(cf);
        f1 = *in1++;
        re2 = re;
        *out1++ = re = ampcorrect * oneminusr * f1 
            + coefr * re2 - coefi * im;
        *out2++ = im = coefi * re2 + coefr * im;
    }
    c->c_re = re;
    c->c_im = im;
    return (w+7);
}

static void sigvcf_dsp(t_sigvcf *x, t_signal **sp)
{
    x->x_ctl->c_isr = 6.28318f/sp[0]->s_sr;
    dsp_add(sigvcf_perform, 6,
        sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, 
            x->x_ctl, sp[0]->s_n);
}

void vcf_tilde_setup(void)
{
    sigvcf_class = class_new(gensym("vcf~"), (t_newmethod)sigvcf_new, 0,
        sizeof(t_sigvcf), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(sigvcf_class, t_sigvcf, x_f);
    class_addmethod(sigvcf_class, (t_method)sigvcf_dsp, gensym("dsp"), 0);
    class_addmethod(sigvcf_class, (t_method)sigvcf_ft1,
        gensym("ft1"), A_FLOAT, 0);
}
