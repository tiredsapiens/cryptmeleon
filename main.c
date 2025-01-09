// #include "cryptmeleon.c"
#include "rsg.h"
#include <stdlib.h>
#include <string.h>
char *BASE_DIR;
char *BASE_IMAGE_DIR;
char *BASE_IMAGE_NAME = "Base-Image";
// #include <argp.h>
static struct argp_option options[] = {
    {"image-path", 'i', "FILE", 0, "Path to the input image file"},
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {"setup", 's', 0, 0, "Set up"},
    {"decode", 'd', 0, 0, "Decode data"},
    {0}};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  // Get the input arguments structure from the state
  Arguments *arguments = state->input;

  switch (key) {
  case 'i':
    arguments->image_path = arg; // Set image path
    break;
  case 'd':
    arguments->decode = 1;
  case 'v':
    arguments->verbose = 1; // Enable verbose output
    break;
  case 's':
    arguments->setup = 1;
  case ARGP_KEY_ARG:
    // Handle positional arguments if needed
    break;
  case ARGP_KEY_END:
    // Final validation (if necessary)
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}
void get_password(char *password, size_t size) {
  struct termios oldt, newt;

  // Turn off terminal echo
  tcgetattr(STDIN_FILENO, &oldt);          // Get current terminal settings
  newt = oldt;                             // Make a copy
  newt.c_lflag &= ~ECHO;                   // Disable echo
  tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply new settings

  // Prompt for password
  printf("Enter password: ");
  fgets(password, size, stdin);

  // Turn echo back on
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore original settings
  printf("\n");                            // Add a newline for cleanliness
}
int setup(Arguments *args) {
  struct stat info;
  char *home = getenv("HOME");
  if (!home) {
    printf("Couldn't find HOME env var\n");
    return -1;
  }
  char *dir = "/.cryptmeleon/";
  char *cdir = (char *)malloc(strlen(home) + strlen(dir) + 1);

  strcpy(cdir, home);
  strcat(cdir, dir);
  if (stat(cdir, &info) == 0 && S_ISDIR(info.st_mode)) {
    printf("Dir already exists");
    return -1;
  }
  if (mkdir(cdir, 0755) == -1) { // Permissions: rwxr-xr-x
    if (errno != EEXIST) {
      perror("Error creating directory");
      exit(EXIT_FAILURE);
    }
  }
  printf("%s,%s\n", args->image_path, cdir);
  fflush(stdout);
  copy_file(args->image_path, cdir, "Base-Image");
  return 0;
}
static char doc[] = "A tool to encode passwords into images";
static char args_doc[] = ""; // No positional arguments
static struct argp argp = {options, parse_opt, args_doc, doc};

