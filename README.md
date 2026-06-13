# High-Fidelity Bluetooth Speaker via ESP32 (I2S Digital Audio) 🔊
Selamat datang di repositori **High-Fidelity Bluetooth Speaker**. Proyek ini bertujuan untuk memodernisasi "speaker pasif/bodoh" menjadi sistem penerima audio nirkabel berkinerja tinggi menggunakan mikrokontroler ESP32, DAC I2S MAX98357A, dan kontrol volume digital berbasis *Rotary Encoder* (KY-040).
## 📖 Latar Belakang & Perjalanan Eksperimen
Modul penerima Bluetooth analog pasaran sering kali memiliki kelemahan mendasar: suara desis yang parah akibat *Ground Loop Noise* dan degradasi sinyal (terutama *bass*).
### Eksperimen Awal (Gagal): DAC Internal ESP32
Pada awalnya, proyek ini mencoba memanfaatkan DAC internal bawaan ESP32 (Output pada GPIO 25 & 26). Meskipun berhasil mengeluarkan suara, hasilnya sangat **sub-optimal**:
 * **Resolusi 8-Bit:** Menghasilkan kurva gelombang yang sangat kasar (*Quantization Noise* / suara pasir konstan).
 * **Solusi Pasif:** Percobaan menggunakan *Low-Pass Filter* sederhana (Resistor 12k Ohm) gagal menyelesaikan masalah. Resistor tersebut justru menciptakan impedansi tinggi yang "mencekik" sinyal, sehingga suara terdengar sangat pelan dan kehilangan dinamika *bass*.
### Solusi Final (Berhasil): Transmisi Digital I2S End-to-End
Sistem direkonstruksi menggunakan protokol digital murni:
 1. **ESP32** menerima aliran data audio Bluetooth terkompresi (A2DP).
 2. Dekode dilakukan di dalam silikon secara *multithreading* (240 MHz).
 3. Data dikirimkan secara mentah via bus **I2S (16-Bit, 44.1kHz)**.
 4. **MAX98357A** menerima data I2S, melakukan konversi (*Digital-to-Analog*), dan langsung memperkuat sinyal (Kelas D 3W) untuk menggerakkan membran speaker dengan tenaga penuh, bebas dari interferensi kelistrikan.
## 🛠️ Perangkat Keras yang Dibutuhkan (BOM)
 * 1x **ESP32 Development Board** (NodeMCU-32S / DOIT DevKit V1)
 * 1x **MAX98357A** I2S Class-D Audio Amplifier Module
 * 1x **KY-040** Rotary Encoder (Untuk antarmuka kontrol volume)
 * 1x Speaker Pasif (Kapasitas 4 Ohm / 8 Ohm, 3 Watt)
 * Kabel Jumper secukupnya
## 🔌 Skema Koneksi Pin (Wiring)
Proses perakitan fisik wajib dilakukan secara presisi untuk menjamin kelancaran komunikasi I2S dan pembacaan interupsi.
### 1. ESP32 ke MAX98357A (Jalur I2S)

| ESP32 Pin | MAX98357A Pin | Fungsi |
| :--- | :--- | :--- |
| **GPIO 27** | BCLK | Bit Clock (Metronom data) |
| **GPIO 26** | LRC | Left/Right Clock (Word Select) |
| **GPIO 25** | DIN | Data Input (Aliran Audio Digital) |
| **VIN / 5V** | VIN | Catu Daya Utama |
| **GND** | GND | Ground |
| *(Catatan: Pin SD dan GAIN pada MAX98357A dibiarkan kosong/floating)* |  |  | <br> ### 2. ESP32 ke KY-040 (Kendali Volume)
| ESP32 Pin | KY-040 Pin | Fungsi |
| :--- | :--- | :--- |
| **GPIO 18** | CLK | Pulsa Utama Quadrature |
| **GPIO 19** | DT | Pulsa Fase Quadrature |
| **GPIO 21** | SW | Tombol Push (Untuk fitur Soft Mute) |
| **3.3V** | + (VCC) | Referensi Pull-Up Internal |
| **GND** | GND | Ground |

