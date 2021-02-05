#include <stdio.h>
#include <stdlib.h>

// 内存区
uint16_t mem[UINT16_MAX] = {
    // ADD R0,R1,R2
    0x1042,

    // ADD R1,R2,#6
    0x12A6,

    // JSR 2;下一条是第 4 条指令，跳转到第 4+2=6 条指令
    0x4802,

    // JMP R1;跳转到第 6 条指令
    0xC040,

    // RTI
    0X8000,

    //RES
    0XD000,

    // NOT R3, R1
    0x967F,

    // trap-halt
    0xf000,
};

// 程序运行状态
int running = 1;

// 寄存器定义
typedef enum
{
  R_R0,
  R_R1,
  R_R2,
  R_R3,
  R_R4,
  R_R5,
  R_R6,
  R_R7,
  R_PC,
  R_COND,
  R_COUNT
} Registers;

// 寄存器数组
uint16_t reg[R_COUNT];

// 宏便捷定义
// PC 寄存器
#define PC (reg[R_PC])

// 标志寄存器
#define COND (reg[R_COND])

// 指令定义
typedef enum
{
  OP_BR = 0,    // 条件分支
  OP_ADD = 1,   // 加法
  OP_LD = 2,    // load
  OP_ST = 3,    // store
  OP_JSR = 4,   // jump resgister
  OP_AND = 5,   // 与运算
  OP_LDR = 6,   // load register
  OP_STR = 7,   // store register
  OP_RTI = 8,   // unused
  OP_NOT = 9,   // 取反
  OP_LDI = 10,  // load indirect
  OP_STI = 11,  // store indirect
  OP_JMP = 12,  // jump
  OP_RES = 13,  // reserved
  OP_LEA = 14,  // load effective address
  OP_TRAP = 15, // trap，陷阱，相当于中断
} InstructionSet;

// 标志定义
typedef enum
{
  FL_POS = 1 << 0, // 正数
  FL_ZRO = 1 << 1, // 0
  FL_NEG = 1 << 2  //负数
} ConditionFlags;

// 从内存读取数据
uint16_t mem_read(int address)
{
  if (address < 0 || address >= UINT16_MAX)
  {
    printf("memory read error!\n");
    exit(4);
  }

  return mem[address];
}

// 将 data 写入内存地址为 address 处
void mem_write(uint16_t address, uint16_t data)
{
  if (address < 0 || address >= UINT16_MAX)
  {
    printf("memory write error!\n");
    exit(3);
  }

  mem[address] = data;
}

// 符号扩展
uint16_t sign_extend(uint16_t x, int bit_count)
{
  // 最高位 1
  if ((x >> (bit_count - 1)) & 0x1)
  {
    x |= 0xFFFF << bit_count;
  }

  return x;
}

// 更新标志寄存器
void update_flags(uint16_t r)
{
  uint16_t value = reg[r];
  if (value == 0)
  {
    COND = FL_ZRO;
  }
  else if ((value >> 15) & 0x1)
  {
    // 最高位 1，负数
    COND = FL_NEG;
  }
  else
  {
    COND = FL_POS;
  }
}

// 加法指令，两种模式
// add r0, r1, imm， 立即数模式
// add r0, r1, r2，寄存器模式
void add(int instr)
{
  // 取出目的寄存器 r0，9~11，占 3 位，与上 111
  uint16_t r0 = (instr >> 9) & 0x7;

  // 源寄存器 r1，6~8 位，
  uint16_t r1 = (instr >> 6) & 0x7;

  // add 模式
  uint16_t flag = (instr >> 5) & 0x1;

  // 立即数模式
  if (flag)
  {
    // 低五位，取出立即数。
    uint16_t data = instr & 0x1F;

    printf("add imm mode, imm:%d\n", data);

    // 符号扩展，若高位是 1，则全部补 1
    uint16_t value = sign_extend(data, 5);

    printf("add imm mode, sign_extend imm:%d\n", value);

    reg[r0] = reg[r1] + value;
  }
  else
  {
    puts("add reg mode");

    // 寄存器模式
    // 取出源寄存器 2，低 3 位
    uint16_t r2 = instr & 0x7;

    reg[r0] = reg[r1] + reg[r2];
  }

  printf("reg_%d value:%d\n", r0, reg[r0]);

  // 更新标志寄存器
  update_flags(r0);
}

