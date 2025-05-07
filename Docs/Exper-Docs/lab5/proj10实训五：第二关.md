# proj10实训五：**文件系统**

# 第二关基于文件系统的执行程序机制的实现

------

## 1 知识阅读

​	该关卡的主要任务是补充和完善项目工程文件kern/process/proc.c中的load_icode()函数。该函数的目标功能为：将指定elf格式文件加载到内存中执行。

​	load_icode()函数的8个部分：

1. 建立内存管理器
2. 建立页目录
3. 将文件逐段加载到内存中(注意设置虚拟地址与物理地址之间的映射)
4. 建立相应的虚拟内存映射表
5. 建立并初始化用户堆栈
6. 处理用户栈中传入的参数
7. 设置用户进程的中断帧
8. 错误处理

## 2 实验步骤

### 2.1 代码补全

​	本关给出代码注释如下：

```c
/* LAB4:EXERCISE2 YOUR CODE  HINT:how to load the file with handler fd  in to process's memory? how to setup argc/argv?
     * MACROs or Functions:
     *  mm_create        - create a mm
     *  setup_pgdir      - setup pgdir in mm
     *  load_icode_read  - read raw data content of program file
     *  mm_map           - build new vma
     *  pgdir_alloc_page - allocate new memory for  TEXT/DATA/BSS/stack parts
     *  lcr3             - update Page Directory Addr Register -- CR3
     */
        /* (1) create a new mm for current process
     * (2) create a new PDT, and mm->pgdir= kernel virtual addr of PDT
     * (3) copy TEXT/DATA/BSS parts in binary to memory space of process
     *    (3.1) read raw data content in file and resolve elfhdr
     *    (3.2) read raw data content in file and resolve proghdr based on info in elfhdr
     *    (3.3) call mm_map to build vma related to TEXT/DATA
     *    (3.4) callpgdir_alloc_page to allocate page for TEXT/DATA, read contents in file
     *          and copy them into the new allocated pages
     *    (3.5) callpgdir_alloc_page to allocate pages for BSS, memset zero in these pages
     * (4) call mm_map to setup user stack, and put parameters into user stack
     * (5) setup current process's mm, cr3, reset pgidr (using lcr3 MARCO)
     * (6) setup trapframe for user environment (You have done in LAB3)
     * (7) store argc and kargv to a0 and a1 register in trapframe
     * (8) if up steps failed, you should cleanup the env.
     */
```

​	通过注释可以获得函数的构建思路：

1. **创建新的内存管理（mm）**：

   调用 `mm_create` 为当前进程创建一个新的 mm。

2. **设置页目录**：

   创建一个新的 PDT，并设置 mm 的 `pgdir` 指向内核虚拟地址的 PDT。

3. **加载程序文件**：

   调用 `load_icode_read` 读取二进制文件的原始数据内容，并解析 ELF 头。

   根据 ELF 头中的信息解析程序头（proghdr）。

4. **映射 TEXT/DATA**：

   使用 `mm_map` 创建与 TEXT/DATA 相关的 VMA。

   使用 `pgdir_alloc_page` 为 TEXT/DATA 分配页面，并复制文件内容到新分配的页面中。

5. **初始化 BSS**：

   使用 `pgdir_alloc_page` 为 BSS 分配页面，并将这些页面的内存设置为零。

6. **设置用户栈**：

   使用 `mm_map` 设置用户栈，并将参数放入用户栈中。

7. **更新页目录基址**：

   使用 `lcr3` 宏更新 CR3 寄存器，设置当前进程的页目录基址。

8. **设置 trapframe**：

   为用户环境设置 trapframe，包括存储 argc 和 argv 到 trapframe 的 a0 和 a1 寄存器中。

9. **错误处理**：

   如果任何步骤失败，需要进行清理，释放分配的资源。

​	可以补全代码如下：

#### 2.2.1 建立内存管理器

```c
	if(current->mm != NULL)//检查进程的内存管理器是否清空
        panic("load_icode: current->mm must be empty.\n");
    int ret = -E_NO_MEM;
    struct mm_struct *mm;
    if((mm = mm_create()) == NULL)//建立内存管理器
        goto bad_mm;
```

​	首先检查当前进程（`current`）是否已经有一个内存管理器（`mm`）。如果`current->mm`不是`NULL`，则表示内存管理器不为空，代码将调用`panic`函数，输出错误信息并导致系统崩溃。

