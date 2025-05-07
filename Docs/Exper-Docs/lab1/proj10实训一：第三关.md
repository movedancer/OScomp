# proj10实训一：裸机程序

# 第三关实现时钟中断处理函数clock_int_handler

------

## 1 知识阅读

​	这一关的任务主要是完成时钟中断处理函数，需要对时钟中断进行处理的部分填写trap函数中处理时钟中断的代码，使操作系统每遇到100次时钟中断后，调用kprintf，向屏幕上打印一行文字”100 ticks”。

​	系统中配置的时钟中断计数周期到达时，LoongArch CPU会生成一个时钟中断，是一个硬件中断，此时CPU会保存当前处理模式（PLV）和中断使能（IE）到CSR.PRMD的PPLV和IE中，并将CSR.CRMD的PLV和IE置为0，同时记录当前指令的地址（PC）到CSR.ERA中，然后跳转到异常入口点（CSR.EBASE寄存器指定的地址）。程序进入异常入口会经过一系列数据保存与状态保存，并到达trap函数进行例外处理，在本关卡中，clock_int_handler被调用，在ticks自增时记录时钟中断触发次数，并判断是否需要打印通知。处理完毕后程序将返回之前的汇编程序，从寄存器中还原现场，回复中断前的程序执行过程，完成整个中断处理过程。

## 2 实验步骤

### 2.1 代码补全

​	待完善代码：

```c
int clock_int_handler(void * data)
{
#ifdef LAB1_EX2
  // LAB1 EXERCISE2: YOUR CODE
  // (1) count ticks here
  __________;
#ifdef _SHOW_100_TICKS
  // (2) if ticks % 100 == 0 then call kprintf to print "100 ticks"
  ______________________________;
#endif
#endif
#ifdef LAB4_EX1
  run_timer_list();
#endif
  reload_timer();
  return 0;
}
```

​	实验给出的部分代码比较完善，空缺部分主要为时钟计数与例外打印。由于代码较为简单，这里直接给出：

```c
//（1）count ticks here
ticks++;
// (2) if ticks % 100 == 0 then call kprintf to print "100 ticks"
if(ticks % 100 ++ 0 ){
    kprintf("100 ticks\n");
}
```

​	直接对ticks进行累加，并使用余操作判断即可。

### 2.2 测试结果

<img src="F:\study\操作系统\OS_comp\picture\lab1-3-2.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab1-3-1.png" style="zoom:67%;" />

​	提交测评，完成测试。

## 3 关卡代码

clock_int_handler函数补全

```c
int clock_int_handler(void * data)
{
#ifdef LAB1_EX2
  // LAB1 EXERCISE2: YOUR CODE
  // (1) count ticks here
  ticks++;
#ifdef _SHOW_100_TICKS
  // (2) if ticks % 100 == 0 then call kprintf to print "100 ticks"
  if(ticks % 100 == 0 ){
    kprintf("100 ticks\n");
  }
#endif
#endif
#ifdef LAB4_EX1
  run_timer_list();
#endif
  reload_timer();
  return 0;
}
```

