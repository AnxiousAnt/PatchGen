/*
 * vrelay: pass a list through a certain outlet based on a match at a given index
 *
 * (c) 2016 Christof Ressi - based on [relay] from zexy
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * use creation arguments to specify the atoms incoming lists are checked against.
 * you can later change these later by sending a [set ...( message.
 * per default, all atoms are compared with the first atom of incoming lists.
 * you can change the index with the second inlet.
 *
 */

#include "m_pd.h"

static t_class *vrelay_class;

typedef struct _vrelay
{
  t_object x_obj;

  t_atom *atoms;
  int    n;
  
  t_outlet **out;
  t_outlet *reject;
  int index;
} t_vrelay;


static int testatoms(t_atom *a, t_atom *b){
	if (a->a_type != b->a_type){
	  return 0;
	}
	
	switch (a->a_type){
      case A_FLOAT: 
	    return (a->a_w.w_float == b->a_w.w_float);
	    break;
	  case A_SYMBOL:
	    return (a->a_w.w_symbol == b->a_w.w_symbol);
	    break;
	  default:
	    return 0;
	    break;
	}
}

static void vrelay_set(t_vrelay *x, t_symbol *s, int argc, t_atom *argv){ 
  if (argc > x->n) argc = x->n;
  
  t_atom *atoms = x->atoms;
  
  while (argc--) {
    atoms->a_type = argv->a_type;
	atoms->a_w = argv->a_w;
	atoms++;
	argv++;
  }
}

static void vrelay_index(t_vrelay *x, t_floatarg index){
  if (index < 0) index = 0;
  x->index = index;
}


static void vrelay_list(t_vrelay *x, t_symbol *s, int argc, t_atom *argv) {
  /* handles float, symbol, list and bang messages */
  
  if (!argc) {
    outlet_bang(x->reject);
    return;
  }
     
  // we have argc elements (selector is not part of the list)
  if (x->index >= argc){
    outlet_list(x->reject, s, argc, argv);
    return;
  } 
  
  const int n = x->n;
  t_atom *test = x->atoms;
  const int k = x->index;
  
  for (int i = 0; i < n; i++){
    if (testatoms(test+i, argv+k)){
	  outlet_list(x->out[i], s, argc, argv);
      return;
	}
  }    
  
  outlet_list(x->reject, s, argc, argv);  
}


static void vrelay_any(t_vrelay *x, t_symbol *s, int argc, t_atom *argv) {
  // we have argc + 1 elements (selector is part of the list)
  if (x->index > argc){
    outlet_anything(x->reject, s, argc, argv);
    return;
  } 
  
  const int n = x->n;
  t_atom *test = x->atoms;
  const int k = x->index - 1;
  
  if (k < 0){
    for (int i = 0; i < n; i++){
      if (test[i].a_type == A_SYMBOL && test[i].a_w.w_symbol == s){
	    outlet_anything(x->out[i], s, argc, argv);
	    return;
	  }
    }  
  } else {
	for (int i = 0; i < n; i++){
      if (testatoms(test+i, argv+k)){
	    outlet_anything(x->out[i], s, argc, argv);
	    return;
	  }
    }    
  }
  
  outlet_anything(x->reject, s, argc, argv);
}


static void *vrelay_new(t_symbol *s, int argc, t_atom *argv)
{
  t_vrelay *x = (t_vrelay *)pd_new(vrelay_class);

  x->index = 0;
  
  // allocate and initialize list of atoms
  x->atoms = (t_atom*)getbytes(argc * sizeof(t_atom));
  x->n = argc;
  vrelay_set(x, s, argc, argv);
  
  // make right inlet
  inlet_new((t_object*)x, (t_pd*)x, &s_float, gensym(""));  

  // allocate outlets
  x->out = (t_outlet **)getbytes(argc * sizeof(t_outlet *));
  t_outlet **out = x->out;
  for (int i = 0; i < argc; i++){
	  *out++ = outlet_new((t_object*)x, 0);
  }
  x->reject = outlet_new((t_object*)x, 0);
  
  return (x);
}

static void vrelay_free(t_vrelay*x){
  const int n = x->n;

  if(x->atoms){
    freebytes(x->atoms, n * sizeof(t_atom));
  }
  
  if(x->out){
    freebytes(x->out, n * sizeof(t_outlet *));
  }
}

void vrelay_setup(void)
{
  vrelay_class = class_new(gensym("vrelay"), (t_newmethod)vrelay_new,
			(t_method)vrelay_free, sizeof(t_vrelay), 0, A_GIMME,  0);
  class_addmethod(vrelay_class, (t_method)vrelay_set, gensym("set"), A_GIMME, 0);
  class_addmethod(vrelay_class, (t_method)vrelay_index, gensym(""), A_FLOAT, 0);
  class_addlist(vrelay_class, vrelay_list);
  class_addanything(vrelay_class, vrelay_any);

}
