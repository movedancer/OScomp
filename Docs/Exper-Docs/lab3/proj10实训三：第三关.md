# proj10实训三：进程管理

# 第三关理解进程执行相关函数的实现

------

## 1 知识阅读

​	本关要求阅读分析源代码，理解进程执行 fork()/exec()/wait()/exit()函数的实现，以及对应的系统调用的实现。主要完成三个选择题。

### 1.1 fork()函数的实现

​	fork（）：fork的功能是创建一个新进程，具体地说是创建一个新进程所需的控制信息。
​	该函数的调用过程为：fork->SYS_fork()->do_fork()+wakeup_proc()。首先当程序执行 fork() 时，fork()使用了系统调用SYS_fork()，而系统调用SYS_fork()则主要是由do_fork()和wakeup_proc()来完成的。

```c
 do_fork(uint32_t clone_flags, uintptr_t stack, struct trapframe *tf) {
    int ret = -E_NO_FREE_PROC;
    struct proc_struct *proc;
    if (nr_process >= MAX_PROCESS) {
    goto fork_out;
    }
    ret = -E_NO_MEM;
```

​	该函数传入的参数有clone_flags（标志是否进行克隆），stack（记录了分配给该进程/线程的内核栈的位置），tf（中断帧的指针），返回类型为int，值为进程号。

​	首先给ret赋值-E_NO_FREE_PROC，尝试为进程分配内存，然后使用struct proc_struct *proc 定义新进程，再和最大进程数MAX_PROCESS比较，如果超出则返回到fork_out代码块则有

```c
if ((proc = alloc_proc()) == NULL) {
 goto fork_out;
 }
 proc->parent = current;
 if(setup_kstack(proc)){
 goto bad_fork_cleanup_proc;
 }
 
 if (copy_fs(clone_flags, proc) != 0) {
 goto bad_fork_cleanup_kstack;
 } 
 if (copy_mm(clone_flags, proc)){
 goto bad_fork_cleanup_fs;
 }
 copy_thread(proc, (uint32_t)stack, tf);
```

​	之后调用alloc_proc() 函数为子进程proc分配内存并初始化进程控制块，将创建出来的子进程proc的parent指针指向当前进程current，调用setup_stack() 函数进行分配并初始化内核栈，为内核进程建立栈空间。然后调用copy_fs()函数和copy_mm()函数根据clone_flags标志分别复制进程的文件相关信息和进程的内存管理结构。再copy_thread()函数设置进程在内核(将来也包括用户态)正常运行和调度所需的中断帧（tf）和执行上下文（stack）。

```c
	proc->pid = get_pid();
    hash_proc(proc);
    set_links(proc);
    wakeup_proc(proc);
    ret = proc->pid;
    fork_out:
    return ret;
```

​	上述操作执行完后，使用get_pid()为进程分配一个PID，再调用hash_proc()为进程建立hash映射，set_links()把设置好的进程控制块放入hash_list 和 proc_list 两个全局进程链表中。最后调用wakeup_proc()函数唤醒该进程并设置其状态为就绪态，并将记录了该进程的进程号proc->pid返回。

### 1.2 exec()函数的实现

​	exec()的功能是在已经存在的进程的上下文中运行新的可执行文件，替换先前的可执行文件。
​	调用过程为： SYS_exec()->do_execve()。当应用程序执行的时候，会调用SYS_exec() 系统调用，而当 ucore 收到此系统调用的时候，则会使用do_execve()函数来实现。

```c
	do_execve(const char *name, size_t len, unsigned char *binary, size_t size) {
    struct mm_struct *mm = current->mm;
    // 检查name指针是否是指向用户空间
    if (len > PROC_NAME_LEN) {
    len = PROC_NAME_LEN;
    }
    // 若合法，copy到内核中
    char local_name[PROC_NAME_LEN + 1];
    memset(local_name, 0, sizeof(local_name));
    memcpy(local_name, name, len);
```

​	首先检查用户态虚拟内存空间是否合法（也就是检查name指针是否是指向用户空间），如果合法则执行memset()和memcpy()函数将其复制到内核中。

```c
	if (mm != NULL) {
       lcr3(boot_cr3);
       if (mm_count_dec(mm) == 0) {
         exit_mmap(mm);
         put_pgdir(mm);
         mm_destroy(mm);
        }
       current->mm = NULL;
    }
```

