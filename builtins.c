#include <setjmp.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "jubil.h"

/**

#include "lib.jb"

booleans:   false, () are falsey
   true, true?      U '(false? not) '(nil? not) and
   false, false?    U '(= false) '(nil?) and
   boolean?         U '(true? false? or)

   and
   or
   ->str
   =
   !=
   >
   >=
   <
   <=
   not              U dup true? [pop pop false] [pop] if



fix:
   fix?
   zero?    U
   pos?     U
   neg?     U
   incr     U
   decr     U
   +
   -
   /
   *
   %
   <<  -- left shift
   >>  -- right shift
   >>> -- right shift truncate

   ->str


flo:
   flo?
   pos?
   neg?
   +, -, /, *
   trunc

sym:
   sym?
   ->str

str:
   str?
   substr
   at




  (define (cat-eval stk exp)
      (if (null? exp)
          stk
          (cat-eval
           (case (car exp)
             ((+)      (cons (+ (car stk) (cadr stk)) (cddr stk)))
             ((and)    (cons (and (car stk) (cadr stk)) (cddr stk)))
             ((apply)  (cat-eval (cdr stk) (car stk)))
             ((at)     ;;; get something at (car stk)th index (strings, lists)
             ((compose)(cons (append (car stk) (cadr stk)) (cddr stk)))
             ((cons)   (cons (list (cadr stk) (car stk)) (cddr stk)))
             ((decr)   (cons (- (car stk) 1) (cdr stk)))
             ((dip)    (cons (cadr stk) (cat-eval (cddr stk) (car stk))))
             ((error)  (error (car stk))) ;; ABORT with error message!
             ((/)      (cons (/ (car stk) (cadr stk)) (cddr stk)))
             ((dup)    (cons (car stk) stk))
             ((eq)     (cons (= (cadr stk) (car stk)) (cddr stk)))
             ((nil?)   (cons (empty? (car stk)) (cdr stk)))
             ((false)  (cons #f stk))
             ((if)     (if (caddr stk)
                          (cat-eval (cdddr stk) (cadr stk))
                          (cat-eval (cdddr stk) (car stk))))
             ((incr)    (cons (+ (car stk) 1) (cdr stk)))
             ((list)   (cons (list (car stk)) (cdr stk)))
             ((%)      (cons (% (cadr stk) (car)) (cddr stk)))
             ((*)      (cons (* (car stk) (cadr)) (cddr stk)))
             ((not)    (cons (not (car stk)) (cdr stk)))
             ((or)     (cons (or (car stk) (cadr stk)) (cddr stk)))
             ((papply) (cons (cons (cadr stk) (car stk)) (cddr stk)))
             ((pop)    (cdr stk))
             ((quote)  (cons (list (car stk)) (cdr stk)))
             ((-)      (cons (- (car stk) (cadr)) (cddr stk)))
             ((swap)   (cons (cadr stk) (cons (car stk) (cddr stk))))
             ((true)   (cons #t stk))
             ((uncons) (cat-eval (cdr stk) (car stk)))
           (else
             (cons (car exp) stk)))
           (cdr exp))))


func: body1 body2 [quote1] whatever.

l1: 1 nil cons.

list-literal: (1).

quotation: '(foo bar baz) dip.

map-literal: { k1 v1 k2 v2 }

 */

static void
jB_nil(j_t *J)
{
  J->Stack = j_push(J, J->Stack, J->Nil);
}

static void
jB_false(j_t *J)
{
  J->Stack = j_push(J, J->Stack, J->False);
}

static void
jB_true(j_t *J)
{
  J->Stack = j_push(J, J->Stack, J->True);
}


static void
jB_dup(j_t *J)
{
  if (J->Stack->head == J->Nil) {
    j_error(J, "'dup' requires one parameter on the stack.");
  }

  J->Stack = j_push(J, J->Stack, J->Stack->head);
}

