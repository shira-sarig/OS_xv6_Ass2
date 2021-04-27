#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;
  int signum;

  if(argint(0, &pid) < 0 || argint(1, &signum))
    return -1;
  
  if(signum < 0 || signum > NSIGS)
    return -1;
    
  return kill(pid, signum);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_sigprocmask(void)
{
  uint sigmask;

  argaddr(0, (uint64*)&sigmask);
  return sigprocmask(sigmask);
}

uint64
sys_sigaction(void)
{
  int signum;
  uint64 act;
  uint64 old;

  if(argint(0, &signum) < 0)
    return -1;
  argaddr(1, &act);
  argaddr(2, &old);

  return sigaction(signum, act, old);
}

uint64
sys_sigret(void)
{
  sigret();
  return 0;
}

uint64
sys_bsem_alloc(void)
{
  return 0;
}

uint64
sys_bsem_free(void)
{
  return 0;
}

uint64
sys_bsem_down(void)
{
  return 0;
}

uint64
sys_bsem_up(void)
{
  return 0;
}

