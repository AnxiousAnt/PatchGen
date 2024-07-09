////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute. IEM
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "cube2obj.h"
#include "Base/GemState.h"

CPPEXTERN_NEW_WITH_ONE_ARG(cube2obj, t_floatarg, A_DEFFLOAT)

/////////////////////////////////////////////////////////
//
// cube2obj
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
cube2obj :: cube2obj(t_floatarg size)
      : GemShape(size)
{  
	m_outletObj = outlet_new(this->x_obj, 0);
	m_size = size;
	if (size == 0) size = 1;
    m_snap = 0;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
cube2obj :: ~cube2obj()
{   outlet_free(m_outletObj);
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void cube2obj :: render(GemState *state)
{
	float mi[16]={0};
	t_atom alist[5];

	if(m_snap==1)
	{
		glGetFloatv(GL_MODELVIEW_MATRIX,mi);

		SETSYMBOL(alist+0, gensym("v"));
		SETFLOAT(alist+1,  mi[12] + mi[0] * m_size + mi[4] * m_size + mi[8]  * m_size );
		SETFLOAT(alist+2,  mi[13] + mi[1] * m_size + mi[5] * m_size + mi[9]  * m_size );
		SETFLOAT(alist+3,  mi[14] + mi[2] * m_size + mi[6] * m_size + mi[10] * m_size );
		outlet_list (m_outletObj, &s_list, 4, alist); // 1 1 1
		SETFLOAT(alist+1,  mi[12] + mi[0] * m_size + mi[4] * m_size - mi[8]  * m_size );
		SETFLOAT(alist+2,  mi[13] + mi[1] * m_size + mi[5] * m_size - mi[9]  * m_size );
		SETFLOAT(alist+3,  mi[14] + mi[2] * m_size + mi[6] * m_size - mi[10] * m_size );
		outlet_list (m_outletObj, &s_list, 4, alist); // 1 1 -1
		SETFLOAT(alist+1,  mi[12] + mi[0] * m_size - mi[4] * m_size - mi[8]  * m_size );
		SETFLOAT(alist+2,  mi[13] + mi[1] * m_size - mi[5] * m_size - mi[9]  * m_size );
		SETFLOAT(alist+3,  mi[14] + mi[2] * m_size - mi[6] * m_size - mi[10] * m_size );
		outlet_list (m_outletObj, &s_list, 4, alist); // 1 -1 -1
		SETFLOAT(alist+1,  mi[12] + mi[0] * m_size - mi[4] * m_size + mi[8]  * m_size );
		SETFLOAT(alist+2,  mi[13] + mi[1] * m_size - mi[5] * m_size + mi[9]  * m_size );
		SETFLOAT(alist+3,  mi[14] + mi[2] * m_size - mi[6] * m_size + mi[10] * m_size );
		outlet_list (m_outletObj, &s_list, 4, alist); // 1 -1 1
		SETFLOAT(alist+1,  mi[12] - mi[0] * m_size + mi[4] * m_size + mi[8]  * m_size );
		SETFLOAT(alist+2,  mi[13] - mi[1] * m_size + mi[5] * m_size + mi[9]  * m_size );
		SETFLOAT(alist+3,  mi[14] - mi[2] * m_size + mi[6] * m_size + mi[10] * m_size );
		outlet_list (m_outletObj, &s_list, 4, alist); // -1 1 1
		SETFLOAT(alist+1,  mi[12] - mi[0] * m_size + mi[4] * m_size - mi[8]  * m_size );
		SETFLOAT(alist+2,  mi[13] - mi[1] * m_size + mi[5] * m_size - mi[9]  * m_size );
		SETFLOAT(alist+3,  mi[14] - mi[2] * m_size + mi[6] * m_size - mi[10] * m_size );
		outlet_list (m_outletObj, &s_list, 4, alist); // -1 1 -1
		SETFLOAT(alist+1,  mi[12] - mi[0] * m_size - mi[4] * m_size - mi[8]  * m_size );
		SETFLOAT(alist+2,  mi[13] - mi[1] * m_size - mi[5] * m_size - mi[9]  * m_size );
		SETFLOAT(alist+3,  mi[14] - mi[2] * m_size - mi[6] * m_size - mi[10] * m_size );
		outlet_list (m_outletObj, &s_list, 4, alist); // -1 -1 -1
		SETFLOAT(alist+1,  mi[12] - mi[0] * m_size - mi[4] * m_size + mi[8]  * m_size );
		SETFLOAT(alist+2,  mi[13] - mi[1] * m_size - mi[5] * m_size + mi[9]  * m_size );
		SETFLOAT(alist+3,  mi[14] - mi[2] * m_size - mi[6] * m_size + mi[10] * m_size );
		outlet_list (m_outletObj, &s_list, 4, alist); // -1 -1 1
	

		SETSYMBOL(alist+0, gensym("f"));
		SETFLOAT(alist+1, -4);
		SETFLOAT(alist+2, -3);
		SETFLOAT(alist+3, -2);
		SETFLOAT(alist+4, -1);
		outlet_list (m_outletObj, &s_list, 5, alist);

		SETFLOAT(alist+1, -5);
		SETFLOAT(alist+2, -6);
		SETFLOAT(alist+3, -7);
		SETFLOAT(alist+4, -8);
		outlet_list (m_outletObj, &s_list, 5, alist);

		SETFLOAT(alist+1, -2);
		SETFLOAT(alist+2, -3);
		SETFLOAT(alist+3, -7);
		SETFLOAT(alist+4, -6);
		outlet_list (m_outletObj, &s_list, 5, alist);

		SETFLOAT(alist+1, -8);
		SETFLOAT(alist+2, -4);
		SETFLOAT(alist+3, -1);
		SETFLOAT(alist+4, -5);
		outlet_list (m_outletObj, &s_list, 5, alist);

		SETFLOAT(alist+1, -3);
		SETFLOAT(alist+2, -4);
		SETFLOAT(alist+3, -8);
		SETFLOAT(alist+4, -7);
		outlet_list (m_outletObj, &s_list, 5, alist);

		SETFLOAT(alist+1, -2);
		SETFLOAT(alist+2, -6);
		SETFLOAT(alist+3, -5);
		SETFLOAT(alist+4, -1);
		outlet_list (m_outletObj, &s_list, 5, alist);

	}

//	m_snap=0;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void cube2obj :: obj_setupCallback(t_class *classPtr)
{ 
    class_addmethod(classPtr, (t_method)&cube2obj::snapMessCallback,
    	    gensym("snap"), A_FLOAT, A_NULL);
}

void cube2obj :: snapMessCallback(void *data, float snap)
{
    GetMyClass(data)->m_snap=int(snap);
}

