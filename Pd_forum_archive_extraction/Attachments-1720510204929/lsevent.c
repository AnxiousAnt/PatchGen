#include <linux/input.h>

#include <sys/stat.h>

#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define LINUXEVENT_DEVICE   "/dev/input/event"

static char *version = "$Revision: 1.6 $";

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

int main(void) {
  int i,eventType, eventCode, buttons, rel_axes, abs_axes, ff, fd;
/*   unsigned long bitmask[EV_MAX][NBITS(KEY_MAX)]; */
  char devicename[256] = "Unknown";
  char devname[18] = "/dev/input/event0";
  struct input_event  x_input_event; 

  printf("Compiled with EV_VERSION 0x%x\n",EV_VERSION);
  
  for (i=0;i<8;++i) 
  {
	  sprintf(&devname,"%s%d",LINUXEVENT_DEVICE,i);
	  /* open device */
	  if (devname) {
		  /* open the device read-only, non-exclusive */
		  fd = open (&devname, O_RDONLY | O_NONBLOCK);
		  /* test if device open */
		  if (fd < 0 ) { 
			  printf("Nothing on %s.\n",&devname);
			  fd = -1;
/* 			  return 0; */
		  } else {
			  /* read input_events from the LINUXEVENT_DEVICE stream 
				* It seems that is just there to flush the event input buffer?
				*/
			  while (read (fd, &(x_input_event), sizeof(struct input_event)) > -1);
			  
			  /* get name of device */
			  ioctl(fd, EVIOCGNAME(sizeof(devicename)), devicename);
			  printf("Found '%s' on '%s'\n",devicename,&devname);

			  close (fd);
		  }
	  } else 
	  {
		  printf("%s does not exist.\n",&devname);
	  }
  }
  
	  /* get bitmask representing supported events (axes, buttons, etc.) */
/*   memset(bitmask, 0, sizeof(bitmask)); */
/*   ioctl(fd, EVIOCGBIT(0, EV_MAX), bitmask[0]); */
/*   printf("\nSupported events:"); */
    
/*   rel_axes = 0; */
/*   abs_axes = 0; */
/*   buttons = 0; */
/*   ff = 0; */
    
/*   /\* cycle through all possible event types *\/ */
/*   for (eventType = 0; eventType < EV_MAX; eventType++) { */
/*     if (test_bit(eventType, bitmask[0])) { */
/*       printf(" %s (type %d) ", events[eventType] ? events[eventType] : "?", eventType); */
/*       //	printf("Event type %d",eventType); */

/*       /\* get bitmask representing supported button types *\/ */
/*       ioctl(fd, EVIOCGBIT(eventType, KEY_MAX), bitmask[eventType]); */

/*       /\* cycle through all possible event codes (axes, keys, etc.)  */
/*        * testing to see which are supported   */
/*        *\/ */
/*       for (eventCode = 0; eventCode < KEY_MAX; eventCode++)  */
/* 	if (test_bit(eventCode, bitmask[eventType])) { */
/* 	  printf("    Event code %d (%s)", eventCode, names[eventType] ? (names[eventType][eventCode] ? names[eventType][eventCode] : "?") : "?"); */

/* 	  switch(eventType) { */
/* #ifdef EV_RST */
/* 	  case EV_RST: */
/* 	    break; */
/* #endif */
/* 	  case EV_KEY: */
/* 	    buttons++; */
/* 	    break; */
/* 	  case EV_REL: */
/* 	    rel_axes++; */
/* 	    break; */
/* 	  case EV_ABS: */
/* 	    abs_axes++; */
/* 	    break; */
/* 	  case EV_MSC: */
/* 	    break; */
/* 	  case EV_LED: */
/* 	    break; */
/* 	  case EV_SND: */
/* 	    break; */
/* 	  case EV_REP: */
/* 	    break; */
/* 	  case EV_FF: */
/* 	    ff++; */
/* 	    break; */
/* 	  } */
/* 	} */
/*     }         */
/*   } */
    
/*   post ("\nUsing %d relative axes, %d absolute axes, and %d buttons.", rel_axes, abs_axes, buttons); */
/*   if (ff > 0) post ("Detected %d force feedback types",ff); */

return 1;
}
