#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

//LCD pin cs   10
//LCD pin dc   9
//LCD pin rst  8
//Using hardware SPI pins for max speed
Adafruit_ST7735 tft = Adafruit_ST7735(); //Library was optimized for max speed, sources available here: https://github.com/Artem-Mamchych/Adafruit-ST7735-Library
#define MIN_LCD_FEFRESH_DELAY 60 //60ms
unsigned long currentFrameMillis, previousFrameMillis = 0;

//MMA7361 definitions
#define PIN_X     A0
#define PIN_Y     A1
#define PIN_Z     A2
#define PIN_SLEEP A3
//#define PIN_FREEFALL_DETECT 12

#define FREEFALL_EVENT   HIGH
#define SLEEP_MODE_ON    LOW
#define SLEEP_MODE_OFF   HIGH
//For lower power consumption, it is recommended to set g-Select to 1.5g mode.

//#define PIN_SENSITIVITY_MODE 11 //Not used
//g-Range Sensitivity:
#define MODE_1.5G LOW  //+- 1.5g 800 mV/g
#define MODE_6G   HIGH //+- 6g 206 mV/g

//This calibration coefficients is specific for each MMA7361 chip
#define CALIB_VAL_X 407
#define CALIB_VAL_Y 427
#define CALIB_VAL_Z 565
//MMA7361 definitions END

//BICYCLE SPEED SENSOR definitions
#define TYRE_CIRCUMFERENCE 2074 //mm
#define DEBOUNCE_DELAY 15 //Debouncing Time in Milliseconds

volatile unsigned long time = 0;
volatile unsigned long speed_value = 0, prev_speed_value = 0;
volatile unsigned long mileage = 0, prev_mileage = 0;

//Interrupt handler for reed switch on pin 2
void speedReedSwInterrupt() {
  if((long)(millis() - time) >= DEBOUNCE_DELAY) {
    speed_value = TYRE_CIRCUMFERENCE/(millis() - time);
    mileage += TYRE_CIRCUMFERENCE/1000;
    if(speed_value > 99) speed_value = 99;
    
    time = millis();
  }
}

void setupSpeedSensorInterrupt() {
  attachInterrupt(0, speedReedSwInterrupt, RISING);
  speed_value = 0;
}
//BICYCLE SPEED SENSOR definitions END

int x, y, z, accel;
int prev_x, prev_y, prev_z, prev_accel;
int width, height;
int graph_pos = 0;
unsigned long lcdCleanTime, lcdFillTime;

void setup(void) {
  pinMode(PIN_SLEEP, OUTPUT);
//  pinMode(PIN_FREEFALL_DETECT, INPUT);
  digitalWrite(PIN_SLEEP, SLEEP_MODE_OFF);
  Serial.begin(9600);
  
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, LCD controller
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(3);
  width = tft.width();
  height = tft.height();
  
  setupSpeedSensorInterrupt(); 
}

void loop() {
  currentFrameMillis = millis();
  if (currentFrameMillis - previousFrameMillis > MIN_LCD_FEFRESH_DELAY) {
    previousFrameMillis = currentFrameMillis; 
    
    //LCD thread

    //uartPrintValues()
    lcdCleanTime = millis();
    tftCleanText();
    lcdFillTime = millis();
    tftPrintValues();
    tftPrintGraph();

    Serial.print(millis() - lcdFillTime); //draw time, ms  
    Serial.print("/");
    Serial.println(lcdFillTime - lcdCleanTime); //clean time, ms
  } else {
    Serial.print(":");
//    Serial.print(millis() - currentFrameMillis);
    getAccelValues(); //Add filtering
    delay(10);
  }
//  delay(80); //Refresh Rate
}

void getAccelValues() {
  x = analogRead(PIN_X) - CALIB_VAL_X;
  y = analogRead(PIN_Y) - CALIB_VAL_Y;
  z = analogRead(PIN_Z) - CALIB_VAL_Z;
  accel = abs(x) + abs(y);
}

void clearLCD() {
  tft.fillRect(34, 0, 130, 95, ST7735_BLACK);
}

void drawAlertMessage(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void uartPrintValues() {
  Serial.print(" X: ");
  Serial.print(x);
  Serial.print(" Y: ");
  Serial.print(y);
  Serial.print(" Z: ");
  Serial.println(z);
}

void tftPrintGraph() {
  tft.drawLine(graph_pos-1, (prev_accel+512)>>3, graph_pos, (accel+512)>>3, ST7735_YELLOW);
  tft.drawLine(graph_pos-1, prev_speed_value, graph_pos, speed_value, ST7735_WHITE);

  tft.drawPixel(graph_pos, (x+512)>>3, ST7735_BLUE);
  tft.drawPixel(graph_pos, (y+512)>>3, ST7735_RED);   
  ++graph_pos;
}

void tftCleanText() {
  tft.setCursor(0, 0);
  if (graph_pos >= width) { //If at the edge of the window, go back to the beginning:
    tft.fillScreen(ST7735_BLACK);
    graph_pos = 1;
  } else {
    tft.setTextColor(ST7735_BLACK);

    tftPrintSpeed(false);
    tftPrintAccelerometerXYZ(false);
  }
}

void tftPrintValues() {
  tft.setCursor(0, 0);
//  tft.fillRect(34, 0, 130, 95, ST7735_BLACK);

  tftPrintSpeed(true);
  tftPrintAccelerometerXYZ(true);
}

inline void tftPrintSpeed(boolean useColor) { //useColor = false used only in clean LCD function
  if (useColor) {
    prev_speed_value = speed_value;
    prev_mileage = mileage; 
  }

  if (useColor) tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(3);
  tft.print(prev_speed_value); tft.print("/"); tft.println(prev_mileage); 
}

inline void tftPrintAccelerometerXYZ(boolean useColor) { //useColor = false used only in clean LCD function
  tft.setTextSize(2);

  if (useColor) {
    prev_x = x;
    prev_y = y;
    prev_z = z;  
    prev_accel = accel;
  }

  if (useColor) tft.setTextColor(ST7735_BLUE);
  tft.print("X:"); tft.print(prev_x);
  if (useColor) tft.setTextColor(ST7735_RED);
  tft.print(" Z:"); tft.println(prev_z);

  if (useColor) tft.setTextColor(ST7735_GREEN);
  tft.print("Y:");   tft.print(prev_y);
  if (useColor) tft.setTextColor(ST7735_YELLOW);
  tft.print(" A:"); tft.println(prev_accel);  
}
