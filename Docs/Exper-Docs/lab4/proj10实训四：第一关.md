# proj10实训四：管道通信

# 第一关完善PIPE相关数据结构和函数

------

## 1 知识阅读

​	本关需要学习pipe相关数据结构的定义，完善pipe初始化函数的功能。

### 1.1 代码阅读

```c
// kern/process/proc.c
#define PIPESIZE 128
struct pipe {
    char* addr;  // vaddr of pipe
    int pipe_size;
    int nread;     // number of bytes read
    int nwrite;    // number of bytes written
    int readopen;   // read fd is still open
    int writeopen;  // write fd is still open
};
struct pipe_port{
    bool port_type; //port type(read or write)
    struct pipe *pipe; //which pipe the port belongs to
    bool readable; 
    bool writable;
};
struct pipe *pipe_enty=NULL;
struct pipe_port *ppt0=NULL;
struct pipe_port *ppt1=NULL;
```

​	上面的代码定义了pipe和pipe_port两个结构体，分别是管道、管道接口的定义，数据结构的属性包含管道和接口的使用情况。

#### 1.1.1 pipe 结构体

​	**用途**：表示一个管道，用于进程间通信。属性如下：

1. `addr`：管道数据在虚拟内存中的地址。

2. `pipe_size`：管道的容量，即可以存储的最大数据量。

3. `nread`：已从管道中读取的字节数。

4. `nwrite`：已写入管道的字节数。

5. `readopen`：指示读文件描述符是否打开。

6. `writeopen`：指示写文件描述符是否打开。

#### 1.1.2 pipe_port 结构体

​	**用途**：表示管道的端口，可以是读端口或写端口。属性如下：

1. `port_type`：布尔值，标识端口类型（读或写）。

2. `pipe`：指向所属`pipe`结构体的指针。

3. `readable`：指示端口是否可读。

4. `writable`：指示端口是否可写。

#### 1.1.3 全局变量

1. `pipe_enty`：指向`pipe`结构体的全局指针，可能用于表示当前进程的管道实体。

2. `ppt0`和`ppt1`：指向`pipe_port`结构体的全局指针，可能分别用于表示管道的读端和写端端口。

## 2 实验步骤

### 2.1 代码补全

​	待补全代码如下：

```c
int pipealloc(struct pipe_port *p0, struct pipe_port *p1)
{
    pipe_enty = (struct pipe*)kmalloc(sizeof(struct pipe));
    // LAB5 YOUR CODE EX1 -----------------
    // 使用kmalloc为pipe_port分配空间
    // LAB5 YOUR CODE EX1 -----------------
    pipe_enty->addr= kmalloc(128);
    pipe_enty->readopen = 1;
    pipe_enty->writeopen = 1;
    pipe_enty->nwrite = 0;
    pipe_enty->nread = 0;
    ppt0->readable = 1;
    ppt0->writable = 0;
    ppt1->readable = 0;
    ppt1->writable = 1;
    ppt0->pipe=pipe_enty;
    ppt1->pipe=pipe_enty;
    return 0;
}
```

​	该函数用于对管道进行初始化，并对管道端口分配对应的管道。实验需要完成该函数的剩余部分：为两个管道端口分配空间，实现形式如该函数的第一行，两个端口指针为ppt1和ppt0，在上一个代码块末尾已经声明。

​	直接仿照给出的示例，写出代码如下：

```c
	ppt0 = (struct pipe_port*)kmalloc(sizeof(struct pipe_port));
    ppt1 = (struct pipe_port*)kmalloc(sizeof(struct pipe_port));
```

​	首先为`pipe`结构体分配空间，然后为两个`pipe_port`结构体分配空间即可。

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab411.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab412.png" style="zoom:67%;" />

​	评测结果如上图，过关成功。

## 3 关卡代码

```c
int pipealloc(struct pipe_port *p0, struct pipe_port *p1)
{
    pipe_enty = (struct pipe*)kmalloc(sizeof(struct pipe));
    // LAB5 YOUR CODE EX1 -----------------
    // use kmalloc to alloc space for pipe_port
    ppt0 = (struct pipe_port*)kmalloc(sizeof(struct pipe_port));
    ppt1 = (struct pipe_port*)kmalloc(sizeof(struct pipe_port));
    // LAB5 YOUR CODE EX1 -----------------
    pipe_enty->addr= kmalloc(128);
    pipe_enty->readopen = 1;
    pipe_enty->writeopen = 1;
    pipe_enty->nwrite = 0;
    pipe_enty->nread = 0;

    ppt0->readable = 1;
    ppt0->writable = 0;
    ppt1->readable = 0;
    ppt1->writable = 1;
    ppt0->pipe=pipe_enty;
    ppt1->pipe=pipe_enty;

    return 0;
}
```

