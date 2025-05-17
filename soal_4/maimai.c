/*
Versi 1
*/

// #define FUSE_USE_VERSION 26
// #include <fuse.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <dirent.h>
// #include <errno.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <sys/stat.h>
// #include <openssl/evp.h>
// #include <openssl/rand.h>
// #include <zlib.h>

// #define MAX_PATH_LEN 4096
// #define BASE_DIR "chiho"
// #define MOUNT_DIR "fuse_dir"

// // Kunci AES-256 (32 byte)
// static const unsigned char secret_key[] = "unique_key_12345678901234567890";

// // Daftar chiho yang valid
// static const char *valid_chiho[] = {"starter", "metro", "dragon", "blackrose", "heaven", "skystreet"};
// static const int num_chiho = 6;

// // Ekstensi khusus untuk setiap chiho
// static const char *chiho_ext[] = {".mai", ".ccc", ".rot", ".bin", ".enc", ".gz"};

// // Ekstrak nama chiho dari path
// static char* get_chiho_name(const char *path) {
//     char *chiho = strdup(path + 1); // Lewati karakter '/' awal
//     char *slash = strchr(chiho, '/');
//     if (slash) *slash = '\0';
//     return chiho;
// }

// // Dapatkan indeks chiho
// static int get_chiho_index(const char *chiho) {
//     for (int i = 0; i < num_chiho; i++) {
//         if (strcmp(chiho, valid_chiho[i]) == 0) return i;
//     }
//     return -1;
// }

// // Bangun path asli berdasarkan chiho dengan ekstensi
// static void construct_real_path(char *real_path, const char *path, const char *chiho) {
//     int chiho_idx = get_chiho_index(chiho);
//     const char *ext = (chiho_idx >= 0) ? chiho_ext[chiho_idx] : "";
//     char *filename = strrchr(path, '/');
//     if (filename) filename++;
//     else filename = (char *)path + 1;

//     snprintf(real_path, MAX_PATH_LEN, "%s/%s/%s%s", BASE_DIR, chiho, filename, ext);
// }

// // Geser isi file untuk Metro Chiho
// static void shift_content(char *data, size_t length, int shift_dir) {
//     for (size_t i = 0; i < length; i++) {
//         data[i] = (data[i] + (shift_dir * (i + 1))) % 256;
//     }
// }

// // ROT13 untuk Dragon Chiho
// static void perform_rot13(char *data, size_t length) {
//     for (size_t i = 0; i < length; i++) {
//         if ((data[i] >= 'A' && data[i] <= 'Z') || (data[i] >= 'a' && data[i] <= 'z')) {
//             char base = (data[i] >= 'A' && data[i] <= 'Z') ? 'A' : 'a';
//             data[i] = (data[i] - base + 13) % 26 + base;
//         }
//     }
// }

// // Enkripsi AES-256-CBC untuk Heaven Chiho
// static int encrypt_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
//     unsigned char iv[16];
//     RAND_bytes(iv, 16);
//     int len, total_len;

//     EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, secret_key, iv);
//     memcpy(output, iv, 16);
//     EVP_EncryptUpdate(ctx, (unsigned char *)output + 16, &len, (unsigned char *)input, in_len);
//     total_len = len;
//     EVP_EncryptFinal_ex(ctx, (unsigned char *)output + 16 + len, &len);
//     total_len += len;
//     *out_len = total_len + 16;
//     EVP_CIPHER_CTX_free(ctx);
//     return 0;
// }

// // Dekripsi AES-256-CBC untuk Heaven Chiho
// static int decrypt_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
//     unsigned char iv[16];
//     memcpy(iv, input, 16);
//     int len, total_len;

//     EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, secret_key, iv);
//     EVP_DecryptUpdate(ctx, (unsigned char *)output, &len, (unsigned char *)input + 16, in_len - 16);
//     total_len = len;
//     EVP_DecryptFinal_ex(ctx, (unsigned char *)output + len, &len);
//     total_len += len;
//     *out_len = total_len;
//     EVP_CIPHER_CTX_free(ctx);
//     return 0;
// }

// // Kompresi gzip untuk Skystreet Chiho
// static int compress_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     z_stream stream = {0};
//     deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
//     stream.next_in = (Bytef *)input;
//     stream.avail_in = in_len;
//     stream.next_out = (Bytef *)output;
//     stream.avail_out = *out_len;
//     deflate(&stream, Z_FINISH);
//     *out_len = stream.total_out;
//     deflateEnd(&stream);
//     return 0;
// }

// // Dekompresi gzip untuk Skystreet Chiho
// static int decompress_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     z_stream stream = {0};
//     inflateInit2(&stream, 15 + 16);
//     stream.next_in = (Bytef *)input;
//     stream.avail_in = in_len;
//     stream.next_out = (Bytef *)output;
//     stream.avail_out = *out_len;
//     inflate(&stream, Z_FINISH);
//     *out_len = stream.total_out;
//     inflateEnd(&stream);
//     return 0;
// }

// // Validasi apakah chiho valid
// static int is_valid_chiho(const char *chiho) {
//     for (int i = 0; i < num_chiho; i++) {
//         if (strcmp(chiho, valid_chiho[i]) == 0) return 1;
//     }
//     return 0;
// }

// // FUSE: getattr
// static int maimai_getattr(const char *path, struct stat *stbuf) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);

//     construct_real_path(real_path, path, chiho);

//     int result = stat(real_path, stbuf);
//     if (result == 0) {
//         free(chiho);
//         return 0;
//     }

//     if (strcmp(path, "/") == 0 || (strlen(path) > 1 && strchr(path + 1, '/') == NULL && is_valid_chiho(chiho))) {
//         memset(stbuf, 0, sizeof(struct stat));
//         stbuf->st_mode = S_IFDIR | 0755;
//         stbuf->st_nlink = 2;
//         free(chiho);
//         return 0;
//     }

//     free(chiho);
//     return -ENOENT;
// }

// // FUSE: readdir
// static int maimai_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);

//     if (strcmp(path, "/") == 0) {
//         filler(buf, ".", NULL, 0);
//         filler(buf, "..", NULL, 0);
//         for (int i = 0; i < num_chiho; i++) {
//             filler(buf, valid_chiho[i], NULL, 0);
//         }
//         free(chiho);
//         return 0;
//     }

//     construct_real_path(real_path, path, chiho);

//     DIR *dir = opendir(real_path);
//     if (!dir) {
//         filler(buf, ".", NULL, 0);
//         filler(buf, "..", NULL, 0);
//         free(chiho);
//         return 0;
//     }

//     struct dirent *entry;
//     while ((entry = readdir(dir)) != NULL) {
//         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

//         char *display_name = strdup(entry->d_name);
//         int ext_len = strlen(chiho_ext[get_chiho_index(chiho)]);
//         if (strncmp(display_name + strlen(display_name) - ext_len, chiho_ext[get_chiho_index(chiho)], ext_len) == 0) {
//             display_name[strlen(display_name) - ext_len] = '\0';
//         }
//         filler(buf, display_name, NULL, 0);
//         free(display_name);
//     }

//     filler(buf, ".", NULL, 0);
//     filler(buf, "..", NULL, 0);

//     closedir(dir);
//     free(chiho);
//     return 0;
// }

// // FUSE: open
// static int maimai_open(const char *path, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     construct_real_path(real_path, path, chiho);

//     int fd = open(real_path, fi->flags);
//     if (fd == -1) {
//         free(chiho);
//         return -errno;
//     }
//     fi->fh = fd;
//     free(chiho);
//     return 0;
// }

// // FUSE: read
// static int maimai_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     construct_real_path(real_path, path, chiho);

//     int fd = fi->fh;
//     if (lseek(fd, offset, SEEK_SET) == -1) return -errno;

//     char *temp_buf = malloc(size + 16);
//     int bytes_read = read(fd, temp_buf, size);
//     if (bytes_read == -1) {
//         free(temp_buf);
//         free(chiho);
//         return -errno;
//     }

//     if (strcmp(chiho, "dragon") == 0) {
//         perform_rot13(temp_buf, bytes_read);
//         memcpy(buf, temp_buf, bytes_read);
//     } else if (strcmp(chiho, "heaven") == 0) {
//         size_t decrypted_len;
//         decrypt_content(temp_buf, bytes_read, buf, &decrypted_len);
//         bytes_read = decrypted_len;
//     } else if (strcmp(chiho, "skystreet") == 0) {
//         size_t decompressed_len = size * 10;
//         char *decompressed = malloc(decompressed_len);
//         decompress_content(temp_buf, bytes_read, decompressed, &decompressed_len);
//         memcpy(buf, decompressed, decompressed_len);
//         bytes_read = decompressed_len;
//         free(decompressed);
//     } else if (strcmp(chiho, "metro") == 0) {
//         shift_content(temp_buf, bytes_read, -1); // Unshift isi file Metro
//         memcpy(buf, temp_buf, bytes_read);
//     } else {
//         memcpy(buf, temp_buf, bytes_read); // Blackrose dan lainnya tanpa transformasi tambahan
//     }

//     free(temp_buf);
//     free(chiho);
//     return bytes_read;
// }

// // FUSE: write
// static int maimai_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     construct_real_path(real_path, path, chiho);

//     int fd = fi->fh;
//     if (lseek(fd, offset, SEEK_SET) == -1) return -errno;

//     char *temp_buf = malloc(size + 16);
//     memcpy(temp_buf, buf, size);
//     size_t processed_len = size;

//     if (strcmp(chiho, "dragon") == 0) {
//         perform_rot13(temp_buf, size);
//     } else if (strcmp(chiho, "heaven") == 0) {
//         encrypt_content(buf, size, temp_buf, &processed_len);
//     } else if (strcmp(chiho, "skystreet") == 0) {
//         processed_len = size * 2;
//         char *compressed = malloc(processed_len);
//         compress_content(buf, size, compressed, &processed_len);
//         memcpy(temp_buf, compressed, processed_len);
//         free(compressed);
//     } else if (strcmp(chiho, "metro") == 0) {
//         shift_content(temp_buf, size, 1); // Shift isi file Metro
//         processed_len = size;
//     } else {
//         processed_len = size; // Blackrose dan lainnya tanpa transformasi tambahan
//     }

