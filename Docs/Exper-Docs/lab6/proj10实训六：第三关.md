# proj10实训六**I/O管理**

# 第三关中断的使能和禁用

------

## 1 知识阅读

​	本关要求实现intr.c中的中断使能和禁用函数。通过模拟 I/O 操作触发中断，验证中断使能和禁用函数的正确性。

​	中断是处理I/O操作中不可或缺的一部分，它允许CPU在处理其他任务时能够响应来自外部设备的请求，进行快速处理。在I/O操作过程中，设备完成数据传输后会产生中断信号，通知CPU进行相应的处理。这种机制可以显著提高系统的效率，因为CPU不需要持续轮询设备状态，从而节省了大量的处理时间和资源。中断处理的优点在于可以快速响应I/O设备的状态变化，提高数据传输的实时性和系统的整体性能。

​	中断使能函数在intr.c中，函数主要通过_csrxchg函数来对控制状态器寄存器中的CRMD来进行配置。

​	由龙芯架构32位精简版参考手册可知，CRMD的第2位，控制中断的使能，所以可以通过csrxchg函数来对其进行修改，csrxchg是特权指令，它根据通用寄存器rj中存放的写掩码信息，将通用寄存器rd中的旧值写入到指定 CSR中对应写掩码为1的那些比特，该CSR中的其余比特保持不变，同时将该CSR的旧值更新到通用寄存器 rd 中。

### 1.1 手册阅读（CRMD）

​	该寄存器的信息可以决定处理器核当前所处的特权等级、全局中断使能和地址翻译模式。其中关键的位有：

| 对应位名字 | 读写 | 描述                                                         |
| ---------- | ---- | ------------------------------------------------------------ |
| IE         | RW   | 当前全局中断使能，高位有效。触发后该位会被硬件置为0，确保陷入后屏蔽中断。 |

## 2 实验步骤

### 2.1 代码补全

​	待补全代码为：

```c
// LAB9 EXERCISE3: YOUR CODE
//You should check the manual to complete the function
/* intr_enable - enable irq interrupt */
void intr_enable(void) {
    __csrxchg(, , );
}
/* intr_disable - disable irq interrupt */
void intr_disable(void) {
    __csrxchg(, , );
}
```

​	这里的__csrxchg()函数是一个内嵌的汇编指令，为原子操作，该操作在手册中描述如下：

​	**汇编指令：csrxchg  rd, rj, csr_num**

​	csrxchg指令根据通用寄存器rj中存放的写掩码信息，将通用寄存器rd中的旧值写入到指定CSR中对应写掩码为1的比特位上，该CSR的其余比特位不变，同时将该CSR的旧值更新到rd中。

​	因此如果需要激活中断，就要指定csr_num为CSR_CRMD，并将写入位rj指定为CRMD_IE进行更新，rd此时需要设置为CRMD_IE，通过写掩码直接将IE位赋值为1。

​	如果需要禁用中断，根据手册指导，硬件直接将改为置为0，即指令中rd直接为0，写入到rj中，即CRMD_IE中。

​	因此补全代码如下，这里还需要注意系统架构使用的型号，这里如果直接使用CSR_CRMD会出现未定义报错，因此查看头文件intr.h，发现其使用LISA作为前缀，LISA一般是定制化的架构，用于专用领域（如实验室研究或嵌入式系统），因此添加前缀：

```c
void intr_enable(void) {
	__csrxchg(LISA_CSR_CRMD_IE, LISA_CSR_CRMD_IE, LISA_CSR_CRMD);
}
/* intr_disable - disable irq interrupt */
void intr_disable(void) {
	__csrxchg(0, LISA_CSR_CRMD_IE, LISA_CSR_CRMD);
}
```

### 2.2 评测结果

<img src="F:\study\操作系统\OS_comp\picture\lab31.png" style="zoom:67%;" />

<img src="F:\study\操作系统\OS_comp\picture\lab632.png" style="zoom:67%;" />

​	评测结果如图，关卡通过。

## 3 关卡代码

```c
void intr_enable(void) {
	__csrxchg(LISA_CSR_CRMD_IE, LISA_CSR_CRMD_IE, LISA_CSR_CRMD);
}
/* intr_disable - disable irq interrupt */
void intr_disable(void) {
	__csrxchg(0, LISA_CSR_CRMD_IE, LISA_CSR_CRMD);
}
```

