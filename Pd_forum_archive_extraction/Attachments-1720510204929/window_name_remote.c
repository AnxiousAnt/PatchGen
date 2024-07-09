#include <stdio.h>
#include <string.h>
#include <m_pd.h>
#include <g_canvas.h>

#define DEBUG(x)

static t_class *window_name_class;
static t_symbol *remote_canvas;

typedef struct _window_name
{
    t_object x_obj;
    t_symbol *windowname;
} t_window_name;

static void window_name_bang(t_window_name *x)
{
    outlet_symbol(x->x_obj.ob_outlet,x->windowname);
}

static void *window_name_new(t_symbol *remote_name)
{
    t_window_name *x = (t_window_name *)pd_new(window_name_class);
    
    remote_canvas = (t_canvas *)pd_findbyclass(remote_name, canvas_class);
    
    char buf[MAXPDSTRING];
    
    snprintf(buf, MAXPDSTRING, ".x%lx", (long unsigned int)remote_canvas);
    x->windowname = gensym(buf);
    
    outlet_new(&x->x_obj, &s_symbol);

    return(x);
}

void window_name_setup(void)
{
    window_name_class = class_new(gensym("window_name"), 
        (t_newmethod)window_name_new, NULL, 
        sizeof(t_window_name), 0, A_DEFSYM, 0);

    class_addbang(window_name_class, (t_method)window_name_bang);
}
