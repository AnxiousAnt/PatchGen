/*
 *   LPC Toolkit
 *   By Mark Cartwright
 *   Pure Data port by Edward Kelly 2018
 *   BSD licence
 *
 *
 */

#include "m_pd.h"
#include <math.h>
//#include <gsl/gsl_vector_float.h>
//#include <gsl/gsl_blas.h>

#ifdef _WIN32
# include <malloc.h> /* MSVC or mingw on windows */
#elif defined(__linux__) || defined(__APPLE__)
# include <alloca.h> /* linux, mac, mingw, cygwin */
#else
# include <stdlib.h> /* BSDs for example */
#endif

static t_class *mbc_lpc_class;

#define MAX_ORDER 200
#define NLOG2(x) (ceil(log2(x)))
#define POW2(x) (1 << x)
#define TWOPI M_PI * 2
#define DEFAULT_FS 44100
#define DEFAULT_FRAMERATE 100
#define DEFAULT_V_SIZE 64
#define DEFAULT_ORDER 32

typedef struct _DSPcomplex
{
  //gsl_vector_float* real;
  //gsl_vector_float* imag;
  t_float* fReal;
  t_float* fImag;

} t_DSPcomplex;

//#define REAL(z,i) gsl_vector_float_set(z,2*(i))
//#define IMAG(z,i) gsl_vector_float_setz(z,2*(i)+1)

typedef struct _lpc 
{
  t_object          x_obj;	           // the object itself (t_pxobject in MSP)
  t_float dummy;
  //t_float*          l_frame_buff;	   //input frame buffer
  //gsl_vector_float*          l_frame_buff;	   //input frame buffer
  //t_float*          l_winframe_buff;       //windowed input frame buffer
  //gsl_vector_float*          l_winframe_buff;       //windowed input frame buffer
  //t_float*	    l_outCoeff_buff;       //coefficient signal
  //gsl_vector_float*	    l_outCoeff_buff;       //coefficient signal
  //t_float*	    l_outParcor_buff; 	   //PARCOR coeffs
  //gsl_vector_float*	    l_outParcor_buff; 	   //PARCOR coeffs
  //t_float*          l_outError_buff;	   //error signal
  //gsl_vector_float*          l_outError_buff;	   //error signal
  //t_float*          l_win;	           //analysis window
  //gsl_vector_float*          l_win;	           //analysis window
  //t_float*	    l_R;
  //gsl_vector_float*	    l_R;
  /*-------------- non-GSL versions --------------*/

  t_float*  frameBuffer;
  t_float*  winFrameBuffer;
  t_float*  outCoeffBuffer;
  t_float*  parcorBuffer;
  t_float*  errorSigBuffer;
  t_float*  window;
  t_float*  lR;
  //  t_float*  vectorBuffer;
  
  double*	    l_W;
  double*	    l_E;
  double*	    l_K;
  double 	    l_G;
  double**	    l_A;
  double 	    l_x1;                  //last samples of pre-emphasis filter
  float 	    l_b1;	           //pre-emph coefficient
  int 	            l_order;	           //predictor order
  int 	            l_order_max;           //max order according to fs = order * frame_rate
  int 	            l_preemph;             //pre-epmhasis filter on/off
  int 	            l_frame_rate;          //analysis frame rate
  int 	            l_frame_size;          //analysis frame size, where fs = frame_rate * frame_size * 2
  int 	            l_hop_size;            //hop_size = frame_size * 2 (b/c of overlap)
  int 	            l_inframe_idx;         //current inframe buffer index
  int 	            l_outframe_idx;	   //current outframe buffer index
  long 	            l_v_size;	           //vector size
  float 	    l_fs;                  //sampling rate
  int 	            l_maxnfft;             //fft length
  int 	            l_log2nfft;	           //log2(fft length)
  int initOrder, initFramerate, initPreemph; // initialization values
  int j;

  //  FFTSetup          l_fftsetup;            //FFTSetup for vDSP FFT functions
  t_DSPcomplex      split;
  //  DSPSplitComplex   l_fftSplitTemp;        //temp split complex vector structure
} t_lpc;

