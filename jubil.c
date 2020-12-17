#include <assert.h>
#include <stdlib.h>
#include "jubil.h"

int
Jbl_init(struct Jbl *J)
{
  J->gc_context = calloc(1, sizeof(J->gc_context));
  if (J->gc_context == NULL) {
    return 0;
  }

  J->gc_context->last_alloc = NULL;
  J->gc_context->gcs = 0;
  J->gc_context->allocations = 0;
  J->gc_context->allocated_bytes = 0;
  J->gc_context->deallocations = 0;
  J->gc_context->deallocated_bytes = 0;

  return 0;
}

/**
 * Jbl_Alloc allocates an object from the heap and places it into a "managed"
 * place for garbage collection purposes
 *
 * @requires: J != NULL
 * @requires: type is a valid Jbl_Type
 *
 * Unless allocation fails...
 * @ensures: new->previous is the old J->last_alloc
 * @ensures: J->last_alloc = new
 *
 * When allocation fails, NULL is returned.
 */
struct Jbl_Object *
Jbl_alloc(struct Jbl *J, enum Jbl_Type type)
{
  assert(J != NULL);
  assert(type < JUBIL_NUM_TYPES);

  struct Jbl_Object *new;
  new = calloc(1, sizeof(*new));
  if (new == NULL) {
    /* There's a world where this doesn't return, but we should
       return anyway */
    Jbl_error(J, "unable to allocate");
    return NULL;
  }

  J->gc_context->allocations++;
  /* TODO(apg): We'll have to remember to take into consideration
     strdup and things as we go farther down the line */
  J->gc_context->allocated_bytes += sizeof(*new);

  struct Jbl_Object *tmp = J->gc_context->last_alloc;
  J->gc_context->last_alloc = new;

  new->previous = tmp;
  new->type = type;
  return new;
}
