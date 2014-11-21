#include <gc.h>

#include <setjmp.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "jubil.h"

static void *(*Malloc)(size_t) = GC_malloc;
static void *(*Realloc)(void *, size_t) = GC_realloc;

static j_obj_t *
mk_str(j_t *J, char *str, size_t len, int flag)
{
  j_obj_t *o = Malloc(sizeof(*o));
  if (o == NULL) {
    perror("malloc");
    exit(1);
  }

  o->flags = flag;
  o->str = strndup(str, len);
  o->str_sz = len;

  return o;
}

static int
sym_find(j_t *J, char *str, size_t len)
{
  int i = -1;
  if (J->Syms_pt > 0) {
    for (i = 0; i < J->Syms_pt; i++) {
      if (len != J->Syms[i]->str_sz) {
        continue;
      }
      if (strncmp(J->Syms[i]->str, str, len) == 0) {
        return i;
      }
    }
  }

  return -1;
}

j_obj_t *
j_fix(j_t *J, long num)
{
  j_obj_t *o = Malloc(sizeof(*o));
  if (o == NULL) {
    perror("malloc");
    exit(1);
  }

  o->flags = J_FIX_T;
  o->fix = num;
  return o;
}

j_obj_t *
j_flo(j_t *J, double num)
{
  j_obj_t *o = Malloc(sizeof(*o));
  if (o == NULL) {
    perror("malloc");
    exit(1);
  }

  o->flags = J_FLO_T;
  o->flo = num;
  return o;
}

j_obj_t *
j_str(j_t *J, char *str, size_t len)
{
  return mk_str(J, str, len, J_STR_T);
}

j_obj_t *
j_intern(j_t *J, char *str, size_t len)
{
  int i;
  j_obj_t *sym;

  i = sym_find(J, str, len);
  if (i >= 0) {
    return J->Syms[i];
  }

  sym = mk_str(J, str, len, J_SYM_T);

  if (J->Syms_pt >= J->Syms_sz) {
    J->Syms_sz = J->Syms_sz > 0 ? J->Syms_sz * 2: 8;
    J->Syms = Realloc(J->Syms, sizeof(*J->Syms) * J->Syms_sz);
    if (J->Syms == NULL) {
      perror("realloc");
      exit(1);
    }
  }

  J->Syms[J->Syms_pt++] = sym;

  return sym;
}

j_obj_t *
j_cons(j_t *J, j_obj_t *h, j_obj_t *t)
{
  j_obj_t *o = Malloc(sizeof(*o));
  if (o == NULL) {
    perror("malloc");
    exit(1);
  }

  o->flags = J_LIST_T;
  o->head = h;
  o->tail = t;
  return o;
}

j_obj_t *
j_usr(j_t *J, j_obj_t *sym, j_obj_t *body)
{
  j_obj_t *o = Malloc(sizeof(*o));
  if (o == NULL) {
    perror("malloc");
    exit(1);
  }

  o->flags = J_USR_T;
  o->uname = sym;
  o->ubody = body;

  return o;
}

j_obj_t *
j_prim(j_t *J, j_obj_t *sym, void (*prim)(j_t *))
{
  j_obj_t *o = Malloc(sizeof(*o));
  if (o == NULL) {
    perror("malloc");
    exit(1);
  }

  o->flags = J_PRIM_T;
  o->pname = sym;
  o->prim = prim;

  return o;
}


j_obj_t *
j_head(j_t *J, j_obj_t *l)
{
  /* TODO: assert it's a list */
  return l->head;
}

j_obj_t *
j_tail(j_t *J, j_obj_t *l)
{
  /* TODO: assert it's a list */
  return l->tail;
}


j_obj_t *
j_define(j_t *J, j_obj_t *sym, j_obj_t *val)
{
  int i;
  for (i = 0; i < J->Names_pt; i++) {
    if (J->Names[i] == sym) {
      J->Values[i] = val;
    }
  }

  if (J->Names_pt >= J->Names_sz) {
    J->Names_sz = J->Names_sz > 0 ? J->Names_sz * 2: 8;
    J->Names = Realloc(J->Names, sizeof(*J->Names) * J->Names_sz);
    if (J->Names == NULL) {
      perror("realloc");
      exit(1);
    }
    J->Values = Realloc(J->Values, sizeof(*J->Values) * J->Names_sz);
    if (J->Values == NULL) {
      perror("realloc");
      exit(1);
    }
  }

  J->Names[J->Names_pt] = sym;
  J->Values[J->Names_pt] = val;
  J->Names_pt++;

  return val;
}

j_obj_t *
j_lookup(j_t *J, j_obj_t *sym)
{
  int i;
  for (i = 0; i < J->Names_pt; i++) {
    if (J->Names[i] == sym) {
      return J->Values[i];
    }
  }
  j_error(J, "unknown name");
  return J->Nil;
}


j_obj_t *
j_push(j_t *J, j_obj_t *s, j_obj_t *o)
{
  return j_cons(J, o, s);
}

j_obj_t *
j_pop(j_t *J, j_obj_t **s)
{
  j_obj_t *h;
  if (*s == J->Nil) {
    j_error(J, "stack underflow");
    return J->Nil;
  }

  h = j_head(J, *s);
  *s = j_tail(J, *s);

  return h;
}

j_obj_t *
j_peek(j_t *J, j_obj_t *s)
{
  if (s == J->Nil) {
    j_error(J, "stack underflow");
    return J->Nil;
  }

  return j_head(J, s);
}



/** TODO: THESE PROBABABLY SHOULD JUST BE MACROS */
j_obj_t *
j_push_fix(j_t *J, j_obj_t *s,  long n)
{
  return j_push(J, s, j_fix(J, n));
}