//     int bytes_written = write(fd, temp_buf, processed_len);
//     free(temp_buf);
//     free(chiho);
//     return bytes_written == -1 ? -errno : bytes_written;
// }

// // FUSE: create
// static int maimai_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     construct_real_path(real_path, path, chiho);

//     char *dir_path = strdup(real_path);
//     char *last_slash = strrchr(dir_path, '/');
//     if (last_slash) *last_slash = '\0';
//     if (mkdir(dir_path, 0755) == -1 && errno != EEXIST) {
//         free(dir_path);
//         free(chiho);
//         return -errno;
//     }
//     free(dir_path);

//     int fd = open(real_path, O_CREAT | O_WRONLY | O_TRUNC, mode);
//     if (fd == -1) {
//         free(chiho);
//         return -errno;
//     }
//     fi->fh = fd;
//     free(chiho);
//     return 0;
// }

// // FUSE: unlink
// static int maimai_unlink(const char *path) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     construct_real_path(real_path, path, chiho);

//     int result = unlink(real_path);
//     free(chiho);
//     return result == -1 ? -errno : 0;
// }

// // FUSE: utimens
// static int maimai_utimens(const char *path, const struct timespec tv[2]) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     construct_real_path(real_path, path, chiho);

//     int result = utimensat(0, real_path, tv, 0);
//     free(chiho);
//     return result == -1 ? -errno : 0;
// }

// // Struktur operasi FUSE
// static struct fuse_operations maimai_ops = {
//     .getattr  = maimai_getattr,
//     .readdir  = maimai_readdir,
//     .open     = maimai_open,
//     .read     = maimai_read,
//     .write    = maimai_write,
//     .create   = maimai_create,
//     .unlink   = maimai_unlink,
//     .utimens  = maimai_utimens,
// };

// // Fungsi utama dengan pembuatan direktori
// int main(int argc, char *argv[]) {
//     struct stat st;
//     if (stat(BASE_DIR, &st) == -1) {
//         if (mkdir(BASE_DIR, 0755) == -1) {
//             perror("Gagal membuat direktori chiho");
//             return 1;
//         }
//     }

//     for (int i = 0; i < num_chiho; i++) {
//         char dir_path[MAX_PATH_LEN];
//         snprintf(dir_path, MAX_PATH_LEN, "%s/%s", BASE_DIR, valid_chiho[i]);
//         if (stat(dir_path, &st) == -1) {
//             if (mkdir(dir_path, 0755) == -1) {
//                 perror("Gagal membuat subdirektori chiho");
//                 return 1;
//             }
//         }
//     }

//     if (argc < 2) {
//         fprintf(stderr, "Penggunaan: %s <mountpoint>\n", argv[0]);
//         return 1;
//     }

//     const char *mountpoint = argv[1];
//     if (stat(mountpoint, &st) == -1) {
//         if (mkdir(mountpoint, 0755) == -1) {
//             perror("Gagal membuat direktori mountpoint");
//             return 1;
//         }
//     }

//     return fuse_main(argc, argv, &maimai_ops, NULL);
// }

/*
Versi ke-2 (kurang 7sref)
*/

// #define FUSE_USE_VERSION 26
// #include <fuse.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <dirent.h>
// #include <errno.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <sys/stat.h>
// #include <openssl/evp.h>
// #include <openssl/rand.h>
// #include <zlib.h>

// #define MAX_PATH_LEN 4096
// #define BASE_DIR "chiho"
// #define MOUNT_DIR "fuse_dir"

// // Kunci AES-256 (32 byte)
// static const unsigned char secret_key[] = "unique_key_12345678901234567890";

// // Daftar chiho yang valid, termasuk 7sref
// static const char *valid_chiho[] = {"starter", "metro", "dragon", "blackrose", "heaven", "skystreet", "7sref"};
// static const int num_chiho = 7;

// // Ekstensi khusus untuk setiap chiho (tidak termasuk 7sref)
// static const char *chiho_ext[] = {".mai", ".ccc", ".rot", ".bin", ".enc", ".gz"};

// // Ekstrak nama chiho dari path
// static char* get_chiho_name(const char *path) {
//     char *chiho = strdup(path + 1); // Lewati karakter '/' awal
//     char *slash = strchr(chiho, '/');
//     if (slash) *slash = '\0';
//     return chiho;
// }

// // Dapatkan indeks chiho
// static int get_chiho_index(const char *chiho) {
//     for (int i = 0; i < num_chiho - 1; i++) { // Tidak termasuk 7sref
//         if (strcmp(chiho, valid_chiho[i]) == 0) return i;
//     }
//     return -1;
// }

// // Bangun path asli berdasarkan chiho dengan ekstensi
// static void construct_real_path(char *real_path, const char *path, const char *chiho) {
//     int chiho_idx = get_chiho_index(chiho);
//     const char *ext = (chiho_idx >= 0) ? chiho_ext[chiho_idx] : "";
//     char *filename = strrchr(path, '/');
//     if (filename) filename++;
//     else filename = (char *)path + 1;

//     snprintf(real_path, MAX_PATH_LEN, "%s/%s/%s%s", BASE_DIR, chiho, filename, ext);
// }

// // Geser isi file untuk Metro Chiho
// static void shift_content(char *data, size_t length, int shift_dir) {
//     for (size_t i = 0; i < length; i++) {
//         data[i] = (data[i] + (shift_dir * (i + 1))) % 256;
//     }
// }

// // ROT13 untuk Dragon Chiho
// static void perform_rot13(char *data, size_t length) {
//     for (size_t i = 0; i < length; i++) {
//         if ((data[i] >= 'A' && data[i] <= 'Z') || (data[i] >= 'a' && data[i] <= 'z')) {
//             char base = (data[i] >= 'A' && data[i] <= 'Z') ? 'A' : 'a';
//             data[i] = (data[i] - base + 13) % 26 + base;
//         }
//     }
// }

// // Enkripsi AES-256-CBC untuk Heaven Chiho
// static int encrypt_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
//     unsigned char iv[16];
//     RAND_bytes(iv, 16);
//     int len, total_len;

//     EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, secret_key, iv);
//     memcpy(output, iv, 16);
//     EVP_EncryptUpdate(ctx, (unsigned char *)output + 16, &len, (unsigned char *)input, in_len);
//     total_len = len;
//     EVP_EncryptFinal_ex(ctx, (unsigned char *)output + 16 + len, &len);
//     total_len += len;
//     *out_len = total_len + 16;
//     EVP_CIPHER_CTX_free(ctx);
//     return 0;
// }

// // Dekripsi AES-256-CBC untuk Heaven Chiho
// static int decrypt_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
//     unsigned char iv[16];
//     memcpy(iv, input, 16);
//     int len, total_len;

//     EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, secret_key, iv);
//     EVP_DecryptUpdate(ctx, (unsigned char *)output, &len, (unsigned char *)input + 16, in_len - 16);
//     total_len = len;
//     EVP_DecryptFinal_ex(ctx, (unsigned char *)output + len, &len);
//     total_len += len;
//     *out_len = total_len;
//     EVP_CIPHER_CTX_free(ctx);
//     return 0;
// }

// // Kompresi gzip untuk Skystreet Chiho
// static int compress_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     z_stream stream = {0};
//     deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
//     stream.next_in = (Bytef *)input;
//     stream.avail_in = in_len;
//     stream.next_out = (Bytef *)output;
//     stream.avail_out = *out_len;
//     deflate(&stream, Z_FINISH);
//     *out_len = stream.total_out;
//     deflateEnd(&stream);
//     return 0;
// }

// // Dekompresi gzip untuk Skystreet Chiho
// static int decompress_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     z_stream stream = {0};
//     inflateInit2(&stream, 15 + 16);
//     stream.next_in = (Bytef *)input;
//     stream.avail_in = in_len;
//     stream.next_out = (Bytef *)output;
//     stream.avail_out = *out_len;
//     inflate(&stream, Z_FINISH);
//     *out_len = stream.total_out;
//     inflateEnd(&stream);
//     return 0;
// }

// // Validasi apakah chiho valid
// static int is_valid_chiho(const char *chiho) {
//     for (int i = 0; i < num_chiho; i++) {
//         if (strcmp(chiho, valid_chiho[i]) == 0) return 1;
//     }
//     return 0;
// }

// // Parse path 7sref: /7sref/[area]_[nama_file] -> /area/nama_file
// static int parse_7sref_path(const char *path, char **target_chiho, char **target_file) {
//     if (strncmp(path, "/7sref/", 7) != 0) return 0;

//     char *filename = (char *)path + 7; // Lewati "/7sref/"
//     char *underscore = strchr(filename, '_');
//     if (!underscore) return 0;

//     *underscore = '\0';
//     *target_chiho = strdup(filename);
//     *target_file = strdup(underscore + 1);

//     if (!is_valid_chiho(*target_chiho) || strcmp(*target_chiho, "7sref") == 0) {
//         free(*target_chiho);
//         free(*target_file);
//         return 0;
//     }

//     *underscore = '_'; // Kembalikan underscore
//     return 1;
// }

// // Bangun path target dari 7sref
// static void construct_7sref_target_path(char *target_path, const char *target_chiho, const char *target_file) {
//     snprintf(target_path, MAX_PATH_LEN, "/%s/%s", target_chiho, target_file);
// }

// // FUSE: getattr
// static int maimai_getattr(const char *path, struct stat *stbuf) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int result = stat(real_path, stbuf);
//     if (result == 0) {
//         free(chiho);
//         return 0;
//     }

//     if (strcmp(path, "/") == 0 || (strlen(path) > 1 && strchr(path + 1, '/') == NULL && is_valid_chiho(chiho))) {
//         memset(stbuf, 0, sizeof(struct stat));
//         stbuf->st_mode = S_IFDIR | 0755;
//         stbuf->st_nlink = 2;
//         free(chiho);
//         return 0;
//     }

//     free(chiho);
//     return -ENOENT;
// }

// // FUSE: readdir
// static int maimai_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);

