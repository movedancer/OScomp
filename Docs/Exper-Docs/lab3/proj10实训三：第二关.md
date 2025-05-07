# proj10实训三：进程管理

# 第二关父进程创建子进程

------

## 1 知识阅读

​	本关任务为编写程序，实现父进程创建一个子进程，拷贝当前进程（即父进程）的用户内存地址空间中的合法内容到新进程中（子进程），并完成内存资源的复制。其中创建子进程的函数do_fork在执行中将拷贝当前进程（即父进程）的用户内存地址空间中的合法内容到新进程中（子进程），完成内存资源的复制。具体是通过copy_range函数（位于kern/mm/pmm.c中）实现的，这也是我们需要完成的。

​	**基本流程：**

​	父进程调用fork()系统调用，进入正常的中断处理机制，最终调用syscall()函数；在syscall()函数中，根据系统调用名，调用sys_fork()函数；该函数进一步调用了do_fork()函数，这个函数的任务就是创建子进程、并且将父进程的内存空间复制给子进程。

​	在do_fork()函数中，调用copy_mm()进行内存空间的复制；在copy_mm()函数中，又调用了dup_mmap()函数，该函数是将父进程的虚拟内存空间的内容复制到子进程的内存空间，其又调用了copy_range()函数来完成父进程内存空间复制到子进程的功能，也就是我们需要补充的部分。

​	copy_range()函数遍历父进程指定的某一段内存空间中的每一个虚拟页，如果这个虚拟页是存在的话，为子进程对应的同一个地址也申请分配一个物理页，然后将前者中的所有内容复制到后者中去，然后为子进程的这个物理页和对应的虚拟地址建立映射关系。由此编程需要完成的内容就是：

1. 找到父进程需要复制的物理页所在内核虚拟地址；

2. 找到子进程需要被填填写的物理页的内核虚拟地址；
3. 将父进程的物理页的内容复制到子进程中去；
4. 建立子进程的物理页与虚拟页的映射。	

## 2 实验步骤

### 2.1 代码补全

```c
 int copy_range(pde_t *to, pde_t *from, uintptr_t start, uintptr_t end, bool share) {
    assert(start % PGSIZE == 0 && end % PGSIZE == 0);
    assert(USER_ACCESS(start, end));
    do {
       pte_t *ptep = get_pte(from, start, 0), *nptep;
       if (ptep == NULL) {
          start = ROUNDDOWN_2N(start + PTSIZE, PGSHIFT);
       continue ;
       }
       if (*ptep & PTE_P) {
          if ((nptep = get_pte(to, start, 1)) == NULL) {
             return -E_NO_MEM;
          }
          uint32_t perm = (*ptep & PTE_USER);
          struct Page *page = pte2page(*ptep);
          struct Page *npage=alloc_page();
          assert(page!=NULL);
          assert(npage!=NULL);
          int ret=0;
    #ifdef LAB3_EX2
    /* LAB3 EXERCISE2: YOUR CODE
    * replicate content of page to npage, build the map of phy addr of nage with the linear addr start
    *
    * Some Useful MACROs and DEFINEs, you can use them in below implementation.
    * MACROs or Functions:
    *    page2kva(struct Page *page): return the kernel vritual addr of memory which page managed (SEE pmm.h)
    *    page_insert: build the map of phy addr of an Page with the linear addr la
    *    memcpy: typical memory copy function
    *
    * (1) find src_kvaddr: the kernel virtual address of page
    * (2) find dst_kvaddr: the kernel virtual address of npage
    * (3) memory copy from src_kvaddr to dst_kvaddr, size is PGSIZE
    * (4) build the map of phy addr of  nage with the linear addr start
    */
    #endif
          assert(ret == 0);
       }
       start += PGSIZE;
    } while (start != 0 && start < end);
    return 0;
    }
```

​	copy_range函数实现了Copy-on-Write(COW)机制，主要任务是在父进程和子进程之间共享物理页框。首先，通过get_pte()函数获取父进程对应线性地址的页表项指针，然后判断是否存在，若不存在则继续查找下一页。如果页表项存在且有效，通过get_pte()函数获取子进程对应线性地址的页表项指针，再通过alloc_page()函数分配一个新的物理页，然后通过memcpy()函数将父进程物理页的内容复制到子进程的物理页中。最后，通过page_insert()函数建立子进程的物理页与虚拟页的映射关系，使得父子进程在写入时能够进行独立的复制。

​	因此我们需要完成的功能就是将父进程的内容复制到子进程并建立映射：

```c
    memcpy(page2kva(npage), page2kva(page), PGSIZE);  //将父进程的物理页的内容复制到子进程中去
    page_insert(to, npage, start,perm);  //建立子进程的物理页与虚拟页的映射关系 
```

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab321.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab322.png" style="zoom:67%;" />

​	评测如图，成功过关。

## 3 实验代码

```c
int copy_range(pde_t *to, pde_t *from, uintptr_t start, uintptr_t end, bool share) {
    assert(start % PGSIZE == 0 && end % PGSIZE == 0);
    assert(USER_ACCESS(start, end));
    do {
       pte_t *ptep = get_pte(from, start, 0), *nptep;
       if (ptep == NULL) {
          start = ROUNDDOWN_2N(start + PTSIZE, PGSHIFT);
       continue ;
       }
       if (*ptep & PTE_P) {
          if ((nptep = get_pte(to, start, 1)) == NULL) {
             return -E_NO_MEM;
          }
          uint32_t perm = (*ptep & PTE_USER);
          struct Page *page = pte2page(*ptep);
          struct Page *npage=alloc_page();
          assert(page!=NULL);
          assert(npage!=NULL);
          int ret=0;
    #ifdef LAB3_EX2
    /* LAB3 EXERCISE2: YOUR CODE
    * replicate content of page to npage, build the map of phy addr of nage with the linear addr start
    *
    * Some Useful MACROs and DEFINEs, you can use them in below implementation.
    * MACROs or Functions:
    *    page2kva(struct Page *page): return the kernel vritual addr of memory which page managed (SEE pmm.h)
    *    page_insert: build the map of phy addr of an Page with the linear addr la
    *    memcpy: typical memory copy function
    *
    * (1) find src_kvaddr: the kernel virtual address of page
    * (2) find dst_kvaddr: the kernel virtual address of npage
    * (3) memory copy from src_kvaddr to dst_kvaddr, size is PGSIZE
    * (4) build the map of phy addr of  nage with the linear addr start
    */
    memcpy(page2kva(npage), page2kva(page), PGSIZE);  //将父进程的物理页的内容复制到子进程中去
    page_insert(to, npage, start,perm);  //建立子进程的物理页与虚拟页的映射关系 
    #endif
          assert(ret == 0);
       }
       start += PGSIZE;
    } while (start != 0 && start < end);
    return 0;
    }
```

