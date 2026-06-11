```python
import machine
import time
import _thread
import a2dp_i2s

# --- KONFIGURASI PIN I2S (MAX98357A) ---
# Jalur Digital Murni: BCLK, LRC, DIN
i2s_config = {
    "bck": 27,
    "ws": 26,
    "din": 25
}

# --- KONFIGURASI KY-040 (ROTARY ENCODER) ---
clk = machine.Pin(18, machine.Pin.IN, machine.Pin.PULL_UP)
dt = machine.Pin(19, machine.Pin.IN, machine.Pin.PULL_UP)
volume_target = 64

def encoder_handler(pin):
    global volume_target
    # Deteksi arah putaran sederhana
    if clk.value() == 0:
        if dt.value() == 1:
            volume_target = min(127, volume_target + 5)
        else:
            volume_target = max(0, volume_target - 5)
        
        # Kirim volume baru ke engine C++ tanpa mengganggu stream audio
        a2dp_i2s.set_volume(volume_target)

clk.irq(trigger=machine.Pin.IRQ_FALLING, handler=encoder_handler)

# --- THREAD AUDIO (CORE 0) ---
# Menggunakan Core 0 untuk sinkronisasi radio-ke-I2S yang berat
def audio_sink_core0():
    print("[Core 0] Memulai Engine A2DP I2S...")
    
    # Inisialisasi bus I2S di level rendah melalui wrapper C
    a2dp_i2s.init_i2s(i2s_config["bck"], i2s_config["ws"], i2s_config["din"])
    a2dp_i2s.start_sink("ESP32_HiFi_Digital")
    
    while True:
        # Loop ini menjaga status koneksi Bluetooth tetap hidup
        # Data audio dipindahkan oleh DMA secara otomatis
        time.sleep(1)

# --- MAIN (CORE 1) ---
def main():
    print("[Core 1] Memulai Sistem Kendali...")
    
    # Menjalankan Engine Audio di Core 0
    _thread.start_new_thread(audio_sink_core0, ())
    
    print("Sistem Hi-Fi I2S Siap.")
    while True:
        # Core 1 bebas untuk tugas UI, logging, atau sensor lain
        time.sleep(1)

if __name__ == "__main__":
    main()

```
