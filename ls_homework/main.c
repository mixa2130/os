#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <time.h>

void get_file_mod_date(time_t ttime) {
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
    if (hours < 10 && minutes > 10)
        printf("0%d:%d ", hours, minutes);
    if (hours > 10 && minutes < 10)
        printf("%d:0%d ", hours, minutes);
    if (hours > 10 && minutes > 10)
        printf("%d:%d ", hours, minutes);
}

void get_file_info(char *filename, int argc) {
    struct stat sb;
    int ret;

    if (argc < 2) {
        fprintf(stderr, "usage: %s\n", filename);
        return;
    }

    stat(filename, &sb);
//    if (ret) {
//        perror("stat");
//        return;
//    }

    // file mods
    unsigned long file_mods = sb.st_mode;
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
    printf("%lu ", sb.st_size);

    // file modification date
    get_file_mod_date(sb.st_mtime);


//    printf("%s\n", filename);
}

int count_memory_blocks(char *dirname) {
    register struct dirent *dir_buf;
    DIR *fdir = opendir(dirname);
    int res = 0;

//    if ((fdir = opendir(dirname)) == NULL) {
//        fprintf(stderr, "Can't read %s\n", dirname);
//        return -1;
//    }

    while ((dir_buf = readdir(fdir)) != NULL)
        res += dir_buf->d_type;

    closedir(fdir);
    return res;
}

int listdir(char *dirname) {
//    printf("total %d\n", count_memory_blocks(dirname));
    register struct dirent *dir_buf;
    DIR *fdir = opendir(dirname);

    while ((dir_buf = readdir(fdir)) != NULL) {
        if (dir_buf->d_ino == 0) continue;
        else {
            // getting absolute file path
            char file_path[512];
            sprintf(file_path, "%s/%s", dirname, dir_buf->d_name);

            get_file_info(file_path, 2);

            printf("%s\n", dir_buf->d_name);
        }
    }
    closedir(fdir);

    return 0;
}

int main(int argc, char *argv[]) {
    int i;

    if (argc > 1) for (i = 1; i < argc; i++) listdir(argv[i]);
    else listdir(".");

    return 0;
}
