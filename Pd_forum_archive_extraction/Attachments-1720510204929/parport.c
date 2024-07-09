/* 
 * parport external 1.0 
 * (C) 2002 by Miha Tom¹iè
 * released under GPL 
 */ 

#include <m_pd.h>
#include <linux/ppdev.h>
#include <linux/parport.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

// delay to read the parport in ms
#define DELAY 10

static t_class *parport_class;

typedef struct _parport {
	// fist entry in the struct must be of type t_object
	t_object	x_obj;  

	t_clock *	x_clock;
	t_outlet *	x_out[8];
	unsigned char	x_data;
	int		x_read;
	int		x_fd;
	int		x_ok;	
} t_parport; 

void parport_read(t_parport *x) {
	
	unsigned char data;
	unsigned char mask;
	int i;
	
	if (x->x_read && x->x_ok) {
		// read parport
		if (ioctl(x->x_fd, PPRDATA, &data))
			post("PPRDATA");
		data = ~data;
		mask = data ^ x->x_data;
		/* debug */
		// post("reading parallel port data %x, %x", data, mask);
		x->x_data = data;
		clock_delay(x->x_clock, DELAY);

	// send bits to outlets
	if (mask)
		for (i = 0; i < 8; i++) {
			if (mask & (1 << i))
				outlet_float(x->x_out[i], 
						(data & (1 << i)) ? 1: 0);
		} // for
	} // if

} // parport_read()

void *parport_new(void) {

	unsigned char ctrl;
	unsigned int mode;
	int i;
	
	t_parport *x = (t_parport *)pd_new(parport_class);

	x->x_clock = clock_new(x, (t_method)parport_read);
	
	x->x_data = 0;
	x->x_read = 0;
	x->x_ok = 1;
	
	// parport init
	ctrl = 0x3f;
	mode = IEEE1284_MODE_EPP;
	if (-1 == (x->x_fd = open("/dev/parport0", O_RDWR))) {
		post("Error opening /dev/parport0: %s\n", strerror(errno));
		x->x_ok = 0;
	} // if
	else if(-1 == (flock(x->x_fd, LOCK_EX|LOCK_NB))) {
		post("Error locking /dev/parport0: %s\n", strerror(errno));
		x->x_ok = 0;
	} // else if
	else if (ioctl(x->x_fd, PPCLAIM)) {
		post("PPCLAIM");
		x->x_ok = 0;
	} // else if
	else if (ioctl(x->x_fd, PPWCONTROL, &ctrl)) {
		post("PPWCONTROL");
		x->x_ok = 0;
	} // else if
	else if (ioctl(x->x_fd, PPSETMODE, &mode)) {
		post("PPGETMODE");
		x->x_ok = 0;
	} // else if
	
	if(x->x_ok) {
		for (i = 0; i < 8; i++)
			x->x_out[i] = outlet_new(&x->x_obj, &s_float);
		
		post("New parport object created."); 
		return (void *)x;
	} // if
	else
		return (void *)NULL;
} // parport_new()

void parport_free(t_parport *x) {
	unsigned int mode;
	
	clock_free(x->x_clock);
	
	// parport clean-up
	mode = IEEE1284_MODE_COMPAT;
	if (!x->x_ok)
		post("parport not OK");
	else if (ioctl(x->x_fd, PPSETMODE, &mode)) {
		post("PPGETMODE");
		x->x_ok = 0;
	} // if
	else if (ioctl(x->x_fd, PPRELEASE)) {
		post("PPRELEASE");
		x->x_ok = 0;
	} // else if

	if (-1 != x->x_fd)
		close(x->x_fd);
	
	post("parport object deleted.");

} // parport_free()

void parport_bang(t_parport *x) {
	post("We've been banged !!");
/*
	if (x->x_read) {
		x->x_read = 0;
		clock_unset(x->x_clock);
	} // if
	else {
		x->x_read = 1;
		clock_delay(x->x_clock, DELAY);
	} // else
*/
} // parport_bang()

void parport_float(t_parport *x, t_float f) {
/*
	post("We've been floated %f!!", f);
*/
	if (1 == f) {
		x->x_read = 1;	
		parport_read(x);
	} // if
	else if (0 == f)
		x->x_read = 0;
} // parport_float()

void parport_setup(void) {
	parport_class = class_new(gensym("parport"),
			(t_newmethod)parport_new,
			(t_method)parport_free,
			sizeof(t_parport),
			CLASS_DEFAULT,0);

	class_addbang(parport_class, parport_bang);
	class_addfloat(parport_class, parport_float);
	
	post("parport external 1.0 loaded");
} // parport_setup()


