#include "m_pd.h"  
#include <stdlib.h>
#include <string.h>
 
void euclid(char* head, int nhead, char* tail, int ntail, char* outstr)
{

    if ((ntail <= 1) || (nhead==0)) {  // flatten and return

        strcpy(outstr, "\0"); // clear out string

        int i;
        for (i=0; i<nhead; i++) {
            strcat(outstr, head);
        }
        for (i=0; i<ntail; i++) {
            strcat(outstr, tail);
        }

    } else if (nhead < ntail) {

        char newhead[strlen(head) + strlen(tail) + 1];
        strcpy(newhead, head);
        strcat(newhead, tail);

        euclid(newhead, nhead, tail, ntail-nhead, outstr);

    } else {

        char newhead[strlen(head) + strlen(tail) + 1];
        strcpy(newhead, head);
        strcat(newhead, tail);

        char newtail[strlen(head)]; 
        strcpy(newtail, head);

        euclid(newhead, ntail, newtail, nhead-ntail, outstr);

    }
}

/*---*/

static t_class* euclid_class;  
 
typedef struct _euclid {  
    t_object  x_obj;  
    t_int nsteps;
    char** data;
    t_int counter;
    t_int density;
    t_int rotation;

    t_inlet *x_in2;
    t_inlet *x_in3;

    t_outlet *x_out;
} t_euclid;  
 
void euclid_bang(t_euclid* x)  
{  
    t_int cnt = x->counter; // tmp var to avoid reentrance
    t_int i = (x->rotation + cnt) % x->nsteps;
    if (x->data[x->density][i]=='1') outlet_bang(x->x_obj.ob_outlet);
    if (++x->counter == x->nsteps) x->counter=0;
}

void* euclid_new(t_floatarg f)  
{  
    t_euclid* x = (t_euclid*) pd_new(euclid_class);

    // initialize data space
    x->nsteps = f;
    x->counter = 0;
    x->density = 0;
    x->rotation = 0;

    // allocate the main data structure (beat grid)
    x->data = (char**) malloc((x->nsteps+1)*sizeof(char*));
    int i;
    for (i=0; i < x->nsteps+1; i++) {
        x->data[i] = (char*) malloc(x->nsteps+1*sizeof(char));
    }

    // populate beat grid
    for (i=0; i <= x->nsteps; i++) {
        euclid("1", i, "0", x->nsteps-i, x->data[i]);
    }

    // inlets
    x->x_in2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("density"));
    x->x_in3 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("rotation"));

    // outlets 
    x->x_out = outlet_new(&x->x_obj, &s_bang);
 
    return (void *)x;  
}

void euclid_free(t_euclid* x)
{
    int i;
    for (i=0; i < x->nsteps+1; i++) {
        free(x->data[i]);
    }
    free(x->data);

    inlet_free(x->x_in2);
    inlet_free(x->x_in3);
    outlet_free(x->x_out);
} 
 
void euclid_density(t_euclid *x, t_float f)
{
    x->density = f;
    if (f < 0) x->density = 0;
    if (f > x->nsteps) x->density = x->nsteps;
} 

void euclid_rotation(t_euclid *x, t_float f)
{
    x->rotation = f;
    if (f < 0) x->rotation = 0;
    if (f > x->nsteps) x->rotation = x->nsteps;
} 

void euclid_setup(void) {  

    euclid_class = class_new(gensym("euclid"),  
        (t_newmethod)euclid_new, (t_method)euclid_free,
        sizeof(t_euclid), CLASS_DEFAULT,
        A_DEFFLOAT, 16);  
        //A_GIMME, 0);  

    class_addbang(euclid_class, euclid_bang);  

    class_addmethod(euclid_class, (t_method)euclid_density,
            gensym("density"), A_DEFFLOAT, 0);

    class_addmethod(euclid_class, (t_method)euclid_rotation,
            gensym("rotation"), A_DEFFLOAT, 0);
}
