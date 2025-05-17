# LAPORAN PRAKTIKUM MODUL-4 #
## Praktikum Sistem Operasi Kelompok IT16 ##

| Nama | NRP       |
|-------|-----------|
| Ahmad Syauqi Reza | 5027241085   |
| Mochammad Atha Tajuddin   | 5027241093  |
| Tiara Fatimah Azzahra   | 5027241090  |
---


### Soal_4 ###
Dalam soal ini intinya praktikan diminta untuk membuat kode program yang menggunakan konsep fuse file system(filesystem in userspace)

### Definisi singkat ###
-Fuse atau Filesystem in userspace ialah sebuah kerangka kerja(framework) yang dapat memungkinkan user dengan akses non-root users untuk membuat sistem berkas (filesystem), tanpa harus menulis kode kernel. Fuse bekerja seperti memanipulasi tampilan atau struktur direktori/berkas yang sudah ada, dan membuatnya seolah-olah menjadi filesystem baru.

### Implementasi dari Fuse pada dunia nyata ##
### SSHFS (SSH Filesystem) ###
➜ Akses file di server remote via SSH, tapi tampil seperti direktori lokal.
    Cocok untuk DevOps atau backup file server secara aman.

### EncFS / gocryptfs ###
➜ Membuat direktori terenkripsi yang hanya bisa diakses jika mount dengan password.
    Cocok untuk menyimpan data sensitif di cloud tanpa adanya potensi risiko.

### Sandboxing System ###
➜ Membuat lingkungan eksperimen tanpa menyentuh file sistem asli.
    kegunaanya : “menguji sistem tanpa risiko menghancurkan environment utama.”

### Filesystem virtual ###
➜ Contoh: GitHub membuat repositori Git terlihat seperti filesystem, padahal semua file disajikan via struktur virtual dari metadata Git.

### Requirement ###
```
sudo apt-get install libfuse*

Opsi lain :
sudo aptitude install libfuse*
sudo apt-get install libssl-dev
sudo apt-get install libsepol-dev
```
Template untuk compile file maimai.c

```
gcc -Wall -o <nama outputan filenya> <nama_file kode program utama>.c `pkg-config fuse --cflags --libs` -lssl -lcrypto -lz
```

### Struktur Repo ###
[1t](https://github.com/rzkcp/Sisop-4-2025-IT16/blob/f955550d9008dcad0d881a92feb4ae89703e7a0b/assets/ss_1.png)

