
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
#include <argp.h>
#include <termios.h>
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

struct s_arguments {
    char *image_path;     
    int verbose;          
    int setup;
    int decode;
};
typedef struct s_arguments Arguments;
void next_number(Lcg*);
int* generate(int,Lcg*);
int * fy_shuffle(Lcg*);
int generate_single(Lcg*);
Lcg* init_seed(char*,int);
Conf* init(char*);
void free_config(Conf*);
void copy_file(char *, char *,char*);
void write_png_file(char *, png_structp , png_infop , png_bytep *);

void encode_bytes(Conf *, int *, char *);

char *decode_bytes(Conf *, int *) ;
 
void create_diff(Conf *, int *) ;
#endif // !rsg
