#include "rsg.h"
#include <png.h>
#include <stdio.h>
#include <string.h>
char VERBOSE = 0;
Conf *init(char *read_path) {

  FILE *fd = fopen(read_path, "rb");
  Conf *ret = (Conf *)malloc(sizeof(Conf));
  unsigned char header[8];
  if (!fd) {
    printf("Error when trying to open file %s\n", read_path);
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
  if (VERBOSE)
    printf("Image: %ux%u, Color Type: %d, Bit Depth: %d\n", width, height,
           color_type, bit_depth);
  ret->height = height;
  ret->width = width;
  ret->color_type = color_type;
  ret->bit_depth = bit_depth;
  ret->read_infop = info_ptr;
  ret->read_pngp = png_ptr;
  // fclose(fd);
  ret->row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * ret->height);
  if (!ret->row_pointers) {
    perror("Failed to allocate memory for row_pointers");
    exit(EXIT_FAILURE);
  }
  for (png_uint_32 y = 0; y < ret->height; y++) {
    ret->row_pointers[y] = (png_byte *)malloc(
        png_get_rowbytes((ret->read_pngp), (ret->read_infop)));
    if (!ret->row_pointers[y]) {
      perror("Failed to allocate memory for a row");
      exit(EXIT_FAILURE);
    }
  }

  png_read_image(ret->read_pngp, ret->row_pointers);
  fclose(fd);

  ret->write_pngp =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  ret->write_infop = png_create_info_struct(ret->write_pngp);
  if (setjmp(png_jmpbuf(ret->write_pngp)))
    exit(EXIT_FAILURE);

  png_set_IHDR(ret->write_pngp, ret->write_infop, ret->width, ret->height,
               ret->bit_depth, ret->color_type, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  return ret;
}
void free_config(Conf *s) {
  png_destroy_read_struct(&s->read_pngp, &s->read_infop, NULL);
  png_destroy_write_struct(&s->write_pngp, &s->write_infop);
  for (int y = 0; y < s->height; y++) {
    free(s->row_pointers[y]);
  }
  free(s->row_pointers);
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

void encode_bytes(Conf *config, int *seq, char *data) {
  int len = strlen(data);
  char current_bit;
  int i, j;
  int z = 0;
  for (int k = 0; k < 32; k++, z++) {
    i = seq[z] / config->width;
    j = seq[z] - config->width * i;

    current_bit = ((len) & (1 << k)) != 0;
    if (VERBOSE)
      printf("current length(%d) bit %d\n", len, current_bit);

    png_bytep px = &config->row_pointers[i][j];
    int last_pixel_bit = (px[2] % 2) != 0;

    if (current_bit == 1) {
      // means that current bit is 1
      if (last_pixel_bit != 1) {
        px[2] > 0 ? px[2]-- : px[2]++;

        // printf("kth bit of *l blue channel after changing : %d\n",
        //        (px[2] % 2 != 0));
        // fflush(stdout);
        //  no reason to check for when its odd cause its lsb value will
        //  already be 1
      }
    } else { // if current bit is 0
      //
      if (last_pixel_bit != 0) {
        px[2] > 0 ? px[2]-- : px[2]++;
      }
    }

    if ((current_bit != 0) != (px[2] % 2) != 0) {
      printf("achtung bits are not the same even after changing \n");
    }
  }
  for (char *l = data; *l; l++) {
    if (VERBOSE)
      printf("next byte\n");
    for (char k = 0; k < 8; k++, z++) {
      i = seq[z] / config->width;
      j = seq[z] - config->width * i;

      // printf("i=%d,j=%d,seq=%d,char=%c,width=%d\n", i, j, *seq, k,
      //        config->width);

      // printf("k:%d,seq:%d,i:%d,j:%d,z:%d\n", k, seq[z], i, j, z);
      png_bytep px = &config->row_pointers[i][j];
      int last_pixel_bit =
          (px[2] % 2) !=
          0; // checking whether the value of the blue channel is either
             // odd or even, cause if its odd its lsb is 1 else 0
      current_bit = ((*l) & (1 << k)) != 0;
      if (VERBOSE)
        printf("lth bit of *l (l=%c,k=%d): %d,while bluechannel bit is %d\n",
               *l, k, (current_bit), (last_pixel_bit));
      if (current_bit == 1) {
        // means that current bit is 1
        if (last_pixel_bit != 1) {
          px[2] > 0 ? px[2]-- : px[2]++;

          if (VERBOSE)
            printf("kth bit of *l blue channel after changing : %d\n",
                   (px[2] % 2 != 0));
          fflush(stdout);
          // no reason to check for when its odd cause its lsb value will
          // already be 1
        }
      } else { // if current bit is 0
        //
        if (last_pixel_bit != 0) {
          px[2] > 0 ? px[2]-- : px[2]++;
          if (VERBOSE)
            printf("kth bit of *l blue channel after changing : %d\n",
                   (px[2] % 2 != 0));
          fflush(stdout);
        }
      }

      if ((current_bit != 0) != (px[2] % 2) != 0) {
        printf("ACHTUNG \n");
      }
    }
  }
}

char *decode_bytes(Conf *config, int *seq) {

  char *decoded_str;
  int z = 0;

  char c = 0;
  int encoded_len = 0;
  for (int k = 0; k < 32; k++, z++) {

    int i = seq[z] / config->width;
    int j = seq[z] - config->width * i;
    png_bytep px = &config->row_pointers[i][j];
    char last_bit = 0;
    int is_odd = px[2] % 2 != 0;
    if (is_odd) {
      last_bit = 1;
    }
    encoded_len = encoded_len | (last_bit << k);
  }
  decoded_str = (char *)malloc(encoded_len + 1);
  for (int l = 0; l < encoded_len; l++) {
    c = 0;
    for (char k = 0; k < 8; k++, z++) {
      int i = seq[z] / config->width;
      int j = seq[z] - config->width * i;
      png_bytep px = &config->row_pointers[i][j];
      char last_bit = 0;
      int is_odd = px[2] % 2 != 0;
      if (is_odd) {
        last_bit = 1;
      }
      c = c | (last_bit << k);
    }
    decoded_str[l] = c;
  }
  decoded_str[encoded_len] = 0;
  printf("%s\n", decoded_str);
  printf("%d\n", encoded_len);
  fflush(stdout);
  return decoded_str;
}
int main(int argc, char **argv) {
  char *string = "super duper secret info";
  char *key = "test234";
  char *path = "./gates_of_divinity.png";
  char *path2 = "./gates_of_divinity2.png";
  Conf *config = init(path);

  // int len = strlen(string);
  int total_pixels = config->height * config->width;
  Lcg *s = init_seed(key, total_pixels);
  int *seq = fy_shuffle(s);
  encode_bytes(config, seq, string);

  write_png_file(path2, config->write_pngp, config->write_infop,
                 config->row_pointers);
  free_config(config);
  config = init(path2);
  // seq = fy_shuffle(s);
  char *decoded_string = decode_bytes(config, seq);
  free_config(config);
}
