#include "m_imp.h"
#include "g_canvas.h"

typedef struct _whofeedsme
{
    t_object  x_ob;
    t_glist  *x_glist;
} t_whofeedsme;

static t_class *whofeedsme_class;

static void *whofeedsme_new(void)
{
    t_whofeedsme *x = (t_whofeedsme *)pd_new(whofeedsme_class);
    x->x_glist = (t_glist *)canvas_getcurrent();
    return (x);
}

static void whofeedsme_whofeedsme(t_whofeedsme *x)
{
    t_gobj *y;
    t_object *src;
    t_symbol *mname = gensym("whofeedsme");
    if (*(t_pd *)x == whofeedsme_class) y = x->x_glist->gl_list;
    else {
	t_glist *gl = (t_glist *)canvas_getcurrent();
	if (gl) y = gl->gl_list;
	else { post("gosh, is it immune?"); return; }  /* the stack is empty... */
    }
    for (; y; y = y->g_next) {
	if (src = pd_checkobject(&y->g_pd)) {
	    t_object *dest;
	    t_outconnect *oc;
	    t_outlet *op;
	    t_inlet *ip;
    	    int outno = obj_noutlets(src);
	    int inno;
	    t_class *sclass = *(t_pd *)src;
	    char *sname = class_getname(sclass);
	    post("[%s]", sname);
	    while (outno--) {
		oc = obj_starttraverseoutlet(src, &op, outno);
		while (oc) {
		    oc = obj_nexttraverseoutlet(oc, &dest, &ip, &inno);
		    post("\t-> [%s]", class_getname(*(t_pd *)dest));
		    if (dest == (t_object *)x) {
			int i = sclass->c_nmethod;
			t_methodentry *m = sclass->c_methods;
			startpost("*\there I am, [%s] is my feeder ", sname);
			while (i--) if (m++->me_name == mname) break;
			if (i >= 0) post("-- already infected...");
			else {
			    class_addmethod(sclass, (t_method)whofeedsme_whofeedsme,
					    mname, 0);
			    post("-- got infected!");
			}
		    }
		}
	    }
	}
    }
}

void whofeedsme_setup(void)
{
    whofeedsme_class = class_new(gensym("whofeedsme"),
				 (t_newmethod)whofeedsme_new, 0,
				 sizeof(t_whofeedsme), 0, 0);
    class_addmethod(whofeedsme_class, (t_method)whofeedsme_whofeedsme,
		    gensym("whofeedsme"), 0);
}
