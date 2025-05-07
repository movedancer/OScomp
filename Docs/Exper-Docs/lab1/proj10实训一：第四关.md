# proj10实训一：裸机程序

# 第四关实现串口中断处理函数serial_int_handler

------

## 1 知识阅读

​	这一关主要针对kern/driver/console.c中的串口中断处理函数serial_int_handler，对该函数进行功能完善。该函数作用为处理串口中断，每接收到字符后读取该字符，中断正在打印的“100ticks”并输出“got input”+该字符，然后将处理器再交由原先进程。整体代码较为简单。

## 2 实验步骤

### 2.1 代码补全

​	项目提供的待补全代码如下：

```c
void serial_int_handler(void *opaque)
{
    unsigned char id = inb(COM1+COM_IIR);
    if(id & 0x01)
        return ;
    //int c = serial_proc_data();
    int c = cons_getc();
#if defined(LAB1_EX3) && defined(_SHOW_SERIAL_INPUT)
    // LAB1 EXERCISE3: YOUR CODE
    ________________;
#endif
#ifdef LAB4_EX2
    extern void dev_stdin_write(char c);
    dev_stdin_write(c);
#endif
}
```

​	该函数负责读取串口输入，代码首先使用char类型的id变量从串行端口COM1的中断识别寄存器并读取一个字节。随后检查id的最低为是否为1，这一步应该是检查中断重要性，对我们的功能实现无影响。随后**使用一个整形变量c从串行端口读取一个字符**，我们只需要将c打印出来即可：

```c
kprintf("got input %c\n", c);
```

### 2.2 测试结果

<img src="F:\study\操作系统\OS_comp\picture\lab1-4-1.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab1-4-2.png" style="zoom:67%;" />

提交测评，测试通过。

## 3 关卡代码

```c
void serial_int_handler(void *opaque)
{
    unsigned char id = inb(COM1+COM_IIR);
    if(id & 0x01)
        return ;
    //int c = serial_proc_data();
    int c = cons_getc();
#if defined(LAB1_EX3) && defined(_SHOW_SERIAL_INPUT)
    // LAB1 EXERCISE3: YOUR CODE
    kprintf("got input %c\n", c);
#endif
#ifdef LAB4_EX2
    extern void dev_stdin_write(char c);
    dev_stdin_write(c);
#endif
}
```

