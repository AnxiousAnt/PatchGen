/* Copyright (c) 1997-2000 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"

static t_class *opanel_class;

typedef struct _opanel
{
    t_object x_obj;
    t_symbol *x_s;
} t_opanel;

static void *opanel_new(void)
{
    char buf[50];
    t_opanel *x = (t_opanel *)pd_new(opanel_class);
    sprintf(buf, "d%x", (t_int)x);
    x->x_s = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->x_s);
    outlet_new(&x->x_obj, &s_symbol);
    return (x);
}

static void opanel_bang(t_opanel *x)
{
    sys_vgui("pdtk_opanel %s\n", x->x_s->s_name);
}

static void opanel_anything(t_opanel *x, t_symbol *s, int argc, t_atom *argv)
{
    char buf[MAXPDSTRING];
    *buf = '\0';
    while (argc--) if (argv->a_type == A_SYMBOL)
    {
	if (*buf) strcat(buf, " ");
	strcat(buf, argv++->a_w.w_symbol->s_name);
    }
    if (*buf)
	outlet_symbol(x->x_obj.ob_outlet, gensym(buf));
}

static void opanel_free(t_opanel *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_s);
}

static void opanel_guidefs(void)
{
    sys_gui("proc  pdtk_opanel {target} {\n");
    sys_gui(" global pd_opendir\n");
    sys_gui(" set filename [tk_getOpenFile -initialdir $pd_opendir]\n");
    sys_gui(" if {$filename != \"\"} {\n");
    sys_gui("  set directory [string range $filename 0 \
		[expr [string last / $filename ] - 1]]\n");
    sys_gui("  set pd_opendir $directory\n");
    sys_gui("  pd [concat $target \"filename\" $filename\\;]\n");
    sys_gui(" }\n");
    sys_gui("}\n");
}

void opanel_setup(void)
{
    opanel_class = class_new(gensym("opanel"),
    	(t_newmethod)opanel_new, (t_method)opanel_free,
    	sizeof(t_opanel), 0, A_DEFFLOAT, 0);
    class_addbang(opanel_class, opanel_bang);
    class_addanything(opanel_class, (t_method)opanel_anything);
    opanel_guidefs();
}