t_int *lpc_perform(t_int *w) 
{
  t_lpc   *x          = (t_lpc *)  (w[1]);
  t_float *in         = (t_float *)(w[2]);
  t_float *out_error  = (t_float *)(w[3]);
  t_float *out_gain   = (t_float *)(w[4]);
  t_float *out_coeff  = (t_float *)(w[5]);
  t_float *out_parcor = (t_float *)(w[6]);
  t_float *out_index  = (t_float *)(w[7]);
  int n               = (int)      (w[8]);
  int p = x->l_order;
  int length = x->l_frame_size;
  int log2nfft = NLOG2(length+p+1);
  int nfft = POW2(log2nfft);
  int nfftO2 = nfft/2;
  t_float recipn = 1.0/nfft;
  int inframeidx = x->l_inframe_idx;
  int outframeidx = x->l_outframe_idx;
  
  int i, j, i1, ji;
  int in_idx = 0, out_idx = 0;
  double val;
  t_float scale, rCorr, re, im;
  //gsl_vector_float *vectorBuffer;
  //vectorBuffer = gsl_vector_float_calloc(MAX_ORDER);	   //input frame buffer
  //x->vectorBuffer = (t_float *) getzbytes( nfftO2 * sizeof(t_float));
  
  if (x->l_preemph)
    {
      while (n--) 
	{
	  val = in[in_idx];
	  in[in_idx] = val + x->l_b1 * x->l_x1;
	  x->l_x1 = val;
	  in_idx++;
	}
      n = (int)(w[8]);
      in_idx = 0;
    }
  
  while (n--) 
    {
      if (inframeidx < length) {
	//copy input into frame buff
	//also copy into winframe buff since GSL puts the result into one of the arg vectors

	//gsl_vector_float_set(&x->l_frame_buff,inframeidx,in[in_idx]);// = x->l_frame_buff[inframeidx] = in[in_idx];
	x->frameBuffer[inframeidx] = in[in_idx];
	
	out_gain[out_idx] = x->l_G;

	//out_error[out_idx] = gsl_vector_float_get(&x->l_outError_buff,outframeidx); //for now
	out_error[out_idx] = x->errorSigBuffer[outframeidx];
	
	if (outframeidx < x->l_order) {

	  //out_coeff[out_idx] = gsl_vector_float_get(&x->l_outCoeff_buff,outframeidx);
	  out_coeff[out_idx] = x->outCoeffBuffer[outframeidx];
	  
	  //out_parcor[out_idx] = gsl_vector_float_get(&x->l_outParcor_buff,outframeidx);
	  out_parcor[out_idx] = x->parcorBuffer[outframeidx];

	  out_index[out_idx] = outframeidx + 1;
	} else {
	  out_coeff[out_idx] = 0.0;
	  out_parcor[out_idx] = 0.0;
	  out_index[out_idx] = 0;
	}
	
	inframeidx++;
	in_idx++;
	outframeidx++;
	out_idx++;
      } else {

	//gsl_vector_float_memcpy(&x->l_winframe_buff,&x->l_frame_buff);
	//memcpy(&x->winFrameBuffer,&x->frameBuffer,x->l_maxnfft * sizeof(t_float));
	x->winFrameBuffer = copybytes(x->frameBuffer, x->l_maxnfft * sizeof(t_float));
	//perform durbin-levinson - for right now, just count to the order---------------
	//clear memory, is this necessary?
	for (i=0; i < p+1; i++){
	  x->lR[i] = 0.0;
	  x->l_W[i] = 0.0;
	  x->l_E[i] = 0.0;
	  x->l_K[i] = 0.0;			
	}
	for(i=0; i<=p; i++) {
	  for(j=0; j < p; j++) x->l_A[i][j] = 0.0;
	}
	//window frame buff
	//vDSP_vmul(x->l_frame_buff, 1, x->l_win, 1, x->l_winframe_buff, 1, length);
	//gsl_vector_float_mul(&x->l_winframe_buff,&x->l_win);
	for(i=0;i<x->l_maxnfft;i++)
	  {
	    x->winFrameBuffer[i] = x->winFrameBuffer[i] * x->window[i];
	  }

#ifdef DEBUG
	for(i=0;i<length;i++)post("\nwinframe(%d) = %g;",i+1,x->winFrameBuffer[i]);
#endif
	
	//create r from auto correlation
	if ((2*nfft*log2(nfft)+nfft) > length*p) { //NOTE: change this to update only when order is changed!
	  //time domain method
	  //for(i=0; i < p+1; i++) vDSP_dotpr(x->l_winframe_buff,1,x->l_winframe_buff+i,1,x->l_R+i,length-i);
	  //gsl_vector_float_memcpy(vectorBuffer,&x->l_winframe_buff);
	  for(i=0; i < p+1; i++)
	    {
	      //gsl_blas_sdot(&x->l_winframe_buff,&x->l_winframe_buff+i,&rCorr);
	      //gsl_vector_float_set(&x->l_R,i,rCorr);
	      for(x->j=0;x->j < length - i; x->j++) x->lR[i] = x->winFrameBuffer[j] * x->winFrameBuffer[j+i];
	    }
	} else {
	  //frequency domain method
	  // zero pad
	  //vDSP_vclr(x->l_winframe_buff+length,1,nfft-length);
	  for(i=length;i<nfft;i++)
	    {
	      //gsl_vector_float_set(&x->l_winframe_buff,i,0);
	      x->winFrameBuffer[i] = 0.0;
	    }
	  for(i=0;i<nfftO2;i++)
	  {
	    x->split.fReal[i] = x->winFrameBuffer[i*2];
	    x->split.fImag[i] = x->winFrameBuffer[i*2+1];
	      //convert to split complex vector
	      //gsl_vector_float_set(&x->split.real,i,gsl_vector_float_get(x->l_winframe_buff,i*2));
	      //gsl_vector_float_set(&x->split.imag,i,gsl_vector_float_get(x->l_winframe_buff,i*2+1));
	  }
	  //vDSP_ctoz( ( DSPComplex * ) x->l_winframe_buff, 2, &x->l_fftSplitTemp, 1, nfftO2);
	  //perform forward in place fft
	  //gsl_fft_complex_radix2_forward(&x->l_winframe_buff,1,nfft);
	  //EXTERN void mayer_fft(int n, t_sample *real, t_sample *imag);
	  //EXTERN void mayer_ifft(int n, t_sample *real, t_sample *imag);
	  mayer_fft(nfftO2,x->split.fReal,x->split.fImag);
	  //vDSP_fft_zrip(x->l_fftsetup, &x->l_fftSplitTemp, 1, log2nfft, FFT_FORWARD);
	  //scaling
	  scale = 0.5;

	  // Ed Kelly: scale real and imaginary parts and calculate magnitudes squared
	  for(i=0;i<nfftO2;i++)
	    {
	      j = i*2;
	      
	      //re = gsl_vector_float_get(&x->l_winframe_buff,j);
	      re = x->split.fReal[j];
	      //im = gsl_vector_float_get(&x->l_winframe_buff,j+1);
	      im = x->split.fImag[j];
	      re *= scale;
	      re = fabs(re * re);
	      im *= scale;
	      im = fabs(im * im);

	      //gsl_vector_float_set(&x->split.real,i,re+im);
	      //gsl_vector_float_set(&x->split.imag,i,0.0);
	      //gsl_vector_float_set(vectorBuffer,j,re+im);
	      x->split.fReal[j] = re+im;
	    }

	  //vDSP_vsmul(x->l_fftSplitTemp.realp,1,&scale,x->l_fftSplitTemp.realp,1,nfftO2);
	  //vDSP_vsmul(x->l_fftSplitTemp.imagp,1,&scale,x->l_fftSplitTemp.imagp,1,nfftO2);
	  //compute PSD
	  //vDSP_zvmags(&x->l_fftSplitTemp,1,x->l_fftSplitTemp.realp,1,nfftO2);

	  //clear imaginary part
	  for(i=0;i<nfftO2;i++)
	  {
	      //IMAG(&x->l_winframe_buff,i) = 0.0;
	      //gsl_vector_float_set(&x->l_winframe_buff,i*2+1,0.0);
	    x->split.fImag[i] = 0;
	  }
	  //vDSP_vclr(x->l_fftSplitTemp.imagp,1,nfftO2);
	  //perform inverse in place fft
	  //gsl_fft_complex_radix2_backward(vectorBuffer,1,nfft);
	  //vDSP_fft_zrip(x->l_fftsetup, &x->l_fftSplitTemp, 1, log2nfft, FFT_INVERSE);

	  mayer_ifft(nfftO2,x->split.fReal,x->split.fImag);
	  
	  //scaling
	  //vDSP_vsmul(x->l_fftSplitTemp.realp,1,&recipn,x->l_fftSplitTemp.realp,1,nfftO2);
	  //vDSP_vsmul(x->l_fftSplitTemp.imagp,1,&recipn,x->l_fftSplitTemp.imagp,1,nfftO2);
	  //gsl_vector_float_scale(vectorBuffer,&recipn);

	  for(i=0;i<nfftO2;i++)
	    {
	      x->lR[i*2] = x->split.fReal[i] * recipn;
	      x->lR[i*2+1] = x->split.fImag[i] * recipn;
	    }
	  
	  //convert back to real number vector
	  //vDSP_ztoc(&x->l_fftSplitTemp, 1, (DSPComplex *)x->l_R, 2, nfftO2);
	  //gsl_vector_float_memcpy(x->l_R,vectorBuffer);
	}
	
	
	/*for(i=0; i < p+1; i++) {
	  x->l_R[i] = 0.0;
	  for(j=0; j < length - i; j++) {
	  x->l_R[i] += x->l_winframe_buff[j] * x->l_winframe_buff[i+j];
	  //x->l_R[i] += x->l_win[j] * x->l_win[i+j];
	  }
	  }*/
#ifdef DEBUG
	for(i=0;i< p+1; i++) post("\nR(%d) = %f;",i+1,x->lR[i]);
#endif
	
	//x->l_W[0] = gsl_vector_float_get(x->l_R,1);
	//x->l_E[0] = gsl_vector_float_get(x->l_R,0);
	x->l_W[0] = x->lR[1];
	x->l_E[0] = x->lR[0];
	
	for (i = 1; i <= p; i++) {
	  x->l_K[i] = x->l_W[i-1] / x->l_E[i-1];
	  
	  x->l_A[i][i] = x->l_K[i];
	  i1 = i - 1;
	  if (i1 >= 1) {
	    for (j = 1; j <=i1; j++) {
	      ji = i - j;
	      x->l_A[j][i] = x->l_A[j][i1] - x->l_K[i] * x->l_A[ji][i1];
	    }				
	  }
	  
	  x->l_E[i] = x->l_E[i-1] * (1.0 - x->l_K[i] * x->l_K[i]);
	  
	  if (i != p) {
	    x->l_W[i] = x->lR[i+1];
	    for (j = 1; j <= i; j++) {
	      x->l_W[i] -= x->l_A[j][i] * x->lR[i-j+1];
	    }
	  }
	}
	
	x->l_G = sqrt(x->l_E[p]);
	for (i=0; i < p; i++) {
	  x->outCoeffBuffer[i] = (t_float)(x->l_A[i+1][p]);
	  x->parcorBuffer[i] = (t_float)(x->l_K[i+1]);
	  //#ifdef DEBUG
	  //post("\nParcor(%d) = %g;",i+1,x->l_K[i+1]);
	  //post("\nCoeff(%d) = %g;",i+1,x->l_A[i+1][p]);
	  //#endif
	}
	
	//--------------------------------------------------------------------------------
	
	//copy right side to left side, move delayed input to output
	for (i=0; i < x->l_hop_size; i++) {
	  x->errorSigBuffer[i] = x->frameBuffer[i];
	  x->frameBuffer[i] = x->frameBuffer[i + x->l_hop_size];
	}
	
	inframeidx = x->l_hop_size;
	outframeidx = 0;
	n++; //to avoid skipping a sample (since we already decremented
	while (n--) {
	  x->frameBuffer[inframeidx] = in[in_idx];
				
	  out_gain[out_idx] = (t_float)(x->l_G);
	  out_error[out_idx] = x->errorSigBuffer[outframeidx]; //for now
	  if (outframeidx < x->l_order) {
	    out_coeff[out_idx] = x->outCoeffBuffer[outframeidx];
	    out_parcor[out_idx] = x->parcorBuffer[outframeidx];
	    out_index[out_idx] = (t_float)(outframeidx + 1);
	  } else {
	    out_coeff[out_idx] = 0.0;
	    out_parcor[out_idx] = 0.0;
	    out_index[out_idx] = 0;
	  }
	  
	  inframeidx++;
	  in_idx++;
	  outframeidx++;
	  out_idx++;
	}
	break;
      }
    }
  
  x->l_inframe_idx = inframeidx;
  x->l_outframe_idx = outframeidx;
  
  return (w+9);
}

