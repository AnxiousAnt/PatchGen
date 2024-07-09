/* code for symsplit pd class */

#include "m_pd.h"

#define MAXHEAD 1024

typedef struct{
    char buffer[MAXHEAD];
} t_symsplit_data;
 
typedef struct symsplit
{
  t_object t_ob;
  t_outlet *x_out;  
  t_outlet *x_out_rest;
  t_symsplit_data x_c;
} t_symsplit;

void symsplit_symbol(t_symsplit *x, t_symbol *s)
{
    char* p    = s->s_name;      
    char* head = x->x_c.buffer;
    int   i    = 0;


    /* copy head */
    while (( i < (MAXHEAD-1)) && (*p) && (*p!='-'))
    {
	head[i] = *p;
	i++;
	p++;
    }

    /* terminate head */
    head[i] = 0;

    /* skip separator */
    if (*p=='-') p++;

    /* if the tail part is not empty, send to right output */
    if (*p) outlet_symbol(x->x_out_rest, gensym(p));

    /* send the head part to left output */
    outlet_symbol(x->x_out, gensym(head));

}


void symsplit_free(void)
{
}

t_class *symsplit_class;

void *symsplit_new(void)
{
    t_symsplit *x = (t_symsplit *)pd_new(symsplit_class);

    x->x_out = outlet_new(&x->t_ob, gensym("symbol"));
    x->x_out_rest = outlet_new(&x->t_ob, gensym("symbol"));


    return (void *)x;
}

void symsplit_setup(void)
{
    symsplit_class = class_new(gensym("symsplit"), (t_newmethod)symsplit_new,
    	(t_method)symsplit_free, sizeof(t_symsplit), 0, 0);
    class_addsymbol(symsplit_class, symsplit_symbol);
}

