#include <stdio.h>
#include <stdlib.h>

// 内存区
uint16_t mem[UINT16_MAX] = {
    // add r0,r1,r2
    0x1042,

    // invalid opcode
    0b1111000011110000,

    // trap-halt
    0xd025,
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
  OP_BR = 0,  // 条件分支
  OP_ADD = 1, // 加法
  OP_LD = 2,  // load
  OP_ST,      // store
  OP_JSR,     // jump resgister
  OP_AND,     // 与运算
  OP_LDR,     // load register
  OP_STR,     // store register
  OP_NOT,     // 取反
  OP_LDI,     // load indirect
  OP_STI,     // store indirect
  OP_JMP,     // jump
  OP_LEA,     // load effective address
  OP_TRAP     // trap，陷阱，相当于中断
} InstructionSet;

// 标志定义
typedef enum
{
  FL_POS = 1 << 0, // 正数
  FL_ZRO = 1 << 1, // 0
  FL_NEG = 1 << 2  //负数
} ConditionFlags;

// 中断类型
typedef enum
{
  TRAP_GETC = 0x20, // 从键盘输入
  TRAP_OUT = 0x21,  //输出字符
  TRAP_PUTS = 0x22, // 输出字符串
  TARP_IN = 0x23,
  TRAP_PUTSP = 0x24,
  TRAP_HALT = 0x25, // 退出程序
} TrapSet;

// 从内存读取数据
uint16_t mem_read(int address)
{
  if (address < 0 || address > UINT16_MAX)
  {
    printf("memory read error!\n");
    exit(4);
  }

  return mem[address];
}

// 将 data 写入内存地址为 address 处
void mem_write(uint16_t address, uint16_t data)
{
  if (address < 0 || address > UINT16_MAX)
  {
    printf("memory write error!\n");
    exit(3);
  }

  mem[address] = data;
}

// 读取指令文件
int read_image(const char *image_path)
{
  return 0;
}

// 符号扩展
uint16_t sign_extend(uint16_t x, int bit_count)
{
  // 最高位 1
  if ((x >> (bit_count)) & 0x1)
  {
    x |= 0xFF << bit_count;
  }

  return x;
}

// 更新标志寄存器
void update_flags(uint16_t r)
{
  uint16_t value = reg[r];
  if (value == 0)
  {
    COND = 0;
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

void add(int instr)
{
  // 取出目的寄存器 dr，9~11，占 3 位，与上 111
  uint16_t dr = (instr >> 9) & 0x7;

  // 源寄存器 sr1，6~8 位，
  uint16_t sr1 = (instr >> 6) & 0x7;

  // add 模式
  uint16_t flag = (instr >> 5) & 0x1;

  // 立即数模式
  if (flag)
  {
    puts("add imm mode");

    // 低五位，取出立即数。
    uint16_t data = instr & 0x1F;

    // 符号扩展，若高位是 1，则全部补 1
    uint16_t value = sign_extend(data, 5);

    reg[dr] = reg[sr1] + value;
  }
  else
  {
    puts("add reg mode");

    // 寄存器模式
    // 取出源寄存器 2，低 3 位
    uint16_t sr2 = instr & 0x7;
    reg[dr] = reg[sr1] + reg[sr2];
  }

  // 更新标志寄存器
  update_flags(dr);
}

// load，从内存中获取数据，放入寄存器
// ld r, pc_offset
// [pc+pc_offset] 的值为待取数据
void ld(uint16_t instr)
{
  uint16_t pc_offset = instr & 0x1ff;

  // 符号扩展
  pc_offset = sign_extend(pc_offset, 9);

  uint16_t address = PC + pc_offset;
  uint16_t data = mem_read(address);

  uint16_t r = (instr >> 9) & 0x7;

  // 更新寄存器
  reg[r] = data;

  // 更新标志寄存器
  update_flags(r);
}

// load indirect，从内存中获取数据，放入寄存器
// ldi dr, pc_offset
// [[pc+pc_offset]]，pc+pc_offset 中的内容是数据的地址。
void ldi(uint16_t instr)
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

// 将地址放入寄存器
// lea r, pc_offset
void lea(uint16_t instr)
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

// 中断，op = 1111
void trap(int instr)
{
  // trap_code，低 8 位
  uint16_t trap_code = instr & 0xff;
  switch (trap_code)
  {
  case TRAP_GETC:
  {
    break;
  }

  case TRAP_OUT:
  {
    break;
  }

  case TRAP_PUTS:
  {
    break;
  }

  case TARP_IN:
  {
    break;
  }

  case TRAP_PUTSP:
  {
    break;
  }

  case TRAP_HALT:
  {
    puts("Halt");
    running = 0;
    break;
  }

  default:
  {
    printf("Unknown TrapCode!\n");
    break;
  }
  }
}

int main(int argc, const char *argv[])
{
  // if (argc < 2)
  // {
  //   printf("lc3 [image-file] ...\n");
  //   exit(2);
  // }

  // for (int i = 0; i < argc; i++)
  // {
  //   if (!read_image(argv[i]))
  //   {
  //     printf("failed to load image %s\n", argv[i]);
  //     exit(1);
  //   }
  // }

  // 设置初始值
  PC = 0;

  while (running)
  {
    // 读取指令
    u_int16_t instr = mem_read(PC++);

    // 指令操作码占 4 位
    u_int16_t op = instr >> 12;
    printf("op:%d\n", op);

    switch (op)
    {
    case OP_ADD:
    {
      add(instr);
      break;
    }

    case OP_AND:
    {
      break;
    }

    case OP_NOT:
    {
      break;
    }

    case OP_BR:
    {
      break;
    }

    case OP_JMP:
    {
      break;
    }

    case OP_JSR:
    {
      break;
    }

    case OP_LD:
    {
      break;
    }

    case OP_LDI:
    {
      ldi(instr);
      break;
    }

    case OP_LDR:
    {
      break;
    }

    case OP_LEA:
    {
      break;
    }

    case OP_ST:
    {
      break;
    }

    case OP_STI:
    {
      break;
    }

    case OP_STR:
    {
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