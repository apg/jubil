#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#include "jubil.h"
#include "internal.h"

#define isdelim(ch) (isspace(ch) || ch == '(' || ch == ')' || ch == '"')

static struct Jbl_Object *
read_number_from_port(struct Jbl *J, struct Jbl_Object *in)
{
#define BUFSIZE 32
  char buffer[BUFSIZE + 1]; /* TODO: How'd we decide this? */
  int bufi = 0;
  int ch = 0;
  int negative = 0;
  long fixval = 0;


  assert(in->type & JUBIL_TYPE_FILEPORT);

  while (bufi < BUFSIZE) {
    ch = Jbl_Port_getc(J, in);
    if (EOF == ch) {
      if (bufi > 0) {
        goto done;
      }

      Jbl_error(J, "EOF while reading integer");
      return NULL;
    } else if (isdelim(ch)) {
      Jbl_port_ungetc(J, in, ch);
      buffer[bufi] = '\0';
      goto done;
    } else if ('+' == ch || '-' == ch) {
      if (0 == bufi) { /* is it the first character? */
        negative = '-' == ch ? 1: 0;
        continue;
      }
      Jbl_error(J, "invalid integer character");
      return NULL;
    } else if (isdigit(ch)) {
      buffer[bufi] = ch;
      bufi++;
    } else {
      Jbl_error(J, "invalid integer character");
      return NULL;
    }
  }

  Jbl_error(J, "integer too big");
  return NULL;

 done:
  /* Do we actually have anything in the buffer? */
  if (0 == bufi) {
    return NULL;
  }

  /* reset errno so we can check it right after */
  errno = 0;
  fixval = strtol(buffer, NULL, 10);
  if (ERANGE == errno && (LONG_MAX == fixval || LONG_MIN == fixval)) {
    Jbl_error(J, "integer out of range");
    return NULL;
  }

  if (negative) {
    fixval *= -1L;
  }

  struct Jbl_Object *new = Jbl_alloc(J, JUBIL_TYPE_INT);
  if (NULL == new) {
    Jbl_error(J, "unable to allocate integer");
    return NULL;
  }

  new->value.integer = fixval;
  return new;
#undef BUFSIZE
}


static struct Jbl_Object *
read_symbol_from_port(struct Jbl *J, struct Jbl_Object *in)
{
#define BUFSIZE 255 /* how do we determine this? */
  char buffer[BUFSIZE + 1];
  int bufi = 0;
  int ch = 0;

  assert(in->type & JUBIL_TYPE_FILEPORT);

  while (bufi < BUFSIZE) {
    ch = Jbl_port_getc(J, in);
    if (EOF == ch) {
      if (bufi > 0) {
        buffer[bufi] = '\0';
        goto intern;
      }

      Jbl_error(J, "EOF while reading symbol");
      return NULL;
    }
    else if (isdelim(ch)) {
      Jbl_port_ungetc(J, in, ch);
      buffer[bufi] = '\0';
      goto intern;
    }
    else if (isgraph(ch)) {
      buffer[bufi++] = ch;
    }
    else if (bufi >= 0 && isalpha(ch)) {
      buffer[bufi++] = ch;
    }
    else if (bufi > 0 && isdigit(ch)) {
      /* can't start with a digit */
      buffer[bufi++] = ch;
    } else {
      Jbl_error(J, "invalid symbol character");
      return NULL;
    }
  }

 intern:
  // FIXME
  return NULL;

#undef BUFSIZE
}



/**
 * Jbl_read reads the next object from the open port.
 *
 * @requires `port` is open and readable.
 * @ensures first object on port is read, an error is raised, or nothing is returned.
 */
struct Jbl_Object *
Jbl_read(struct Jbl *J, struct Jbl_Object *port)
{
  assert(NULL != port);

  if (!OF_TYPE(port, JUBIL_TYPE_FILEPORT)) {
    Jbl_error(J, "not a port");
    return NULL;
  }

  return read_number_from_port(J, port);
}
