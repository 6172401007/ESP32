import machine
import time
import _thread

# Mengimpor modul bahasa C kustom yang terintegrasi di dalam firmware.
# Modul ini menangani transfer data DMA berkecepatan tinggi dari radio ke bus I2S.
import a2dp_i2s

# --- KONFIGURASI KY-040 (ROTARY ENCODER) ---
clk = machine.Pin(18, machine.Pin.IN, machine.Pin.PULL_UP)
dt = machine.Pin(19, machine.Pin.IN, machine.Pin.PULL_UP)
sw = machine.Pin(21, machine.Pin.IN, machine.Pin.PULL_UP)

# Variabel State untuk Logika Volume (EMA Filter)
target_volume = 64
actual_volume = 64.0
saved_volume = 64
is_muted = False

# --- FUNGSI INTERUPSI HARDWARE (ISR) ---
def encoder_isr(pin):
    global target_volume, is_muted
    # Mendeteksi arah putaran secara instan
    if clk.value() == 0:
        if dt.value() == 1:
            target_volume = min(127, target_volume + 4) # Volume Naik
        else:
            target_volume = max(0, target_volume - 4)   # Volume Turun
            
        # Fitur Auto-Unmute saat kenop diputar
        if is_muted:
            is_muted = False
            print("Auto-Unmute Aktif.")

def sw_isr(pin):
    global target_volume, is_muted, saved_volume
    time.sleep_ms(20) # Debounce mekanis sederhana
    
    if sw.value() == 0:
        if is_muted:
            target_volume = saved_volume
            is_muted = False
            print("Speaker Diaktifkan (Unmute).")
        else:
            saved_volume = target_volume
            target_volume = 0
            is_muted = True
            print("Speaker Disenyapkan (Mute).")

# Mengikat interupsi ke pin fisik
clk.irq(trigger=machine.Pin.IRQ_FALLING, handler=encoder_isr)
sw.irq(trigger=machine.Pin.IRQ_FALLING, handler=sw_isr)

# --- THREAD 1: LOGIKA ANTARMUKA & EMA (CORE 1) ---
def ui_logic_thread():
    global actual_volume, target_volume
    print("[Core 1] Thread Antarmuka & Kalkulasi Volume Berjalan...")
    
    while True:
        # Algoritma Exponential Moving Average (Soft Fading)
        # Sinyal volume akan merayap menuju angka target_volume
        if abs(actual_volume - target_volume) > 0.5:
            actual_volume += (target_volume - actual_volume) * 0.1
            
            # Melemparkan instruksi volume terbaru ke engine pemroses C
            a2dp_i2s.set_volume(int(actual_volume))
            
        # Tidur 10ms menjaga kestabilan memori dan Watchdog Timer
        time.sleep_ms(10)

# --- THREAD 0: AUDIO ENGINE MAIN LOOP (CORE 0) ---
def main():
    print("\n=============================================")
    print("[EKSPERIMEN 2] I2S Digital Audio (MAX98357A)")
    print("=============================================")
    
    # 1. Jalankan UI Thread di Core kedua agar tidak mengganggu audio
    _thread.start_new_thread(ui_logic_thread, ())
    
    # 2. Inisialisasi Bus I2S Digital di Engine C
    # Pin: BCLK=27, LRC=26, DIN=25
    a2dp_i2s.init_i2s(bck=27, ws=26, din=25)
    
    # 3. Mulai transmisi sink Bluetooth
    a2dp_i2s.start_sink("ESP32_HiFi_Speaker")
    
    print("Sistem Digital Siap! Silakan hubungkan perangkat Bluetooth Anda.")
    
    while True:
        # Main thread mempertahankan status perangkat tetap hidup
        time.sleep(1)

if __name__ == "__main__":
    main()