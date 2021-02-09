#include <stdio.h>
#include <stdlib.h>

int main()
{
  const char *path = "test";

  FILE *file = fopen(path, "rwb");

  uint16_t data = 0xf025;

  fwrite(&data, sizeof(data), 1, file);

  uint16_t *p = malloc(sizeof(uint16_t));

  fread(p, sizeof(data), 1, file);

  printf("read:%0x,%0x\n", *p & 0xff, *p >> 8);

  fclose(file);

  uint16_t t = 65;
  char *p1 = (char *)&t;
  printf("%0d,%0d\n", p1[0], p1[1]);

  char c1 = getchar();
  printf("address:%0x, %0x\n", c1, &c1);

  printf("%d\n", sizeof("123"));
  return 0;
}