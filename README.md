# Panduan Proyek ESP32 Bluetooth & Wlan Speaker
Dokumentasi ini berisi instruksi teknis pembuatan receiver audio Bluetooth menggunakan ESP32. Proyek mendukung dua arsitektur perangkat lunak: C++ dan MicroPython.
## 🌟 Pengakuan & Atribusi (Credits)
Proyek ini sangat bergantung pada kapabilitas dekode radio Bluetooth Classic tingkat rendah yang luar biasa. Atribusi khusus dan ucapan terima kasih yang sebesar-besarnya diberikan kepada:
 * **Phil Schatzmann (pschatzmann)**: Pembuat pustaka **ESP32-A2DP**. Pustaka inti inilah yang memungkinkan pemrosesan I2S dan dekompresi SBC Codec dapat berjalan dengan mulus di arsitektur ESP32. Seluruh sistem C++ dan *wrapper* MicroPython pada proyek ini dibangun dengan menjadikan pustaka tersebut sebagai mesin pemroses utama (*core engine*).
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
| **Kekurangan** | Output hanya 8-bit, rentan *quantization noise* (desis), output daya sinyal sangat lemah | Membutuhkan modul hardware tambahan, wiring jalur digital I2S harus presisi |
## 3. Percobaan 1: Penggunaan Internal DAC 
Percobaan ini merupakan *proof-of-concept* fungsionalitas A2DP sink pada ESP32 tanpa antarmuka kendali tambahan. Output langsung menggunakan pin DAC bawaan yang dilewatkan pada *passive low-pass filter* (resistor). <br> ### Daftar Kebutuhan Komponen
| Komponen | Jumlah |
| :--- | :--- |
| **ESP32 Dev Board** | 1 |
| **Resistor 12k Ohm** | 2 |
| **Modul Amplifier Eksternal** | 1 |
| **Speaker Pasif** | 1 | 
### Skema Sambungan (Wiring) 
Resistor 12k Ohm dipasang secara seri pada jalur output audio. Tidak ada tombol atau *rotary encoder* pada tahap ini.
| ESP32 Pin Output | Pemasangan Pasif | Tujuan | Keterangan |
| :--- | :--- | :--- | :--- |
| **GPIO 25** | Seri dengan **Resistor 12k \Omega** | Input Amplifier (L / Kiri) | Output DAC Channel 1 (8-Bit) |
| **GPIO 26** | Seri dengan **Resistor 12k \Omega** | Input Amplifier (R / Kanan) | Output DAC Channel 2 (8-Bit) |
| **GND** | Langsung | GND Amplifier | Referensi Ground |

## 4. Implementasi Eksternal DAC (I2S) dan Kendali Volume
Bagian ini mendokumentasikan implementasi akhir menggunakan transmisi digital murni melalui bus I2S menuju modul DAC eksternal, lengkap dengan antarmuka kendali volume fisik. Penggunaan resistor 12k Ohm dari percobaan sebelumnya ditiadakan sepenuhnya.
### Daftar Kebutuhan Komponen

