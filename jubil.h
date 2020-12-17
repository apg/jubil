#ifndef JUBIL_H_
#define JUBIL_H_

#include <stdio.h>

#define Jbl_error(x, y) do { fputs(y, stderr); putc('\n', stderr); exit(1); } while (0);

enum Jbl_Type {
  JUBIL_TYPE_NULL,
  JUBIL_TYPE_INT,
  JUBIL_TYPE_FILEPORT,
  JUBIL_NUM_TYPES
};

struct Jbl_Object {
  enum Jbl_Type type;

  /* TODO(apg): There's unlikely to be more than a few flags. Can likely combine this with `type` */
  int flags;
  union {
    long integer;
    FILE *fileport;
  } value;
  struct Jbl_Object *previous;
};

struct Jbl_GC_Context {
  struct Jbl_Object *last_alloc;
  size_t gcs; /* number of GCs that have run */
  size_t allocations;
  size_t allocated_bytes;
  size_t deallocations;
  size_t deallocated_bytes;
};

struct Jbl {
  struct Jbl_GC_Context *gc_context;
};

/* Jbl_init initializes a Jbl struct for future use. */
int Jbl_init(struct Jbl *J);

/* Jbl_Alloc heap allocates a Jbl_Object and initializes the type flag */
struct Jbl_Object *Jbl_alloc(struct Jbl *J, enum Jbl_Type);

struct Jbl_Object *Jbl_open_file(struct Jbl *J, const char *path, const char *mode);
int Jbl_port_getc(struct Jbl *J, struct Jbl_Object *port);
int Jbl_port_ungetc(struct Jbl *J, struct Jbl_Object *port, int ch);
int Jbl_port_iseof(struct Jbl *J, struct Jbl_Object *port);

struct Jbl_Object *Jbl_read(struct Jbl *J, struct Jbl_Object *port);

#endif
