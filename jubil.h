#ifndef JUBIL_H_
#define JUBIL_H_

typedef enum {
  JUBIL_T_BOOL = 1,
  JUBIL_T_FIX,
  JUBIL_T_FLO,
  JUBIL_T_STR,
  JUBIL_T_SYM,
  JUBIL_T_LIST,
  JUBIL_T_PRIM,
} jubil_flag;

typedef struct jubil_value jubil_value;
typedef struct jubil_obj jubil_obj;
typedef struct jubil jubil;

struct jubil_value {
  int flags;
  union {
    long fix;
    double flo;
    jubil_obj *object;
  };
};

struct jubil_obj {
  struct jubil_obj *next;
  union {
    struct {
      jubil_value builtin_name;
      int (*builtin)(jubil *);
    };
    struct {
      jubil_value head;
      jubil_value tail;
    };
    struct {
      unsigned long str_hash;
      size_t str_sz;
      char *str;
    };
  };
};



struct jubil {
  jubil_value Nil;
  jubil_value True;
  jubil_value False;

  jubil_value Conts;
  jubil_value Stack;

  /* Global namespace: linearly searched */
  jubil_value *Names;
  jubil_value *Values;
  size_t Names_sz;
  size_t Names_pt;

  FILE *in; /* current input file */
  FILE *out; /* current output file */
  FILE *err; /* current output file */

  jmp_buf *point;

  /* Interned Symbols */
  jubil_value *Syms;
  size_t Syms_sz;
  size_t Syms_pt;
};


jubil_value j_push(jubil *, jubil_value , jubil_value );
jubil_value j_push_fix(jubil *, jubil_value , long);
jubil_value j_push_flo(jubil *, jubil_value , double);
jubil_value j_push_str(jubil *, jubil_value , char *, size_t);
jubil_value j_push_sym(jubil *, jubil_value , char *, size_t);
jubil_value j_push_nil(jubil *, jubil_value );

jubil_value j_pop(jubil *, jubil_value *);
jubil_value j_peek(jubil *, jubil_value );

jubil_value j_fix(jubil *, long);
jubil_value j_flo(jubil *, double);
jubil_value j_str(jubil *, char *, size_t);
jubil_value j_intern(jubil *, char *, size_t);
jubil_value j_cons(jubil *, jubil_value , jubil_value );
jubil_value j_usr(jubil *, jubil_value , jubil_value );
jubil_value j_prim(jubil *, jubil_value , int (*prim)(jubil *));

jubil_value j_define(jubil *, jubil_value , jubil_value );
jubil_value j_lookup(jubil *, jubil_value );

jubil_value j_head(jubil *, jubil_value );
jubil_value j_tail(jubil *, jubil_value );

jubil_value j_read(jubil *);
void j_write(jubil *, jubil_value o);

void j_init(jubil *);
void j_init_builtins(jubil *);
void j_exec(jubil *, jubil_value );
void j_error(jubil *, char *);

#endif
