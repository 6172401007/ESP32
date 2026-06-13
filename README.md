# High-Fidelity Bluetooth Speaker via ESP32 (Part 1: Setup & Eksperimen DAC Internal) 🔊
Selamat datang di panduan proyek ESP32 Audio Receiver. Repositori ini berisi kode dan skema untuk memodifikasi *speaker* biasa menjadi sistem audio Bluetooth. Proyek ini mendukung dua lingkungan: **C++ (PlatformIO)** untuk performa maksimal, dan **MicroPython (Custom Firmware)** untuk pengembangan lapis atas yang cepat.
## 💻 Panduan Instalasi Lingkungan Pengembangan
Pilih salah satu metode pengembangan di bawah ini sesuai dengan bahasa pemrograman yang ingin Anda gunakan.
### Opsi A: Instalasi C++ via Visual Studio Code (PlatformIO)
Ini adalah metode paling stabil karena memberikan akses tingkat rendah ke perangkat keras ESP32 untuk pemrosesan audio berkecepatan tinggi.
 1. **Unduh Editor:** Instal Visual Studio Code.
 2. **Instal Ekstensi:** Buka VS Code, masuk ke tab *Extensions* (Ctrl+Shift+X), cari **PlatformIO IDE**, lalu instal. Tunggu hingga proses instalasi komponen inti selesai, kemudian *restart* VS Code.
 3. **Buka Proyek:** Klik ikon PlatformIO (kepala semut) di menu kiri, pilih **Open Project**, dan arahkan ke folder repositori ini.
 4. **Konfigurasi Otomatis:** PlatformIO akan membaca *file* platformio.ini untuk mengatur CPU ESP32 berjalan di kecepatan 240MHz dan mengunduh pustaka secara otomatis (seperti ESP32-A2DP).
 5. **Eksekusi:** Sambungkan ESP32, klik tombol **Build (Ikon Centang)** di baris bawah layar untuk memverifikasi kode, lalu klik **Upload (Ikon Panah Kanan)** untuk memasukkan program.
### Opsi B: Instalasi MicroPython via Thonny IDE
Metode ini bergantung pada *Custom Firmware* MicroPython yang telah "disuntikkan" modul bahasa C khusus untuk membongkar paket audio Bluetooth di latar belakang.
 1. **Flash Firmware Utama:** ESP32 wajib di-*flash* terlebih dahulu menggunakan *custom firmware* (misalnya firmware_custom_a2dp.bin) melalui perintah esptool.py.
 2. **Siapkan IDE:** Instal Thonny IDE.
 3. **Atur Interpreter:** Buka Thonny, navigasi ke **Tools > Options > Interpreter**. Pilih **MicroPython (ESP32)** dan pastikan *Port* yang dipilih sesuai dengan kabel USB Anda.
 4. **Simpan Kode:** Buka skrip main.py dari repositori ini ke dalam Thonny. Klik tombol **Save**, lalu pilih *"MicroPython device"*, dan simpan dengan nama persis main.py agar ESP32 langsung menjalankannya setiap kali dihidupkan.
## ⚖️ Analisis Perbandingan: DAC Internal vs DAC Eksternal
Sebelum merakit, mari bedah perbedaan kapabilitas fisik dari komponen konversi audio (Digital ke Analog) pada sistem ini.
### 📉 Karakteristik DAC Internal (ESP32 Built-in)
 * **Keuntungan:** * Rangkaian super sederhana.
   * *Zero extra active components*: Tidak membutuhkan modul IC DAC tambahan, menghemat biaya dan ruang perakitan.
 * **Kerugian:** * **Kualitas Suara:** Resolusi mentok di 8-Bit. Kurva audio yang kasar menghasilkan *Quantization Noise* (suara desis pasir) yang konstan.
   * **Kekuatan Daya:** Sinyal analog yang keluar sangat lemah, wajib disambungkan ke *amplifier* eksternal tambahan. Rentan terkena *ground loop noise*.
### 📈 Karakteristik DAC Eksternal (Modul MAX98357A I2S)
 * **Keuntungan:** * **Kualitas Suara:** Menerima data digital murni I2S tanpa distorsi dengan resolusi tinggi (16-Bit hingga 32-Bit), menghasilkan suara *High-Fidelity* yang jernih.
   * **Kekuatan Daya:** Dilengkapi *Amplifier* Kelas D internal berdaya 3 Watt. Siap langsung dicolokkan ke *speaker* pasif untuk menghasilkan suara yang kencang dengan *bass* padat.
 * **Kerugian:** * Membutuhkan modul perangkat keras tambahan.
   * Membutuhkan ketelitian ekstra dalam merakit pengkabelan jalur digital (BCLK, LRC, DIN).
