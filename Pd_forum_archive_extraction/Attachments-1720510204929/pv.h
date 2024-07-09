#ifndef __PV_H
#define __PV_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <fast.h>
#include <stereotypes.h>
#include <optim-math.h>

#include <vecLib/vecLib.h>

float synt;

/* linux uses PI and TWOPI (the crude nerve) already */

/*
float PI;
float TWOPI;
*/

float myPI;
float myTWOPI;

typedef struct {
  float real;
  float imag;
} complex;

#define CABS(x) fsqrtd( (x).real*(x).real + (x).imag*(x).imag )

complex cadd(), csub(), cmult(), smult(), cdiv(), conjg(), csqrt();

extern int shiftin();
extern int turnin();
extern int rfshiftin();
extern int rsshiftin();
extern int readin();
extern int readsnd();

extern int fshiftin();
extern int fshiftout();

extern void makewindows();
extern void makehanning();
extern void fold();
extern void overlapadd();
extern void resonoverlap();
extern void shiftout();
extern void resonshift();
extern void oscbank();
extern void noscbank();
extern void convert();
extern void unconvert();
extern void in_unconvert();
extern void leanconvert();
extern void leanunconvert();
extern void faxcompand();
extern void daxcompand();
extern void scompand();
extern void norman();
extern void shape();


extern void vfold();
extern void voverlapadd();
extern void vconvert();
extern void vunconvert();
extern void vleanconvert();
extern void vleanunconvert();
extern void vmagnitude();

extern void voscbank();

extern void vshape();
extern void vshapefreq();

extern int isVectorCpu();

extern void sconvert();
extern void sunconvert();
extern void sleanconvert();
extern void sleanunconvert();
extern void smagnitude();


extern Bound extrema();
extern Bound nextrema();

/* fft function declarations */

extern void init_rdft();
extern void rdft();

extern void rfft();
extern void cfft();

#define FORWARD 1
#define INVERSE 0

#endif
