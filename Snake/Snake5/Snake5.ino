#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define dfc 250            // устанавливает длину игрового тика
#define len 5              // устанавливает начальную длину змеи
#define SnakeColor 0x07e0  // цвет змеи (rbg565)
#define AppleColor 0xf800  // цвет яблока
#define AppleMax 5         // максимальное число яблок на поле
#define plus 1             // столько получает очков за сбор каждого яблока

Adafruit_ST7735 tft = Adafruit_ST7735(8, 9, 10);

byte direction;  // 0 - вниз, 1 - вправо, 2 - вверх, 3 - влево

int length = 0;
int head_coordinates[512]{};
int Apples_coordinates[AppleMax]{};

int headCoordinateX;
int headCoordinateY;

int lastCoordinateX;
int lastCoordinateY;

int AppleX;
int AppleY;
int FlatAppleCoordinate;

int flatHeadCoordinate;
int difficult;
byte applesCounter;
unsigned long timer = 0;
unsigned long timer1 = 0;

byte n = 0;


void setup() {
  Serial.begin(9600);
  tft.initR(INITR_MINI160x80_PLUGIN);
  tft.setRotation(3);
  pinMode(A2, INPUT_PULLUP);
  restart();
}


void loop() {

  direction_reader();

  if (millis() - timer >= difficult && direction != 4) {
    switch (direction) {
      case 0:
        flatHeadCoordinate = flatHeadCoordinate - 32;
        break;
      case 1:
        flatHeadCoordinate = flatHeadCoordinate + 1;
        break;
      case 2:
        flatHeadCoordinate = flatHeadCoordinate + 32;
        break;
      case 3:
        flatHeadCoordinate = flatHeadCoordinate - 1;
        break;
    }

    int headCoordinateXOld = headCoordinateX;
    coordinate_counter();

    if (millis() - timer1 >= 5000 && applesCounter < AppleMax) {  // спавнит яблоки
      randomSeed(analogRead(A7));
      FlatAppleCoordinate = random(0, 512);
      for (int i = 0; i <= length; i++) {  // не позволяет яблоку заспавнится внутри змеи
        while (FlatAppleCoordinate == head_coordinates[i] || FlatAppleCoordinate == Apples_coordinates[i]) { FlatAppleCoordinate = random(0, 512); }
      }
      for (int i = 0; i < AppleMax; i++) {  // если находит, то заполняет пустую ячейку в массиве с координатами яблок новой координатой яблока
        if (Apples_coordinates[i] == 0) {
          Apples_coordinates[i] = FlatAppleCoordinate;
          break;
        }
      }
      timer1 = millis();
      coordinate_counter();
      tft.fillRect(AppleX, AppleY, 5, 5, AppleColor);
      applesCounter++;
    }

    for (int i = 0; i <= length; i++) {
      head_coordinates[i] = head_coordinates[i + 1];  // передвигает старые значения головы
    }
    for (int i = 0; i < length; i++) {
      if (flatHeadCoordinate == head_coordinates[i] && flatHeadCoordinate != 0) {  // врезание в себя рестартит
        loss();
        break;
      }
    }
    for (int i = 0; i <= AppleMax; i++) {  // поедание яблок
      if (flatHeadCoordinate == Apples_coordinates[i]) {
        length = length + plus;
        applesCounter--;
        Apples_coordinates[i] = 0;
      }
    }

    head_coordinates[length] = flatHeadCoordinate;

    if (headCoordinateY < 0 || headCoordinateY > 79) { loss(); }
    if (abs(headCoordinateX - headCoordinateXOld) > 10) { loss(); }
    tft.fillRect(headCoordinateX, headCoordinateY, 5, 5, SnakeColor);
    tft.fillRect(lastCoordinateX, lastCoordinateY, 5, 5, 0);

    timer = millis();
    n = 0;
  }
}



void direction_reader() {
  if (digitalRead(A2) == 0) { difficult = dfc / 2; }  //обработка кнопки
  else if (digitalRead(A2) == 1) {
    difficult = dfc;
  }

  if (analogRead(A0) <= 50 && direction != 3 && n <= 2 && direction != 1) {
    direction = 1;
    n++;
  }  //обработка джойстика VRY
  else if (analogRead(A0) >= 1000 && direction != 1 && n <= 2 && direction != 3) {
    direction = 3;
    n++;
  }

  if (analogRead(A1) <= 50 && direction != 2 && n <= 2 && direction != 0) {
    direction = 0;
    n++;
  }  //обработка джойстика VRX
  else if (analogRead(A1) >= 1000 && direction != 0 && n <= 2 && direction != 2) {
    direction = 2;
    n++;
  }
}


void coordinate_counter() {
  headCoordinateX = (flatHeadCoordinate % 32) * 5;
  headCoordinateY = (flatHeadCoordinate / 32) * 5;
  lastCoordinateX = (head_coordinates[0] % 32) * 5;
  lastCoordinateY = (head_coordinates[0] / 32) * 5;
  AppleX = (FlatAppleCoordinate % 32) * 5;
  AppleY = (FlatAppleCoordinate / 32) * 5;
}


void loss() {
  tft.fillScreen(0xf800);
  delay(1000);
  tft.fillScreen(0);
  tft.setCursor(25, 20);
  tft.setTextColor(0xffff);
  tft.setTextSize(6);
  tft.print(length - len + 1);
  delay(5000);
  restart();
}


void restart() {
  memset(Apples_coordinates, 0, AppleMax);
  memset(head_coordinates, 0, 512);
  difficult = dfc;
  length = len - 1;
  direction = 4;
  tft.fillScreen(0);
  tft.fillRect(80, 40, 5, 5, SnakeColor);
  flatHeadCoordinate = 272;
  head_coordinates[length] = 272;
  applesCounter = 0;
  n = 0;
  coordinate_counter();
}