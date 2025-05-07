# proj10实训七：**LOOK调度算法的实现**

# 第一关实现LOOK算法调度器

------

## 1 知识阅读

​	本关卡要求在uCore中添加一个简单的磁盘I/O调度模拟模块，通过LOOK算法遍历链表并返回遍历顺序数组，数组成员为BIO的逻辑块地址。需要掌握：1.链表的基本操作：如何根据请求的块号插入BIO节点，2.LOOK调度算法：如何根据LOOK算法遍历BIO节点并记录遍历顺序。

### 1.1 代码阅读

​	关卡给出了相关函数如下：

#### init_queue_look函数

​	init_queue_look函数用于初始化I/O调度器的请求队列。其主要任务是初始化链表头、链表尾和当前磁头位置的结构体。首先，设置初始遍历方向和磁头位置，并分配内存给disk_head_look、list_head_look和list_tail_look三个BIO结构体。如果内存分配失败，则打印错误信息并返回-1。成功分配内存后，设置disk_head_look的块号为初始磁头位置，将链表头和链表尾连接起来，形成一个初始的双向链表，磁头位置在链表中间。最后，将请求计数器request_count设置为0。

```c
static int init_queue_look() {
    direction_look = INITIAL_DIRECTION;
    disk_head_look = (struct bio*)kmalloc(sizeof(struct bio));
    if(disk_head_look == NULL){
        kprintf("Memory allocation failed for disk_head_look\n");
        return -1;
    }
    disk_head_look->block_num = INITIAL_HEAD_POS;
    request_count = 0;
    list_head_look = (struct bio*)kmalloc(sizeof(struct bio));
    if(list_head_look == NULL){
        kprintf("Memory allocation failed for list_head_look\n");
        kfree(disk_head_look);
        return -1;
    }
    list_head_look->block_num = -1;
    list_tail_look = (struct bio*)kmalloc(sizeof(struct bio));
    if(list_tail_look == NULL){
        kprintf("Memory allocation failed for list_tail_look\n");
        kfree(disk_head_look);
        kfree(list_head_look);
        return -1;
    }
    list_tail_look->block_num = -1;
    disk_head_look->next = list_tail_look;
    disk_head_look->prev = list_head_look;
    list_head_look->next = disk_head_look;
    list_head_look->prev = NULL;
    list_tail_look->next = NULL;
    list_tail_look->prev = disk_head_look;
    return 0;
}
```

#### submit_bio函数

​	submit_bio()函数用于将新的BIO请求插入到调度队列中。首先，函数检查请求是否为空，如果为空则直接返回。然后，从链表头开始遍历，找到第一个块号大于或等于新请求块号的位置，将新请求插入该位置。通过更新相邻节点的指针，将新请求节点插入到链表中。最后，增加请求计数器request_count，以记录当前队列中的请求数量。这样可以确保所有请求按照块号有序排列，方便LOOK调度算法的遍历。

```c
static void submit_bio(struct bio* request) {
    if(request == NULL){
        return;
    }
    struct bio *p = list_head_look->next;
    while(p->block_num != -1 && request->block_num >= p->block_num) {
        p = p->next;
    }
    request->next = p;
    request->prev = p->prev;
    if(p->prev != NULL){
        p->prev->next = request;
    }
    p->prev = request;
    request_count++;
}
```

​	总结来说，上面两个函数解释了请求是如何通过双向链表进行存储的。请求链表的一头一尾分别为list_head_look和list_tail_look，并且需要知道disk_head_look为初始磁头方向，位置已经初始化好。根据direction方向直接对磁头进行赋值即可。

### 1.2 链表的基本操作

​	在插入节点时，我们从当前磁头位置开始遍历链表，找到适当的位置插入新的请求节点。具体步骤如下：

1. **初始化指针：**从当前磁头位置开始遍历链表。
2. **寻找插入位置：**遍历链表，直到找到一个节点，其块号大于或等于要插入的请求块号。
3. **更新指针：**将新的请求节点插入到找到的位置，更新相邻节点的指针。
4. 这种方式确保链表中的请求按块号有序排列，有利于之后的LOOK调度算法的遍历。

### 1.3 LOOK调度算法

​	在遍历链表时，我们使用LOOK调度算法按以下步骤进行：

1. **初始化遍历方向：**根据direction_clook决定初始遍历方向（从头到尾或从尾到头）。
2. **遍历链表：**按当前方向遍历请求节点，将每个节点的块号记录到traversed_blocks数组中，并更新当前磁头位置。
3. **切换方向：**当到达链表末尾时，切换遍历方向，继续遍历剩余的请求节点。
4. **终止条件：**当在两个方向上都没有未处理的请求时，结束遍历。
5. 这种方式确保所有请求按照LOOK调度算法的顺序得到处理，并记录遍历顺序以供后续使用。

## 2  实验步骤

### 2.1 代码补全

​	关卡给出待补全代码如下：

