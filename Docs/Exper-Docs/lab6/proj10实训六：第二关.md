# proj10实训六**I/O管理**

# 第二关块设备读写操作的实现

------

## 1 知识阅读

​	本关需要完善disk0_io（）函数，实现块设备的读写操作。

​	磁盘I/O（Disk I/O）是操作系统与存储设备（如硬盘、SSD）之间进行数据传输的核心过程。它是实现文件系统、虚拟内存管理等功能的基础。通过磁盘I/O操作，操作系统可以读取和写入存储设备上的数据。

### 1.1 块设备与字符设备

​	在磁盘I/O操作中，我们主要涉及两种设备：块设备和字符设备。块设备以固定大小的块（通常是512字节或更大）为单位进行数据传输，支持随机访问，即可以直接访问任意一个块的数据。典型的块设备包括硬盘、SSD等。字符设备则以字符为单位进行数据传输，通常不支持随机访问，典型的字符设备包括键盘、串口等。在本实验中主要关注块设备的操作。

### 1.2 块对齐与边界检查

​	块对齐和边界检查是磁盘I/O操作中的两个重要概念。在进行磁盘I/O操作时，数据的起始地址和长度通常需要是块大小的整数倍，这样可以确保数据的完整性和传输效率。块对齐检查可以防止部分块读写，提高I/O操作的效率。边界检查则确保I/O操作不会超出磁盘的边界，防止数据错误和越界访问。这些检查可以通过简单的取模运算实现，可以在disk0_io()函数中通过如下语句来判断块对齐和边界对齐：

 `if ((offset % DISK0_BLKSIZE) != 0 || (resid % DISK0_BLKSIZE) != 0) 和 if (blkno + nblks > dev->d_blocks)` 

## 2 实验步骤

### 2.1 代码补全

​	待补全代码如下：

```c
static int disk0_io(struct device *dev, struct iobuf *iob, bool write) {
    off_t offset = iob->io_offset;
    size_t resid = iob->io_resid;
    uint32_t blkno = offset / DISK0_BLKSIZE;
    uint32_t nblks = resid / DISK0_BLKSIZE;
    /* don't allow I/O that isn't block-aligned */
    if ((offset % DISK0_BLKSIZE) != 0 || (resid % DISK0_BLKSIZE) != 0) {
        return -E_INVAL;
    }
    /* don't allow I/O past the end of disk0 */
    if (blkno + nblks > dev->d_blocks) {
        return -E_INVAL;
    }
    /* read/write nothing ? */
    if (nblks == 0) {
        return 0;
    }
    lock_disk0();
    while (resid != 0) {
    // LAB9 EXERCISE2: YOUR CODE
    //(1)Distinguish Read and Write by using the 'write' flag
    //(2)Move data between 'iobuf' and 'disk0_buf'
    //Note that the order of read and write operations is different
    //(3)Ensure 'copied' is a multiple of 'DISK0_BLKSIZE' and move date between'disk0_buf' and disk0
    //Your may need to use 'iobuf_move()' , 'disk0_write_blks_nolock()' and 'disk0_read_blks_nolock()' functions 
    }
    unlock_disk0();
    return 0;
}
```

​	disk0_io函数负责处理块设备的读写请求。实现该函数时，需要遵循以下步骤：

1. 首先，计算起始块号 blkno 和需要传输的块数量 nblks。
2. 然后，进行块对齐和边界检查，确保I/O操作合法。
3. 使用 iobuf_move 函数在 I/O 缓冲区和磁盘缓冲区之间移动数据，并调用相应的磁盘读写函数（如 disk0_write_blks_nolock 和 disk0_read_blks_nolock）。
4. 最后，更新 resid 和 blkno，以反映传输的进度，并使用锁机制 lock_disk0 和 unlock_disk0 确保在多线程环境下操作的原子性和一致性。

​	部分函数阅读如下：

```c
static void disk0_write_blks_nolock(uint32_t blkno, uint32_t nblks) {
    int ret;
    uint32_t sectno = blkno * DISK0_BLK_NSECT, nsecs = nblks * DISK0_BLK_NSECT;
    if ((ret = ide_write_secs(DISK0_DEV_NO, sectno, disk0_buffer, nsecs)) != 0) {
        panic("disk0: write blkno = %d (sectno = %d), nblks = %d (nsecs = %d): 0x%08x.\n",
                blkno, sectno, nblks, nsecs, ret);
    }
}
```

