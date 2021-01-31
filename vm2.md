上一篇文章，我们实现了一个最小的虚拟机，但是它还不太完善。今天，我们在原来的基础上继续添砖加瓦，变得更加有血有肉。

新增功能如下：

1. 实现更多指令，比如 `MOV、SUB、DIV、MUL、LOGR、IF、SET、LDR、STR`。
2. 引入寄存器。
3. 实现条件跳转，也就是上面的 IF 指令。

## 寄存器定义

寄存器在硬件中的功能是用于存取数据。这里呢，我们使用数组来模拟。照葫芦画瓢，定义如下几种寄存器。

- 通用寄存器：A、B、C、D、E、F。
- IP 寄存器：IP。
- 栈顶指针寄存器：SP。

具体定义如下：

```objectivec
// 寄存器类型定义
typedef enum
{
  A,B,C,D,E,F,      // 通用寄存器
  IP,               // IP 寄存器
  SP,               // 栈顶指针寄存器
  NUM_OF_REGISTERS
} Registers;

// 寄存器
int registers[NUM_OF_REGISTERS];
```

`NUM_OF_REGISTERS` 是一个取巧的设计，可以很方便的知道有多少个寄存器。

那么获取某个寄存器的方式就很简单了，`registers[r]`，`r` 是寄存器下标。比如获取 A 寄存器，使用 `registers[A]` 即可。

另外，为了方便存取 `IP、SP`，将其定义为宏，简化处理。

```objectivec
#define sp (registers[SP])
#define ip (registers[IP])
```

同样，将 `sp` 初始化为 `-1`，`ip` 初始化为 `0`。

```objectivec
//  初始化寄存器
sp = -1;
ip = 0;
```

## 新增指令定义

新增下图中的几种指令：

![指令定义](https://cdn.jsdelivr.net/gh/silan-liu/picRepo/img20210130074425.png)

## 指令实现

有了上一篇的基础后，要完成这几个指令应该是信手拈来。下面，我们一个个来实现。

### SET

SET 主要用于给某个寄存器赋值。分 2 步走：

1. 取出两个参数，分别是寄存器下标和数据。

```objectivec
// 寄存器下标
int r = program[++ip];

// 值
int value = program[++ip];
```

2. 更新寄存器的值。

```objectivec
registers[r] = value;
```

### MOV

MOV 用于寄存器之间的赋值。分三步走：

1. 取出 2 个参数，目标寄存器和源寄存器。

```objectivec
// 目的寄存器
int dr = program[++ip];

// 源寄存器
int sr = program[++ip];
```

2. 取出源寄存器的值。

```objectivec
// 源寄存器的值
int sourceValue = registers[sr];
```

3. 赋值给目的寄存器。

```objectivec
registers[dr] = sourceValue;
```

### SUB

SUB 用于取出栈中的两个数相减，将结果放回栈中，同时也保存到寄存器 A 中。分 4 步走：

1. 取出栈中的两个数。

```objectivec
// 从栈中取出两个数
int a = stack[sp--];
int b = stack[sp--];
```

2. 计算相减结果，注意顺序。

```objectivec
int result = b - a;
```

3. 结果放回栈中。

```objectivec
// 入栈
stack[++sp] = result;
```

4. 结果放到寄存器 A 中。

```objectivec
registers[A] = result;
```

### DIV

DIV，除法指令。操作步骤同 SUB。

注意被除数为 0 的情况。

```objectivec
// 从栈中取出两个数，相除，再 push 回栈
int a = stack[sp--];
int b = stack[sp--];

if (a != 0)
{
  int result = b / a;

  // 入栈
  stack[++sp] = result;
  registers[A] = result;
}
else
{
  printf("exception occur, divid 0 \n");
}
```

### MUL

乘法指令，步骤同 SUB。

### LOGR

LOGR 用于打印寄存器的值。实现也很简单，分 2 步走：

1. 取出寄存器的值。

```objectivec
// 寄存器下标
int r = program[++ip];

// 寄存器的值
int value = registers[r];
```

2. 调用 printf 进行打印。

```objectivec
printf("log register_%d %d\n", r, value);
```

### STR

STR 相当于存储指令，将指定寄存器中的数据放入栈中。分 2 步走：

1. 取出寄存器参数。

```objectivec
int r = program[++ip];
```

2. 将寄存器中数据放入栈中。

```objectivec
stack[++sp] = registers[r];
```

### LDR

LDR 相当于取数据指令，将栈顶数据放入指定寄存器中。分 2 步走：

1. 取栈顶数据和寄存器，但栈顶指针不变。

```objectivec
// 只取栈顶数据，sp 指针不变
int value = stack[sp];

// 取出寄存器参数
int r = program[++ip];
```

2. 将数据放入寄存器中

```objectivec
registers[r] = value;
```

### IF

唯一有些不同的就是 IF 条件跳转指令。当满足跳转条件时，会直接跳转到新指令处。

前面的实现中，在执行完某个条指令后，会默认指向下一条指令。但如果要进行跳转，ip 就不能再继续加 1，只需将 ip 设为新指令下标即可。

所以呢，需要额外添加变量 `is_jump` 来标识是否`要执行跳转操作`。在取指令的 while 循环中，判断若不是跳转操作，则 ip++，继续指向下一条指令。

```objectivec
while (running)
  {
    int instr = program[ip];
    eval(instr);

		// 非跳转，才 +1
    if (!is_jump)
    {
      ip++;
    }
  }
```

IF 指令的处理，分为下面几个步骤：

1. 取出前两个参数，分别是寄存器下标和待比较的数据。

```objectivec
// 寄存器下标
int r = program[++ip];

// 要比较的值
int value = program[++ip];
```

2. 判断寄存器中的数据与数值 value 是否相等。如果相等，则进行下面的操作。
* 取出新指令下标，并更新 ip 寄存器的值。
* 更重要的一点，更新标识 is_jump，表示是跳转操作。

```objectivec
if (registers[r] == value)
{
  // 新指令下标
  ip = program[++ip];

  // 更新为跳转操作
  is_jump = true;
}
```

可能有同学会疑惑，当 is_jump 更新为 true 之后，什么时候还原呢？很简单啦，在执行每条指令之前，将其置为 false 就好了。

至此，新增的功能就全部实现了。完整代码可[点击文末链接 2 查看](https://github.com/silan-liu/virtual-machine/blob/master/mac/vm_2.c)。

## 总结

这篇文章，我们主要完成了寄存器的定义、几个新指令的实现，其中还包括 IF 跳转指令。希望对你有帮助。

下一篇文章，将会进入更高阶的玩法，写一个更贴近真实定义的虚拟机。包括不限于：

- 指令定义标准化，操作码+操作数
- 指令的解析与执行
- 丰富常见指令集，比如逻辑运算、内存读写
- 引入标志寄存器
- 指令以二进制存储
- 中断处理
- ...

准备好了吗？让我们迎着微光出发吧~

## 参考资料

- [https://github.com/felixangell/mac/tree/master/mac-improved](https://github.com/felixangell/mac/tree/master/mac-improved)
- [https://github.com/silan-liu/virtual-machine/blob/master/mac/vm_2.c](https://github.com/silan-liu/virtual-machine/blob/master/mac/vm_2.c)

