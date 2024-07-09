/* creates object */
#define S3D_FACTORY_VERSION "0.1.0"
#include "m_pd.h"

static t_class *factory_class;               /* the factory class */

typedef struct _factory t_factory;

struct _factory 
{
  t_object  obj;      /* internal object structures */
  t_canvas* parent;
  int       objlistsize;
  t_pd**    objlist;  /* objects created by tis factory */
};

void *factory_new(t_symbol *s, int argc, t_atom *argv);

void *factory_new(t_symbol *s, int argc, t_atom *argv)
{
  char dirbuf[MAXPDSTRING], *nameptr;
  t_factory *x = (t_factory *)pd_new(factory_class);
  
  x->parent=canvas_getcurrent(); //s__X.s_thing;
  
  x->objlistsize=sizeof(t_pd*)*1000; // TODO change this dynamicly
  x->objlist=(t_pd*) getzbytes(x->objlistsize);

  return x;
}

void *factory_destroy(t_factory *x)
{
  freebytes(x->objlist, x->objlistsize);
}

static void factory_make(t_factory* x, t_symbol *s, int argc, t_atom *argv);

static void factory_make(t_factory *x, t_symbol *s, int argc, t_atom *argv)
{
  /* s->s_name */
  typedmess(x, s, argc, argv);
}
//extern void pd_doloadbang(void);

static void factory_dsrcdef(t_factory *x, float fid)
{
  t_atom xargv[10];
  int id=(int)fid;

  SETFLOAT(&xargv[0], 100.0f*fid);
  SETFLOAT(&xargv[1], 0.0f);
  SETSYMBOL(&xargv[2], gensym("dsrcdef"));
  SETFLOAT(&xargv[3], fid);
  typedmess(x->parent, gensym("obj"), 4, xargv);
  x->objlist[id]=pd_newest();
}

static void factory_psrcdef(t_factory *x, float fid)
{
  t_atom xargv[10];
  int id=(int)fid;

  SETFLOAT(&xargv[0], 100.0f*fid);
  SETFLOAT(&xargv[1], 0.0f);
  SETSYMBOL(&xargv[2], gensym("psrcdef"));
  SETFLOAT(&xargv[3], fid);
  typedmess(x->parent, gensym("obj"), 4, xargv);
  x->objlist[id]=pd_newest();
}

static void factory_fstrmdef(t_factory *x, float fid)
{
  t_atom xargv[10];
  int id=(int)fid;

  SETFLOAT(&xargv[0], 100.0f*fid);
  SETFLOAT(&xargv[1], 0.0f);
  SETSYMBOL(&xargv[2], gensym("fstrmdef"));
  SETFLOAT(&xargv[3], fid);
  typedmess(x->parent, gensym("obj"), 4, xargv);
  x->objlist[id]=pd_newest();
}

static void factory_delete(t_factory *x, t_floatarg fid)
{
  post("factory_delete");
  if(x->objlist[(int)fid])
  {
    glist_delete(x->parent, x->objlist[(int)fid]);
    x->objlist[(int)fid]=0;
  }
}

static void factory_version(t_factory *x)
{
  post(S3D_FACTORY_VERSION);
}

/* class initialization (loaded on pd startup) */
void factory_setup(void)
{
  factory_class = class_new( gensym("factory"),           /* symbolic name of class */
                            (t_newmethod)factory_new,     /* constructor */
                            (t_method)factory_destroy,    /* destructor */
                            sizeof(t_factory),            /* size of datastructure */
                            CLASS_DEFAULT, 
                            0,                            /* no arguments */
                            0);                           /* end */
                                                   
//    class_addmethod(factory_class, (t_method)factory_make, gensym("make"), A_SYMBOL, A_GIMME, 0);
    class_addmethod(factory_class, (t_method)factory_dsrcdef, gensym("dsrcdef"), A_FLOAT, 0);
    class_addmethod(factory_class, (t_method)factory_psrcdef, gensym("psrcdef"), A_FLOAT, 0);
    class_addmethod(factory_class, (t_method)factory_fstrmdef, gensym("fstrmdef"), A_FLOAT, 0);
    class_addmethod(factory_class, (t_method)factory_delete, gensym("delete"), A_FLOAT, 0);
    class_addmethod(factory_class, (t_method)factory_version, gensym("version"), 0);

     //class_sethelpsymbol( uvsrpd_class, gensym("help-uvsrpd.pd"));
}
