#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

int
exec(char *path, char **argv)
{
  char *s, *last;
  int i, off;
  uint argc, sz, ssz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;
  struct proc *curproc = myproc();

  begin_op();

  if((ip = namei(path)) == 0){
    end_op();
    cprintf("exec: fail\n");
    return -1;
  }
  ilock(ip);
  pgdir = 0;

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pgdir = setupkvm()) == 0)
    goto bad;

  // Load program into memory.
  sz = 0;
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
      goto bad;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  end_op();
  ip = 0;


  // 
  // Old mem:
  // 	--------- USER MEMORY:
  // 	code
  // 	stack 
  // 	heap
  // 	--------- KERNEL MEMORY:
  // 	KERN_BASE
  // 	  
  //
  //  sz: tracks size of stack and heap
  //
  // ===================================
  //
  // New mem:
  // 	--------- USER MEMORY:
  // 	code
  // 	heap
  // 	...
  // 	stack
  // 	--------- KERNEL MEMORY:
  // 	KERN_BASE at 0x80000000
  //
  //  sz: tracks size of stack and heap
  //  ssz: tracks size of the stack 
  //  hsz: tracks size of the heap (should work like old sz)
  //
  //  Could possibly use arithmetic like
  //    st = sz - ht
  //  but it doesn't hurt to just track everything
  //

  /* NEW STACK ALLOCATION */
  // Allocate stack page just before KERN_BASE
  
  // function reuse: allocate, map, and fill page with PTEs
  uint stack_loc = KERNBASE - 1*PGSIZE;
  if (allocmap(pgdir, stack_loc) < 0)
    goto bad;

  // Create guard page below stack
  uint guard_loc = KERNBASE - 2*PGSIZE;
  if (allocmap(pgdir, guard_loc) < 0)
    goto bad;
  
  clearpteu(pgdir, (char*)guard_loc);
  
  sp = KERNBASE;
  ssz = PGSIZE;



  /* OLD STACK ALLOCATION 
  // Allocate two pages at the next page boundary.
  // Make the first inaccessible.  Use the second as the user stack.
  sz = PGROUNDUP(sz);
  if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
    goto bad;
  clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
  sp = sz;
  */
  
  // cprintf("\tbefore:\n");
  // printproc();
  // cprintf("sp = 0x%x = %d (dec);\n", sp, sp);

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    // cprintf("\tduring:\n");
    // printproc();
    // cprintf("sp = 0x%x = %d (dec);\n", sp, sp);
    // cprintf("stack begin: 0x%x, stack end: 0x%x\n", KERNBASE, KERNBASE - curproc->ssz);

    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(curproc->name, last, sizeof(curproc->name));

  // Commit to the user image.
  oldpgdir = curproc->pgdir;
  curproc->pgdir = pgdir;
  curproc->sz = sz;
  
  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = sp;
  
  curproc->ssz = ssz;
  curproc->hsz = sz;

  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = sp;
  
  // printproc();

  switchuvm(curproc);
  freevm(oldpgdir);
  return 0;

 bad:
  if(pgdir)
    freevm(pgdir);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}
