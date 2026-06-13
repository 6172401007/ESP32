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
 * **Keuntungan:** Rangkaian sangat sederhana; tidak membutuhkan IC tambahan (menghemat biaya dan ruang).
 * **Kerugian (Kualitas Suara):** Karena hanya beresolusi 8-Bit, kurva audionya kasar dan menghasilkan *Quantization Noise* (desis pasir) yang konstan.
 * **Kerugian (Tenaga):** Sinyal analog yang keluar sangat lemah dan tidak mampu menggerakkan *speaker* secara langsung tanpa *amplifier* tambahan. Sangat rentan terhadap gangguan kelistrikan (*ground loop*).
### 📈 Karakteristik DAC Eksternal (Modul MAX98357A I2S)
 * **Keuntungan (Kualitas Suara):** Menerima data digital murni tanpa distorsi. Memiliki resolusi tinggi (16-Bit hingga 32-Bit) yang menghasilkan suara *High-Fidelity* jernih, setara kualitas CD.
 * **Keuntungan (Tenaga):** Dilengkapi dengan *Amplifier* Kelas D internal berdaya 3 Watt, siap langsung dicolokkan ke membran *speaker* dengan keluaran suara yang menggelegar dan *bass* yang padat.
 * **Kerugian:** Membutuhkan modul *hardware* tambahan dan pengkabelan jalur digital (I2S) yang presisi.
## 🧪 Percobaan 1: Pembuktian Konsep Menggunakan DAC Internal
Eksperimen pertama ini bertujuan untuk memvalidasi bahwa sistem perangkat lunak mampu menerima sinyal Bluetooth. Kita akan menggunakan jalur analog bawaan (DAC Internal).
Untuk meminimalisir *noise* frekuensi tinggi bawaan cip ESP32, kita menyisipkan komponen pasif (Resistor 12k Ohm) sebagai bentuk sederhana dari *Low-Pass Filter*.
### 📦 Daftar Barang yang Dibutuhkan (Percobaan 1)

| Komponen | Jumlah | Fungsi |
| :--- | :--- | :--- |
| **ESP32 Dev Board** | 1 | Otak utama (*NodeMCU-32S / DOIT DevKit V1*) |
| **Resistor 12k Ohm** | 2 | Peredam daya dan *filter* desis dasar |
| **KY-040 Rotary Encoder** | 1 | Pemutar untuk kendali volume digital |
| **Tactile Push Button** | 1 | Tombol pengatur Mode (WLAN / Bluetooth / Standby) |
| **Speaker Pasif** | 1 | Pengeras suara eksternal (Disarankan menggunakan modul amplifier tambahan untuk eksperimen ini) | <br> ### 🔌 Skema Perakitan (Wiring) Percobaan 1 <br> Perhatikan bahwa skema ini dibagi menjadi dua bagian: jalur khusus audio dan jalur kendali antarmuka. <br> #### Tabel Wiring: Jalur Audio Analog (DAC Internal) <br> Pada jalur ini, **resistor dipasang secara seri**—artinya resistor bertindak sebagai jembatan yang menyambungkan pin keluaran ESP32 menuju kabel masukan *speaker/amplifier*.
| ESP32 Pin Output | Pemasangan Komponen Pasif | Tujuan Akhir | Fungsi Fisik |
| :--- | :--- | :--- | :--- |
| **GPIO 25** | \rightarrow **Resistor 12k \Omega** \rightarrow | Input Audio (Kiri / L) | Sinyal Analog Saluran Kiri (8-Bit) |
| **GPIO 26** | \rightarrow **Resistor 12k \Omega** \rightarrow | Input Audio (Kanan / R) | Sinyal Analog Saluran Kanan (8-Bit) |
| **GND** | *(Sambungan langsung)* | GND Speaker/Amplifier | Referensi Nol Volt (Penutup Sirkuit) | <br> #### Tabel Wiring: Antarmuka Modul Kendali (KY-040 & Tombol Mode) <br> Modul antarmuka ini akan tetap sama konfigurasinya, baik untuk eksperimen DAC Internal maupun Eksternal. Pastikan suplai daya (VCC) KY-040 hanya masuk ke 3.3V, bukan 5V.
| ESP32 Pin | Komponen Eksternal | Pin Komponen | Fungsi Perangkat Lunak |
| :--- | :--- | :--- | :--- |
| **GPIO 18** | Rotary Encoder KY-040 | CLK (Clock) | Membaca setiap klik putaran (*Volume*) |
| **GPIO 19** | Rotary Encoder KY-040 | DT (Data) | Menentukan arah putaran (Naik/Turun) |
| **GPIO 21** | Rotary Encoder KY-040 | SW (Switch) | Interupsi *Soft Mute* saat kenop ditekan |
| **GPIO 12** | Tombol *Push Button* | Kaki 1 (Sisi A) | Kendali multi-mode (WLAN/Bluetooth) & Memicu *Deep Sleep Wakeup* |
| **GND** | Tombol *Push Button* | Kaki 2 (Sisi B) | Ground untuk mendeteksi tekanan tombol |
| **3.3V** | Rotary Encoder KY-040 | + (VCC) | Catu daya referensi sinyal (*Pull-up*) |
| **GND** | Rotary Encoder KY-040 | GND | Ground modul KY-040 |

*(Bersambung ke Part 2: Rekonstruksi Sistem Final menggunakan DAC I2S MAX98357A...)*