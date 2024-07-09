/* (C) Guenter Geiger <geiger@epy.co.at> */
/*                                       */
/* changed 10 03 2007 by Orm Finnendahl */
/* Changes:
/* - added varispeed as (left) signal inlet */
/* - added 4 point interpolation and renamed sfread4~ */

#include <m_pd.h>

#include <stdio.h>
#include <string.h>
#ifndef NT
#include <unistd.h>
#include <sys/mman.h>
#else
#include <io.h>
#endif


#include <fcntl.h>
#include <sys/stat.h>

/* ------------------------ sfread4~ ----------------------------- */

#ifdef NT
#define BINREADMODE "rb"
#else
#define BINREADMODE "r"
#endif

static t_class *sfread4_class;


typedef struct _sfread4
{
     t_object x_obj;
     void*     x_mapaddr;
     int       x_fd;
     t_int   x_play;
     t_int   x_channels;
     t_int   x_size;
     t_int   x_loop;
     t_float x_offset;
     t_float x_skip;
     t_float x_debug;
  float x_frac;
  float x_inc;
     t_canvas * x_canvas;
     t_outlet *x_bangout;
} t_sfread4;


void sfread4_open(t_sfread4 *x,t_symbol *filename)
{
     struct stat  fstate;
     char fname[MAXPDSTRING];

     if (filename == &s_) {
	  post("sfread4: open without filename");
	  return;
     }

     canvas_makefilename(x->x_canvas, filename->s_name,
			 fname, MAXPDSTRING);


     /* close the old file */

     if (x->x_mapaddr) munmap(x->x_mapaddr,x->x_size);
     if (x->x_fd >= 0) close(x->x_fd);

     if ((x->x_fd = open(fname,O_RDONLY)) < 0)
     {
	  error("can't open %s",fname);
	  x->x_play = 0;
	  x->x_mapaddr = NULL;
	  return;
     }

     /* get the size */

     fstat(x->x_fd,&fstate);
     x->x_size = fstate.st_size;

     /* map the file into memory */

     if (!(x->x_mapaddr = mmap(NULL,x->x_size,PROT_READ,MAP_PRIVATE,x->x_fd,0)))
     {
	  error("can't mmap %s",fname);
	  return;
     }
}

#define MAX_CHANS 4

static t_int *sfread4_perform(t_int *w)
{
     t_sfread4* x = (t_sfread4*)(w[1]);
     short* buf = x->x_mapaddr;
     t_float *in1 = (t_float *)(w[2]);
     x->x_debug=*in1;
     int c = x->x_channels;
     t_float offset = x->x_offset*c;
     t_float speed = *in1++;
     float frac = x->x_frac;
     float inc = x->x_inc;
     long aoff = x->x_offset;
/*      post("startfrac-inc: %8.4f  %8.4f", frac, inc); */
     int i,n;
     int end =  x->x_size/sizeof(short) - 2;
     t_float* out[MAX_CHANS];
     
     for (i=0;i<c;i++)  
	  out[i] = (t_float *)(w[3+i]);
     n = (int)(w[3+c]);
     
     /* loop */

     if (offset >  end)
	  offset = end;

     if (offset + n*c*speed > end) { // playing forward end
	  if (!x->x_loop) {
	       x->x_play=0;
	       offset = x->x_skip*c;
	  }
     }

     if (offset + n*c*speed < 0) {  // playing backwards end
	  if (!x->x_loop) {
	       x->x_play=0;
	       offset = end;
	  }

     }


     if (x->x_play && x->x_mapaddr) {

	  if (1) { /* different speed */
/* 	    post ("start: aoff=%d", aoff); */
	    while (n--) {
 	      for (i=0;i<c;i++)  {
 		if (aoff <= c) { 
 		  *out[i]++ = (t_sample) *(buf+i)*3.052689e-05;
 		}
 		else 
		  {
 		    float as = *(buf+aoff+i - c);
		    float bs = *(buf+aoff+i);
		    float cs = *(buf+aoff+i+c);
		    float ds = *(buf+aoff+i+c+c);
		    float cminusb;
		    float tmp;
		    cminusb = cs-bs;
		    tmp = bs + frac * (
				       cminusb - 0.1666667f * (1.-frac) * (
									   (ds - as - 3.0f * cminusb) * frac +
									   (ds + 2.0f*as - 3.0f*bs)
									   )
				       );
 		    *out[i]++ = tmp * 3.052689e-05;
		  }		
	      }
 	      inc = frac + speed;
	      aoff+=  ((int) inc) * c;
 	      frac = inc - (int)inc;
 	      speed = *in1++;
	      if (aoff > end) { 
		if (x->x_loop) aoff = x->x_skip;
		else break;
	      }
	      if (aoff < 0) {
		if (x->x_loop) aoff = end;
		else break;
	      }
	    }
	    /* Fill with zero in case of end */ 
	    n++;
	    while (n--) 
	      for (i=0;i<c;i++)  
		*out[i]++ = 0;
	    offset = aoff;
	  }
     }
     else {
       while (n--) {
	 for (i=0;i<c;i++)
	   *out[i]++ = 0.;
       }
     }
     x->x_offset = (float) aoff; /* this should always be integer !! */
     x->x_frac = frac;
     x->x_inc = inc;
     return (w+c+4);
}