```c
函数 look_schedule(输入: 磁头所在块号指针, 磁头方向指针){
    初始化遍历数组
    设置 磁头所在块号指针
    设置 磁头方向指针
    打印 begin_pos 和 begin_direction
    while(1){
            根据方向选择下一个请求
            while(遍历所有请求直到block_num为结束标记 -1){
                记录当前块号到遍历数组
                打印当前服务的块号
                移动磁头到当前块
                释放当前请求相关的内存
            }
            更改搜索方向
            如果新方向没有请求，则终止循环
    }
    清零请求计数
    返回 遍历数组
}
```

​	在关卡中，我们只需要实现while(1)结构中的语句即可，实现代码如下：

```c
int flag = disk_head_look->block_num;  //设置磁盘初始位置指针，用于反向检查时跳过
    while (1) {
        if (*begin_direction == 1 && request_count){  //方向未增大且有请求
            current = disk_head_look->next;  //设置当前指针为下一个位置
            while(current != list_tail_look && current->block_num != -1){
                if (current->block_num == flag){  //遍历到初始位置，跳过
                    current = current->next;
                    continue;
                }
                traversed_blocks[count] = current->block_num; //将操作块号记入
                count++;
                kprintf("Serving block: %d\n", current->block_num);
                *begin_pos = current->block_num;  //设置开始位置为当前释放位置
                disk_head_look->next = current->next;  //调整双向链表，将下一个请求设置在disk_head_look后，并清除当前请求在链表中空间，完成请求
                if(current->next != NULL){
                    current->next->prev = disk_head_look;
                }
                kfree(current);
                request_count--;
                current = disk_head_look->next;
            }
            *begin_direction = -1;  //处理完请求或者遍历到一边终点退出，反向
        }
        if (*begin_direction == -1 && request_count){  //与正向过程相同
            current = disk_head_look->prev;
            while(current != list_head_look && current->block_num != -1){
                if (current->block_num == flag){
                    current = current->prev;
                    continue;
                }
                traversed_blocks[count] = current->block_num;
                count++;
                kprintf("Serving block: %d\n", current->block_num);
                *begin_pos = current->block_num;
                disk_head_look->prev = current->prev;
                if(current->prev != NULL){
                    current->prev->next = disk_head_look;
                }
                kfree(current);
                request_count--;
                current = disk_head_look->prev;
            }
            *begin_direction = 1;
        }
        break;
    }
    *begin_pos = flag;  //设置begin_pos为初始磁盘位置，用于第二问计算时间
    *size = count;
```

​	代码思路较为简单，首先初始化一个标志标记磁头起始位置（这是因为begin_pos虽然没有在请求队列中，但是由于初始化原因，begin_pos成为了双向链表的一部分，需要跳过），随后读取遍历方向，正向则将current指针设置为当前初始位置的next链表，表示向LBA增大的顺序遍历，此后未到达链表尾便每继续遍历，同时对请求-1，直到到达链表尾部；反方向同理。最终在都没有请求后退出即可。

​	这里需要注意在反向时，如果经过了磁盘初始位置需要进行跳过，防止读入不需要的请求。此外在释放块之前，还需要对双向链表的结构进行更新：

```c
	disk_head_look->prev = current->prev;
	if(current->prev != NULL){
    	current->prev->next = disk_head_look;
    }
	kfree(current);
```

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab712.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab711.png" style="zoom:67%;" />

​	评测结果如图，关卡通过。其中有不通过的原因为将初始磁头位置也计入了请求队列中，应当删去跳过。

## 3 关卡代码

```c
int* look_schedule(int *begin_pos, int *begin_direction, int *size) {
    struct bio* current;
    int* traversed_blocks = (int*)kmalloc(request_count * sizeof(int));
    int count = 0;

    *begin_pos = disk_head_look->block_num;
    *begin_direction = direction_look;
    kprintf("begin_pos = %d, begin_direction = %d\n", *begin_pos, *begin_direction);
#ifdef LAB10_EX1
    // LAB10 EXERCISE1: YOUR CODE
    int flag = disk_head_look->block_num;
    while (1) {
        if (*begin_direction == 1 && request_count){
            current = disk_head_look->next;
            while(current != list_tail_look && current->block_num != -1){
                if (current->block_num == flag){
                    current = current->next;
                    continue;
                }
                traversed_blocks[count] = current->block_num;
                count++;
                kprintf("Serving block: %d\n", current->block_num);
                *begin_pos = current->block_num;
                disk_head_look->next = current->next;
                if(current->next != NULL){
                    current->next->prev = disk_head_look;
                }
                kfree(current);
                request_count--;
                current = disk_head_look->next;
            }
            *begin_direction = -1;
        }
        if (*begin_direction == -1 && request_count){
            current = disk_head_look->prev;
            while(current != list_head_look && current->block_num != -1){
                if (current->block_num == flag){
                    current = current->prev;
                    continue;
                }
                traversed_blocks[count] = current->block_num;
                count++;
                kprintf("Serving block: %d\n", current->block_num);
                *begin_pos = current->block_num;
                disk_head_look->prev = current->prev;
                if(current->prev != NULL){
                    current->prev->next = disk_head_look;
                }
                kfree(current);
                request_count--;
                current = disk_head_look->prev;
            }
            *begin_direction = 1;
        }
        break;
    }
    *begin_pos = flag;
    *size = count;
#endif
    request_count = 0;
    return traversed_blocks;
}
```

