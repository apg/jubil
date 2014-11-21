#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>

#include "jubil.h"

static void
print_atom(j_t *J, j_obj_t *a)
{
  int i;
  switch (a->flags) {
  case J_BOOL_T:
    fputs(a->fix == 0 ? "false": "true", J->out);
  case J_FIX_T:
    fprintf(J->out, "%ld", a->fix);
    break;
  case J_FLO_T:
    fprintf(J->out, "%lf", a->flo);
    break;
  case J_SYM_T:
    fputs(a->str, J->out);
    break;
  case J_STR_T:
    fputc('"', J->out);
    for (i = 0; i < a->str_sz; i++) {
      switch (a->str[i]) {
      case '"':
        fputs("\\\"", J->out);
        break;
      default:
        fputc(a->str[i], J->out);
      }
    }
    fputc('"', J->out);
    break;
  }
}

static void
print_list(j_t *J, j_obj_t *a)
{
  j_obj_t *obj;

  if (a == J->Nil) {
    fputs("nil", J->out);
    return;
  }
  fputc('(', J->out);
  for (obj = a->head; obj; ) {
    j_write(J, obj);

    obj = a->tail;
    if (obj && obj->flags == J_LIST_T) {
      if (obj == J->Nil) {
        break;
      }
      obj = a->head;
      fputc(' ', J->out);
    }
    else if (obj) {
      fputc(' ', J->out);
      j_write(J, obj);
      obj = NULL;
    }
  }
  fputc(')', J->out);
}

void
j_write(j_t *J, j_obj_t *o)
{
  switch (o->flags) {
  case J_BOOL_T:
  case J_FIX_T:
  case J_FLO_T:
  case J_STR_T:
  case J_SYM_T:
    print_atom(J, o);
    break;
  case J_LIST_T:
    print_list(J, o);
    break;
  case J_USR_T:
    fputs("<# ", J->out);
    print_atom(J, o->uname);
    print_list(J, o->ubody);
    fputs(">", J->out);
    break;
  case J_PRIM_T:
    fputs("<#Primitive: ", J->out);
    print_atom(J, o->pname);
    break;
  default:
    fprintf(stderr, "Invalid object! Aborting\n");
    exit(1);
  }
}
