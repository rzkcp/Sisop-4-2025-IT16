#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

static const char *RELICS_DIR_NAME = "relics";
static const char *LOG_FILE_NAME = "activity.log";
static const char *BAYMAX_VIRTUAL_FILENAME = "Baymax.jpeg";
static const int FRAGMENT_SIZE = 1024;
static const int BAYMAX_FRAGMENTS_COUNT = 14;

static char relics_base_path[PATH_MAX];

void log_activity(const char *operation, const char *details) {
    FILE *log_file = fopen(LOG_FILE_NAME, "a");
    if (log_file == NULL) {
        perror("Gagal membuka file log");
        return;
    }
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
    fprintf(log_file, "[%s] %s: %s\n", timestamp, operation, details);
    fclose(log_file);
    printf("[LOG] %s: %s\n", operation, details);
}

void get_relics_path(const char *path_suffix, char *full_path) {
    if (path_suffix[0] == '/') {
        strcpy(full_path, path_suffix);
    } else {
        snprintf(full_path, PATH_MAX, "%s/%s", relics_base_path, path_suffix);
    }
}

void get_fragment_filename(const char *base_filename, int index, char *out_fragment_name) {
    snprintf(out_fragment_name, FILENAME_MAX, "%s.%03d", base_filename, index);
}

void get_full_fragment_path(const char *base_filename, int index, char *out_full_path) {
    char fragment_name[FILENAME_MAX];
    get_fragment_filename(base_filename, index, fragment_name);
    get_relics_path(fragment_name, out_full_path);
}

static int bb_getattr(const char *path, struct stat *stbuf,
                      struct fuse_file_info *fi) {
    (void)fi;
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    if (strcmp(path + 1, BAYMAX_VIRTUAL_FILENAME) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = FRAGMENT_SIZE * BAYMAX_FRAGMENTS_COUNT;
        return 0;
    }

    const char *filename_in_mount = path + 1;
    DIR *dp = opendir(relics_base_path);
    if (dp == NULL) {
        return -errno;
    }

    struct dirent *de;
    long total_size = 0;
    int fragment_count = 0;
    char first_fragment_path[PATH_MAX] = {0};
    int found = 0;

    while ((de = readdir(dp)) != NULL) {
        if (strncmp(de->d_name, filename_in_mount, strlen(filename_in_mount)) == 0 &&
            de->d_name[strlen(filename_in_mount)] == '.' &&
            strlen(de->d_name) > strlen(filename_in_mount) + 4 &&
            isdigit(de->d_name[strlen(filename_in_mount) + 1]) &&
            isdigit(de->d_name[strlen(filename_in_mount) + 2]) &&
            isdigit(de->d_name[strlen(filename_in_mount) + 3])) {
            
            if (!found) {
                 get_relics_path(de->d_name, first_fragment_path);
                 found = 1;
            }
            
            char current_frag_path[PATH_MAX];
            get_relics_path(de->d_name, current_frag_path);
            struct stat frag_st;
            if (stat(current_frag_path, &frag_st) == 0) {
                total_size += frag_st.st_size;
            }
            fragment_count++;
        }
    }
    closedir(dp);

    if (fragment_count > 0) {
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
        stbuf->st_size = total_size;
        if (found) {
            struct stat first_frag_st;
            if (stat(first_fragment_path, &first_frag_st) == 0) {
                 stbuf->st_atime = first_frag_st.st_atime;
                 stbuf->st_mtime = first_frag_st.st_mtime;
                 stbuf->st_ctime = first_frag_st.st_ctime;
            }
        }
        return 0;
    }

    return -ENOENT;
}

static int bb_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi,
                      enum fuse_readdir_flags flags) {
    (void)offset;
    (void)fi;    
    (void)flags; 

    if (strcmp(path, "/") != 0) {
        return -ENOENT;
    }

    filler(buf, ".", NULL, 0, 0); 
    filler(buf, "..", NULL, 0, 0);

    filler(buf, BAYMAX_VIRTUAL_FILENAME, NULL, 0, 0);

    DIR *dp = opendir(relics_base_path);
    if (dp == NULL) {
        return -errno;
    }

    struct dirent *de;
    char listed_files[256][FILENAME_MAX];
    int listed_count = 0;

    while ((de = readdir(dp)) != NULL) {
        if (de->d_type == DT_REG) {
            char *last_dot = strrchr(de->d_name, '.');
            if (last_dot && (last_dot - de->d_name > 0) && strlen(last_dot + 1) == 3 &&
                isdigit(last_dot[1]) && isdigit(last_dot[2]) && isdigit(last_dot[3])) {
                
                size_t base_len = last_dot - de->d_name;
                char base_filename[FILENAME_MAX];
                strncpy(base_filename, de->d_name, base_len);
                base_filename[base_len] = '\0';

                if (strcmp(base_filename, BAYMAX_VIRTUAL_FILENAME) == 0) {
                    continue;
                }

                int already_listed = 0;
                for (int i = 0; i < listed_count; ++i) {
                    if (strcmp(listed_files[i], base_filename) == 0) {
                        already_listed = 1;
                        break;
                    }
                }

                if (!already_listed) {
                    filler(buf, base_filename, NULL, 0, 0);
                    if (listed_count < 255) {
                       strncpy(listed_files[listed_count++], base_filename, FILENAME_MAX -1);
                       listed_files[listed_count-1][FILENAME_MAX-1] = '\0';
                    }
                }
            }
        }
    }
    closedir(dp);
    return 0;
}