//     if (strcmp(path, "/") == 0) {
//         filler(buf, ".", NULL, 0);
//         filler(buf, "..", NULL, 0);
//         for (int i = 0; i < num_chiho; i++) {
//             filler(buf, valid_chiho[i], NULL, 0);
//         }
//         free(chiho);
//         return 0;
//     }

//     if (strcmp(chiho, "7sref") == 0) {
//         filler(buf, ".", NULL, 0);
//         filler(buf, "..", NULL, 0);
//         for (int i = 0; i < num_chiho - 1; i++) { // Tidak termasuk 7sref
//             char chiho_path[MAX_PATH_LEN];
//             snprintf(chiho_path, MAX_PATH_LEN, "%s/%s", BASE_DIR, valid_chiho[i]);
//             DIR *dir = opendir(chiho_path);
//             if (!dir) continue;

//             struct dirent *entry;
//             while ((entry = readdir(dir)) != NULL) {
//                 if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

//                 char *display_name = strdup(entry->d_name);
//                 int ext_len = strlen(chiho_ext[i]);
//                 if (strncmp(display_name + strlen(display_name) - ext_len, chiho_ext[i], ext_len) == 0) {
//                     display_name[strlen(display_name) - ext_len] = '\0';
//                 }

//                 char virtual_name[MAX_PATH_LEN];
//                 snprintf(virtual_name, MAX_PATH_LEN, "%s_%s", valid_chiho[i], display_name);
//                 filler(buf, virtual_name, NULL, 0);
//                 free(display_name);
//             }
//             closedir(dir);
//         }
//         free(chiho);
//         return 0;
//     }

//     construct_real_path(real_path, path, chiho);

//     DIR *dir = opendir(real_path);
//     if (!dir) {
//         filler(buf, ".", NULL, 0);
//         filler(buf, "..", NULL, 0);
//         free(chiho);
//         return 0;
//     }

//     struct dirent *entry;
//     while ((entry = readdir(dir)) != NULL) {
//         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

//         char *display_name = strdup(entry->d_name);
//         int ext_len = strlen(chiho_ext[get_chiho_index(chiho)]);
//         if (strncmp(display_name + strlen(display_name) - ext_len, chiho_ext[get_chiho_index(chiho)], ext_len) == 0) {
//             display_name[strlen(display_name) - ext_len] = '\0';
//         }
//         filler(buf, display_name, NULL, 0);
//         free(display_name);
//     }

//     filler(buf, ".", NULL, 0);
//     filler(buf, "..", NULL, 0);

//     closedir(dir);
//     free(chiho);
//     return 0;
// }

// // FUSE: open
// static int maimai_open(const char *path, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int fd = open(real_path, fi->flags);
//     if (fd == -1) {
//         free(chiho);
//         return -errno;
//     }
//     fi->fh = fd;
//     free(chiho);
//     return 0;
// }

// // FUSE: read
// static int maimai_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int fd = fi->fh;
//     if (lseek(fd, offset, SEEK_SET) == -1) return -errno;

//     char *temp_buf = malloc(size + 16);
//     int bytes_read = read(fd, temp_buf, size);
//     if (bytes_read == -1) {
//         free(temp_buf);
//         free(chiho);
//         return -errno;
//     }

//     if (strcmp(chiho, "dragon") == 0) {
//         perform_rot13(temp_buf, bytes_read);
//         memcpy(buf, temp_buf, bytes_read);
//     } else if (strcmp(chiho, "heaven") == 0) {
//         size_t decrypted_len;
//         decrypt_content(temp_buf, bytes_read, buf, &decrypted_len);
//         bytes_read = decrypted_len;
//     } else if (strcmp(chiho, "skystreet") == 0) {
//         size_t decompressed_len = size * 10;
//         char *decompressed = malloc(decompressed_len);
//         decompress_content(temp_buf, bytes_read, decompressed, &decompressed_len);
//         memcpy(buf, decompressed, decompressed_len);
//         bytes_read = decompressed_len;
//         free(decompressed);
//     } else if (strcmp(chiho, "metro") == 0) {
//         shift_content(temp_buf, bytes_read, -1); // Unshift isi file Metro
//         memcpy(buf, temp_buf, bytes_read);
//     } else {
//         memcpy(buf, temp_buf, bytes_read); // Blackrose, Starter, dan lainnya tanpa transformasi tambahan
//     }

//     free(temp_buf);
//     free(chiho);
//     return bytes_read;
// }

// // FUSE: write
// static int maimai_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int fd = fi->fh;
//     if (lseek(fd, offset, SEEK_SET) == -1) return -errno;

//     char *temp_buf = malloc(size + 16);
//     memcpy(temp_buf, buf, size);
//     size_t processed_len = size;

//     if (strcmp(chiho, "dragon") == 0) {
//         perform_rot13(temp_buf, size);
//     } else if (strcmp(chiho, "heaven") == 0) {
//         encrypt_content(buf, size, temp_buf, &processed_len);
//     } else if (strcmp(chiho, "skystreet") == 0) {
//         processed_len = size * 2;
//         char *compressed = malloc(processed_len);
//         compress_content(buf, size, compressed, &processed_len);
//         memcpy(temp_buf, compressed, processed_len);
//         free(compressed);
//     } else if (strcmp(chiho, "metro") == 0) {
//         shift_content(temp_buf, size, 1); // Shift isi file Metro
//         processed_len = size;
//     } else {
//         processed_len = size; // Blackrose, Starter, dan lainnya tanpa transformasi tambahan
//     }

//     int bytes_written = write(fd, temp_buf, processed_len);
//     free(temp_buf);
//     free(chiho);
//     return bytes_written == -1 ? -errno : bytes_written;
// }

// // FUSE: create
// static int maimai_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     char *dir_path = strdup(real_path);
//     char *last_slash = strrchr(dir_path, '/');
//     if (last_slash) *last_slash = '\0';
//     if (mkdir(dir_path, 0755) == -1 && errno != EEXIST) {
//         free(dir_path);
//         free(chiho);
//         return -errno;
//     }
//     free(dir_path);

//     int fd = open(real_path, O_CREAT | O_WRONLY | O_TRUNC, mode);
//     if (fd == -1) {
//         free(chiho);
//         return -errno;
//     }
//     fi->fh = fd;
//     free(chiho);
//     return 0;
// }

// // FUSE: unlink
// static int maimai_unlink(const char *path) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int result = unlink(real_path);
//     free(chiho);
//     return result == -1 ? -errno : 0;
// }

// // FUSE: utimens
// static int maimai_utimens(const char *path, const struct timespec tv[2]) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int result = utimensat(0, real_path, tv, 0);
//     free(chiho);
//     return result == -1 ? -errno : 0;
// }

// // Struktur operasi FUSE
// static struct fuse_operations maimai_ops = {
//     .getattr  = maimai_getattr,
//     .readdir  = maimai_readdir,
//     .open     = maimai_open,
//     .read     = maimai_read,
//     .write    = maimai_write,
//     .create   = maimai_create,
//     .unlink   = maimai_unlink,
//     .utimens  = maimai_utimens,
// };

// // Fungsi utama dengan pembuatan direktori
// int main(int argc, char *argv[]) {
//     struct stat st;
//     if (stat(BASE_DIR, &st) == -1) {
//         if (mkdir(BASE_DIR, 0755) == -1) {
//             perror("Gagal membuat direktori chiho");
//             return 1;
//         }
//     }

//     for (int i = 0; i < num_chiho - 1; i++) { // Tidak termasuk 7sref
//         char dir_path[MAX_PATH_LEN];
//         snprintf(dir_path, MAX_PATH_LEN, "%s/%s", BASE_DIR, valid_chiho[i]);
//         if (stat(dir_path, &st) == -1) {
//             if (mkdir(dir_path, 0755) == -1) {
//                 perror("Gagal membuat subdirektori chiho");
//                 return 1;
//             }
//         }
//     }

//     if (argc < 2) {
//         fprintf(stderr, "Penggunaan: %s <mountpoint>\n", argv[0]);
//         return 1;
//     }

//     const char *mountpoint = argv[1];
//     if (stat(mountpoint, &st) == -1) {
//         if (mkdir(mountpoint, 0755) == -1) {
//             perror("Gagal membuat direktori mountpoint");
//             return 1;
//         }
//     }

//     return fuse_main(argc, argv, &maimai_ops, NULL);
// }



/*Versi 3*/

// #define FUSE_USE_VERSION 26
// #include <fuse.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <dirent.h>
// #include <errno.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <sys/stat.h>
// #include <openssl/evp.h>
// #include <openssl/rand.h>
// #include <zlib.h>

// #define MAX_PATH_LEN 4096
// #define BASE_DIR "chiho"
// #define MOUNT_DIR "fuse_dir"

// // Kunci AES-256 (32 byte)
// static const unsigned char secret_key[] = "unique_key_12345678901234567890";

// // Daftar chiho yang valid, termasuk 7sref
// static const char *valid_chiho[] = {"starter", "metro", "dragon", "blackrose", "heaven", "skystreet", "7sref"};
// static const int num_chiho = 7;

// // Ekstensi khusus untuk setiap chiho (tidak termasuk 7sref)
// static const char *chiho_ext[] = {".mai", ".ccc", ".rot", ".bin", ".enc", ".gz"};

// // Ekstrak nama chiho dari path
// static char* get_chiho_name(const char *path) {
//     char *chiho = strdup(path + 1); // Lewati karakter '/' awal
//     char *slash = strchr(chiho, '/');
//     if (slash) *slash = '\0';
//     return chiho;
// }

// // Dapatkan indeks chiho
// static int get_chiho_index(const char *chiho) {
//     for (int i = 0; i < num_chiho - 1; i++) { // Tidak termasuk 7sref
//         if (strcmp(chiho, valid_chiho[i]) == 0) return i;
//     }
//     return -1;
// }

// // Bangun path asli berdasarkan chiho dengan ekstensi
// static void construct_real_path(char *real_path, const char *path, const char *chiho) {
//     int chiho_idx = get_chiho_index(chiho);
//     const char *ext = (chiho_idx >= 0) ? chiho_ext[chiho_idx] : "";
//     char *filename = strrchr(path, '/');
//     if (filename) filename++;
//     else filename = (char *)path + 1;