## 🚀 Panduan Instalasi (Langkah demi Langkah)
Proyek ini dibangun menggunakan **PlatformIO** di atas editor Visual Studio Code (VS Code).
### Prasyarat Umum
 1. Unduh dan instal **Visual Studio Code** (VS Code).
 2. Di VS Code, buka menu *Extensions* (Ctrl+Shift+X), cari **PlatformIO IDE**, dan klik instal. Tunggu hingga proses instalasi *core* selesai (biasanya memerlukan *restart* VS Code).
### Langkah 1: Kloning Repositori
Buka terminal dan jalankan perintah berikut:
```bash
git clone [https://github.com/UsernameKamu/NamaRepositori.git](https://github.com/UsernameKamu/NamaRepositori.git)
cd NamaRepositori
```
### Langkah 2: Membuka Proyek di PlatformIO
 1. Buka VS Code.
 2. Klik ikon kepala semut (PlatformIO) di panel kiri.
 3. Pilih menu **Open Project** dan arahkan ke folder yang baru saja di-*clone*.
### Langkah 3: Verifikasi platformio.ini
Pastikan *file* konfigurasi di *root folder* telah diatur untuk memaksa (*overclock*) ESP32 berjalan di kecepatan 240 MHz (sangat krusial untuk mencegah audio macet) dan mengunduh pustaka secara otomatis:
```ini
[env:esp32doit-devkit-v1]
platform = espressif32@6.5.0
board = esp32doit-devkit-v1
framework = arduino
monitor_speed = 115200
board_build.f_cpu = 240000000L  ; Overclock ke 240 MHz
lib_deps =
    [https://github.com/pschatzmann/ESP32-A2DP.git#v1.7.0](https://github.com/pschatzmann/ESP32-A2DP.git#v1.7.0)
    igorantolic/Ai Esp32 Rotary Encoder @ ^1.6
```
### Langkah 4: Proses Flashing (Unggah Kode)
#### 🪟 Pengguna Windows
 1. Hubungkan ESP32 ke port USB komputer.
 2. Windows biasanya akan otomatis menginstal *driver* CH340 atau CP2102. (Jika tidak, Anda perlu mengunduhnya secara manual).
 3. Di VS Code (PlatformIO), klik ikon **Tanda Centang (Build)** di baris status paling bawah layar untuk memverifikasi kode.
 4. Klik ikon **Tanda Panah Kanan (Upload)** untuk mengunggah program ke dalam ESP32.
#### 🐧 Pengguna Linux (Ubuntu/Debian)
Pada Linux, Anda sering kali perlu memberikan izin akses ke port Serial (USB) sebelum bisa melakukan *flashing*.
 1. Hubungkan ESP32 ke komputer.
 2. Buka Terminal dan tambahkan *user* Anda ke dalam grup dialout agar memiliki izin baca/tulis ke port serial:
   ```bash
   sudo usermod -a -G dialout $USER
   
   ```
 3. **Penting:** Lakukan *Log out* (keluar sesi) dan *Log in* kembali agar perubahan grup diterapkan (atau cukup *restart* PC).
 4. Di VS Code (PlatformIO), klik ikon **Tanda Centang (Build)**.
 5. Klik ikon **Tanda Panah Kanan (Upload)** untuk mem-flashing perangkat.
## 🎶 Penggunaan Sistem
 1. **Daya:** Sambungkan ESP32 ke catu daya yang memadai (Minimal adaptor USB 2 Ampere).
 2. **Koneksi:** Nyalakan Bluetooth di ponsel Anda dan cari perangkat bernama **ESP32_HiFi_Speaker**. Lakukan *pairing*.
 3. **Volume:** Putar kenop KY-040 untuk menaikkan atau menurunkan volume. Algoritma *Exponential Moving Average* (EMA) akan memastikan perubahannya sangat halus (*Soft Fading*).
 4. **Mute:** Tekan kenop KY-040 untuk mengaktifkan mode *Senyap* (Volume akan menurun secara otomatis perlahan, tidak putus mendadak). Putar kenop sedikit untuk mengaktifkan kembali suara (*Auto-Unmute*).
## ⚖️ Lisensi
Proyek ini didistribusikan di bawah lisensi MIT. Silakan gunakan, pelajari, dan modifikasi untuk keperluan pribadi maupun komersial.