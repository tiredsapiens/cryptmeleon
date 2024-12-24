#include "rsg.h"
#include <string.h>
Conf *init(char *path) {

  FILE *fd = fopen(path, "rb");
  Conf *ret = (Conf *)malloc(sizeof(Conf));
  unsigned char header[8];
  if (!fd) {
    printf("Error when trying to open file %s\n", path);
    exit(EXIT_FAILURE);
  }
  if (fread(header, 1, 8, fd) != 8) {
    perror("Failed to read PNG signature\n");
    fclose(fd);
    exit(EXIT_FAILURE);
  }
  if (png_sig_cmp(header, 0, 8)) {
    fprintf(stderr, "File is not a PNG.\n");
    fclose(fd);
    exit(EXIT_FAILURE);
  }
  png_structp png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!png_ptr || !info_ptr) {
    fprintf(stderr, "Failded to initialize libpng\n");
    perror(strerror(errno));
    fclose(fd);
    exit(EXIT_FAILURE);
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    fprintf(stderr, "Error during PNG processing, \n");
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fd);
    exit(EXIT_FAILURE);
  }
  png_init_io(png_ptr, fd);
  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);
  png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
  png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
  int color_type = png_get_color_type(png_ptr, info_ptr);
  int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  printf("Image: %ux%u, Color Type: %d, Bit Depth: %d\n", width, height,
         color_type, bit_depth);
  ret->height = height;
  ret->width = width;
  ret->color_type = color_type;
  ret->bit_depth = bit_depth;
  ret->infop = info_ptr;
  ret->structp = png_ptr;
  // fclose(fd);
  return ret;
}
void write_png_file(char *path, png_structp png, png_infop info,
                    png_bytep *row_pointers) {
  FILE *fp = fopen(path, "wb");
  if (!fp) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  png_init_io(png, fp);
  png_write_info(png, info);
  png_write_image(png, row_pointers);
  png_write_end(png, NULL);
  fclose(fp);
}

int main() {
  char *string = "Graceless tarnished what is thy business with these thrones";
  char *key = "test";
  char *path = "./gates_of_divinity.png";
  char *path2 = "./gates_of_divinity2.png";
  Conf *config = init(path);
  png_bytep *row_pointers =
      (png_bytep *)malloc(sizeof(png_bytep) * config->height);
  if (!row_pointers) {
    perror("Failed to allocate memory for row_pointers");
    exit(EXIT_FAILURE);
  }
  for (png_uint_32 y = 0; y < config->height; y++) {
    row_pointers[y] = (png_byte *)malloc(
        png_get_rowbytes((config->structp), (config->infop)));
    if (!row_pointers[y]) {
      perror("Failed to allocate memory for a row");
      exit(EXIT_FAILURE);
    }
  }

  printf("hi\n");
  fflush(stdout);
  png_read_image(config->structp, row_pointers);
  printf("hi\n");
  fflush(stdout);
  int len = strlen(string);
  int total_pixels = config->height * config->width;
  Lcg *s = init_seed(key, total_pixels);
  int *seq = fy_shuffle(s);
  char current_bit;
  for (char *l = string; *l; l++) {
    printf("%c ", *l);
    for (char k = 1; k <= 8; k++, seq++) {
      int i = *seq / config->width;
      int j = *seq - config->width * i;
      // printf("i=%d,j=%d,seq=%d,char=%c,width=%d\n", i, j, *seq, k,
      //        config->width);
      png_bytep px = &row_pointers[i][j];
      int is_odd =
          px[2] % 2; // checking whether the value of the blue channel is either
                     // odd or even, cause if its odd its LSB is 1 else 0
      current_bit = l && (1 << k);
      if (current_bit != 0) {
        // means that current bit is 1
        if (!is_odd) {
          px[2] > 0 ? px[2]-- : px[2]++;
          // no reason to check for when its odd cause its lsb value will
          // already be 1
        } else { // if current bit is 0
          if (is_odd) {
            px[2] > 0 ? px[2]-- : px[2]++;
          }
        }
      }
    }
  }
  FILE *out_fp = fopen(path2, "wb");
  if (!out_fp) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  printf("hi\n");
  fflush(stdout);
  png_structp write_png =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop write_info = png_create_info_struct(write_png);
  if (setjmp(png_jmpbuf(write_png)))
    exit(EXIT_FAILURE);

  printf("hi\n");
  fflush(stdout);
  png_set_IHDR(write_png, write_info, config->width, config->height,
               config->bit_depth, config->color_type, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  printf("hi\n");
  fflush(stdout);
  write_png_file(path2, write_png, write_info, row_pointers);
}
