#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
#endif

#define STRICT
#define DIRECTINPUT_VERSION 0x0800

#include <windows.h>
#include <basetsd.h>
#include <dinput.h>

#include "m_pd.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
// DirectInput PD external (c) 2002 Bert Schiettecatte [bschiett@ccrma.stanford.edu]
//
// This external gives access to DirectInput on Windows machines, part of DirectX, and used to access
// force-feedback compatible gaming devices.
//
// A metronome should be connected to the bang input for polling buttons and axis data.
// The first argument of the object is the direcinput device number, in the list of all available
// connected devices. Next, an integer argument list follows for the object corresponding to
// button numbers of the input device. An outlet will be created for every button in the list.
//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Types, defines, prototypes
//////////////////////////////////////////////////////////////////////////////////////////////////////

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

static t_class* dinput_class;

typedef struct _dinput {
  t_object  x_obj;

	t_int m_devid;	// device number (index in list of available devices)
	t_int m_nrdevs; // number of devices available
	t_int m_currdev; // device counter for enumeration

	t_int m_nrbuttons; // nr of buttons found on device (check through caps)
	t_int* m_buttons; // array of button numbers passed to object
	t_int* m_newstate; // state for buttons

	t_outlet** m_outlets; // outlets for object

	t_float m_value; // just for holding temp list values

	LPDIRECTINPUT8A m_di;	// directinput interface
	LPDIRECTINPUTDEVICE8A m_dev; // device this object gives access to
} t_dinput;

static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
static BOOL CALLBACK EnumCountJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);

static HRESULT InitDirectInput(t_dinput*);
static HRESULT UpdateInputState(t_dinput*);
static void FreeDirectInput(t_dinput*);

//////////////////////////////////////////////////////////////////////////////////////////////////////
// DirectInput functions
//////////////////////////////////////////////////////////////////////////////////////////////////////

static BOOL CALLBACK EnumCountJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
                                     VOID* pContext )
{
  HRESULT hr;
	LPDIRECTINPUTDEVICE8A pad;
	t_dinput* x = (t_dinput*) pContext;

  // Obtain an interface to the enumerated joystick.
  hr = x->m_di->CreateDevice( pdidInstance->guidInstance, &pad, NULL );

  // If it failed, then we can't use this joystick. (Maybe the user unplugged
  // it while we were in the middle of enumerating it.)
	// if ok, found a joystick. increase number
  if(!FAILED(hr) )
		x->m_nrdevs++;

	// next device
	return DIENUM_CONTINUE;
}

static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
                                     VOID* pContext )
{
  HRESULT hr;
	t_dinput* x = (t_dinput*) pContext;

	if (x->m_currdev < x->m_nrdevs)
	{
		x->m_currdev++;

		return DIENUM_CONTINUE;
	}
	else
	{
		// Obtain an interface to the enumerated joystick.
		hr = x->m_di->CreateDevice( pdidInstance->guidInstance, &(x->m_dev), NULL );

		// If it failed, then we can't use this joystick. (Maybe the user unplugged
		// it while we were in the middle of enumerating it.)
		if( FAILED(hr) )
			error("can't open directinput device %d", x->m_devid);

		return DIENUM_STOP;
	}
}

static void FreeDirectInput(t_dinput* x)
{
	if( x->m_dev )
			x->m_dev->Unacquire();

 	SAFE_RELEASE( x->m_dev );
	SAFE_RELEASE( x->m_di );

	SAFE_DELETE( x->m_buttons );
	SAFE_DELETE( x->m_newstate );
	SAFE_DELETE( x->m_outlets );
}

