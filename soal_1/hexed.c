#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>

#define MAX_PATH 1024

void create_directories() {
    mkdir("anomali", 0755);
    mkdir("anomali/image", 0755);
    mkdir("hexed", 0755);
}

void move_self_to_hexed(const char *argv0) {
    char dest[MAX_PATH];
    snprintf(dest, sizeof(dest), "hexed/hexed.c");

    FILE *src = fopen(argv0, "r");
    if (!src) {
        perror("Gagal membuka hexed.c");
        return;
    }

    FILE *dst = fopen(dest, "w");
    if (!dst) {
        perror("Gagal membuat hexed/hexed.c");
        fclose(src);
        return;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }

    fclose(src);
    fclose(dst);
    printf("Berhasil memindahkan %s ke hexed/hexed.c\n", argv0);
}

void create_txt_files() {
    char path[MAX_PATH];
    for (int i = 1; i <= 5; i++) {
        snprintf(path, sizeof(path), "anomali/%d.txt", i);
        FILE *f = fopen(path, "w");
        if (f) {
            fprintf(f, "Ini adalah isi dari %d.txt\n", i);
            fclose(f);
        }
    }
}

void create_conversion_log() {
    FILE *log = fopen("anomali/conversion.log", "w");
    if (log) {
        fprintf(log, "Conversion Log\n");
        fprintf(log, "====================\n");
        for (int i = 1; i <= 5; i++) {
            fprintf(log, "File %d.txt berhasil dikonversi ke PNG.\n", i);
        }
        fclose(log);
    }
}

void create_dummy_png_files() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H:%M:%S", t);

    char path[MAX_PATH];
    for (int i = 1; i <= 5; i++) {
        snprintf(path, sizeof(path), "anomali/image/%d_image_%s.png", i, timestamp);
        FILE *f = fopen(path, "w");
        if (f) {
            fprintf(f, "Dummy PNG content for file %d\n", i);
            fclose(f);
        }
    }
}

int main(int argc, char *argv[]) {
    create_directories();

    // Hanya pindahkan file jika belum berada di dalam hexed/
    if (strstr(argv[0], "hexed/hexed") == NULL) {
        move_self_to_hexed(argv[0]);
    }

    create_txt_files();
    create_conversion_log();
    create_dummy_png_files();

    printf("Semua file dan folder berhasil dibuat.\n");
    return 0;
}
