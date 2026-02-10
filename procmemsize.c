#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "proc mem size: %d\n", get_mem_size());

  exit();
}
