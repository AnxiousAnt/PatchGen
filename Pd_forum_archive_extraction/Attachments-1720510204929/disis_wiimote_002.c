// ===================================================================
// Wiimote external for Puredata
// Written by Mike Wozniewki (Feb 2007), www.mikewoz.com
//
// Requires the CWiid library (version 0.6.00) by L. Donnie Smith
//
// ===================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
// ===================================================================

//  ChangeLog:
//  2008-04-14 Florian Krebs 
//  * adapt wiimote external for the actual version of cwiid (0.6.00)

//  ChangeLog:
//  2009-06-09 DISIS (Michael Hawthorne <rustmik2@gmail.com> & Ivica Ico Bukvic <ico@vt.edu>)
//  http://disis.music.vt.edu
//  * Bug-fixes (connecting and disconnecting crashes)
//  * Multithreaded implementation to prevent wiimote from starving PD audio thread
//  * Bang implementation to allow for better data rate control
//  * Updated help file

//  Changelog:
//  2009-10-05 DISIS (Ivica Ico Bukvic <ico@vt.edu>)
//  http://disis.music.vt.edu
//  * Additional bug-fixes, code clean-up, and improvements

#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <bluetooth/bluetooth.h>
#include <m_pd.h>
#include <math.h>
#include <pthread.h>
#include "cwiid_internal.h"
#define PI	3.14159265358979323

#define DARWIN_CALIB

struct acc {
	unsigned char x;
	unsigned char y;
	unsigned char z;
};

/* Wiimote Callback */
cwiid_mesg_callback_t cwiid_callback;

// class and struct declarations for wiimote pd external:
static t_class *cwiid_class;
typedef struct _wiimote
{
	t_object x_obj; // standard pd object (must be first in struct)
	
	cwiid_wiimote_t *wiimote; // individual wiimote handle per pd object, represented in libcwiid

	t_float connected;
	int wiimoteID;
	int extensionAttached;

	int mode; //0 = use metro for timer (default), 1 = send as fast as possible

	//Creating separate threads for actions known to cause sample drop-outs
	t_float rumble;
	pthread_t rumble_t;

	t_float led;
	pthread_t led_t;

	t_float connection;
	pthread_t connection_t;
	t_symbol *addr;

	t_float rpt;
	unsigned char rpt_mode;
	pthread_t rpt_t;

	t_float toggle_acc, toggle_ir, toggle_nc;

	struct acc acc_zero, acc_one; // acceleration
	struct acc nc_acc_zero, nc_acc_one; // nunchuck acceleration

	// We store atom list for each data type so we don't waste time
	// allocating memory at every callback:
	t_atom btn_atoms[2];
	t_atom acc_atoms[3];
	t_atom ir_atoms[4];
	t_atom nc_btn_atoms[1];
	t_atom nc_acc_atoms[3];
	t_atom nc_stick_atoms[2];
	
	// outlets:
	t_outlet *outlet_btn;
	t_outlet *outlet_acc;
	t_outlet *outlet_ir;
	t_outlet *outlet_nc_btn;
	t_outlet *outlet_nc_acc;
	t_outlet *outlet_nc_stick;
	t_outlet *outlet_connected;
	
} t_wiimote;

// For now, we make one global t_wiimote pointer that we can refer to
// in the cwiid_callback. This means we can support maximum of ONE
// wiimote. ARGH. We'll have to figure out how to have access to the
// pd object from the callback (without modifying the CWiid code):
#define MAX_WIIMOTES 16
t_wiimote *g_wiimoteList[MAX_WIIMOTES];

// Structure to pass generic parameters into a threaded function.
// Added by VT DISIS
typedef struct
{
	t_wiimote *wiimote;
	t_floatarg f;
} threadedFunctionParams;

// ==============================================================

