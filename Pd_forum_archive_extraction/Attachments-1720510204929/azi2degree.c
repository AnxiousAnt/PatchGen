
/* code for azi2degree pd class */

#include "m_pd.h"

typedef struct azi2degree
{
  t_object x_obj;
} t_azi2degree;

void azi2degree_float(t_azi2degree *x, t_floatarg f)
{
    //post("azi2degree: %f", f);
    t_float degree;
    t_int a = f;
    
  switch(a)
  {
	  case 1:
	   	degree = -80;
		break;
	  case 2:
		degree = -65;
		break;
	  case 3:
	 	degree = -55;
		break;
	  case 4:
		degree = -45;
		break;
	  case 5:
	 	degree = -40;
		break;
	  case 6:
 		degree = -35;
		break;
	  case 7:
 		degree = -30;
		break;
	  case 8:
	 	degree = -25;
		break;
	  case 9:
	 	degree = -20;
		break;
	  case 10:   
  		degree = -15;
		break;
	  case 11:
		degree = -11;
		break;
	  case 12:
		degree = -5;
		break;
       	  case 13:
		degree = 0;
		break;
	  case 14:
		degree = 5;
		break;
	  case 15:
		degree = 10;
		break;
	  case 16:
		degree = 15;
		break;
	  case 17:
		degree = 20;
		break;
	  case 18:
		degree = 25;
		break;
	  case 19:
		degree = 30;
		break;
	  case 20:
		degree = 35;
		break;
	  case 21:
		degree = 40;
		break;
	  case 22:
		degree = 45;
		break;
	  case 23:
		degree = 55;
		break;
	  case 24:
		degree = 65;
		break;
	  case 25:
		degree = 80;
		break;
  }
		  
    
    outlet_float(x->x_obj.ob_outlet, degree);
}

void azi2degree_rats(t_azi2degree *x)
{
    post("azi2degree: rats");
}

t_class *azi2degree_class;

void *azi2degree_new(void)
{
    t_azi2degree *x = (t_azi2degree *)pd_new(azi2degree_class);
    post("azi2degree_new");
    outlet_new(&x->x_obj, &s_float);
    return (void *)x;
}

void azi2degree_setup(void)
{
    post("azi2degree_setup");
    azi2degree_class = class_new(gensym("azi2degree"), (t_newmethod)azi2degree_new, 0,
    	sizeof(t_azi2degree), 0, 0);
    class_addmethod(azi2degree_class, (t_method)azi2degree_rats, gensym("rats"), 0);
    class_addfloat(azi2degree_class, azi2degree_float);
}