// 与运算，同 add ，两种模式
void and (uint16_t instr)
{
  // 取出目的寄存器 r0，9~11，占 3 位，与上 111
  uint16_t r0 = (instr >> 9) & 0x7;

  // 源寄存器 r1，6~8 位，
  uint16_t r1 = (instr >> 6) & 0x7;

  // add 模式
  uint16_t flag = (instr >> 5) & 0x1;

  // 立即数模式
  if (flag)
  {
    // 低五位，取出立即数。
    uint16_t data = instr & 0x1F;

    printf("add imm mode, imm:%d\n", data);

    // 符号扩展，若高位是 1，则全部补 1
    uint16_t value = sign_extend(data, 5);

    printf("add imm mode, sign_extend imm:%d\n", value);

    reg[r0] = reg[r1] & value;
  }
  else
  {
    puts("add reg mode");

    // 寄存器模式
    // 取出源寄存器 2，低 3 位
    uint16_t r2 = instr & 0x7;

    reg[r0] = reg[r1] & reg[r2];
  }

  printf("reg_%d value:%d\n", r0, reg[r0]);

  // 更新标志寄存器
  update_flags(r0);
}

// NOT r0, r1。将 r1 取反后，放入 r0
void not(uint16_t instr)
{
  // 取出目的寄存器 r0，9~11，占 3 位，与上 111
  uint16_t r0 = (instr >> 9) & 0x7;

  // 源寄存器 r1，6~8 位，
  uint16_t r1 = (instr >> 6) & 0x7;

  reg[r0] = ~reg[r1];
  update_flags(r0);
}

// 标志条件跳转
// br cond_flag, pc_offset
void branch(uint16_t instr)
{
  uint16_t cond_flag = (instr >> 9) & 0x7;
  uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

  // 传入标识与标志寄存器的值相符，N,P,Z
  if (cond_flag & COND)
  {
    PC += pc_offset;
  }
}

// jump r
// 跳转到寄存器中的值
void jump(uint16_t instr)
{
  uint16_t r1 = (instr >> 6) & 0x7;
  PC = reg[r1];
}

// load indirect，从内存中获取数据，放入寄存器。间接模式
// 以 pc 寄存器作为偏移基准
// ldi dr, pc_offset
// [[pc+pc_offset]]，pc+pc_offset 中的内容是数据的地址。
void load_indirect(uint16_t instr)
{
  uint16_t pc_offset = instr & 0x1ff;

  // 符号扩展
  pc_offset = sign_extend(pc_offset, 9);

  uint16_t r = (instr >> 9) & 0x7;

  // 取出存储数据的地址
  uint16_t address = mem_read(PC + pc_offset);

  // 取出数据
  uint16_t data = mem_read(address);

  // 更新寄存器
  reg[r] = data;

  // 更新标志寄存器
  update_flags(r);
}

// 将地址放入寄存器 r
// 以 pc 寄存器作为偏移基准
// lea r, pc_offset
void load_effective_address(uint16_t instr)
{
  uint16_t pc_offset = instr & 0x1ff;

  // 符号扩展
  pc_offset = sign_extend(pc_offset, 9);

  uint16_t address = PC + pc_offset;

  uint16_t r = (instr >> 9) & 0x7;

  // 更新寄存器
  reg[r] = address;

  // 更新标志寄存器
  update_flags(r);
}

// jump resgister
// 偏移量跳转
void jump_subroutine(uint16_t instr)
{
  uint16_t long_flag = (instr >> 11) & 0x1;

  // R7 保存 pc 值
  reg[R_R7] = PC;

  if (long_flag)
  {
    // long_pc_offset
    uint16_t long_pc_offset = sign_extend(instr & 0x7ff, 11);
    PC += long_pc_offset;
  }
  else
  {
    uint16_t r1 = (instr >> 6) & 0x7;
    PC = reg[r1];
  }
}

