#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

/**
 * 1 - archive not exist or can't be opened
 * 0 - ok
 * -1 - read error
 * -2 - no requested line
 * */
int read_file(const char *filename, size_t requested_line, char *str) {
    // C opens file with extra "\n" string
    requested_line += 1;

    FILE *ffile = fopen(filename, "r");

    if (ffile == NULL) {
//        printf("Unable to open file %s\n", filename);
        return 1;
    }

    size_t line = 1;
    while (line != requested_line) {
        fgets(str, 200, ffile);

        // Checking for the end of file or read error
        if (str == NULL) {
            if (feof(ffile) != 0) {
                // File has ended, there are no requested_line
                fclose(ffile);
                return -2;
            }

            // Read error
            printf("Error reading from file %s\n", filename);
            fclose(ffile);
            return -1;
        }
        line += 1;

//        printf("%s", str);
    }

    fclose(ffile);
    return 0;
}


/**
 * 1 - archive need to be created
 * 0 - ok
 * -1 - an error has occured
 * */
int check_archive_exists(const char *filename) {
    char str[200];
    int res = read_file(filename, 1, str);
//    printf("%s\n", str);

    switch (res) {
        // res=-1 was processed in read_file
        case 1:;
            return 1;
        case -2:
            printf("The archive name is missing\n");
            return -1;
        case 0:
            // search watermark
            if (!strncmp(str, "Watermark", 9))
                return 0;

            printf("The archive name is missing\n");
            return -1;
        default:
            return -1;
    }
}

int write_archive(int file_count, char *files[], int status_code) {
    FILE *ffile = fopen(files[file_count - 1], "a");

    if (ffile == NULL) {
        printf("Unable to open or create file '%s'.\nTry a different file extension."
               "\n'.smth' or '.txt' for example\n", files[file_count-1]);
        return -1;
    }
    return 0;
}

void tar(int file_count, char *files[]) {
    if (file_count < 3) {
        printf("Not enough arguments");
        return;
    }
    if (strcmp(files[1], "tar") != 0) {
        printf("Unrecognized command %s.\nTry tar ...", files[1]);
        return;
    }

    int read_res = check_archive_exists(files[file_count - 1]);

//    printf("%d\n", read_res);
    if (read_res != -1) {
        // can write our improvised archive
        int write_res = write_archive(file_count, files, read_res);

        if(write_res == -1)
            return;
    }

}

int main(int argc, char *argv[]) {
    tar(argc, argv);
    return 0;
}
