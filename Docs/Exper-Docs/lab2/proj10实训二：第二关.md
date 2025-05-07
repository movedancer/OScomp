# proj10实训二：内存管理

# 第二关实现寻找虚拟地址对应的页表项

------

## 1 知识阅读

​	本关卡需要实现寻找虚拟地址对应的页表项功能。根据我们在知识整理中所做的积累，虚拟内存采用多级页表进行地址管理，通过设置页表和对应的页表项，可建立虚拟内存地址和物理内存地址的对应关系。其中的get_pte函数是设置页表项环节中的一个重要步骤。此函数找到一个虚地址对应的二级页表项的内核虚地址，如果此二级页表项不存在，则分配一个包含此项的二级页表。

## 2 实验步骤

### 2.1 代码补全

​	实验要求补全的代码如下：

```c
pte_t *
get_pte(pde_t *pgdir, uintptr_t la, bool create) {
#ifdef LAB2_EX2
    /* LAB2 EXERCISE2: YOUR CODE
     *
     * If you need to visit a physical address, please use KADDR()
     * please read pmm.h for useful macros
     *
     * Maybe you want help comment, BELOW comments can help you finish the code
     *
     * Some Useful MACROs and DEFINEs, you can use them in below implementation.
     * MACROs or Functions:
     *   PDX(la) = the index of page directory entry of VIRTUAL ADDRESS la.
     *   KADDR(pa) : takes a physical address and returns the corresponding kernel virtual address.
     *   set_page_ref(page,1) : means the page be referenced by one time
     *   page2pa(page): get the physical address of memory which this (struct Page *) page  manages
     *   struct Page * alloc_page() : allocation a page
     *   memset(void *s, char c, size_t n) : sets the first n bytes of the memory area pointed by s
     *                                       to the specified value c.
     * DEFINEs:
     *   PTE_P           0x001                   // page table/directory entry flags bit : Present
     *   PTE_W           0x002                   // page table/directory entry flags bit : Writeable
     *   PTE_U           0x004                   // page table/directory entry flags bit : User can access
     */
    pde_t *pdep = NULL;   // (1) find page directory entry
    if (0) {              // (2) check if entry is not present
                          // (3) check if creating is needed, then alloc page for page table
                          // CAUTION: this page is used for page table, not for common data page
                          // (4) set page reference
        uintptr_t pa = 0; // (5) get linear address of page
                          // (6) clear page content using memset
                          // (7) set page directory entry's permission
    }
    return NULL;          // (8) return page table entry
#endif
}
```

​	阅读代码，其实这部分注释基本把每一步都说明了，下面给出代码与注释：

```c
pde_t *pdep = NULL; // 寻找一级页表中的索引，入口
pdep = pgdir + PDX(la); // PDX(la)  前10位(PDE)
// 检查页表项是否存在
if (((*pdep) & PTE_P) == 0)
{
    // 不需要分配或者分配的页为NULL
    if (!create)
        return NULL;
    // 设置引用次数一次
    struct Page *new_pte = alloc_page();
    if (!new_pte)
        return NULL;
page_ref_inc(new_pte);
// 得到物理地址
    uintptr_t pa = (uintptr_t)page2kva(new_pte);
     // 清理虚拟地址
    memset((void *)pa, 0, PGSIZE);
    kprintf("@@@ %x\n", pa);
    //  set page directory entry's permission
    *pdep = PADDR(pa);
    (*pdep) |= (PTE_U | PTE_P | PTE_W);
}
// 将物理地址再转化为内核虚拟地址,就能得到二级页表的起始地址,然后加上PTE对应的偏移量,得到最终的二级页表虚地址
pte_t *ret = (pte_t *)KADDR((uintptr_t)((pte_t *)(PDE_ADDR(*pdep)) + PTX(la)));
kprintf("@@GET_PTE %x %x %x\n", *pdep, ret, *ret);
return ret; // return page table entry
```

​	下面讲述下代码思路：

​	首先进行页表项查找，通过虚拟地址`la`计算出一级页表目录（PGDIR）中对应的页表项（PDE）的位置。然后进行页表项存在性检查，检查计算出的页表项是否已经存在，即检查其`PTE_P`位是否被设置。再进行页表项分配与页表项初始化，如果页表项不存在，并且`create`标志为`true`，则分配一个新的页面用于创建二级页表；如果分配失败，返回`NULL`。创建成功后，增加新分配页面的引用计数，将新页面的物理地址清零，确保没有旧数据，设置页表项的权限，使其可读写，并标记为存在。最后计算二级页表地址，将新页表项的物理地址转换为内核虚拟地址，得到二级页表的起始地址，再根据虚拟地址`la`计算在二级页表中的偏移量，得到最终的二级页表项地址。

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab221.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\拉邦22.png" style="zoom:67%;" />

​	评测如图，成功过关。有一次评测没有过为忘记将结果写入评测点。

## 3 实验代码

```c
pte_t *
get_pte(pde_t *pgdir, uintptr_t la, bool create) {
#ifdef LAB2_EX2
    pde_t *pdep = NULL;
    pdep = pgdir + PDX(la); 
    if (((*pdep) & PTE_P) == 0)
    {

        if (!create)
            return NULL;

        struct Page *new_pte = alloc_page();
        if (!new_pte)
            return NULL;
    page_ref_inc(new_pte);
        uintptr_t pa = (uintptr_t)page2kva(new_pte);

        memset((void *)pa, 0, PGSIZE);
        kprintf("@@@ %x\n", pa);

        *pdep = PADDR(pa);
        (*pdep) |= (PTE_U | PTE_P | PTE_W);
    }
    pte_t *ret = (pte_t *)KADDR((uintptr_t)((pte_t *)(PDE_ADDR(*pdep)) + PTX(la)));
    kprintf("@@GET_PTE %x %x %x\n", *pdep, ret, *ret);
    return ret;

#endif
}
```