​	随后定义一个`mm_struct`类型的指针`mm`，用于指向新创建的内存管理器。调用`mm_create`函数尝试创建一个新的内存管理器，并检查返回值。如果`mm_create`返回`NULL`，表示创建失败，通常是由于内存不足（`-E_NO_MEM`）。

​	如果内存管理器创建失败，代码将跳转到`bad_mm`标签，这个标签处的代码将处理错误情况，例如释放已分配的资源或返回错误代码。

#### 2.2.2 建立页目录

```c
if(setup_pgdir(mm) != 0)//设置内存管理器的页目录表
     goto bad_pgdir_cleanup_mm;
```

#### 2.2.3 将文件逐段加载到内存中

```c
struct __elfhdr ___elfhdr__;  // 定义一个临时变量来存储ELF文件头
  struct elfhdr32 __elf, *elf = &__elf;  // 定义一个elfhdr32结构体变量来存储解析后的ELF文件头，并设置一个指针指向它
  // 尝试从文件描述符fd中读取ELF文件头，如果失败则跳转到bad_elf_cleanup_pgdir标签处理错误
  if((ret = load_icode_read(fd, &___elfhdr__, sizeof(struct __elfhdr), 0)) != 0)
      goto bad_elf_cleanup_pgdir;

  _load_elfhdr((unsigned char*)&___elfhdr__, &__elf);  // 解析读取到的ELF文件头
  if(elf->e_magic != ELF_MAGIC) // 检查ELF文件头的魔术数字是否正确，不正确则设置错误码并跳转到bad_elf_cleanup_pgdir标签处理错误
  {
      ret = -E_INVAL_ELF;
      goto bad_elf_cleanup_pgdir;
}
  struct proghdr _ph, *ph = &_ph;  // 定义一个临时变量来存储程序头，并设置一个指针指向它
  uint32_t vm_flags, phnum;  // 定义变量来存储虚拟内存标志和程序头的数量
  uint32_t perm = 0;  // 定义权限变量并设置用户权限
  struct Page *page;
  for (phnum = 0; phnum < elf->e_phnum; phnum ++) // 遍历所有的程序头
  {
      off_t phoff = elf->e_phoff + sizeof(struct proghdr) * phnum;  // 计算当前程序头的文件偏移量
      if((ret = load_icode_read(fd, ph, sizeof(struct proghdr), phoff)) != 0)  // 尝试读取程序头，如果失败则跳转到bad_cleanup_mmap标签处理错误
          goto bad_cleanup_mmap;
      if(ph->p_type != ELF_PT_LOAD)  // 如果程序头类型不是EPT_LOAD，则跳过
          continue ;
      if(ph->p_filesz > ph->p_memsz)  // 如果程序头的文件大小大于内存大小，则设置错误码并跳转到bad_cleanup_mmap标签处理错误
      {
          ret = -E_INVAL_ELF;
          goto bad_cleanup_mmap;
      }

      vm_flags = 0;  // 初始化虚拟内存标志
      // 设置可执行、可写、可读权限
      perm |= PTE_U;
      if(ph->p_flags & ELF_PF_X)
          vm_flags |= VM_EXEC;
      if(ph->p_flags & ELF_PF_W)
          vm_flags |= VM_WRITE;
      if(ph->p_flags & ELF_PF_R)
          vm_flags |= VM_READ;
      if(vm_flags & VM_WRITE)  // 如果可写，则设置写权限
          perm |= PTE_W;   
      if((ret = mm_map(mm, ph->p_va, ph->p_memsz, vm_flags, NULL)) != 0)  // 尝试映射虚拟内存，如果失败则跳转到bad_cleanup_mmap标签处理错误
          goto bad_cleanup_mmap;
      off_t offset = ph->p_offset;  // 计算偏移量和大小
      size_t off, size;
      uintptr_t start = ph->p_va, end, la = ROUNDDOWN_2N(start, PGSHIFT);
      end = ph->p_va + ph->p_filesz;
      while (start < end)  // 读取程序段到内存 
      { // 分配页面，如果失败则设置错误码并跳转到bad_cleanup_mmap标签处理错误
          if((page = pgdir_alloc_page(mm->pgdir, la, perm)) == NULL) 
          {
              ret = -E_NO_MEM;
              goto bad_cleanup_mmap;
          }
          off = start - la, size = PGSIZE - off, la += PGSIZE;  //计算偏移和大小
          if(end < la)
              size -= la - end;
          if((ret = load_icode_read(fd, page2kva(page) + off, size, offset)) != 0)  // 读取数据到页面，如果失败则跳转到bad_cleanup_mmap标签处理错误
              goto bad_cleanup_mmap;
          fence_i(page2kva(page)+off, size);  // 确保数据被写入内存
          start += size, offset += size;  // 更新起始地址和偏移量
      } 
      end = ph->p_va + ph->p_memsz;  // 处理剩余的内存区域
      if(start < la) 
      {
          if(start >= end)
              continue ;
          off = start + PGSIZE - la, size = PGSIZE - off;
          if(end < la)
              size -= la - end;
          memset(page2kva(page) + off, 0, size);
          fence_i(page2kva(page) + off, size);
          start += size;
          assert((end < la && start == end) || (end >= la && start == la));  // 确保地址计算正确
      }
      while (start < end) {  // 继续处理剩余的内存区域
          if((page = pgdir_alloc_page(mm->pgdir, la, perm)) == NULL)
          {
              ret = -E_NO_MEM;
              goto bad_cleanup_mmap;
          }
          off = start - la, size = PGSIZE - off, la += PGSIZE;
          if(end < la)
              size -= la - end;
          memset(page2kva(page) + off, 0, size);
          fence_i(page2kva(page) + off, size);
          start += size;
      }
  }
  sysfile_close(fd);  // 关闭文件描述符
```