| Komponen | Jumlah | Keterangan |
| :--- | :--- | :--- |
| **ESP32 Dev Board** | 1 | Mikrokontroler utama |
| **MAX98357A Module** | 1 | I2S DAC dan Amplifier Kelas D (Maksimal 3W) |
| **KY-040 Rotary Encoder** | 1 | Modul antarmuka kendali volume digital |
| **Speaker Pasif** | 1 | Impedansi 4 - 8 Ohm | <br> ### Skema Sambungan (Wiring) <br> Koneksi fisik dibagi menjadi dua segmen independen: jalur transmisi data I2S dan jalur logika antarmuka volume. 
#### A. Wiring I2S Audio (ESP32 ke MAX98357A) Jalur transmisi ini menggunakan logika digital 3.3V. Hubungkan secara langsung tanpa komponen pasif di tengah jalur.
| ESP32 Pin | MAX98357A Pin | Keterangan Fungsi |
| :--- | :--- | :--- |
| **GPIO 27** | BCLK | Bit Clock (Sinkronisasi per bit data) |
| **GPIO 26** | LRC | Left/Right Clock (Word Select) |
| **GPIO 25** | DIN | Data Input (Aliran sinyal audio digital 16-bit) |
| **5V / VIN** | VIN | Catu daya operasional modul DAC/Amp |
| **GND** | GND | Referensi Ground bersama | *(Catatan: Pin SD dan GAIN pada MAX98357A dibiarkan tidak terhubung. Konfigurasi default ini akan mencampur sinyal stereo menjadi mono standar dengan penguatan internal 9dB).* 
#### B. Wiring Antarmuka Volume (Rotary Encoder) 
Sumber tegangan utama untuk modul KY-040 wajib menggunakan pin 3.3V dari ESP32 untuk menghindari masuknya tegangan 5V ke dalam pin GPIO mikrokontroler.
| ESP32 Pin | Modul Eksternal | Pin Modul | Parameter Fungsi |
| :--- | :--- | :--- | :--- |
| **GPIO 18** | KY-040 | CLK | Input sinyal *Quadrature A* (Clock) |
| **GPIO 19** | KY-040 | DT | Input sinyal *Quadrature B* (Data/Arah) |
| **GPIO 21** | KY-040 | SW | Input status tombol tekan encoder |
| **3.3V** | KY-040 | + (VCC) | Suplai daya referensi *Pull-Up* |
| **GND** | KY-040 | GND | Referensi Ground KY-040 |

## 5. Logika Fungsi Kendali Volume (UI)
Perangkat lunak membaca input dari modul KY-040 untuk mengeksekusi parameter instruksi audio berikut:
 1. **Volume Control (KY-040 Putar):**
   Mendeteksi arah putaran untuk menyesuaikan nilai amplitudo audio. Kode menerapkan algoritma *Exponential Moving Average* (EMA) agar transisi perubahan volume terjadi secara halus (*Soft Fading*), mencegah lonjakan output mendadak yang dapat merusak speaker.
 2. **Soft Mute (KY-040 Klik):**
   Penekanan kenop volume memicu rutin *Mute*. Sinyal diturunkan ke nilai nol secara bertahap (*ramp-down*) dan tidak dipotong paksa. Memutar kenop secara otomatis akan membatalkan status *Mute* dan mengembalikan volume ke level sesuai putaran baru (*Auto-Unmute*).

## 6. Manajemen Daya (Deep Sleep) dan Multi-Mode Transmisi
Fase akhir dari eksperimen ini adalah penambahan fungsionalitas penghematan daya menggunakan fitur Ultra Low Power (ULP) bawaan ESP32 dan kemampuan perpindahan mode transmisi dari Bluetooth A2DP ke WLAN (Web Radio). Seluruh logika ini dikendalikan oleh satu tombol fisik (*Push Button*).
### Komponen Tambahan

| Komponen | Jumlah | Keterangan |
| :--- | :--- | :--- |
| **Push Button (Tactile)** | 1 | Saklar sesaat (*momentary switch*) untuk pemicu interupsi perangkat keras | 
### Skema Sambungan (Wiring) Push Button 
Pin GPIO 12 dipilih secara spesifik karena pin ini merupakan pin RTC (Real-Time Clock) yang mendukung pemicu *Wakeup* dari status *Deep Sleep* (ext0_wakeup).
| ESP32 Pin | Komponen Eksternal | Pin Komponen | Parameter Fungsi |
| :--- | :--- | :--- | :--- |
| **GPIO 12** | Push Button | Kaki 1 | Input logika kendali mode & pemicu interupsi RTC Wakeup |
| **GND** | Push Button | Kaki 2 | Memberikan logika LOW (0V) saat tombol ditekan |