void copy_file(char *source, char *dest, char *name) {
  // char *filename = malloc(256 * sizeof(char));
  int path_len = strlen(source);
  int j = 0;
  int i = path_len - 1;
  for (; i >= 0 && source[i] != '/'; i--)
    ;
  char *filename = malloc((path_len - i) * sizeof(char));
  strcpy(filename, &source[i + 1]);
  char *new_dest = NULL;
  new_dest = malloc(strlen(dest) + strlen(filename) + 1);
  strcpy(new_dest, dest);
  strcat(new_dest, name);
  int src = open(source, O_RDONLY);
  int destination = open(new_dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  char buf[256];
  while (read(src, buf, 256)) {
    write(destination, buf, 256);
  }
}
int is_directory(const char *path) {
  struct stat path_stat;

  // Get file status
  if (stat(path, &path_stat) != 0) {
    perror("stat failed");
    return -1; // Could not retrieve information
  }
  // Check if it's a directory
  return S_ISDIR(path_stat.st_mode);
}

int create_directories(const char *path) {
  char temp_path[1024];
  size_t len = strlen(path);

  // Start with an empty string
  strncpy(temp_path, path, sizeof(temp_path));
  temp_path[sizeof(temp_path) - 1] = '\0';

  for (size_t i = 1; i <= len; i++) {
    // Check for directory separators ('/' or '\\')
    if (temp_path[i] == '/' || temp_path[i] == '\0') {
      char saved_char = temp_path[i];
      temp_path[i] = '\0'; // Temporarily terminate the string

      // Try to create the directory
      if (mkdir(temp_path, 0755) != 0) {
        if (errno != EEXIST) { // Ignore if the directory already exists
          perror("mkdir failed");
          return -1;
        }
      }

      temp_path[i] = saved_char; // Restore the original character
    }
  }

  return 0;
}
void strip_from_str(char *str, char c) {
  int l = strlen(str);
  while (str[0] == c) {
    str++;
    // if(str[0]==0)
    //   break;
  }
  while (str[l - 1] == c) {
    str[l - 1] = 0;
    l = strlen(str);
    printf("%d\n", l);
  }
}

void encode(Arguments *args, char *data) {
  char *password = (char *)malloc(256);
  get_password(password, 256);
  char *image_path =
      (char *)malloc(strlen(args->image_path) + strlen(BASE_DIR));
  char *dir_path = (char *)malloc(strlen(args->image_path) + strlen(BASE_DIR));
  Conf *config = init(BASE_IMAGE_DIR);
  Lcg *s = init_seed(password, config->width * config->height);
  int *seq = fy_shuffle(s);
  encode_bytes(config, seq, data);
  int i = strlen(args->image_path) - 1;
  for (; i >= 0 && args->image_path[i] != '/'; i--)
    ;
  printf("args->image_path=%s\n", args->image_path);
  printf("i=%d\n", i);
  printf("image_path %s\n", image_path);
  strcpy(image_path, BASE_DIR);
  strcpy(dir_path, BASE_DIR);
  strcat(image_path, args->image_path);
  strncat(dir_path, args->image_path, i);
  printf("dir_path %s \nimage_path %s\n", dir_path, image_path);
  int is_dir = is_directory(dir_path);
  if (is_dir == -1) {
    create_directories(dir_path);
  } else if (is_dir == 0) {
    printf("%s is not a directory\n", dir_path);
    exit(-1);
  }
  perror(strerror(errno));
  write_png_file(image_path, config->write_pngp, config->write_infop,
                 config->row_pointers);
}

void decode(Arguments *args) {
  char *password = (char *)malloc(256);
  char *dir = (char *)malloc(strlen(args->image_path) + strlen(BASE_DIR) + 1);
  get_password(password, 256);
  strcpy(dir, BASE_DIR);
  strcat(dir, args->image_path);
  printf("path is %s\n", dir);

  Conf *config = init(dir);
  Lcg *s = init_seed(password, config->width * config->height);
  int *seq = fy_shuffle(s);
  char *message = decode_bytes(config, seq);
  printf("%s\n", message);
  free(s);
  free(seq);
  free_config(config);
}

void fill_base_dirs() {
  char *home = getenv("HOME");
  if (!home) {
    printf("Couldn't find HOME env var\n");
    exit(-1);
  }
  char *dir = "/.cryptmeleon/";
  BASE_DIR = (char *)malloc(strlen(home) + strlen(dir) + 1);
  strcpy(BASE_DIR, home);
  strcat(BASE_DIR, dir);
  BASE_IMAGE_DIR =
      (char *)malloc(strlen(BASE_DIR) + strlen(BASE_IMAGE_NAME) + 1);
  strcpy(BASE_IMAGE_DIR, BASE_DIR);
  strcat(BASE_IMAGE_DIR, BASE_IMAGE_NAME);
}

int main(int argc, char **argv) {
  // for (int i = 0; i < argc; i++) {
  //   printf("%s\n", argv[i]);
  // }
  Arguments arguments;
  fill_base_dirs();
  char *home = getenv("HOME");
  // char *dir = "/.cryptmeleon";
  // char *cdir = (char *)malloc(strlen(home) + strlen(dir) + 1);
  //  printf("%s", cdir);
  //   Default values

  arguments.image_path = NULL;
  arguments.verbose = 0;
  arguments.setup = 0;
  arguments.decode = 0;
  char *password = (char *)malloc(256);

  // char *source = "./gates_of_divinity.png";
  // char *dest = "/home/endrit/.cryptmeleon/";
  // return 0;
  //  Parse arguments
  argp_parse(&argp, argc, argv, 0, 0, &arguments);
  if (arguments.setup) {
    if (!arguments.image_path) {
      printf("You need to provide path for image to used for encoding\n");
      return -1;
    } else {
      setup(&arguments);
    }
  } else if (arguments.decode) {

    if (!arguments.image_path) {
      printf("You need to provide path for image to used for encoding\n");
      return -1;
    } else {
      strip_from_str(arguments.image_path, '/');
      decode(&arguments);
    }
  } else {
    if (!arguments.image_path) {
      printf("You need to provide path for image to used for encoding\n");
      return -1;
    } else {
      strip_from_str(arguments.image_path, '/');
      printf("%s", argv[argc]);
      encode(&arguments, argv[argc - 1]);
    }
  }

  // get_password(password, 256);
  // printf("%s\n", password);

  return 0;
}