void cwiid_debug(t_wiimote *x)
{
	post("\n======================");
	if (x->connected) post("Wiimote (id: %d) is connected.", x->wiimoteID);
	else post("Wiimote (id: %d) is NOT connected.", x->wiimoteID);
	if (x->toggle_acc) post("acceleration: ON");
	else post("acceleration: OFF");
	if (x->toggle_ir)  post("IR:           ON");
	else post("IR:           OFF");
	if (x->toggle_nc)  post("Nunchuck:     ON");
	else post("Nunchuck:     OFF");
	post("");
	post("Accelerometer calibration: zero=(%d,%d,%d) one=(%d,%d,%d)",x->acc_zero.x,x->acc_zero.y,x->acc_zero.z,x->acc_one.x,x->acc_one.y,x->acc_one.z);
	post("Nunchuck calibration:      zero=(%d,%d,%d) one=(%d,%d,%d)",x->nc_acc_zero.x,x->nc_acc_zero.y,x->nc_acc_zero.z,x->nc_acc_one.x,x->nc_acc_one.y,x->nc_acc_one.z);
}

// ==============================================================

// Button handler:
void cwiid_btn(t_wiimote *x, struct cwiid_btn_mesg *mesg)
{
	//post("Buttons: %X %X", (mesg->buttons & 0xFF00)>>8, data->btn_data.buttons & 0x00FF);
	SETFLOAT(x->btn_atoms+0, (mesg->buttons & 0xFF00)>>8);
	SETFLOAT(x->btn_atoms+1, mesg->buttons & 0x00FF);
	outlet_anything(x->outlet_btn, &s_list, 2, x->btn_atoms);
/*
	if (mesg->buttons & CWIID_BTN_UP) {}
	if (mesg->buttons & CWIID_BTN_DOWN) {}
	if (mesg->buttons & CWIID_BTN_LEFT) {}
	if (mesg->buttons & CWIID_BTN_RIGHT) {}
	if (mesg->buttons & CWIID_BTN_A) {}
	if (mesg->buttons & CWIID_BTN_B) {}
	if (mesg->buttons & CWIID_BTN_MINUS) {}
	if (mesg->buttons & CWIID_BTN_PLUS) {}
	if (mesg->buttons & CWIID_BTN_HOME) {}
	if (mesg->buttons & CWIID_BTN_1) {}
	if (mesg->buttons & CWIID_BTN_2) {}
*/
}

// Records acceleration into wiimote object. 
// To retrieve the information in pd, send a bang to input or change output mode to 1
void cwiid_acc(t_wiimote *x, struct cwiid_acc_mesg *mesg)
{
	if (x->toggle_acc)
	{
		double a_x, a_y, a_z;
		
		a_x = ((double)mesg->acc[CWIID_X] - x->acc_zero.x) / (x->acc_one.x - x->acc_zero.x);
		a_y = ((double)mesg->acc[CWIID_Y] - x->acc_zero.y) / (x->acc_one.y - x->acc_zero.y);
		a_z = ((double)mesg->acc[CWIID_Z] - x->acc_zero.z) / (x->acc_one.z - x->acc_zero.z);

		/*
		double a, roll, pitch;
		a = sqrt(pow(a_x,2)+pow(a_y,2)+pow(a_z,2));
		roll = atan(a_x/a_z);
		if (a_z <= 0.0) roll += PI * ((a_x > 0.0) ? 1 : -1);
		roll *= -1;
		pitch = atan(a_y/a_z*cos(roll));
		*/
		
		SETFLOAT(x->acc_atoms+0, a_x);
		SETFLOAT(x->acc_atoms+1, a_y);
		SETFLOAT(x->acc_atoms+2, a_z);
	}
}

void cwiid_ir(t_wiimote *x, struct cwiid_ir_mesg *mesg)
{
	unsigned int i;

	if (x->toggle_ir)
	{
		//post("IR (valid,x,y,size) #%d: %d %d %d %d", i, data->ir_data.ir_src[i].valid, data->ir_data.ir_src[i].x, data->ir_data.ir_src[i].y, data->ir_data.ir_src[i].size);
		for (i=0; i<CWIID_IR_SRC_COUNT; i++)
		{		
			if (mesg->src[i].valid)
			{
				SETFLOAT(x->ir_atoms+0, i);
				SETFLOAT(x->ir_atoms+1, mesg->src[i].pos[CWIID_X]);
				SETFLOAT(x->ir_atoms+2, mesg->src[i].pos[CWIID_Y]);
				SETFLOAT(x->ir_atoms+3, mesg->src[i].size);
			}
		}
	}
}

