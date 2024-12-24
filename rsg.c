#include "rsg.h"
#include "lcg.c"
#include <fcntl.h>
#include <png.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int *fy_shuffle(Lcg *s) {
  int temp, rnd;
  int *arr = malloc(s->m * sizeof(int));
  for (int i = 0; i < s->m; i++) {
    arr[i] = i;
  }

  for (int i = s->m - 1; i > 0; i--) {
    s->m = i;
    rnd = generate_single(s);
    // printf("%d\n", rnd);
    temp = arr[i];
    arr[i] = arr[rnd];
    arr[rnd] = temp;
  }

  return arr;
}
unsigned long int hash(char *key) {
  //
  unsigned long hash = 5381;
  int c;
  while ((c = *key++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}
Lcg *init_seed(char *key, int range) {
  unsigned long int hashed_key = hash(key);
  Lcg *s = (Lcg *)malloc(sizeof(Lcg));
  s->seed = hashed_key;
  s->current = hashed_key;
  s->a = 987654321;
  s->c = 33;
  s->m = range;
  return s;
}
// int main() {
//
//   char *key = "heyyyy";
//   Lcg *t = init_seed(key);
//   t->m = 10;
//   int *arr = fy_shuffle(t);
//   printf("-----\n");
//   for (int i = 0; i < 10; i++) {
//     printf("%d\n", arr[i]);
//   }
//   return 0;
// }
