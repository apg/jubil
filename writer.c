#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>

#include "jubil.h"

static void
print_atom(jubil *J, jubil_value a)
{
  int i;
  switch (a.flags) {
  case JUBIL_T_BOOL:
    fputs(a->fix == 0 ? "false": "true", J->out);
    break;
  case JUBIL_T_FIX:
    fprintf(J->out, "%ld", a.fix);
    break;
  case JUBIL_T_FLO:
    fprintf(J->out, "%lf", a.flo);
    break;
  case JUBIL_T_SYM:
    fputs(a.object->str, J->out);
    break;
  case JUBIL_T_STR:
    fputc('"', J->out);
    for (i = 0; i < a.object->str_sz; i++) {
      switch (a.object->str[i]) {
      case '"':
        fputs("\\\"", J->out);
        break;
      default:
        fputc(a.object->str[i], J->out);
      }
    }
    fputc('"', J->out);
    break;
  }
}

static void
print_list(jubil *J, jubil_value a)
{
  jubil_value obj;

  if (a == J->Nil) {
    fputs("nil", J->out);
    return;
  }
  fputc('(', J->out);
  for (obj = a.object->cons->head; obj; ) {
    j_write(J, obj);

    obj = a.object->tail;
    if (obj.flags == JUBIL_T_LIST) {
      if (obj == J->Nil) {
        break;
      }
      obj = a.object->head;
      fputc(' ', J->out);
    }
    else {
      fputc(' ', J->out);
      j_write(J, obj);
      obj = NULL;
    }
  }
  fputc(')', J->out);
}

void
j_write(jubil *J, jubil_value o)
{
  switch (o->flags) {
  case JUBIL_T_BOOL:
  case JUBIL_T_FIX:
  case JUBIL_T_FLO:
  case JUBIL_T_STR:
  case JUBIL_T_SYM:
    print_atom(J, o);
    break;
  case JUBIL_T_LIST:
    print_list(J, o);
    break;
  case JUBIL_T_USR:
    fputs("<# ", J->out);
    print_atom(J, o->uname);
    print_list(J, o->ubody);
    fputs(">", J->out);
    break;
  case JUBIL_T_PRIM:
    fputs("<#Primitive: ", J->out);
    print_atom(J, o->pname);
    break;
  default:
    fprintf(stderr, "Invalid object! Aborting\n");
    exit(1);
  }
}
