/*        wmed~   Charles Z Henry 12/6/06
          implements a weighted median filter 
          based on a list of integers as arguments  */


#include "m_pd.h"
#include "stdlib.h"
#include "stdio.h"

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ wmed~ ----------------------------- */

static t_class *wmed_tilde_class;

typedef struct _wmed_tilde
{
    t_object x_obj; 	/* obligatory header */
    t_sample x_f;    	/* place to hold inlet's value if it's set by message */
    int num_args;
    int *weights;
    int median_index;
    t_sample *buffer;
} t_wmed_tilde;

static t_int *wmed_tilde_perform(t_int *w)
{
    t_wmed_tilde *x = (t_wmed_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n=(int) (w[4]);
    int i=0;
    float *input=getbytes(sizeof(float) * n);
    for (i=0; i < n; i++)
      input[i]=in[i];
    int j=0;
    int k=0;
    int l=0;
    int tempi=0;
    int tempi2=0;
    float temp, temp2=0.0;


    int *check=getbytes(sizeof(int) * x->num_args);
    for (i=1; i < x->num_args; i++)
    {
      // first we handle the part which overlaps with the buffer
      // initializing here:
      l=0;
      for (j=0; j < x->num_args; j++)
        check[j]=1;
      // running here:
      while (l < x->median_index)
      {
        // A***A  we find the first entry which is not checked
        k=i;
        temp=x->buffer[i];
        tempi=i;
        while(!check[(k++)-i])
          if (k < x->num_args)
          {
            temp=x->buffer[k];
            tempi=k-i;
          }
          else
          {
            temp=input[k-x->num_args];
            tempi=k-i;
          }
        // result: temp is the first unchecked entry and k is the index after that

        // B***B we find the lowest value and set it to temp, tempi=index in (0, num_args-1)

        for (j=k; j < (x->num_args+i); j++)
        {
          if (check[j-i])
            if (j < x->num_args)
            {
              if (x->buffer[j] < temp)
              {
                temp=x->buffer[j];
                tempi=j-i;
              }
            }
            else
            {
              if (input[j-x->num_args] < temp)
              {
                temp=input[j-x->num_args];
                tempi=j-i;
              }
            }
        }

        // C***C we set check[tempi]=0, temp2 = temp, and l+=weights[tempi]
        check[tempi]=0;
        temp2=temp;
        l+=x->weights[tempi];
      }
      out[i-1]=temp2;
    }

    tempi=n-x->num_args+1;
    for (i=0; i < tempi; i++)
    {
      l=0;
      for (j=0; j < x->num_args; j++)
        check[j]=1;
      // running here:
      while (l < x->median_index)
      {
        // A***A  we find the first entry which is not checked
        k=i;
        temp=input[i];
        while(!check[(k++)-i])
        {
          temp=input[k];
          tempi2=k-i;
        }
        // result: temp is the first unchecked entry and k is the index after that

        // B***B we find the lowest value and set it to temp, tempi=index in (0, num_args-1)

        for (j=k; j < (x->num_args+i); j++)
        {
          if (check[j-i])
            if (input[j] < temp)
            {
              temp=input[j];
              tempi2=j-i;
            }
        }

        // C***C we set check[tempi]=0, temp2 = temp, and l+=weights[tempi]
        check[tempi2]=0;
        temp2=temp;
        l+=x->weights[tempi2];
      }
      out[i+x->num_args-1]=temp2;
      //printf("%i %f %f %f %f\n", l, input[i+x->num_args-1], temp, temp2, out[i+x->num_args-1]);
    }

    for(i=0; i<x->num_args; i++)
      x->buffer[i]=input[n - x->num_args + i];
    freebytes(check, sizeof(int) * x->num_args);
    freebytes(input, sizeof(float) * n);
    return (w+5);
}
static void wmed_tilde_dsp(t_wmed_tilde *x, t_signal **sp)
{
    dsp_add(wmed_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void wmed_tilde_free(t_wmed_tilde *x)
{
  freebytes(x->buffer, x->num_args*sizeof(t_sample));
  freebytes(x->weights, x->num_args*sizeof(t_int));
}

static void *wmed_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    int i;
    t_wmed_tilde *x = (t_wmed_tilde *) pd_new(wmed_tilde_class);
    x->x_f = 0.0;
    outlet_new(&x->x_obj, gensym("signal"));
    x->num_args=argc;
    x->buffer=(t_sample *) getbytes(argc*sizeof(t_sample));
    x->weights=(int *) getbytes(argc*sizeof(int));
    printf("%i arguments\n", argc);
    for(i=0; i<argc; i++)
    {
      x->weights[i]=atom_getfloatarg(i, argc, argv);
      printf("%i ", x->weights[i]);
      x->buffer[i]=0.0;
    }
    int list_length=0;
    for (i=0; i<x->num_args; i++)
      list_length+=x->weights[i];
    x->median_index=list_length/2+list_length%2;
    printf("\n");
    return(x);
}
void wmed_tilde_setup(void)
{
    wmed_tilde_class = class_new(gensym("wmed~"), (t_newmethod)wmed_tilde_new, (t_method)wmed_tilde_free,
    	sizeof(t_wmed_tilde), 0, A_GIMME, 0);
    CLASS_MAINSIGNALIN(wmed_tilde_class, t_wmed_tilde, x_f);
    class_addmethod(wmed_tilde_class, (t_method)wmed_tilde_dsp, gensym("dsp"), 0);
}
