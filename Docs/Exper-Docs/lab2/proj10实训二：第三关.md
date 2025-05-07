# proj10实训二：内存管理

# 第三关释放某虚地址所在的页并取消对应二级页表项的映射

------

## 1 知识阅读

​	本关要求编写代码，释放某虚地址所在的页并取消对应二级页表项的映射。当释放一个包含某虚地址的物理内存页时，需要让对应此物理内存页的管理数据结构Page做相关的清除处理，使得此物理内存页成为空闲；另外还需把表示虚地址与物理地址对应关系的二级页表项清除。即我们需要实现函数以下功能：

1. **查找二级页表项**：首先，我们需要找到虚拟地址对应的二级页表项（PTE）。
2. **清除二级页表项**：将该二级页表项清零或设置为无效，以取消虚拟地址到物理地址的映射。
3. **释放物理页**：对相应的物理页进行释放处理，包括减少引用计数，并在引用计数为零时将其标记为空闲。
4. **更新内存管理数据结构**：确保内存管理数据结构（如Page结构）反映了物理页的新状态。

​	下面阅读关卡需要补全的page_remove_pte函数。

## 2 实验步骤

### 2.1 代码补全

​	根据我们在知识阅读中整理的步骤，page_remove_pte()函数主要作用是传入虚地址和二级页表项地址，释放某虚地址所在的物理页并取消对应二级页表项的映射：

```c
    if (0) {                      //(1) check if this page table entry is present
        struct Page *page = NULL; //(2) find corresponding page to pte
                                  //(3) decrease page reference
                                  //(4) and free this page when page reference reachs 0
                                  //(5) clear second page table entry
                                  //(6) flush tlb
    }
```

​	根据注释内容，设计过程如下：首先通过ptep指针和PTE_P标志是否都不为零，判断二级页表入口是否存在，若不存在该函数就不需要继续执行了，存在则需要清除对应的页面内容。

​	接下来，先使用pte2page函数找到ptep指针对应的页面，再通过page_ref_dec将页面的引用数减1。之后判断若页面引用数是否为0，如果为0则代表该页面已经空闲，使用free_page函数释放页面。最后，当页面已经清除，将ptep指针置0，取消对应二级页表项的映射，再使用tlb_invalidate函数冲刷快表即可。

```c
// 关卡提供的代码
// 检查ptep是否存在
if (ptep && (*ptep & PTE_P))
{   
 // 找到二级页表entry对应的页面
    struct Page *page = pte2page(*ptep); 
    // page 引用减一
    page_ref_dec(page);
    // 如果page引用数为0,则清除页面
    if (page_ref(page) == 0)
    {
        free_page(page);
    }
    // 将二级页表entry清零
    *ptep = 0;
}
// 清洗快表
tlb_invalidate_all();
```

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab232.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab231.png" style="zoom:67%;" />

​	评测结果如上，成功过关。

## 3 实验代码

```c
static inline void
page_remove_pte(pde_t *pgdir, uintptr_t la, pte_t *ptep) {
#ifdef LAB2_EX3
    if (ptep && (*ptep & PTE_P))
    {
        struct Page *page = pte2page(*ptep);
        page_ref_dec(page);
        if (page_ref(page) == 0)
        {
            free_page(page);
        }
        *ptep = 0;
    }
    tlb_invalidate_all();
#endif
}
```