​	该部分首先将程序从文件加载到内存，从磁盘上读取ELF文件的首部(Elf-header)中的信息，据此循环加载各程序段的程序头信息(Program header)。

​	随后，在进行一系列的特判后，建立虚拟地址与物理地址之间的映射，并根据ELF文件中的信息，设置各段的权限，并将对应虚拟内存地址设置为合法。

​	然后复制数据段和代码段。在物理内存空间分配完成后，读取指定大小的磁盘块，并将读入的数据存储到分配的空间中。当BSS段(存放未初始化或初始化为0的全局变量和静态局部变量)存在，并且先前的TEXT/DATA段(存放代码/初始化不为0的全局变量和静态局部变量)分配的最后一页未被完全占用，则BSS段在进行清零初始化后占用剩余部分，当空间不足时则进一步分配。

​	操作完成后，关闭该文件。

#### 2.2.4 建立相应的虚拟内存映射表

```c
vm_flags = VM_READ | VM_WRITE | VM_STACK;//设置用户栈的权限,将用户栈所在的虚拟内存区域设置为合法
  // 尝试将用户栈映射到内存中，栈的起始地址为USTACKTOP - USTACKSIZE，大小为USTACKSIZE
// 如果映射失败，则跳转到bad_cleanup_mmap标签处理错误
  if((ret = mm_map(mm, USTACKTOP - USTACKSIZE, USTACKSIZE, vm_flags, NULL)) != 0)
      goto bad_cleanup_mmap;
```

#### 2.2.5 建立并初始化用户堆栈

```c
  mm_count_inc(mm);//切换到用户进程空间
  current->mm = mm;  // 将当前进程的内存管理器设置为新创建的用户进程的内存管理器
  current->cr3 = PADDR(mm->pgdir);  // 将当前进程的页目录地址设置为新创建的用户进程的页目录地址
// PADDR是将虚拟地址转换为物理地址的宏
  lcr3(PADDR(mm->pgdir));  // 刷新TLB（Translation Lookaside Buffer），使cr3寄存器的更改立即生效
// lcr3是加载cr3寄存器的汇编指令。
```

#### 2.2.6 处理用户栈中传入的参数

```c
  uintptr_t stacktop = USTACKTOP - argc * PGSIZE;  // 计算用户栈的顶部地址，减去命令行参数所占的内存页大小的总和
  char **uargv = (char **)(stacktop - argc * sizeof(char *));  // 将用户栈顶部地址转换为指向字符指针的指针，用于存储命令行参数的地址
  for (int i = 0; i < argc; i ++)  // 遍历命令行参数，将它们复制到用户栈中，并记录参数地址
      uargv[i] = strcpy((char *)(stacktop + i * PGSIZE), kargv[i]);  // 在用户栈中为第i个参数分配一页内存
    // (char *)转换确保地址被解释为字符数组的指针
```