//     snprintf(real_path, MAX_PATH_LEN, "%s/%s/%s%s", BASE_DIR, chiho, filename, ext);
// }

// // Geser isi file untuk Metro Chiho
// static void shift_content(char *data, size_t length, int shift_dir) {
//     for (size_t i = 0; i < length; i++) {
//         data[i] = (data[i] + (shift_dir * (i + 1))) % 256;
//     }
// }

// // ROT13 untuk Dragon Chiho
// static void perform_rot13(char *data, size_t length) {
//     for (size_t i = 0; i < length; i++) {
//         if ((data[i] >= 'A' && data[i] <= 'Z') || (data[i] >= 'a' && data[i] <= 'z')) {
//             char base = (data[i] >= 'A' && data[i] <= 'Z') ? 'A' : 'a';
//             data[i] = (data[i] - base + 13) % 26 + base;
//         }
//     }
// }

// // Enkripsi AES-256-CBC untuk Heaven Chiho
// static int encrypt_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
//     unsigned char iv[16];
//     RAND_bytes(iv, 16);
//     int len, total_len;

//     EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, secret_key, iv);
//     memcpy(output, iv, 16);
//     EVP_EncryptUpdate(ctx, (unsigned char *)output + 16, &len, (unsigned char *)input, in_len);
//     total_len = len;
//     EVP_EncryptFinal_ex(ctx, (unsigned char *)output + 16 + len, &len);
//     total_len += len;
//     *out_len = total_len + 16;
//     EVP_CIPHER_CTX_free(ctx);
//     return 0;
// }

// // Dekripsi AES-256-CBC untuk Heaven Chiho
// static int decrypt_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
//     unsigned char iv[16];
//     memcpy(iv, input, 16);
//     int len, total_len;

//     EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, secret_key, iv);
//     EVP_DecryptUpdate(ctx, (unsigned char *)output, &len, (unsigned char *)input + 16, in_len - 16);
//     total_len = len;
//     EVP_DecryptFinal_ex(ctx, (unsigned char *)output + len, &len);
//     total_len += len;
//     *out_len = total_len;
//     EVP_CIPHER_CTX_free(ctx);
//     return 0;
// }

// // Kompresi gzip untuk Skystreet Chiho
// static int compress_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     z_stream stream = {0};
//     deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
//     stream.next_in = (Bytef *)input;
//     stream.avail_in = in_len;
//     stream.next_out = (Bytef *)output;
//     stream.avail_out = *out_len;
//     deflate(&stream, Z_FINISH);
//     *out_len = stream.total_out;
//     deflateEnd(&stream);
//     return 0;
// }

// // Dekompresi gzip untuk Skystreet Chiho
// static int decompress_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     z_stream stream = {0};
//     inflateInit2(&stream, 15 + 16);
//     stream.next_in = (Bytef *)input;
//     stream.avail_in = in_len;
//     stream.next_out = (Bytef *)output;
//     stream.avail_out = *out_len;
//     inflate(&stream, Z_FINISH);
//     *out_len = stream.total_out;
//     inflateEnd(&stream);
//     return 0;
// }

// // Validasi apakah chiho valid
// static int is_valid_chiho(const char *chiho) {
//     for (int i = 0; i < num_chiho; i++) {
//         if (strcmp(chiho, valid_chiho[i]) == 0) return 1;
//     }
//     return 0;
// }

// // Parse path 7sref: /7sref/[area]_[nama_file] -> /area/nama_file
// static int parse_7sref_path(const char *path, char **target_chiho, char **target_file) {
//     if (strncmp(path, "/7sref/", 7) != 0) return 0;

//     char *filename = (char *)path + 7; // Lewati "/7sref/"
//     char *underscore = strchr(filename, '_');
//     if (!underscore) return 0;

//     *underscore = '\0';
//     *target_chiho = strdup(filename);
//     *target_file = strdup(underscore + 1);

//     if (!is_valid_chiho(*target_chiho) || strcmp(*target_chiho, "7sref") == 0) {
//         free(*target_chiho);
//         free(*target_file);
//         return 0;
//     }

//     *underscore = '_'; // Kembalikan underscore
//     return 1;
// }

// // Bangun path target dari 7sref
// static void construct_7sref_target_path(char *target_path, const char *target_chiho, const char *target_file) {
//     snprintf(target_path, MAX_PATH_LEN, "/%s/%s", target_chiho, target_file);
// }

// // FUSE: getattr
// static int maimai_getattr(const char *path, struct stat *stbuf) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int result = stat(real_path, stbuf);
//     if (result == 0) {
//         free(chiho);
//         return 0;
//     }

//     if (strcmp(path, "/") == 0 || (strlen(path) > 1 && strchr(path + 1, '/') == NULL && is_valid_chiho(chiho))) {
//         memset(stbuf, 0, sizeof(struct stat));
//         stbuf->st_mode = S_IFDIR | 0755;
//         stbuf->st_nlink = 2;
//         free(chiho);
//         return 0;
//     }

//     free(chiho);
//     return -ENOENT;
// }

// // FUSE: readdir
// static int maimai_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);

//     if (strcmp(path, "/") == 0) {
//         filler(buf, ".", NULL, 0);
//         filler(buf, "..", NULL, 0);
//         for (int i = 0; i < num_chiho; i++) {
//             filler(buf, valid_chiho[i], NULL, 0);
//         }
//         free(chiho);
//         return 0;
//     }

//     if (strcmp(chiho, "7sref") == 0) {
//         filler(buf, ".", NULL, 0);
//         filler(buf, "..", NULL, 0);
//         for (int i = 0; i < num_chiho - 1; i++) { // Tidak termasuk 7sref
//             char chiho_path[MAX_PATH_LEN];
//             snprintf(chiho_path, MAX_PATH_LEN, "%s/%s", BASE_DIR, valid_chiho[i]);
//             DIR *dir = opendir(chiho_path);
//             if (!dir) continue;

//             struct dirent *entry;
//             while ((entry = readdir(dir)) != NULL) {
//                 if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

//                 char *display_name = strdup(entry->d_name);
//                 int ext_len = strlen(chiho_ext[i]);
//                 if (strncmp(display_name + strlen(display_name) - ext_len, chiho_ext[i], ext_len) == 0) {
//                     display_name[strlen(display_name) - ext_len] = '\0';
//                 }

//                 char virtual_name[MAX_PATH_LEN];
//                 snprintf(virtual_name, MAX_PATH_LEN, "%s_%s", valid_chiho[i], display_name);
//                 filler(buf, virtual_name, NULL, 0);
//                 free(display_name);
//             }
//             closedir(dir);
//         }
//         free(chiho);
//         return 0;
//     }

//     construct_real_path(real_path, path, chiho);
//     DIR *dir = opendir(real_path);
//     if (!dir) {
//         filler(buf, ".", NULL, 0);
//         filler(buf, "..", NULL, 0);
//         free(chiho);
//         return 0;
//     }

//     struct dirent *entry;
//     while ((entry = readdir(dir)) != NULL) {
//         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

//         char *display_name = strdup(entry->d_name);
//         int chiho_idx = get_chiho_index(chiho);
//         if (chiho_idx >= 0) {
//             int ext_len = strlen(chiho_ext[chiho_idx]);
//             if (strncmp(display_name + strlen(display_name) - ext_len, chiho_ext[chiho_idx], ext_len) == 0) {
//                 display_name[strlen(display_name) - ext_len] = '\0';
//             }
//         }
//         filler(buf, display_name, NULL, 0);
//         free(display_name);
//     }

//     filler(buf, ".", NULL, 0);
//     filler(buf, "..", NULL, 0);
//     closedir(dir);
//     free(chiho);
//     return 0;
// }

// // FUSE: open
// static int maimai_open(const char *path, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int fd = open(real_path, fi->flags);
//     if (fd == -1) {
//         free(chiho);
//         return -errno;
//     }
//     fi->fh = fd;
//     free(chiho);
//     return 0;
// }

// // FUSE: read
// static int maimai_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int fd = fi->fh;
//     if (lseek(fd, offset, SEEK_SET) == -1) return -errno;

//     char *temp_buf = malloc(size + 16);
//     int bytes_read = read(fd, temp_buf, size);
//     if (bytes_read == -1) {
//         free(temp_buf);
//         free(chiho);
//         return -errno;
//     }

//     if (strcmp(chiho, "dragon") == 0) {
//         perform_rot13(temp_buf, bytes_read);
//         memcpy(buf, temp_buf, bytes_read);
//     } else if (strcmp(chiho, "heaven") == 0) {
//         size_t decrypted_len;
//         decrypt_content(temp_buf, bytes_read, buf, &decrypted_len);
//         bytes_read = decrypted_len;
//     } else if (strcmp(chiho, "skystreet") == 0) {
//         size_t decompressed_len = size * 10;
//         char *decompressed = malloc(decompressed_len);
//         decompress_content(temp_buf, bytes_read, decompressed, &decompressed_len);
//         memcpy(buf, decompressed, decompressed_len);
//         bytes_read = decompressed_len;
//         free(decompressed);
//     } else if (strcmp(chiho, "metro") == 0) {
//         shift_content(temp_buf, bytes_read, -1); // Unshift isi file Metro
//         memcpy(buf, temp_buf, bytes_read);
//     } else {
//         memcpy(buf, temp_buf, bytes_read); // Blackrose, Starter, dan lainnya tanpa transformasi tambahan
//     }

//     free(temp_buf);
//     free(chiho);
//     return bytes_read;
// }

// // FUSE: write
// static int maimai_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int fd = fi->fh;
//     if (lseek(fd, offset, SEEK_SET) == -1) return -errno;

//     char *temp_buf = malloc(size + 16);
//     memcpy(temp_buf, buf, size);
//     size_t processed_len = size;

//     if (strcmp(chiho, "dragon") == 0) {
//         perform_rot13(temp_buf, size);
//     } else if (strcmp(chiho, "heaven") == 0) {
//         encrypt_content(buf, size, temp_buf, &processed_len);
//     } else if (strcmp(chiho, "skystreet") == 0) {
//         processed_len = size * 2;
//         char *compressed = malloc(processed_len);
//         compress_content(buf, size, compressed, &processed_len);
//         memcpy(temp_buf, compressed, processed_len);
//         free(compressed);
//     } else if (strcmp(chiho, "metro") == 0) {
//         shift_content(temp_buf, size, 1); // Shift isi file Metro
//         processed_len = size;
//     } else {
//         processed_len = size; // Blackrose, Starter, dan lainnya tanpa transformasi tambahan
//     }

