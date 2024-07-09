/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A cube2obj2obj

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute. IEM. zmoelnig@iem.kug.ac.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef INCLUDE_CUBE2OBJ_H_
#define INCLUDE_CUBE2OBJ_H_

#include "Base/GemShape.h"
#include <string.h>

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    cube2obj
    
    Creates a cube2obj

KEYWORDS
    geo

DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN cube2obj : public GemShape
{
    CPPEXTERN_HEADER(cube2obj, GemShape)

    public:

	    //////////
	    // Constructor
    	cube2obj(t_floatarg size);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~cube2obj();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);
	
	int	m_snap;
	
    private:
    //////////
    // Static member functions
    static void 	snapMessCallback(void *data, float snap);
	t_outlet    	*m_outletObj;

};

#endif	// for header file
