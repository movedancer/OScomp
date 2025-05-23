# proj10实训五：文件系统

## 背景了解：文件系统

------

[TOC]

### 一、文件系统概述

​	文件系统是操作系统中负责管理持久数据的子系统，说简单点，就是负责把用户的文件存到磁盘硬件中，因为即使计算机断电了，磁盘里的数据并不会丢失，所以可以持久化的保存文件。文件系统的基本数据单位是文件，它的目的是对磁盘上的文件进行组织管理，那组织的方式不同，就会形成不同的文件系统。LINUX中有句经典：**一切皆文件**。

​	在Linux 文件系统会为每个文件分配两个数据结构：**索引节点（index node）**和**目录项（directory entry）**，它们主要用来记录文件的元信息和目录层次结构。

1. **索引节点**，也就是 inode，用来记录文件的元信息，比如 inode 编号、文件大小、访问权限、创建时间、修改时间、数据在磁盘的位置等等。索引节点是文件的唯一标识，它们之间一一对应，也同样都会被存储在硬盘中，所以索引节点同样占用磁盘空间。
2. **目录项**，也就是 dentry，用来记录文件的名字、索引节点指针以及与其他目录项的层级关联关系。多个目录项关联起来，就会形成目录结构，但它与索引节点不同的是，目录项是由内核维护的一个数据结构，不存放于磁盘，而是缓存在内存。

​	由于索引节点唯一标识一个文件，而目录项记录着文件的名，所以目录项和索引节点的关系是多对一，也就是说，一个文件可以有多个别字。比如，硬链接的实现就是多个目录项中的索引节点指向同一个文件。


​	注意，目录也是文件，也是用索引节点唯一标识，和普通文件不同的是，普通文件在磁盘里面保存的是文件数据，而目录文件在磁盘里面保存子目录或文件。

#### 1.1 目录项与目录

​	虽然名字很相近，但是它们不是一个东西，目录是个文件，持久化存储在磁盘，而目录项是内核一个数据结构，缓存在内存。


​	如果查询目录频繁从磁盘读，效率会很低，所以内核会把已经读过的目录用目录项这个数据结构缓存在内存，下次再次读到相同的目录时，只需从内存读就可以，大大提高了文件系统的效率。


​	**注意，目录项这个数据结构不只是表示目录，也是可以表示文件的。**

#### 1.2 关系概览

<img src="F:\study\操作系统\OS_comp\picture\lab501.png" style="zoom:67%;" />

​	索引节点是存储在硬盘上的数据，那么为了加速文件的访问，通常会把索引节点加载到内存中。另外，磁盘进行格式化的时候，会被分成三个存储区域，分别是超级块、索引节点区和数据块区。

1. 超级块，用来存储文件系统的详细信息，比如块个数、块大小、空闲块等等。
2. 索引节点区，用来存储索引节点；
3. 数据块区，用来存储文件或目录数据；

​	我们不可能把超级块和索引节点区全部加载到内存，这样内存肯定撑不住，所以只有当需要使用的时候，才将其加载进内存，它们加载进内存的时机是不同的：

1. 超级块：当文件系统挂载时进入内存；
2. 索引节点区：当文件被访问时进入内存；

### 二、文件分配

#### 2.1 连续分配

​	连续分配方式要求每个文件在磁盘上占有一道连续的块。

1. 优点：支持顺序访问和直接访问（即随机访问）；连续分配的文件在顺序访问时速度最快。
2. 缺点：不方便文件拓展；存储空间利用率低，会产生磁盘碎片。

#### 2.2 链接分配

​	链接分配采取离散分配的方式，可以为文件分配离散的磁盘块。分为隐式链接和显式链接两种。

#### 2.3 隐式链接

​	除文件的最后一个盘块外，每个盘块中都存有指向下一个盘块的指针。文件目录包括文件第一块的指针和最后一块的指针。

1. 优点：很方便文件拓展，不会有碎片问题，外存利用率高。
2. 缺点：只支持顺序访问，不支持随机访问，查找效率低，指向下一个盘块的指针也需要耗费少量的存储空间。

#### 2.4 显式链接

​	把用于链接文件各物理块的指针显式地存放在一张表中，即文件分配表（FAT, File Allocation Table）。一个磁盘只会建立一张文件分配表，开机时文件分配表放入内存，并常驻内存。

1. 优点：很方便文件拓展，不会有碎片问题，外存利用率高，并且支持随机访问。相比于隐式链接来说，地址转换时不需要访问磁盘，因此文件的访问效率更高。
2. 缺点：文件分配表需要占用一定的存储空间。

#### 2.5 索引分配

​	索引分配允许文件离散地分配在个磁盘块中，系统会为每个文件建立一张索引表，索引表中记录了文件的各个逻辑块对应的物理块（索引表的功能类似于内存管理中的页表——建立逻辑页面到物理页之间的映射关系）。索引表存放的磁盘块称为索引块，文件数据存放的磁盘块称为数据块。若文件太大，索引表项太多，可以采取以下三种方法解决：

1. 链接方案：如果索引表太大，一个索引块装不下，那么可以将多个索引块链接起来存放。缺点：若文件很大，索引表很长，就需要将很多个索引块链接起来。想要找到第i号索引块，必须先依次读入0~i-1号索引块，这就导致磁盘I/O次数过多，查找效率低下。

2. 多层索引：建立多层索引（原理类似于多级页表）。使第一层索引块指向第二层的索引块，还可根据文件大小的要求再建立第三层、第四层索引块。采用k层索引结构，且顶级索引表未调入内存，则访问一个数据块只需要k＋1次读磁盘操作。缺点：即使是小文件，访问一个数据块依然需要k＋1次读磁盘。

3. 混合索引：多种索引分配方式的结合。例如，一个文件的顶级索引表中，既包含直接地址索引（直接指向数据块），又包含一级间接索引（指向单层索引表），还包含两级间接索引（指向两层索引表）。优点：对于小文件来说，访问一个数据块所需的读磁盘次数更少。

待补充。。。

参考文章：

[1]操作系统——文件系统：https://blog.csdn.net/zhouhengzhe/article/details/123320438

[2]操作系统：文件的物理结构（文件分配方式）：https://blog.csdn.net/qq_40821469/article/details/105661645