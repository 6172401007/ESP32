#include "BluetoothA2DPSink.h"
#include <Arduino.h>
#include "AiEsp32RotaryEncoder.h"

// --- OBJEK UTAMA ---
BluetoothA2DPSink a2dp_sink;

// --- KONFIGURASI KY-040 ROTARY ENCODER ---
#define ROTARY_ENCODER_A_PIN 18      // CLK
#define ROTARY_ENCODER_B_PIN 19      // DT
#define ROTARY_ENCODER_BUTTON_PIN 21 // SW
#define ROTARY_ENCODER_VCC_PIN -1    // Dibiarkan -1 jika langsung dihubungkan ke 3.3V
#define ROTARY_ENCODER_STEPS 4
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

// --- VARIABEL LOGIKA VOLUME (EMA FILTER) ---
int target_volume = 64;
float actual_volume = 64.0;
int saved_volume = 64;
bool is_muted = false;

// --- FUNGSI INTERUPSI HARDWARE (ISR) ---
// Memastikan tidak ada putaran kenop yang terlewat
void IRAM_ATTR readEncoderISR() {
    rotaryEncoder.readEncoder_ISR();
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=============================================");
    Serial.println("[EKSPERIMEN 2] I2S Digital Audio (MAX98357A)");
    Serial.println("=============================================");

    // 1. INISIALISASI ROTARY ENCODER
    rotaryEncoder.begin();
    rotaryEncoder.setup(readEncoderISR);
    rotaryEncoder.setBoundaries(0, 127, false); 
    rotaryEncoder.setEncoderValue(target_volume);
    rotaryEncoder.setAcceleration(50);

    // 2. KONFIGURASI PIN I2S DIGITAL
    // Mengarahkan aliran data audio murni (16-bit) ke pin eksternal
    i2s_pin_config_t my_pin_config = {
        .bck_io_num = 27,
        .ws_io_num = 26,
        .data_out_num = 25,
        .data_in_num = I2S_PIN_NO_CHANGE // Tidak menggunakan input mikrofon
    };
    a2dp_sink.set_pin_config(my_pin_config);
    
    // Set volume awal
    a2dp_sink.set_volume(target_volume);

    // 3. JALANKAN ENGINE BLUETOOTH
    a2dp_sink.start("ESP32_HiFi_Speaker");
    
    Serial.println("Sistem Digital Siap! Hubungkan Bluetooth Anda.");
}

void loop() {
    // --- 1. PEMBACAAN ARAH PUTARAN (VOLUME) ---
    if (rotaryEncoder.encoderChanged()) {
        target_volume = rotaryEncoder.readEncoder();
        
        // Fitur Auto-Unmute: Memutar kenop akan membatalkan mode mute
        if (is_muted) {
            is_muted = false;
            Serial.println("Auto-Unmute Aktif.");
        }
    }

    // --- 2. PEMBACAAN KLIK KENOP (SOFT MUTE) ---
    if (rotaryEncoder.isEncoderButtonClicked()) {
        if (is_muted) {
            // Unmute: Kembalikan ke volume sebelum di-mute
            target_volume = saved_volume;
            rotaryEncoder.setEncoderValue(target_volume);
            is_muted = false;
            Serial.println("Speaker Diaktifkan (Unmute).");
        } else {
            // Mute: Simpan volume saat ini, lalu turunkan target ke 0
            saved_volume = target_volume;
            target_volume = 0;
            rotaryEncoder.setEncoderValue(0