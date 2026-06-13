import machine
import time
import _thread

# Mengimpor C-Module kustom yang telah di-compile ke dalam firmware
# Modul ini menjembatani ESP-IDF Bluetooth Stack dengan lingkungan MicroPython
import a2dp_internal 

def audio_sink_task():
    print("[Core 0] Memulai mesin dekode A2DP...")
    
    # Inisialisasi pin DAC internal ESP32 (Resolusi 8-bit)
    dac_left = machine.DAC(machine.Pin(25))
    dac_right = machine.DAC(machine.Pin(26))
    
    # Mengikat objek hardware DAC ke dalam engine pemroses bahasa C
    a2dp_internal.init_internal_dac(dac_left, dac_right)
    
    # Mulai penyiaran nama perangkat Bluetooth
    a2dp_internal.start("ESP32_Legacy_Audio")
    
    while True:
        # Loop ini menjaga status _thread tetap hidup di Core 0
        # Seluruh pemrosesan audio (DMA/I2S internal) ditangani silikon di background
        time.sleep(1)

def main():
    print("\n=============================================")
    print("[EKSPERIMEN 1] MicroPython A2DP ke DAC Internal")
    print("=============================================")
    print("Output analog aktif di GPIO 25 (Kiri) dan GPIO 26 (Kanan).")
    print("Pastikan resistor 12k Ohm terpasang secara seri sebelum menuju amplifier.")
    
    # Mendelegasikan beban berat radio Bluetooth ke Core 0 menggunakan _thread
    # agar tidak tercekik oleh Global Interpreter Lock (GIL) MicroPython
    _thread.start_new_thread(audio_sink_task, ())
    
    # Core 1 (Main Thread) bebas dari beban pemrosesan audio
    while True:
        time.sleep(1)

if __name__ == "__main__":
    main()