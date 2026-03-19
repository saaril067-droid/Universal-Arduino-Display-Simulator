#include <avr/pgmspace.h>

// Карта 8x8 в PROGMEM (экономим оперативку)
const uint8_t worldMap[] PROGMEM = {
  0b11111111,
  0b10000001,
  0b10110001,
  0b10000001,
  0b10011001,
  0b10000001,
  0b10000001,
  0b11111111
};

float posX = 3.0, posY = 3.0;
float dirX = -1.0, dirY = 0.0;
float planeX = 0.0, planeY = 0.66;

void setup() {
  Serial.begin(115200); // Возвращаем стандарт
}

void loop() {
  // Вращение камеры
  float rotSpeed = 0.05;
  float oldDirX = dirX;
  dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
  dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
  float oldPlaneX = planeX;
  planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
  planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);

  // Отрисовка кадра (1024 байта)
  for (uint8_t page = 0; page < 8; page++) {
    for (uint8_t x = 0; x < 128; x++) {
      float cameraX = 2.0 * x / 128.0 - 1.0;
      float rayDirX = dirX + planeX * cameraX;
      float rayDirY = dirY + planeY * cameraX;

      int mapX = (int)posX;
      int mapY = (int)posY;

      float deltaDistX = abs(1.0 / rayDirX);
      float deltaDistY = abs(1.0 / rayDirY);
      float sideDistX, sideDistY;
      int stepX, stepY;

      if (rayDirX < 0) { stepX = -1; sideDistX = (posX - mapX) * deltaDistX; }
      else { stepX = 1; sideDistX = (mapX + 1.0 - posX) * deltaDistX; }
      if (rayDirY < 0) { stepY = -1; sideDistY = (posY - mapY) * deltaDistY; }
      else { stepY = 1; sideDistY = (mapY + 1.0 - posY) * deltaDistY; }

      int hit = 0, side;
      while (hit == 0) {
        if (sideDistX < sideDistY) { sideDistX += deltaDistX; mapX += stepX; side = 0; }
        else { sideDistY += deltaDistY; mapY += stepY; side = 1; }
        if ((pgm_read_byte(&worldMap[mapY]) >> (7 - mapX)) & 1) hit = 1;
      }

      float perpWallDist = (side == 0) ? (sideDistX - deltaDistX) : (sideDistY - deltaDistY);
      int lineHeight = (int)(64 / perpWallDist);
      int drawStart = 32 - (lineHeight / 2);
      int drawEnd = 32 + (lineHeight / 2);

      uint8_t columnData = 0;
      int yMin = page * 8;
      
      // Проверка пикселей только в текущей странице (8 пикселей высотой)
      for (int bit = 0; bit < 8; bit++) {
        int y = yMin + bit;
        if (y >= drawStart && y <= drawEnd) {
          if (side == 1) columnData |= (1 << bit);
          else if ((x + y) % 2 == 0) columnData |= (1 << bit);
        }
      }
      
      Serial.write(columnData);
    }
  }
  
  // Даем эмулятору время "продышаться" между кадрами
  delay(15); 
}