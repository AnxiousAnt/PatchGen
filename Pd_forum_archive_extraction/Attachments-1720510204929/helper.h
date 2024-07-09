#ifndef __HELPER_H
#define __HELPER_H

#include <crack.h>
#include <myrand.h>
#include <expr.h>
#include <stereotypes.h>

extern char crack();
extern void *space();
extern void *respace();
extern void *zerospace();
extern void freespace();
extern void *nsspace();
extern void *nszerospace();
extern void nsfreespace();
extern int usage();

extern void srrand();
extern float rrand();
extern float prand();

extern float interptabl();
extern float looktabl();
extern void extrematabl();
extern void rescaletabl();
extern void morphtabl();
extern void dbmorphtabl();

extern float sfexpr();

extern void stripheader();

extern Tabl *loadtabl();

extern void reverse2();
extern void reverse4();

int SNDReadHeader();


#endif
