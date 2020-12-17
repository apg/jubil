#include <stdlib.h>
#include <stdio.h>
#include "jubil.h"
#include "internal.h"

int
main(int argc, char **argv)
{
  UNUSED(argc);
  UNUSED(argv);

  struct Jbl J;
  if (!Jbl_init(&J)) {
    fprintf(stderr, "Unable to initialize Jubil.");
    exit(1);
  }

  struct Jbl_Object *new = Jbl_alloc(&J, JUBIL_TYPE_FILEPORT);
  if (new == NULL) {
    fprintf(stderr, "unable to allocate file port\n");
    exit(-1);
  }

  new->value.fileport = stdin;

  struct Jbl_Object *read = Jbl_read(&J, new);
  if (read != NULL) {
    printf("Read the value: %ld\n", read->value.integer);
  } else {
    printf("No integer.\n");
  }

  return 0;
}
