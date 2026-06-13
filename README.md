# High-Fidelity Bluetooth Speaker via ESP32 (I2S Digital Audio) 🔊
Selamat datang di repositori **High-Fidelity Bluetooth Speaker**. Proyek ini bertujuan untuk memodernisasi "speaker pasif/bodoh" menjadi sistem penerima audio nirkabel berkinerja tinggi menggunakan mikrokontroler ESP32.
Proyek ini mendukung dua arsitektur perangkat lunak: **C++ (ESP-IDF/PlatformIO)** dan **MicroPython (Custom C-Module Firmware)**, dengan fitur kendali volume digital (KY-040) dan manajemen daya cerdas.
## 📖 Latar Belakang & Perjalanan Eksperimen
Modul penerima Bluetooth analog pasaran sering kali memiliki kelemahan mendasar: suara desis yang parah akibat *Ground Loop Noise* dan degradasi sinyal. Untuk mengatasi hal ini, eksperimen dilakukan dalam dua fase:
### Percobaan Pertama: DAC Internal ESP32
Pada tahap awal, proyek ini memanfaatkan DAC internal bawaan ESP32 (Output pada GPIO 25 & GPIO 26). Untuk menekan *switching noise*, diterapkan *Passive Low-Pass Filter* menggunakan **resistor 12k Ohm** secara seri sebelum menuju input amplifier.
Sistem ini **berhasil** menerima aliran Bluetooth (A2DP) dan memproduksi suara secara langsung. Namun, karena limitasi arsitektur DAC internal yang hanya memiliki resolusi 8-Bit, kurva gelombangnya masih menghasilkan *Quantization Noise* ringan dan pelemahan dinamika *bass* akibat impedansi resistor yang tinggi.
### Solusi Final & Optimal: Transmisi Digital I2S End-to-End
Untuk mencapai kualitas *High-Fidelity*, sistem direkonstruksi menggunakan protokol digital murni:
 1. **ESP32** menerima aliran data audio Bluetooth terkompresi (SBC Codec).
 2. Dekode dilakukan di dalam silikon menggunakan *Asymmetric Multiprocessing* (Core 0 untuk radio, Core 1 untuk UI).
 3. Data dikirimkan secara mentah via bus **I2S (16-Bit, 44.1kHz)**.
 4. **MAX98357A** menerima data I2S, melakukan konversi digital-ke-analog presisi tinggi, dan langsung memperkuat sinyal (Kelas D 3W) ke speaker secara transparan.
## 🌟 Fitur Lanjutan: Multi-Mode & Manajemen Daya (ULP)
Sistem ini dilengkapi dengan tombol multifungsi terdedikasi untuk mengatur mode operasi dan efisiensi daya:
 * **Tekan Singkat (< 2 Detik):** Memicu *Soft Reboot* secara aman untuk berpindah (*toggle*) antara **Mode Bluetooth A2DP** dan **Mode WLAN (Web Radio)**.
 * **Tekan Tahan (> 3 Detik):** Memutus koneksi audio dan memasukkan mikrokontroler ke mode **Deep Sleep (Standby)**. Konsumsi daya turun drastis ke tingkat mikroampere. Menekan tombol ini lagi akan membangunkan sistem via *ULP (Ultra Low Power) Coprocessor Wakeup*.
## 🛠️ Perangkat Keras yang Dibutuhkan (BOM)
 * 1x **ESP32 Development Board** (NodeMCU-32S / DOIT DevKit V1)
 * 1x **MAX98357A** I2S Class-D Audio Amplifier Module (Untuk konfigurasi final)
 * 2x **Resistor 12k Ohm** (Khusus untuk konfigurasi DAC Internal)
 * 1x **KY-040** Rotary Encoder (Untuk antarmuka kontrol volume)
 * 1x **Push Button (Tactile Switch)** (Untuk mode WLAN & Standby)
 * 1x Speaker Pasif (4 Ohm / 8 Ohm, 3 Watt)
## 🔌 Skema Koneksi Pin (Wiring)
Proses perakitan fisik memegang peranan penting. Silakan ikuti tabel yang sesuai dengan konfigurasi keras (DAC) dan perangkat lunak (C++ / MicroPython) yang Anda pilih.
### Tabel 1: Konfigurasi DAC EKSTERNAL MAX98357A (C++ & MicroPython)
*Koneksi ini berlaku identik baik saat menggunakan framework C++ maupun Custom MicroPython.*