// ld r, pc_offset
// 以 pc 寄存器作为偏移基准
// 将距离下一条指令 pc_offset 处里的数据取出来，放入 r 中。
void load(uint16_t instr)
{
  uint16_t pc_offset = sign_extend(instr & 0x1ff, 9);
  uint16_t r0 = (instr >> 9) & 0x7;
  reg[r0] = mem_read(PC + pc_offset);
  update_flags(r0);
}

// ldr r0, r1, offset
// 以 r1 作为偏移基准
// 将距离 r1，offset 处的数据取出来，放入 r0。
void load_register(uint16_t instr)
{
  uint16_t r0 = (instr >> 9) & 0x7;

  uint16_t r1 = (instr >> 6) & 0x7;

  uint16_t offset = sign_extend(instr & 0x3f, 6);

  uint16_t address = reg[r1] + offset;
  uint16_t value = mem_read(address);

  reg[r0] = value;
  update_flags(r0);
}

// st r, pc_offset
// 以 pc 寄存器作为偏移基准
// 将 r 中的数据放入距离下一条指令，pc_offset 的地址中。
void store(uint16_t instr)
{
  uint16_t pc_offset = sign_extend(instr & 0x1ff, 9);
  uint16_t r0 = (instr >> 9) & 0x7;

  uint16_t address = PC + pc_offset;
  uint16_t value = reg[r0];

  mem_write(address, value);
}

// sti r, pc_offset，间接存储，pc+pc_offset 是待存储数据地址的地址。
void store_indirect(uint16_t instr)
{
  uint16_t pc_offset = sign_extend(instr & 0x1ff, 9);
  uint16_t r0 = (instr >> 9) & 0x7;

  uint16_t indirect_address = PC + pc_offset;
  uint16_t address = mem_read(indirect_address);

  uint16_t value = reg[r0];

  mem_write(address, value);
}

// str r0, r1, offsets
// 以 r1 作为偏移基准
void store_register(uint16_t instr)
{
  // r0
  uint16_t r0 = (instr >> 9) & 0x7;

  // r1
  uint16_t r1 = (instr >> 6) & 0x7;

  uint16_t offset = sign_extend(instr & 0x3f, 6);

  uint16_t address = reg[r1] + offset;
  uint16_t value = reg[r0];

  mem_write(address, value);
}

// op = 1111
void trap(int instr)
{
  running = 0;
}

int main(int argc, const char *argv[])
{
  // 设置初始值
  PC = 0;

  // 用于打印当前执行操作码
  const char *op_list[] = {"BR", "ADD", "LD", "ST", "JSR", "AND", "LDR", "STR", "RTI", "NOT", "LDI", "STI", "JMP", "RES", "LEA", "TRAP"};

  while (running)
  {
    // 读取指令
    u_int16_t instr = mem_read(PC++);

    // 指令操作码占 4 位
    u_int16_t op = instr >> 12;

    printf("exe op:%s\n", op_list[op]);

    switch (op)
    {
    case OP_ADD:
    {
      add(instr);
      break;
    }

    case OP_AND:
    {
      and(instr);
      break;
    }

    case OP_NOT:
    {
      not(instr);
      break;
    }

    case OP_BR:
    {
      branch(instr);
      break;
    }

    case OP_JMP:
    {
      jump(instr);
      break;
    }

    case OP_JSR:
    {
      jump_subroutine(instr);
      break;
    }

    case OP_LD:
    {
      load(instr);
      break;
    }

    case OP_LDI:
    {
      load_indirect(instr);
      break;
    }

    case OP_LDR:
    {
      load_register(instr);
      break;
    }

    case OP_LEA:
    {
      load_effective_address(instr);
      break;
    }

    case OP_ST:
    {
      store(instr);
      break;
    }

    case OP_STI:
    {
      store_indirect(instr);
      break;
    }

    case OP_STR:
    {
      store_register(instr);
      break;
    }

    case OP_TRAP:
    {
      trap(instr);
      break;
    }

    default:
    {
      printf("Unknown OpCode!\n");
    }
    break;
    }
  }

  return 0;
}