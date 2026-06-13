# High-Fidelity Bluetooth Speaker via ESP32 (Part 1: Setup & Eksperimen DAC Internal) 🔊
Selamat datang di panduan proyek ESP32 Audio Receiver. Repositori ini berisi kode dan skema untuk memodifikasi *speaker* biasa menjadi sistem audio Bluetooth. Proyek ini mendukung dua lingkungan: **C++ (PlatformIO)** untuk performa maksimal, dan **MicroPython (Custom Firmware)** untuk pengembangan lapis atas yang cepat.
## 💻 Panduan Instalasi Lingkungan Pengembangan
Pilih salah satu metode pengembangan di bawah ini sesuai dengan bahasa pemrograman yang ingin Anda gunakan.
### Opsi A: Instalasi C++ via Visual Studio Code (PlatformIO)
Ini adalah metode paling stabil karena memberikan akses tingkat rendah ke *hardware* ESP32 untuk *streaming* audio berkecepatan tinggi.
 1. **Unduh Editor:** Instal Visual Studio Code.
 2. **Instal Ekstensi:** Buka VS Code, masuk ke tab *Extensions* (Ctrl+Shift+X), cari **PlatformIO IDE**, lalu instal. Tunggu hingga proses intalasi komponen *core* selesai dan *restart* VS Code.
 3. **Buka Proyek:** Klik ikon PlatformIO (kepala semut) di menu kiri, pilih **Open Project**, dan arahkan ke folder *repository* ini.
 4. **Konfigurasi Otomatis:** PlatformIO akan secara otomatis membaca *file* platformio.ini untuk mengatur *overclock* CPU ke 240MHz dan mengunduh pustaka (seperti ESP32-A2DP).
 5. **Build & Upload:** Sambungkan ESP32, klik tombol **Build (Tanda Centang)** di baris bawah untuk memverifikasi, lalu klik **Upload (Tanda Panah Kanan)** untuk memasukkan kode.
### Opsi B: Instalasi MicroPython via Thonny IDE
Metode ini menggunakan *Custom Firmware* MicroPython yang telah disisipkan modul C untuk menangani *decoding* Bluetooth di latar belakang.
 1. **Flash Firmware:** Pertama, ESP32 harus di-*flash* dengan *custom firmware* (misalnya firmware_custom_a2dp.bin) menggunakan *tool* seperti esptool.py.
 2. **Unduh IDE:** Instal Thonny IDE.
 3. **Konfigurasi Interpreter:** Buka Thonny, pergi ke **Tools > Options > Interpreter**. Pilih **MicroPython (ESP32)** dan pilih *Port* COM/USB yang sesuai dengan perangkat Anda.
 4. **Unggah Kode:** Buka *file* main.py dari repositori ini di Thonny. Klik tombol **Save (Ikon Disket)**, pilih *MicroPython device*, dan simpan dengan nama main.py agar langsung berjalan saat ESP32 dinyalakan.
## ⚖️ Analisis Hardware: DAC Internal vs DAC Eksternal (I2S)
Sebelum merakit komponen, penting untuk memahami perbedaan kapabilitas konversi digital-ke-analog (DAC) pada sistem ini.

| Fitur | DAC Internal (ESP32 Built-in) | DAC Eksternal (MAX98357A I2S) |
| :--- | :--- | :--- |
| **Resolusi Audio** | 8-Bit (Tangga gelombang sangat kasar) | 16-Bit hingga 32-Bit (Sangat halus) |
| **Kualitas Suara** | Penuh desis (*Quantization Noise*) & *Bass* tipis | Sangat jernih, rentang dinamis penuh (*Hi-Fi*) |
| **Kekuatan Output** | Sangat lemah (Hanya sinyal level baris/Aux) | Sangat kuat (Built-in Amplifier Kelas D 3W) |
| **Hambatan Fisik** | Rentan terhadap interferensi sirkuit (*EMI*) | Transmisi data digital murni kebal gangguan |
| **Kompleksitas** | Mudah, hemat komponen (hanya resistor) | Butuh IC ekstra & kabel digital (I2S) | <br> ## 🧪 Percobaan 1: Menggunakan DAC Internal ESP32 <br> Meskipun secara akustik tidak ideal, menggunakan DAC bawaan ESP32 (GPIO 25 & 26) adalah cara termudah untuk membuktikan bahwa sistem perangkat lunak dapat menerima dan mengarahkan aliran audio Bluetooth. <br> Untuk mengurangi *switching noise* dari sirkuit internal, kita menerapkan modifikasi *hardware* sederhana berupa pemasangan **Passive Low-Pass Filter** menggunakan resistor dengan nilai hambatan tinggi. <br> ### Barang yang Dibutuhkan (Eksperimen DAC Internal) <br> * 1x **ESP32 Development Board** (NodeMCU-32S / DOIT DevKit V1) <br> * 2x **Resistor 12k Ohm** (Berfungsi sebagai *attenuator* & filter desis dasar) <br> * 1x Modul Rotary Encoder **KY-040** (Kendali Volume) <br> * 1x Push Button / Tactile Switch (Kendali Mode) <br> * 1x Speaker Pasif / Modul Amplifier Eksternal Biasa <br> * Kabel Jumper <br> ### Skema Sambungan Kabel (Wiring) Percobaan 1 <br> Pada konfigurasi ini, isyarat suara *Stereo* dikeluarkan langsung dari ESP32. Resistor 12k Ohm **wajib dipasang secara seri** (menjadi jembatan) antara pin GPIO ESP32 dan terminal masukan (*input*) kabel *speaker/amplifier*. <br> #### 1. Jalur Audio Analog (DAC Internal)
| ESP32 Pin | Komponen Pasif | Tujuan Output | Fungsi |
| :--- | :--- | :--- | :--- |
| **GPIO 25** | -> Resistor 12k Ohm -> | Input Audio Kiri (L) | Output DAC 1 (Saluran Kiri 8-bit) |
| **GPIO 26** | -> Resistor 12k Ohm -> | Input Audio Kanan (R) | Output DAC 2 (Saluran Kanan 8-bit) |
| **GND** | Tidak Ada | Ground Speaker/Amp | Referensi Tegangan Nol | <br> #### 2. Antarmuka Modul Kendali (Berlaku untuk semua percobaan)
| ESP32 Pin | Komponen | Pin Komponen | Fungsi Utama |
| :--- | :--- | :--- | :--- |
| **GPIO 18** | KY-040 | CLK | Mendeteksi putaran volume |
| **GPIO 19** | KY-040 | DT | Menentukan arah putaran volume |
| **GPIO 21** | KY-040 | SW | Tombol *Soft Mute* (Tekan kenop) |
| **GPIO 12** | Push Button | Kaki 1 | Mode Switch & Sleep (*Ext0 Wakeup*) |
| **GND** | Push Button | Kaki 2 | Ground Pemicu Tombol |
| **3.3V** | KY-040 | + (VCC) | Tegangan referensi *Pull-Up* KY-040 |
| **GND** | KY-040 | GND | Ground KY-040 |

*(Berlanjut ke Part 2 untuk implementasi solusi final menggunakan DAC Eksternal I2S...)*