void lpc_hanning(t_lpc *x)
{
  t_float N = (t_float)x->l_frame_size;
  int n;
	
  for (n=0; n<(int)N; n++)
    {
      x->window[n] = 0.5 * (1.0 - cos((TWOPI*((t_float)n+1))/(N+1)));
    }
  post("Generating Hanning window for LPC analysis");  
}

void lpc_hamming(t_lpc *x)
{
  t_float N = (t_float)x->l_frame_size;
  int n;
	
  for (n=0; n<(int)N; n++)
    {
      x->window[n] = (0.54 - 0.46*cos((TWOPI*(t_float)n)/(N-1)));
    }
  post("Generating Hamming window for LPC analysis");
}

void lpc_bartlett(t_lpc *x)
{
  t_float N = (t_float)x->l_frame_size;
  int n;

  for (n=0; n<(int)N; n++)
    {
      x->window[n] = 2.0/(N - 1) * ((N-1)/2 - fabs((t_float)n - (N-1)/2.0));
    }
  post("Generating Bartlett window for LPC analysis");
}

void lpc_preemph(t_lpc *x, t_floatarg n)
{
  x->l_preemph = n != 0 ? 1 : 0;
}

void lpc_init(t_lpc *x) {
	int i;
	
	if (x->l_fs < (t_float)x->l_frame_rate * (t_float)x->l_order) {
	  x->l_frame_rate = (long)(x->l_fs / (t_float)x->l_order);
		error("mbc.lpc~: framerate * order must be less than or equal to sampling rate.  framerate has been changed to %d", x->l_frame_rate);
	}
	
	x->l_hop_size = (int)(x->l_fs / (t_float)x->l_frame_rate);
	
	if (x->l_v_size > x->l_hop_size) {
		error("mbc.lpc~: warning: frame_size is less than vector size.  this may cause errors, please reduce the frame rate or increase the vector size");
	}
	
	x->l_order_max = x->l_fs / (t_float)x->l_frame_rate;
	x->l_frame_size = x->l_hop_size * 2;
	x->l_inframe_idx = x->l_hop_size;
	x->l_outframe_idx = 0;
	x->l_log2nfft = (int) NLOG2(x->l_frame_size+MAX_ORDER+1);
	x->l_maxnfft = POW2(x->l_log2nfft);
	
	// free memory if needed
	lpc_free_arrays(x);
	
	//allocate memory
	//x->l_frame_buff = (t_float *) getzbytes( x->l_frame_size * sizeof(t_float));
	//x->l_frame_buff = gsl_vector_float_calloc(x->l_frame_size);
	x->frameBuffer = (t_float *) getzbytes( x->l_frame_size * sizeof(t_float));
	x->winFrameBuffer = (t_float *) getzbytes( x->l_maxnfft * sizeof(t_float));
	x->outCoeffBuffer = (t_float *) getzbytes(MAX_ORDER * sizeof(t_float));
	x->parcorBuffer = (t_float *) getzbytes(MAX_ORDER * sizeof(t_float));
	x->errorSigBuffer = (t_float *) getzbytes( x->l_hop_size * sizeof(t_float));
	x->window = (t_float *) getzbytes( x->l_frame_size * sizeof(t_float));
	x->lR = (t_float *) getzbytes( x->l_maxnfft * sizeof(t_float));
	x->l_W = (double *) getzbytes( (MAX_ORDER + 1) * sizeof(double));
	x->l_E = (double *) getzbytes( (MAX_ORDER + 1) * sizeof(double));
	x->l_K = (double *) getzbytes( (MAX_ORDER + 1) * sizeof(double));
	x->l_A = (double **) getzbytes( (MAX_ORDER + 1) * sizeof(double*));
	for(i=0; i<MAX_ORDER; i++) {
		x->l_A[i] = (double *)getzbytes( (MAX_ORDER + 1) * sizeof(double));
	}
	x->split.fReal = (float *)getzbytes( (x->l_maxnfft / 2) * sizeof(float));
	x->split.fImag = (float *)getzbytes( (x->l_maxnfft / 2) * sizeof(float));
		
	x->l_x1 = 0;
	x->l_G = 0.0;
	
	x->l_b1 = -0.98;
	
	//calculate window
	lpc_hamming(x);
	//create FFTsetups
	//x->l_fftsetup = vDSP_create_fftsetup(x->l_log2nfft,FFT_RADIX2);
	//if (!x->l_fftsetup) error("mbc.lpc~: not enough available memory");
	
}