*(Catatan: Konfigurasi kode harus mengaktifkan internal Pull-Up pada GPIO 12. Tombol akan memicu sinyal LOW / *Active-Low*).*
### Logika Eksekusi Push Button & ULP
Sistem akan menghitung durasi penekanan tombol (*debounce* dan *timer*) untuk mengeksekusi tiga instruksi berbeda:
 1. **Toggle Mode / Soft Reboot (Tekan Singkat < 2 Detik):**
   * Saat tombol ditekan singkat, ESP32 akan menulis flag *state* mode berikutnya (Bluetooth atau WLAN) ke dalam Non-Volatile Storage (NVS) atau memori Flash.
   * Setelah flag tersimpan, sistem mengeksekusi perintah *Soft Reboot* (ESP.restart() atau machine.reset()).
   * Saat *booting* kembali, ESP32 membaca flag dari NVS dan memutuskan apakah akan memuat modul Bluetooth A2DP Sink atau melakukan koneksi ke jaringan WiFi untuk *streaming* Web Radio.
 2. **Mode Standby / Deep Sleep (Tekan Tahan > 3 Detik):**
   * Sistem mendeteksi penekanan panjang dan memulai rutin terminasi.
   * Transmisi radio (Bluetooth/WiFi) dan bus I2S dihentikan.
   * Perintah masuk ke *Deep Sleep* dipanggil. CPU utama dimatikan dan konsumsi daya sistem akan turun drastis ke tingkat mikroampere (\mu A), menyisakan hanya *RTC Coprocessor* yang menyala.
 3. **Hardware Wakeup (Dari status Deep Sleep):**
   * Sebelum masuk ke mode *Deep Sleep*, sistem dikonfigurasi dengan perintah esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, 0).
   * Saat ESP32 dalam keadaan tertidur, menekan Push Button yang sama akan memberikan sinyal LOW ke RTC GPIO 12.
   * Sinyal ini secara instan membangunkan mikrokontroler dari *Deep Sleep* dan sistem akan melakukan siklus *booting* normal.
## 7. Lisensi Proyek
Proyek ini menggunakan lisensi *Open Source*.
**MIT License**
Copyright (c) 2026 Clement Korinthian
Dengan ini diberikan izin, secara cuma-cuma, kepada siapa pun yang mendapatkan salinan perangkat lunak ini dan file dokumentasi terkait ("Perangkat Lunak"), untuk memperlakukan Perangkat Lunak tanpa batasan, termasuk namun tidak terbatas pada hak untuk menggunakan, menyalin, memodifikasi, menggabungkan, menerbitkan, mendistribusikan, mensublisensikan, dan/atau menjual salinan Perangkat Lunak, dan untuk mengizinkan orang kepada siapa Perangkat Lunak ini diberikan untuk melakukan hal tersebut, dengan tunduk pada ketentuan berikut:
Pemberitahuan hak cipta di atas dan pemberitahuan izin ini harus disertakan dalam semua salinan atau bagian substansial dari Perangkat Lunak.
PERANGKAT LUNAK DISEDIAKAN "SEBAGAIMANA ADANYA", TANPA JAMINAN APAPUN, TERSURAT MAUPUN TERSIRAT, TERMASUK NAMUN TIDAK TERBATAS PADA JAMINAN KELAYAKAN UNTUK DIPERDAGANGKAN, KESESUAIAN UNTUK TUJUAN TERTENTU DAN TIDAK ADANYA PELANGGARAN. DALAM HAL APAPUN PENULIS ATAU PEMEGANG HAK CIPTA TIDAK BERTANGGUNG JAWAB ATAS KLAIM, KERUSAKAN ATAU KEWAJIBAN LAINNYA, BAIK DALAM TINDAKAN KONTRAK, KESALAHAN ATAU LAINNYA, YANG TIMBUL DARI, DARI ATAU SEHUBUNGAN DENGAN PERANGKAT LUNAK ATAU PENGGUNAAN ATAU TRANSAKSI LAINNYA DALAM PERANGKAT LUNAK.
