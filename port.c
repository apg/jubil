#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "jubil.h"
#include "internal.h"

/**
 * Jbl_openfile opens the file at `path` with the given `mode`,
 * creating a port, usable by Jubil.
 *
 * @requires path != "" and path != NULL
 * @requires mode != "" and mode != NULL
 *
 * When allocations and file open is successful:
 * @ensures returned port has an open file handle, with semantics of
 *  fopen(3).
 */
struct Jbl_Object *
Jbl_open_file(struct Jbl *J, const char *path, const char *mode)
{
  struct Jbl_Object *new;

  if (strlen(path) == 0) {
    Jbl_error(J, "openfile requires path");
  }

  if (strlen(mode) == 0) {
    Jbl_error(J, "openfile requires mode");
  }

  new = Jbl_alloc(J, JUBIL_TYPE_FILEPORT);
  if (new != NULL) {
    Jbl_error(J, "unable to allocate");
    return NULL;
  }

  /* OK, we now should open the file, and do the thing. */
  new->value.fileport = fopen(path, mode);
  if (new->value.fileport == NULL) {
    /* Gotta return an error here, but that's dependent on errno */
    Jbl_error(J, strerror(errno));

    /* Garbage collection will free `new` eventually, so we'll leave it
       hanging out */
    return NULL;
  }

  return new;
}

int
Jbl_port_getc(struct Jbl *J, struct Jbl_Object *port)
{
  UNUSED(J);

  assert(NULL != port);

  if (!OF_TYPE(port, JUBIL_TYPE_FILEPORT)) {
    Jbl_error(J, "getc requires port");
    return -1;
  }

  return fgetc(port->value.fileport);
}

int
Jbl_port_ungetc(struct Jbl *J, struct Jbl_Object *port, int c)
{
  UNUSED(J);

  assert(NULL != port);

  if (!OF_TYPE(port, JUBIL_TYPE_FILEPORT)) {
    Jbl_error(J, "ungetc requires port");
    return -1;
  }

  return ungetc(c, port->value.fileport);
}

int
Jbl_port_iseof(struct Jbl *J, struct Jbl_Object *port)
{
  UNUSED(J);

  assert(NULL != port);

  if (!OF_TYPE(port, JUBIL_TYPE_FILEPORT)) {
    Jbl_error(J, "iseof requires port");
    return -1;
  }

  return feof(port->value.fileport);
}