//     int bytes_written = write(fd, temp_buf, processed_len);
//     free(temp_buf);
//     free(chiho);
//     return bytes_written == -1 ? -errno : bytes_written;
// }

// // FUSE: create
// static int maimai_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     char *dir_path = strdup(real_path);
//     char *last_slash = strrchr(dir_path, '/');
//     if (last_slash) *last_slash = '\0';
//     if (mkdir(dir_path, 0755) == -1 && errno != EEXIST) {
//         free(dir_path);
//         free(chiho);
//         return -errno;
//     }
//     free(dir_path);

//     int fd = open(real_path, O_CREAT | O_WRONLY | O_TRUNC, mode);
//     if (fd == -1) {
//         free(chiho);
//         return -errno;
//     }
//     fi->fh = fd;
//     free(chiho);
//     return 0;
// }

// // FUSE: unlink
// static int maimai_unlink(const char *path) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int result = unlink(real_path);
//     free(chiho);
//     return result == -1 ? -errno : 0;
// }

// // FUSE: utimens
// static int maimai_utimens(const char *path, const struct timespec tv[2]) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int result = utimensat(0, real_path, tv, 0);
//     free(chiho);
//     return result == -1 ? -errno : 0;
// }

// // Struktur operasi FUSE
// static struct fuse_operations maimai_ops = {
//     .getattr  = maimai_getattr,
//     .readdir  = maimai_readdir,
//     .open     = maimai_open,
//     .read     = maimai_read,
//     .write    = maimai_write,
//     .create   = maimai_create,
//     .unlink   = maimai_unlink,
//     .utimens  = maimai_utimens,
// };

// // Fungsi utama dengan pembuatan direktori
// int main(int argc, char *argv[]) {
//     struct stat st;
//     if (stat(BASE_DIR, &st) == -1) {
//         if (mkdir(BASE_DIR, 0755) == -1) {
//             perror("Gagal membuat direktori chiho");
//             return 1;
//         }
//     }

//     for (int i = 0; i < num_chiho - 1; i++) { // Tidak termasuk 7sref
//         char dir_path[MAX_PATH_LEN];
//         snprintf(dir_path, MAX_PATH_LEN, "%s/%s", BASE_DIR, valid_chiho[i]);
//         if (stat(dir_path, &st) == -1) {
//             if (mkdir(dir_path, 0755) == -1) {
//                 perror("Gagal membuat subdirektori chiho");
//                 return 1;
//             }
//         }
//     }

//     if (argc < 2) {
//         fprintf(stderr, "Penggunaan: %s <mountpoint>\n", argv[0]);
//         return 1;
//     }

//     const char *mountpoint = argv[1];
//     if (stat(mountpoint, &st) == -1) {
//         if (mkdir(mountpoint, 0755) == -1) {
//             perror("Gagal membuat direktori mountpoint");
//             return 1;
//         }
//     }

//     return fuse_main(argc, argv, &maimai_ops, NULL);
// }



/*Version 4*/

// #define FUSE_USE_VERSION 26
// #include <fuse.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <dirent.h>
// #include <errno.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <sys/stat.h>
// #include <openssl/evp.h>
// #include <openssl/rand.h>
// #include <zlib.h>

// #define MAX_PATH_LEN 4096
// #define BASE_DIR "chiho"
// #define MOUNT_DIR "fuse_dir"

// // Kunci AES-256 (32 byte)
// static const unsigned char secret_key[] = "unique_key_12345678901234567890";

// // Daftar chiho yang valid, termasuk 7sref
// static const char *valid_chiho[] = {"starter", "metro", "dragon", "blackrose", "heaven", "skystreet", "7sref"};
// static const int num_chiho = 7;

// // Ekstensi khusus untuk setiap chiho (tidak termasuk 7sref)
// static const char *chiho_ext[] = {".mai", ".ccc", ".rot", ".bin", ".enc", ".gz"};

// // Ekstrak nama chiho dari path
// static char* get_chiho_name(const char *path) {
//     char *chiho = strdup(path + 1); // Lewati karakter '/' awal
//     char *slash = strchr(chiho, '/');
//     if (slash) *slash = '\0';
//     return chiho;
// }

// // Dapatkan indeks chiho
// static int get_chiho_index(const char *chiho) {
//     for (int i = 0; i < num_chiho - 1; i++) { // Tidak termasuk 7sref
//         if (strcmp(chiho, valid_chiho[i]) == 0) return i;
//     }
//     return -1;
// }

// // Bangun path asli berdasarkan chiho dengan ekstensi
// static void construct_real_path(char *real_path, const char *path, const char *chiho) {
//     int chiho_idx = get_chiho_index(chiho);
//     const char *ext = (chiho_idx >= 0) ? chiho_ext[chiho_idx] : "";
//     char *filename = strrchr(path, '/');
//     if (filename) filename++;
//     else filename = (char *)path + 1;

//     snprintf(real_path, MAX_PATH_LEN, "%s/%s/%s%s", BASE_DIR, chiho, filename, ext);
// }

// // Geser isi file untuk Metro Chiho
// static void shift_content(char *data, size_t length, int shift_dir) {
//     for (size_t i = 0; i < length; i++) {
//         data[i] = (data[i] + (shift_dir * (i + 1))) % 256;
//     }
// }

// // ROT13 untuk Dragon Chiho
// static void perform_rot13(char *data, size_t length) {
//     for (size_t i = 0; i < length; i++) {
//         if ((data[i] >= 'A' && data[i] <= 'Z') || (data[i] >= 'a' && data[i] <= 'z')) {
//             char base = (data[i] >= 'A' && data[i] <= 'Z') ? 'A' : 'a';
//             data[i] = (data[i] - base + 13) % 26 + base;
//         }
//     }
// }

// // Enkripsi AES-256-CBC untuk Heaven Chiho
// static int encrypt_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
//     if (!ctx) return -ENOMEM;
//     unsigned char iv[16];
//     RAND_bytes(iv, 16);
//     int len, total_len;

//     if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, secret_key, iv) != 1) {
//         EVP_CIPHER_CTX_free(ctx);
//         return -EIO;
//     }
//     memcpy(output, iv, 16);
//     if (EVP_EncryptUpdate(ctx, (unsigned char *)output + 16, &len, (unsigned char *)input, in_len) != 1) {
//         EVP_CIPHER_CTX_free(ctx);
//         return -EIO;
//     }
//     total_len = len;
//     if (EVP_EncryptFinal_ex(ctx, (unsigned char *)output + 16 + len, &len) != 1) {
//         EVP_CIPHER_CTX_free(ctx);
//         return -EIO;
//     }
//     total_len += len;
//     *out_len = total_len + 16;
//     EVP_CIPHER_CTX_free(ctx);
//     return 0;
// }

// // Dekripsi AES-256-CBC untuk Heaven Chiho
// static int decrypt_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
//     if (!ctx) return -ENOMEM;
//     unsigned char iv[16];
//     memcpy(iv, input, 16);
//     int len, total_len;

//     if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, secret_key, iv) != 1) {
//         EVP_CIPHER_CTX_free(ctx);
//         return -EIO;
//     }
//     if (EVP_DecryptUpdate(ctx, (unsigned char *)output, &len, (unsigned char *)input + 16, in_len - 16) != 1) {
//         EVP_CIPHER_CTX_free(ctx);
//         return -EIO;
//     }
//     total_len = len;
//     if (EVP_DecryptFinal_ex(ctx, (unsigned char *)output + len, &len) != 1) {
//         EVP_CIPHER_CTX_free(ctx);
//         return -EIO;
//     }
//     total_len += len;
//     *out_len = total_len;
//     EVP_CIPHER_CTX_free(ctx);
//     return 0;
// }

// // Kompresi gzip untuk Skystreet Chiho
// static int compress_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     z_stream stream = {0};
//     if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
//         return -EIO;
//     }
//     stream.next_in = (Bytef *)input;
//     stream.avail_in = in_len;
//     stream.next_out = (Bytef *)output;
//     stream.avail_out = *out_len;
//     if (deflate(&stream, Z_FINISH) != Z_STREAM_END) {
//         deflateEnd(&stream);
//         return -EIO;
//     }
//     *out_len = stream.total_out;
//     deflateEnd(&stream);
//     return 0;
// }

// // Dekompresi gzip untuk Skystreet Chiho
// static int decompress_content(const char *input, size_t in_len, char *output, size_t *out_len) {
//     z_stream stream = {0};
//     if (inflateInit2(&stream, 15 + 16) != Z_OK) {
//         return -EIO;
//     }
//     stream.next_in = (Bytef *)input;
//     stream.avail_in = in_len;
//     stream.next_out = (Bytef *)output;
//     stream.avail_out = *out_len;
//     if (inflate(&stream, Z_FINISH) != Z_STREAM_END) {
//         inflateEnd(&stream);
//         return -EIO;
//     }
//     *out_len = stream.total_out;
//     inflateEnd(&stream);
//     return 0;
// }

// // Validasi apakah chiho valid
// static int is_valid_chiho(const char *chiho) {
//     for (int i = 0; i < num_chiho; i++) {
//         if (strcmp(chiho, valid_chiho[i]) == 0) return 1;
//     }
//     return 0;
// }

// // Parse path 7sref: /7sref/[area]_[nama_file] -> /area/nama_file
// static int parse_7sref_path(const char *path, char **target_chiho, char **target_file) {
//     if (strncmp(path, "/7sref/", 7) != 0) return 0;

//     char *filename = (char *)path + 7; // Lewati "/7sref/"
//     char *underscore = strchr(filename, '_');
//     if (!underscore) return 0;

//     *underscore = '\0';
//     *target_chiho = strdup(filename);
//     *target_file = strdup(underscore + 1);

//     if (!is_valid_chiho(*target_chiho) || strcmp(*target_chiho, "7sref") == 0) {
//         free(*target_chiho);
//         free(*target_file);
//         return 0;
//     }

