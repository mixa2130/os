#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <time.h>

int count_digits(int num){
    int digits = 0;
    while (num > 0){
        digits += 1;
        num /= 10;
    }

    return digits;
}

void get_file_modification_date(time_t ttime) {
    struct tm *ptm = localtime(&ttime);

    // month
    switch (ptm->tm_mon) {
        case 0:
            printf("Jan ");
            break;
        case 1:
            printf("Feb ");
            break;
        case 2:
            printf("Mar ");
            break;
        case 3:
            printf("Apr ");
            break;
        case 4:
            printf("May ");
            break;
        case 5:
            printf("Jun ");
            break;
        case 6:
            printf("Jul ");
            break;
        case 7:
            printf("Aug ");
            break;
        case 8:
            printf("Sep ");
            break;
        case 9:
            printf("Oct ");
            break;
        case 10:
            printf("Nov ");
            break;
        default:
            printf("Dec ");
            break;
    }

    // day
    int day = ptm->tm_mday;
    printf(day < 10 ? " %d " : "%d ", day);

    // time
    int hours = ptm->tm_hour;
    int minutes = ptm->tm_min;

    if (hours < 10 && minutes < 10)
        printf("0%d:0%d ", hours, minutes);
    if (hours < 10 && minutes >= 10)
        printf("0%d:%d ", hours, minutes);
    if (hours >= 10 && minutes < 10)
        printf("%d:0%d ", hours, minutes);
    if (hours >= 10 && minutes >= 10)
        printf("%d:%d ", hours, minutes);
}

void get_file_mods(unsigned long file_mods) {
    switch (file_mods & S_IFMT) {
        case S_IFBLK:
            printf("b");
            break;
        case S_IFCHR:
            printf("c");
            break;
        case S_IFLNK:
            printf("l");
            break;
        case S_IFIFO:
            printf("p");
            break;
        case S_IFSOCK:
            printf("s");
            break;
        case S_IFDIR:
            printf("d");
            break;
        default:
            printf("-");
            break;
    }

    printf(file_mods & S_IRUSR ? "r" : "-");
    printf(file_mods & S_IWUSR ? "w" : "-");
    printf(file_mods & S_IXUSR ? "x" : "-");

    printf(file_mods & S_IRGRP ? "r" : "-");
    printf(file_mods & S_IWGRP ? "w" : "-");
    printf(file_mods & S_IXGRP ? "x" : "-");

    printf(file_mods & S_IROTH ? "r" : "-");
    printf(file_mods & S_IWOTH ? "w" : "-");
    printf(file_mods & S_IXOTH ? "x" : "-");
}

void get_file_info(char *filename, int argc, int max_digits) {
    struct stat sb;
    int ret;

    if (argc < 2) {
        fprintf(stderr, "usage: %s\n", filename);
        return;
    }

    ret = stat(filename, &sb);
    if (ret) {
        perror("stat");
        return;
    }

    // file mods
    get_file_mods(sb.st_mode);

    // links
    printf(" %lu ", sb.st_nlink);

    struct passwd *pwd;

    // file owner
    pwd = getpwuid(sb.st_uid);
    if (pwd == NULL) {
        perror("getpwuid");
        return;
    } else
        printf("%s ", pwd->pw_name);

    // group file owner
    pwd = getpwuid(sb.st_gid);
    if (pwd == NULL) {
        perror("getpwuid");
        return;
    } else
        printf("%s ", pwd->pw_name);

    // file size
    int file_size_digits = count_digits(sb.st_size);
    for (int i=0; i < max_digits - file_size_digits; i++)
        printf(" ");

    printf("%lu ", sb.st_size);

    // file modification date
    get_file_modification_date(sb.st_mtime);
}

int count_memory_blocks(char *dirname, int *max_size) {
    register struct dirent *dir_buf;
    DIR *fdir;
    int res = 0;

    struct stat sb;
    if ((fdir = opendir(dirname)) == NULL) {
        fprintf(stderr, "Can't read %s\n", dirname);
        return -1;
    }

    while ((dir_buf = readdir(fdir)) != NULL) {
        char file_path[strlen(dirname) + strlen(dir_buf->d_name) + 10];
        sprintf(file_path, "%s/%s", dirname, dir_buf->d_name);
        stat(file_path, &sb);

        if (*max_size < sb.st_size)
            *max_size = sb.st_size;

        res += sb.st_blocks;
    }
    closedir(fdir);

    *max_size = count_digits(*max_size);
    return res / 2;
}

void listdir(char *dirname) {
    int max_file_size_digits = 0; // to understand how much spaces we need in an output
    int memory_blocks = count_memory_blocks(dirname, &max_file_size_digits);

    if (memory_blocks == -1)
        return;

    printf("total: %d\n", memory_blocks);
    register struct dirent *dir_buf;
    DIR *fdir = opendir(dirname);

    while ((dir_buf = readdir(fdir)) != NULL) {
        if (dir_buf->d_ino == 0) continue;
        else {
            // getting absolute file path
            char file_path[strlen(dirname) + strlen(dir_buf->d_name) + 10];
            sprintf(file_path, "%s/%s", dirname, dir_buf->d_name);

            get_file_info(file_path, 2, max_file_size_digits);

            printf("%s\n", dir_buf->d_name);
        }
    }
    closedir(fdir);
}

int main(int argc, char *argv[]) {
    int i;

    if (argc > 1) for (i = 1; i < argc; i++) listdir(argv[i]);
    else listdir(".");

    return 0;
}
