# proj10实训七：**LOOK调度算法的实现**

# 第二关统计调度一组磁盘I/O请求的读写时间

## 1 知识阅读

​	本关要求在实现了LOOK算法的调度之后，统计并打印调度一组磁盘I/O请求的读写时间。相关知识如下：

### 1.1 磁盘LBA到扇区、磁道、柱面的换算

​	逻辑块地址（LBA）是一个线性地址，需要转换为磁盘的物理地址（即柱面、磁道和扇区）。以下是换算公式：

1. 扇区号 = LBA % SPT（每个柱面扇区数）
2. 磁道号 = (LBA / SPT) % HPC（每个柱面柱面数）
3. 柱面号 = LBA / (SPT * HPC)

​	其中，SPT = 每个柱面扇区数，HPC = 每个柱面柱面数。

### 1.2 磁盘读写时间的计算

​	计算磁盘读写时间时，需要考虑以下几个因素：

1. 跨越磁道时间：每跨越一个磁道需要固定的时间。
2. 跨越扇区时间：每跨越一个扇区需要固定的时间。
3. 数据块传输时间：传输一个数据块所需的时间。
4. 设定时间参数为：

​	其中，TRACK_SEEK_TIME = 跨越磁道时间（单位：毫秒），SECTOR_SEEK_TIME = 跨越扇区时间（单位：毫秒），TIME_DATA_BLOCK_TRANSFER = 传输一个块数据时间（单位：毫秒）

## 2 实验步骤

### 2.1 代码补全

​	待补全代码如下：

```c
calculate_track_to_track_time(当前块号, 目的块号){
    计算当前块和下一块之间的磁道距离。
    返回磁道间距离的绝对值乘以磁道寻道时间。
}
calculate_sector_to_sector_time(当前块号, 目的块号){
    计算两个块之间的扇区距离。
    返回扇区间距离的正规化值乘以扇区寻道时间。
}
measure_time(遍历数组, 磁头所在块号，磁头运动方向){
    遍历每个块，累加总时间包括数据传输时间和每次移动到新块的磁道到磁道和扇区到扇区时间。
    更新开始位置为当前块位置。
    返回总时间。
}
```

​	这里需要注意的是，在计算跨磁道时间时，实际计算的是跨柱面的时间开销。在相同柱面上的不同磁道不需要时间开销，可以直接使用磁头进行读取。

​	补全代码如下：

```c
static int calculate_track_to_track_time(int current_block, int next_block) {
#ifdef LAB10_EX2
    // LAB10 EXERCISE2: YOUR CODE
    int current_cyi = current_block / (SPT * HPC);
    int next_cyi = next_block / (SPT * HPC);
    int track_distance = (current_cyi - next_cyi);
    if(track_distance < 0){
        track_distance = -track_distance;
    }
    return track_distance * TRACK_SEEK_TIME;
#endif
}

static int calculate_sector_to_sector_time(int current_block, int next_block) {
#ifdef LAB10_EX2
    // LAB10 EXERCISE2: YOUR CODE
    int current_sector = current_block % SPT;
    int next_sector = next_block % SPT;
    int sector_distance;
    if (current_sector>next_sector){
        sector_distance = next_sector + 50 - current_sector;
    }else{
        sector_distance = next_sector - current_sector;
    }
    return sector_distance * SECTOR_SEEK_TIME;
#endif
}

static int measure_time(int* traversed_blocks, int size, int begin_pos, int direction) {
#ifdef LAB10_EX2
    // LAB10 EXERCISE2: YOUR CODE
    int total_time = 0;
    int current_block = begin_pos;
    for (int i = 0; i < size; i++) {
        int next_block = traversed_blocks[i];
        total_time += calculate_track_to_track_time(current_block, next_block);
        total_time += calculate_sector_to_sector_time(current_block, next_block);
        total_time += TIME_DATA_BLOCK_TRANSFER;
        current_block = next_block;
    }
    return total_time;
#endif
}
```

​	这里还需要注意在计算扇区时，考虑磁盘单向转动，当目标扇区号<当前扇区号的时候，磁盘需要先转到50号扇区（0号），再转到目标扇区。

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab721.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab722.png" style="zoom:67%;" />

​	评测结果如上，关卡完成。

## 3 关卡代码

```c
static int calculate_track_to_track_time(int current_block, int next_block) {
#ifdef LAB10_EX2
    // LAB10 EXERCISE2: YOUR CODE
    int current_cyi = current_block / (SPT * HPC);
    int next_cyi = next_block / (SPT * HPC);
    int track_distance = (current_cyi - next_cyi);
    if(track_distance < 0){
        track_distance = -track_distance;
    }
    return track_distance * TRACK_SEEK_TIME;
#endif
}

static int calculate_sector_to_sector_time(int current_block, int next_block) {
#ifdef LAB10_EX2
    // LAB10 EXERCISE2: YOUR CODE
    int current_sector = current_block % SPT;
    int next_sector = next_block % SPT;
    int sector_distance;
    if (current_sector>next_sector){
        sector_distance = next_sector + 50 - current_sector;
    }else{
        sector_distance = next_sector - current_sector;
    }
    return sector_distance * SECTOR_SEEK_TIME;
#endif
}

static int measure_time(int* traversed_blocks, int size, int begin_pos, int direction) {
#ifdef LAB10_EX2
    // LAB10 EXERCISE2: YOUR CODE
    int total_time = 0;
    int current_block = begin_pos;
    for (int i = 0; i < size; i++) {
        int next_block = traversed_blocks[i];
        total_time += calculate_track_to_track_time(current_block, next_block);
        total_time += calculate_sector_to_sector_time(current_block, next_block);
        total_time += TIME_DATA_BLOCK_TRANSFER;
        current_block = next_block;
    }
    return total_time;
#endif
}
```

