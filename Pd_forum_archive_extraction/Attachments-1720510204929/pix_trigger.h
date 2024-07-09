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

#ifndef _PIX_TRIGGER_H
#define _PIX_TRIGGER_H

#include "Base/GemPixObj.h"
#include "Base/GemPixUtil.h"

class GEM_EXTERN pix_trigger : public GemPixObj
{
	CPPEXTERN_HEADER(pix_trigger, GemPixObj);

 public:
	// constructor
	pix_trigger(int argc, t_atom *argv);


 protected:
	// destructor
	virtual ~pix_trigger();

	// processing routines /* todo which one? */
	virtual void processGrayImage(imageStruct &image);
	
	// trigger area
	unsigned int x1, x2, y1, y2;
	
	t_outlet * m_bang;

    // quantiles of last n frames
    unsigned char quantiles[1024];
    // actual position in quantile ringbuffer
    unsigned int quantile_pos;
	// last frame
	unsigned char* old;

	t_float m_quantile;
	t_float m_factor;

	void set_quantile(void *data, t_floatarg f);
	void set_factor(void *data, t_floatarg f);

	void pix_trigger :: set_quantile(t_floatarg f);
	void pix_trigger :: set_factor(t_floatarg f);

	static void pix_trigger :: set_quantile_callback(void *data, t_floatarg f);
	static void pix_trigger :: set_factor_callback(void *data, t_floatarg f);
	
};


#endif /* _PIX_TRIGGER_H */