void lpc_order(t_lpc *x, t_floatarg n)
{
  if (x->l_order_max < (int)n)
    {
      error("mbc.lpc~: framerate * order must be less than or equal to sampling rate.  At the current setting maxorder is %d.  For a higher order, lower the framerate.", x->l_order_max);
      x->l_order = x->l_order_max;
    }
  else
    {
      x->l_order = (int)n > 4 ? (int)n : 4;
    }
  x->l_frame_rate = x->l_fs / x->l_order;
  lpc_init(x);  
    //lpc_init(x) ?
}

void lpc_free_arrays(t_lpc *x)
{
  //int i;
	
  /*gsl_vector_float_free(x->l_frame_buff);
  x->l_frame_buff = NULL;
  gsl_vector_float_free(x->l_winframe_buff);
  x->l_winframe_buff = NULL;
  gsl_vector_float_free(x->l_outCoeff_buff);
  x->l_outCoeff_buff = NULL;
  gsl_vector_float_free(x->l_outError_buff);
  x->l_outError_buff = NULL;
  gsl_vector_float_free(x->l_outParcor_buff);
  x->l_outParcor_buff = NULL;
  gsl_vector_float_free(x->l_win);
  x->l_win = NULL;
  gsl_vector_float_free(x->l_R);
  x->l_R = NULL;*/
  //x->frameBuffer = (t_float *) getzbytes( x->l_frame_size * sizeof(t_float));
  //	x->winFrameBuffer = (t_float *) getzbytes( x->l_maxnfft * sizeof(t_float));
  //	x->OutCoeffBuffer = (t_float *) getzbytes(MAX_ORDER * sizeof(t_float));
  //	x->parcorBuffer = (t_float *) getzbytes(MAX_ORDER * sizeof(t_float));
  //	x->errorSigBuffer = (t_float *) getzbytes( x->l_hop_size * sizeof(t_float));
  //	x->window = (t_float *) getzbytes( x->l_frame_size * sizeof(t_float));
  freebytes(x->frameBuffer, x->l_frame_size * sizeof(t_float));
  freebytes(x->winFrameBuffer, x->l_maxnfft * sizeof(t_float));
  freebytes(x->outCoeffBuffer, MAX_ORDER * sizeof(t_float));
  freebytes(x->parcorBuffer, MAX_ORDER * sizeof(t_float));
  freebytes(x->errorSigBuffer, x->l_hop_size * sizeof(t_float));
  freebytes(x->window, x->l_frame_size * sizeof(t_float));
  freebytes(x->lR, x->l_maxnfft * sizeof(t_float));
  x->frameBuffer = NULL;
  x->winFrameBuffer = NULL;
  x->outCoeffBuffer = NULL;
  x->parcorBuffer = NULL;
  x->errorSigBuffer = NULL;
  x->window = NULL;
  x->lR = NULL;
  
  if (x->l_W) {
    freebytes(x->l_W, (MAX_ORDER + 1) * sizeof(double));
    x->l_W = NULL;
  }
  if (x->l_E) {
    freebytes(x->l_E, (MAX_ORDER + 1) * sizeof(double));
    x->l_E = NULL;
  }
  if (x->l_K) {
    freebytes(x->l_K, (MAX_ORDER + 1) * sizeof(double));
    x->l_K = NULL;
  }
  if (x->l_A) {		
    freebytes(x->l_A, (MAX_ORDER + 1) * sizeof(double));
    x->l_A = NULL;
  }
  /*	if (x->l_fftSplitTemp.realp) {
	sysmem_freeptr(x->l_fftSplitTemp.realp);
	x->l_fftSplitTemp.realp = NULL;
	}
	if (x->l_fftSplitTemp.imagp) {
	sysmem_freeptr(x->l_fftSplitTemp.imagp);
	x->l_fftSplitTemp.imagp = NULL;
	}
	
	if (x->l_fftsetup) {
	vDSP_destroy_fftsetup(x->l_fftsetup);
	x->l_fftsetup = NULL;
	}*/
  freebytes(x->split.fReal, (x->l_maxnfft / 2) * sizeof(t_float));
  freebytes(x->split.fImag, (x->l_maxnfft / 2) * sizeof(t_float));
  x->split.fReal = NULL;
  x->split.fImag = NULL;
}

