// Copyright (c) 2004 Tim Blechmann, Thomas Rühr
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "GEM.LICENSE.TERMS"  in this distribution.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// See file GEM.LICENSE.TERMS for further informations on licensing terms.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "pix_trigger.h"


CPPEXTERN_NEW_WITH_GIMME(pix_trigger)

  /* const, destructor */
  pix_trigger::pix_trigger(int argc, t_atom *argv)
{
  if (argc != 4)
    {
      error("4 arguments are required");
      return;
    }
  x1 = atom_getintarg(0, argc, argv);
  x2 = atom_getintarg(1, argc, argv);
  y1 = atom_getintarg(2, argc, argv);
  y2 = atom_getintarg(3, argc, argv);

  inlet_new(this->x_obj, &this->x_obj->ob_pd, 
	    gensym("float"), gensym("quantile"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, 
	    gensym("float"), gensym("factor"));

  m_bang = outlet_new(this->x_obj, 0);

  m_quantile = 0.98f;
  m_factor = 3.0;
	
  /* last frame */
  old = (unsigned char*)getbytes (320*240* sizeof(unsigned char));
}

pix_trigger::~pix_trigger()
{
  freebytes (old, 320*240* sizeof(unsigned char));
  return;
}

void pix_trigger::processGrayImage(imageStruct &image)
{
  int datasize = image.xsize * image.ysize;
  int xsize = image.xsize;

  unsigned char *base = image.data;
  unsigned long histo[256];
  int j = datasize;
  long actsum = 0;
  int quantil = -1;
	
  memset(histo,0,256*sizeof(unsigned long));

  while(j--)
    {
      if (x1 <= (j / xsize) && x2 >= (j / xsize)
	  && y1 <= (j % xsize) && y2 >= (j % xsize))
	{
	  unsigned char diff = base[j] - old[j];
	  histo[diff]++;
	}
    }
	
  for (int i =0; i < 256; i++) 
    {
      actsum += histo[i];
      if ((actsum >= datasize * m_quantile) && (quantil == -1))
	quantil = i;
    }

  quantiles[quantile_pos] = quantil;
  quantile_pos++;

  if (quantile_pos >= 100) 
    quantile_pos = 0;

  long quantil_mittelwert = 0;
  for (int i =0; i < 100; i++) 
    {
      quantil_mittelwert += quantiles[i];
    }
	
  quantil_mittelwert /= 100;
  actsum = 0;
  for (int i =quantil_mittelwert; i < 256; i++) 
    {
      actsum += histo[i];
    }

  if (actsum > (1-m_quantile) * (x2 - x1) * (y2 - y1) * m_factor) 
    {
      outlet_bang(m_bang);
    }
  

  memcpy(old, base, 320*240* sizeof(unsigned char));
}
	



void pix_trigger :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, (t_method)&pix_trigger::set_quantile_callback,
		  gensym("quantile"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_trigger::set_factor_callback,
		  gensym("factor"), A_FLOAT, A_NULL);
}



void pix_trigger :: set_quantile(t_floatarg f)
{
  m_quantile = f;
}

void pix_trigger :: set_factor(t_floatarg f)
{
  m_factor = f;
}


void pix_trigger :: set_quantile_callback(void *data, t_floatarg f)
{
  GetMyClass(data)->set_quantile(f);
}

void pix_trigger :: set_factor_callback(void *data, t_floatarg f)
{
  GetMyClass(data)->set_factor(f);
}