​	后续判断mm内存管理指针如果不为NULL，则调用lcr3()函数设置页表为内核空间页表，再调用mm_count_dec()判断mm的引用计数减1后是否为0，如果为0则表明没有进程再需要此进程所占用的内存空间，则使用exit_mmap(),mm_destroy()等函数释放进程所占用户空间内存和进程页表本身所占空间，最后把当前进程的mm内存管理指针置为NULL。

```c
	int ret;
    if ((ret = load_icode(binary, size)) != 0) {
          goto execve_exit;
    }
    set_proc_name(current, local_name);
    return 0;
```

​	最后调用load_icode()加载应用程序执行码（binary）到当前进程的新创建的用户态虚拟空间中,调用set_proc_name()设置进程的名字。

### 1.3 wait()函数的实现

​	wait()的功能是等待子进程结束，从而释放子进程占用的资源。
​	调用过程为： SYS_wait()->do_wait()。首先当程序执行 wait()时，wait()使用了系统调用SYS_wait()，而系统调用SYS_wait()则主要是由do_wait()来完成的

```c
	repeat:
      haskid = 0;
      if (pid != 0) {
        proc = find_proc(pid);
        if (proc != NULL && proc->parent == current) {
          haskid = 1;
          if (proc->state == PROC_ZOMBIE) {
            goto found;
          }
        }
      }
      else {
        proc = current->cptr;
        for (; proc != NULL; proc = proc->optr) {
          haskid = 1;
          if (proc->state == PROC_ZOMBIE) {
            goto found;
          }
        }
      }
```

​	在repeat代码块中，第一个if-else判断分支中先判断pid是否为0，如果pid！=0，则使用find_proc()函数找到进程id为pid的处于退出状态的子进程，如果pid==0，则在for循环中随意找一个处于退出状态的子进程。

```c
	if (haskid) {
      current->state = PROC_SLEEPING;
      current->wait_state = WT_CHILD;
      schedule();
      if (current->flags & PF_EXITING) {
        do_exit(-E_KILLED);
      }
      goto repeat;
    }
    return -E_BAD_PROC;
```

​	在上面的if-else判断分支中，如果子进程的执行状态为 PROC_ZOMBIE，都会跳转到found代码块，不会进入到这里的if语句中，因此如果子进程的执行状态不为 PROC_ZOMBIE，则说明子进程还没有退出，则父进程重新进入睡眠（设置当前状态为PROC_SLEEPING（睡眠），睡眠原因为WT_CHILD（等待子进程退出）），调用schedule()函数选择新的进程执行，自己睡眠等待，如果被唤醒则再执行goto repeat进行重复寻找。

```c
	found:
      if (proc == idleproc || proc == initproc) {
        panic("wait idleproc or initproc.\n");
      }
      if (code_store != NULL) {
        *code_store = proc->exit_code;
      }
      local_intr_save(intr_flag);
      {
        unhash_proc(proc);
        remove_links(proc);
      }
      local_intr_restore(intr_flag);
      put_kstack(proc);
      kfree(proc);
      return 0;
```

​	如果此子进程的执行状态为 PROC_ZOMBIE，表明此子进程处于退出状态，此时进入found代码块，使用unhash_proc()（消除哈希映射）和remove_links()函数把子进程控制块从两个进程队列proc_list和hash_list中删除，并调用put_kstack()和kfree()释放子进程的内核堆栈和进程控制块。自此，子进程才彻底地结束了它的执行过程，它所占用的所有资源均已释放。

### 1.4 exit()函数的实现

​	首先，exit()函数会把一个退出码error_code传递给ucore，ucore通过执行内核函数do_exit()来完成对当前进程的退出处理，主要工作就是回收当前进程所占的大部分内存资源，并通知父进程完成最后的回收工作。

```c
	struct mm_struct *mm = current->mm;
        if (mm != NULL) {
            lcr3(boot_cr3);
            if (mm_count_dec(mm) == 0) {
                exit_mmap(mm);
                put_pgdir(mm);
                mm_destroy(mm);
            }
            current->mm = NULL;
        }
```

