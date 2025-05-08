# Laporan Resmi Praktikum SISOP Modul 3
## Anggota Kelompok

| No | Nama                     | NRP         |
|----|--------------------------|-------------|
| 1  | Tiara Putri Prasetya     | 5027241013  |
| 2  | Danuja Prasasta Bastu    | 5027241037  |
| 3  | Imam Mahmud Dalil Fauzan | 5027241100  |

# SOAL 1
Laporan Terpadu: Sistem Client-Server untuk Pertukaran dan Proses File

Gambaran Umum Sistem
Sistem ini terdiri dari dua komponen utama:
1. *Client* - Aplikasi berbasis teks untuk berinteraksi dengan user
2. *Server* - Layanan backend untuk memproses permintaan client

Kedua komponen berkomunikasi melalui socket TCP pada port 8080 dengan protokol khusus.

Fungsi Utama Client

1. Koneksi ke Server
- `establish_server_connection()`:
- Membuat socket TCP dengan `socket()`
- Mengkonfigurasi alamat server (127.0.0.1:8080)
- Menghubungkan ke server dengan `connect()`
- Menangani berbagai error koneksi

2. Operasi File
- `fetch_server_file()`:
- Mengirim permintaan "DOWNLOAD <filename>"
- Menerima ukuran file dan konten file dari server
- Menyimpan file ke direktori "client/"

- `upload_for_processing()`:
- Mencari file di `client/secrets/`
- Mengirim permintaan "DECRYPT <path_file>"
- Menerima dan menampilkan respons server

3. Antarmuka Pengguna
- `show_client_options()`: Menampilkan menu interaktif
- `run_client_program()`: Loop utama untuk memproses input user

Fungsi Utama Server

1. Inisialisasi
- `initialize_server()`:
- Membuat direktori server dan database
- Menjalankan sebagai daemon (background process)
- Membuat socket dan binding ke port 8080
- Mencatat log startup

2. Proses Permintaan
- `process_client_request()`:
- Menerima dan mengidentifikasi perintah client
- Memproses tiga jenis permintaan:
- DECRYPT: Membalik teks dan menyimpan sebagai .jpeg
- DOWNLOAD: Mengirim file ke client
- EXIT: Mencatat log tanpa aksi khusus

 3. Fitur Pendukung
- `reverse_string()`: Membalikkan string input
- `store_jpeg_file()`: Menyimpan konten sebagai file .jpeg
- `record_event()`: Mencatat aktivitas ke server.log
- `send_error_response()`: Mengirim pesan error ke client

 Protokol Komunikasi

 Format Permintaan Client:
1. Download: "DOWNLOAD nama_file"
2. Decrypt: "DECRYPT path_lengkap_file"
3. Exit: "EXIT"

 Format Respons Server:
1. Untuk Download:
 - Mengirim ukuran file (long)
 - Mengirim konten file biner
2. Untuk Decrypt:
 - Mengirim string konfirmasi penyimpanan
3. Untuk Error:
 - Mengirim string deskripsi error

 Struktur Direktori

 Client:
- `client/` - Untuk menyimpan file yang didownload
- `client/secrets/` - Untuk file yang akan diupload

 Server:
- `server/` - Direktori utama
- `server/database/` - Penyimpanan file hasil proses
- `server/server.log` - Catatan aktivitas sistem

 Alur Kerja Sistem

1. Client menghubungi server pada port 8080
2. User memilih operasi melalui menu:
 - Upload: File diambil dari secrets/, dikirim untuk diproses
 - Download: Meminta file dari server database
3. Server memproses permintaan:
 - Decrypt: Membalik isi file dan simpan sebagai JPEG
 - Download: Mengirim file dari database
4. Hasil operasi dikembalikan ke client
5. Semua aktivitas dicatat dalam server.log

 Penanganan Error

1. Koneksi gagal
2. File tidak ditemkan
3. Gagal membuat/membuka file
4. Input tidak valid

