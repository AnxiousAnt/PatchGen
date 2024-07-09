/* xcov~ -- cross covariance computed in frequency domain
computes the cross covariance between two signals on the blocksize
as 
output(i)=sum(k=0...last_sample, sig1(k)*sig2(k+i))

the output is computed across as many blocks until resetting
bang resets the object

when sig2 is a delayed version of sig1, the peak of xcov
occurs in the right half of the output block
as shown in the following ascii graph
output:

i=
N/2.....0...-N/2+1
|-------|------|
0......N/2....2N-1
indexes in output array

*/


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <m_pd.h>
#define SQ(a) (a * a)

static t_class *xcov_class;

typedef struct _xcov {
    t_object  x_obj;
    t_float f;
    t_sample *bufferNsig1, *bufferNsig2;
    double *output_prev_block;
    t_int is_new_or_rszd;
    t_int n;
    t_int old_size;
} t_xcov;

static t_int *xcov_perform(t_int *w)
{
  t_xcov *x = (t_xcov *)(w[1]);

  t_sample *sig1 = (t_sample *)(w[2]);
  t_sample *sig2 = (t_sample *)(w[3]);
  t_sample *out  = (t_sample *)(w[4]);
  t_int size  = (int) w[5];
  t_int size2 = size*2;
  t_int half = size/2;
  t_int thrhalf = 3*half;
  t_float *expsig1 = NULL;
  t_float *revsig2 = NULL;
  t_float temp, temp2;
  t_int i=0;

  if (x->n!=size)
{
    x->n = size;
    x->is_new_or_rszd=1;
}

// This stuff here sets up two buffers to hold the previous N samples
// To get the usual overlapping block (2) design on each input

  if (x->is_new_or_rszd)
{
    if (x->bufferNsig1!=NULL)
    {
      resizebytes(x->bufferNsig1, x->old_size*sizeof(t_sample), size*sizeof(t_sample));
      resizebytes(x->bufferNsig2, x->old_size*sizeof(t_sample), size*sizeof(t_sample));
      resizebytes(x->output_prev_block, x->old_size*sizeof(t_sample), size*sizeof(double));
    }
    else
    {
      x->bufferNsig1=getbytes(size*sizeof(t_float));
      x->bufferNsig2=getbytes(size*sizeof(t_float));
      x->output_prev_block=getbytes(size*sizeof(double));
    }
      for(i=0; i<size; i++)
    {
      x->bufferNsig1[i]=0;
      x->bufferNsig2[i]=0;
      x->output_prev_block[i]=0;
    }
    x->old_size=size;
    x->is_new_or_rszd=0;
}

// The two signals are created, using a block size of 2N, --size2
// expsig1 is the expanded signal1 x->bufferNsig1 + sig1
// revsig2 is the reversed signal2 (reversed about i=0)
// it is made 0.0 on 0 to (half-1) and thrhalf to (2N-1)


  expsig1=(t_float *) getbytes(size2*sizeof(t_float));
  revsig2=(t_float *) getbytes(size2*sizeof(t_float));

// Loops for assignment of old values in new block + buffer

  for (i=0; i < half ; i++)
{
    expsig1[i]=x->bufferNsig1[i];
    revsig2[i]=0.0;
}
  for (i=half; i < size ; i++)
{
    expsig1[i]=x->bufferNsig1[i];
    revsig2[i]=sig2[size-i];
}
  expsig1[size]=sig1[0];
  revsig2[size]=sig2[0];
  for (i=size+1; i < thrhalf ; i++)
{
    expsig1[i]=sig1[i-size];
    revsig2[i]=x->bufferNsig2[size2-i];
}
  for (i=thrhalf; i < size2 ; i++)
{
    expsig1[i]=sig1[i-size];
    revsig2[i]=0.0;
}
//  Here we set the buffers for the next round
  for(i=0; i < size; i++)
{
    x->bufferNsig1[i]=(t_float) sig1[i];
    x->bufferNsig2[i]=(t_float) sig2[i];
}
//  fft the two blocks and multiply them
  mayer_realfft(size2, expsig1);
  mayer_realfft(size2, revsig2);
  
  expsig1[0]*=(revsig2[0]/size2);
  expsig1[size]*=(revsig2[size]/size2);
  
  
  for(i=1; i < size; i++)  
{
    temp=expsig1[i];
    temp2=expsig1[size2-i];
    expsig1[i]=(temp*revsig2[i]-temp2*revsig2[size2-i])/size2;
    expsig1[size2-i]=-1.0*(temp*revsig2[size2-i]+temp2*revsig2[i])/size2;
}
//  ifft
  
  mayer_realifft(size2, expsig1);

  for(i=0; i < half; i++)
  {
      out[i]=((float) x->output_prev_block[i]) + expsig1[thrhalf+i]/size2;
      out[half + i]=((float) x->output_prev_block[half + i]) + expsig1[i]/size2;
      x->output_prev_block[i] = x->output_prev_block[i] + expsig1[thrhalf+i]/((double) size2);
      x->output_prev_block[half + i] = x->output_prev_block[half + i] + expsig1[i]/((double) size2);
  }


  freebytes(expsig1, size2*sizeof(t_float));
  freebytes(revsig2, size2*sizeof(t_float));
  return(w+6);
}

static void xcov_dsp(t_xcov *x, t_signal **sp)
{
  dsp_add(xcov_perform, 5, x,
    sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}


//  Send a bang to clear the buffer and start over with calculations

static void xcov_bang(t_xcov *x)
{
  int i;
  for(i=0;i<x->n;i++)
    x->output_prev_block[i]=0;
}


static void *xcov_new()
{
    t_xcov *x = (t_xcov *)pd_new(xcov_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    x->is_new_or_rszd=1;
    x->bufferNsig1=NULL;
    x->bufferNsig2=NULL;
    x->output_prev_block=NULL;
    x->n=-1;
    x->old_size=-1;
    return (void *)x;
}

static void xcov_free(t_xcov *x)
{
  if     (x->bufferNsig1 != NULL)
    freebytes(x->bufferNsig1, x->n*sizeof(t_float));
  if     (x->bufferNsig2 != NULL)
    freebytes(x->bufferNsig2, x->n*sizeof(t_float));
  if     (x->output_prev_block != NULL)
    freebytes(x->output_prev_block, x->n*sizeof(t_float));
}

void xcov_tilde_setup(void) {
    xcov_class = class_new(gensym("xcov~"),
      (t_newmethod)xcov_new,
      (t_method)xcov_free, sizeof(t_xcov),
      CLASS_DEFAULT, A_GIMME, 0);

    class_addbang(xcov_class, (t_method)xcov_bang);
    class_addmethod(xcov_class,
      (t_method)xcov_dsp, gensym("dsp"), 0);
    CLASS_MAINSIGNALIN(xcov_class, t_xcov, f);
}


