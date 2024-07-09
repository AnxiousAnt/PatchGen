/*
  Arpeggio external, Atte André Jensen, 2007
  
  This code is released under GPL
*/
#include <m_pd.h>
#include <stdlib.h>
#include <time.h>
#define NB_ELEMENTS 200
static t_class *arp_class;

typedef struct _arp {
    t_object  x_obj;
    t_int nb_elements, direction;
    t_float notes[NB_ELEMENTS];
    t_int velocities[NB_ELEMENTS];
    //t_int current_pos, current_direction;
    t_float current_note, current_velocity, last_note, last_velocity, tmp;
    t_float note, velocity, mode, octaves;
    t_inlet *note_in, *velocity_in;
    t_outlet *note_out, *velocity_out;
} t_arp;


void arp_bang(t_arp *x) {
    t_float note, best_note, delta, best_delta;
    t_int octave, index;
    t_int random_number;

    //post("inside arp_bang");
    //post("x->mode: %f",x->mode);

    if(x->nb_elements == 0){
	return;
    }

    if(x->mode >= 0 && x->mode <= 3){
	if(x->current_note >= 0){
	    // send noteoff for sounding note
	    outlet_float(x->note_out, x->current_note);
	    outlet_float(x->velocity_out, 0);
	}

	best_delta = 1000 * -1 * x->direction;
	best_note = x->current_note;
	if(x->direction == 0){
	    // first time arround, different for each mode
	    if(x->mode == 0 || x->mode == 2){
		x->current_note = x->notes[0];
		x->current_velocity = x->velocities[0];
		x->direction = 1;
	    } else if(x->mode == 1 || x->mode == 3){
		x->current_note = x->notes[x->nb_elements - 1] + ((x->octaves -1)* 12);
		x->current_velocity = x->velocities[x->nb_elements - 1];
		x->direction = -1;
	    }
	} else {
	    for(octave = 0; octave < x->octaves; octave++){
		for(index = 0; index < x->nb_elements; index++){
		    note = x->notes[index] + 12 * octave;
		    delta = x->current_note - note;
		    //post("index:%i, octave:%i, note:%f, delta:%f",index, octave, note, delta);
		    if((x->direction > 0 && delta < 0 && delta > best_delta) ||
		       (x->direction < 0 && delta > 0 && delta < best_delta)){
			best_delta = delta;
			x->last_velocity = x->current_velocity;
			x->current_velocity = x->velocities[index];
			best_note = note;
			//post("delta::%f, best_delta: %f, note: %f, best_note:%f", delta, best_delta, note, best_note);
		    }
		}
	    }
	    if(best_note == x->current_note && x->nb_elements > 1){
		//post("at the end");
		// at the end
		if(x->mode == 0){
		    //post("mode 0");
		    x->current_note = x->notes[0];
		    x->current_velocity = x->velocities[0];
		} else if (x->mode == 1){
		    //post("mode 1");
		    x->current_note = x->notes[x->nb_elements - 1] + ((x->octaves -1)* 12);
		    x->current_velocity = x->velocities[x->nb_elements - 1];
		} else if(x->mode == 2 || x->mode == 3){
		    //post("mode 2/3");
		    x->tmp = x->current_note;
		    x->current_note = x->last_note;
		    x->last_note = x->tmp;
		    
		    x->tmp = x->current_velocity;
		    x->current_velocity = x->last_velocity;
		    x->last_velocity = x->tmp;
		    
		    x->direction = -1 * x->direction;
		}
		//post("end of switch if, x->current_note:%f, x->last_note:%f",x->current_note,x->last_note);
	    } else {
		x->last_note = x->current_note;
		x->current_note = best_note;
	    }
	}
	outlet_float(x->note_out, x->current_note);
	outlet_float(x->velocity_out, x->current_velocity);
    } else if(x->mode == 4){
	// random
	//post("mode:4");
	
	random_number = rand();
	
	index = random_number%x->nb_elements;
	octave = random_number%(int)x->octaves;
	/*
	post("random index:%i",index);
	post("random octave:%i",octave);
	*/
	x->current_note = x->notes[index] + 12 * octave;
	x->current_velocity = x->velocities[index];
	
	outlet_float(x->note_out, x->current_note);
	outlet_float(x->velocity_out, x->current_velocity);

    } else {
	//post("unknown mode");
    }
    
}

