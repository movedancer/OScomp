# proj10实训五：**文件系统**

# 第一关读文件操作的实现

------

## 1 知识阅读

​	本关需要编写代码实现文件的读操作。该部分的主要任务是补充和完善项目工程文件/kern/fs/sfs/sfs_inode.c中的sfs_io_nolock()函数。该函数的目标功能为：从sin对应的文件中，依照给定的iob限制读取alen长度的数据，并存放在io_base中。

​	该函数主要分为三个部分：

1. 检查输入参数的合法性，根据给定的读/写控制变量write(bool)调整数据操作函数sfs_buf_op()和sfs_block_op()至相应的读/写操作模式。
2. 数据读/写。
3. 更新inode的相应控制信息，返回函数执行情况。

​	我们需要完善部分为第2部分，即“数据读/写”部分中的读操作内容。在进行文件读取时，单次读操作的基本工作单位是(盘)块，即单次读操作将加载相应块的所有内容到内存空间。一般情况下，单个文件通常会存储在若干个块上。当文件的起始或结束地址不是块长的倍数，即地址未对齐时，文件首/尾部的部分内容可能只占据相应块的部分存储空间，因此需要进行特殊处理。

### 1.1 代码阅读

​	**相关定义：**

```c
sfs_buf_op = sfs_rbuf;//sfs_rbuf:读取sfs文件系统中blkno指向的磁盘块中的len长度数据,并存放在buf指针指向的区域
sfs_block_op = sfs_rblock;//sfs_rblock:读取sfs文件系统中blkno指向的磁盘块中连续的nblks个块数据,并存放在buf指针指向的区域
uint32_t blkno = offset / SFS_BLKSIZE;//起始块号,offset为文件起始地址
uint32_t nblks = endpos / SFS_BLKSIZE - blkno;//文件跨越块数,endpos为文件结束地址
```

## 2 实验步骤

### 2.1 代码补全

​	待补全代码提示如下：

```c
//LAB4:EXERCISE1 YOUR CODE HINT: call sfs_bmap_load_nolock, sfs_rbuf, sfs_rblock,etc. read different kind of blocks in file
        /*
         * (1) If offset isn't aligned with the first block, Rd/Wr some content from offset to the end of the first block
         *       NOTICE: useful function: sfs_bmap_load_nolock, sfs_buf_op
         *               Rd/Wr size = (nblks != 0) ? (SFS_BLKSIZE - blkoff) : (endpos - offset)
         * (2) Rd/Wr aligned blocks 
         *       NOTICE: useful function: sfs_bmap_load_nolock, sfs_block_op
         * (3) If end position isn't aligned with the last block, Rd/Wr some content from begin to the (endpos % SFS_BLKSIZE) of the last block
         *       NOTICE: useful function: sfs_bmap_load_nolock, sfs_buf_op      
         */
```

​	通过释给出的步骤解读如下：

1. **非对齐的偏移量处理**：

   （1）如果数据的偏移量（offset）不是块（block）的边界对齐的，那么需要从偏移量开始读取或写入直到第一个块的末尾。

   （2）使用 `sfs_bmap_load_nolock` 来加载块映射，使用 `sfs_buf_op` 来执行读/写操作。

   （3）读/写的数据大小取决于是否还有更多的块需要处理（`nblks != 0`），如果有，则写入剩余的块大小（`SFS_BLKSIZE - blkoff`），否则写入到偏移量结束位置的数据（`endpos - offset`）。

2. **对齐的块处理**：

   （1）对于对齐的块，可以直接进行读/写操作。

   （2）使用 `sfs_bmap_load_nolock` 来加载块映射，使用 `sfs_block_op` 来执行块级别的读/写操作。

3. **非对齐的结束位置处理**：

   （1）如果结束位置（end position）不是块的边界对齐的，那么需要从块的开始读取或写入直到结束位置的扇区（`endpos % SFS_BLKSIZE`）。

   （2）使用 `sfs_bmap_load_nolock` 来加载块映射，使用 `sfs_buf_op` 来执行读/写操作。

​	可以根据解读给出完善代码如下：

```c
if((blkoff = offset % SFS_BLKSIZE)!= 0)//如果文件起始位置不对齐
    { 
        size = (nblks != 0) ? (SFS_BLKSIZE - blkoff) : (endpos - offset);//计算起始块需要读取的块长度
        if((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0)//计算文件索引号
            goto out;
        if ((ret = sfs_buf_op(sfs, buf, size, ino, blkoff)) != 0)//读文件
            goto out;
        
        alen += size;//更新已读取块长度
        if (nblks == 0)
            goto out;
        
        buf += size, blkno ++, nblks --;//更新缓冲区已读数据规模,当前块,剩余读取块数量
    }
    size = SFS_BLKSIZE;//对齐部分单次读取块长度恒定
    while(nblks != 0)//循环处理对齐部分
    {
        if((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0)//同上
            goto out;
        if((ret = sfs_block_op(sfs, buf, ino, 1)) != 0)
            goto out;
        
        alen += size, buf += size, blkno ++, nblks --;
    }
    if((size = endpos % SFS_BLKSIZE) != 0)//计算结束块读取的块长度,如果文件结束位置不对齐
    {
        if((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0)//同上
            goto out;
        if((ret = sfs_buf_op(sfs, buf, size, ino, 0)) != 0)
            goto out;
        alen += size;//同上
    }
```

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab511.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab512.png" style="zoom:67%;" />

​	评测结果如上，关卡完成。

## 3 关卡代码

```c
if((blkoff = offset % SFS_BLKSIZE)!= 0)//如果文件起始位置不对齐
    { 
        size = (nblks != 0) ? (SFS_BLKSIZE - blkoff) : (endpos - offset);//计算起始块需要读取的块长度
        if((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0)//计算文件索引号
            goto out;
        if ((ret = sfs_buf_op(sfs, buf, size, ino, blkoff)) != 0)//读文件
            goto out;
        
        alen += size;//更新已读取块长度
        if (nblks == 0)
            goto out;
        
        buf += size, blkno ++, nblks --;//更新缓冲区已读数据规模,当前块,剩余读取块数量
    }
    size = SFS_BLKSIZE;//对齐部分单次读取块长度恒定
    while(nblks != 0)//循环处理对齐部分
    {
        if((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0)//同上
            goto out;
        if((ret = sfs_block_op(sfs, buf, ino, 1)) != 0)
            goto out;
        
        alen += size, buf += size, blkno ++, nblks --;
    }
    if((size = endpos % SFS_BLKSIZE) != 0)//计算结束块读取的块长度,如果文件结束位置不对齐
    {
        if((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0)//同上
            goto out;
        if((ret = sfs_buf_op(sfs, buf, size, ino, 0)) != 0)
            goto out;
        alen += size;//同上
    }
```

​	将上述代码填充在LAB5所需位置即可。