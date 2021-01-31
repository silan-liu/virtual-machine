#include <stdio.h>
#include <stdlib.h>

int main()
{
  const char *path = "test";
  const char msg[] = "Hello!";

  printf("msg size:%d\n", sizeof(msg) / sizeof(char));

  FILE *file = fopen(path, "rwb");

  uint16_t data = 0xf025;
  // int16_t binary = 0x25;

  // fwrite(&binary, sizeof(binary), 1, file);
  fwrite(&data, sizeof(data), 1, file);

  uint16_t *p = malloc(sizeof(uint16_t));

  fread(p, sizeof(data), 1, file);

  printf("read:%0x,%0x\n", *p & 0xff, *p >> 8);

  fclose(file);

  return 0;
}