static void
jB_numeric_binop(j_t *J, char op)
{
  j_obj_t *left, *right, *res;
  double rd, ld;

  if (J->Stack->head == J->Nil ||
      J->Stack->tail == J->Nil ||
      J->Stack->tail->head == J->Nil) {
    j_error(J, "combinator requires two numeric parameters on the stack.");
  }

  left = j_pop(J, &J->Stack);
  right = j_pop(J, &J->Stack);

  if (left->flags == right->flags && right->flags == J_FIX_T) {
    switch (op) {
    case '+':
      res = j_fix(J, left->fix + right->fix);
      break;
    case '-':
      res = j_fix(J, left->fix - right->fix);
      break;
    case '*':
      res = j_fix(J, left->fix * right->fix);
      break;
    case '/':
      if (right->fix != 0) {
        res = j_fix(J, left->fix / right->fix);
      }
      else {
        j_error(J, "division by zero.");
        return;
      }
      break;
    case '%':
      if (right->fix != 0) {
        res = j_fix(J, left->fix % right->fix);
      }
      else {
        j_error(J, "division by zero.");
        return;
      }
    case '<':
      if (left->fix < right->fix) {
        res = J->True;
      }
      else {
        res = J->False;
      }
      break;
    case '>':
      if (left->fix > right->fix) {
        res = J->True;
      }
      else {
        res = J->False;
      }
      break;
    case '=':
      if (left->fix == right->fix) {
        res = J->True;
      }
      else {
        res = J->False;
      }
      break;
    case 'N':
      if (left->fix != right->fix) {
        res = J->True;
      }
      else {
        res = J->False;
      }
      break;
    case 'G':
      if (left->fix >= right->fix) {
        res = J->True;
      }
      else {
        res = J->False;
      }
      break;
    case 'L':
      if (left->fix <= right->fix) {
        res = J->True;
      }
      else {
        res = J->False;
      }
      break;
    default:
      j_error(J, "unknown operator.");
      return;
    }
  }
  else {
    ld = (left->flags == J_FIX_T) ? (double) left->fix: left->flo;
    rd = (right->flags == J_FIX_T) ? (double) right->fix: right->flo;
    switch (op) {
    case '+':
      res = j_flo(J, ld + rd);
      break;
    case '-':
      res = j_flo(J, ld - rd);
      break;
    case '*':
      res = j_flo(J, ld * rd);
      break;
    case '/':
      ld = fmod(ld, rd);
      if (errno == EDOM) {
        j_error(J, "division by zero.");
        return;
      }
      res = j_flo(J, ld);
      break;
    case '%':
      ld = fmod(ld, rd);
      if (errno == EDOM) {
        j_error(J, "division by zero.");
        return;
      }
      res = j_flo(J, ld);
    case '<':
      if (ld < rd) {
        res = J->True;
      }
      else {
        res = J->False;
      }
      break;
    case '>':
      if (ld > rd) {
        res = J->True;
      }
      else {
        res = J->False;
      }
      break;
    case '=':
      if (ld == rd) {
        res = J->True;
      }
      else {
        res = J->False;
      }
      break;
    case 'N':
      if (ld != rd) {
        res = J->True;
      }
      else {
        res = J->False;
      }
      break;
    case 'G':
      if (ld >= rd) {
        res = J->True;
      }
      else {
        res = J->False;
      }
      break;
    case 'L':
      if (ld <= rd) {
        res = J->True;
      }
      else {
        res = J->False;
      }
      break;
    default:
      j_error(J, "unknown operator.");
      return;
    }
  }

  J->Stack = j_push(J, J->Stack, res);
}

/* TODO: We want equality operators to work with strings and other
 * types too. So, this isn't ideal, but we'll deal with it for
 * now... */
#define DEF_OP(N, OP) static void\
  jB_##N(j_t *J) {\
  jB_numeric_binop(J, OP);\
}

DEF_OP(plus, '+')
DEF_OP(minus, '-')
DEF_OP(mult, '*')
DEF_OP(div, '/')
DEF_OP(mod, '%')
DEF_OP(gt, '>')
DEF_OP(lt, '<')
DEF_OP(eq, '=')
DEF_OP(ne, 'N')
DEF_OP(ge, 'G')
DEF_OP(le, 'L')

static void
jB_puts(j_t *J)
{
  if (J->Stack->head == J->Nil) {
    j_error(J, "'puts' requires 1 argument on the stack.");
  }

  j_write(J, j_pop(J, &J->Stack));
  fputc('\n', J->out);
}

static j_builtin_t builtins[] = {
  { jB_true, "true" },
  { jB_false, "false" },
  { jB_nil, "nil" },
  { jB_dup, "dup" },
  { jB_plus, "+" },
  { jB_minus, "-" },
  { jB_mult, "*" },
  { jB_div, "/" },
  { jB_mod, "%" },
  { jB_lt, ">" },
  { jB_gt, "<" },
  { jB_eq, "=" },
  { jB_ne, "!=" },
  { jB_ge, ">=" },
  { jB_le, "<=" },
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