static int bb_open(const char *path, struct fuse_file_info *fi) {
    if (strcmp(path + 1, BAYMAX_VIRTUAL_FILENAME) == 0) {
        if ((fi->flags & O_ACCMODE) != O_RDONLY) {
            return -EACCES;
        }
    }
    return 0;
}

static int bb_read(const char *path, char *buf, size_t size, off_t offset,
                   struct fuse_file_info *fi) {
    (void)fi;
    const char *filename_in_mount = path + 1;
    char log_details[512];
    size_t total_bytes_read = 0;

    if (strcmp(filename_in_mount, BAYMAX_VIRTUAL_FILENAME) == 0) {
        snprintf(log_details, sizeof(log_details), "%s -> /tmp/%s", BAYMAX_VIRTUAL_FILENAME, BAYMAX_VIRTUAL_FILENAME);
        log_activity("COPY", log_details);

        for (int i = 0; i < BAYMAX_FRAGMENTS_COUNT; ++i) {
            char fragment_path[PATH_MAX];
            get_full_fragment_path(BAYMAX_VIRTUAL_FILENAME, i, fragment_path);

            FILE *f = fopen(fragment_path, "rb");
            if (f == NULL) continue;

            fseek(f, 0, SEEK_END);
            long frag_size = ftell(f);
            fseek(f, 0, SEEK_SET);

            long current_file_offset_start = (long)i * FRAGMENT_SIZE;
            long current_file_offset_end = current_file_offset_start + frag_size;

            if (offset < current_file_offset_end && offset + (long)size > current_file_offset_start) {
                long read_start_in_frag = 0;
                if (offset > current_file_offset_start) {
                    read_start_in_frag = offset - current_file_offset_start;
                }

                long bytes_to_read_from_frag = frag_size - read_start_in_frag;
                if (bytes_to_read_from_frag > (long)(size - total_bytes_read)) {
                    bytes_to_read_from_frag = (long)(size - total_bytes_read);
                }
                
                if (bytes_to_read_from_frag > 0) {
                     fseek(f, read_start_in_frag, SEEK_SET);
                     size_t read_now = fread(buf + total_bytes_read, 1, bytes_to_read_from_frag, f);
                     total_bytes_read += read_now;
                }
            }
            fclose(f);
            if (total_bytes_read >= size) break;
        }
        return total_bytes_read;
    }

    snprintf(log_details, sizeof(log_details), "%s", filename_in_mount);
    log_activity("READ", log_details);

    DIR *dp = opendir(relics_base_path);
    if (!dp) return -errno;

    struct dirent *de;
    char fragment_paths[512][PATH_MAX];
    int num_fragments = 0;

    while((de = readdir(dp)) != NULL && num_fragments < 512) {
        if (strncmp(de->d_name, filename_in_mount, strlen(filename_in_mount)) == 0 &&
            de->d_name[strlen(filename_in_mount)] == '.' &&
            isdigit(de->d_name[strlen(filename_in_mount) + 1])) {
            
            get_relics_path(de->d_name, fragment_paths[num_fragments]);
            num_fragments++;
        }
    }
    closedir(dp);
    
    qsort(fragment_paths, num_fragments, PATH_MAX, (int (*)(const void*, const void*)) strcmp);
    
    long current_virtual_offset = 0;
    for (int i = 0; i < num_fragments; ++i) {
        FILE *f = fopen(fragment_paths[i], "rb");
        if (!f) continue;

        fseek(f, 0, SEEK_END);
        long frag_len = ftell(f);
        fseek(f, 0, SEEK_SET);

        long frag_start_offset_in_virtual = current_virtual_offset;
        long frag_end_offset_in_virtual = current_virtual_offset + frag_len;

        if (offset < frag_end_offset_in_virtual && offset + (long)size > frag_start_offset_in_virtual) {
            long read_offset_in_frag = 0;
            if (offset > frag_start_offset_in_virtual) {
                read_offset_in_frag = offset - frag_start_offset_in_virtual;
            }
            
            long bytes_to_read_this_frag = frag_len - read_offset_in_frag;
            if (bytes_to_read_this_frag > (long)(size - total_bytes_read)) {
                 bytes_to_read_this_frag = (long)(size - total_bytes_read);
            }

            if (bytes_to_read_this_frag > 0) {
                fseek(f, read_offset_in_frag, SEEK_SET);
                size_t just_read = fread(buf + total_bytes_read, 1, bytes_to_read_this_frag, f);
                total_bytes_read += just_read;
            }
        }
        fclose(f);
        current_virtual_offset += frag_len;
        if (total_bytes_read >= size) break;
    }
    return total_bytes_read;
}