#### 2.2.7 设置用户进程的中断帧

```c
	struct trapframe *tf = current->tf;  // 获取当前进程的陷阱框架指针
    memset(tf, 0, sizeof(struct trapframe));  // 将陷阱框架清零，以确保没有旧的或随机的数据影响进程的执行
    tf->tf_era = elf->e_entry;  // 设置陷阱框架的tf_era字段，用户程序的入口点地址
    tf->tf_regs.reg_r[LOONGARCH_REG_SP] = USTACKTOP;  // 设置栈指针寄存器的值，指向用户栈的顶部
    // 初始化状态寄存器，设置为用户模式并允许中断
    uint32_t status = 0;
    status |= PLV_USER;
    status |= CSR_CRMD_IE;
    tf->tf_prmd = status;  // 将状态寄存器的值设置到陷阱框架的tf_prmd字段
	// 设置寄存器的值，传递命令行参数的数量和地址给用户程序
    tf->tf_regs.reg_r[LOONGARCH_REG_A0] = argc;  // 第一个参数，通常是命令行参数的数量
    tf->tf_regs.reg_r[LOONGARCH_REG_A1] = (uint32_t)uargv;  // 第二个参数，通常是指向命令行参数数组的指针
```

#### 2.2.8 错误处理

```c
ret = 0;//函数正常退出
    out:
        return ret;  // 从函数返回ret值，这里是0，表示成功
    bad_cleanup_mmap:  // 标签bad_cleanup_mmap，处理mm_map函数失败的情况
        panic("bad_cleanup_mmap");  // 调用panic函数，打印错误信息并停止执行
        exit_mmap(mm);  // 清理映射的内存，并退出
    bad_elf_cleanup_pgdir:  // 标签bad_elf_cleanup_pgdir，处理ELF头解析失败的情况
        panic("bad_elf_cleanup_pgdir");  // 调用panic函数，打印错误信息并停止执行
        put_pgdir(mm);  // 释放页目录资源
    bad_pgdir_cleanup_mm:  // 标签bad_pgdir_cleanup_mm，处理页目录设置失败的情况
        panic("bad_pgdir_cleanup_mm");  // 调用panic函数，打印错误信息并停止执行 
        mm_destroy(mm);  // 销毁内存管理器结构体
    bad_mm:  // 标签bad_mm，处理内存管理器创建失败的情况
        panic("bad_mm");  // 调用panic函数，打印错误信息并停止执行
        goto out;  // 跳转到标签out，执行退出操作
```

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab521.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab522.png" style="zoom:67%;" />

​	评测结果如图，关卡通过。

## 3 关卡代码

