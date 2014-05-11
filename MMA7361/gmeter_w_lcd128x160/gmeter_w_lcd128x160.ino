//LCD
#define cs   10
#define dc   9
#define rst  8  // you can also connect this to the Arduino reset

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

//MMA7361
#define PIN_X     A0
#define PIN_Y     A1
#define PIN_Z     A2
#define PIN_SLEEP A3
//#define PIN_FREEFALL_DETECT 12

#define SLEEP_MODE_ON    LOW
#define SLEEP_MODE_OFF   HIGH
//For lower power consumption, it is recommended to set g-Select to 1.5g mode.

#define FREEFALL_EVENT   HIGH

//#define PIN_SENSITIVITY_MODE 11 //Not used
//g-Range Sensitivity:
#define MODE_1.5G LOW  //+- 1.5g 800 mV/g
#define MODE_6G   HIGH //+- 6g 206 mV/g

#define CALIB_VAL_X 407
#define CALIB_VAL_Y 427
#define CALIB_VAL_Z 565

int x, y, z, accel;
int prev_x, prev_y, prev_z, prev_accel;
int width, height;
int graph_pos = 0;
uint16_t time1;
uint16_t time2;

// Use hardware SPI pins (for UNO thats sclk = 13 and sid = 11)
//and pin 10 must be an output. This is much faster
Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);

void setup(void) {
  pinMode(PIN_SLEEP, OUTPUT);
//  pinMode(PIN_FREEFALL_DETECT, INPUT);
  digitalWrite(PIN_SLEEP, SLEEP_MODE_OFF);
  Serial.begin(9600);
  
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(3);
  tft.setTextSize(2); //3
  width = tft.width();
  height = tft.height();
}

void loop() {
  getAccelValues();
   
  //uartPrintValues()
  tftCleanText();
  tftPrintValues();
  
  tftPrintGraph();
  //Refresh Rate
  delay(80);
//  delay(500);
//25Hz = 40ms
//10Hz = 100ms
  
//  time1 = millis();
//  clearLCD()
//  Serial.println(millis() - time1); //LCD Cleaning time = 93ms
}

void getAccelValues() {
  prev_x = x;
  prev_y = y;
  prev_z = z;  
  prev_accel = accel;
  
  x = analogRead(PIN_X) - CALIB_VAL_X;
  y = analogRead(PIN_Y) - CALIB_VAL_Y;
  z = analogRead(PIN_Z) - CALIB_VAL_Z;
  accel = abs(x) + abs(y);
}

void clearLCD() {
  tft.fillRect(34, 0, 130, 95, ST7735_BLACK);
}

void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void uartPrintValues() {
  // Вывод в Serial monitor
  Serial.print(" X: ");
  Serial.print(x);
  Serial.print(" Y: ");
  Serial.print(y);
  Serial.print(" Z: ");
  Serial.println(z);
}

void tftPrintGraph() {
  if (graph_pos >= width) { //If at the edge of the window, go back to the beginning:
    tft.fillScreen(ST7735_BLACK);
    graph_pos = 0;
  } else {
    tft.drawPixel(graph_pos, x, ST7735_BLUE);
    tft.drawPixel(graph_pos, y, ST7735_RED);
    tft.drawPixel(graph_pos, accel, ST7735_WHITE);    
//    tft.drawFastHLine(x, y, width, ST7735_RED);
    ++graph_pos;
  }
}

void tftCleanText() {
  tft.setCursor(0, 0);
  tft.setTextColor(ST7735_BLACK);
  
  tft.print("X:");
  tft.println(prev_x);
  tft.print("Y:");
  tft.println(prev_y);
  tft.print("Z:");
  tft.println(prev_z);
  tft.print("Ac");
  tft.println(prev_accel);
}

void tftPrintValues() {
//  time2 = millis();    
  tft.setCursor(0, 0);
//  tft.fillScreen(ST7735_BLACK);
//  tft.fillRect(34, 0, 130, 95, ST7735_BLACK);
//  tft.setTextSize(3);
  tft.setTextColor(ST7735_BLUE);
  tft.print("X:");
  tft.println(x);
  tft.setTextColor(ST7735_RED);
  tft.print("Y:");
  tft.println(y);
  tft.setTextColor(ST7735_GREEN);  
  tft.print("Z:");
  tft.println(z);
  tft.setTextColor(ST7735_YELLOW);
  tft.print("Ac");
  tft.println(accel);
//  Serial.print("LCD ");

//  Serial.println(millis() - time2); //Text printing time = 42ms
//In v2 =29ms (without abs(x) + abs(y))
}
