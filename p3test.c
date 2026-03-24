//Test case code for the address translation.

#include "types.h"
#include "stat.h"
#include "user.h"

int gData = 0;
int gun;

int main(void) {
  int i =0;
  char *p = 0;

  p = malloc(16);
  
  printf(1,"Code vadd %x phy: %x\n\n", main, vaddr2phyaddr((char *)main));
  printf(1,"Stack vadd %x phy: %x\n\n", &i, vaddr2phyaddr((char *)&i));
  printf(1,"Data vadd %x phy: %x\n\n", &gData, vaddr2phyaddr((char *)&gData));
  printf(1,"BSS vadd %x phy: %x\n\n", &gun, vaddr2phyaddr((char *)&gun));
  printf(1,"Heap vadd %x phy: %x\n\n", p, vaddr2phyaddr(p));

  exit();
}