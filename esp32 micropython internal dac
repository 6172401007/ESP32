```python
import machine
import time
import _thread
# Asumsi: Kita memiliki modul C kustom 'a2dp_internal'
# yang mampu memetakan audio stream langsung ke DAC hardware
import a2dp_internal 

# --- KONFIGURASI INTERNAL DAC (GPIO 25 & 26) ---
# Menggunakan Internal DAC ESP32 (8-bit)
# Pin 25 dan 26 adalah output analog bawaan
dac1 = machine.DAC(machine.Pin(25))
dac2 = machine.DAC(machine.Pin(26))

# --- KONFIGURASI KY-040 ---
clk = machine.Pin(18, machine.Pin.IN, machine.Pin.PULL_UP)
dt = machine.Pin(19, machine.Pin.IN, machine.Pin.PULL_UP)
volume = 128

def encoder_handler(pin):
    global volume
    if clk.value() == 0:
        if dt.value() == 1:
            volume = min(255, volume + 8)
        else:
            volume = max(0, volume - 8)
        # Mengatur gain langsung ke engine A2DP
        a2dp_internal.set_gain(volume)

clk.irq(trigger=machine.Pin.IRQ_FALLING, handler=encoder_handler)

# --- THREAD DEKODER (CORE 0) ---
# Menggunakan mesin C untuk menangani buffer audio
def audio_sink_task():
    print("[Core 0] Memulai Bluetooth Sink (DAC Internal Mode)")
    # Menghubungkan engine ke DAC internal ESP32
    a2dp_internal.init_internal_dac(dac1, dac2)
    a2dp_internal.start("ESP32_Legacy_Audio")
    
    while True:
        # Engine C secara otomatis mengirim sample ke DAC
        # tanpa intervensi Python
        time.sleep(1)

# --- MAIN (CORE 1) ---
def main():
    print("[Core 1] Inisialisasi Sistem...")
    _thread.start_new_thread(audio_sink_task, ())
    
    print("Sistem Berjalan dengan DAC 8-bit Internal.")
    while True:
        time.sleep(1)

if __name__ == "__main__":
    main()

```
