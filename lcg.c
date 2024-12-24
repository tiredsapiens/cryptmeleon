#include "rsg.h"
#include <bits/types/struct_itimerspec.h>
void next_number(Lcg *s) {
  s->current = (((s->a) * (s->current) + (s->c)) % (s->m) + s->m) % s->m;
}

int *generate(int n, Lcg *s) {
  int *arr = malloc(n * sizeof(int));
  for (int i = 0; i < n; i++) {
    next_number(s);
    arr[i] = s->current;
  }
  return arr;
}
int generate_single(Lcg *s) {
  return (((s->a) * (s->current) + (s->c)) % (s->m) + s->m) % s->m;
}
