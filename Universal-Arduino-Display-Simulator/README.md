# 📟 Universal Arduino Display Simulator (3D Edition)

**Author:** Yarik  
**Version:** 1.0 (Stable)

A software suite for emulating an OLED display (128x64) on a PC, using **Arduino Uno** as a dedicated graphics processor. The core feature is a real-time pseudo-3D engine.

---

## 🌟 Key Features

* **Real-time Raycasting**: Full 3D perspective calculation (DDA algorithm) on an 8-bit ATmega328 microcontroller.
* **Dual Interface**:
    * **Launcher (Tkinter)**: User-friendly GUI for COM-port and resolution setup.
    * **Engine (PyGame)**: High-speed rendering with post-processing effects.
* **Display Modes (Hotkeys)**:
    * `1` — **Fast**: Direct pixel rendering.
    * `2` — **Dot**: Emulates the physical grid of an OLED panel.
    * `3` — **Bloom**: Phosphor glow effect (highly recommended for 3D).

---

## 🛠 Technical Stack

* **Arduino (C++)**:
    * Optimized `float` mathematics.
    * Level maps stored in `PROGMEM` to save SRAM.
    * Binary frame data transfer (1024 bytes) via Serial.
* **Python**:
    * `pyserial` — Low-level serial communication.
    * `pygame` — Graphics engine for data visualization.
    * `tkinter` — Windowed launcher interface.

---

## 🚀 Quick Start

### 1. Hardware Setup
1. Connect your **Arduino Uno** or **Nano** to your PC.
2. Upload the `Uno_3D_Engine.ino` sketch via Arduino IDE.
3. Ensure the baud rate is set to `115200` in the code.

### 2. Running the Simulator
Install dependencies if needed:
```bash
pip install pygame pyserial