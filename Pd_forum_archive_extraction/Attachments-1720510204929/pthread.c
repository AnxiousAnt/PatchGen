#include "m_pd.h"
#include <usb.h> //http://libusb-win32.sourceforge.net
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pthread.h"

// ==============================================================================
// Constants
// ------------------------------------------------------------------------------
#define USBDEV_SHARED_VENDOR    	0x16c0  /* VOTI */
#define USBDEV_SHARED_PRODUCT   	0x05dc  /* Obdev's free shared PID */
#define OUTLETS 					17
#define DEFAULT_CLOCK_INTERVAL		4000 //(usleep)
#define USBREPLYBUFFER 				21

// ==============================================================================
// Our External's Memory structure
// ------------------------------------------------------------------------------

typedef struct _gac				// defines our object's internal variables for each instance in a patch
{
	t_object 		p_ob;					// object header - ALL pd external MUST begin with this...
	usb_dev_handle	*dev_handle;			// handle to the gac usb device
	int				do_10_bit;				// output analog values with 8bit or 10bit resolution?
	void 			*outlets[OUTLETS];		// handle to the objects outlets
	int 			values[10];				// stored values from last poll /////////////////////////////////
	int			x_verbose;
	pthread_attr_t 	gac_thread_attr;
	pthread_t    	 x_threadid;
} t_gac;

void *gac_class;					// global pointer to the object class - so pd can reference the object 

// ==============================================================================
// Function Prototypes
// ------------------------------------------------------------------------------
void *gac_new(t_symbol *s);
void gac_assist(t_gac *x, void *b, long m, long a, char *s);
void gac_bang(t_gac *x);				

// functions used to find the USB device
static int  	usbGetStringAscii(usb_dev_handle *dev, int ndex, int langid, char *buf, int buflen);
void 			find_device(t_gac *x);

// =============================================================================
// Thread
// =============================================================================
static void *usb_thread_read(void *w)
{
	t_gac *x = (t_gac*) w;
	while(1) {
		pthread_testcancel();
		gac_bang(x);
		usleep(DEFAULT_CLOCK_INTERVAL);	
	}
}

static void usb_thread_start(t_gac *x)
{

	// create the worker thread
    if(pthread_attr_init(&x->gac_thread_attr) < 0)
	{
       error("gac: could not launch receive thread");
       return;
    }
    if(pthread_attr_setdetachstate(&x->gac_thread_attr, PTHREAD_CREATE_DETACHED) < 0)
	{
       error("gac: could not launch receive thread");
       return;
    }
    if(pthread_create(&x->x_threadid, &x->gac_thread_attr, usb_thread_read, x) < 0)
	{
       error("gac: could not launch receive thread");
       return;
    }
    else
    {
       if(x->x_verbose)post("gac: thread %d launched", (int)x->x_threadid );
    }
}

//--------------------------------------------------------------------------
// - Message: bang  -> poll gac
//--------------------------------------------------------------------------
void gac_bang(t_gac *x)	// poll gac
{
	int                 nBytes,i,n;
	int 				replymask,replyshift,replybyte;
	int					temp;
	unsigned char       buffer[USBREPLYBUFFER];
	
	if (!(x->dev_handle)) find_device(x);
	else {
			// ask gac to send us data
			nBytes = usb_control_msg(x->dev_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
										EDUBEAT_CMD_POLL, 0, 0, (char *)buffer, sizeof(buffer), 10);

			for (i = 0; i < OUTLETS; i++) {
			
				outlet_float(x->outlets[i], HEREYOURVALUE);

			}
			
	}
}


//--------------------------------------------------------------------------
// - Object creation and setup
//--------------------------------------------------------------------------
int gac_setup(void)
{
	gac_class = class_new ( gensym("gac"),(t_newmethod)gac_new, 0, sizeof(t_gac), 	CLASS_DEFAULT,0);
	
	// Add message handlers
	class_addbang(gac_class, (t_method)gac_bang);
	post("gac version 0.1",0);
	return 1;
}

//--------------------------------------------------------------------------
void *gac_new(t_symbol *s)		// s = optional argument typed into object box (A_SYM) -- defaults to 0 if no args are typed
{
	t_gac *x;									// local variable (pointer to a t_gac data structure)
	x = (t_gac *)pd_new(gac_class);			 // create a new instance of this object

    x->x_verbose = 1;

	//default to 10bit
	x->do_10_bit = 1;

	x->dev_handle = NULL;
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
void gac_free(t_gac *x)
{
	if (x->dev_handle) usb_close(x->dev_handle);

		while(pthread_cancel(x->x_threadid) < 0)
			if(x->x_verbose)post("gac: killing thread\n");
		if(x->x_verbose)post("gac: thread canceled\n");
    		
}

//--------------------------------------------------------------------------
// - USB Utility Functions
//--------------------------------------------------------------------------
static int  usbGetStringAscii(usb_dev_handle *dev, int ndex, int langid, char *buf, int buflen)
{
char    buffer[256];
int     rval, i;

    if((rval = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + ndex, langid, buffer, sizeof(buffer), 1000)) < 0)
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
void find_device(t_gac *x) {
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
                /* now find out whether the device actually is gac */
                len = usbGetStringAscii(handle, dev->descriptor.iManufacturer, 0x0409, string, sizeof(string));
                if(len < 0){
                    post("gac: warning: cannot query manufacturer for device: %s", usb_strerror());
                    goto skipDevice;
                }
                
                if(strcmp(string, "11h11") != 0)
                    goto skipDevice;
                len = usbGetStringAscii(handle, dev->descriptor.iProduct, 0x0409, string, sizeof(string));
                if(len < 0){
                    post("gac: warning: cannot query product for device: %s", usb_strerror());
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
        post("Could not find USB device 11h11/gac");
		x->dev_handle = NULL;
	} else {
		 x->dev_handle = handle;
		 post("Found USB device 11h11/gac");
	}
}
