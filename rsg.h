
#ifndef rsg
#define rsg 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <zlib.h>
#include <png.h>
#include <errno.h>
#include <pngconf.h>
struct s_lcg{
  unsigned long int seed;
  int a;
  int c;
  int m;
  int n;
  int current;
};
typedef struct s_lcg Lcg;
 
struct s_png_conf{
  png_structp read_pngp;
  png_infop   read_infop;
  png_structp write_pngp;
  png_infop   write_infop;
  png_uint_32 width; 
  png_uint_32 height;
  png_bytep *row_pointers;
  int color_type ;
  int bit_depth ;
};
typedef struct s_png_conf Conf;

void next_number(Lcg*);
int* generate(int,Lcg*);
int * fy_shuffle(Lcg*);
int generate_single(Lcg*);
Lcg* init_seed(char*,int);
Conf* init(char*);
void free_config(Conf*);
#endif // !rsg
