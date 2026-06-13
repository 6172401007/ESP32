# Panduan Proyek ESP32 Bluetooth Speaker (Part 1: Setup & Internal DAC)
Dokumentasi ini berisi instruksi teknis pembuatan receiver audio Bluetooth menggunakan ESP32. Proyek mendukung dua arsitektur perangkat lunak: C++ dan MicroPython.
## 1. Instalasi Lingkungan Pengembangan
Pilih salah satu metode instalasi berdasarkan bahasa pemrograman yang digunakan.
### Opsi A: C++ (Visual Studio Code & PlatformIO)
 1. Instal **Visual Studio Code**.
 2. Buka tab *Extensions* (Ctrl+Shift+X), cari dan instal **PlatformIO IDE**. Restart VS Code setelah instalasi selesai.
 3. Klik ikon PlatformIO, pilih **Open Project**, dan buka folder repositori proyek ini.
 4. Buka file platformio.ini dan pastikan terdapat parameter overclock CPU ke 240MHz:
   board_build.f_cpu = 240000000L
 5. Sambungkan ESP32 via USB.
 6. Klik ikon **Build** (tanda centang di bawah) untuk kompilasi, lalu klik **Upload** (tanda panah kanan) untuk mem-flash kode.
### Opsi B: MicroPython (Thonny IDE & Custom Firmware)
 1. Flash ESP32 dengan custom firmware MicroPython yang memiliki C-Module A2DP. Gunakan esptool:
   esptool.py --chip esp32 --port /dev/ttyUSB0 write_flash -z 0x1000 firmware.bin
 2. Instal **Thonny IDE**.
 3. Buka Thonny, masuk ke menu **Tools > Options > Interpreter**.
 4. Pilih **MicroPython (ESP32)** pada opsi interpreter dan pilih port serial ESP32 yang sesuai.
 5. Buka file main.py dari repositori ini di Thonny.
 6. Klik **File > Save as**, pilih **MicroPython device**, dan simpan dengan nama main.py agar dieksekusi otomatis saat *boot*.
## 2. Perbandingan Hardware Konversi Audio
Tabel perbandingan teknis antara penggunaan DAC internal bawaan ESP32 dan modul DAC I2S eksternal.

| Parameter | Internal DAC (ESP32 Built-in) | Eksternal DAC (MAX98357A I2S) |
| :--- | :--- | :--- |
| **Kelebihan** | Komponen minimal, tidak butuh IC tambahan, wiring sederhana | Kualitas audio presisi (16-bit murni), output daya tinggi (built-in Amp Kelas D 3W) |
| **Kekurangan** | Output hanya 8-bit, rentan *quantization noise* (desis), output daya sinyal sangat lemah | Membutuhkan modul hardware tambahan, wiring jalur digital I2S harus presisi | <br> ## 3. Percobaan 1: Penggunaan Internal DAC <br> Percobaan ini merupakan *proof-of-concept* fungsionalitas A2DP sink pada ESP32 tanpa antarmuka kendali tambahan. Output langsung menggunakan pin DAC bawaan yang dilewatkan pada *passive low-pass filter* (resistor). 
### Daftar Kebutuhan Komponen
| Komponen | Jumlah |
| :--- | :--- |
| **ESP32 Dev Board** | 1 |
| **Resistor 12k Ohm** | 2 |
| **Modul Amplifier Eksternal** | 1 |
| **Speaker Pasif** | 1 | 
### Skema Sambungan (Wiring) <br> Resistor 12k Ohm dipasang secara seri pada jalur output audio. Tidak ada tombol atau *rotary encoder* pada tahap ini.
| ESP32 Pin Output | Pemasangan Pasif | Tujuan | Keterangan |
| :--- | :--- | :--- | :--- |
| **GPIO 25** | Seri dengan **Resistor 12k Ohm ** | Input Amplifier (L / Kiri) | Output DAC Channel 1 (8-Bit) |
| **GPIO 26** | Seri dengan **Resistor 12k Ohm** | Input Amplifier (R / Kanan) | Output DAC Channel 2 (8-Bit) |
| **GND** | Langsung | GND Amplifier | Referensi Ground |

*(Bersambung ke Part 2: Solusi DAC Eksternal MAX98357A beserta skema antarmuka UI)*