#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define ERROR_ALLOCATE_MEMORY -2
#define ERROR_OPEN_FILE -3
#define ERROR_GET_FILE_INFO -4
#define MAX_FILE_SIZE 100
#define MAX_FILE_SIZE_STR_COUNT 3

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

int count_filenames_header_str_size(struct tar *my_tar) {

  int size = GREETING_BYTES;
  for (int i = 0; i < my_tar->file_count; i++) {
    size += my_tar->file_infos[i].name_length + 1;
  }
  return size;
}

int get_file_binary_size(const char *filename) {
  struct stat sb;

  int ret = stat(filename, &sb);
  if (ret) {
    fprintf(stderr, "Error open file info");
    exit(ERROR_GET_FILE_INFO);
  }

  return sb.st_size;
}

char *with_open_file(const char *filename, const char *mode,
                     struct file_info *file_description) {
  FILE *file = fopen(filename, mode);

  if (file == NULL) {
    fprintf(stderr, "Error opening file");
    exit(ERROR_OPEN_FILE);
  }

  int size = get_file_binary_size(filename);
  file_description->name_length = strlen(filename);
  file_description->file_size = size;

  char *content = (char *)malloc(size * sizeof(char));
  if (content == NULL) {
    fprintf(stderr, "Error allocate memory!\n");
    exit(ERROR_ALLOCATE_MEMORY);
  }

  fread(content, sizeof(char), size, file);

  fclose(file);
  return content;
}

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
    exit(ERROR_ALLOCATE_MEMORY);
  }

  for (int i = 3, j = 0; i < count - 1; i++, j++) {
    my_tar->filenames[j] = (char *)malloc(strlen(argv[i]) * sizeof(char));
    if (my_tar->filenames[j] == NULL) {
      fprintf(stderr, "Error allocate memory!\n");
      exit(ERROR_ALLOCATE_MEMORY);
    }

    my_tar->filenames[j] = argv[i];
    my_tar->file_contents[j] =
        with_open_file(argv[i], "rb", &my_tar->file_infos[j]);
  }
}

void itoa(int number, char *destination, int base) {
  int count = 0;
  do {
    if (count > MAX_FILE_SIZE) {
      fprintf(stderr, "File is too big!\n");
      exit(EXIT_FAILURE);
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

int write_tar(int argc, char *argv[]) {
  struct tar my_tar;
  int file_offset = GREETING_BYTES;

  collect_tar_(argc, argv, &my_tar);

  FILE *file = fopen(argv[argc - 1], "wb");

  if (file == NULL) {
    printf("Error opening file");
    exit(ERROR_OPEN_FILE);
  }

  // header
  fwrite(my_tar.header, sizeof(char), strlen(my_tar.header), file); // Watermark
  fwrite("\n", sizeof(char), 1, file);

  for (int i = 0; i < 99; i++)
    fwrite("", sizeof(char), 1, file);
  fwrite("\n", sizeof(char), 1, file);
  file_offset += 100;
  // files content
  for (int i = 0; i < my_tar.file_count; i++) {

    //    if(i!=0){
    //      fwrite(my_tar.header, sizeof(char), strlen(my_tar.header), file); //
    //      Watermark fwrite("\n", sizeof(char), 1, file);
    //    }

    //    fwrite(my_tar.filenames[i], sizeof(char), strlen(my_tar.filenames[i]),
    //           file);
    //    fwrite(" ", sizeof(char), 1, file);

    int size = my_tar.file_infos[i].file_size;
    char size_str[MAX_FILE_SIZE];
    itoa(size, size_str, 10);

    int size_str_count = strlen(size_str);
    file_offset += size_str_count + 1;

    fwrite(size_str, sizeof(char), size_str_count, file);
    fwrite("\n", sizeof(char), 1, file);
    fwrite(my_tar.file_contents[i], sizeof(char), size, file);

    my_tar.file_infos[i].file_offset = file_offset;
    file_offset += size;
  }

  fseek(file, GREETING_BYTES, SEEK_SET);
  for (int i = 0; i < my_tar.file_count; i++) {
    fwrite(my_tar.filenames[i], sizeof(char), strlen(my_tar.filenames[i]),
           file);
    fwrite(" ", sizeof(char), 1, file);

    char file_size_str[MAX_FILE_SIZE];
    int file_size = my_tar.file_infos[i].file_offset;
    itoa(file_size, file_size_str, 10);

    fwrite(file_size_str, sizeof(char), strlen(file_size_str), file);

    if (i + 1 != my_tar.file_count)
      fwrite(" ", sizeof(char), 1, file);
  }

  fclose(file);

  tar_destructor(&my_tar);

  //    FILE *ffile = fopen(argv[argc - 1], "rb");
  //
  //    if (ffile == NULL) {
  //      printf("Error opening file");
  //      exit(ERROR_OPEN_FILE);
  //    }
  //    printf("%d",fgetc(file));
  //    printf("%d",fgetc(file));

  return 0;
}

void files_in_tar(char *filename) {
  FILE *file = fopen(filename, "rb");
  fseek(file, GREETING_BYTES, SEEK_SET);
  char str[MAX_FILE_SIZE];
  fread(&str, sizeof(char), 100, file);

  char *istr;
  istr = strtok(str, " ");
  while (istr != NULL) {
    for (int i = 0; i < strlen(istr); i++)
      if(!isdigit(istr[i])) {
        printf("%s\n", istr);
        break;
      }

    istr = strtok(NULL, " ");
  }
}

int main(int argc, char *argv[]) {
  printf("-c create tar\n"
         "-l - files in tar\n");
  //  if (argc < 5) {
  //    printf("Not enough arguments\n");
  //    return -1;
  //  }

  if (!strcmp(argv[2], "-c")) {
    int smth = write_tar(argc, argv);
  }

  if (!strcmp(argv[2], "-l")) {
    files_in_tar(argv[argc - 1]);
  }

  return 0;
}
