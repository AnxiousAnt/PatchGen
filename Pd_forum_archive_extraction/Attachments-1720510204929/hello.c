#include "m_pd.h"
#include <stdio.h>

static t_class *hello_class;

typedef struct _hello{
  t_object  x_obj;
} t_hello;

void hello_bang(t_hello *x)
{
  FILE *fil;

  fil=fopen("any","rb");
  post("File any opened");
  fclose(fil);
}

void *hello_new(void)
{
  t_hello *x = (t_hello *)pd_new(hello_class);

  return (void *)x;
}

void hello_setup(void) {
  hello_class = class_new(gensym("hello"),
        (t_newmethod)hello_new,
        0, sizeof(t_hello),
        CLASS_DEFAULT, 0);
  class_addbang(hello_class, hello_bang);
}

