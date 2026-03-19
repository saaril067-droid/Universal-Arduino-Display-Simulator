"""
Universal Arduino Display Simulator
Full Source Code - Stable Release
Author: Yarik
"""

import pygame
import serial
import serial.tools.list_ports
import sys
import tkinter as tk
from tkinter import ttk, messagebox

class Launcher:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("Simulator Launcher")
        self.root.geometry("350x450")
        self.root.resizable(False, False)
        self.config = None

        tk.Label(self.root, text="DISPLAY SETTINGS", font=("Arial", 12, "bold")).pack(pady=20)

        # Выбор порта
        tk.Label(self.root, text="Select Port:").pack()
        self.port_var = tk.StringVar()
        ports = [p.device for p in serial.tools.list_ports.comports()]
        self.port_cb = ttk.Combobox(self.root, textvariable=self.port_var, values=ports, state="readonly")
        self.port_cb.pack(pady=5)
        if ports: self.port_cb.current(0)

        # Выбор разрешения
        tk.Label(self.root, text="Resolution:").pack()
        self.res_var = tk.StringVar(value="OLED 128x64")
        self.res_cb = ttk.Combobox(self.root, textvariable=self.res_var, values=["OLED 128x64", "TFT 350x250"], state="readonly")
        self.res_cb.pack(pady=5)

        # Выбор режима цвета
        tk.Label(self.root, text="Mode:").pack()
        self.mode_var = tk.StringVar(value="Monochrome")
        self.mode_cb = ttk.Combobox(self.root, textvariable=self.mode_var, values=["Monochrome", "Full Color (RGB565)"], state="readonly")
        self.mode_cb.pack(pady=5)

        tk.Button(self.root, text="START ENGINE", command=self.confirm, bg="#27ae60", fg="white", font=("Arial", 10, "bold"), width=20, height=2).pack(pady=40)

    def confirm(self):
        if not self.port_var.get():
            messagebox.showerror("Error", "Please select a COM port!")
            return
        res = (128, 64) if "128x64" in self.res_var.get() else (350, 250)
        self.config = {
            "port": self.port_var.get(),
            "res": res,
            "is_mono": "Monochrome" in self.mode_var.get()
        }
        self.root.destroy()

    def run(self):
        self.root.mainloop()
        return self.config

class Engine:
    def __init__(self, config):
        pygame.init()
        self.width, self.height = config["res"]
        self.is_mono = config["is_mono"]
        self.scale = 8 if self.width == 128 else 3
        
        self.screen = pygame.display.set_mode((self.width * self.scale, self.height * self.scale))
        pygame.display.set_caption(f"Arduino Simulator - {self.width}x{self.height}")
        
        try:
            # Открываем порт. Скорость 115200 — стандарт для Uno/ESP
            self.ser = serial.Serial(config["port"], 115200, timeout=0)
        except Exception as e:
            print(f"Serial Error: {e}")
            sys.exit()

        # Размер кадра в байтах
        if self.is_mono:
            self.frame_size = (self.width * self.height) // 8
        else:
            self.frame_size = (self.width * self.height) * 2

        self.quality = 2 # 1: Fast, 2: Dot, 3: Bloom
        self.clock = pygame.time.Clock()

    def run(self):
        while True:
            for event in pygame.event.get():
                if event.type == pygame.QUIT: return
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_1: self.quality = 1
                    if event.key == pygame.K_2: self.quality = 2
                    if event.key == pygame.K_3: self.quality = 3

            # Если в буфере накопилось достаточно данных для целого кадра
            if self.ser.in_waiting >= self.frame_size:
                # Читаем ВСЁ что есть, но берем только ПОСЛЕДНИЙ кадр
                raw_data = self.ser.read(self.ser.in_waiting)
                frame = raw_data[-self.frame_size:]
                
                self.draw_frame(frame)
                pygame.display.flip()
            
            self.clock.tick(60)

    def draw_frame(self, data):
        self.screen.fill((15, 15, 15))
        s = self.scale
        
        if self.is_mono:
            # Отрисовка OLED (вертикальные байты-колонки)
            for i, byte in enumerate(data):
                x = i % self.width
                page = i // self.width
                for bit in range(8):
                    if (byte >> bit) & 1:
                        y = page * 8 + bit
                        self.render_pixel(x, y, (0, 180, 255))
        else:
            # Отрисовка TFT (RGB565 горизонтально)
            for i in range(0, len(data), 2):
                if i+1 >= len(data): break
                c = (data[i] << 8) | data[i+1]
                r = ((c >> 11) & 0x1F) << 3
                g = ((c >> 5) & 0x3F) << 2
                b = (c & 0x1F) << 3
                idx = i // 2
                self.render_pixel(idx % self.width, idx // self.width, (r, g, b))

    def render_pixel(self, x, y, color):
        px, py = x * self.scale, y * self.scale
        s = self.scale
        if self.quality == 3: # Эффект свечения (Bloom)
            pygame.draw.rect(self.screen, (color[0]//5, color[1]//5, color[2]//5), (px-1, py-1, s+2, s+2))
            pygame.draw.rect(self.screen, color, (px, py, s-1, s-1))
        elif self.quality == 2: # Точечный режим
            pygame.draw.rect(self.screen, color, (px, py, s-1, s-1))
        else: # Сплошной режим (быстрый)
            pygame.draw.rect(self.screen, color, (px, py, s, s))

if __name__ == "__main__":
    app_config = Launcher().run()
    if app_config:
        Engine(app_config).run()