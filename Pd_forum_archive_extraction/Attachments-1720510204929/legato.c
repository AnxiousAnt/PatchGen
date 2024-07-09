#include "m_pd.h"
#define NB_ELEMENTS 200
static t_class *legato_class;

typedef struct _legato {
    t_object  x_obj;
    t_int nb_elements;
    t_float buffer[NB_ELEMENTS];
    t_float note, velocity;
    t_inlet *note_in, *velocity_in;
    t_outlet *note_out, *velocity_out;
} t_legato;


void legato_float(t_legato *x, t_floatarg f1) {
    int i;
    int found = -1;

    // search buffer for note
    for(i=0; i<x->nb_elements; i++){
	if(x->buffer[i] == f1){
	    found = i;
	}
    }

    if(x->velocity != 0){
	// noteon
	if(NB_ELEMENTS == x->nb_elements){
	    //overflow, not doing anything...
	    return;
	}

	if(found == -1){
	    // note not in buffer,
	    // put it at the end...
	    x->buffer[x->nb_elements] = f1;
	    x->nb_elements++;
	    // ...save it...
	    x->note = f1;
	    // ...and send it out
	    outlet_float(x->note_out, f1);
	} else {
	    //post("this isn't supposed to happen...");
	    for(i=found; i<x->nb_elements;i++){
		x->buffer[i] = x->buffer[i+1];
	    }
	}

	if(x->nb_elements == 1){
	    // this is first note in buffer, send velocity
	    outlet_float(x->velocity_out, x->velocity);
	}
	

    } else {
	// noteoff
	if(found != -1){
	    for(i=found; i<x->nb_elements;i++){
		x->buffer[i] = x->buffer[i+1];
	    }
	    x->nb_elements--;
	    if(x->buffer[x->nb_elements-1] != x->note && x->nb_elements>0){
		x->note = x->buffer[x->nb_elements-1];
		outlet_float(x->note_out, x->note);
	    }

	}
	if(x->nb_elements == 0){
	    // this was the last note in buffer, send velocity 0
	    outlet_float(x->velocity_out, 0);
	}
    }
    
    /*
    post("nb_elements:%i",x->nb_elements);
    post("%f %f %f %f %f %f %f %f %f %f",
	 x->buffer[0],
	 x->buffer[1],
	 x->buffer[2],
	 x->buffer[3],
	 x->buffer[4],
	 x->buffer[5],
	 x->buffer[6],
	 x->buffer[7],
	 x->buffer[8],
	 x->buffer[9]
	 );
    */
}

void legato_velocity(t_legato *x, t_floatarg f1) {
    x->velocity = f1;
}


//void *legato_new(t_floatarg f) {
void *legato_new(t_symbol *s, int argc, t_atom *argv){
    t_legato *x = (t_legato *)pd_new(legato_class);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd,
	      gensym("float"), gensym("velocity"));
    
    x->note_out = outlet_new(&x->x_obj, &s_float);
    x->velocity_out = outlet_new(&x->x_obj, &s_float);

    return (void *)x;
}

void legato_setup(void) {
    legato_class = class_new(gensym("legato"),
			     (t_newmethod)legato_new,
			     0, sizeof(t_legato),
			     CLASS_DEFAULT,
			     A_GIMME, 0);

    class_addfloat  (legato_class, legato_float);
    class_addmethod(legato_class,
		    (t_method)legato_velocity, gensym("velocity"),
		    A_DEFFLOAT, 0);
}

