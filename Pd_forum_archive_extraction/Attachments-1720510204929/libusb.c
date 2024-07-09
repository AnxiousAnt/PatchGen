#include "m_pd.h"
#include <usb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==============================================================================
// Constants
// ------------------------------------------------------------------------------
#define USBDEV_SHARED_VENDOR    	0x16c0  /* VOTI */
#define USBDEV_SHARED_PRODUCT   	0x05dc  /* Obdev's free shared PID */
#define OUTLETS 					17
#define DEFAULT_CLOCK_INTERVAL		10		// Magic number 6 (with alsa, under 6 = glitch)
#define USBREPLYBUFFER 				21

// ==============================================================================
// Our External's Memory structure
// ------------------------------------------------------------------------------
typedef struct _edubeat				// defines our object's internal variables for each instance in a patch
{
	t_object 		p_ob;					// object header - ALL pd external MUST begin with this...
	usb_dev_handle	*dev_handle;			// handle to the edubeat usb device
	void			*m_clock;				// handle to our clock
	double 			m_interval;				// clock interval for polling edubeat
	double 			m_interval_bak;			// backup clock interval for polling edubeat
	int				is_running;				// is our clock ticking?
	void 			*outlets[OUTLETS];		// handle to the objects outlets
	int 			values[10];				// stored values from last poll /////////////////////////////////
} t_edubeat;

void *edubeat_class;					// global pointer to the object class - so pd can reference the object 


// ==============================================================================
// Function Prototypes
// ------------------------------------------------------------------------------
void *edubeat_new(t_symbol *s);
void edubeat_assist(t_edubeat *x, void *b, long m, long a, char *s);
void edubeat_bang(t_edubeat *x);				
void edubeat_close(t_edubeat *x);
void edubeat_int(t_edubeat *x, t_floatarg n);
void edubeat_open(t_edubeat *x);
void edubeat_poll(t_edubeat *x, t_floatarg n);
void edubeat_start(t_edubeat *x);
void edubeat_stop(t_edubeat *x);

// functions used to find the USB device
static int  	usbGetStringAscii(usb_dev_handle *dev, int index, int langid, char *buf, int buflen);
void 			find_device(t_edubeat *x);


//--------------------------------------------------------------------------
// - Message: bang  -> poll edubeat
//--------------------------------------------------------------------------
void edubeat_bang(t_edubeat *x)	// poll edubeat
{
	int                 nBytes,i,n;
	int 				replymask,replyshift,replybyte;
	int					temp;
	unsigned char       buffer[USBREPLYBUFFER];
	
	if (!(x->dev_handle)) find_device(x);
	else {
			// ask edubeat to send us data
			nBytes = usb_control_msg(x->dev_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
										EDUBEAT_CMD_POLL, 0, 0, (char *)buffer, sizeof(buffer), 10);
			
			for (i = 0; i < OUTLETS; i++) {
				outlet_float(x->outlets[i], buffer[i]);
			}
	}
}

//--------------------------------------------------------------------------
// - Message: poll 		-> set polling interval
//--------------------------------------------------------------------------
void edubeat_poll(t_edubeat *x, t_floatarg n){
	if (n > 0) { 
		x->m_interval = n;
		x->m_interval_bak = n;
		edubeat_start(x);
	} else {
		edubeat_stop(x);
	}
}

//--------------------------------------------------------------------------
// - Message: open 		-> open connection to edubeat
//--------------------------------------------------------------------------
void edubeat_open(t_edubeat *x)
{
	if (x->dev_handle) {
		post("There is already a connection to Edubeat", 0);
	} else find_device(x);
}

//--------------------------------------------------------------------------
// - Message: close 	-> close connection to edubeat
//--------------------------------------------------------------------------
void edubeat_close(t_edubeat *x)
{
	if (x->dev_handle) {
		usb_close(x->dev_handle);
		x->dev_handle = NULL;
		post("Closed connection to Edubeat",0);
	} else
		post("There was no open connection to Edubeat",0);
}

//--------------------------------------------------------------------------
// - Message: int 		-> zero stops / nonzero starts
//--------------------------------------------------------------------------
void edubeat_int(t_edubeat *x, t_floatarg n) {
	if (n) {
		if (!x->is_running) edubeat_start(x);
	} else {
		if (x->is_running) edubeat_stop(x);
	}
}

//--------------------------------------------------------------------------
// - Message: start 	-> start automatic polling
//--------------------------------------------------------------------------
void edubeat_start (t_edubeat *x) { 
	if (!x->is_running) {
		clock_delay(x->m_clock,0.);
		x->is_running  = 1;
	}
} 

//--------------------------------------------------------------------------
// - Message: stop 		-> stop automatic polling
//--------------------------------------------------------------------------
void edubeat_stop (t_edubeat *x) { 
	if (x->is_running) {
		x->is_running  = 0;
		clock_unset(x->m_clock); 
		edubeat_close(x);
	}
}

//--------------------------------------------------------------------------
// - The clock is ticking, tic, tac...
//--------------------------------------------------------------------------
void edubeat_tick(t_edubeat *x) { 
	clock_delay(x->m_clock, x->m_interval); 	// schedule another tick
	edubeat_bang(x); 								// poll the edubeat
} 

