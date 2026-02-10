#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "proc mem size: %d\n", get_mem_size());

  printf(1, "malloc 10kb\n");

  char *ptr = malloc(1024*10);
  
  printf(1, "proc mem size: %d\n", get_mem_size());

  printf(1, "free ptr\n");
  
  free(ptr);
  
  printf(1, "proc mem size: %d\n", get_mem_size());
  exit();
}
