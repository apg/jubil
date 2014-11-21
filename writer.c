#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>

#include "jubil.h"

static void
print_atom(j_t *J, FILE *out, j_obj_t *a)
{
  int i;
  switch (a->flags) {
  case J_BOOL_T:
    fputs(a->fix == 0 ? "false": "true", out);
  case J_FIX_T:
    fprintf(out, "%ld", a->fix);
    break;
  case J_FLO_T:
    fprintf(out, "%lf", a->flo);
    break;
  case J_SYM_T:
    fputs(a->str, out);
    break;
  case J_STR_T:
    fputc('"', out);
    for (i = 0; i < a->str_sz; i++) {
      switch (a->str[i]) {
      case '"':
        fputc('\\', out);
        fputc('"', out);
        break;
      default:
        fputc(a->str[i], out);
      }
    }
    fputc('"', out);
    break;
  }
}

static void
print_list(j_t *J, FILE *out, j_obj_t *a)
{
  j_obj_t *obj;

  if (a == J->Nil) {
    fprintf(out, "nil");
    return;
  }
  fputc('(', out);
  for (obj = a->head; obj; ) {
    j_write(J, out, obj);

    obj = a->tail;
    if (obj && obj->flags == J_LIST_T) {
      if (obj == J->Nil) {
        break;
      }
      obj = a->head;
      fputc(' ', out);
    }
    else if (obj) {
      fputc(' ', out);
      j_write(J, out, obj);
      obj = NULL;
    }
  }
  fputc(')', out);
}

void
j_write(j_t *J, FILE *out, j_obj_t *o)
{
  switch (o->flags) {
  case J_BOOL_T:
  case J_FIX_T:
  case J_FLO_T:
  case J_STR_T:
  case J_SYM_T:
    print_atom(J, out, o);
    break;
  case J_LIST_T:
    print_list(J, out, o);
    break;
  case J_USR_T:
    fputs("<# ", out);
    print_atom(J, out, o->uname);
    print_list(J, out, o->ubody);
    fputs(">", out);
    break;
  case J_PRIM_T:
    fputs("<#Primitive: ", out);
    print_atom(J, out, o->pname);
    break;
  default:
    fprintf(stderr, "Invalid object! Aborting\n");
    exit(1);
  }
}