| ESP32 Pin | MAX98357A Pin | Fungsi Komunikasi I2S |
| :--- | :--- | :--- |
| **GPIO 27** | BCLK | Bit Clock (Metronom data sinkron) |
| **GPIO 26** | LRC | Left/Right Clock (Word Select) |
| **GPIO 25** | DIN | Data Input (Aliran Audio Digital 16-bit) |
| **VIN / 5V** | VIN | Catu Daya Utama Amplifier |
| **GND** | GND | Ground Bersama | <br> ### Tabel 2: Konfigurasi DAC INTERNAL ESP32 (C++ & MicroPython) <br> *Eksperimen pertama menggunakan output analog bawaan. Pasang resistor 12k Ohm secara seri di jalur audio.*
| ESP32 Pin | Komponen Pasif | Tujuan Output | Fungsi |
| :--- | :--- | :--- | :--- |
| **GPIO 25** | -> Resistor 12k Ohm -> | Input Audio Kiri (L) / Speaker | Output DAC 1 (8-bit) |
| **GPIO 26** | -> Resistor 12k Ohm -> | Input Audio Kanan (R) / Speaker | Output DAC 2 (8-bit) |
| **GND** | - | Ground Speaker/Amp | Ground Referensi Audio | <br> ### Tabel 3: Modul Antarmuka & Kendali (UI) <br> *Berlaku untuk semua mode (Eksternal/Internal dan C++/MicroPython).*
| ESP32 Pin | Komponen | Pin Komponen | Fungsi Interupsi (IRQ) |
| :--- | :--- | :--- | :--- |
| **GPIO 18** | KY-040 | CLK | Quadrature A (Volume Naik/Turun) |
| **GPIO 19** | KY-040 | DT | Quadrature B (Fase Putaran) |
| **GPIO 21** | KY-040 | SW | Soft Mute (Tekan Kenop) |
| **GPIO 12** | Push Button | Kaki 1 | Mode Switch (WLAN) & ULP Deep Sleep |
| **GND** | Push Button | Kaki 2 | Ground Pemicu Tombol |
| **3.3V** | KY-040 | + (VCC) | Tegangan Referensi Pull-Up |
| **GND** | KY-040 | GND | Ground Modul Encoder |

*(Catatan: GPIO 12 dipilih khusus untuk push button karena mendukung fitur ext0_wakeup dari status Deep Sleep).*
## 🚀 Panduan Instalasi (Langkah demi Langkah)
### Opsi A: Instalasi Lingkungan C++ (PlatformIO)
Sangat direkomendasikan untuk stabilitas transmisi A2DP maksimal.
 1. Instal **Visual Studio Code** dan ekstensi **PlatformIO IDE**.
 2. *Clone* repositori ini: git clone https://github.com/UsernameKamu/NamaRepositori.git
 3. Buka folder proyek melalui PlatformIO.
 4. Pastikan platformio.ini memiliki flag *overclock*:
   ```ini
   board_build.f_cpu = 240000000L  ; Overclock ke 240 MHz
   
   ```
 5. Hubungkan ESP32, lalu klik tombol **Upload (Tanda Panah Kanan)** di baris status bawah VS Code.
### Opsi B: Instalasi Lingkungan MicroPython (Custom Firmware)
Arsitektur ini menggunakan MicroPython yang telah di-*compile* ulang dengan *wrapper* bahasa C untuk mem-bypass *Global Interpreter Lock* (GIL) saat mendekode audio.
 1. *Flash* ESP32 Anda menggunakan *file* firmware .bin khusus yang tersedia di folder /firmware (berisi *C-Module* A2DP I2S). Anda bisa menggunakan *esptool.py*:
   ```bash
   esptool.py --chip esp32 --port /dev/ttyUSB0 write_flash -z 0x1000 firmware_custom_a2dp.bin
   
   ```
 2. Gunakan IDE seperti **Thonny** atau **ampy** untuk mengunggah main.py dari repositori ini ke dalam sistem file ESP32.
 3. *Restart* ESP32, dan skrip *multithreading* Python akan otomatis menginisiasi mesin audio C++ di latar belakang.
## 🐧 Catatan Tambahan Pengguna Linux (Ubuntu/Debian)
Jika Anda mengalami masalah Permission Denied saat melakukan *flashing* ke port Serial (USB), berikan izin akses permanen pada *user* Anda:
```bash
sudo usermod -a -G dialout $USER
```
*(Lakukan Log out dan Log in kembali agar perubahan grup diterapkan).*
## ⚖️ Lisensi
Proyek ini didistribusikan di bawah lisensi MIT. Silakan gunakan, pelajari, dan modifikasi untuk keperluan pribadi maupun komersial.