#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>

static const char *it24_host_path = "/it24_host";
static const char *log_file_path = "/var/log/it24.log";

static char *rot13(const char *str) {
    char *result = strdup(str);
    if (!result) return NULL;
    for (int i = 0; result[i] != '\0'; i++) {
        char c = result[i];
        if (('a' <= c && c <= 'm') || ('A' <= c && c <= 'M')) {
            result[i] += 13;
        } else if (('n' <= c && c <= 'z') || ('N' <= c && c <= 'Z')) {
            result[i] -= 13;
        }
    }
    return result;
}

static void log_warning(const char *filename) {
    FILE *log_file = fopen(log_file_path, "a");
    if (log_file) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
        fprintf(log_file, "[%s] WARNING: Dangerous file detected: %s\n", timestamp, filename);
        fclose(log_file);
    } else {
        fprintf(stderr, "ERROR: Could not open log file %s\n", log_file_path);
    }
}

static int antink_getattr(const char *path, struct stat *stbuf) {
    int res;
    char full_path[PATH_MAX];
    sprintf(full_path, "%s%s", it24_host_path, path);

    res = lstat(full_path, stbuf);
    if (res == -1)
        return -errno;

    return 0;
}

static int antink_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;

    char full_path[PATH_MAX];
    sprintf(full_path, "%s%s", it24_host_path, path);

    dp = opendir(full_path);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        char *displayed_name = strdup(de->d_name);
        if (strstr(de->d_name, "nafis") || strstr(de->d_name, "kimcun")) {
            char *reversed_name = strdup(de->d_name);
            int len = strlen(reversed_name);
            for (int i = 0; i < len / 2; i++) {
                char temp = reversed_name[i];
                reversed_name[i] = reversed_name[len - 1 - i];
                reversed_name[len - 1 - i] = temp;
            }
            free(displayed_name);
            displayed_name = reversed_name;
            log_warning(de->d_name);
        }

        if (filler(buf, displayed_name, &st, 0))
            break;
        free(displayed_name);
    }
    closedir(dp);
    return 0;
}

static int antink_open(const char *path, struct fuse_file_info *fi) {
    char full_path[PATH_MAX];
    sprintf(full_path, "%s%s", it24_host_path, path);

    int res = open(full_path, fi->flags);
    if (res == -1)
        return -errno;

    fi->fh = res;
    return 0;
}

static int antink_read(const char *path, char *buf, size_t size, off_t offset,
                       struct fuse_file_info *fi) {
    int fd = (int)fi->fh;
    int res;

    char *file_content = NULL;
    long file_size;

    lseek(fd, 0, SEEK_SET);
    file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    file_content = malloc(file_size + 1);
    if (!file_content) return -ENOMEM;
    res = read(fd, file_content, file_size);
    file_content[file_size] = '\0';

    char *processed_content = NULL;
    if (strstr(path, "nafis") || strstr(path, "kimcun")) {
        processed_content = file_content;
    } else {
        processed_content = rot13(file_content);
    }

    size_t len = strlen(processed_content);
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, processed_content + offset, size);
    } else {
        size = 0;
    }

    if (processed_content != file_content) {
        free(processed_content);
    }
    free(file_content);

    return size;
}

static int antink_write(const char *path, const char *buf, size_t size,
                        off_t offset, struct fuse_file_info *fi) {
    int fd = (int)fi->fh;
    int res;

    res = pwrite(fd, buf, size, offset);
    if (res == -1)
        return -errno;

    return res;
}

static int antink_release(const char *path, struct fuse_file_info *fi) {
    (void) path;
    close((int)fi->fh);
    return 0;
}

static struct fuse_operations antink_oper = {
    .getattr    = antink_getattr,
    .readdir    = antink_readdir,
    .open       = antink_open,
    .read       = antink_read,
    .write      = antink_write,
    .release    = antink_release,
};

int main(int argc, char *argv[]) {
    umask(0);
    return fuse_main(argc, argv, &antink_oper, NULL);
}
