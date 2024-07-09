/*       The controlfreak code version 2.0  by Charles Z Henry
         7/10/06.  This code is used for subband adaptive filtering
         upon wavelet coefficients from the wpp~ object               
         Uses a type of block frequency domain filter acting on the
         subbands of a wavelet transform.
*/

#include "m_pd.h"
#include <math.h>
#include <stdlib.h>
#include "fftw3.h"
#include <stdio.h>
#include <alloca.h>
#include <fcntl.h>
#include <unistd.h>
#define SQ(a) (a * a)

//  unwrap function; unwraps a phase vector a
// specified input arguments will be vector a and length(a)
//    tol is assumed to be pi
//    the first number is 0, and so on, this is 1-sided
//    the unwrap is fftw R2HC format compatible
//  a is modified to smooth out jumps in phase > tol and returned

void unwrap(float *a, int n)
{
  float tol = 3.1416;
  double *phi;
  phi = alloca(n * sizeof(double));
  phi[0]=0.0;
  int i;
  for(i=1; i < n; i++)
    phi[i]=phi[i-1]+( (fabsf(a[i]-a[i-1]) > tol) ? ((a[i] > a[i-1]) ? -6.28318530717959 : 6.28318530717959 ): 0 );
  for(i=1; i < n; i++)
    a[i]+=((float) phi[i]);
}

typedef int integer;

static t_class *controlfreak_tilde_class;

typedef struct _controlfreak_tilde {
  t_object x_obj;
  t_sample x_f;
  t_sample alpha;
  t_sample adapt_on;
  int is_new_or_rszd;
  int adapt_state;
  int adapt_level;
  int n;
  t_outlet *cc_out;
  double cc_prev;
  t_outlet *err_out;
  double err_prev;
  t_sample *w;                              // place for creating filter coefficients
  t_sample *w_prev;
  int check_adapt;
} t_controlfreak_tilde;