j_obj_t *
j_push_flo(j_t *J,  j_obj_t *s, double n)
{
  return j_push(J, s, j_flo(J, n));
}

j_obj_t *
j_push_str(j_t *J,  j_obj_t *s, char *str, size_t len)
{
  return j_push(J, s, j_str(J, str, len));
}

j_obj_t *
j_push_sym(j_t *J,  j_obj_t *s, char *str, size_t len)
{
  return j_push(J, s, j_intern(J, str, len));
}

j_obj_t *
j_push_nil(j_t *J,  j_obj_t *s)
{
  return j_push(J, s, J->Nil);
}

void
j_error(j_t *J, char *error)
{
  fputs(error, stderr);
  fputc('\n', stderr);

  /* longjmp to wherever after outputting the error message */
  if (J->point != NULL) {
    longjmp(*(J->point), 1);
  }
  else {
    exit(1);
  }
}

void
j_exec(j_t *J, j_obj_t *program)
{
  j_obj_t *cursor;

 recur:
  if (program == J->Nil) {
    return;
  }

  J->Conts = j_push(J, J->Conts, program);

  while (J->Conts->head != J->Nil) {
    cursor = j_head(J, J->Conts->head);
    switch (cursor->flags) {
    case J_BOOL_T:
    case J_FIX_T:
    case J_FLO_T:
    case J_STR_T:
    case J_SYM_T:
    case J_LIST_T:
      J->Stack = j_push(J, J->Stack, cursor);
      break;

    case J_USR_T:
      if (J->Conts->head->tail == J->Nil) {
        J->Conts = j_tail(J, J->Conts);
        program = cursor->ubody;
        goto recur;
      }
      else {
        j_exec(J, cursor->ubody);
      }
      break;
    default:
      (cursor->prim)(J);
    }

    J->Conts->head = j_tail(J, J->Conts->head);
  }
  j_pop(J, &J->Conts);
}

void
j_init(j_t *J)
{
  /* Zero symtab */
  J->Syms = NULL;
  J->Syms_sz = 0;
  J->Syms_pt = 0;

  J->Names = NULL;
  J->Names_sz = 0;
  J->Names_pt = 0;

  J->Values = NULL;

  J->point = NULL;

  /* Circular Nil */
  J->Nil = j_cons(J, NULL, NULL);
  J->Nil->head = J->Nil;
  J->Nil->tail = J->Nil;

  J->True = j_fix(J, 1);
  J->True->flags = J_BOOL_T;
  J->False = j_fix(J, 0);
  J->False->flags = J_BOOL_T;

  J->Conts = J->Nil;
  J->Stack = J->Nil;

  j_init_builtins(J);
}

void
j_repl(j_t *J)
{
  j_obj_t *tmp;
  J->point = Malloc(sizeof(*J->point));
  if (J->point == NULL) {
    perror("malloc");
    exit(1);
  }

  setjmp(*J->point);
  for (;;) {
    tmp = j_read(J, stdin);
    if (tmp->flags == J_LIST_T) {
      j_exec(J, tmp);
    }
    else {
      J->Stack = j_push(J, J->Stack, tmp);
    }

  }
}

int
main(int argc, char **argv)
{
  j_t j;
  j_t *J = &j;

<<<<<<< HEAD
  j_obj_t *f, *dbl, *dblsym, *trace;

  j_init(J);

  /* dblsym = j_intern(J, "dbl", 3); */

  /* trace = j_usr(J, j_intern(J, "trace", 5), */
  /*               j_cons(J, j_lookup(J, j_intern(J, "dup", 3)), */
  /*                      j_cons(J, j_lookup(J, j_intern(J, "puts", 4)), */
  /*                             J->Nil))); */

  /* dbl = j_usr(J, dblsym, */
  /*             j_cons(J, j_lookup(J, j_intern(J, "dup", 3)), */
  /*                    j_cons(J, j_lookup(J, j_intern(J, "+", 1)), */
  /*                           j_cons(J, trace, J->Nil)))); */


  /* /\* */
  /*    trace = dup puts */
  /*    dbl = dup + trace */
  /*    (10 dbl dbl dbl) *\/ */
  /* f = J->Nil; */
  /* f = j_cons(J, dbl, f); */
  /* f = j_cons(J, dbl, f); */
  /* f = j_cons(J, dbl, f); */
  /* f = j_cons(J, j_fix(J, 10), f); */
  /* j_exec(J, f); */

  j_repl(J);

=======
  
  j_obj_t *f, *dbl, *dblsym, *trace;
  
  j_init(J);

  dblsym = j_intern(J, "dbl", 3);

  trace = j_usr(J, j_intern(J, "trace", 5),
                j_cons(J, j_lookup(J, j_intern(J, "dup", 3)),
                       j_cons(J, j_lookup(J, j_intern(J, "puts", 4)),
                              J->Nil)));
  
  dbl = j_usr(J, dblsym,
              j_cons(J, j_lookup(J, j_intern(J, "dup", 3)),
                     j_cons(J, j_lookup(J, j_intern(J, "+", 1)),
                            j_cons(J, trace, J->Nil))));

  
  /* 
     trace = dup puts
     dbl = dup + trace
     (10 dbl dbl dbl) */
  f = J->Nil;
  f = j_cons(J, dbl, f);  
  f = j_cons(J, dbl, f);
  f = j_cons(J, dbl, f);
  f = j_cons(J, j_fix(J, 10), f);  
  j_exec(J, f);
>>>>>>> 6ac442e9eb35836d6f5953ea49a7a478c405f9bf
  return 0;
}