void cwiid_nunchuk(t_wiimote *x, struct cwiid_nunchuk_mesg *mesg)
{
		double a_x, a_y, a_z;

		a_x = ((double)mesg->acc[CWIID_X] - x->nc_acc_zero.x) / (x->nc_acc_one.x - x->nc_acc_zero.x);
		a_y = ((double)mesg->acc[CWIID_Y] - x->nc_acc_zero.y) / (x->nc_acc_one.y - x->nc_acc_zero.y);
		a_z = ((double)mesg->acc[CWIID_Z] - x->nc_acc_zero.z) / (x->nc_acc_one.z - x->nc_acc_zero.z);

		/*
		double a, roll, pitch;
		a = sqrt(pow(a_x,2)+pow(a_y,2)+pow(a_z,2));
		roll = atan(a_x/a_z);
		if (a_z <= 0.0) roll += PI * ((a_x > 0.0) ? 1 : -1);
		roll *= -1;
		pitch = atan(a_y/a_z*cos(roll));
		*/
		
		//if (mesg->buttons & CWIID_NUNCHUK_BTN_C) {}
		if (atom_getint(x->nc_btn_atoms) != mesg->buttons) {
			SETFLOAT(x->nc_btn_atoms+0, mesg->buttons);
			outlet_float(x->outlet_nc_btn, mesg->buttons);
		}
		
		SETFLOAT(x->nc_acc_atoms+0, a_x);
		SETFLOAT(x->nc_acc_atoms+1, a_y);
		SETFLOAT(x->nc_acc_atoms+2, a_z);	

		SETFLOAT(x->nc_stick_atoms+0, mesg->stick[CWIID_X]);
		SETFLOAT(x->nc_stick_atoms+1, mesg->stick[CWIID_Y]);
		//outlet_anything(x->outlet_nc_stick, &s_list, 2, x->nc_stick_atoms);		
}

void cwiid_doBang(t_wiimote *x)
{
	if (x->toggle_nc == 1 && x->extensionAttached == 1) {
		outlet_anything(x->outlet_nc_stick, &s_list, 2, x->nc_stick_atoms);
		outlet_anything(x->outlet_nc_acc, &s_list, 3, x->nc_acc_atoms);
	}
	if (x->toggle_ir == 1) outlet_anything(x->outlet_ir, &s_list, 4, x->ir_atoms);
	if (x->toggle_acc == 1) outlet_anything(x->outlet_acc, &s_list, 3, x->acc_atoms);
}

void cwiid_pthread_setRumble(void *ptr)
{
	threadedFunctionParams *rPars = (threadedFunctionParams*)ptr;
	t_wiimote *x = rPars->wiimote;
	t_floatarg f = rPars->f;

	while(x->rumble > -1) {
		if (f != x->rumble) {
			f = x->rumble;
			if (cwiid_command(x->wiimote, CWIID_CMD_RUMBLE, f)) {
				//post("wiiremote error: problem setting rumble.");
			}
		} else {
			usleep(10000);
		}
	}
	pthread_exit(0);
}

void cwiid_setRumble(t_wiimote *x, t_floatarg f)
{
	if (x->connected) {
		x->rumble = f;
	}
}

// The CWiid library invokes a callback function whenever events are
// generated by the wiimote. This function is specified when connecting
// to the wiimote (in the cwiid_open function).

// Unfortunately, the mesg struct passed as an argument to the
// callback does not have a pointer to the wiimote instance, and it
// is thus impossible to know which wiimote has invoked the callback.
// For this case we provide a hard-coded set of wrapper callbacks to
// indicate which Pd wiimote instance to control.

// So far I have only checked with one wiimote

