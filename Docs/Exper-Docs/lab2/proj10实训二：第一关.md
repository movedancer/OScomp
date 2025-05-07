# proj10实训二：内存管理

# 第一关实现 first-fit 连续物理内存分配算法

------

## 1 知识阅读

​	本部分关卡需要实现first-in连续物理内存分配算法。该算法主要有以下要点：

### 1.1 页空闲块维护

​	first_fit分配算法需要维护一个查找有序（地址按从小到大排列）空闲块（以页为最小单位的连续地址空间）的数据结构，双向链表是一个很好的选择。本实验中使用了libs/list.h定义的可挂接任意元素的通用双向链表，可以完成对双向链表的初始化/插入/删除等。具体来说，是定义了一个名为free_area_t的数据结构：

```c
list_entry_t free_list; // the list header
unsigned int nr_free; // # of free pages in this free list
} free_area_t;
```

   其中free_area_t包含了一个list_entry结构的双向链表指针和记录当前空闲页的个数的无符号整型变量nr_free。其中的链表指针指向了空闲的物理页。

### 1.2 空闲页链表初始化

​	default_init_memmap函数将根据每个物理页帧的情况来建立空闲页链表，也就是初始化了一个空闲块，空闲块应该是根据地址高低形成一个有序链表。地址从高到低的设计是为了在default_ free_pages中实现空闲块的合并。初始化过程中default_init_memmap(base, n)会在物理内存中从base开始连续申请n个Page，并对这些Page做初始化，比如将ref设置为0（表示这一表项没有被其他虚拟页表引用），将除第一页的其他Page的property, flag设置为0，表示property属性无效并且非预留，将第一页property设置为n，表示这一个空闲块里面有n个Page。

### 1.3 块分配

​	default_alloc_pages函数从空闲页链表中寻找第一个大小足够的空闲块并分配（first fit）。当想要申请n个Pages，先会判断nr_free是否大于等于n，如果小于则返回NULL。firstfit需要从空闲链表头开始查找最小的地址，通过list_next找到下一个空闲块元素，通过le2page宏可以由链表元素获得对应的Page指针p。通过p->property可以了解此空闲块的大小。如果>=n，就找到并返回，如果<n，则list_next，继续查找。直到list_next== &free_list，这表示找完了一遍了。找到后，就要从新组织空闲块，然后把找到的page返回。

### 1.4 块释放

​	default_free_pages函数的实现其实是default_alloc_pages的逆过程。先遍历这个块的所有页，确认所有页都合法（reserved==0&&property==0），然后将需要释放的空间标记为空之后，找到空闲表中合适的位置。由于空闲表中的记录都是按照物理页地址排序的，所以如果插入位置的前驱或者后继刚好和释放后的空间邻接，那么需要将新的空间与前后邻接的空间合并形成更大的空间。

### 1.5 改进

​	在不改变first fit算法的情况下，default_free_pages函数中存在可优化空间。在default_free_pages函数中，是通过遍历整个链表寻找可以合并的前驱后继节点的，时间复杂度是O(N)。如果把链表数据结构换成树结构等可以通过内存地址进行快速检索的结构，就可以降低default_free_pages函数的时间复杂度。

## 2 实验步骤

### 2.1 代码补全

​	实验给出了待修改代码如下：

```c
static struct Page *
default_alloc_pages(size_t n) {
#ifdef LAB2_EX1
    assert(n > 0);
    if (n > nr_free) {
        return NULL;
    }
    struct Page *page = NULL;
    list_entry_t *le = &free_list;
    // TODO: optimize (next-fit)
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        if (p->property >= n) {
            page = p;
            break;
        }
    }
    if (page != NULL) {
        if (page->property > n) {
            struct Page *p = page + n;
            p->property = page->property - n;
            SetPageProperty(p);
            list_add_after(&(page->page_link), &(p->page_link));
        }
        list_del(&(page->page_link));
        nr_free -= n;
        ClearPageProperty(page);
    }
    return page;
#endif
}
```

