#include <setjmp.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "jubil.h"

#define isdelim(ch) (isspace(ch) || ch == '(' || ch == ')' || ch == '"')

/**
 * Should pass stuff on the stack, instead of via return values.
 */

static j_obj_t *
read_string(j_t *J, FILE *in)
{
  char buffer[255];
  int bufi = 0;
  int ch, la;

  while (bufi < 255) {
    ch = fgetc(in);
    if (ch == EOF) {
      j_error(J, "eof while reading string.");
      return NULL;
    }
    if (ch == '"') {
      buffer[bufi] = '\0';
      return j_str(J, buffer, bufi);
    }
    else if (ch == '\\') {
      la = fgetc(in);
      if (la == EOF) {
        j_error(J, "eof while reading string.");
        return NULL;
      }
      switch (la) {
      case '\\':
        buffer[bufi++] = '\\';
        break;
      case '"':
        buffer[bufi++] = '"';
        break;
      case 'a':
        buffer[bufi++] = '\a';
        break;
      case 'n':
        buffer[bufi++] = '\n';
        break;
      case 'r':
        buffer[bufi++] = '\r';
        break;
      case 't':
        buffer[bufi++] = '\t';
        break;
      }
    } else {
      buffer[bufi++] = ch;
    }
  }

  j_error(J, "string too long\n");
  return NULL;
}

static j_obj_t *
read_number(j_t *J, FILE *in, int negative)
{
  char buffer[32];
  int bufi = 0;
  int sawdot = 0;
  int ch;
  double floval;
  long fixval;

  while (bufi < 32) {
    ch = fgetc(in);
    if (ch == EOF) {
      j_error(J, "eof  while reading number.");
      return NULL;
    }

    if (isdigit(ch)) {
      buffer[bufi++] = ch;
    }
    else if (ch == '.') {
      if (sawdot) {
        j_error(J, "invalid number found.");
        return NULL;
      }
      else {
        buffer[bufi++] = '.';
        sawdot = 1;
      }
    }
    else if (isdelim(ch)) {
      ungetc(ch, in);
      /* have our number. Let's do it */
      buffer[bufi] = '\0';
      if (bufi > 0) {
        if (sawdot) {
          floval = strtod(buffer, NULL);
          if (negative) {
            floval *= -1.0;
          }
          return j_flo(J, floval);
        }
        else {
          fixval = strtol(buffer, NULL, 10);
          if (negative) {
            fixval *= -1L;
          }
          return j_fix(J, fixval);
        }
      }
      else {
        j_error(J, "invalid number found.");
        return NULL;
      }
    }
  }
  j_error(J, "can't support numbers that large.");
  return NULL;
}

static j_obj_t *
read_symbol(j_t *J, FILE *in)
{
  char buffer[255];
  int bufi = 0;
  int ch, la, sawdot = -1;
  j_obj_t *module, *identifier;

  while (bufi < 255) {
    ch = fgetc(in);
    if (ch == EOF) {
      j_error(J, "eof while reading symbol.");
      return NULL;
    }

    if (isalnum(ch)) {
      buffer[bufi++] = ch;
    }
    else if (ch == '.') { /* TODO: Need to turn this into a refer form ideally ... */
      sawdot = bufi;
      buffer[bufi++] = ch;
    }
    else if (isgraph(ch) && !isdelim(ch)) {
      buffer[bufi++] = ch;
    }
    else if (isdelim(ch)) {
      ungetc(ch, in);
      buffer[bufi] = '\0';
      return j_intern(J, buffer, bufi);
    }
    else {
      j_error(J, "invalid symbol character.");
      return NULL;
    }
  }
}

static char
eat_space(FILE *in)
{
  int ch;
  do {
    ch = fgetc(in);
  } while (isspace(ch));
  return ch;
}

static j_obj_t *
read_list(j_t *J, FILE *in)
{
  j_obj_t *obj;
  int ch;
  ch = fgetc(in);
  if (ch == EOF) {
    j_error(J, "eof while reading list.");
    return NULL;
  }

  /* Is this just nil? */
  if (ch == ')') {
    return J->Nil;
  }
  else {
    ungetc(ch, in);
  }

  /* Ok. Legitimate list it seems. Let's read it recursively */
  obj = j_read(J, in);
  if (!obj) {
    return obj;
  }

  ch = eat_space(in);
  if (ch == ')') {
    return j_cons(J, obj, J->Nil);
  }

  ungetc(ch, in);
  return j_cons(J, obj, read_list(J, in));
}

j_obj_t *
j_read(j_t *J, FILE *in)
{
  j_obj_t *tmp;
  int ch, la;
 next:
  ch = fgetc(in);
  if (ch == EOF) {
    return NULL;
  }

  switch (ch) {
  case '(':
    return read_list(J, in);
  case ';': /* read til end of line */
    while ((ch = fgetc(in)) != '\n') {
      if (ch == EOF) {
        return NULL;
      }
    }
    goto next;
  case ' ':
  case '\t':
  case '\n':
  case '\r':
    goto next;
  case '"':
    return read_string(J, in);
  case '\'':
    tmp = j_read(J, in);
    if (tmp != NULL && tmp->flags == J_LIST_T) {
      fprintf(stderr, "TRACE: quoted list becomes anonymous USR\n");
      return j_usr(J, j_intern(J, "<anonymous>", strlen("<anonymous>")), tmp);
    }
    break;
  case '-':
  case '+':
    la = fgetc(in);
    if (la == EOF) {
      j_error(J, "eof reached in mid form.");
      return NULL;
    }
    if (isdigit(la)) {
      ungetc(la, in);
      return read_number(J, in, ch == '-');
    }
    else {
      ungetc(la, in);
    }
  default:
    ungetc(ch, in);
    if (isdigit(ch)) {
      return read_number(J, in, 0);
    }
  }

  tmp = read_symbol(J, in);
  fprintf(stderr, "TRACE: Looking up a symbol\n");
  return j_lookup(J, tmp);
}
