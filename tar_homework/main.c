#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILE_SIZE 100
#define WATERMARK "TAR_BINARY_WATERMARK"
#define GREETING_BYTES 21 // Watermark size + \n

struct file_info {
  int name_length;
  int file_size; // в байтах
  int file_offset; // смещение относительно начала файла

  // положение файл в массиве файлов будет определяться индексом данной
  // структуры в tar.file_infos
};

struct tar {
  const char *header;
  int file_count;
  struct file_info *file_infos;
  char **filenames;
  char **file_contents;
};

/**
 * Makes a string from a number in the base number system
 * @param number        number
 * @param destination   the string in which the number will be written
 * @param base          number system
 *
 * @return -1          file is too big
 * @return  0          ok
 * */
int itoa(int number, char *destination, int base) {
  int count = 0;
  do {
    if (count > MAX_FILE_SIZE) {
      fprintf(stderr, "File is too big!\n");
      return -1;
    }
    int digit = number % base;
    destination[count++] = (digit > 9) ? digit - 10 + 'A' : digit + '0';
  } while ((number /= base) != 0);

  destination[count] = '\0';
  int i;
  for (i = 0; i < count / 2; ++i) {
    char symbol = destination[i];
    destination[i] = destination[count - i - 1];
    destination[count - i - 1] = symbol;
  }

  return 0;
}

void tar_destructor(struct tar *my_tar) {
  for (int i = 0; i < my_tar->file_count; i++) {
    my_tar->file_contents[i] = 0;
    free(my_tar->file_contents[i]);

    my_tar->filenames[i] = 0;
    free(my_tar->filenames[i]);
  }
  free(my_tar->filenames);
  free(my_tar->file_contents);
  free(my_tar->file_infos);
}

int get_file_binary_size(const char *filename) {
  struct stat sb;

  int ret = stat(filename, &sb);
  if (ret) {
    fprintf(stderr, "Error open file info");
    exit(EXIT_FAILURE);
  }

  return sb.st_size;
}

/**
 * Opens file, fills struct file_info and returns it's content
 * @param filename           opening filename
 * @param mode               file opening mode
 * @param file_description   file_info struct
 *
 * @return                  file content
 * */
char *with_open_file_(const char *filename, const char *mode,
                      struct file_info *file_description) {
  FILE *file = fopen(filename, mode);

  if (file == NULL) {
    fprintf(stderr, "Error opening file");
    exit(EXIT_FAILURE);
  }

  int size = get_file_binary_size(filename);
  file_description->name_length = strlen(filename);
  file_description->file_size = size;

  char *content = (char *)malloc(size * sizeof(char));
  if (content == NULL) {
    fprintf(stderr, "Error allocate memory!\n");
    exit(EXIT_FAILURE);
  }

  fread(content, sizeof(char), size, file);

  fclose(file);
  return content;
}

/**
 * Fills my_tar structure
 *
 * @param count     argc argument
 * @param argv      argv argument
 * @param my_tar    tar structure
 * */