//--------------------------------------------------------------------------
// - Object creation and setup
//--------------------------------------------------------------------------
int edubeat_setup(void)
{
	edubeat_class = class_new ( gensym("edubeat"),(t_newmethod)edubeat_new, 0, sizeof(t_edubeat), 	CLASS_DEFAULT,0);
	
	// Add message handlers
	class_addbang(edubeat_class, (t_method)edubeat_bang);
	class_addfloat(edubeat_class, (t_method)edubeat_int);
	class_addmethod(edubeat_class, (t_method)edubeat_open, gensym("open"), 0);		
	class_addmethod(edubeat_class, (t_method)edubeat_close, gensym("close"), 0);	
	class_addmethod(edubeat_class, (t_method)edubeat_poll, gensym("poll"), A_DEFFLOAT,0);	
	class_addmethod(edubeat_class, (t_method)edubeat_start, gensym("start"), 0);	
	class_addmethod(edubeat_class, (t_method)edubeat_stop, gensym("stop"), 0);	
	post("edubeat version 0.1",0);
	return 1;
}

//--------------------------------------------------------------------------
void *edubeat_new(t_symbol *s)		// s = optional argument typed into object box (A_SYM) -- defaults to 0 if no args are typed
{
	t_edubeat *x;									// local variable (pointer to a t_edubeat data structure)
	x = (t_edubeat *)pd_new(edubeat_class);			 // create a new instance of this object
	x->m_clock = clock_new(x,(t_method)edubeat_tick); 	// make new clock for polling and attach gnsub_tick function to it
	
	x->m_interval = DEFAULT_CLOCK_INTERVAL;
	x->m_interval_bak = DEFAULT_CLOCK_INTERVAL;

	x->dev_handle = NULL;
	int i;
	
	// create outlets and assign it to our outlet variable in the instance's data structure
	for (i=0; i < OUTLETS; i++) {
		x->outlets[i] = outlet_new(&x->p_ob, &s_float);
	}	

	return x; // return a reference to the object instance 
}

//--------------------------------------------------------------------------
// - Object destruction
//--------------------------------------------------------------------------
void edubeat_free(t_edubeat *x)
{
	if (x->dev_handle) usb_close(x->dev_handle);
	freebytes((t_object *)x->m_clock, sizeof(x->m_clock));
}

//--------------------------------------------------------------------------
// - USB Utility Functions
//--------------------------------------------------------------------------
static int  usbGetStringAscii(usb_dev_handle *dev, int index, int langid, char *buf, int buflen)
{
char    buffer[256];
int     rval, i;

    if((rval = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, langid, buffer, sizeof(buffer), 1000)) < 0)
        return rval;
    if(buffer[1] != USB_DT_STRING)
        return 0;
    if((unsigned char)buffer[0] < rval)
        rval = (unsigned char)buffer[0];
    rval /= 2;
    /* lossy conversion to ISO Latin1 */
    for(i=1;i<rval;i++){
        if(i > buflen)  /* destination buffer overflow */
            break;
        buf[i-1] = buffer[2 * i];
        if(buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
            buf[i-1] = '?';
    }
    buf[i-1] = 0;
    return i-1;
}

//--------------------------------------------------------------------------
void find_device(t_edubeat *x) {
	usb_dev_handle      *handle = NULL;
	struct usb_bus      *bus;
	struct usb_device   *dev;
	
	usb_init();
	usb_find_busses();
    usb_find_devices();
	 for(bus=usb_busses; bus; bus=bus->next){
        for(dev=bus->devices; dev; dev=dev->next){
            if(dev->descriptor.idVendor == USBDEV_SHARED_VENDOR && dev->descriptor.idProduct == USBDEV_SHARED_PRODUCT){
                char    string[256];
                int     len;
                handle = usb_open(dev); /* we need to open the device in order to query strings */
                if(!handle){
                    error ("Warning: cannot open USB device: %s", usb_strerror());
                    continue;
                }
                /* now find out whether the device actually is edubeat */
                len = usbGetStringAscii(handle, dev->descriptor.iManufacturer, 0x0409, string, sizeof(string));
                if(len < 0){
                    post("Edubeat: warning: cannot query manufacturer for device: %s", usb_strerror());
                    goto skipDevice;
                }
                
                if(strcmp(string, "11h11") != 0)
                    goto skipDevice;
                len = usbGetStringAscii(handle, dev->descriptor.iProduct, 0x0409, string, sizeof(string));
                if(len < 0){
                    post("Edubeat: warning: cannot query product for device: %s", usb_strerror());
                    goto skipDevice;
                }
                if(strcmp(string, "Gac") == 0)
                    break;
				skipDevice:
                usb_close(handle);
                handle = NULL;
            }
        }
        if(handle)
            break;
    }
	
    if(!handle){
        post("Could not find USB device Edubeat");
		x->dev_handle = NULL;
		if (x->m_interval < 10000) x->m_interval *=2; // throttle polling down to max 20s if we can't find edubeat
	} else {
		x->dev_handle = handle;
		 post("Found USB device Edubeat");
		 x->m_interval = x->m_interval_bak;			// restore original polling interval
		 if (x->is_running) edubeat_tick(x);
		 else edubeat_bang(x);
	}
}