void lpc_window(t_lpc *x, t_floatarg f)
{
  if(f == 1) lpc_bartlett(x);
  else if(f == 2) lpc_hanning(x);
  else lpc_hamming(x);
}

void lpc_dsp(t_lpc *x, t_signal **sp)
{
  x->l_fs = sys_getsr();
  if(sp[0]->s_n != x->l_v_size) x->l_v_size = sp[0]->s_n;
  lpc_init(x);
  //  dsp_add(lpc_blit_perform, 4, x,
  //  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

  dsp_add(lpc_perform, 8, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[6]->s_vec, sp[0]->s_n);
}

void *lpc_new(int argc, t_atom *argv)
{
  t_lpc *x = (t_lpc *)pd_new(mbc_lpc_class);

  //#define DEFAULT_FS 44100
  //#define DEFAULT_FRAMERATE 100
  //#define DEFAULT_V_SIZE 64
  //#define DEFAULT_ORDER 32
  /*if(argc >= 3)
    {
      x->initOrder = atom_getfloat(argv);
      x->initFramerate = atom_getfloat(argv+1);
      x->initPreemph = atom_getfloat(argv+2);
      }
      else
      if(argc == 2)*/
  if(argc >= 2)
    {
      x->initOrder = atom_getfloat(argv);
      //x->initFramerate = atom_getfloat(argv+1);
      x->initPreemph = atom_getfloat(argv+1);
    }
  else if(argc == 1)
    {
      x->initOrder = atom_getfloat(argv);
      //x->initFramerate = DEFAULT_FRAMERATE;
      x->initPreemph = 0;
    }
  else
    {
      x->initOrder = DEFAULT_ORDER;
      //x->initFramerate = DEFAULT_FRAMERATE;
      x->initPreemph = 0;
    }
      
  if(x->initOrder < 1)
    {
      post("Order must be from 1 to 200! Setting to %d",DEFAULT_ORDER);
      x->l_order = DEFAULT_ORDER;
    }
  else if(x->initOrder > MAX_ORDER)
    {
      post("Order cannot be greater than %d!",MAX_ORDER);
      x->l_order = MAX_ORDER;
    }
  else
    {
      post("Setting order to %d",x->initOrder);
      x->l_order = x->initOrder;
    }
  x->l_fs = sys_getsr();
  x->l_frame_rate = (int)(x->l_fs / (t_float)x->l_order);
  lpc_init(x);  
  //int length = x->l_frame_size;
  //int log2nfft = NLOG2(length+p+1);
  //int nfft = POW2(log2nfft);
  //int nfftO2 = nfft/2;
    //MAX_ORDER is wrong - check fft size calculation!
  /*  x->l_frame_buff = gsl_vector_float_calloc(MAX_ORDER);	   //input frame buffer
  x->l_winframe_buff = gsl_vector_float_calloc(MAX_ORDER);       //windowed input frame buffer
  x->l_outCoeff_buff = gsl_vector_float_calloc(MAX_ORDER);       //coefficient signal
  x->l_outParcor_buff = gsl_vector_float_calloc(MAX_ORDER); 	   //PARCOR coeffs
  x->l_outError_buff = gsl_vector_float_calloc(MAX_ORDER);	   //error signal
  x->l_win = gsl_vector_float_calloc(MAX_ORDER);	           //analysis window
  */
  inlet_new (&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  //floatinlet_new (&x->x_obj, &x->f_win);
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
}

void lpc_setup(void)
{
  mbc_lpc_class = class_new(gensym("mbc_lpc~"), 
  (t_newmethod)lpc_new, 
  0, sizeof(t_lpc),
  CLASS_DEFAULT, A_DEFFLOAT, 0);

  post("->mbc_lpc~");
  post("->by Mark Cartwright");
  post("->Pd port by Edward Kelly");
  post("->2018");

  class_addmethod(mbc_lpc_class,
  (t_method)lpc_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(mbc_lpc_class, t_lpc, dummy);
  class_addmethod(mbc_lpc_class, (t_method)lpc_order, gensym("order"), A_DEFFLOAT, 12);
  class_addmethod(mbc_lpc_class, (t_method)lpc_preemph, gensym("preemph"), A_DEFFLOAT, 0);
}