​	代码实现的是next-fit，其步骤如下：

1. **参数检查**：函数首先检查请求分配的页面数`n`是否大于0，并且是否超过了系统中可用的空闲页面数`nr_free`。如果请求的页面数超过了可用页面数，函数返回`NULL`。
2. **查找空闲页面**：使用一个循环遍历空闲页面链表`free_list`，寻找第一个足够大的空闲页面（即`property`属性大于或等于请求的页面数`n`）。
3. **页面分割**：如果找到的页面比请求的页面数大，函数会将页面分割成两部分：一部分正好等于请求的页面数，另一部分包含剩余的页面。分割后的页面被插入到空闲页面链表中。
4. **更新链表和计数**：从空闲页面链表中移除被分配的页面，并更新系统中空闲页面的数量`nr_free`。
5. **清除和返回**：清除被分配页面的属性，并返回页面的指针。

​	next-fit的问题就是它不一定会找到最合适的空闲页面，可能会导致内存碎片问题。而first-fit可以优化这个问题，使用题目所给的one-fit代码进行替换：

```c
// 优化代码
int THRESHOLD = 1; // 设置阈值，如果剩余空间小于这一阈值，则全部分配出去
if(page->property >= n){
    // 只有在剩余空间大于某一阈值的情况下
    // 才划分为两页，否则全部分配出去
    if(page->property - n < THRESHOLD){
        nr_free -= page->property;
    }
    else{
        struct Page *new_page = page + n;
        nr_free -= n;
        new_page->property = page->property - n;
        SetPageProperty(new_page);
        list_add_after(&(page->page_link), &(new_page->page_link));
    }
    list_del(&(page->page_link));
    ClearPageProperty(page);
}
```

​	first-hit算法避免了划分出多个小页面，代码思路如下：

1. **设置阈值**：定义了一个阈值`THRESHOLD`，用于决定是否将找到的页面分割成两个更小的页面。
2. **查找合适的页面**：在空闲页面链表中查找第一个足够大的页面，其`property`属性（表示页面大小或可用空间）至少等于请求的大小`n`。
3. **判断是否分割**：如果找到的页面在分配请求的页面后，剩余的空间小于阈值`THRESHOLD`，则不进行分割，而是将整个页面分配出去。
4. **分割页面**：如果剩余空间大于或等于阈值，则将页面分割成两个页面。新页面的`property`属性设置为剩余的空间，并且将其添加回空闲页面链表中。
5. **更新链表和计数**：无论是否分割页面，都将原始页面从空闲页面链表中删除，并更新系统中的空闲页面计数`nr_free`。
6. **清除属性并返回**：清除被分配页面的属性，并返回页面的指针。

​	在原代码的if判断处，当发现符合需求页面不为空时，直接将原来的next-fit实现代码替换为first-fit代码即可。

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab211.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab212.png" style="zoom:67%;" />

​	实验结果如上图，成功替换了first-fit代码。

## 3 实验代码

```c
tatic struct Page *
default_alloc_pages(size_t n) {
#ifdef LAB2_EX1
    assert(n > 0);
    if (n > nr_free) {
        return NULL;
    }
    struct Page *page = NULL;
    list_entry_t *le = &free_list;
    int THRESHOLD = 1;
    // TODO: optimize (next-fit)
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        if (p->property >= n) {
            page = p;
            break;
        }
    }

    if (page != NULL) {
        if (page->property - n < THRESHOLD) {
            nr_free -= page->property;
            list_del(&(page->page_link));
            ClearPageProperty(page);
        } else {
            struct Page *new_page = page + n;
            nr_free -= n;
            new_page->property = page->property - n;
            SetPageProperty(new_page);
            list_add_after(&(page->page_link), &(new_page->page_link));
            list_del(&(page->page_link));
            ClearPageProperty(page);
        }
        return page;
    }
#endif
    return NULL;
}
```

​	将上述代码替换目标代码文件中的default_alloc_pages()函数即可。