Setiap error akan menghasilkan pesan ke user dan dicatat dalam log.

Keamanan

1. Server berjalan sebagai daemon
2. Port dan alamat dikonfigurasi secara eksplisit
3. Aktivitas tercatat lengkap dengan timestamp
4. File hasil proses disimpan dengan nama unik (timestamp)

Revisi :
- code sudah jalan
- code sudah bisa decrypt dan download dari server
- namun decrypt masih salah

# SOAL 2
Soal ini ada dua program utama:
`delivery_agent.c` – Menjalankan otomatisasi pengiriman Express menggunakan 3 agen.
`dispatcher.c` – Mengelola pengiriman Reguler, monitoring status, serta menampilkan daftar pesanan.

1. `delivery_agent.c`
Saat di run, langsung mengubah status paket yang bertipe Express menjadi diantar.
Program ini bertanggung jawab atas pengiriman otomatis untuk pesanan Express.
Terdapat 3 agen pengiriman yang akan aktif secara paralel (menggunakan thread).
Setiap agen akan mengambil pesanan Express dari file CSV `delivery_order.csv`.
Setelah selesai mengantar, informasi akan dicatat ke dalam file log `delivery.log`.

2. `dispatcher.c`
Program ini berfungsi sebagai pengendali utama pengiriman Reguler.
Admin dapat memilih dan memproses pesanan satu per satu secara manual.
Tersedia fitur untuk melihat status pengiriman dan daftar pesanan.
Semua log aktivitas juga akan tercatat di `delivery.log`.
Argumen untuk menjalankan program ini:
```
./dispatcher -deliver <namapengirim> <namapenerima>
./dispatcher -status <namapenerima>
./dispatcher -list
```

# SOAL 3
NO.3 THE LOST DUNGEON  

===============================  
1. Sistem Client-Server:  
-`dungeon.c` berfungsi sebagai server yang menangani semua logika game
-`player.c` berfungsi sebagai client yang menampilkan antarmuka ke pemain

2. Progres Game:
-Pemain mulai dengan senjata dasar ("Fists") dan sejumlah gold
-Pemain bisa membeli senjata lebih kuat di toko
-Pemain bisa bertarung melawan musuh untuk mendapatkan lebih banyak gold

3. Sistem Pertarungan:
-Musuh muncul dengan HP acak
-Pemain menyerang dengan damage berdasarkan senjata yang dipakai
  
===============================  
WEAPON SHOP  

Toko menawarkan 5 senjata dengan karakteristik berbeda dan passive yang berbeda:
1. Wooden Stick (Damage: 10, Harga: 50)
2. Flame Dagger (Damage: 15, Harga: 100)
3. Ice Mace (Damage: 20, Harga: 150, Passive: 10% Insta-Kill Chance)
4. Venom Sword (Damage: 25, Harga: 200, Passive: +30% Crit Chance)
5. Dragon Slayer (Damage: 40, Harga: 400)
   
===============================  
PENJELASAN PER FILE  

1. `dungeon.c` (Server)  
File ini berfungsi sebagai inti dari game atau game server, menangani semua logika permainan dan berkomunikasi dengan client.

Fungsi Utama:  
-`typedef struct {}` = membuat data player dalam permainan.
-`Weapon weapons[]` = Struct yang menyimpan info senjata (nama, damage, harga, passive ability).  
-`main()`: Membuat socket, bind ke port 8080, listen koneksi, menerima koneksi client via `accept()`, membaca perintah client,mengeksekusi fungsi sesuai perintah, dan mengirim respon e client  
-`show_weapon_shop()`: Menampilkan daftar senjata yang tersedia di toko dan mengirim ke player  
-`buy_weapon()`: Menangani proses pembelian senjata  
-`battle_mode()`: Mengelola mode pertarungan melawan musuh  
-`create_new_player` : Membuat dan menginisialisasi satu entitas Player baru dalam sistem, berdasarkan nama dan file descriptor socket-nya.