t_int *controlfreak_tilde_perform(t_int *w)
{
  t_controlfreak_tilde *x = (t_controlfreak_tilde *)(w[1]);

  t_sample *sig = (t_sample *)(w[2]);       // main signal, to be filtered
  t_sample *rec = (t_sample *)(w[3]);       // received signal from mic
  t_sample *ref = (t_sample *)(w[4]);       // reference signal to be matched
  t_sample *out = (t_sample *)(w[5]);       // filtered output signal
  t_sample *wtout = (t_sample *)(w[6]);     // coefficients from 0 to N/2 (length N/2+1)
  int n = (int)(w[7]);                      // 1st three coefficients unused

  t_sample sum;               //And the rest
  t_sample *wptr = NULL;
  t_sample *sigptr = NULL;
  t_sample *sigptr2= NULL;
  float *recsub, *refsub, *weights, *phase, *mag, *signal;
  fftwf_plan plan;

  float temp, temp2, temp3, temp4, temp5, adj_coeff;

  double tempd, tempd2, tempd3, tempd4, tempd5;
  double r, xnormsqr, factor, mnormsqr, ratio;
  double acc;

  int wlength = n/2+1;
  int half, size, end, wbeg, mid;
  int k = (int) (log(n)/M_LN2);
  int adapt = 0;
  int l = k-1;
  int t = (((l-10) > 2) ? (l-10) : 2);
  int b,d,f, g,h,i,j=0;
  int tempn=1;

  if (x->n != n)
  {
    x->is_new_or_rszd=1;
    x->n=n;
  }

//               What to do when new; set up filter, make impulse response

  if (x->is_new_or_rszd)
  {
    if (x->w == NULL)
    {
      x->w=(t_sample *) calloc(wlength, sizeof(t_sample));
      x->w_prev=(t_sample *) calloc(wlength, sizeof(t_sample));
    }
    else
    {
      free(x->w_prev);
      free(x->w);
      x->w=(t_sample *) calloc(wlength, sizeof(t_sample));
      x->w_prev=(t_sample *) calloc(wlength, sizeof(t_sample));
    }
    x->is_new_or_rszd=0;
    x->w[0]=0;
    x->w[1]=0;
    x->w[2]=0;
    for (j=1;j<k;j++)
    {
      x->w[(1<<j)+1]=1;
      x->w_prev[(1<<j)+1]=1;
    }
  }
//               :::::::::::::::::::::Adaptation scheme:::::::::::::::::::::::
  if (x->adapt_on)
  {
    switch (x->adapt_state)
    {
      case 0:
  //    adapt_level = 0; no adaptation;  this prevents the adaptation
  // algorithm from running consecutively; it makes sure that the new
  // filter is run for at least one cycle before the next adaptation
  // do nothings during state 0; advance to 1
        x->adapt_state=1;    
        break;
      case 1:
  //    adapt_level = 1 performs an adaptation
  // Find the subinterval of the wavelet transform with the greatest L2 norm
  // in the reference signal, prepare to adapt on that interval
  // sets x->adapt_state to 2, and x->adapt_level

        b=0;
        d=(t-1);
        temp2=0;
        for(d=l;d>=t;d--)
        {
          size=1<<d;
          temp=0;
          for(h=size; h < 2*size; h++)
            temp+=SQ(ref[h]);
          temp/= ((float) size);
          if (temp > temp2)
          {
            temp2=temp;
            b=d;
          }
        }
        if (temp2 == 0.0)
  				x->adapt_state=1;
  			else
        {
          if (x->adapt_level != b)
          {
            x->adapt_level=b;  
            x->cc_prev=10.0;
            x->err_prev=10.0;
          }

//    connect state 1, to the other states;
//    so, x->check_adapt is a state that tells where the the next state is
//    values:
//    0_>Take adaptation on next step; case 2 sets x->check_adapt to 1
//    1_>Normalize weights by testing; case 3 sets x->check_adapt to 2
//    2_>calc error and reject adaptation unless improvement occurs; case 4
//       case 4 sets x->check_adapt to 0
x->adapt_state= x->check_adapt+2;

        }
        break;
      case 2:
//    adapt_state = 2 performs an adaptation


        j=x->adapt_level;

//		Initialization within the subinterval of choice
//    these variables are also repeated within

        size=1<<j;
        half=size>>1;
        wbeg=half+1;
        end=(size<<1)-1;
        mid=size+half;
        adapt = 0;

        for(i=0; i<wlength; i++)
            x->w_prev[i]=x->w[i];

        h=0;
        i=0;
        if (ref[mid]==0.0)
        {
          sigptr=ref+mid;
          g=half-1;
          while((*(sigptr++)==0.0) && ((h++) < g));
          sigptr=ref+mid;
          while((*(sigptr--)==0.0) && ((i++) < g));
          if ((h+i) < half)
            adapt=1;
        }
        else
          adapt=1;

//                adapt, duh

        if (adapt==1)
        {
          weights=calloc(size,sizeof(float));
          refsub=alloca(size*sizeof(float));
          recsub=alloca(size*sizeof(float));
          phase=alloca(half*sizeof(float));
          mag=alloca((half+1)*sizeof(float));

          for(i=0; i < half; i++)
          {
            weights[i]=x->w[wbeg+i];
            refsub[i]=ref[size+i];
            recsub[i]=rec[size+i];
          }
          for(i=half; i < size; i++)
          {
            weights[i]=0;
            refsub[i]=ref[size+i];
            recsub[i]=rec[size+i];
          }
          plan=fftwf_plan_r2r_1d(size, refsub, refsub, FFTW_R2HC, FFTW_MEASURE);
          fftwf_execute(plan);
          plan=fftwf_plan_r2r_1d(size, recsub, recsub, FFTW_R2HC, FFTW_MEASURE);
          fftwf_execute(plan);
          plan=fftwf_plan_r2r_1d(size, weights, weights, FFTW_R2HC, FFTW_MEASURE);
          fftwf_execute(plan);
          mag[0]=((fabsf(refsub[0]) > fabsf(recsub[0]))? 1.1 : 0.9);
          mag[half]=((fabsf(refsub[half]) > fabsf(recsub[half]))? 1.1 : 0.9);
          phase[0]=0;
          for(i=1; i < half; i++)
          {
            mag[i]=( ((recsub[i]!=0.0) || (recsub[size-i]!=0.0)) ? sqrtf(SQ(refsub[i])+SQ(refsub[size-i]))/sqrtf(SQ(recsub[i])+SQ(recsub[size-i])) : 1.0);
            phase[i]=atan2f((refsub[size-i]*recsub[i]) - (refsub[i]*recsub[size-i]), (refsub[i]*recsub[i]) + (refsub[size-i]*recsub[size-i]));
          }
          unwrap(phase,half);
          weights[0]*=(((refsub[0]*recsub[0]) > 0.0)?1.0:-1.0)*powf(mag[0],x->alpha);
          weights[half]*=(((refsub[half]*recsub[half]) > 0.0)?1.0:-1.0)*powf(mag[half],x->alpha);
          for(i=1; i < half; i++)
          {
            temp=weights[i];
            temp2=weights[size-i];
            temp3=powf(mag[i],x->alpha);
            temp4=x->alpha*phase[i];
            temp5=sinf(temp4);
            temp4=cosf(temp4);
            weights[i]=temp3*(temp*temp4 - temp2*temp5);
            weights[size-i]=temp3*(temp*temp5 + temp2*temp4);
          }
          plan=fftwf_plan_r2r_1d(size, weights, weights, FFTW_HC2R, FFTW_ESTIMATE);
          fftwf_execute(plan);

//  Heres the tricky part, we're going to multiply the weights in the time 
//  domain by a smooth signal, a truncated Gaussian with a mean
//  of the first moment of t, E(t) the expectation of t, in sum(arg:t, t*u(t)^2)/sum(u(t)^2);
//  gaussian, g=K*e^(-(t-E(t))^2/2*sigma^2), then 
//  G(f)= K*sigma/sqrt(2*pi) * e^(i*2*pi*f*E(t)) * e^(-sigma^2*f^2 * 2pi / 2)

//  G(f) is to be normalized!, so we choose K, but the math is not real clear
//  keeping in mind the Gaussian dist is truncated...
//  we have to divide our distribution to have the time domain signal
//  converge to 1, as alpha goes to 0

//  first we calculate the first moment of t on the weights;
//  interpreted as a periodic expectation, which I have made up
//  r(t)=atan2f(sum(arg:j, sin(2*pi*j/half)*u^2(j)),sum(arg:j, cos(2*pi*j/half)*u^2(j))
//  results: (0, pi), okay; (-pi, 0) maps to (pi, 2pi)
//  then multiply by half/2pi; E(t)=half/2pi*r(t)


          tempd=0.0;
          tempd2=0.0;
          tempd3=0.0;
          for(i=0; i<half; i++)
          {
            tempd=SQ((double) weights[i]);
            tempd2+=tempd*sin(2*M_PI*i/((double) half));
            tempd3+=tempd*cos(2*M_PI*i/((double) half));
          }
          tempd=atan2(tempd2,tempd3);
          tempd=half/2/M_PI*((tempd < 0.0) ? (2*M_PI + tempd) : tempd);
          //tempd is the E(t) in samples
          adj_coeff = .06;
          tempd2=(double) ((x->alpha != 0.0) ? x->alpha/adj_coeff/half : 1); 
          //temp2 is 1/std dev.
          tempd4=2.0/(erf((((double) half)-tempd)*tempd2) - erf(-1.0*tempd*tempd2));
          //temp4 is 2/(difference of erfs)
          for(i=0; i<half; i++)
          {
// correct: tempd5=half/sqrt(M_PI)*tempd2*tempd4*exp( -0.5 * SQ( (((double) i)-tempd) *tempd2));
            tempd5=exp( -0.5 * SQ( (((double) i)-tempd) *tempd2));
            weights[i]*=((float) tempd5);
          }

          temp=1.0/((float) size);
          for(i=0; i < half; i++)
            x->w[wbeg+i]=weights[i]*temp;
          free(weights);
//  adaptation finished; do nothings & run filter only, on next block
          x->adapt_state=0;
          x->check_adapt=1;
/*
          if (x->alpha < 0.2)
            x->check_adapt=1;
          else
            x->check_adapt=0;
*/


        break;

      case 3:
//  adapt_level = 3; reached from adapt_level = 1 state
//  take a measurement and normalize weights

        j=x->adapt_level;

//		Initialization within the subinterval of choice
//    these variables are also repeated within
//    the subinterval's first 


        size=1<<j;
        half=size>>1;
        wbeg=half+1;
        end=(size<<1)-1;
        mid=size+half;
        xnormsqr=0.0;
        mnormsqr=0.0;
        for(i=(size+1); i <= end; i++)
          {
            xnormsqr+=SQ((double) ref[i]);
            mnormsqr+=SQ((double) rec[i]);
          }
          factor=(float) xnormsqr/mnormsqr;
        for(i=0; i < half; i++)
          {
            x->w[wbeg+i]*=factor;
          }

//          factor=((mnormsqr != 0) ? r/sqrtf(xnormsqr*mnormsqr): 0);
//option        outlet_float(x->cc_out, (float) factor);   //correlation coefficient, <x,m>/sqrt(|x|2*|m|2)          


//  These connect state

        x->check_adapt=2;  ///goto case 4 next time reference signal is nonzero
        x->adapt_state=0;
          
        break;
      case 4:


//    adapt_level = 4: check calculations for improvement;
// reject adaptation if error or correlation coefficient get worse
        j=x->adapt_level;

//		Initialization within the subinterval of choice
//    these variables are also repeated within

        size=1<<j;
        half=size>>1;
        wbeg=half+1;
        end=(size<<1)-1;
        mid=size+half;
//	compute dot product,r  between recevied signal from mic and reference signal
//      and the norm of the ref/rec signals squared, xnormsqr and mnormsqr

          r=((double) rec[size])*((double) ref[size]);
          xnormsqr=SQ((double) ref[size]);
          mnormsqr=SQ((double) rec[size]);
        
          for(i=(size+1); i <= end; i++)
          {
            r+=((double) ref[i])*((double) rec[i]);
            xnormsqr+=SQ((double) ref[i]);
            mnormsqr+=SQ((double) rec[i]);
          }
          factor=((mnormsqr != 0) ? r/sqrtf(xnormsqr*mnormsqr): 0);

// compute error within subband
      
          acc=0.0;
          for(i=0; i < size; i++)
            acc+=SQ( ((double) (ref[size+i]-rec[size+i])));
          acc=sqrt(acc/((double) size));
          outlet_float(x->cc_out, (float) factor);   //correlation coefficient, <x,m>/sqrt(|x|2*|m|2)
          outlet_float(x->err_out, (float) acc);    //square root mean square error within subband

      if  ( fabs(x->cc_prev-1.0) < fabs(fabs(factor)-1.0) ) 
        {
// revert to old set of weights whenever either, r, the corr gets worse by adaptation
          for(i=0; i < wlength; i++)
              x->w[i]=x->w_prev[i];      
        }
        else
        {
          if (r < 0.0)
          {
            for(i=0; i<half; i++)
              x->w[wbeg+i]*=-1.0;
            factor*=-1.0;
          }
          x->cc_prev=((float) factor);
          x->err_prev=((float) acc);
        }
        x->check_adapt=0;
        x->adapt_state=0;
        break;

      }
    }
  } 

//         ::::::::::::::::::::::::filtering routine::::::::::::::::::::::::::::::

//  After some consideration, I just decided to set these to zero
//  rather than try to adapt them

//  out[0]=sig[0]*x->w[0];
//  out[1]=sig[1]*x->w[1];
//  out[2]=sig[2]*x->w[2];
//  out[3]=sig[3]*x->w[2];                   ///Come back to this later...

  for (h=0; h < (1<<t); h++)
    out[h]=0;

// for incoming signal subinterval sizes of 4 or greater

  for (j=t; j<k; j++)
  {
    size=1<<j;
    half=size>>1;
    g=half-1;
    wbeg=half+1;
    end=(size<<1)-1;
    i = size*3/2;
    signal=calloc(i , sizeof(float));
    weights=calloc(i , sizeof(float));
    for(h=0; h < size; h++)
      signal[h]= sig[size+h];
    for(h=size; h < i; h++)
      signal[h]=0; 
    for(h=0; h < half;  h++)
      weights[h]= x->w[wbeg+h];
    for(h=half; h < i; h++)
      weights[h]= 0.0;      
    
    plan=fftwf_plan_r2r_1d(i, signal, signal, FFTW_R2HC, 0);
    fftwf_execute(plan);
    plan=fftwf_plan_r2r_1d(i, weights, weights, FFTW_R2HC, 0);
    fftwf_execute(plan);
    signal[0]*=weights[0];
    signal[i/2]*=weights[i/2];
    for(h=1; h < i/2; h++)
    {
      temp=signal[h];
      temp2=signal[i-h];
      signal[h]=temp*weights[h]-temp2*weights[i-h];
      signal[i-h]=temp*weights[i-h]+temp2*weights[h];
    }
    plan=fftwf_plan_r2r_1d(i, signal, signal, FFTW_HC2R, 0);
    fftwf_execute(plan);
    temp=1.0/((float) i);

    for(h=0; h < size; h++)
    {
      out[size+h]=signal[h]*temp;
    }

  free(signal);
  free(weights);

  }

  // Lastly output weights

  *wtout=*(x->w);
  i=0;
  wptr=x->w;
  while(i++ < wlength)
    *(wtout++)=*(wptr++);

  return (w+8);
}