//     *underscore = '_'; // Kembalikan underscore
//     return 1;
// }

// // Bangun path target dari 7sref
// static void construct_7sref_target_path(char *target_path, const char *target_chiho, const char *target_file) {
//     snprintf(target_path, MAX_PATH_LEN, "/%s/%s", target_chiho, target_file);
// }

// // FUSE: getattr
// static int maimai_getattr(const char *path, struct stat *stbuf) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int result = stat(real_path, stbuf);
//     if (result == 0) {
//         free(chiho);
//         return 0;
//     }

//     if (strcmp(path, "/") == 0 || (strlen(path) > 1 && strchr(path + 1, '/') == NULL && is_valid_chiho(chiho))) {
//         memset(stbuf, 0, sizeof(struct stat));
//         stbuf->st_mode = S_IFDIR | 0755;
//         stbuf->st_nlink = 2;
//         free(chiho);
//         return 0;
//     }

//     free(chiho);
//     return -ENOENT;
// }

// // FUSE: readdir
// static int maimai_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);

//     if (strcmp(path, "/") == 0) {
//         filler(buf, ".", NULL, 0);
//         filler(buf, "..", NULL, 0);
//         for (int i = 0; i < num_chiho; i++) {
//             filler(buf, valid_chiho[i], NULL, 0);
//         }
//         free(chiho);
//         return 0;
//     }

//     if (strcmp(chiho, "7sref") == 0) {
//         filler(buf, ".", NULL, 0);
//         filler(buf, "..", NULL, 0);
//         for (int i = 0; i < num_chiho - 1; i++) { // Tidak termasuk 7sref
//             char chiho_path[MAX_PATH_LEN];
//             snprintf(chiho_path, MAX_PATH_LEN, "%s/%s", BASE_DIR, valid_chiho[i]);
//             DIR *dir = opendir(chiho_path);
//             if (!dir) continue;

//             struct dirent *entry;
//             while ((entry = readdir(dir)) != NULL) {
//                 if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

//                 char *display_name = strdup(entry->d_name);
//                 int ext_len = strlen(chiho_ext[i]);
//                 if (strncmp(display_name + strlen(display_name) - ext_len, chiho_ext[i], ext_len) == 0) {
//                     display_name[strlen(display_name) - ext_len] = '\0';
//                 }

//                 char virtual_name[MAX_PATH_LEN];
//                 snprintf(virtual_name, MAX_PATH_LEN, "%s_%s", valid_chiho[i], display_name);
//                 filler(buf, virtual_name, NULL, 0);
//                 free(display_name);
//             }
//             closedir(dir);
//         }
//         free(chiho);
//         return 0;
//     }

//     construct_real_path(real_path, path, chiho);
//     DIR *dir = opendir(real_path);
//     if (!dir) {
//         filler(buf, ".", NULL, 0);
//         filler(buf, "..", NULL, 0);
//         free(chiho);
//         return 0;
//     }

//     struct dirent *entry;
//     while ((entry = readdir(dir)) != NULL) {
//         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

//         char *display_name = strdup(entry->d_name);
//         int chiho_idx = get_chiho_index(chiho);
//         if (chiho_idx >= 0) {
//             int ext_len = strlen(chiho_ext[chiho_idx]);
//             if (strncmp(display_name + strlen(display_name) - ext_len, chiho_ext[chiho_idx], ext_len) == 0) {
//                 display_name[strlen(display_name) - ext_len] = '\0';
//             }
//         }
//         filler(buf, display_name, NULL, 0);
//         free(display_name);
//     }

//     filler(buf, ".", NULL, 0);
//     filler(buf, "..", NULL, 0);
//     closedir(dir);
//     free(chiho);
//     return 0;
// }

// // FUSE: open
// static int maimai_open(const char *path, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int fd = open(real_path, fi->flags);
//     if (fd == -1) {
//         free(chiho);
//         return -errno;
//     }
//     fi->fh = fd;
//     free(chiho);
//     return 0;
// }

// // FUSE: read
// static int maimai_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int fd = fi->fh;
//     if (lseek(fd, offset, SEEK_SET) == -1) {
//         free(chiho);
//         return -errno;
//     }

//     char *temp_buf = malloc(size + 16);
//     if (!temp_buf) {
//         free(chiho);
//         return -ENOMEM;
//     }
//     int bytes_read = read(fd, temp_buf, size);
//     if (bytes_read == -1) {
//         free(temp_buf);
//         free(chiho);
//         return -errno;
//     }

//     if (strcmp(chiho, "dragon") == 0) {
//         perform_rot13(temp_buf, bytes_read);
//         memcpy(buf, temp_buf, bytes_read > size ? size : bytes_read);
//     } else if (strcmp(chiho, "heaven") == 0) {
//         size_t decrypted_len;
//         if (decrypt_content(temp_buf, bytes_read, buf, &decrypted_len) != 0) {
//             free(temp_buf);
//             free(chiho);
//             return -EIO;
//         }
//         bytes_read = decrypted_len > size ? size : decrypted_len;
//     } else if (strcmp(chiho, "skystreet") == 0) {
//         size_t decompressed_len = size * 2;
//         char *decompressed = malloc(decompressed_len);
//         if (!decompressed) {
//             free(temp_buf);
//             free(chiho);
//             return -ENOMEM;
//         }
//         if (decompress_content(temp_buf, bytes_read, decompressed, &decompressed_len) != 0) {
//             free(temp_buf);
//             free(decompressed);
//             free(chiho);
//             return -EIO;
//         }
//         bytes_read = decompressed_len > size ? size : decompressed_len;
//         memcpy(buf, decompressed, bytes_read);
//         free(decompressed);
//     } else if (strcmp(chiho, "metro") == 0) {
//         shift_content(temp_buf, bytes_read, -1);
//         memcpy(buf, temp_buf, bytes_read > size ? size : bytes_read);
//     } else {
//         memcpy(buf, temp_buf, bytes_read > size ? size : bytes_read);
//     }

//     free(temp_buf);
//     free(chiho);
//     return bytes_read;
// }

// // FUSE: write
// static int maimai_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int fd = fi->fh;
//     if (lseek(fd, offset, SEEK_SET) == -1) {
//         free(chiho);
//         return -errno;
//     }

//     size_t temp_buf_size = size;
//     if (strcmp(chiho, "heaven") == 0) {
//         temp_buf_size = size + 32; // Untuk IV dan padding
//     } else if (strcmp(chiho, "skystreet") == 0) {
//         temp_buf_size = size * 2; // Untuk kompresi
//     }
//     char *temp_buf = malloc(temp_buf_size);
//     if (!temp_buf) {
//         free(chiho);
//         return -ENOMEM;
//     }

//     size_t processed_len = size;
//     if (strcmp(chiho, "dragon") == 0) {
//         memcpy(temp_buf, buf, size);
//         perform_rot13(temp_buf, size);
//     } else if (strcmp(chiho, "heaven") == 0) {
//         if (encrypt_content(buf, size, temp_buf, &processed_len) != 0) {
//             free(temp_buf);
//             free(chiho);
//             return -EIO;
//         }
//     } else if (strcmp(chiho, "skystreet") == 0) {
//         processed_len = temp_buf_size;
//         if (compress_content(buf, size, temp_buf, &processed_len) != 0) {
//             free(temp_buf);
//             free(chiho);
//             return -EIO;
//         }
//     } else if (strcmp(chiho, "metro") == 0) {
//         memcpy(temp_buf, buf, size);
//         shift_content(temp_buf, size, 1);
//     } else {
//         memcpy(temp_buf, buf, size);
//     }

//     int bytes_written = write(fd, temp_buf, processed_len);
//     free(temp_buf);
//     free(chiho);
//     return bytes_written == -1 ? -errno : size; // Kembalikan size asli, bukan processed_len
// }

// // FUSE: create
// static int maimai_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     char *dir_path = strdup(real_path);
//     char *last_slash = strrchr(dir_path, '/');
//     if (last_slash) *last_slash = '\0';
//     if (mkdir(dir_path, 0755) == -1 && errno != EEXIST) {
//         free(dir_path);
//         free(chiho);
//         return -errno;
//     }
//     free(dir_path);

//     int fd = open(real_path, O_CREAT | O_WRONLY | O_TRUNC, mode);
//     if (fd == -1) {
//         free(chiho);
//         return -errno;
//     }
//     fi->fh = fd;
//     free(chiho);
//     return 0;
// }

// // FUSE: unlink
// static int maimai_unlink(const char *path) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int result = unlink(real_path);
//     free(chiho);
//     return result == -1 ? -errno : 0;
// }

// // FUSE: utimens
// static int maimai_utimens(const char *path, const struct timespec tv[2]) {
//     char real_path[MAX_PATH_LEN];
//     char *chiho = get_chiho_name(path);
//     char *target_chiho = NULL, *target_file = NULL;

//     if (parse_7sref_path(path, &target_chiho, &target_file)) {
//         char target_path[MAX_PATH_LEN];
//         construct_7sref_target_path(target_path, target_chiho, target_file);
//         free(chiho);
//         chiho = target_chiho;
//         construct_real_path(real_path, target_path, chiho);
//         free(target_file);
//     } else {
//         construct_real_path(real_path, path, chiho);
//     }

//     int result = utimensat(0, real_path, tv, 0);
//     free(chiho);
//     return result == -1 ? -errno : 0;
// }

// // Struktur operasi FUSE
// static struct fuse_operations maimai_ops = {
//     .getattr  = maimai_getattr,
//     .readdir  = maimai_readdir,
//     .open     = maimai_open,
//     .read     = maimai_read,
//     .write    = maimai_write,
//     .create   = maimai_create,
//     .unlink   = maimai_unlink,
//     .utimens  = maimai_utimens,
// };

// // Fungsi utama dengan pembuatan direktori
// int main(int argc, char *argv[]) {
//     struct stat st;
//     if (stat(BASE_DIR, &st) == -1) {
//         if (mkdir(BASE_DIR, 0755) == -1) {
//             perror("Gagal membuat direktori chiho");
//             return 1;
//         }
//     }