void cwiid_callback(cwiid_wiimote_t *wiimote, int mesg_count,
                    union cwiid_mesg mesg_array[], struct timespec *timestamp)
{
	int i;
	t_wiimote *x;
	if (g_wiimoteList[wiimote->id] == NULL) {
		post("no wiimote loaded: %d%",wiimote->id);
	}
	else {	
		x = g_wiimoteList[wiimote->id];
			
		for (i=0; i < mesg_count; i++)
		{	
			switch (mesg_array[i].type) {
				case CWIID_MESG_STATUS:
					post("Battery: %d%", (int) (100.0 * mesg_array[i].status_mesg.battery / CWIID_BATTERY_MAX));
					switch (mesg_array[i].status_mesg.ext_type) {
						case CWIID_EXT_NONE:
							post("No nunchuck attached");
							x->extensionAttached = 0;
							break;
						case CWIID_EXT_NUNCHUK:
							post("Nunchuck extension attached");
							x->extensionAttached = 1;
#ifdef DARWIN_CALIB
							x->nc_acc_zero.x = 128;
							x->nc_acc_zero.y = 129;
							x->nc_acc_zero.z = 128;
							x->nc_acc_one.x  = 153;
							x->nc_acc_one.y  = 154;
							x->nc_acc_one.z  = 154;
#else
							if (cwiid_read(x->wiimote, CWIID_RW_REG | CWIID_RW_DECODE, 0xA40020, 								7, buf)) {
								post("Unable to retrieve Nunchuk calibration");
							}
							else {
								x->nc_acc_zero.x = buf[0];
								x->nc_acc_zero.y = buf[1];
								x->nc_acc_zero.z = buf[2];
								x->nc_acc_one.x  = buf[4];
								x->nc_acc_one.y  = buf[5];
								x->nc_acc_one.z  = buf[6];
							}	
#endif
							break;
						case CWIID_EXT_CLASSIC:
							post("Classic controller attached. There is no support for this yet.");
							break;
						case CWIID_EXT_UNKNOWN:
							post("Unknown extension attached");
							break;
					}
					break;
				case CWIID_MESG_BTN:
					cwiid_btn(x, &mesg_array[i].btn_mesg);
					break;
				case CWIID_MESG_ACC:
					cwiid_acc(x, &mesg_array[i].acc_mesg);
					break;
				case CWIID_MESG_IR:
					cwiid_ir(x, &mesg_array[i].ir_mesg);
					break;
				case CWIID_MESG_NUNCHUK:
					cwiid_nunchuk(x, &mesg_array[i].nunchuk_mesg);
					break;
				case CWIID_MESG_CLASSIC:
					// todo
					break;
				default:
					break;
			}
		}
		if (x->mode == 1) {
			cwiid_doBang(x);
		}
	}
}

// ==============================================================

void cwiid_pthread_setReportMode(void *ptr)
{
	threadedFunctionParams *sPars = (threadedFunctionParams*)ptr;
	t_wiimote *x = sPars->wiimote;
	t_floatarg loc_rpt = 0;
	unsigned char loc_rpt_mode;

	while (x->rpt > -1) {
		if (loc_rpt != x->rpt) {
			loc_rpt = x->rpt;
			loc_rpt_mode = CWIID_RPT_STATUS | CWIID_RPT_BTN;
			if (x->toggle_ir) loc_rpt_mode |= CWIID_RPT_IR;
			if (x->toggle_acc) loc_rpt_mode |= CWIID_RPT_ACC;
			if (x->toggle_nc) loc_rpt_mode |= CWIID_RPT_EXT;

			if (cwiid_command(x->wiimote, CWIID_CMD_RPT_MODE, loc_rpt_mode)) { 
				//post("wiimote error: problem setting report mode.");
			}
		}
		else {
			usleep(10000);
		}
	}
	pthread_exit(0);
}

