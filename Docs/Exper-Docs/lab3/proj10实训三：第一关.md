# proj10实训三：进程管理

# 第一关第一个用户进程的创建

------

## 1 知识阅读

​	本关任务为加载一个应用程序并执行。进程创建的一般原理如下：

​	当一个用户进程创建时，**do_execve()**函数调用**load_icode()**函数（kern/process/proc.c中）来加载并解析一个处于内存中的ELF执行文件格式的应用程序，建立相应的用户内存空间来放置应用程序的代码段、数据段等，且要设置好**proc_struct**结构中的成员变量**trapframe**中的内容，确保在执行此进程后，能够从应用程序设定的起始执行地址开始执行。

```c
	/* LAB3:EXERCISE1 YOUR CODE
    * should set tf_era,tf_regs.reg_r[LOONGARCH_REG_SP],tf->tf_prmd
    * NOTICE: If we set trapframe correctly, then the user level process can return to USER MODE from kernel and enable interrupt. So
    *          tf->tf_prmd should be PLV_USER | CSR_CRMD_IE
    *          tf->tf_era should be the entry point of this binary program (elf->e_entry)
    *          tf->tf_regs.reg_r[LOONGARCH_REG_SP] should be the top addr of user stack (USTACKTOP)
    */
```

​	以上为代码补全部分提示。阅读可以发现函数大致完成过程：先完成一个优先级的转变，从内核态切换到用户态（即特权级从0到3）。先解释一下代码中变量tf的含义：tf是一个是中断帧的指针，指向内核栈的某个位置，当进程从用户空间跳到内核空间时，中断帧记录了进程在被中断前的状态。当内核需要跳回用户空间时，需要调整中断帧以恢复让进程继续执行的各寄存器值。tf的定义在kern/trap/trap.h中。优先级转变的具体做法如下：

​	① 将tf_era设置为用户态，这个定义在kern/mm/memlayout.h中，有一个宏定义已经定义了用户态和内核态；

​	② status也需要设置为用户态；

​	③ 需要将tf_regs.reg_r[LOONGARCH_REG_SP]设置为用户栈的栈顶，直接使用之前建立用户栈时的参数USTACKTOP就可以；

​	④ 最后打开中断。

## 2 实验步骤

### 2.1 代码补全

​	由于原函数较长，这里只给出待补全代码：

```c
	#ifdef LAB3_EX1
    /* LAB3:EXERCISE1 YOUR CODE
     * should set tf_era,tf_regs.reg_r[LOONGARCH_REG_SP],tf->tf_prmd
     * NOTICE: If we set trapframe correctly, then the user level process can return to USER MODE from kernel and enable interrupt. So
     *          tf->tf_prmd should be PLV_USER | CSR_CRMD_IE
     *          tf->tf_era should be the entry point of this binary program (elf->e_entry)
     *          tf->tf_regs.reg_r[LOONGARCH_REG_SP] should be the top addr of user stack (USTACKTOP)
     */

    #endif
```

​	以下为等会需要使用的相关宏定义与变量

- `PLV_USER`：这是一个宏，代表用户模式的优先级级别（通常是3）。
- `CSR_CRMD_IE`：这是一个宏，代表允许中断的标志。
- `LOONGARCH_REG_SP`：这是一个宏，代表LoongArch架构中栈指针寄存器的索引。
- `USTACKTOP`：这是一个宏或变量，代表用户栈的顶部地址。
- `elf`：这是一个指针，指向当前正在执行的ELF程序的头部信息。
- `tf`：这是一个指向陷阱框架结构体的指针。

​	按照知识阅读中整理的步骤，可以完善代码：

```c
	tf->tf_era = elf->e_entry;
    tf->tf_regs.reg_r[LOONGARCH_REG_SP] = USTACKTOP;
    uint32_t status = 0;
    status |= PLV_USER; // set plv=3(User Mode)
    status |= CSR_CRMD_IE;
    tf->tf_prmd = status;
```

