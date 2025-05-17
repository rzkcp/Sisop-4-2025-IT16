#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_PATH 2048
#define IMAGE_FOLDER "image"
#define LOG_FILE "conversion.log"

// Membuat folder "image" jika belum ada di dalam base_path
void ensure_image_directory(const char *base_path) {
    char image_path[MAX_PATH];
    snprintf(image_path, sizeof(image_path), "%s/%s", base_path, IMAGE_FOLDER);

    struct stat st = {0};
    if (stat(image_path, &st) == -1) {
        if (mkdir(image_path, 0700) == -1) {
            fprintf(stderr, "Failed to create directory %s: %s\n", image_path, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
}

// Mendapatkan timestamp dengan format custom
void get_timestamp(char *buffer, size_t size, const char *format) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, size, format, tm_info);
}

// Mengubah file hex (.txt) menjadi file image (.png)
int convert_hex_to_image(const char *hex_path, const char *output_dir, const char *filename, FILE *log_file) {
    FILE *hex_file = fopen(hex_path, "r");
    if (!hex_file) {
        perror("Failed to open hex file");
        return -1;
    }

    fseek(hex_file, 0, SEEK_END);
    long length = ftell(hex_file);
    fseek(hex_file, 0, SEEK_SET);

    char *hex_string = malloc(length + 1);
    if (!hex_string) {
        fclose(hex_file);
        return -1;
    }

    fread(hex_string, 1, length, hex_file);
    hex_string[length] = '\0';
    fclose(hex_file);

    // Convert hex string to binary data
    size_t hex_len = strlen(hex_string);
    size_t bin_len = hex_len / 2;
    unsigned char *bin_data = malloc(bin_len);
    if (!bin_data) {
        free(hex_string);
        return -1;
    }

    for (size_t i = 0; i < bin_len; i++) {
        sscanf(&hex_string[i * 2], "%2hhx", &bin_data[i]);
    }

    // Generate output filename dengan format timestamp berisi ':'
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp), "%Y-%m-%d_%H:%M:%S");

    char image_name[256];
    snprintf(image_name, sizeof(image_name), "%s_image_%s.png", filename, timestamp);

    char image_path[MAX_PATH];
    int written = snprintf(image_path, sizeof(image_path), "%s/%s/%s", output_dir, IMAGE_FOLDER, image_name);
    if (written < 0 || written >= (int)sizeof(image_path)) {
        fprintf(stderr, "Path too long for file: %s\n", image_name);
        free(hex_string);
        free(bin_data);
        return -1;
    }

    FILE *image_file = fopen(image_path, "wb");
    if (!image_file) {
        perror("Failed to create image file");
        free(hex_string);
        free(bin_data);
        return -1;
    }

    fwrite(bin_data, 1, bin_len, image_file);
    fclose(image_file);

    char log_time[32];
    get_timestamp(log_time, sizeof(log_time), "[%Y-%m-%d][%H:%M:%S]");
    fprintf(log_file, "%s: Successfully converted %s to %s\n", log_time, filename, image_name);

    free(hex_string);
    free(bin_data);
    return 0;
}

void process_txt_files(const char *folder_path) {
    ensure_image_directory(folder_path);

    char log_path[MAX_PATH];
    snprintf(log_path, sizeof(log_path), "%s/%s", folder_path, LOG_FILE);

    FILE *log_file = fopen(log_path, "a");
    if (!log_file) {
        perror("Failed to open log file");
        return;
    }

    DIR *dir = opendir(folder_path);
    if (!dir) {
        perror("Failed to open directory");
        fclose(log_file);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG) {
            const char *name = entry->d_name;
            size_t len = strlen(name);
            if (len > 4 && strcmp(&name[len - 4], ".txt") == 0) {
                char file_path[MAX_PATH];
                int written = snprintf(file_path, sizeof(file_path), "%s/%s", folder_path, name);
                if (written < 0 || written >= (int)sizeof(file_path)) {
                    fprintf(stderr, "File path too long, skipping: %s\n", name);
                    continue;
                }

                char filename_without_ext[256];
                strncpy(filename_without_ext, name, len - 4);
                filename_without_ext[len - 4] = '\0';

                if (convert_hex_to_image(file_path, folder_path, filename_without_ext, log_file) == -1) {
                    fprintf(stderr, "Failed to convert %s\n", name);
                }
            }
        }
    }

    closedir(dir);
    fclose(log_file);
}

int main() {
    const char *folder_path = "./anomali";  // Folder lokasi file .txt dan folder image

    process_txt_files(folder_path);

    printf("All hex files processed and converted to images.\n");
    return 0;
}
