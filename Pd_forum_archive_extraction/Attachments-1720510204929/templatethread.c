#include "m_pd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pthread.h"

typedef struct _pdexterna
{
	t_object 		p_ob;					// object header - ALL pd external MUST begin with this...
	void 			*outlets[OUTLETS];		// handle to the objects outlets
	int				x_verbose;
	
	pthread_attr_t 	pdexterna_thread_attr;
	pthread_t    	x_threadid;
} t_pdexterna;

void *pdexterna_class;					// global pointer to the object class - so pd can reference the object 

// ==============================================================================
// Function Prototypes
// ------------------------------------------------------------------------------
void *pdexterna_new(t_symbol *s);
void pdexterna_bang(t_pdexterna *x);				

// =============================================================================
// Thread crap
// =============================================================================
static void *usb_thread_read(void *w)
{
	t_pdexterna *x = (t_pdexterna*) w;
	while(1) {
		pthread_testcancel();
		//sys_lock();
		pdexterna_bang(x);
		//sys_unlock();
	}
}

static void usb_thread_start(t_pdexterna *x)
{

	// create the worker thread
    if(pthread_attr_init(&x->pdexterna_thread_attr) < 0)
	{
       error("pdexterna: could not launch receive thread");
       return;
    }
    if(pthread_attr_setdetachstate(&x->pdexterna_thread_attr, PTHREAD_CREATE_DETACHED) < 0)
	{
       error("pdexterna: could not launch receive thread");
       return;
    }
    if(pthread_create(&x->x_threadid, &x->pdexterna_thread_attr, usb_thread_read, x) < 0)
	{
       error("pdexterna: could not launch receive thread");
       return;
    }
    else
    {
       if(x->x_verbose)post("pdexterna: thread %d launched", (int)x->x_threadid );
    }
}



//--------------------------------------------------------------------------
// - Message: bang  -> poll pdexterna
//--------------------------------------------------------------------------
void pdexterna_bang(t_pdexterna *x)	// poll pdexterna
{
		//whatever
}


//--------------------------------------------------------------------------
// - Object creation and setup
//--------------------------------------------------------------------------
int pdexterna_setup(void)
{
	pdexterna_class = class_new ( gensym("pdexterna"),(t_newmethod)pdexterna_new, 0, sizeof(t_pdexterna), 	CLASS_DEFAULT,0);
	
	// Add message handlers
	class_addbang(pdexterna_class, (t_method)pdexterna_bang);
	post("pdexterna version 0.1",0);
	return 1;
}

//--------------------------------------------------------------------------
void *pdexterna_new(t_symbol *s)		// s = optional argument typed into object box (A_SYM) -- defaults to 0 if no args are typed
{
	t_pdexterna *x;									// local variable (pointer to a t_pdexterna data structure)
	x = (t_pdexterna *)pd_new(pdexterna_class);			 // create a new instance of this object

    x->x_verbose = 1;

	int i;
	
	// create outlets and assign it to our outlet variable in the instance's data structure
	for (i=0; i < OUTLETS; i++) {
		x->outlets[i] = outlet_new(&x->p_ob, &s_float);
	}	

    usb_thread_start(x);

	return x; // return a reference to the object instance 
}

//--------------------------------------------------------------------------
// - Object destruction
//--------------------------------------------------------------------------
void pdexterna_free(t_pdexterna *x)
{
		while(pthread_cancel(x->x_threadid) < 0)
			if(x->x_verbose)post("pdexterna: killing thread\n");
		if(x->x_verbose)post("pdexterna: thread canceled\n");
    		
}
