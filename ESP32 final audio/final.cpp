#include <Arduino.h>
#include "BluetoothA2DPSink.h"
#include "AiEsp32RotaryEncoder.h"
#include <Preferences.h>
#include <WiFi.h>

// --- OBJEK UTAMA ---
BluetoothA2DPSink a2dp_sink;
Preferences preferences; // Untuk menyimpan state mode di memori Flash

// --- KONFIGURASI PIN ---
#define I2S_BCK_PIN 27
#define I2S_WS_PIN 26
#define I2S_DIN_PIN 25

#define ROTARY_ENCODER_A_PIN 18      // CLK
#define ROTARY_ENCODER_B_PIN 19      // DT
#define ROTARY_ENCODER_BUTTON_PIN 21 // SW
#define ROTARY_ENCODER_VCC_PIN -1    
#define ROTARY_ENCODER_STEPS 4

#define MODE_BUTTON_PIN 12           // Tombol Multifungsi & Wakeup ULP

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

// --- VARIABEL STATE SISTEM ---
int target_volume = 64;
float actual_volume = 64.0;
int saved_volume = 64;
bool is_muted = false;

// 0 = Mode Bluetooth A2DP, 1 = Mode WLAN (Web Radio)
int current_mode = 0; 

// Variabel untuk menghitung durasi tekan tombol
unsigned long button_press_start = 0;
bool is_button_pressing = false;
bool last_button_state = HIGH;

// --- FUNGSI INTERUPSI ENCODER ---
void IRAM_ATTR readEncoderISR() {
    rotaryEncoder.readEncoder_ISR();
}

// --- FUNGSI MANAJEMEN DAYA (DEEP SLEEP) ---
void goToDeepSleep() {
    Serial.println("\n[SISTEM] Menginisiasi rutinitas Shutdown...");
    
    // Matikan modul radio
    if (current_mode == 0) a2dp_sink.end();
    if (current_mode == 1) WiFi.disconnect(true);
    
    // Konfigurasi pin 12 agar bisa membangunkan ESP32 saat ditekan (LOW)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, 0);
    
    Serial.println("[SISTEM] Memasuki Deep Sleep. Tekan tombol Mode untuk Wakeup.");
    delay(500); // Beri waktu Serial untuk selesai mencetak pesan
    
    esp_deep_sleep_start();
}

// --- FUNGSI TOGGLE MODE (SOFT REBOOT) ---
void toggleOperationMode() {
    current_mode = (current_mode == 0) ? 1 : 0;
    
    // Simpan mode baru ke dalam Flash Memory (NVS)
    preferences.begin("audio_app", false);
    preferences.putInt("mode", current_mode);
    preferences.end();
    
    Serial.print("\n[SISTEM] Mode diubah. Melakukan Soft Reboot ke Mode: ");
    Serial.println(current_mode == 0 ? "BLUETOOTH" : "WLAN");
    delay(500);
    
    ESP.restart(); // Reboot mikrokontroler secara aman
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=============================================");
    Serial.println("ESP32 Hi-Fi Speaker (Multi-Mode & ULP)");
    Serial.println("=============================================");

    // 1. SETUP TOMBOL MODE
    pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);

    // 2. BACA STATE MODE DARI FLASH (NVS)
    preferences.begin("audio_app", false);
    current_mode = preferences.getInt("mode", 0); // Default ke 0 (Bluetooth) jika belum ada data
    preferences.end();

    // 3. SETUP ROTARY ENCODER
    rotaryEncoder.begin();
    rotaryEncoder.setup(readEncoderISR);
    rotaryEncoder.setBoundaries(0, 127, false); 
    rotaryEncoder.setEncoderValue(target_volume);
    rotaryEncoder.setAcceleration(50);

    // 4. INISIALISASI MODE BERDASARKAN STATE
    if (current_mode == 0) {
        Serial.println("[STARTUP] Menjalankan Mode Bluetooth A2DP Sink...");
        
        i2s_pin_config_t my_pin_config = {
            .bck_io_num = I2S_BCK_PIN,
            .ws_io_num = I2S_WS_PIN,
            .data_out_num = I2S_DIN_PIN,
            .data_in_num = I2S_PIN_NO_CHANGE
        };
        a2dp_sink.set_pin_config(my_pin_config);
        a2dp_sink.set_volume(target_volume);
        a2dp_sink.start("ESP32_HiFi_Speaker");
        
    } else if (current_mode == 1) {
        Serial.println("[STARTUP] Menjalankan Mode WLAN (Web Radio)...");
        // Di sini Anda bisa memasukkan logika koneksi WiFi.h
        // dan library streaming Web Radio (misalnya ESP32-audioI2S).
        // Sebagai contoh (Placeholder):
        // WiFi.begin("SSID", "PASSWORD");
        // audio.setPinout(I2S_BCK_PIN, I2S_WS_PIN, I2S_DIN_PIN);
    }
}

void loop() {
    // --- 1. LOGIKA TOMBOL MULTIFUNGSI (MODE & DEEP SLEEP) ---
    bool current_button_state = digitalRead(MODE_BUTTON_PIN);
    
    // Deteksi saat tombol baru saja ditekan (Transisi HIGH ke LOW)
    if (current_button_state == LOW && last_button_state == HIGH) {
        button_press_start = millis();
        is_button_pressing = true;
    } 
    // Deteksi saat tombol dilepas (Transisi LOW ke HIGH)
    else if (current_button_state == HIGH && last_button_state == LOW) {
        is_button_pressing = false;
        unsigned long press_duration = millis() - button_press_start;
        
        // Tekan singkat (Debounce 50ms, maksimal 2000ms) -> Toggle Mode
        if (press_duration > 50 && press_duration < 2000) {
            toggleOperationMode();
        }
    }

    // Deteksi tekan tahan (Lebih dari 3 detik secara terus-menerus) -> Deep Sleep
    if (is_button_pressing && (millis() - button_press_start > 3000)) {
        goToDeepSleep();
    }
    
    last_button_state = current_button_state;


    // --- 2. LOGIKA KENDALI VOLUME (ROTARY ENCODER) ---
    // Logika ini hanya dieksekusi jika sistem berada di mode Bluetooth
    if (current_mode == 0) {
        
        // Pembacaan Putaran
        if (rotaryEncoder.encoderChanged()) {
            target_volume = rotaryEncoder.readEncoder();
            if (is_muted) {
                is_muted = false;
                Serial.println("Auto-Unmute Aktif.");
            }
        }

        // Pembacaan Klik Kenop (Soft Mute)
        if (rotaryEncoder.isEncoderButtonClicked()) {
            if (is_muted) {
                target_volume = saved_volume;
                rotaryEncoder.setEncoderValue(target_volume);
                is_muted = false;
            } else {
                saved_volume = target_volume;
                target_volume = 0;
                rotaryEncoder.setEncoderValue(0);
                is_muted = true;
            }
        }

        // Algoritma EMA (Soft Fading Volume)
        if (abs(actual_volume - target_volume) > 0.5) {
            actual_volume += (target_volume - actual_volume) * 0.05;
            a2dp_sink.set_volume((int)actual_volume);
        }
    }

    delay(10); // Menjaga WDT dan kestabilan CPU
}