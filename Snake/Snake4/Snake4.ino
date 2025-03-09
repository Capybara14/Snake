#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define dfc 300            // устанавливает длину игрового тика
#define len 100            // устанавливает начальную длину змеи
#define SnakeColor 0x07e0  // цвет змеи (rbg565)
#define AppleColor 0xf800  // цвет яблока
#define AppleMax 5         // максимальное число яблок на поле

Adafruit_ST7735 tft = Adafruit_ST7735(8, 9, 10);

byte direction;  // 0 - вниз, 1 - вправо, 2 - вверх, 3 - влево

int length;
int head_coordinates[600]{};
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
  Serial.println(headCoordinateX);
  if (millis() - timer >= difficult && direction != 4) {
    switch (direction) {
      case 0:
        flatHeadCoordinate = flatHeadCoordinate - 40;
        break;
      case 1:
        flatHeadCoordinate = flatHeadCoordinate + 1;
        break;
      case 2:
        flatHeadCoordinate = flatHeadCoordinate + 40;
        break;
      case 3:
        flatHeadCoordinate = flatHeadCoordinate - 1;
        break;
    }
    for (int i = 0; i <= length; i++) {
      head_coordinates[i] = head_coordinates[i + 1];  // передвигает старые значения головы
      if (flatHeadCoordinate == head_coordinates[i]) {
        loss();
        break;
      }                                                   // врезание в себя рестартит
      if (flatHeadCoordinate == Apples_coordinates[i]) {  // поедание яблок
        length++;
        Apples_coordinates[i] = 0;
        applesCounter--;
      }
    }
    head_coordinates[length] = flatHeadCoordinate;

    int headCoordinateXOld;
    headCoordinateXOld = headCoordinateX;

    coordinate_counter();

    if (headCoordinateY < 0 || headCoordinateY > 79) { loss(); }
    if (abs(headCoordinateX - headCoordinateXOld) > 10) { loss(); }
    tft.fillRect(headCoordinateX, headCoordinateY, 4, 4, SnakeColor);
    tft.fillRect(lastCoordinateX, lastCoordinateY, 4, 4, 0);

    timer = millis();
    n = 0;
  }

  if (millis() - timer1 >= 5000 && applesCounter < AppleMax) {  // спавнит яблоки
    randomSeed(analogRead(A7));
    FlatAppleCoordinate = random(0, 800);
    for (int i = 0; i <= length; i++) {  // не позволяет яблоку заспавнится внутри змеи
      if (FlatAppleCoordinate == head_coordinates[i]) { FlatAppleCoordinate = random(0, 800); }
    }
    for (int i = 0; i < AppleMax; i++) {  // если находит, то заполняет пустую ячейку в массиве с координатами яблок новой координатой яблока
      if (Apples_coordinates[i] == 0) {
        Apples_coordinates[i] = FlatAppleCoordinate;
        break;
      }
    }

    applesCounter++;
    timer1 = millis();
    coordinate_counter();
    tft.fillRect(AppleX, AppleY, 4, 4, AppleColor);
  }
}


void direction_reader() {
  if (digitalRead(A2) == 0) { difficult = dfc / 2; }  //обработка кнопки
  else if (digitalRead(A2) == 1) {
    difficult = dfc;
  }

  if (analogRead(A0) <= 50 && direction != 3 && n <= 2) {
    direction = 1;
    n++;
  }  //обработка джойстика VRY
  else if (analogRead(A0) >= 1000 && direction != 1 && n <= 2) {
    direction = 3;
    n++;
  }

  if (analogRead(A1) <= 50 && direction != 2 && n <= 2) {
    direction = 0;
    n++;
  }  //обработка джойстика VRX
  else if (analogRead(A1) >= 1000 && direction != 0 && n <= 2) {
    direction = 2;
    n++;
  }
}


void coordinate_counter() {
  headCoordinateX = (flatHeadCoordinate % 40) * 4;
  headCoordinateY = (flatHeadCoordinate / 40) * 4;
  lastCoordinateX = (head_coordinates[0] % 40) * 4;
  lastCoordinateY = (head_coordinates[0] / 40) * 4;
  AppleX = (FlatAppleCoordinate % 40) * 4;
  AppleY = (FlatAppleCoordinate / 40) * 4;
}


void loss() {
  tft.fillScreen(0xf800);
  delay(1000);
  tft.fillScreen(0);
  tft.setCursor(25, 20);
  tft.setTextColor(0xffff);
  tft.setTextSize(6);
  tft.print(length - len);
  delay(5000);
  restart();
}


void restart() {
  difficult = dfc;
  length = len;
  direction = 4;
  tft.fillScreen(0);
  tft.fillRect(80, 40, 4, 4, SnakeColor);
  flatHeadCoordinate = 420;
  for (int i = 0; i <= length; i++) { head_coordinates[i] = 0; }
  head_coordinates[length] = 420;
  applesCounter = 0;
  coordinate_counter();
  n = 0;
  loop();
}