void controlfreak_tilde_free (t_controlfreak_tilde *x)
{       
  free(x->w);
  free(x->w_prev);
}  

void controlfreak_tilde_save (t_controlfreak_tilde *x, t_symbol *instr)
{
  char *filename=instr->s_name;
  printf(filename);
  FILE *fd = fopen(filename, "w+");
  size_t i;
  if (fd == NULL)
    printf("can't open file\n");
  else if (x->w == NULL)
    printf("start audio first\n");
  else
  {
    i = fwrite(&x->n, sizeof(int), (size_t) 1, fd);
    if (i != 0)
      i = fwrite(x->w, sizeof(float),(size_t) (x->n/2 + 1),fd);
    i = fclose(fd); 
  }
}

void controlfreak_tilde_load (t_controlfreak_tilde *x, t_symbol *instr)
{
  char *filename=instr->s_name;
  FILE *fd = fopen(filename, "r");
  size_t i;
  int n;
  if (fd == NULL)
    printf("can't open file\n");
  else if (x->w == NULL)
    printf("start audio first\n");
  else 
  {
    i = fread( &n , sizeof(int), 1 , fd);
    if (i != 1)
      printf("error reading file\n");
    else if (n != x->n)
      printf("incompatible size; set blocksize to %i", n );
    else
      i = fread( x->w, sizeof(float), (size_t) (x->n/2 + 1),fd);
      i = fclose(fd); 
  }
}