Fitur yang Ditangani:  
-Menyimpan status pemain (gold, senjata, damage, jumlah kill)  
-Mengelola inventory pemain  
-Menghitung damage dalam pertarungan  
-Memberikan reward setelah mengalahkan musuh  
-Menangani semua perintah dari client (SHOW_STATS, SHOP, dll)  

2. `player.c` (Client)  
File ini berfungsi sebagai antarmuka pemain, menampilkan menu dan menangani output/input dari pemain.  

Fungsi Utama:  
-`main()`: Membuat koneksi ke server dan menampilkan menu utama  
-Menampilkan pilihan menu dan mengirim perintah ke server  
-Menerima dan menampilkan respons dari server  

Fitur yang Ditangani:  
-Menampilkan menu utama (Status, Toko, Inventory, Battle, Exit)  
-Mengirim perintah ke server berdasarkan pilihan pemain  
-Menerima dan menampilkan informasi dari server  
-Menangani input pemain selama mode pertarungan  

3. `shop.h` (Header File)  
File ini berisi mendefinisikan struktur data dan deklarasi fungsi yang terkait dengan toko senjata.  

Fungsi Utama:  
-`struct Weapon`: Menyimpan properti senjata: name, damage, price, passive.  
-`show_weapon_shop()`: Untuk menampilkan toko daftar senjata.  
-`buy_weapon()`: Untuk logika pembelian senjata  
-`MAX_INVENTORY`: Untuk Batas maksimal inventory (10)  

===============================   
CARA RUN  
1. `player.c`
   
   ```
   gcc player.c -o player
    ```

3. `dungeon.c` dan `shop.h`
   
   ```
    gcc dungeon.c shop.h -o dungeon -lpthread
   ```

Lalu jalankan  
```
./dungeon      # di terminal 1 (server)
./player       # di terminal 2 (client 1)
./player       # di terminal 3 (client 2)
```


# SOAL 4
1. `system.c`
Program admin yang bertugas:
- Mengelola data hunter dan dungeon
- Generate dungeon secara acak
- Melihat statistik hunter
- Fitur ban/reset hunter

Spesifikasi Dungeon:
Level Minimal : 1-5
ATK Reward   : 100-150
HP Reward    : 50-100  
DEF Reward   : 25-50
EXP Reward   : 150-300

2. `hunter.c`
Program player yang bisa:
- Registrasi/login hunter
- Melihat daftar dungeon
- Raid dungeon untuk mendapatkan reward
- Battle dengan hunter lain
- Sistem notifikasi
- Stat Awal Hunter:
Level : 1
ATK   : 10
HP    : 100
DEF   : 5
EXP   : 0

Cara me Run:
- Jalankan `system.c` terlebih dahulu (sebagai server):
```
gcc system.c -o system
./system
```

- Jalankan `hunter.c` (sebagai client):
```
gcc hunter.c -o hunter -lpthread
./hunter
```

Fitur Utama
Menu System (Admin)
```
1. Hunter Info   // Lihat stat semua hunter
2. Dungeon Info     // Lihat info dungeon
3. Generate Dungeon // Buat dungeon baru
4. Ban Hunter       // Blokir hunter
5. Reset Hunter     // Reset stat hunter
6. Exit
```

Menu Hunter (Player)
```
1. List Dungeon    // Lihat dungeon tersedia
2. Raid Dungeon       // Serang dungeon untuk dapat reward
3. Battle Hunter      // Duel dengan hunter lain
4. Notification       // Toggle notifikasi
5. Exit
```

Mekanisme Penting
Level Up:
Saat EXP ≥ 500, level naik dan EXP reset ke 0

Battle System:
Total power = ATK + HP + DEF
Yang kalah akan dihapus dari sistem
Pemenang dapat semua stat lawan

Shared Memory:
Menggunakan struktur SystemData yang dibagi antara system.c dan hunter.c
Data tersimpan selama system.c berjalan