//     for (int i = 0; i < num_chiho - 1; i++) { // Tidak termasuk 7sref
//         char dir_path[MAX_PATH_LEN];
//         snprintf(dir_path, MAX_PATH_LEN, "%s/%s", BASE_DIR, valid_chiho[i]);
//         if (stat(dir_path, &st) == -1) {
//             if (mkdir(dir_path, 0755) == -1) {
//                 perror("Gagal membuat subdirektori chiho");
//                 return 1;
//             }
//         }
//     }

//     if (argc < 2) {
//         fprintf(stderr, "Penggunaan: %s <mountpoint>\n", argv[0]);
//         return 1;
//     }

//     const char *mountpoint = argv[1];
//     if (stat(mountpoint, &st) == -1) {
//         if (mkdir(mountpoint, 0755) == -1) {
//             perror("Gagal membuat direktori mountpoint");
//             return 1;
//         }
//     }

//     return fuse_main(argc, argv, &maimai_ops, NULL);
// }





#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <zlib.h>

#define MAX_PATH_LEN 4096
#define BASE_DIR "chiho"
#define MOUNT_DIR "fuse_dir"

// Kunci AES-256 (32 byte)
static const unsigned char secret_key[] = "unique_key_12345678901234567890";

// Daftar chiho yang valid, termasuk 7sref
static const char *valid_chiho[] = {"starter", "metro", "dragon", "blackrose", "heaven", "skystreet", "7sref"};
static const int num_chiho = 7;

// Ekstensi khusus untuk setiap chiho (tidak termasuk 7sref)
static const char *chiho_ext[] = {".mai", ".ccc", ".rot", ".bin", ".enc", ".gz"};

// Ekstrak nama chiho dari path
static char* get_chiho_name(const char *path) {
    char *chiho = strdup(path + 1); // Lewati karakter '/' awal
    char *slash = strchr(chiho, '/');
    if (slash) *slash = '\0';
    return chiho;
}

// Dapatkan indeks chiho
static int get_chiho_index(const char *chiho) {
    for (int i = 0; i < num_chiho - 1; i++) { // Tidak termasuk 7sref
        if (strcmp(chiho, valid_chiho[i]) == 0) return i;
    }
    return -1;
}

// Bangun path asli berdasarkan chiho dengan ekstensi
static void construct_real_path(char *real_path, const char *path, const char *chiho) {
    int chiho_idx = get_chiho_index(chiho);
    const char *ext = (chiho_idx >= 0) ? chiho_ext[chiho_idx] : "";
    char *filename = strrchr(path, '/');
    if (filename) filename++;
    else filename = (char *)path + 1;

    snprintf(real_path, MAX_PATH_LEN, "%s/%s/%s%s", BASE_DIR, chiho, filename, ext);
}

// Bangun path direktori asli untuk readdir
static void construct_real_dir_path(char *real_path, const char *chiho) {
    snprintf(real_path, MAX_PATH_LEN, "%s/%s", BASE_DIR, chiho);
}

// Geser isi file untuk Metro Chiho
static void shift_content(char *data, size_t length, int shift_dir) {
    for (size_t i = 0; i < length; i++) {
        data[i] = (data[i] + (shift_dir * (i + 1))) % 256;
    }
}

// ROT13 untuk Dragon Chiho
static void perform_rot13(char *data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        if ((data[i] >= 'A' && data[i] <= 'Z') || (data[i] >= 'a' && data[i] <= 'z')) {
            char base = (data[i] >= 'A' && data[i] <= 'Z') ? 'A' : 'a';
            data[i] = (data[i] - base + 13) % 26 + base;
        }
    }
}

// Enkripsi AES-256-CBC untuk Heaven Chiho
static int encrypt_content(const char *input, size_t in_len, char *output, size_t *out_len) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -ENOMEM;
    unsigned char iv[16];
    RAND_bytes(iv, 16);
    int len, total_len;

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, secret_key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -EIO;
    }
    memcpy(output, iv, 16);
    if (EVP_EncryptUpdate(ctx, (unsigned char *)output + 16, &len, (unsigned char *)input, in_len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -EIO;
    }
    total_len = len;
    if (EVP_EncryptFinal_ex(ctx, (unsigned char *)output + 16 + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -EIO;
    }
    total_len += len;
    *out_len = total_len + 16;
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

// Dekripsi AES-256-CBC untuk Heaven Chiho
static int decrypt_content(const char *input, size_t in_len, char *output, size_t *out_len) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -ENOMEM;
    unsigned char iv[16];
    memcpy(iv, input, 16);
    int len, total_len;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, secret_key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -EIO;
    }
    if (EVP_DecryptUpdate(ctx, (unsigned char *)output, &len, (unsigned char *)input + 16, in_len - 16) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -EIO;
    }
    total_len = len;
    if (EVP_DecryptFinal_ex(ctx, (unsigned char *)output + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return -EIO;
    }
    total_len += len;
    *out_len = total_len;
    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

// Kompresi gzip untuk Skystreet Chiho
static int compress_content(const char *input, size_t in_len, char *output, size_t *out_len) {
    z_stream stream = {0};
    if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return -EIO;
    }
    stream.next_in = (Bytef *)input;
    stream.avail_in = in_len;
    stream.next_out = (Bytef *)output;
    stream.avail_out = *out_len;
    if (deflate(&stream, Z_FINISH) != Z_STREAM_END) {
        deflateEnd(&stream);
        return -EIO;
    }
    *out_len = stream.total_out;
    deflateEnd(&stream);
    return 0;
}

// Dekompresi gzip untuk Skystreet Chiho
static int decompress_content(const char *input, size_t in_len, char *output, size_t *out_len) {
    z_stream stream = {0};
    if (inflateInit2(&stream, 15 + 16) != Z_OK) {
        return -EIO;
    }
    stream.next_in = (Bytef *)input;
    stream.avail_in = in_len;
    stream.next_out = (Bytef *)output;
    stream.avail_out = *out_len;
    if (inflate(&stream, Z_FINISH) != Z_STREAM_END) {
        inflateEnd(&stream);
        return -EIO;
    }
    *out_len = stream.total_out;
    inflateEnd(&stream);
    return 0;
}

// Validasi apakah chiho valid
static int is_valid_chiho(const char *chiho) {
    for (int i = 0; i < num_chiho; i++) {
        if (strcmp(chiho, valid_chiho[i]) == 0) return 1;
    }
    return 0;
}

// Parse path 7sref: /7sref/[area]_[nama_file] -> /area/nama_file
static int parse_7sref_path(const char *path, char **target_chiho, char **target_file) {
    if (strncmp(path, "/7sref/", 7) != 0) return 0;

    char *filename = (char *)path + 7; // Lewati "/7sref/"
    char *underscore = strchr(filename, '_');
    if (!underscore) return 0;

    *underscore = '\0';
    *target_chiho = strdup(filename);
    *target_file = strdup(underscore + 1);

    if (!is_valid_chiho(*target_chiho) || strcmp(*target_chiho, "7sref") == 0) {
        free(*target_chiho);
        free(*target_file);
        return 0;
    }

    *underscore = '_'; // Kembalikan underscore
    return 1;
}

// Bangun path target dari 7sref
static void construct_7sref_target_path(char *target_path, const char *target_chiho, const char *target_file) {
    snprintf(target_path, MAX_PATH_LEN, "/%s/%s", target_chiho, target_file);
}

// FUSE: getattr
static int maimai_getattr(const char *path, struct stat *stbuf) {
    char real_path[MAX_PATH_LEN];
    char *chiho = get_chiho_name(path);
    char *target_chiho = NULL, *target_file = NULL;

    if (parse_7sref_path(path, &target_chiho, &target_file)) {
        char target_path[MAX_PATH_LEN];
        construct_7sref_target_path(target_path, target_chiho, target_file);
        free(chiho);
        chiho = target_chiho;
        construct_real_path(real_path, target_path, chiho);
        free(target_file);
    } else {
        construct_real_path(real_path, path, chiho);
    }

    int result = stat(real_path, stbuf);
    if (result == 0) {
        free(chiho);
        return 0;
    }

    if (strcmp(path, "/") == 0 || (strlen(path) > 1 && strchr(path + 1, '/') == NULL && is_valid_chiho(chiho))) {
        memset(stbuf, 0, sizeof(struct stat));
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        free(chiho);
        return 0;
    }

    free(chiho);
    return -ENOENT;
}

// FUSE: readdir
static int maimai_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    char real_path[MAX_PATH_LEN];
    char *chiho = get_chiho_name(path);

    if (strcmp(path, "/") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        for (int i = 0; i < num_chiho; i++) {
            filler(buf, valid_chiho[i], NULL, 0);
        }
        free(chiho);
        return 0;
    }

    if (strcmp(chiho, "7sref") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        for (int i = 0; i < num_chiho - 1; i++) { // Tidak termasuk 7sref
            char chiho_path[MAX_PATH_LEN];
            snprintf(chiho_path, MAX_PATH_LEN, "%s/%s", BASE_DIR, valid_chiho[i]);
            DIR *dir = opendir(chiho_path);
            if (!dir) {
                fprintf(stderr, "Gagal membuka direktori %s: %s\n", chiho_path, strerror(errno));
                continue;
            }

            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

                char *display_name = strdup(entry->d_name);
                int ext_len = strlen(chiho_ext[i]);
                if (strncmp(display_name + strlen(display_name) - ext_len, chiho_ext[i], ext_len) == 0) {
                    display_name[strlen(display_name) - ext_len] = '\0';
                }

                char virtual_name[MAX_PATH_LEN];
                snprintf(virtual_name, MAX_PATH_LEN, "%s_%s", valid_chiho[i], display_name);
                filler(buf, virtual_name, NULL, 0);
                free(display_name);
            }
            closedir(dir);
        }
        free(chiho);
        return 0;
    }

    // Untuk chiho lain, baca direktori fisik di chiho/[chiho]
    construct_real_dir_path(real_path, chiho);
    DIR *dir = opendir(real_path);
    if (!dir) {
        fprintf(stderr, "Gagal membuka direktori %s: %s\n", real_path, strerror(errno));
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        free(chiho);
        return 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char *display_name = strdup(entry->d_name);
        int chiho_idx = get_chiho_index(chiho);
        if (chiho_idx >= 0) {
            int ext_len = strlen(chiho_ext[chiho_idx]);
            if (strncmp(display_name + strlen(display_name) - ext_len, chiho_ext[chiho_idx], ext_len) == 0) {
                display_name[strlen(display_name) - ext_len] = '\0';
            }
        }
        filler(buf, display_name, NULL, 0);
        free(display_name);
    }

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    closedir(dir);
    free(chiho);
    return 0;
}