void arp_float(t_arp *x, t_floatarg f1) {
    //post("inside arp_float");
    int i;
    int found = -1;
    //t_float max = 0, min = 100000;
    int pos = 0;
    
    x->note = f1;

    //post("note:%f,velocity:%f",x->note,x->velocity);

    //post("max:%f, min:%f",max, min);
    if(x->velocity != 0){
	for(i=0; i<=x->nb_elements; i++){
	    // find position for new note
	    if(x->notes[i] == f1){
		pos = -1;
		i = x->nb_elements;
	    }
	    else if(x->notes[i] > x->note){
		pos = i;
		i = x->nb_elements;
	    } else {
		pos = i;
	    }
	}
	if(pos >= 0){
	    x->nb_elements++;
	    for(i=x->nb_elements; i>=pos; i--){
		x->notes[i] =x->notes[i-1];
		x->velocities[i] =x->velocities[i-1];
	    }
	    x->notes[pos] = x->note;
	    x->velocities[pos] = x->velocity;
	}
    } else {
	pos = -1;
	for(i=0; i<x->nb_elements; i++){
	    // find position for new note
	    if(x->notes[i] == f1){
		pos = i;
		i = x->nb_elements;
	    }
	}
	if(pos >= 0){
	    x->nb_elements--;
	    for(i=pos; i<=x->nb_elements; i++){
		x->notes[i] =x->notes[i+1];
		x->velocities[i] =x->velocities[i+1];
	    }
	}
	
    }

    /*
      if(x->velocity != 0){
      post("noteon...");
      } else {
      post("noteoff...");

      }
    */

    /*
      post("nb_elements:%i",x->nb_elements);
      post("%f %f %f %f %f %f %f %f %f %f",
      x->notes[0],
      x->notes[1],
      x->notes[2],
      x->notes[3],
      x->notes[4],
      x->notes[5],
      x->notes[6],
      x->notes[7],
      x->notes[8],
      x->notes[9]
      );
      post("%i %i %i %i %i %i %i %i %i %i",
      x->velocities[0],
      x->velocities[1],
      x->velocities[2],
      x->velocities[3],
      x->velocities[4],
      x->velocities[5],
      x->velocities[6],
      x->velocities[7],
      x->velocities[8],
      x->velocities[9]
      );
    */
}



void arp_velocity(t_arp *x, t_floatarg f1) {
    x->velocity = f1;
}

void arp_mode(t_arp *x, t_floatarg f1) {
    if(f1 >= 0 && f1 <= 4){
	x->mode = f1;
    } else {
	post("[arp]: mode out of range (0-3)");
    }
    //post("x->mode:%f",x->mode);
}

void arp_octaves(t_arp *x, t_floatarg f1) {
    if(f1 >= 1 && f1 <= 10){
	x->octaves = f1;
    } else {
	post("[arp]: octave out of range (1-10)");
    }
    //post("x->octaves:%f",x->octaves);
}


//void *arp_new(t_floatarg f) {
void *arp_new(t_symbol *s, int argc, t_atom *argv){
    //post("inside new");
    t_float mode, octaves;
    t_arp *x = (t_arp *)pd_new(arp_class);

    // seed random
    srand(time(0));

    inlet_new(&x->x_obj, &x->x_obj.ob_pd,
	      gensym("float"), gensym("velocity"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd,
	      gensym("float"), gensym("mode"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd,
	      gensym("float"), gensym("octaves"));
    
    x->note_out = outlet_new(&x->x_obj, &s_float);
    x->velocity_out = outlet_new(&x->x_obj, &s_float);

    mode = atom_getfloat(argv);
    x->mode = 0;
    //x->current_pos = -1;
    x->current_note = -1;
    x->direction = 0;
    //post("argc:%i",argc);
    if(argc > 0){
	if(mode >=0 && mode <=4){
	    x->mode = mode;
	} else {
	    post("[arp]: mode out of range (0-3)");
	}
    }
    
    octaves = atom_getfloat(argv+1);
    x->octaves = 1;
    //post("argc:%i",argc);
    if(argc > 1){
	if(octaves >=1 && octaves <=10){
	    x->octaves = octaves;
	} else {
	    post("[arp]: octaves out of range (0-2)");
	}
    }
    
    //post("mode:%f",x->mode);
    //post("octaves:%f",x->octaves);
    return (void *)x;
}

void arp_setup(void) {
    //post("inside arp_setup");
    arp_class = class_new(gensym("arp"),
			  (t_newmethod)arp_new,
			  0, sizeof(t_arp),
			  CLASS_DEFAULT,
			  A_GIMME, 0);

    class_addbang  (arp_class, arp_bang);
    class_addfloat  (arp_class, arp_float);

    class_addmethod(arp_class,
		    (t_method)arp_velocity, gensym("velocity"),
		    A_DEFFLOAT, 0);
    class_addmethod(arp_class,
		    (t_method)arp_bang, gensym("bang"),
		    A_DEFFLOAT, 0);
    class_addmethod(arp_class,
		    (t_method)arp_octaves, gensym("octaves"),
		    A_DEFFLOAT, 0);
    class_addmethod(arp_class,
		    (t_method)arp_mode, gensym("mode"),
		    A_DEFFLOAT, 0);
}

