#ifndef JUBIL_H_
#define JUBIL_H_

typedef enum {
  J_BOOL_T = 1,
  J_FIX_T,
  J_FLO_T,
  J_STR_T,
  J_SYM_T,
  J_LIST_T,
  J_USR_T,
  J_PRIM_T
} j_flag_t;

typedef struct jubil_obj j_obj_t;
typedef struct jubil j_t;

struct jubil_obj {
  int flags;
  union {
    long fix;
    double flo;
    struct {
      j_obj_t *head;
      j_obj_t *tail;
    };
    struct {
      size_t str_sz;
      char *str;
    };
    struct {
      j_obj_t *uname; /* sym */
      j_obj_t *ubody; /* list */
    };
    struct {
      j_obj_t *pname; /* sym */
      void (*prim)(j_t *);
    };
  };
};

struct jubil {
  j_obj_t *Nil;
  j_obj_t *True;
  j_obj_t *False;
  j_obj_t *Conts;
  j_obj_t *Stack;
  j_obj_t **Syms;

  /* Dictionary: for now, I single namespace */
  j_obj_t **Names;
  j_obj_t **Values;

  FILE *in; /* current input file */
  FILE *out; /* current output file */
  FILE *err; /* current output file */

  jmp_buf *point;

  int Syms_sz;
  int Syms_pt;

  int Names_sz;
  int Names_pt;
};

typedef struct jubil_builtin {
  void (*prim)(j_t *);
  char *name;
} j_builtin_t;

j_obj_t *j_push(j_t *, j_obj_t *, j_obj_t *);
j_obj_t *j_push_fix(j_t *, j_obj_t *, long);
j_obj_t *j_push_flo(j_t *, j_obj_t *, double);
j_obj_t *j_push_str(j_t *, j_obj_t *, char *, size_t);
j_obj_t *j_push_sym(j_t *, j_obj_t *, char *, size_t);
j_obj_t *j_push_nil(j_t *, j_obj_t *);

j_obj_t *j_pop(j_t *, j_obj_t **);
j_obj_t *j_peek(j_t *, j_obj_t *);

j_obj_t *j_fix(j_t *, long);
j_obj_t *j_flo(j_t *, double);
j_obj_t *j_str(j_t *, char *, size_t);
j_obj_t *j_intern(j_t *, char *, size_t);
j_obj_t *j_cons(j_t *, j_obj_t *, j_obj_t *);
j_obj_t *j_usr(j_t *, j_obj_t *, j_obj_t *);
j_obj_t *j_prim(j_t *, j_obj_t *, void (*prim)(j_t *));

j_obj_t *j_define(j_t *, j_obj_t *, j_obj_t *);
j_obj_t *j_lookup(j_t *, j_obj_t *);

j_obj_t *j_head(j_t *, j_obj_t *);
j_obj_t *j_tail(j_t *, j_obj_t *);

j_obj_t *j_read(j_t *);
void j_write(j_t *, j_obj_t *o);

void j_init(j_t *);
void j_init_builtins(j_t *);
void j_exec(j_t *, j_obj_t *);
void j_error(j_t *, char *);

#endif
