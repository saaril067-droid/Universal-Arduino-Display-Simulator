#include <Arduino.h>

// Карта лабиринта (можно сделать больше, чем на Uno!)
const uint8_t worldMap[10] = {
  0b11111111, 0b11000000,
  0b10000101, 0b00000000,
  0b10110101, 0b00000000,
  0b10000001, 0b00000000,
  0b10111101, 0b00000000,
  0b10100001, 0b00000000,
  0b10001001, 0b00000000,
  0b11111111, 0b11000000
};

float posX = 3.5, posY = 3.5;
float dirX = -1.0, dirY = 0.0;
float planeX = 0.0, planeY = 0.66;

void setup() {
  // Ставим высокую скорость для ESP
  Serial.begin(921600); 
}

void loop() {
  // Вращение камеры (на ESP оно будет очень плавным)
  float rotSpeed = 0.04;
  float oldDirX = dirX;
  dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
  dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
  float oldPlaneX = planeX;
  planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
  planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);

  // Создаем буфер кадра прямо в памяти ESP (1024 байта)
  uint8_t frameBuffer[1024];
  memset(frameBuffer, 0, 1024);

  for (int x = 0; x < 128; x++) {
    float cameraX = 2 * x / 128.0 - 1;
    float rayDirX = dirX + planeX * cameraX;
    float rayDirY = dirY + planeY * cameraX;

    int mapX = int(posX);
    int mapY = int(posY);

    float sideDistX, sideDistY;
    float deltaDistX = abs(1 / rayDirX);
    float deltaDistY = abs(1 / rayDirY);
    
    int stepX, stepY;
    int hit = 0, side;

    if (rayDirX < 0) { stepX = -1; sideDistX = (posX - mapX) * deltaDistX; }
    else { stepX = 1; sideDistX = (mapX + 1.0 - posX) * deltaDistX; }
    if (rayDirY < 0) { stepY = -1; sideDistY = (posY - mapY) * deltaDistY; }
    else { stepY = 1; sideDistY = (mapY + 1.0 - posY) * deltaDistY; }

    while (hit == 0) {
      if (sideDistX < sideDistY) { sideDistX += deltaDistX; mapX += stepX; side = 0; }
      else { sideDistY += deltaDistY; mapY += stepY; side = 1; }
      
      // Читаем карту (учитываем, что ESP может больше)
      if (mapX < 0 || mapX >= 8 || mapY < 0 || mapY >= 8) hit = 1;
      else if ((worldMap[mapY] >> (7 - mapX)) & 1) hit = 1;
    }

    float perpWallDist;
    if (side == 0) perpWallDist = (sideDistX - deltaDistX);
    else          perpWallDist = (sideDistY - deltaDistY);

    int lineHeight = (int)(64 / perpWallDist);
    int drawStart = -lineHeight / 2 + 32;
    int drawEnd = lineHeight / 2 + 32;

    // Заполняем буфер кадра
    for (int y = 0; y < 64; y++) {
      if (y >= drawStart && y <= drawEnd) {
        int page = y / 8;
        int bit = y % 8;
        
        bool draw = false;
        if (side == 1) draw = true; // Одна сторона залита полностью
        else if ((x + y) % 2 == 0) draw = true; // Вторая сторона - сетка

        // Добавляем "шум" для текстуры на расстоянии
        if (perpWallDist > 4.0 && random(0, 10) > 7) draw = false;

        if (draw) frameBuffer[page * 128 + x] |= (1 << bit);
      }
    }
  }

  // Отправляем весь кадр одной пачкой — так быстрее!
  Serial.write(frameBuffer, 1024);
  
  // Минимальная задержка для стабильности
  delay(5);
}