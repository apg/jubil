#include <setjmp.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "jubil.h"

#ifndef MIN
#define MIN(x, y) ((x < y) ? (x): (y))
#endif

#ifndef IS_NIL
#define IS_NIL(J, o) ((o.flags & JUBIL_T_CONS) && \
                      o.object == J->Nil.object)
#endif


static unsigned long
djb2_hash(char *str)
{
  unsigned long hash = 5381;
  int c;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) ^ c;
  }
  return hash;
}


static jubil_value
mk_str(jubil *J, char *str, size_t len, int flag)
{
  jubil_value o;

  o.flags = flag;
  o.object->str = strndup(str, len);
  o.object->str[len-1] = '\0'; /* force NULL termination */
  o.object->str_sz = len;
  o.object->str_hash = djb2_hash(str);

  return o;
}

static int
sym_find(jubil *J, char *str, size_t len)
{
  int i = -1;
  unsigned long shash = djb2_hash(str);
  if (J->Syms_pt > 0) {
    for (i = 0; i < J->Syms_pt; i++) {
      if (len != J->Syms[i].object->str_sz) {
        continue;
      } else if (J->Syms[i].object->str_hash != shash) {
        continue;
      } else if (strncmp(J->Syms[i].object->str, str, len) == 0) {
        return 1;
      }
    }
  }

  return -1;
}


jubil_value
j_fix(jubil *J, long num)
{
  jubil_value o;
  o.flags = JUBIL_T_FIX;
  o.fix = num;
  return o;
}

jubil_value
j_flo(jubil *J, double num)
{
  jubil_value o;
  o.flags = JUBIL_T_FLO;
  o.flo = num;
  return o;
}

jubil_value
j_str(jubil *J, char *str, size_t len)
{
  return mk_str(J, str, len, JUBIL_T_STR);
}

jubil_value
j_intern(jubil *J, char *str, size_t len)
{
  int i;
  jubil_value sym;

  i = sym_find(J, str, len);
  if (i >= 0) {
    return J->Syms[i];
  }

  sym = mk_str(J, str, len, JUBIL_T_SYM);

  if (J->Syms_pt >= J->Syms_sz) {
    J->Syms_sz = J->Syms_sz > 0 ? J->Syms_sz * 2: 8;
    J->Syms = realloc(J->Syms, sizeof(*J->Syms) * J->Syms_sz);
    if (J->Syms == NULL) {
      perror("realloc");
      exit(1);
    }
  }

  J->Syms[J->Syms_pt++] = sym;

  return sym;
}

jubil_value
j_cons(jubil *J, jubil_value h, jubil_value t)
{
  jubil_value o;

  o.object = malloc(sizeof(*o.object));
  if (o.object == NULL) {
    perror("malloc");
    exit(1);
  }

  o.flags = JUBIL_T_LIST;
  o.object->head = h;
  o.object->tail = t;
  return o;
}


jubil_value
j_prim(jubil *J, jubil_value sym, int (*prim)(jubil *))
{
  jubil_value o;

  o.object = malloc(sizeof(*o.object));
  if (o.object == NULL) {
    perror("malloc");
    exit(1);
  }

  o.flags = JUBIL_T_PRIM;
  o.object->builtin_name = sym;
  o.object->builtin = prim;

  return o;
}

int
j_strcmp(jubil_value a, jubil_value b)
{
  int same = (a.flags & b.flags) & (JUBIL_T_SYM | JUBIL_T_STR);
  if (same) {
    return strncmp(a.object->str, b.object->str,
                  MIN(a.object->str_sz, b.object->str_sz));
  }
  return -1;
}


jubil_value
j_head(jubil *J, jubil_value l)
{
  /* TODO: assert it's a list */
  return l.object->head;
}

jubil_value
j_tail(jubil *J, jubil_value l)
{
  /* TODO: assert it's a list */
  return l.object->tail;
}

static int
find_name(jubil *J, jubil_value sym)
{
  int i;
  for (i = 0; i < J->Names_pt; i++) {
    if ((J->Names[i].object->str_hash == sym.object->str_hash) &&
        j_strcmp(J->Names[i], sym)) {
      return i;
    }
  }
  return -1;
}

jubil_value
j_define(jubil *J, jubil_value sym, jubil_value val)
{
  int i;

  if ((sym.flags & JUBIL_T_SYM) == 0) {
    j_error(J, "def called with non-symbol as first argument");
    return J->Nil;
  }

  i = find_name(J, sym);
  if (i >= 0) {
    J->Names[i] = val;
    return val;
  }

  /* Not found. */
  if (J->Names_pt >= J->Names_sz) {
    J->Names_sz = J->Names_sz > 0 ? J->Names_sz * 2: 8;
    J->Names = realloc(J->Names, sizeof(*J->Names) * J->Names_sz);
    if (J->Names == NULL) {
      perror("realloc");
      exit(1);
    }
    J->Values = realloc(J->Values, sizeof(*J->Values) * J->Names_sz);
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

jubil_value
j_lookup(jubil *J, jubil_value sym)
{
  int i;

  i = find_name(J, sym);
  if (i >= 0) {
    return J->Values[i];
  }

  j_error(J, "unknown name");
  return J->Nil;
}

void
j_error(jubil *J, char *error)
{
  fputs(error, J->err);
  fputc('\n', J->err);
  fflush(J->in); /* might have read buffer in there */
  J->Stack = J->Conts = J->Nil;

  /* longjmp to wherever after outputting the error message */
  if (J->point != NULL) {
    longjmp(*(J->point), 1);
  }
  else {
    exit(1);
  }
}


void
j_init(jubil *J)
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

  J->True = j_fix(J, 1);
  J->True.flags = JUBIL_T_BOOL;
  J->False = j_fix(J, 0);
  J->False.flags = JUBIL_T_BOOL;

  /* Circular Nil */
  J->Nil = j_cons(J, J->False, J->False);
  J->Nil.object->head = J->Nil;
  J->Nil.object->tail = J->Nil;

  J->Conts = J->Nil;
  J->Stack = J->Nil;

  J->in = stdin;
  J->out = stdout;
  J->err = stderr;
}

void
j_repl(jubil *J)
{
  jubil_value tmp;
  J->point = malloc(sizeof(*J->point));
  if (J->point == NULL) {
    perror("malloc");
    exit(1);
  }

  setjmp(*J->point);
  for (;;) {
    tmp = j_read(J);
    j_write(J, tmp);
  }
}

int
main(int argc, char **argv)
{
  jubil j;
  jubil *J = &j;

  jubil_value tmp;

  j_init(J);

  tmp = j_read(J);
  j_write(J, tmp);

  /*j_repl(J);*/

  return 0;
}
