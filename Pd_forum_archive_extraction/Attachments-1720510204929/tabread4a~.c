/******************** tabread4a~ ***********************/
/*  The guts of this external are "borrowed" 
    from tabread4~.  For normal or slower playback 
    speeds, this external is intended to simply run
    the same as tabread4~. For faster speeds, the 
    interpolation polynomial is modified to eliminate 
    aliased frequencies. */

#include<m_pd.h>
#include<math.h>

float interp(float x)
{
    float absx=fabsf(x);
    return ((absx<2.0f)*((absx<1.0f)?(1-absx*(0.5f+absx*(1-0.5f*absx))):(1-absx*(1.833333f-absx*(1.0f-0.1666666f*absx)))));
}

static t_class *tabread4a_tilde_class;

typedef struct _tabread4a_tilde
{
    t_object x_obj;
    int x_npoints;
    float *x_vec;
    t_symbol *x_arrayname;
    float x_f;
    float last_input;
} t_tabread4a_tilde;

static void *tabread4a_tilde_new(t_symbol *s)
{
    t_tabread4a_tilde *x = (t_tabread4a_tilde *)pd_new(tabread4a_tilde_class);
    x->x_arrayname = s;
    x->x_vec = 0;
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    x->last_input=0;
    return (x);
}

static t_int *tabread4a_tilde_perform(t_int *w)
{
    t_tabread4a_tilde *x = (t_tabread4a_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);    
    int maxindex;
    float *buf = x->x_vec, *fp;
    int i;
    float diff, findex;
    
    maxindex = x->x_npoints - 3;

    if (!buf) goto zero;

#if 0       /* test for spam -- I'm not ready to deal with this */
    for (i = 0,  xmax = 0, xmin = maxindex,  fp = in1; i < n; i++,  fp++)
    {
        float f = *in1;
        if (f < xmin) xmin = f;
        else if (f > xmax) xmax = f;
    }
    if (xmax < xmin + x->c_maxextent) xmax = xmin + x->c_maxextent;
    for (i = 0, splitlo = xmin+ x->c_maxextent, splithi = xmax - x->c_maxextent,
        fp = in1; i < n; i++,  fp++)
    {
        float f = *in1;
        if (f > splitlo && f < splithi) goto zero;
    }
#endif

    findex = *in;
    diff=fabsf(findex-x->last_input);
    x->last_input=*(in+n-1);
    if (diff > ((float) (maxindex/2)))
        diff=((float) maxindex) - diff;
    for (i = 0; i < n; i++)
    {
        if (diff <= 1.0f)
        {
            int index = findex;
            float frac,  a,  b,  c,  d, cminusb;
            static int count;
            if (index < 1)
                index = 1, frac = 0;
            else if (index > maxindex)
                index = maxindex, frac = 1;
            else frac = findex - index;
            fp = buf + index;
            a = fp[-1];
            b = fp[0];
            c = fp[1];
            d = fp[2];
            /* if (!i && !(count++ & 1023))
                post("fp = %lx,  shit = %lx,  b = %f",  fp, buf->b_shit,  b); */
            cminusb = c-b;
            *out++ = b + frac * (
                cminusb - 0.1666667f * (1.-frac) * (
                    (d - a - 3.0f * cminusb) * frac + (d + 2.0f*a - 3.0f*b)
                )
            );
        }
        else
        {
            int a,b,c, itemp, itemp2, itemp3;
            float sum_left, sum_right;
            a=ceilf(findex-2*diff);    //lowest value used in interpolation
            b=findex;                  //floor( findex)
            c=findex+2*diff;           // highest value used, floor(findex+2*diff)
            
            if ((a>0)&&(c<=maxindex))
            {
                fp=buf+a;
                sum_left=(*fp++)*interp((findex-a)/diff);
                while((a++)<b)
                    sum_left+=(*(fp++))*interp((findex-a)/diff);
                fp=buf+c;
                sum_right=(*(fp--))*interp((c-findex)/diff);
                itemp=b+1;
                while((c--)>itemp)
                    sum_right+=(*(fp--))*interp((c-findex)/diff);
            }
            else
            {
                itemp=a;               //1st, wrap "a" btw 1 and maxindex
                while(itemp<1)
                    itemp+=maxindex;
                fp=buf+(itemp++);
                sum_left=(*fp++)*interp((findex-a++)/diff);  //eval at 1st point
                while(a<=b)   // eval at each other point, from a to b
                {
                    itemp2=b-a+1;
                    itemp3=maxindex-itemp+1;
                    itemp=(itemp2<itemp3)?itemp2:itemp3;
                    while(itemp--)
                        sum_left+=(*fp++)*interp((findex-a++)/diff);
                    fp=buf+1;
                    itemp=1;                        
                }
                itemp=c;
                while(itemp>maxindex)
                    itemp-=maxindex;
                fp=buf+(itemp--);
                sum_right=(*fp--)*interp(((c--)-findex)/diff);
                while(c>b)
                {
                    itemp2=c-b;
                    itemp3=itemp;
                    itemp=(itemp2<itemp3)?itemp2:itemp3;
                    while(itemp--)
                        sum_right+=(*fp--)*interp(((c--)-findex)/diff);
                    fp=buf+maxindex;                        
                    itemp=maxindex;
                }
            }
            *out++=(sum_left+sum_right)/diff;
        }
        in++;
        diff=fabsf(*in-findex);
        if (diff > ((float) (maxindex/2)))
            diff=((float) maxindex) - diff;
        findex = *in;
    }
    return (w+5);
 zero:
    while (n--) *out++ = 0;

    return (w+5);
}

void tabread4a_tilde_set(t_tabread4a_tilde *x, t_symbol *s)
{
    t_garray *a;
    
    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            pd_error(x, "tabread4a~: %s: no such array", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getfloatarray(a, &x->x_npoints, &x->x_vec))
    {
        pd_error(x, "%s: bad template for tabread4a~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_usedindsp(a);
}

static void tabread4a_tilde_dsp(t_tabread4a_tilde *x, t_signal **sp)
{
    tabread4a_tilde_set(x, x->x_arrayname);

    dsp_add(tabread4a_tilde_perform, 4, x,
        sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void tabread4a_tilde_free(t_tabread4a_tilde *x)
{
}

EXTERN void tabread4a_tilde_setup(void)
{
    tabread4a_tilde_class = class_new(gensym("tabread4a~"),
        (t_newmethod)tabread4a_tilde_new, (t_method)tabread4a_tilde_free,
        sizeof(t_tabread4a_tilde), 0, A_DEFSYM, 0);
    CLASS_MAINSIGNALIN(tabread4a_tilde_class, t_tabread4a_tilde, x_f);
    class_addmethod(tabread4a_tilde_class, (t_method)tabread4a_tilde_dsp,
        gensym("dsp"), 0);
    class_addmethod(tabread4a_tilde_class, (t_method)tabread4a_tilde_set,
        gensym("set"), A_SYMBOL, 0);
}