void cwiid_setReportMode(t_wiimote *x, t_floatarg r)
{
	if (x->connected || x->connection == 3) {
		if (r >= 0) x->rpt_mode = (unsigned char) r;
		else {
			x->rpt_mode = CWIID_RPT_STATUS | CWIID_RPT_BTN;
			if (x->toggle_ir) x->rpt_mode |= CWIID_RPT_IR;
			if (x->toggle_acc) x->rpt_mode |= CWIID_RPT_ACC;
			if (x->toggle_nc) x->rpt_mode |= CWIID_RPT_EXT;
		}
		x->rpt += 1;
		if (x->rpt > 1) x->rpt = 0;
	}
}

void cwiid_setMode(t_wiimote *x, t_floatarg f)
{
	if (f == 0) {
		x->mode = 0;
		post("setting metro output mode (default)");
	}
	else if (f == 1) {
		x->mode = 1;
		post("setting maximum output rate mode");
	}
}

void cwiid_reportAcceleration(t_wiimote *x, t_floatarg f)
{
	x->toggle_acc = f;
	cwiid_setReportMode(x, -1);
}

void cwiid_reportIR(t_wiimote *x, t_floatarg f)
{
	x->toggle_ir = f;
	cwiid_setReportMode(x, -1);
}

void cwiid_reportNunchuck(t_wiimote *x, t_floatarg f)
{
	x->toggle_nc = f;
	cwiid_setReportMode(x, -1);
}

void cwiid_pthread_setLED(void *ptr)
{
	threadedFunctionParams *lPars = (threadedFunctionParams*)ptr;
	t_wiimote *x = lPars->wiimote;
	t_floatarg f = lPars->f;

	while(x->led > -1) {
		if (f != x->led) {
			f = x->led;
			// some possible values:
			// CWIID_LED0_ON		0x01
			// CWIID_LED1_ON		0x02
			// CWIID_LED2_ON		0x04
			// CWIID_LED3_ON		0x08
			if (cwiid_command(x->wiimote, CWIID_CMD_LED, f)) {
				//post("wiiremote error: problem setting LED.");
			}
		} else {
			usleep(10000);
		}
	}
	pthread_exit(0);
}

void cwiid_setLED(t_wiimote *x, t_floatarg f)
{
	if (x->connected) {
		x->led = f;
	}
}

// The following function attempts to connect to a wiimote at a
// specific address, provided as an argument. eg, 00:19:1D:70:CE:72
// This address can be discovered by running the following command
// in a console:
//   hcitool scan | grep Nintendo