​	首先，需要将陷阱框架中的`tf_era`字段设置为ELF文件的入口地址，这样当从内核返回用户空间时，程序可以从正确的位置继续执行。随后需要将陷阱框架中的栈指针寄存器设置为用户栈的顶部地址，确保返回用户空间时，栈是正确的。将状态变量设置为用户模式与允许中断。最后将设置好的状态变量赋值给陷阱框架的`tf_prmd`字段，这样当返回用户空间时，CPU会处于用户模式并且允许中断。

​	至此，创建一个用户态并加载应用程序后，应用程序执行的全过程流程如下：

​	① 调用mm_create函数申请进程内存管理的数据结构mm所需的内存空间，并对mm初始化；

​	② 调用setup_pgdir函数，申请一个页目录表所需的一个页大小的内存空间，并把描述ucore内核虚空间映射的内核页表的内容拷贝到此新目录表中，最后mm->pgdir指向此页目录表，也就是进程新的页目录表了，且能够正确映射内核；

​	③ 根据应用程序执行码的起始位置来解析此ELF格式的执行程序，并调用mm_map函数根据ELF格式的执行程序说明的各个段（代码段、数据段、BSS段等）的起始位置和大小建立对应的vma结构，并把vma插入到mm结构中，从而表明了用户进程的合法用户态虚拟地址空间；

​	④ 调用根据执行程序各个段的大小分配物理内存空间，并根据执行程序各个段的起始位置确定虚拟地址，在页表中建立好物理地址和虚拟地址的映射关系。然后把执行程序各个段的内容拷贝到相应的内核虚拟地址中，至此应用程序执行码和数据已经根据编译时设定地址放置到虚拟内存中了；

​	⑤ 需要给用户进程设置用户栈，为此调用mm_mmap函数建立用户栈的vma结构，明确用户栈的位置在用户虚空间的顶端，大小为256个页，即1MB，并分配一定数量的物理内存且建立好栈的虚地址-物理地址映射关系；
​	⑥ 至此，进程内的内存管理vma和mm数据结构已经建立完成，于是把mm->pgdir赋值到cr3寄存器中，即更新了用户进程的虚拟内存空间，此时的initproc已经被hello的代码和数据覆盖，成为了第一个用户进程，但此时这个用户进程的执行现场还没建立好；

​	⑦ 先清空进程的中断帧，再重新设置进程的中断帧，使得在执行中断返回指令“iret”后，能够让CPU转到用户态特权级，并回到用户态内存空间，使用用户态的代码段、数据段和堆栈，且能够跳转到用户进程的第一条指令执行，并确保在用户态能够响应中断。

​	至此，CPU让这个应用程序最终以用户态执行起来。

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab311.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab312.png" style="zoom:67%;" />

​	评测如图，成功通过评测。

## 3 实验代码

```c
#ifdef LAB3_EX1
    /* LAB3:EXERCISE1 YOUR CODE
     * should set tf_era,tf_regs.reg_r[LOONGARCH_REG_SP],tf->tf_prmd
     * NOTICE: If we set trapframe correctly, then the user level process can return to USER MODE from kernel and enable interrupt. So
     *          tf->tf_prmd should be PLV_USER | CSR_CRMD_IE
     *          tf->tf_era should be the entry point of this binary program (elf->e_entry)
     *          tf->tf_regs.reg_r[LOONGARCH_REG_SP] should be the top addr of user stack (USTACKTOP)
     */
    tf->tf_era = elf->e_entry;
    tf->tf_regs.reg_r[LOONGARCH_REG_SP] = USTACKTOP;
    uint32_t status = 0;
    status |= PLV_USER; // set plv=3(User Mode)
    status |= CSR_CRMD_IE;
    tf->tf_prmd = status;
#endif
```

​	在proc.c的load_icode()函数中寻找到EX3字样，将以上代码段填入即可。