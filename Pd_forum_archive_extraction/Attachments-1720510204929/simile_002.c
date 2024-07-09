#include "m_pd.h"

typedef struct _simile {
  t_object  x_obj;
  t_float x_win;
  float min;
  float sign;
  t_float x_in1;
  t_float x_in2;
  t_float x_result;
  t_outlet *x_outlet;
} t_simile;

void simile_float(t_simile *x) {
  x->min = ( x->x_in1 > x->x_in2 ) ? x->x_in1 - x->x_in2 : x->x_in2 - x->x_in1;
  x->sign = ( x->x_in1 > x->x_in2 ) ? 1 : -1;
  x->x_result = (x->min * x->x_win) * x->sign;
  outlet_float(x->x_outlet, x->x_result);
}
	
void simile_fin1(t_simile *x) {
  x->min = ( x->x_in1 > x->x_in2 ) ? x->x_in1 - x->x_in2 : x->x_in2 - x->x_in1;
  x->sign = ( x->x_in1 > x->x_in2 ) ? 1 : -1;
  x->x_result = (x->min * x->x_win) * x->sign;
  outlet_float(x->x_outlet, x->x_result);
}
	
void simile_bang(t_simile *x) {
  outlet_float(x->x_outlet, x->x_result);
}

t_class *simile_class;

void *simile_new(t_floatarg x_win) {
  t_simile *x = (t_simile *)pd_new(simile_class);
	
  x->x_outlet = outlet_new(&x->x_obj, gensym("float"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("fin1"));
  floatinlet_new(&x->x_obj, &x->x_in2);
  floatinlet_new(&x->x_obj, &x->x_win);
  return (void *)x;
}

void simile_setup(void) {
  simile_class = class_new(gensym("simile")),
  (t_newmethod)simile_new,
  0, sizeof(t_simile),
  0, A_DEFFLOAT, 0);
	
  class_addbang(simile_class, simile_bang);
  class_addfloat(simile_class, simile_float);
}
	
	