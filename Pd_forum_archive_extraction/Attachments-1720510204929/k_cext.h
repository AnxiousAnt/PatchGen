/*
 *   k_cext.h copyright 2002-2003 Kjetil S. Matheussen.
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef _MSC_VER
typedef int bool;
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif
#include <stdarg.h>

typedef struct k_cext
{
  t_object x_ob;
  t_float *values;

  int num_ins;
  int num_outs;

  t_inlet **inlets;
  t_outlet **outlets;

  void (*k_cext_process)(struct k_cext *x);
  void *handle;

  char filename[40];

  bool iscext;

  void *userdata; // This attribute /can/ be used by a patch, but using "static" is much cleaner, so please don't.
} t_k_cext;



/* The following functions are used by intsort and floatsort */
extern int k_cext_intcompare(const void *p1, const void *p2);
extern int k_cext_floatcompare(const void *p1, const void *p2);


/* The following functions are system dependant, and called internally from k_cext only.
   All ports must implement these functions.
 */

extern int k_sys_getprocessfunction(t_k_cext *x,char *funcname,char *name);
extern void k_sys_freehandle(t_k_cext *x);
extern void k_sys_mktempfilename(char *to);
extern void k_sys_writeincludes(FILE *file);
extern void k_sys_makecompilestring(char *to,char *name,char *funcname);
extern void k_sys_deletefile(char *name);
extern void k_sys_init(void);



#define V(a) (x->values[a])
#define I(a) ((int)(x->values[a]))


#define O(a,b) outlet_float(x->outlets[a],b)

#define O0(b) O(0,b)
#define O1(b) O(1,b)
#define O2(b) O(2,b)
#define O3(b) O(3,b)
#define O4(b) O(4,b)
#define O5(b) O(5,b)
#define O6(b) O(6,b)

#define BETWEEN(dasmin,dasmax) ((dasmin) + (((float)(dasmax-(dasmin)))*rand())/(RAND_MAX+1.0))
#define RANDOM(dasmax) BETWEEN(0,dasmax)

#define SEND(symname,val) \
do{ \
  static t_symbol *k_cext_internal_symbol=NULL; \
  if(k_cext_internal_symbol==NULL) k_cext_internal_symbol=gensym(symname); \
  if(k_cext_internal_symbol->s_thing) pd_float(k_cext_internal_symbol->s_thing, val);  \
}while(0)



#define INTARRAY(name,len) int name[len]={0}
#define FLOATARRAY(name,len) t_float name[len]={0.0f}

#define INTSORT(a,b) qsort((void *)(a),b, sizeof (int), k_cext_intcompare);
#define FLOATSORT(a,b) qsort((void *)(a),b, sizeof (float), k_cext_floatcompare);


#define IF if(
#define FOR for(
#define RANGE(a,b,c) for(a=b;a<c;a++)
#define WHILE while(
#define SWITCH switch(

#define THEN )
#define BEGIN {

#define LOOP for(;;){

#define ELIF }else if(

/* If you think "END ELSE BEGIN" is more natural, just write "END else BEGIN". */
#define ELSE }else{

#define END }
#define ENDFOR END
#define ENDRANGE END
#define ENDIF END
#define ENDWHILE END
#define ENDLOOP END
#define ENDSWITCH END

#define SC ;

#define NL "\n"
  
#define SP " "


typedef int (*k_cext_f_int_callback)(t_k_cext *x,...);
typedef float (*k_cext_f_float_callback)(t_k_cext *x,...);

/*
// T.Grill - static doesn't work with [] !
static k_cext_f_int_callback *k_cext_int_funcs[];
static k_cext_f_float_callback *k_cext_float_funcs[];
static t_k_cext **k_cext_int_x[];
static t_k_cext **k_cext_float_x[];
*/

bool k_cext_get_int_funcs(k_cext_f_int_callback **funcs,t_k_cext **xs[],int length,...);
bool k_cext_get_float_funcs(k_cext_f_float_callback **funcs,t_k_cext **xs[],int length,...);

//#define F_INT(a,...) (*k_cext_int_funcs[a])(*k_cext_int_x[a],__VA_ARGS__)
#define F_INT_1(a,p1) (*k_cext_int_funcs[a])(*k_cext_int_x[a],p1)
#define F_INT_2(a,p1,p2) (*k_cext_int_funcs[a])(*k_cext_int_x[a],p1,p2)
#define F_INT_3(a,p1,p2,p3) (*k_cext_int_funcs[a])(*k_cext_int_x[a],p1,p2,p3)
#define F2_INT(a) (*k_cext_int_funcs[a])(*k_cext_int_x[a])

//#define F_FLOAT(a,...) (*k_cext_float_funcs[a])(*k_cext_float_x[a],__VA_ARGS__)
#define F_FLOAT_1(a,p1) (*k_cext_float_funcs[a])(*k_cext_float_x[a],p1)
#define F_FLOAT_2(a,p1,p2) (*k_cext_float_funcs[a])(*k_cext_float_x[a],p1,p2)
#define F_FLOAT_3(a,p1,p2,p3) (*k_cext_float_funcs[a])(*k_cext_float_x[a],p1,p2,p3)
#define F2_FLOAT(a) (*k_cext_float_funcs[a])(*k_cext_float_x[a])
