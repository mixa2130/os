#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define ERROR_OPEN_FILE -3
#define ERROR_GET_FILE_INFO -4

#define GREETING_BYTES 12//Watermark \
                            //          \
                            //
// обработчик malloc: if null - освободить
//if (numbers == NULL) {  // если память не дали
//        fprintf(stderr, "Error allocate memory.\n");    // сообщаем об ошибке
//        return 2;           // выходим с кодом ошибки
//    }
struct file_info {
    size_t name_length;
    int file_size;// в байтах
    //    int file_offset;// смещение относительно начала файла
    // положение файл в массиве файлов будет определяться индексом данной структуры в tat.file_infos
};

struct tar {
    const char *header;
    int file_count;
    struct file_info *file_infos;
    char **filenames;
    char **file_contents;
};

int get_file_binary_size(const char *filename) {
    struct stat sb;

    int ret = stat(filename, &sb);
    if (ret) {
        printf("Error opening file");
        exit(ERROR_GET_FILE_INFO);
    }

    return sb.st_size;
}

char *with_open_file(const char *filename,
                     const char *mode,
                     struct file_info *file_description) {
    FILE *file = fopen(filename, mode);

    if (file == NULL) {
        printf("Error opening file");
        exit(ERROR_OPEN_FILE);
    }

    int size = get_file_binary_size(filename);
    file_description->name_length = strlen(filename);
    file_description->file_size = size;

    char *content = (char *) malloc(size);
    fread(content, sizeof(char), size, file);

    fclose(file);
    return content;
}

void collect_tar_(int count, char *argv[], struct tar *my_tar) {
    int file_count = count - 4;

    my_tar->header = "Watermark";
    my_tar->file_count = file_count;

    my_tar->filenames = (char **) malloc(file_count);
    my_tar->file_contents = (char **) malloc(file_count);
    my_tar->file_infos = (struct file_info *) malloc(file_count * sizeof(struct file_info));

    for (int i = 3, j = 0; i < count - 1; i++, j++) {
        my_tar->filenames[j] = (char *) malloc(strlen(argv[i]));
        my_tar->filenames[j] = argv[i];

        my_tar->file_contents[j] = with_open_file(argv[i], "rb", &my_tar->file_infos[j]);
    }
}

int write_tar(int argc, char *argv[]){
    struct tar my_tar;
    collect_tar_(argc, argv, &my_tar);


}

int main(int argc, char *argv[]) {
    char *file;
    if (argc < 5) {
        printf("Not enough arguments\n");
        return -1;
    }

    if (!strcmp(argv[2], "-c")) {
        int smth = write_tar(argc, argv);
        //        printf("%d", smth);
    }
    return 0;
}


//void main() {
//    FILE *iofile = NULL;
//    unsigned counter;
//    int i, num;
//
//    iofile = fopen("numbers.bin", "rb");
//    if (iofile == NULL) {
//        printf("Error opening file");
//        exit(ERROR_OPEN_FILE);
//    }
//
//    fread(&counter, sizeof(int), 1, iofile);
//    printf("%u\n",counter);
//    for (i = 0; i < counter; i++) {
//        fread(&num, sizeof(int), 1, iofile);
//        printf("%d\n", num);
//    }
//
//    fclose(iofile);
//}