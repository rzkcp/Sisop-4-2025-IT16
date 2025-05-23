#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAX_PATH 1024
#define IMAGE_DIR "anomali/image"
#define LOG_FILE "anomali/conversion.log"
#define HEXED_DIR "hexed"
#define NUM_FILES 5

void create_directories() {
    mkdir("anomali", 0755);
    mkdir(IMAGE_DIR, 0755);
    mkdir(HEXED_DIR, 0755);
}

void create_txt_files() {
    char path[MAX_PATH];
    for (int i = 1; i <= NUM_FILES; i++) {
        snprintf(path, sizeof(path), "anomali/%d.txt", i);
        FILE *f = fopen(path, "w");
        if (f) {
            fprintf(f, "Isi file %d\n", i);
            fclose(f);
        }
    }
}

void create_dummy_images() {
    FILE *log_fp = fopen(LOG_FILE, "a");
    if (!log_fp) {
        perror("Gagal membuka conversion.log");
        return;
    }

    for (int i = 1; i <= NUM_FILES; i++) {
        char txt_file[MAX_PATH];
        snprintf(txt_file, sizeof(txt_file), "anomali/%d.txt", i);

        // Waktu sekarang
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H:%M:%S", tm_info);

        // Nama file gambar
        char image_file[MAX_PATH];
        snprintf(image_file, sizeof(image_file), "%s/%d_image_%s.png", IMAGE_DIR, i, timestamp);

        // Buat data gambar 32x32 warna merah (RGBA)
        int width = 32, height = 32;
        unsigned char image_data[32 * 32 * 3];
        for (int p = 0; p < width * height * 3; p += 3) {
            image_data[p] = 255;     // R
            image_data[p + 1] = 0;   // G
            image_data[p + 2] = 0;   // B
        }

        int success = stbi_write_png(image_file, width, height, 3, image_data, width * 3);
        if (success) {
            fprintf(log_fp, "Berhasil membuat %s dari %s\n", image_file, txt_file);
        } else {
            fprintf(log_fp, "Gagal membuat gambar %s\n", image_file);
        }

        sleep(1); // Beda timestamp
    }

    fclose(log_fp);
}

void copy_self_to_hexed() {
    char *self_path = realpath("/proc/self/exe", NULL);
    if (!self_path) {
        perror("Gagal mendapatkan path executable");
        return;
    }

    FILE *src = fopen(self_path, "rb");
    if (!src) {
        perror("Gagal membuka executable");
        free(self_path);
        return;
    }

    char dest_path[MAX_PATH];
    snprintf(dest_path, sizeof(dest_path), "%s/hexed.c", HEXED_DIR);

    FILE *dest = fopen(dest_path, "wb");
    if (!dest) {
        perror("Gagal membuat hexed.c di hexed/");
        fclose(src);
        free(self_path);
        return;
    }

    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        fwrite(buf, 1, n, dest);
    }

    fclose(src);
    fclose(dest);
    free(self_path);
}

int main() {
    create_directories();
    create_txt_files();
    create_dummy_images();
    copy_self_to_hexed();

    printf("Semua file dan folder berhasil dibuat.\n");
    return 0;
}