void cwiid_manageConnection(void *ptr)
{
	threadedFunctionParams *rPars = (threadedFunctionParams*)ptr;
	t_wiimote *x = rPars->wiimote;
	t_floatarg f = rPars->f;

	// -1 = free
	//  0 = disconnected
	//  1 = disconnecting
	//  2 = connected
	//  3 = connecting
	while(x->connection > -1) {
		if (f != x->connection) {
			if (x->connection == 1) {
				f = x->connection;
				if (cwiid_close(x->wiimote)) {
					//post("wiimote error: problems when disconnecting.");
				} 
				else {
					//post("disconnect successfull, resetting values");
					g_wiimoteList[x->wiimoteID] = NULL;
					x->connected = 0;
					outlet_float(x->outlet_connected, x->connected);

					//we have disconnected so make connection 0 and sync thread's f
					x->connection = 0;
					f = x->connection;
				}
			}
			else if (x->connection == 3) {
				f = x->connection;
				int i;
				bdaddr_t bdaddr;
				// determine address:
				if (x->addr==gensym("NULL")) {
					//post("Searching automatically...");		
					bdaddr = *BDADDR_ANY;
				}
				else {
					str2ba(x->addr->s_name, &bdaddr);
					//post("Connecting to given address...");
					//post("Press buttons 1 and 2 simultaneously.");
					} 
				// bdaddr = *BDADDR_ANY;
				// connect:
				for (i=0;i<MAX_WIIMOTES;++i) {
					if (g_wiimoteList[i]==NULL) {
						//post("open: Connect wiimote %d",i);		
						x->wiimote = cwiid_open(&bdaddr,CWIID_FLAG_MESG_IFC);
						x->wiimoteID = i;
						if (x->wiimote) {
							 x->wiimote->id = i;
							g_wiimoteList[i] = x;
						}		
						break;
					}
				}

				if (x->wiimote == NULL) {
					//post("Error: could not find and/or connect to a wiimote. Please ensure that bluetooth is enabled, and that the 'hcitool scan' command lists your Nintendo device.");

					//we have failed to connect so make connection 2 and sync thread's f
					x->connection = 0;
					f = x->connection;
				} else {
					//post("wiimote has successfully connected");
			#ifdef DARWIN_CALIB
					x->acc_zero.x = 128;
					x->acc_zero.y = 129;
					x->acc_zero.z = 128;
					x->acc_one.x  = 153;
					x->acc_one.y  = 154;
					x->acc_one.z  = 154;
			#else
					if (cwiid_read(x->wiimote, CWIID_RW_EEPROM, 0x16, 7, buf)) {
						//post("Unable to retrieve accelerometer calibration");
					} else {
						x->acc_zero.x = buf[0];
						x->acc_zero.y = buf[1];
						x->acc_zero.z = buf[2];
						x->acc_one.x  = buf[4];
						x->acc_one.y  = buf[5];
						x->acc_one.z  = buf[6];
						//post("Retrieved wiimote calibration: zero=(%.1f,%.1f,%.1f) one=(%.1f,%.1f,%.1f)",buf[0],buf[2],buf[3],buf[4],buf[5],buf[6]);
					}
			#endif
					x->connected = 1;
					cwiid_setReportMode(x,-1);
					if (cwiid_set_mesg_callback(x->wiimote, &cwiid_callback)) {
						//fprintf(stderr, "Unable to set message callback\n");
					}

					outlet_float(x->outlet_connected, x->connected);

					// send brief rumble to acknowledge connect
					// and give a bit of a wait before doing so
					usleep(250000);
					cwiid_setRumble(x, 1);
					usleep(250000);
					cwiid_setRumble(x, 0);

					//we have connected so make connection 2 and sync thread's f
					x->connection = 2;
					f = x->connection;
				}
			}
		} else {
			usleep(10000);
		}
	}
	pthread_exit(0);
}

void cwiid_doConnect(t_wiimote *x, t_symbol *addr)
{
	if (!x->connected && x->connection != 3) {
		x->addr = addr;
		x->connection = 3;
	}
}

// The following function attempts to discover a wiimote. It requires
// that the user puts the wiimote into 'discoverable' mode before being
// called. This is done by pressing the red button under the battery
// cover, or by pressing buttons 1 and 2 simultaneously.

void cwiid_discover(t_wiimote *x)
{
	if (!x->connected && x->connection != 3) {
		post("Put the wiimote into discover mode by pressing buttons 1 and 2 simultaneously.");	
		cwiid_doConnect(x, gensym("NULL"));
	}
	else {
		post("connect: device already connected!");
	}
}

void cwiid_doDisconnect(t_wiimote *x)
{
	if (x->connected && x->connection != 1)
	{
		x->connection = 1;
	}
	else post("disconnect: device is not connected!");
}

// ==============================================================
// ==============================================================