​	函数 disk0_write_blks_nolock 的核心功能如下：

1. **接收参数**：

   （1）`uint32_t blkno`：表示要写入的起始块号（block number）。

   （2）`uint32_t nblks`：表示要写入的块数（number of blocks）。

2. **完成的功能**：

   （1）将 `nblks` 个块的数据从内存缓冲区 `disk0_buffer` 写入到磁盘0的 `blkno` 起始块号位置。

   （2）计算对应的扇区号和扇区数，每个块被转换为对应数量的扇区，因为磁盘通常以扇区为单位进行读写操作。

   （3）调用 `ide_write_secs` 函数，通过IDE接口将数据写入磁盘。

   ```c
   static void disk0_read_blks_nolock(uint32_t blkno, uint32_t nblks) {
       int ret;
       uint32_t sectno = blkno * DISK0_BLK_NSECT, nsecs = nblks * DISK0_BLK_NSECT;
       if ((ret = ide_read_secs(DISK0_DEV_NO, sectno, disk0_buffer, nsecs)) != 0) {
           panic("disk0: read blkno = %d (sectno = %d), nblks = %d (nsecs = %d): 0x%08x.\n",
                   blkno, sectno, nblks, nsecs, ret);
       }
   }
   ```

   函数 disk0_read_blks_nolock 同理。

​	因此写入部分可以完成如下：

```c
size_t copied, alen = DISK0_BUFSIZE;
    if (write) {
        iobuf_move(iob, disk0_buffer, alen, 0, &copied);
        if (copied % DISK0_BLKSIZE != 0) {
            return 0;
        }
        nblks = copied / DISK0_BLKSIZE;
        disk0_write_blks_nolock(blkno, nblks);
    }
```

​	对于写操作首先使用iobuf_move函数将iob写入到memory中，随后判断块对齐，获取copied为数据长度，检查对齐。对齐则直接设置块数nblks，随后使用disk0_write_blks_nolock函数写入。

​	对于读部分：

```c
else{
    if (alen > resid) {
        alen = resid;
    }
    nblks = alen / DISK0_BLKSIZE;
    disk0_read_blks_nolock(blkno, nblks);
    iobuf_move(iob, disk0_buffer, alen, 1, &copied);
    if (copied % DISK0_BLKSIZE != 0) {
        return 0;
        }
    }
    resid -= copied;
    blkno += nblks;
```

​	读的时候需要多做一个判断，当缓冲区长度大于剩余字节数的时候需要更新alen的值，确保不会读取超过剩余字节数的数据。随后与写操作相同，交换目的地址与源地址即可，同时也要检查是否块对齐。最后更新起始块地址与resid剩余字节数。

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab621.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab622.png" style="zoom:67%;" />

​	评测结果如图，关卡通过。

## 3 关卡代码

```c
static int disk0_io(struct device *dev, struct iobuf *iob, bool write) {
    off_t offset = iob->io_offset;
    size_t resid = iob->io_resid;
    uint32_t blkno = offset / DISK0_BLKSIZE;
    uint32_t nblks = resid / DISK0_BLKSIZE;
    /* don't allow I/O that isn't block-aligned */
    if ((offset % DISK0_BLKSIZE) != 0 || (resid % DISK0_BLKSIZE) != 0) {
        return -E_INVAL;
    }
    /* don't allow I/O past the end of disk0 */
    if (blkno + nblks > dev->d_blocks) {
        return -E_INVAL;
    }
    /* read/write nothing? */
    if (nblks == 0) {
        return 0;
    }
    lock_disk0();
    while (resid != 0) {
        size_t copied, alen = DISK0_BUFSIZE;
        if (write) {
            iobuf_move(iob, disk0_buffer, alen, 0, &copied);
            if (copied % DISK0_BLKSIZE != 0) {
                return 0;
            }
            nblks = copied / DISK0_BLKSIZE;
            disk0_write_blks_nolock(blkno, nblks);
        }
        else {
            if (alen > resid) {
                alen = resid;
            }
            nblks = alen / DISK0_BLKSIZE;
            disk0_read_blks_nolock(blkno, nblks);
            iobuf_move(iob, disk0_buffer, alen, 1, &copied);
            if (copied % DISK0_BLKSIZE != 0) {
                return 0;
            }
        }
        resid -= copied;
        blkno += nblks;
    }
    unlock_disk0();
    return 0;
}
```