static int bb_mknod(const char *path, mode_t mode, dev_t rdev) {
    (void)rdev;
    const char *filename_in_mount = path + 1;

    if (strcmp(filename_in_mount, BAYMAX_VIRTUAL_FILENAME) == 0) {
        return -EPERM;
    }
    return 0;
}

static int bb_write(const char *path, const char *buf, size_t size,
                    off_t offset, struct fuse_file_info *fi) {
    (void)fi;
    const char *base_filename = path + 1;
    char log_details[1024];

    if (strcmp(base_filename, BAYMAX_VIRTUAL_FILENAME) == 0) {
        return -EROFS;
    }

    size_t bytes_written_total = 0;
    off_t current_offset_in_virtual_file = offset;
    int fragment_index = offset / FRAGMENT_SIZE;

    while (bytes_written_total < size) {
        char fragment_path[PATH_MAX];
        get_full_fragment_path(base_filename, fragment_index, fragment_path);

        FILE *f = fopen(fragment_path, "r+b");
        if (f == NULL) {
            f = fopen(fragment_path, "w+b");
            if (f == NULL) {
                perror("Gagal membuka/membuat file fragmen untuk ditulis");
                return -errno;
            }
        }

        off_t offset_in_fragment = current_offset_in_virtual_file % FRAGMENT_SIZE;
        fseek(f, offset_in_fragment, SEEK_SET);

        size_t bytes_to_write_in_fragment = FRAGMENT_SIZE - offset_in_fragment;
        if (bytes_to_write_in_fragment > (size - bytes_written_total)) {
            bytes_to_write_in_fragment = size - bytes_written_total;
        }

        size_t written_now = fwrite(buf + bytes_written_total, 1, bytes_to_write_in_fragment, f);
        fclose(f);

        if (written_now == 0 && bytes_to_write_in_fragment > 0) {
             if (bytes_written_total == 0) return -EIO;
             break;
        }
        
        bytes_written_total += written_now;
        current_offset_in_virtual_file += written_now;
        fragment_index++;

        if (written_now < bytes_to_write_in_fragment) {
            break;
        }
    }
    
    DIR *dp = opendir(relics_base_path);
    if (!dp) return bytes_written_total;

    struct dirent *de;
    char fragment_list_str[512] = "";
    int first_frag = 1;
    char existing_fragments[256][FILENAME_MAX];
    int existing_frag_count = 0;

    while((de = readdir(dp)) != NULL && existing_frag_count < 255) {
        if (strncmp(de->d_name, base_filename, strlen(base_filename)) == 0 &&
            de->d_name[strlen(base_filename)] == '.' &&
            isdigit(de->d_name[strlen(base_filename) + 1])) {
            strncpy(existing_fragments[existing_frag_count++], de->d_name, FILENAME_MAX -1);
            existing_fragments[existing_frag_count-1][FILENAME_MAX-1] = '\0';
        }
    }
    closedir(dp);
    qsort(existing_fragments, existing_frag_count, FILENAME_MAX, (int (*)(const void*, const void*)) strcmp);

    for(int k=0; k < existing_frag_count; ++k) {
        if (!first_frag) {
            strncat(fragment_list_str, ", ", sizeof(fragment_list_str) - strlen(fragment_list_str) - 1);
        }
        strncat(fragment_list_str, existing_fragments[k], sizeof(fragment_list_str) - strlen(fragment_list_str) - 1);
        first_frag = 0;
         if (strlen(fragment_list_str) > 450) {
            strncat(fragment_list_str, "...", sizeof(fragment_list_str) - strlen(fragment_list_str) - 1);
            break;
        }
    }

    snprintf(log_details, sizeof(log_details), "%s -> %s", base_filename, fragment_list_str);
    log_activity("WRITE", log_details);

    return bytes_written_total;
}