## 🧪 Percobaan 1: Pembuktian Konsep Menggunakan DAC Internal
Eksperimen pertama ini murni bertujuan untuk memvalidasi bahwa ESP32 sanggup bertindak sebagai penerima Bluetooth A2DP dan merutekan aliran datanya ke pin keluaran analog.
Pada fase ini, **semua antarmuka kendali (seperti volume fisik atau tombol mode) diabaikan**. Fokus hanya pada pemrosesan suara dasar. Untuk meminimalisir *noise* frekuensi tinggi bawaan cip ESP32, kita menyisipkan komponen pasif (Resistor 12k Ohm) sebagai bentuk sederhana dari *Passive Low-Pass Filter*.
### 📦 Daftar Barang yang Dibutuhkan (Percobaan 1)

| Komponen | Jumlah | Fungsi |
| :--- | :--- | :--- |
| **ESP32 Dev Board** | 1 | Otak utama (*NodeMCU-32S / DOIT DevKit V1*) |
| **Resistor 12k Ohm** | 2 | Peredam daya dan *filter* pasif untuk menghaluskan kurva 8-bit |
| **Modul Amplifier/Speaker Aktif** | 1 | Penguat suara akhir (karena output ESP32 terlalu lemah) |

### 🔌 Skema Perakitan (Wiring) Percobaan 1 Pada konfigurasi minimalis ini, pin GPIO 25 dan 26 ESP32 diatur sebagai keluaran *DAC channel 1* dan *DAC channel 2*. <br> **Perhatian:** Resistor **wajib dipasang secara seri**—artinya resistor tersebut bertindak sebagai jembatan yang berada tepat di tengah-tengah antara pin ESP32 dan kabel masukan (input L/R) menuju amplifier Anda.
| ESP32 Pin Output | Komponen Pasif | Tujuan Akhir | Fungsi Fisik |
| :--- | :--- | :--- | :--- |
| **GPIO 25** | \rightarrow **Resistor 12k \Omega** \rightarrow | Input Audio (Kiri / L) | Sinyal Analog Saluran Kiri (8-Bit) |
| **GPIO 26** | \rightarrow **Resistor 12k \Omega** \rightarrow | Input Audio (Kanan / R) | Sinyal Analog Saluran Kanan (8-Bit) |
| **GND** | *(Sambungan langsung kabel)* | GND Amplifier/Speaker | Referensi Nol Volt (Penutup Sirkuit) |

## 🚀 Part 2: Solusi Final (Transmisi Digital I2S via DAC Eksternal)
Melanjutkan dari Eksperimen 1, kita menyadari bahwa DAC internal 8-bit ESP32 memiliki keterbatasan hukum fisika yang menyebabkan desis kuantisasi dan kurangnya daya. Oleh karena itu, pada tahap final ini, kita melakukan rekonstruksi arsitektur menuju sistem digital murni (*End-to-End Digital Audio*).
Pada fase final ini, **penggunaan Resistor 12k Ohm dihilangkan sepenuhnya**. Data audio tidak lagi diubah menjadi analog di dalam ESP32, melainkan dikirim dalam bentuk biner mentah (16-bit) melalui bus I2S menuju modul DAC Eksternal (MAX98357A). Sistem ini juga mengintegrasikan *Rotary Encoder* untuk kendali volume presisi dan tombol fisik untuk manajemen daya/mode.
### 📦 Daftar Barang yang Dibutuhkan (Solusi Final)

