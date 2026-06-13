#include "BluetoothA2DPSink.h"
#include <Arduino.h>

// Deklarasi objek utama dari library pschatzmann
BluetoothA2DPSink a2dp_sink;

void setup() {
    Serial.begin(115200);
    Serial.println("\n=============================================");
    Serial.println("[EKSPERIMEN 1] Bluetooth Audio ke DAC Internal");
    Serial.println("=============================================");

    // Konfigurasi register tingkat rendah untuk memaksa I2S 
    // agar merutekan aliran data ke DAC silikon bawaan ESP32
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
        .sample_rate = 44100, 
        .bits_per_sample = (i2s_bits_per_sample_t) 16, // Hardware akan menurunkan resolusi ke 8-bit secara fisik
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t) I2S_COMM_FORMAT_STAND_MSB,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = true
    };
    
    // Terapkan konfigurasi ke dalam engine C++
    a2dp_sink.set_i2s_config(i2s_config);
    
    // Mulai penyiaran nama Bluetooth (Bisa diubah sesuai keinginan)
    a2dp_sink.start("ESP32_Legacy_Audio");
    
    Serial.println("Sistem Siap! Silakan hubungkan perangkat Anda.");
    Serial.println("Catatan: Output analog akan keluar melalui GPIO 25 (L) dan GPIO 26 (R).");
    Serial.println("Pastikan resistor 12k Ohm terpasang secara seri sebelum masuk ke amplifier.");
}

void loop() {
    // Loop dibiarkan hampir kosong. 
    // Pada arsitektur C++ ini, library A2DP berjalan menggunakan 
    // sistem Task (FreeRTOS) di latar belakang pada Core 0 secara asinkron.
    delay(1000); 
}