static void *cwiid_new(t_symbol* s, int argc, t_atom *argv)
{
	post( "DISIS threaded implementation of wiimote object v.0.6.3");
	//bdaddr_t bdaddr; // wiimote bdaddr
	t_wiimote *x = (t_wiimote *)pd_new(cwiid_class);
	
	// create outlets:
	x->outlet_btn = outlet_new(&x->x_obj, &s_list);
	x->outlet_acc = outlet_new(&x->x_obj, &s_list);
	x->outlet_ir = outlet_new(&x->x_obj, &s_list);
	x->outlet_nc_btn = outlet_new(&x->x_obj, &s_float);
	x->outlet_nc_acc = outlet_new(&x->x_obj, &s_list);
	x->outlet_nc_stick = outlet_new(&x->x_obj, &s_list);

	// status outlet:
	x->outlet_connected = outlet_new(&x->x_obj, &s_float);

	// initialize toggles:
	x->toggle_acc = 0;
	x->toggle_ir = 0;
	x->toggle_nc = 0;

	// initialize values:
	SETFLOAT(x->acc_atoms+0, 0);
	SETFLOAT(x->acc_atoms+1, 0);
	SETFLOAT(x->acc_atoms+2, 0);
	SETFLOAT(x->ir_atoms+0, 0);
	SETFLOAT(x->ir_atoms+1, 0);
	SETFLOAT(x->ir_atoms+2, 0);
	SETFLOAT(x->ir_atoms+3, 0);
	SETFLOAT(x->nc_acc_atoms+0, 0);
	SETFLOAT(x->nc_acc_atoms+1, 0);
	SETFLOAT(x->nc_acc_atoms+2, 0);

	x->connected = 0;
	x->wiimoteID = -1;
	x->extensionAttached = 0;

	x->rumble = 0;
	x->led = 0;
	x->connection = 0; // -1 = free, 0 = disconnected, 1 = disconnecting, 2 = connected, 3 = connecting
	x->addr = gensym("NULL");
	x->rpt = 0;
	x->rpt_mode = -1;

	// spawn threads for actions known to cause sample drop-outs
	threadedFunctionParams rPars;
	rPars.wiimote = x;
	rPars.f = 0;
	pthread_create( &x->rpt_t, NULL, (void *) &cwiid_pthread_setReportMode, (void *) &rPars);
	pthread_create( &x->rumble_t, NULL, (void *) &cwiid_pthread_setRumble, (void *) &rPars);
	pthread_create( &x->led_t, NULL, (void *) &cwiid_pthread_setLED, (void *) &rPars);
	pthread_create( &x->connection_t, NULL, (void *) &cwiid_manageConnection, (void *) &rPars);

	post("setting metro output mode (default)");
	x->mode = 0;
	
	// connect if user provided an address as an argument:

	if (argc==2)
	{
		post("conecting to provided address...");
		if (argv->a_type == A_SYMBOL)
		{
			cwiid_doConnect(x, atom_getsymbol(argv));
		} else {
			error("[wiimote] expects either no argument, or a bluetooth address as an argument. eg, 00:19:1D:70:CE:72");
			return NULL;
		}
	}
	return (x);
}


static void cwiid_free(t_wiimote* x)
{
	if (x->connected) {
		cwiid_doDisconnect(x);
		usleep(1000000); //let the thread do its thing
	}

	x->rumble = -1;
	x->led = -1;
	x->connection = -1;
	x->rpt = -1;

	pthread_join(x->rpt_t, NULL);
	pthread_join(x->rumble_t, NULL);
	pthread_join(x->led_t, NULL);
	pthread_join(x->connection_t, NULL);
}

void disis_wiimote_setup(void)
{
	int i;
	for (i=0; i<MAX_WIIMOTES; i++) g_wiimoteList[i] = NULL;
	
	cwiid_class = class_new(gensym("disis_wiimote"), (t_newmethod)cwiid_new, (t_method)cwiid_free, sizeof(t_wiimote), CLASS_DEFAULT, A_GIMME, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_debug, gensym("debug"), 0);
	class_addmethod(cwiid_class, (t_method) cwiid_doConnect, gensym("connect"), A_SYMBOL, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_doDisconnect, gensym("disconnect"), 0);
	class_addmethod(cwiid_class, (t_method) cwiid_discover, gensym("discover"), 0);
	class_addmethod(cwiid_class, (t_method) cwiid_setReportMode, gensym("setReportMode"), A_DEFFLOAT, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_reportAcceleration, gensym("reportAcceleration"), A_DEFFLOAT, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_reportNunchuck, gensym("reportNunchuck"), A_DEFFLOAT, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_reportIR, gensym("reportIR"), A_DEFFLOAT, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_setRumble, gensym("setRumble"), A_DEFFLOAT, 0);
	class_addmethod(cwiid_class, (t_method) cwiid_setLED, gensym("setLED"), A_DEFFLOAT, 0);
	class_addbang(cwiid_class, cwiid_doBang);
	class_addmethod(cwiid_class, (t_method) cwiid_setMode, gensym("mode"), A_DEFFLOAT, 0);
}