void collect_tar_(int count, char *argv[], struct tar *my_tar) {
  int file_count = count - 4;

  my_tar->header = WATERMARK;
  my_tar->file_count = file_count;

  my_tar->filenames = (char **)malloc(file_count * sizeof(char *));
  my_tar->file_contents = (char **)malloc(file_count * sizeof(char *));
  my_tar->file_infos =
      (struct file_info *)malloc(file_count * sizeof(struct file_info));

  if (my_tar->filenames == NULL || my_tar->file_contents == NULL ||
      my_tar->file_infos == NULL) {
    fprintf(stderr, "Error allocate memory!\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 3, j = 0; i < count - 1; i++, j++) {
    my_tar->filenames[j] = (char *)malloc(strlen(argv[i]) * sizeof(char));

    if (my_tar->filenames[j] == NULL) {
      fprintf(stderr, "Error allocate memory!\n");
      exit(EXIT_FAILURE);
    }

    my_tar->filenames[j] = argv[i];
    my_tar->file_contents[j] =
        with_open_file_(argv[i], "rb", &my_tar->file_infos[j]);
  }
}

void write_tar(int argc, char *argv[]) {
  struct tar my_tar;
  int file_offset = GREETING_BYTES;

  collect_tar_(argc, argv, &my_tar);

  FILE *file = fopen(argv[argc - 1], "wb");
  if (file == NULL) {
    printf("Error opening file");
    exit(EXIT_FAILURE);
  }

  // Header
  fwrite(my_tar.header, sizeof(char), strlen(my_tar.header), file); // Watermark
  fwrite("\n", sizeof(char), 1, file);

  /* Here will be filenames and their offsets,
   * after filling other parts*/
  for (int i = 0; i < 48; i++)
    fwrite(" a", sizeof(char), 2, file);
  fwrite("\n", sizeof(char), 1, file);
  file_offset += 97;

  // Files content
  for (int i = 0; i < my_tar.file_count; i++) {
    int size = my_tar.file_infos[i].file_size;
    char size_str[MAX_FILE_SIZE];

    if (itoa(size, size_str, 10) == -1) {
      tar_destructor(&my_tar);
      exit(EXIT_FAILURE);
    }

    int size_str_count = strlen(size_str);
    file_offset += size_str_count + 1;

    /* File sizes line that will
     * divide files between themselves*/
    fwrite(size_str, sizeof(char), size_str_count, file);
    fwrite("\n", sizeof(char), 1, file);

    fwrite(my_tar.file_contents[i], sizeof(char), size, file);

    my_tar.file_infos[i].file_offset = file_offset;
    file_offset += size;
  }

  // Filenames and their offsets
  fseek(file, GREETING_BYTES, SEEK_SET);

  for (int i = 0; i < my_tar.file_count; i++) {

    fwrite(my_tar.filenames[i], sizeof(char), strlen(my_tar.filenames[i]),
           file);
    fwrite(" ", sizeof(char), 1, file);

    char file_size_str[MAX_FILE_SIZE];
    int file_size = my_tar.file_infos[i].file_offset;

    if (itoa(file_size, file_size_str, 10) == -1) {
      tar_destructor(&my_tar);
      exit(EXIT_FAILURE);
    }

    fwrite(file_size_str, sizeof(char), strlen(file_size_str), file);

    if (i + 1 != my_tar.file_count)
      fwrite(" ", sizeof(char), 1, file);
  }

  fclose(file);
  tar_destructor(&my_tar);
}

/**
 * Displays filenames written in tar
 * @param filename - tar filename
 * */
void files_in_tar(char *filename) {
  FILE *file = fopen(filename, "rb");

  if (file == NULL) {
    printf("Error opening file");
    exit(EXIT_FAILURE);
  }

  fseek(file, GREETING_BYTES, SEEK_SET);

  char str[MAX_FILE_SIZE];
  fread(&str, sizeof(char), 100, file);

  // Sifting out file offsets
  char *istr;
  istr = strtok(str, " ");
  while (istr != NULL) {
    if (!strcmp(istr, "a"))
      break;

    for (int i = 0; i < strlen(istr); i++)
      if (!isdigit(istr[i])) {
        printf("%s\n", istr);
        break;
      }

    istr = strtok(NULL, " ");
  }

  fclose(file);
}

void update_tar(char *file_to_add, char *upd_tar) {
  struct file_info struct_file_info;
  int file_offset = get_file_binary_size(upd_tar);

  char *content = with_open_file_(file_to_add, "rb", &struct_file_info);

  FILE *file = fopen(upd_tar, "r+b");

  if (file == NULL) {
    fprintf(stderr, "Error opening file");
    exit(EXIT_FAILURE);
  }

  fseek(file, file_offset, SEEK_SET);

  char file_size_str[MAX_FILE_SIZE];
  if (itoa(file_offset, file_size_str, 10) == -1)
    exit(EXIT_FAILURE);

  fwrite(file_size_str, sizeof(char), strlen(file_size_str), file);
  fwrite("\n", sizeof(char), 1, file);
  fwrite(content, sizeof(char), strlen(content), file);

  fclose(file);

  // Header update
  // Filenames and their offsets
  int fd;
  if ((fd = open(upd_tar, O_RDWR)) == -1) {
    printf("Cannot open file.\n");
    exit(1);
  }
  lseek(fd, GREETING_BYTES, SEEK_SET);
  char buf[MAX_FILE_SIZE];

  if (read(fd, buf, MAX_FILE_SIZE) != MAX_FILE_SIZE) {
    printf("Cannot read file.\n");
    exit(EXIT_FAILURE);
  }

  lseek(fd, GREETING_BYTES, SEEK_SET);
  char *istr;
  int offset = GREETING_BYTES;
  istr = strtok(buf, " ");

  while (istr != NULL) {
    if (!strcmp(istr, "a"))
      break;

    offset += strlen(istr) + 1;
    istr = strtok(NULL, " ");
  }

  lseek(fd, offset, SEEK_SET);
  write(fd, file_to_add, strlen(file_to_add));
  write(fd, " ", 1);
  write(fd, file_size_str, strlen(file_size_str));
  write(fd, " ", 1);


  close(fd);
}

int main(int argc, char *argv[]) {

  if (argc < 4) {
    printf("Not enough arguments\n");
    exit(EXIT_FAILURE);
  }

  if (!strcmp(argv[2], "-h")) {
    printf("-h tar man\n"
           "-c create tar\n"
           "-u add file to existing tar\n"
           "-l files in tar\n");
  }

  if (!strcmp(argv[2], "-c")) {

    if (argc < 5) {
      printf("Not enough arguments for writing tar\n");
      exit(EXIT_FAILURE);
    }

    write_tar(argc, argv);
  }

  if (!strcmp(argv[2], "-l"))
    files_in_tar(argv[argc - 1]);

  if (!strcmp(argv[2], "-u")) {

    if (argc != 5) {
      printf("Incorrect count of arguments");
      exit(EXIT_FAILURE);
    }

    update_tar(argv[3], argv[4]);
  }

  return 0;
}
