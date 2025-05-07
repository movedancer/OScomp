# proj10实训六**I/O管理**

# 第一关io缓冲区传输操作的实现

------

## 1 知识阅读

​	本关要求完善iobuf_move()函数，并理解其作用。

### 1.1 数据传输与缓冲区管理

​	iobuf.c/iobuf_move()用于io缓冲区的数据传输，数据传输是磁盘I/O操作的核心。读操作将数据从磁盘块设备读取到内存缓冲区，写操作将数据从内存缓冲区写入磁盘块设备。数据传输通常通过DMA（直接内存访问）或PIO（编程I/O）方式进行。DMA方式可以减少CPU的负担，提高传输效率。缓冲区管理是指在内存中分配、使用和释放数据缓冲区的过程。在磁盘I/O操作中，缓冲区管理至关重要，可以提高数据传输的效率和可靠性。操作系统通常会维护一个缓冲区池，用于存储正在传输的数据。

### 1.2 Iobuf机制

​	在操作系统中，I/O（输入/输出）操作是核心功能之一，涉及数据在内存和外部设备（如硬盘、SSD等）之间的传输。为了高效地管理这些数据传输，ucore利用了iobuf（输入/输出缓冲区）数据结构。iobuf的主要作用是作为内存和设备之间的数据桥梁，管理数据的传输过程。它包含以下几个关键字段：

1. io_base，指向当前数据缓冲区的起始地址；
2. io_offset，指示当前操作的偏移量；
3. io_resid，表示剩余的数据量。 

​	iobuf结构共同工作，确保数据在内存和设备之间有效地移动，同时保持操作的正确性和数据的完整性。iobuf_move函数是一个核心函数，用于在iobuf结构和用户空间或其他缓冲区之间移动数据。这个函数的设计直接影响系统在进行文件读写操作时的性能和效率。在实现iobuf_move时，需要考虑数据方向、数据完整性和错误处理等因素。

## 2 实验步骤

### 2.1 代码补全

​	待补全代码如下：

```c
int iobuf_move(struct iobuf *iob, void *data, size_t len, bool m2b, size_t *copiedp) {
    size_t alen;
    if ((alen = iob->io_resid) > len) {
        alen = len;
    }
// LAB9 EXERCISE1: YOUR CODE
//(1)Determine whether the data size to be transferred is greater than 0Determine if Alen is greater than 0
//(2)Determine the direction of transmission based on m2b
//(3)transfer date and update iobuf
//You may need use the 'memmove()' and  'iobuf_skip()' functions
    if (copiedp != NULL) {
        *copiedp = alen;
    }
    return (len == 0) ? 0 : -E_NO_MEM;
}
```

​	阅读函数与注释，可以得知：iobuf_move()函数首先确定实际可以移动的数据长度alen，并根据m2b标志决定数据的移动方向。接着，通过memmove()函数将数据从源缓冲区复制到目标缓冲区，然后调用iobuf_skip()函数更新iobuf的偏移量和剩余数据量，最后返回实际移动的数据长度。

​	iobuf_skip函数实现如下，其接收*iob指针获取I/O操作信息，接收n为跳过的字节数。

```c
void iobuf_skip(struct iobuf *iob, size_t n) {
    assert(iob->io_resid >= n);
    iob->io_base += n, iob->io_offset += n, iob->io_resid -= n;
}
```

​	补全代码如下：

```c
	if (alen == 0) {
        if (copiedp != NULL) {
            *copiedp = 0;
        }
        return 0;
    }
    if (m2b) {
        memmove(iob->io_base, data, alen);
        iobuf_skip(iob, alen);
        len -= alen;
    } else {
        memmove(data, iob->io_base, alen);
        iobuf_skip(iob, alen);
        len -= alen;
    }
```

​	首先判断可传输字节数是否为0，为0则直接返回。不为0进行判断，如果m2b为1，则表示从memory读取到iobuf中，调用menmove()；反之则为从iobuf到memory。操作完毕使用iobuf_skip更新指针，跳过已操作数据，更新用户请求的字节数len，跟踪用户剩下的字节数。

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab611.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab612.png" style="zoom:67%;" />

​	评测结果如图，关卡通过。

## 3 关卡代码

```c
int iobuf_move(struct iobuf *iob, void *data, size_t len, bool m2b, size_t *copiedp) {
    size_t alen;
    if ((alen = iob->io_resid) > len) {
        alen = len;
    }
    if (alen == 0) {
        if (copiedp != NULL) {
            *copiedp = 0;
        }
        return 0;
    }
    if (m2b) {
        memmove(iob->io_base, data, alen);
        iobuf_skip(iob, alen);
        len -= alen;
    } else {
        memmove(data, iob->io_base, alen);
        iobuf_skip(iob, alen);
        len -= alen;
    }
    if (copiedp != NULL) {
        *copiedp = alen;
    }
    return (len == 0) ? 0 : -E_NO_MEM;
}
```

