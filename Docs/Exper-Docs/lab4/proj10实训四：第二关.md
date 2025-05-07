# proj10实训四：管道通信

# 第二关实现管道通信并进行编译、测试

------

## 1 知识阅读

​	本关需要在了解管道读写基本原理的基础上，完善pipewrite()函数，实现内核线程之间的通信。需要参考**piperead函数**的实现方式，用for循环对大小为size的空间进行写入，写入地址为(pi->addr+w+x)（w是已经写入的空间，x是当前for循环遍历到的位置），写入值为(msg+x+w)。写入完成后需要将pipe已经写入的统计量nwrit加上size即：pi->nwrite += size;；并将本次写入的大小w加上size，即w += size。

### 1.1 piperead函数

```c
int piperead(struct pipe *pi, int n)
{
  // r 记录已经写的字节数
  int r = 0;
  // 若 pipe 可读内容为空，阻塞或者报错
  while(pi->nread == pi->nwrite) {
      if(pi->writeopen)
          do_yield();
      else
          return -1;
  }
  uint64_t size = MIN3(
  n - r,
  pi->nwrite - pi->nread,
  PIPESIZE - (pi->nread % PIPESIZE)
  );
  while(r < n && size!=0){
      kprintf("size %d\n",size);
      // pipe 可读内容为空，返回
      if(pi->nread == pi->nwrite)
          break;
      // 一次写的 size 为：min(用户buffer剩余，可读内容，pipe剩余线性容量)
      int x=0;
      for (;x<size;x+=1){
        kprintf("%c",*(pi->addr+r+x));
      }
      pi->nread += size;
      r += size;
  }
  return r;
}
```

​	该函数用于从管道中读取数据。它的工作流程如下：

1. 初始化：**r 变量记录已经读取的字节数**。
   检查管道是否为空：如果管道为空，且写端打开，调用do_yield() 等待数据；如果写端关闭，返回 -1。

2. **计算可读取的大小**：使用 MIN3 宏计算一次读取的最大字节数，这取决于用户缓冲区剩余大小、管道中可读数据大小以及环形缓冲区的线性剩余容量。

3. 读取数据：将数据从管道缓冲区复制到用户缓冲区，并**更新读指针 pi->nread**和已读取字节数 r。

### 1.2 环形缓冲区

​	环形缓冲区是一种固定大小的缓冲区，它的读写指针可以在缓冲区的两端循环使用。在上述实现中，pi->nwrite 和 pi->nread 分别是写指针和读指针。通过对这些指针取模操作（% PIPESIZE），实现了环形效果。

## 2 实验步骤

### 2.1 代码补全

​	我们需要补全的代码如下：

```c
// LAB5 CODES for communication
int pipewrite(struct pipe *pi, char* msg, int len)
{
    // w 记录已经写的字节数
    int w = 0;
    while(w < len){
        // 若不可读，写也没有意义
        if(pi->readopen == 0){
            return -1;
        }
        if(pi->nwrite == pi->nread + PIPESIZE){
            // pipe write 端已满，阻塞
            do_yield();
        } else {
            // 一次读的 size 为 min(用户buffer剩余，pipe 剩余写容量，pipe 剩余线性容量)
            int size = MIN3(
                len - w,
                pi->nread + PIPESIZE - pi->nwrite,
                PIPESIZE - (pi->nwrite % PIPESIZE)
            );
            kprintf("size %d\n",size);
            //LAB5 YOUR CODE EX2 -----------------
            //LAB5 YOUR CODE EX2 -----------------
        }
    }
    return w;
}
```

​	该函数用于将数据写入管道。它的工作流程如下：

1. 初始化：w 变量记录已经写入的字节数。

2. 循环写入：在循环中，不断将数据从用户缓冲区写入管道，直到所有数据都写入完毕或者遇到阻塞。

3. 检查读端是否打开：如果读端关闭（pi->readopen == 0），写操作无效，返回 -1。

4. 检查管道是否已满：如果管道已满（pi->nwrite == pi->nread + PIPESIZE），调用 do_yield() 进行让出处理器时间片，等待空间可用。

5. 计算可写入的大小：使用 MIN3 宏计算一次写入的最大字节数，这取决于用户缓冲区剩余大小、管道剩余空间大小以及环形缓冲区的线性剩余容量。

6. 写入数据：将数据从用户缓冲区复制到管道缓冲区，并更新写指针 pi->nwrite 和已写入字节数 w。

​	根据代码阅读与知识阅读中的提示，可以仿照piperead函数进行写入：

​	首先使用了一个`for`循环来遍历size指定的空间，并将msg缓冲区中的数据写入到管道的addr缓冲区中。在每次写入后，更新pi->nwrite和w的值，以反映已经写入的数据量。

```c
            for (int x = 0; x < size; x++) {
                pi->addr[pi->nwrite % PIPESIZE + x] = msg[w + x];
            }
            pi->nwrite += size;
            w += size;
```

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab421.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab422.png" style="zoom:67%;" />

​	评测结果如图，成功过关。

## 3 关卡代码

```c
// LAB5 CODES for communication
int pipewrite(struct pipe *pi, char* msg, int len)
{
    // w 记录已经写的字节数
    int w = 0;
    while(w < len){
        // 若不可读，写也没有意义
        if(pi->readopen == 0){
            return -1;
        }
        if(pi->nwrite == pi->nread + PIPESIZE){
            // pipe write 端已满，阻塞
            do_yield();
        } else {
            // 一次读的 size 为 min(用户buffer剩余，pipe 剩余写容量，pipe 剩余线性容量)
            int size = MIN3(
                len - w,
                pi->nread + PIPESIZE - pi->nwrite,
                PIPESIZE - (pi->nwrite % PIPESIZE)
            );
            kprintf("size %d\n",size);
            //LAB5 YOUR CODE EX2 -----------------
            for (int x = 0; x < size; x++) {
                pi->addr[pi->nwrite % PIPESIZE + x] = msg[w + x];
            }
            pi->nwrite += size;
            w += size;
            //LAB5 YOUR CODE EX2 -----------------
        }
    }
    return w;
}
```