| Komponen | Jumlah | Fungsi |
| :--- | :--- | :--- |
| **ESP32 Dev Board** | 1 | Otak utama pemrosesan *Dual-Core* (Dekode A2DP & UI) |
| **MAX98357A Module** | 1 | I2S DAC + Amplifier Kelas D 3W (Mengubah digital ke analog dan menguatkan sinyal) |
| **KY-040 Rotary Encoder** | 1 | Kendali volume putar (*Quadrature*) dan *Soft Mute* |
| **Tactile Push Button** | 1 | Tombol pengatur Mode (WLAN / Bluetooth) & *Deep Sleep* |
| **Speaker Pasif** | 1 | Pengeras suara 4 Ohm / 8 Ohm (Maksimal 3 Watt) |
| **Kabel Jumper** | Secukupnya | Menghubungkan jalur komunikasi digital dan daya | <br> ### 🔌 Skema Perakitan (Wiring) Solusi Final <br> Perakitan dipecah menjadi dua segmen: **Jalur Audio Digital (I2S)** dan **Jalur Kendali Antarmuka (UI)**. Pastikan setiap sambungan kuat karena komunikasi I2S membutuhkan integritas sinyal berkecepatan tinggi. <br> #### 1. Wiring Jalur Audio Digital (I2S ke MAX98357A) <br> Jalur ini murni mengalirkan data logika (3.3V). Pin keluaran dari ESP32 langsung dihubungkan ke pin input MAX98357A tanpa komponen pasif (tanpa resistor/kapasitor) di tengah jalur.
| ESP32 Pin | MAX98357A Pin | Fungsi Komunikasi I2S |
| :--- | :--- | :--- |
| **GPIO 27** | BCLK | *Bit Clock* (Metronom sinkronisasi data per bit) |
| **GPIO 26** | LRC | *Left/Right Clock / Word Select* (Penanda kanal Kiri/Kanan) |
| **GPIO 25** | DIN | *Data Input* (Aliran *payload* audio digital 16-bit) |
| **VIN / 5V** | VIN | Catu Daya Utama Amplifier (Disarankan menggunakan arus 5V untuk daya maksimal) |
| **GND** | GND | Ground Referensi Bersama | <br> *(Catatan: Pin SD dan GAIN pada MAX98357A dibiarkan kosong/tidak tersambung. Secara default sistem akan mencampur saluran stereo Kiri & Kanan menjadi Mono untuk 1 Speaker dengan penguatan standar 9dB).* <br> #### 2. Wiring Modul Kendali (KY-040 & Tombol Multifungsi) <br> Pemasangan antarmuka ini krusial untuk mengeksekusi fitur-fitur seperti interpolasi *Soft Fading* volume dan perpindahan mode. **Penting:** Pastikan VCC KY-040 masuk ke jalur 3.3V, bukan 5V, agar aman bagi pin GPIO ESP32.
| ESP32 Pin | Komponen Eksternal | Pin Komponen | Fungsi Perangkat Lunak |
| :--- | :--- | :--- | :--- |
| **GPIO 18** | Rotary Encoder KY-040 | CLK (Clock) | Sinyal *Quadrature A* (Mendeteksi setiap klik putaran) |
| **GPIO 19** | Rotary Encoder KY-040 | DT (Data) | Sinyal *Quadrature B* (Mendeteksi arah putaran Naik/Turun) |
| **GPIO 21** | Rotary Encoder KY-040 | SW (Switch) | Interupsi *Soft Mute* (Mengurangi volume ke 0 secara perlahan saat kenop ditekan) |
| **GPIO 12** | Tombol *Push Button* | Kaki 1 (Sisi A) | Kendali Mode & *Deep Sleep* |
| **GND** | Tombol *Push Button* | Kaki 2 (Sisi B) | Ground untuk pemicu tekanan tombol |
| **3.3V** | Rotary Encoder KY-040 | + (VCC) | Catu daya tegangan referensi *Pull-up* |
| **GND** | Rotary Encoder KY-040 | GND | Ground utama modul KY-040 |

### 🎛️ Fitur Antarmuka Pengguna (UI) & Manajemen Daya
Arsitektur pada fase final ini mengizinkan kontrol tingkat komersial yang mulus tanpa mengganggu *streaming* audio yang sedang berjalan:
 1. **Kendali Volume Pintar (KY-040):**
   * **Putar:** Mengubah volume. Algoritma *Exponential Moving Average (EMA)* memastikan transisi nilai terjadi secara merayap halus (*Soft Fading*).
   * **Tekan Kenop (Klik):** Memicu fitur *Soft Mute*. Suara tidak putus mendadak, melainkan meluncur turun hingga senyap (dan sebaliknya saat di-*unmute*).
   * **Auto-Unmute:** Jika *speaker* dalam keadaan *Mute*, memutar kenop volume akan otomatis membatalkan status *Mute* dan mengembalikan suara mengikuti level yang baru diputar.
 2. **Tombol Multifungsi (GPIO 12):**
   * **Tekan Singkat (< 2 Detik):** Memicu *Soft Reboot* untuk berpindah secara aman dari **Mode Bluetooth A2DP** ke **Mode WLAN (Web Radio)**, dan sebaliknya saat ditekan kembali.
   * **Tekan Tahan (> 3 Detik):** Menyimpan *state* terakhir ke dalam memori *Flash*, memutuskan transmisi audio, dan memasukkan ESP32 ke dalam mode **Ultra Low Power (ULP) Deep Sleep / Standby**.
   * **Wakeup:** Saat dalam mode *Deep Sleep*, menekan tombol ini (GPIO 12) akan membangunkan mikrokontroler kembali ke mode operasional penuh.