void controlfreak_tilde_init(t_controlfreak_tilde *x, t_atom init_case)
{
        int k = (int) (log(x->n)/M_LN2);
        int l = k-1;
        int t = (((l-10) > 2) ? (l-10) : 2);
        int h,i,j, wlength = x->n/2 + 1;
        int maxlength, wbeg, wend;
  if (atom_getfloat(&init_case) == 0.0)
  {
        for (j=0; j < wlength; j++)
          x->w[j]=0;
        for (j=1;j<k;j++)
          x->w[(1<<j)+1]=1;
        x->cc_prev=10.0;
        x->err_prev=10.0;
        x->check_adapt=0;        
  }
  else
  {
        maxlength=1<<((int) atom_getfloat(&init_case));
        wbeg=maxlength+1;
        wend=2*maxlength+1;
        for(j=wbeg+1; j < wend; j++)
          x->w[j]=0;
        x->w[wbeg]=1;
  }
}
  

//       And then comes the stuff that I'm much indebted for.  Thanks.  *Yoink*

void controlfreak_tilde_dsp(t_controlfreak_tilde *x, t_signal **sp)
{
  dsp_add(controlfreak_tilde_perform, 7, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[0]->s_n);
}

void *controlfreak_tilde_new(void)
{
  t_controlfreak_tilde *x = (t_controlfreak_tilde *)pd_new(controlfreak_tilde_class);

  x->adapt_state=0;
  x->adapt_on=0;
  x->alpha=0;
  x->is_new_or_rszd=1;
  x->w=NULL;
  x->w_prev=NULL;
  x->n=0;  
  x->cc_prev=10;
  x->err_prev=10;
  x->check_adapt=0;


  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  floatinlet_new(&x->x_obj,&x->alpha);
  floatinlet_new(&x->x_obj,&x->adapt_on);
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  x->cc_out=outlet_new(&x->x_obj, &s_float);
  x->err_out=outlet_new(&x->x_obj, &s_float);
  return (void *)x;
}

void controlfreak_tilde_setup(void) {
  controlfreak_tilde_class = class_new(gensym("controlfreak~"), (t_newmethod)controlfreak_tilde_new, (t_method)controlfreak_tilde_free, sizeof(t_controlfreak_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
  class_addmethod(controlfreak_tilde_class, (t_method)controlfreak_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(controlfreak_tilde_class, (t_method)controlfreak_tilde_load, gensym("load"), A_SYMBOL,0);
  class_addmethod(controlfreak_tilde_class, (t_method)controlfreak_tilde_save, gensym("save"), A_SYMBOL,0);
  class_addmethod(controlfreak_tilde_class, (t_method)controlfreak_tilde_init, gensym("init"), A_FLOAT, 0);
  CLASS_MAINSIGNALIN(controlfreak_tilde_class, t_controlfreak_tilde, x_f);
}
