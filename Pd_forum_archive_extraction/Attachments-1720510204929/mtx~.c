#include "m_pd.h"

typedef struct  _mtx{
	t_object    x_obj;
    t_int       x_ch;
    t_float   **x_in_vectors;
    t_float   **x_out_vectors;
}t_mtx;

static t_class *mtx_class;

static t_int *mtx_perform(t_int *w){
    t_mtx *x = (t_mtx *)(w[1]);
    int nblock = (int)(w[2]);
    t_float **in_vectors = x->x_in_vectors;
    t_float **out_vectors = x->x_out_vectors;
    t_int i;
    for(i = 0; i < x->x_ch; i++){
        t_float *in = in_vectors[i];
        t_float *out = out_vectors[i];
        t_int n = nblock;
        while(n--)
            *out++ = *in++;
    }
    return (w + 3);
}

static void mtx_dsp(t_mtx *x, t_signal **sp){
    t_int i, nblock = sp[0]->s_n;
    t_signal **sigp = sp;
    t_float **vecp = x->x_in_vectors;
    for(i = 0; i < x->x_ch; i++)
		*vecp++ = (*sigp++)->s_vec; // input vectors
    vecp = x->x_out_vectors;
    for(i = 0; i < x->x_ch; i++)
		*vecp++ = (*sigp++)->s_vec; // output vectors
    dsp_add(mtx_perform, 2, x, nblock);
}

static void *mtx_free(t_mtx *x){
    freebytes(x->x_in_vectors, x->x_ch * sizeof(*x->x_in_vectors));
    freebytes(x->x_out_vectors, x->x_ch * sizeof(*x->x_out_vectors));
	return(void *)x;
}

static void *mtx_new(t_floatarg f){
	t_mtx *x = (t_mtx *)pd_new(mtx_class);
    x->x_ch = (int)f < 1 ? 1 : (int)f > 64 ? 64 : (int)f;
	x->x_in_vectors = getbytes(x->x_ch * sizeof(*x->x_in_vectors));
    x->x_out_vectors = getbytes(x->x_ch * sizeof(*x->x_out_vectors));
    t_int i;
    for(i = 0; i < x->x_ch; i++)
        inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	for(i = 0; i < x->x_ch; i++)
	 	outlet_new(&x->x_obj, gensym("signal"));
	return(x);
}

void mtx_tilde_setup(void){
    mtx_class = class_new(gensym("mtx~"), (t_newmethod)mtx_new, (t_method)mtx_free,
			     sizeof(t_mtx), CLASS_NOINLET, A_DEFFLOAT, 0);
    class_addmethod(mtx_class, (t_method)mtx_dsp, gensym("dsp"), A_CANT, 0);
}