```c
static int load_icode(int fd, int argc, char **kargv) { // load_icode from disk fd, For LAB4
    #ifdef LAB4_EX2
    if(current->mm != NULL)//检查进程的内存管理器是否清空
        panic("load_icode: current->mm must be empty.\n");
    int ret = -E_NO_MEM;
    struct mm_struct *mm;
    if((mm = mm_create()) == NULL)//建立内存管理器
        goto bad_mm;
    if(setup_pgdir(mm) != 0)//设置内存管理器的页目录表
     goto bad_pgdir_cleanup_mm;
    struct __elfhdr ___elfhdr__;
    struct elfhdr32 __elf, *elf = &__elf;
    if((ret = load_icode_read(fd, &___elfhdr__, sizeof(struct __elfhdr), 0)) != 0)
        goto bad_elf_cleanup_pgdir;
  
    _load_elfhdr((unsigned char*)&___elfhdr__, &__elf);
    if(elf->e_magic != ELF_MAGIC) {
        ret = -E_INVAL_ELF;
        goto bad_elf_cleanup_pgdir;
    }
    struct proghdr _ph, *ph = &_ph;
    uint32_t vm_flags, phnum;
    uint32_t perm = 0;
    struct Page *page;
    for (phnum = 0; phnum < elf->e_phnum; phnum ++) {
        off_t phoff = elf->e_phoff + sizeof(struct proghdr) * phnum;
        if((ret = load_icode_read(fd, ph, sizeof(struct proghdr), phoff)) != 0)
            goto bad_cleanup_mmap;
        if(ph->p_type != ELF_PT_LOAD)
            continue ;
        if(ph->p_filesz > ph->p_memsz){
            ret = -E_INVAL_ELF;
            goto bad_cleanup_mmap;
        }
      
        vm_flags = 0;
        perm |= PTE_U;
        if(ph->p_flags & ELF_PF_X)
            vm_flags |= VM_EXEC;
        if(ph->p_flags & ELF_PF_W)
            vm_flags |= VM_WRITE;
        if(ph->p_flags & ELF_PF_R)
            vm_flags |= VM_READ;
        if(vm_flags & VM_WRITE)
            perm |= PTE_W; 
        if((ret = mm_map(mm, ph->p_va, ph->p_memsz, vm_flags, NULL)) != 0)
            goto bad_cleanup_mmap;
        off_t offset = ph->p_offset;
        size_t off, size;
        uintptr_t start = ph->p_va, end, la = ROUNDDOWN_2N(start, PGSHIFT);
        end = ph->p_va + ph->p_filesz;
        while (start < end) {
            if((page = pgdir_alloc_page(mm->pgdir, la, perm)) == NULL) {
                ret = -E_NO_MEM;
                goto bad_cleanup_mmap;
            }
            off = start - la, size = PGSIZE - off, la += PGSIZE;
            if(end < la)
                size -= la - end;
            if((ret = load_icode_read(fd, page2kva(page) + off, size, offset)) != 0)
                goto bad_cleanup_mmap;
            fence_i(page2kva(page)+off, size);
            start += size, offset += size;
        }
        end = ph->p_va + ph->p_memsz;
        if(start < la) {
            if(start >= end)
                continue ;
            off = start + PGSIZE - la, size = PGSIZE - off;
            if(end < la)
                size -= la - end;
            memset(page2kva(page) + off, 0, size);
            fence_i(page2kva(page) + off, size);
            start += size;
            assert((end < la && start == end) || (end >= la && start == la));
        }
        while (start < end) {
            if((page = pgdir_alloc_page(mm->pgdir, la, perm)) == NULL){
                ret = -E_NO_MEM;
                goto bad_cleanup_mmap;
            }
            off = start - la, size = PGSIZE - off, la += PGSIZE;
            if(end < la)
                size -= la - end;
            memset(page2kva(page) + off, 0, size);
            fence_i(page2kva(page) + off, size);
            start += size;
        }
    }
    sysfile_close(fd);
    vm_flags = VM_READ | VM_WRITE | VM_STACK;//设置用户栈的权限,将用户栈所在的虚拟内存区域设置为合法
    if((ret = mm_map(mm, USTACKTOP - USTACKSIZE, USTACKSIZE, vm_flags, NULL)) != 0)
        goto bad_cleanup_mmap;
    mm_count_inc(mm);//切换到用户进程空间
    current->mm = mm;
    current->cr3 = PADDR(mm->pgdir);
    lcr3(PADDR(mm->pgdir));
    uintptr_t stacktop = USTACKTOP - argc * PGSIZE;
    char **uargv = (char **)(stacktop - argc * sizeof(char *));
    for (int i = 0; i < argc; i ++)
        uargv[i] = strcpy((char *)(stacktop + i * PGSIZE), kargv[i]);
    struct trapframe *tf = current->tf;
    memset(tf, 0, sizeof(struct trapframe));
    tf->tf_era = elf->e_entry;
    tf->tf_regs.reg_r[LOONGARCH_REG_SP] = USTACKTOP;
    uint32_t status = 0;
    status |= PLV_USER;
    status |= CSR_CRMD_IE;
    tf->tf_prmd = status;
    tf->tf_regs.reg_r[LOONGARCH_REG_A0] = argc;
    tf->tf_regs.reg_r[LOONGARCH_REG_A1] = (uint32_t)uargv;
    ret = 0;//函数正常退出
    out:
        return ret;
    bad_cleanup_mmap:
        panic("bad_cleanup_mmap");
        exit_mmap(mm);
    bad_elf_cleanup_pgdir:
        panic("bad_elf_cleanup_pgdir");
        put_pgdir(mm);
    bad_pgdir_cleanup_mm:
        panic("bad_pgdir_cleanup_mm");
        mm_destroy(mm);
    bad_mm:
        panic("bad_mm");
        goto out;
#endif
}
```









