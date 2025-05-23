# LAPORAN PRAKTIKUM MODUL-4 #
## Praktikum Sistem Operasi Kelompok IT16 ##

| Nama | NRP       |
|-------|-----------|
| Ahmad Syauqi Reza | 5027241085   |
| Mochammad Atha Tajuddin   | 5027241093  |
| Tiara Fatimah Azzahra   | 5027241090  |
---
### Soal_1 ###
Dalam soal ini intinya praktikan diminta untuk mencari arti dari te
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
![1t](https://github.com/rzkcp/Sisop-4-2025-IT16/blob/f955550d9008dcad0d881a92feb4ae89703e7a0b/assets/ss_1.png)

![1t](https://github.com/rzkcp/Sisop-4-2025-IT16/blob/0619c3ff36e5222b649f63cb5f3afc5b8a236f05/assets/ss_2.png)

-Struktur repo untuk fuse_dir & chiho
### Pengerjaan ###

-1. Compile file maimai.c dengan format template compile yang sudah tersedia, kemudian membuat sebuah direktori bernamakan fuse_dir.
-2. Jalankan file maimai.c dengan cara 
```
./maimai -f fuse_dir
```
-3. Pastikan setelah menjalankan file maimai.c untuk membuka tab terminal baru sebagai jalan atau tempat untuk melakukan uji coba dari kode maimai.c,setelah dijalankan berikut hasilnya :
![1t](https://github.com/rzkcp/Sisop-4-2025-IT16/blob/0619c3ff36e5222b649f63cb5f3afc5b8a236f05/assets/ss_3.png)

![1t](https://github.com/rzkcp/Sisop-4-2025-IT16/blob/0619c3ff36e5222b649f63cb5f3afc5b8a236f05/assets/ss_4.png)

-4. Lakukan uji coba dengan melakukan write/delete dsj ke dalam folder fuse_dir, bisa menggunakan konsep redirection,tee,nano dsb.
```
echo "<bebas kalimat apa saja>" > fuse_dir/starter/tes.txt
```
![1t](https://github.com/rzkcp/Sisop-4-2025-IT16/blob/0619c3ff36e5222b649f63cb5f3afc5b8a236f05/assets/ss_5.png)

-5. Untuk selanjutnya dan seterusnya dapat menggunakan metode uji coba yang sama,kecuali untuk folder blackrose pada direktori ➜ fuse_dir/blackrose, ini khusus karena metode pengujiannya mengunaakan gambar atau screenshot.

![1t](https://github.com/rzkcp/Sisop-4-2025-IT16/blob/0619c3ff36e5222b649f63cb5f3afc5b8a236f05/assets/ss_6.png)

![1t](https://github.com/rzkcp/Sisop-4-2025-IT16/blob/0619c3ff36e5222b649f63cb5f3afc5b8a236f05/assets/ss_7.png)
- Contoh output dari fuse_dir ke direktori chiho(sesuai format) di mana karena ini berada pada direktori dragon maka teks akan di enkripsi dengan menggunakan metode kriptografi klasik ROT13

![1t](https://github.com/rzkcp/Sisop-4-2025-IT16/blob/0619c3ff36e5222b649f63cb5f3afc5b8a236f05/assets/ss_8.png)

![1t](https://github.com/rzkcp/Sisop-4-2025-IT16/blob/0619c3ff36e5222b649f63cb5f3afc5b8a236f05/assets/ss_9.png)
- Contoh berikut cara pengujian untuk folder blackrose pada direktori fuse_dir

![1t](https://github.com/rzkcp/Sisop-4-2025-IT16/blob/0619c3ff36e5222b649f63cb5f3afc5b8a236f05/assets/ss_10.png)
- Karena pada kriteria dan ketentuan soal diminta bahwasannya untuk hasil dari blackrose diubah ke format binary murni maka hasilnya akan seperti gambar di atas

![1t](https://github.com/rzkcp/Sisop-4-2025-IT16/blob/0619c3ff36e5222b649f63cb5f3afc5b8a236f05/assets/ss_11.png)
