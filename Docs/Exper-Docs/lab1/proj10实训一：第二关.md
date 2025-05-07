# proj10实训一：裸机程序

# 第二关编写一个简单的裸机程序

------

## 1 知识阅读

​	在进行知识积累的时候了解过，裸机程序是一个运行在无操作系统环境中的程序。阅读下表可以发现，逻辑程序的所有运行步骤都需要自己配置完成而不借助操作系统：

| 对比对象     | 裸机程序                                                     | 常规用户程序                                          |
| ------------ | ------------------------------------------------------------ | ----------------------------------------------------- |
| 内存地址空间 | 自行管理物理地址空间，可以自行对虚拟内存进行配置后使自己运行在虚拟地址空间 | 由操作系统管理的虚拟地址空间（不考虑Linux BOMMU模式） |
| 系统调用     | 调用自己                                                     | 调用更高特权级的操作系统/固件                         |
| 栈的初始化   | 自行完成                                                     | 操作系统载入用户进程时完成                            |
| BSS段的清空  | 自行完成                                                     | 操作系统分配虚拟页面时完成清零                        |

## 2 实验步骤

### 2.1 源文件编译基础

​	下面开始认识关卡，首先通过编译一个简单的C程序完成对裸机程序的进一步认知：

```C++
#include <stdio.h>
int main() {
  printf("Hello World\n");
  return 0;
}
```

​	执行以下命令编译为目标文件并查看目标文件的头：

```bash
gcc hello.c -c -o hello.o
objdump -h hello.o
```

​	我们可以从输出中得到如下结果：

<img src="F:\study\操作系统\OS_comp\picture\lab1-2.png" style="zoom: 67%;" />

​	这里可以看到程序分为了.text、.data、.bss、.rodata各存储段。

### 2.2 初始化的汇编代码

​	随后编写start.S，该程序主要完成了对CSR的DMWIN的设置，并修改CSR_CRMD开启虚拟地址翻译模式，然后从栈地址直接进入到main函数。

<img src="F:\study\操作系统\OS_comp\picture\91D68CD88DE559AEF4FD99A4EB9389A0.png" style="zoom:67%;" />

1. `.extern main`：声明外部符号`main`，这意味着`main`函数在其他地方定义，这里只是引用。

2. `.text`：指示接下来的代码属于程序的文本段（代码段）。

3. `.globl _start`：声明`_start`为全局符号，使其在整个程序中可见。

4. `_start:`：标记程序的入口点。

   随后代码设置了直接映射窗口与设置分页并启用，完成了MMU窗口的配置，一个用于缓存区域，另一个用于非缓存区域。

   最后将主函数`main`的地址加载到寄存器中，并跳转到该地址执行，从而开始程序的运行。

### 2.3 编写简单串口输出程序

​	main函数的编写如下，该程序是一个简单的串口打印程序。通过MMIO方式访问串口，并编写ns16550a（串口规格）对应的驱动完成打印：

![](F:\study\操作系统\OS_comp\picture\lab1-2-2.png)

### 2.4 编写链接脚本

​	链接脚本的工作就是指定一个起始地址，并删去程序中不需要的存储区段：

![](F:\study\操作系统\OS_comp\picture\喇叭-2-3.png)

### 2.5 编写Makefile文件

​	这是本关卡的最后一份代码，编写Makefile文件进行文件编译：

<img src="F:\study\操作系统\OS_comp\picture\lab-2-4.png" style="zoom:67%;" />

### 2.6 测试结果

<img src="F:\study\操作系统\OS_comp\picture\7AF7347D020ADD5CDE2175FA95905984.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\Lab-2-5.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\C59FCFB7B40095445769844E16955970.png" style="zoom:67%;" />

​	可以看到测评点均通过，关卡完成

## 3 关卡代码

### 3.1 start.S

```risc-V
.extern main
.text
.globl _start
_start:
    # Config direct window and set PG
    li.w    $t0, 0xa0000011
    csrwr   $t0, 0x180
    /* CSR_DMWIN0(0x180): 0xa0000000-0xbfffffff->0x00000000-0x1fffffff Cached */
    li.w    $t0, 0x80000001
    /* CSR_DMWIN1(0x181): 0x80000000-0x9fffffff->0x00000000-0x1fffffff Uncached */
    # Enable PG
    li.w    $t0, 0xb0
    csrwr   $t0, 0x0
    /* CSR_CRMD(0x0): PLV=0, IE=0, PG */
    la  $sp, bootstacktop
    la  $t0, main
    jr  $t0
poweroff:
    b poweroff
_stack:
.section .data
    .global bootstack
bootstack:
    .space 1024
    .global bootstacktop
bootstacktop:
    .space 64
```

### 3.2 main.c

```c
#define UART_BASE 0x9fe001e0
#define UART_RX     0   /* In:  Receive buffer */
#define UART_TX     0   /* Out: Transmit buffer */
#define UART_LSR    5   /* In:  Line Status Register */
#define UART_LSR_TEMT       0x40 /* Transmitter empty */
#define UART_LSR_THRE       0x20 /* Transmit-hold-register empty */
#define UART_LSR_DR         0x01 /* Receiver data ready */
void uart_put_c(char c) {
    while (!(*((volatile char*)UART_BASE + UART_LSR) & (UART_LSR_THRE)));
    *((volatile char*)UART_BASE + UART_TX) = c;
}
void print_s(const char *c) {
    while (*c) {
        uart_put_c(*c);
        c ++;
    }
}
void main() {
    print_s("\nHere is my first bare-metal machine program on LoongArch32!\n\n");
}
```

### 3.3 lab0.ld

```c
SECTIONS
{
    . = 0xa0000000;
    .text : { *(.text) }
    .rodata : { *(.rodata) }
    .bss : { *(.bss) }
}
```

### 3.4 Makefile

```makefile
TOOL    :=  loongarch32r-linux-gnusf-
CC      :=  $(TOOL)gcc
OBJCOPY :=  $(TOOL)objcopy
OBJDUMP :=  $(TOOL)objdump
QEMU    :=  qemu-system-loongarch32
.PHONY: clean qemu
start.elf: start.S main.c lab0.ld
   $(CC) -nostdlib -T lab0.ld start.S main.c -O3 -o $@
qemu: start.elf
   $(QEMU) -M ls3a5k32 -m 32M -kernel start.elf -nographic
clean:
   rm start.elf
```

