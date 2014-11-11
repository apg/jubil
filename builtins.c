#include <setjmp.h>
#include <string.h>
#include <stdio.h>

#include "jubil.h"

/* dup
 * + 
 * puts
 */

static void
jB_dup(j_t *J)
{
  fputs("Hello from 'dup'\n", stderr);  
  if (J->Stack->head == J->Nil) {
    j_error(J, "'dup' requires one parameter on the stack.");
  }

  J->Stack = j_push(J, J->Stack, J->Stack->head);
}

static void
jB_plus(j_t *J)
{
  j_obj_t *left, *right;
  fputs("Hello from '+'\n", stderr);
  
  if (J->Stack->head == J->Nil ||
      J->Stack->tail == J->Nil ||
      J->Stack->tail->head == J->Nil) {
    j_error(J, "'+' requires two numeric parameters on the stack.");
  }

  right = j_pop(J, &J->Stack);
  left = j_pop(J, &J->Stack);  

  switch (left->flags) {
  case J_FIX_T:
    if (right->flags == J_FIX_T) {
      J->Stack = j_push_fix(J, J->Stack, left->fix + right->fix);
    }
    else if (right->flags == J_FLO_T) {
      J->Stack = j_push_flo(J, J->Stack, left->fix + right->flo);
    }
    else {
      j_error(J, "'+' requires 2 numeric values.");
    }
    break;
  case J_FLO_T:
    if (right->flags == J_FIX_T) {
      J->Stack = j_push_flo(J, J->Stack, left->flo + right->fix);
    }
    else if (right->flags == J_FLO_T) {
      J->Stack = j_push_flo(J, J->Stack, left->flo + right->flo);
    }
    else {
      j_error(J, "'+' requires 2 numeric values.");
    }
  }
}

static void
jB_puts(j_t *J)
{
  j_obj_t *arg;
  if (J->Stack->head == J->Nil) {
    j_error(J, "'puts' requires 1 argument on the stack.");
  }

  fputs("Hello from 'puts'\n", stderr);
  
  arg = j_pop(J, &J->Stack);

  switch (arg->flags) {
  case J_FIX_T:
    fprintf(stdout, "%ld", arg->fix);
    break;
  case J_FLO_T:
    fprintf(stdout, "%lf", arg->flo);
    break;
  case J_STR_T:
  case J_SYM_T:
    fprintf(stdout, "%s", arg->str);
    break;
  default:
    fputs("<VALUE>", stdout);
  }

  fflush(stdout);
}

static j_builtin_t builtins[] = {
  { jB_dup, "dup" },
  { jB_plus, "+" },
  { jB_puts, "puts" },
  { NULL, NULL }
};


void
j_init_builtins(j_t *J)
{
  j_builtin_t *b = builtins;
  j_obj_t *p, *s;
  
  while (b->name != NULL) {
    s = j_intern(J, b->name, strlen(b->name));
    p = j_prim(J, s, b->prim);
    j_define(J, s, p);
    b++;
  }
}