static void sfread4_float(t_sfread4 *x, t_floatarg f)
{
     int t = f;
     if (t && x->x_mapaddr) {
	  x->x_play=1;
     }
     else {
	  x->x_play=0;
     }

}

static void sfread4_loop(t_sfread4 *x, t_floatarg f)
{
     x->x_loop = f;
}



static void sfread4_size(t_sfread4* x)
{
     t_atom a;

     SETFLOAT(&a,x->x_size*0.5/x->x_channels);
     outlet_list(x->x_bangout, gensym("size"),1,&a);
}

static void sfread4_state(t_sfread4* x)
{
     t_atom a;

     SETFLOAT(&a,x->x_play);
     outlet_list(x->x_bangout, gensym("state"),1,&a);
}


static void sfread4_bang(t_sfread4* x)
{
     x->x_offset = x->x_skip*x->x_channels;
     sfread4_float(x,1.0);
}


static void sfread4_dsp(t_sfread4 *x, t_signal **sp)
{
/*     post("sfread4: dsp"); */
     switch (x->x_channels) {
     case 1:
	  dsp_add(sfread4_perform, 4, x, sp[0]->s_vec, 
		  sp[1]->s_vec, sp[0]->s_n);
	  break;
     case 2:
	  dsp_add(sfread4_perform, 5, x, sp[0]->s_vec, 
		  sp[1]->s_vec,sp[2]->s_vec, sp[0]->s_n);
	  break;
     case 4:
	  dsp_add(sfread4_perform, 6, x, sp[0]->s_vec, 
		  sp[1]->s_vec,sp[2]->s_vec,
		  sp[3]->s_vec,sp[4]->s_vec,
		  sp[0]->s_n);
	  break;
     }
}


static void *sfread4_new(t_floatarg chan,t_floatarg skip)
{
    t_sfread4 *x = (t_sfread4 *)pd_new(sfread4_class);
    t_int c = chan;

    x->x_canvas =  canvas_getcurrent();

    if (c<1 || c > MAX_CHANS) c = 1;
    floatinlet_new(&x->x_obj, &x->x_offset);


    x->x_fd = -1;
    x->x_mapaddr = NULL;

    x->x_size = 0;
    x->x_frac = 0;
    x->x_inc = 1;
    x->x_loop = 0;
    x->x_channels = c;
    x->x_mapaddr=NULL;
    x->x_offset = skip;
    x->x_skip = skip;
    x->x_play = 0;

    while (c--) {
	 outlet_new(&x->x_obj, gensym("signal"));
    }

     x->x_bangout = outlet_new(&x->x_obj, &s_float);

/*  post("sfread4: x_channels = %d, x_speed = %f",x->x_channels,x->x_speed);*/

    return (x);
}

void sfread4_tilde_setup(void)
{
     /* sfread4 */

    sfread4_class = class_new(gensym("sfread4~"), (t_newmethod)sfread4_new, 0,
    	sizeof(t_sfread4), 0,A_DEFFLOAT,A_DEFFLOAT,0);
    class_addmethod(sfread4_class, nullfn, gensym("signal"), 0);
    class_addmethod(sfread4_class, (t_method) sfread4_dsp, gensym("dsp"), 0);
    class_addmethod(sfread4_class, (t_method) sfread4_open, gensym("open"), A_SYMBOL,A_NULL);
    class_addmethod(sfread4_class, (t_method) sfread4_size, gensym("size"), 0);
    class_addmethod(sfread4_class, (t_method) sfread4_state, gensym("state"), 0);
    class_addfloat(sfread4_class, sfread4_float);
    class_addbang(sfread4_class,sfread4_bang);
    class_addmethod(sfread4_class,(t_method)sfread4_loop,gensym("loop"),A_FLOAT,A_NULL);

}