static int bb_unlink(const char *path) {
    const char *filename_in_mount = path + 1;
    char log_details[1024] = "";

    if (strcmp(filename_in_mount, BAYMAX_VIRTUAL_FILENAME) == 0) {
        char first_frag_name[FILENAME_MAX], last_frag_name[FILENAME_MAX];
        get_fragment_filename(BAYMAX_VIRTUAL_FILENAME, 0, first_frag_name);
        get_fragment_filename(BAYMAX_VIRTUAL_FILENAME, BAYMAX_FRAGMENTS_COUNT - 1, last_frag_name);
        snprintf(log_details, sizeof(log_details), "%s - %s", first_frag_name, last_frag_name);

        int all_deleted_or_not_exist = 1;
        int actual_deletions = 0;
        for (int i = 0; i < BAYMAX_FRAGMENTS_COUNT; ++i) {
            char fragment_path[PATH_MAX];
            get_full_fragment_path(BAYMAX_VIRTUAL_FILENAME, i, fragment_path);
            if (remove(fragment_path) != 0) {
                if (errno != ENOENT) {
                    all_deleted_or_not_exist = 0;
                }
            } else {
                actual_deletions++;
            }
        }
        if(all_deleted_or_not_exist) {
             log_activity("DELETE", log_details);
             return 0;
        }
        if (!all_deleted_or_not_exist) return -errno;
        return 0;
    }

    DIR *dp = opendir(relics_base_path);
    if (dp == NULL) {
        return -errno;
    }

    struct dirent *de;
    int deleted_count = 0;
    char deleted_fragments_str[512] = "";
    int first_del_frag = 1;

    char fragments_to_delete[256][FILENAME_MAX];
    int num_to_delete = 0;

    while((de = readdir(dp)) != NULL && num_to_delete < 255) {
        if (strncmp(de->d_name, filename_in_mount, strlen(filename_in_mount)) == 0 &&
            de->d_name[strlen(filename_in_mount)] == '.' &&
            isdigit(de->d_name[strlen(filename_in_mount)+1])) {
            strncpy(fragments_to_delete[num_to_delete++], de->d_name, FILENAME_MAX -1);
            fragments_to_delete[num_to_delete-1][FILENAME_MAX-1] = '\0';
        }
    }
    closedir(dp);
    
    if (num_to_delete == 0) return -ENOENT;

    qsort(fragments_to_delete, num_to_delete, FILENAME_MAX, (int (*)(const void*, const void*)) strcmp);

    int any_delete_failed_other_than_enoent = 0;
    int last_errno = 0;

    for (int i = 0; i < num_to_delete; ++i) {
        char full_frag_path[PATH_MAX];
        get_relics_path(fragments_to_delete[i], full_frag_path);
        if (remove(full_frag_path) == 0) {
            deleted_count++;
            if (!first_del_frag) {
                strncat(deleted_fragments_str, ", ", sizeof(deleted_fragments_str) - strlen(deleted_fragments_str) - 1);
            }
            strncat(deleted_fragments_str, fragments_to_delete[i], sizeof(deleted_fragments_str) - strlen(deleted_fragments_str) - 1);
            first_del_frag = 0;
            if (strlen(deleted_fragments_str) > 450) {
                 strncat(deleted_fragments_str, "...", sizeof(deleted_fragments_str) - strlen(deleted_fragments_str) - 1);
            }
        } else if (errno != ENOENT) {
            any_delete_failed_other_than_enoent = 1;
            last_errno = errno;
        }
    }

    if (deleted_count > 0) {
        snprintf(log_details, sizeof(log_details), "%s", deleted_fragments_str);
        log_activity("DELETE", log_details);
    }
    
    if (any_delete_failed_other_than_enoent) return -last_errno;
    if (deleted_count == 0 && num_to_delete > 0) return -ENOENT;
    
    return 0;
}

static const struct fuse_operations bb_oper = {
    .getattr = bb_getattr,
    .readdir = bb_readdir,
    .open    = bb_open,
    .read    = bb_read,
    .mknod   = bb_mknod,
    .write   = bb_write,
    .unlink  = bb_unlink,
};

int main(int argc, char *argv[]) {
    char cwd_path[PATH_MAX];
    if (getcwd(cwd_path, sizeof(cwd_path)) == NULL) {
        perror("Gagal mendapatkan current working directory");
        return 1;
    }
    
    char temp_relics_path[PATH_MAX];
    snprintf(temp_relics_path, PATH_MAX, "%s/%s", cwd_path, RELICS_DIR_NAME);

    if (realpath(temp_relics_path, relics_base_path) == NULL) {
         fprintf(stderr, "Peringatan: Direktori '%s' mungkin tidak ada atau tidak dapat diakses. Menggunakan path relatif.\n", temp_relics_path);
         strncpy(relics_base_path, temp_relics_path, PATH_MAX -1);
         relics_base_path[PATH_MAX-1] = '\0';
    }
    
    printf("Direktori Relics diatur ke: %s\n", relics_base_path);
    printf("File Log akan ditulis ke: %s (di direktori saat ini jika path tidak absolut)\n", LOG_FILE_NAME);

    return fuse_main(argc, argv, &bb_oper, NULL);
}