​	首先判断如果current->mm != NULL，表示是用户进程，则开始回收此用户进程所占用的用户态虚拟内存空间。 a) 首先执行“lcr3(boot_cr3)”，切换到内核态的页表上，这样当前用户进程目前只能在内核虚拟地址空间执行了，这是为了确保后续释放用户态内存和进程页表的工作能够正常执行。 b) 如果当前进程控制块的成员变量mm的成员变量mm_count减1后为0（表明这个mm没有再被其他进程共享，可以彻底释放进程所占的用户虚拟空间了。），则开始回收用户进程所占的内存资源。 i. 调用exit_mmap()函数释放current->mm->vma链表中每个vma描述的进程合法空间中实际分配的内存，然后把对应的页表项内容清空，最后还把页表所占用的空间释放并把对应的页目录表项清空。 ii. 调用put_pgdir()函数释放当前进程的页目录所占的内存。 iii. 调用mm_destroy()函数释放mm中的vma所占内存，最后释放mm所占内存。 c) 此时设置current->mm为NULL，表示与当前进程相关的用户虚拟内存空间和对应的内存管理成员变量所占的内核虚拟内存空间已经回收完毕。 1 current->state = PROC_ZOMBIE; 2 current->exit_code = error_code;
​	这时，设置当前进程的执行状态current->state=PROC_ZOMBIE，当前进程的退出码current->exit_code=error_code。此时当前进程已经不能被调度了，需要此进程的父进程来做最后的回收工作（即回收描述此进程的内核栈和进程控制块）。

```c
bool intr_flag;
     struct proc_struct *proc;
     local_intr_save(intr_flag);
     {
         proc = current->parent;
         if (proc->wait_state == WT_CHILD) {
             wakeup_proc(proc);
         }
```

​	如果当前进程的父进程current->parent处于等待子进程状态：即current->parent->wait_state==WT_CHILD，则唤醒父进程（执行“wakup_proc(current->parent)”），让父进程帮助自己完成最后的资源回收。

```c
while (current->cptr != NULL) {
             proc = current->cptr;
             current->cptr = proc->optr;
     
             proc->yptr = NULL;
             if ((proc->optr = initproc->cptr) != NULL) {
                 initproc->cptr->yptr = proc;
             }
             proc->parent = initproc;
             initproc->cptr = proc;
             if (proc->state == PROC_ZOMBIE) {
                 if (initproc->wait_state == WT_CHILD) {
                     wakeup_proc(initproc);
                 }
             }
         }
     }
     local_intr_restore(intr_flag);
     
     schedule();
     panic("do_exit will not return!! %d.\n", current->pid);
 }
```

​	如果当前进程还有子进程，则需要把这些子进程的父进程指针设置为内核线程initproc，且各个子进程指针需要插入到initproc的子进程链表中。如果某个子进程的执行状态是PROC_ZOMBIE，则需要唤醒initproc来完成对此子进程的最后回收工作。 最后执行schedule()函数，选择新的进程执行。

## 2 题目解答

### 2.1 <img src="F:\study\操作系统\OS_comp\picture\lab331.png" style="zoom: 67%;" />

解答：在第一关的注释中，有tf->tf_era should be the entry point of this binary program (elf->e_entry)，可以得知，用户进程的ELF执行文件的起始地址应当存在tf_ra中，这是整个程序的起始点。当进程进行状态转化的时候，堆栈寄存器的指针是存在tf_regs中，第一关的注释tf->tf_regs.reg_r[LOONGARCH_REG_SP] should be the top addr of user stack (USTACKTOP)同样告诉了我们这一点。

### 2.2 <img src="F:\study\操作系统\OS_comp\picture\lab332.png" style="zoom:67%;" />

解答：ucore实现进程退出需要完成以上四个选项。需要回收进程占用的虚拟空间，完成资源回收；还需要修改进程状态，让进程处于终止状态；还需要修改子进程的指针，如果父进程终止了子进程也需要相应修改；结束进程CPU不会处于停止状态，其还需要调度选择下一个执行的进程。

### 2.3 <img src="F:\study\操作系统\OS_comp\picture\lab333.png" style="zoom:67%;" />

解答：父进程创建子进程的系统调用为fork()函数。

## 3 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab334.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab335.png" style="zoom:67%;" />

​	评测如图，成功通过。