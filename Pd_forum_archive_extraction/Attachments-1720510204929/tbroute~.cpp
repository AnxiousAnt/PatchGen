/* Copyright (c) 2003 Tim Blechmann.                                            */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "COPYING"  in this distribution.                   */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/* tbroute uses the flext C++ layer for Max/MSP and PD externals.               */
/* get it at http://www.parasitaere-kapazitaeten.de/PD/ext                      */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* See file LICENSE for further informations on licensing terms.                */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */


#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error upgrade your flext version!!!!!!
#endif

class tbroute_tilde: public flext_dsp
{
  FLEXT_HEADER(tbroute_tilde,flext_dsp);

public: // constructor
  tbroute_tilde(int chan);

protected:
  virtual void route (int n, float *const *in, float *const *out);
  void set_route(int i);

private:
  FLEXT_CALLBACK_1(set_route,t_int);
  t_int dest;
    int chan;
};


FLEXT_NEW_DSP_1("tbroute~",tbroute_tilde,int);

tbroute_tilde::tbroute_tilde(t_int chan): chan(chan)
{
    AddInSignal();
    AddInInt();

    for (t_int i=0; i!=chan;++i)
    {    
	AddOutSignal();
    }

    FLEXT_ADDMETHOD(1,set_route);
    
    dest=0;
} 


void tbroute_tilde::route(int n, float *const *in, float *const *out)
{
    const float *ins = in[0];

    float *outs = out[dest];

    int tmp = n;

    while (n--)
    {
	*outs++ = *ins++;
    }

    for (int i = 0; i != chan; i++)
    {
	if (i!=dest)
	{
	    float *outs = out[i];
	    n=tmp;
	    while (n--)
	    {
		*outs++ = 0;
	    }

	}
    }
    
}


void tbroute_tilde::set_route(t_int i)
{
    --i;
    if ((i>-1) && (i<chan))
    {  
	dest=i;
	post("routing to outlet %i",i+1);
    }
    else
    {
	post("no such outlet");
    }

}
