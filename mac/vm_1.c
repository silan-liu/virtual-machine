#include <stdio.h>
#include <stdbool.h>

// 指令定义
typedef enum
{
  PSH,
  ADD,
  POP,
  HLT,
} InstructionSet;

// 程序
const int program[] = {
    PSH, 5,
    PSH, 6,
    ADD,
    POP,
    HLT};

int ip = 0;
int sp = -1;
int stack[256];

bool running = true;

void eval(int instr)
{
  switch (instr)
  {
  case HLT:
    running = false;
    break;

  case PSH:
    sp++;
    stack[sp] = program[++ip];
    break;

  case POP:
  {
    int popValue = stack[sp--];
    printf("poped %d\n", popValue);
  }

  break;

  case ADD:
  {
    // 从栈中取出两个数，相加，再 push 回栈
    int a = stack[sp--];
    int b = stack[sp--];
    int sum = a + b;
    sp++;
    stack[sp] = sum;
  }

  break;

  default:
    break;
  }
}

int main()
{
  while (running)
  {
    int instr = program[ip];
    eval(instr);
    ip++;
  }

  return 0;
}