# proj10实训五：**文件系统**

# 第三关理论知识检测

------

​	本部分为选择题，直接进行解答。

## 1 题目解答

### 1.1<img src="F:\study\操作系统\OS_comp\picture\lab533.png" style="zoom:50%;" />

解答：函数源代码如下：

```c
static int
sfs_bmap_load_nolock(struct sfs_fs *sfs, struct sfs_inode *sin, uint32_t index, uint32_t *ino_store) {
    struct sfs_disk_inode *din = sin->din;
    assert(index <= din->blocks);
    int ret;
    uint32_t ino;
    bool create = (index == din->blocks);
    if ((ret = sfs_bmap_get_nolock(sfs, sin, index, create, &ino)) != 0) {
        return ret;
    }
    assert(sfs_block_inuse(sfs, ino));
    if (create) {
        din->blocks ++;
    }
    if (ino_store != NULL) {
        *ino_store = ino;
    }
    return 0;
}
```

​	可以看到，函数执行的是一个加载或创建文件的块映射（block map）功能。其核心部分如下：

- 调用 `sfs_bmap_get_nolock` 函数尝试获取或创建一个新的块索引（`ino`）。这个函数会返回一个整型值表示操作结果，如果操作失败则返回非零值。
- `create` 变量是一个布尔值，当 `index` 等于 `din->blocks` 时为 `true`，表示需要创建一个新的块索引。
- 如果 `sfs_bmap_get_nolock` 返回非零值，即操作失败，`sfs_bmap_load_nolock` 函数将直接返回这个错误值。
- 如果 `ino_store` 不是 `NULL`，则将获取或创建的块索引 `ino` 存储到 `ino_store` 指向的位置。

​	因此该函数执行的是一个加载文件磁盘块映射的功能。

### 1.2<img src="F:\study\操作系统\OS_comp\picture\lab534.png" style="zoom:50%;" />

解答：在代码中如果发现文件读取位置偏移量未对齐，函数会首先处理未对齐的部分，再循环处理对齐部分。函数源代码中（详见第一关代码）未对齐处理的语句再对齐处理语句上。

### 1.3<img src="F:\study\操作系统\OS_comp\picture\lab535.png" style="zoom:50%;" />

解答：在练习二中我们通过文件描述符进行可执行文件的加载。函数的参数中使用了int整型fd来获取文件，fd一般指文件描述符。

### 1.4<img src="F:\study\操作系统\OS_comp\picture\lab536.png" style="zoom:50%;" />

解答：错误。索引节点在文件系统中的作用是从文件的个体角度描述特定文件的元数据和数据位置，而不是从文件系统的全局角度描述全局信息。每个文件或目录都有一个对应的索引节点，其中包含了该文件或目录的权限、所有者、大小、时间戳、数据块位置等信息。这些信息是特定于单个文件或目录的，而不是整个文件系统的全局信息。文件系统的全局信息通常由其他结构来描述，如超级块（superblock），它包含了整个文件系统的元数据，例如inode和数据块的数量、大小、空闲和已使用的块列表等。

## 2 实验结果

<img src="F:\study\操作系统\OS_comp\picture\lab531.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab532.png" style="zoom:67%;" />

​	评测结果如图，关卡通过。

