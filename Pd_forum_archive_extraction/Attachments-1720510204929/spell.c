/*

spel object

*/

/* derived from foo1 example extern */

#include "m_pd.h"
#include <stdlib.h>
#include <stdio.h>


typedef struct cabangs
{
  t_object t_ob;
  t_outlet *outlet1;
} t_spell;

void spell_float(t_spell *x, t_floatarg f)
{
      post("Array is not that big: %f  -- not possible",f);
}

void spell_bang(t_spell *x)
{
}

void spell_print(t_spell *x)
{
}

void spell_symbol(t_spell *x, t_symbol *s)  
/* guessing that the symbol comes here */
{
   char name[2]={32,0};
   int i;
   for (i=0;s->s_name[i]; i++)
     {
       name[0] = s->s_name[i];
       outlet_symbol(x->outlet1,gensym(name));
     }

}


t_class *spell_class;


void rulenumber2rule(int n, int *array)
{
  int i;
}


void *spell_new(void)
{
  int i,j;
    t_spell *x = (t_spell *)pd_new(spell_class);
    post("spell_new");
    x->outlet1 = outlet_new(&x->t_ob,&s_bang);
    return (void *)x;
}

void spell_setup(void)
{
    post("spell_setup");
    spell_class = class_new(gensym("spell"), (t_newmethod)spell_new, 0,
    	sizeof(t_spell), 0, 0);
    class_addmethod(spell_class, (t_method)spell_print, 
		    gensym("print"), 0);
    /* add method for any symbol*/
    class_addsymbol(spell_class, (t_method)spell_symbol);
    class_addbang(spell_class, (t_method)spell_bang);
    class_addfloat(spell_class, spell_float);
}