// FUSE: open
static int maimai_open(const char *path, struct fuse_file_info *fi) {
    char real_path[MAX_PATH_LEN];
    char *chiho = get_chiho_name(path);
    char *target_chiho = NULL, *target_file = NULL;

    if (parse_7sref_path(path, &target_chiho, &target_file)) {
        char target_path[MAX_PATH_LEN];
        construct_7sref_target_path(target_path, target_chiho, target_file);
        free(chiho);
        chiho = target_chiho;
        construct_real_path(real_path, target_path, chiho);
        free(target_file);
    } else {
        construct_real_path(real_path, path, chiho);
    }

    int fd = open(real_path, fi->flags);
    if (fd == -1) {
        fprintf(stderr, "Gagal membuka file %s: %s\n", real_path, strerror(errno));
        free(chiho);
        return -errno;
    }
    fi->fh = fd;
    free(chiho);
    return 0;
}

// FUSE: read
static int maimai_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char real_path[MAX_PATH_LEN];
    char *chiho = get_chiho_name(path);
    char *target_chiho = NULL, *target_file = NULL;

    if (parse_7sref_path(path, &target_chiho, &target_file)) {
        char target_path[MAX_PATH_LEN];
        construct_7sref_target_path(target_path, target_chiho, target_file);
        free(chiho);
        chiho = target_chiho;
        construct_real_path(real_path, target_path, chiho);
        free(target_file);
    } else {
        construct_real_path(real_path, path, chiho);
    }

    int fd = fi->fh;
    if (lseek(fd, offset, SEEK_SET) == -1) {
        free(chiho);
        return -errno;
    }

    char *temp_buf = malloc(size + 16);
    if (!temp_buf) {
        free(chiho);
        return -ENOMEM;
    }
    int bytes_read = read(fd, temp_buf, size);
    if (bytes_read == -1) {
        free(temp_buf);
        free(chiho);
        return -errno;
    }

    if (strcmp(chiho, "dragon") == 0) {
        perform_rot13(temp_buf, bytes_read);
        memcpy(buf, temp_buf, bytes_read > size ? size : bytes_read);
    } else if (strcmp(chiho, "heaven") == 0) {
        size_t decrypted_len;
        if (decrypt_content(temp_buf, bytes_read, buf, &decrypted_len) != 0) {
            free(temp_buf);
            free(chiho);
            return -EIO;
        }
        bytes_read = decrypted_len > size ? size : decrypted_len;
    } else if (strcmp(chiho, "skystreet") == 0) {
        size_t decompressed_len = size * 2;
        char *decompressed = malloc(decompressed_len);
        if (!decompressed) {
            free(temp_buf);
            free(chiho);
            return -ENOMEM;
        }
        if (decompress_content(temp_buf, bytes_read, decompressed, &decompressed_len) != 0) {
            free(temp_buf);
            free(decompressed);
            free(chiho);
            return -EIO;
        }
        bytes_read = decompressed_len > size ? size : decompressed_len;
        memcpy(buf, decompressed, bytes_read);
        free(decompressed);
    } else if (strcmp(chiho, "metro") == 0) {
        shift_content(temp_buf, bytes_read, -1);
        memcpy(buf, temp_buf, bytes_read > size ? size : bytes_read);
    } else {
        memcpy(buf, temp_buf, bytes_read > size ? size : bytes_read);
    }

    free(temp_buf);
    free(chiho);
    return bytes_read;
}

// FUSE: write
static int maimai_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    char real_path[MAX_PATH_LEN];
    char *chiho = get_chiho_name(path);
    char *target_chiho = NULL, *target_file = NULL;

    if (parse_7sref_path(path, &target_chiho, &target_file)) {
        char target_path[MAX_PATH_LEN];
        construct_7sref_target_path(target_path, target_chiho, target_file);
        free(chiho);
        chiho = target_chiho;
        construct_real_path(real_path, target_path, chiho);
        free(target_file);
    } else {
        construct_real_path(real_path, path, chiho);
    }

    int fd = fi->fh;
    if (lseek(fd, offset, SEEK_SET) == -1) {
        free(chiho);
        return -errno;
    }

    size_t temp_buf_size = size;
    if (strcmp(chiho, "heaven") == 0) {
        temp_buf_size = size + 32; // Untuk IV dan padding
    } else if (strcmp(chiho, "skystreet") == 0) {
        temp_buf_size = size * 2; // Untuk kompresi
    }
    char *temp_buf = malloc(temp_buf_size);
    if (!temp_buf) {
        free(chiho);
        return -ENOMEM;
    }

    size_t processed_len = size;
    if (strcmp(chiho, "dragon") == 0) {
        memcpy(temp_buf, buf, size);
        perform_rot13(temp_buf, size);
    } else if (strcmp(chiho, "heaven") == 0) {
        if (encrypt_content(buf, size, temp_buf, &processed_len) != 0) {
            free(temp_buf);
            free(chiho);
            return -EIO;
        }
    } else if (strcmp(chiho, "skystreet") == 0) {
        processed_len = temp_buf_size;
        if (compress_content(buf, size, temp_buf, &processed_len) != 0) {
            free(temp_buf);
            free(chiho);
            return -EIO;
        }
    } else if (strcmp(chiho, "metro") == 0) {
        memcpy(temp_buf, buf, size);
        shift_content(temp_buf, size, 1);
    } else {
        memcpy(temp_buf, buf, size);
    }

    int bytes_written = write(fd, temp_buf, processed_len);
    free(temp_buf);
    free(chiho);
    return bytes_written == -1 ? -errno : size; // Kembalikan size asli
}

// FUSE: create
static int maimai_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char real_path[MAX_PATH_LEN];
    char *chiho = get_chiho_name(path);
    char *target_chiho = NULL, *target_file = NULL;

    if (parse_7sref_path(path, &target_chiho, &target_file)) {
        char target_path[MAX_PATH_LEN];
        construct_7sref_target_path(target_path, target_chiho, target_file);
        free(chiho);
        chiho = target_chiho;
        construct_real_path(real_path, target_path, chiho);
        free(target_file);
    } else {
        construct_real_path(real_path, path, chiho);
    }

    char *dir_path = strdup(real_path);
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash) *last_slash = '\0';
    if (mkdir(dir_path, 0755) == -1 && errno != EEXIST) {
        free(dir_path);
        free(chiho);
        return -errno;
    }
    free(dir_path);

    int fd = open(real_path, O_CREAT | O_WRONLY | O_TRUNC, mode);
    if (fd == -1) {
        fprintf(stderr, "Gagal membuat file %s: %s\n", real_path, strerror(errno));
        free(chiho);
        return -errno;
    }
    fi->fh = fd;
    free(chiho);
    return 0;
}

// FUSE: unlink
static int maimai_unlink(const char *path) {
    char real_path[MAX_PATH_LEN];
    char *chiho = get_chiho_name(path);
    char *target_chiho = NULL, *target_file = NULL;

    if (parse_7sref_path(path, &target_chiho, &target_file)) {
        char target_path[MAX_PATH_LEN];
        construct_7sref_target_path(target_path, target_chiho, target_file);
        free(chiho);
        chiho = target_chiho;
        construct_real_path(real_path, target_path, chiho);
        free(target_file);
    } else {
        construct_real_path(real_path, path, chiho);
    }

    int result = unlink(real_path);
    free(chiho);
    return result == -1 ? -errno : 0;
}

// FUSE: utimens
static int maimai_utimens(const char *path, const struct timespec tv[2]) {
    char real_path[MAX_PATH_LEN];
    char *chiho = get_chiho_name(path);
    char *target_chiho = NULL, *target_file = NULL;

    if (parse_7sref_path(path, &target_chiho, &target_file)) {
        char target_path[MAX_PATH_LEN];
        construct_7sref_target_path(target_path, target_chiho, target_file);
        free(chiho);
        chiho = target_chiho;
        construct_real_path(real_path, target_path, chiho);
        free(target_file);
    } else {
        construct_real_path(real_path, path, chiho);
    }

    int result = utimensat(0, real_path, tv, 0);
    free(chiho);
    return result == -1 ? -errno : 0;
}

// Struktur operasi FUSE
static struct fuse_operations maimai_ops = {
    .getattr  = maimai_getattr,
    .readdir  = maimai_readdir,
    .open     = maimai_open,
    .read     = maimai_read,
    .write    = maimai_write,
    .create   = maimai_create,
    .unlink   = maimai_unlink,
    .utimens  = maimai_utimens,
};

// Fungsi utama dengan pembuatan direktori
int main(int argc, char *argv[]) {
    struct stat st;
    if (stat(BASE_DIR, &st) == -1) {
        if (mkdir(BASE_DIR, 0755) == -1) {
            perror("Gagal membuat direktori chiho");
            return 1;
        }
    }

    for (int i = 0; i < num_chiho - 1; i++) { // Tidak termasuk 7sref
        char dir_path[MAX_PATH_LEN];
        snprintf(dir_path, MAX_PATH_LEN, "%s/%s", BASE_DIR, valid_chiho[i]);
        if (stat(dir_path, &st) == -1) {
            if (mkdir(dir_path, 0755) == -1) {
                perror("Gagal membuat subdirektori chiho");
                return 1;
            }
        }
    }

    if (argc < 2) {
        fprintf(stderr, "Penggunaan: %s <mountpoint>\n", argv[0]);
        return 1;
    }

    const char *mountpoint = argv[1];
    if (stat(mountpoint, &st) == -1) {
        if (mkdir(mountpoint, 0755) == -1) {
            perror("Gagal membuat direktori mountpoint");
            return 1;
        }
    }

    return fuse_main(argc, argv, &maimai_ops, NULL);
}