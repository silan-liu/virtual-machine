#include <stdio.h>
#include <stdbool.h>

// 指令定义
typedef enum
{
  PSH,  // PSH 5;              ::将数据放入栈中
  POP,  // POP;                ::栈顶指针-1
  SET,  // SET reg, 3;         ::给寄存器赋值
  HLT,  // HLT;                ::停止程序
  MOV,  // MOV reg1, reg2;     ::将寄存器 reg2 中的值放入 reg1
  ADD,  // ADD;                ::取出栈中的两个数据相加后，结果放入栈中
  SUB,  // SUB;                ::取出栈中的两个数据相减后，结果放入栈中
  DIV,  // DIV;                ::取出栈中的两个数据相除后，结果放入栈中
  MUL,  // MUL;                ::取出栈中的两个数据相乘后，结果放入栈中
  STR,  // STR reg;            ::将寄存器的数据放入栈中
  LDR,  // LDR reg;            ::将栈顶数据放入寄存器
  IF,   // IF reg, value, ip;  ::如果 reg 的值等于 value，则跳转到新 ip 指向的指令。
  LOGR, // LOG reg;            ::打印寄存器中的数据
} InstructionSet;

// 寄存器类型定义
typedef enum
{
  A,
  B,
  C,
  D,
  E,
  F,  // A-F 通用寄存器
  IP, // IP 寄存器
  SP, // 栈顶指针寄存器
  NUM_OF_REGISTERS
} Registers;

// 寄存器
int registers[NUM_OF_REGISTERS];

// 程序
const int program[] = {

    IF, B, 0, 6,
    PSH, 1,
    PSH, 2,
    PSH, 3,
    PSH, 4,
    PSH, 5,
    PSH, 6,
    PSH, 7,
    PSH, 8,
    ADD,
    MUL,
    DIV,
    SUB,
    POP,
    SET, C, 2,
    LOGR, C,
    MOV, B, A,
    STR, B,
    LDR, C,
    HLT};

#define sp (registers[SP])
#define ip (registers[IP])

int stack[256];

bool is_jump = false;
bool running = true;

void printStack()
{
  printf("\n\n=========begin print stack:=========\n\n");

  for (int i = 0; i <= sp; i++)
  {
    printf("%d ", stack[i]);

    // 4 个一行
    if ((i + 1) % 4 == 0)
    {
      printf("\n");
    }
  }

  printf("\n\n=========print stack done=========\n\n");
}

void printRegisters()
{
  printf("\n\n=========begin print registers:=========\n\n");
  for (int i = 0; i < NUM_OF_REGISTERS; i++)
  {
    printf("%d ", registers[i]);
  }

  printf("\n\n=========print registers done=========\n\n");
}

void eval(int instr)
{
  is_jump = false;

  switch (instr)
  {
  case HLT:
  {
    running = false;
    break;
  }

  case PSH:
  {
    stack[++sp] = program[++ip];
    break;
  }

  case POP:
  {
    sp--;
    break;
  }

  case ADD:
  {
    // 从栈中取出两个数，相加，再 push 回栈
    int a = stack[sp--];
    int b = stack[sp--];

    int result = a + b;

    stack[++sp] = result;

    registers[A] = result;

    break;
  }

  case SUB:
  {
    // 从栈中取出两个数，相减，再 push 回栈
    int a = stack[sp--];
    int b = stack[sp--];

    int result = b - a;

    // 入栈
    stack[++sp] = result;
    registers[A] = result;

    break;
  }

  case MUL:
  {
    // 从栈中取出两个数，相乘，再 push 回栈
    int a = stack[sp--];
    int b = stack[sp--];

    int result = a * b;

    // 入栈
    stack[++sp] = result;
    registers[A] = result;

    break;
  }

  case DIV:
  {
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

    break;
  }

  case MOV:
  {
    // 将一个寄存器的值放到另一个寄存器中
    // 目的寄存器
    int dr = program[++ip];

    // 源寄存器
    int sr = program[++ip];

    // 源寄存器的值
    int sourceValue = registers[sr];
    registers[dr] = sourceValue;

    break;
  }

  case STR:
  {
    // 将指定寄存器中的参数，放入栈中
    int r = program[++ip];
    stack[++sp] = registers[r];
    break;
  }

  case LDR:
  {
    int value = stack[sp];
    int r = program[++ip];
    registers[r] = value;
    break;
  }

  case IF:
  {
    // 如果寄存器的值和后面的数值相等，则跳转
    int r = program[++ip];
    if (registers[r] == program[++ip])
    {
      ip = program[++ip];

      is_jump = true;
      printf("jump if:%d\n", ip);
    }
    else
    {
      ip += 1;
    }
    break;
  }

  case SET:
  {
    // set register value
    int r = program[++ip];
    int value = program[++ip];

    registers[r] = value;
    break;
  }

  case LOGR:
  {
    int r = program[++ip];
    int value = registers[r];
    printf("log register_%d %d\n", r, value);
    break;
  }

  default:
  {
    printf("Unknown Instruction %d!\n", instr);
    break;
  }
  }
}

int main()
{
  //  初始化寄存器
  sp = -1;
  ip = 0;

  while (running)
  {
    int instr = program[ip];
    eval(instr);

    if (!is_jump)
    {
      ip++;
    }
  }

  printStack();

  printRegisters();

  return 0;
}
