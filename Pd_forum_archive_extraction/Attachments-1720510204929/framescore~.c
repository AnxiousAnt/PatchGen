/*
 *  framescore~ : Weighted block comparison.
 *  Copyright (C) 2005 Edward Kelly <morph_2016@yahoo.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "m_pd.h"
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#ifdef NT
#include <windows.h>
#include <commctrl.h>
#endif

static t_class *framescore_tilde_class;
extern void framescore_tilde_setup;

typedef struct _framescore_tilde 
{
  t_object x_obj;
  t_float f;
  t_float fmax, fwin, faccum;
  t_outlet *fscore;
} t_framescore_tilde;

t_int *framescore_tilde_perform(t_int *w)
{
  t_framescore_tilde *x = (t_framescore_tilde *)(w[1]);
  t_sample         *in1 =           (t_sample *)(w[2]);
  t_sample         *in2 =           (t_sample *)(w[3]);
  int                 n =                  (int)(w[4]);
  float vector1;
  float vector2;
  x->faccum = 0;
  float blockaccum = 0;
  x->fmax = 0;
  float score = 0;
  float diff = 0;
  float avg = 0;
  int block = n;
  x->fwin = x->fwin > 0 ? x->fwin : 0.01;

  while (n--)
    {
      vector1 = (*in1++);
      vector2 = (*in2++);
      vector1 = vector1 > 0 ? vector1 : 0 - vector1;
      vector2 = vector2 > 0 ? vector2 : 0 - vector2;
      blockaccum += vector2;
      x->fmax = vector2 > x->fmax ? vector2 : x->fmax;
      diff = vector1 > vector2 ? vector1 - vector2 : vector2 - vector1;
      x->faccum += (1/((diff/x->fwin)+1)) * vector2;
    }
  score = x->faccum / x->fmax;
  blockaccum /= x->fmax;
  avg = score / blockaccum;
  outlet_float(x->fscore, avg);
  
  return(w+5);
}

void framescore_tilde_dsp(t_framescore_tilde *x, t_signal **sp)
{
  dsp_add(framescore_tilde_perform, 4, x,
	  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void *framescore_tilde_new(t_floatarg f)
{
  t_framescore_tilde *x = (t_framescore_tilde *)pd_new(framescore_tilde_class);

  x->fwin = f;

  inlet_new (&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  floatinlet_new (&x->x_obj, &x->fwin);
  x->fscore = outlet_new(&x->x_obj, gensym("float"));

  return (void *)x;
}

void framescore_tilde_setup(void)
{
  framescore_tilde_class = class_new(gensym("framescore~"),
				     (t_newmethod)framescore_tilde_new,
				     0, sizeof(t_framescore_tilde),
				     CLASS_DEFAULT, A_DEFFLOAT, 0);

  post("|+++++++++++framescore~+++++++++++++|");
  post("|+++++weighted block comparison+++++|");
  post("|+++edward+++++++kelly+++++++2005+++|");

  class_addmethod(framescore_tilde_class, (t_method)framescore_tilde_dsp,
		  gensym("dsp"), 0);
  class_sethelpsymbol(framescore_tilde_class, gensym("help-framescore~"));
  CLASS_MAINSIGNALIN(framescore_tilde_class, t_framescore_tilde, f);
}