static HRESULT InitDirectInput(t_dinput* x)
{
  HRESULT hr;

  // Register with the DirectInput subsystem and get a pointer
  // to a IDirectInput interface we can use.
  // Create a DInput object
  if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION,
                                       IID_IDirectInput8, (VOID**)&(x->m_di), NULL ) ) )
  {
		error("failed to create directinput interface");
		return hr;
	}

  // count joysticks
  if( FAILED( hr = x->m_di->EnumDevices( DI8DEVCLASS_GAMECTRL,
                                       EnumCountJoysticksCallback,
                                       x, DIEDFL_ATTACHEDONLY ) ) )
	{
		error("failed to enumerate devices for counting");
		return hr;
	}

  // Make sure we got one joystick at least
  if( x->m_nrdevs < 1)
  {
		error("no input devices found");
		return S_OK;
  }

	// make sure ID is in range
	if (x->m_devid < 0)
	{
		error("device number negative");
		return S_OK;
	}

	// make sure ID is in range
	if (x->m_devid >= x->m_nrdevs)
	{
		error("device number is out of range, there are only %d devices connected, with ids from 0 to %d",
			x->m_nrdevs, x->m_nrdevs-1);
		return S_OK;
	}

  // open joystick
  if( FAILED( hr = x->m_di->EnumDevices( DI8DEVCLASS_GAMECTRL,
                                       EnumJoysticksCallback,
                                       x, DIEDFL_ATTACHEDONLY ) ) )
  {
		error("failed to enumerate devices for opening");
    return hr;
 	}

  // Set the data format to "simple joystick" - a predefined data format
  //
  // A data format specifies which controls on a device we are interested in,
  // and how they should be reported. This tells DInput that we will be
  // passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().

	if( FAILED( hr = x->m_dev->SetDataFormat( &c_dfDIJoystick2 ) ) )
	{
		error("failed to set data format for input device");
		return hr;
	}

	// Set the cooperative level to let DInput know how this device should
	// interact with the system and with other DInput applications.
	if( FAILED( hr = x->m_dev->SetCooperativeLevel( GetForegroundWindow(),
													DISCL_EXCLUSIVE | DISCL_BACKGROUND  ) ) )
	{
		error("failed to set cooperative level for input device");
		return hr;
	}

	// everything fine...
  return S_OK;
}

static HRESULT UpdateInputState(t_dinput* x)
{
  HRESULT     hr;
  DIJOYSTATE2 js;           // DInput joystick state

	// Poll the device to read the current state
	hr = x->m_dev->Poll();
	if( FAILED(hr) )
	{
		// failed this time. try an acquire before trying again.
		x->m_dev->Acquire();
		error("failed to poll input device, tried acquire");
		return hr;
	}

	// Get the input's device state
	if( FAILED( hr = x->m_dev->GetDeviceState( sizeof(DIJOYSTATE2), &js ) ) )
	{
		error("failed to get input device state");
		return hr; // The device should have been acquired during the Poll()
	}

	// which buttons are pressed?
	for (t_int j=0; j<x->m_nrbuttons; j++)
		x->m_newstate[j] = js.rgbButtons[x->m_buttons[j]] & 0x80;

	// everything ok
  return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// PD functions
//////////////////////////////////////////////////////////////////////////////////////////////////////

static void dinput_bang(t_dinput* x)
{
	// update input state...
  UpdateInputState(x);

	// write changes to outlets
	for (t_int j=0; j<x->m_nrbuttons; j++)
	  outlet_float(x->m_outlets[j], x->m_newstate[j]);
}

static void* dinput_new(t_symbol* s, int argc, t_atom* argv)
{
	if (argc < 2)
	{
		error("not enough arguments");
		return 0;
	}

	// create the object
  t_dinput *x = (t_dinput *)pd_new(dinput_class);

	// get device number
	x->m_devid = atom_getfloatarg(0, argc, argv);
	x->m_nrbuttons = argc-1;
	x->m_buttons = new t_int[x->m_nrbuttons];
	x->m_newstate = new t_int[x->m_nrbuttons];

	// initialization
	x->m_di=NULL;
	x->m_dev=NULL;
	x->m_currdev=0;
	x->m_nrdevs=0;

	// init directinput stuff
  if(FAILED(InitDirectInput(x)))
		return NULL;

	// get button numbers from input list
	for (t_int j=0; j<x->m_nrbuttons; j++)
		x->m_buttons[j] = atom_getfloatarg(0, argc, argv);

	// alloc memory for outlet ptrs
	x->m_outlets = (t_outlet**) calloc(x->m_nrbuttons, sizeof(t_outlet*));

	// create all outlets
  for (t_int j=0; j<x->m_nrbuttons; j++)
  	x->m_outlets[j] = outlet_new(&x->x_obj, &s_float);

  return (void *)x;
}

static void dinput_delete(t_dinput* x)
{
	// clean up
	FreeDirectInput(x);
}

static void dinput_setup(void)
{
	// create class
  dinput_class = class_new(gensym("dinput"),
        (t_newmethod)dinput_new,
        (t_method)dinput_delete,
        sizeof(t_dinput),
        CLASS_DEFAULT,
        A_GIMME, 0);

	// just a bang input for polling
  class_addbang(dinput_class, dinput_bang);
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif