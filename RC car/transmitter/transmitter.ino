//ST7735 LCD library
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

//LCD pin cs   10
//LCD pin dc   9
//LCD pin rst  8
//Using hardware SPI pins for max speed
Adafruit_ST7735 tft = Adafruit_ST7735(); //Library was optimized for max speed, sources available here: https://github.com/Artem-Mamchych/Adafruit-ST7735-Library
#define LCD_REFRESH_INTERVAL 400
unsigned long currentFrameMillis, previousFrameMillis = 0;
int width, height;
int graph_pos = 0;

//Other code
#define PIN_LED 13
int ledState = LOW;
long previousMillis = 0;

#define PIN_X A0
#define PIN_Y A1
//#define PIN_SW 13
#define PIN_STEER_SERVO 2

#define CALIBRATED_OFFSET_Y 15

#define BACK_DELAY_INTERVAL 1000
#define FRONT 1
#define BACK 0
int throttleState = FRONT;

int previousSteerValue = 0;

#include <Servo.h>
Servo myservo;

int steerValue, accelValue;
int prev_steerValue, prev_accelValue;

void setup() {
  //LCD init
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, LCD controller
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(3);
  width = tft.width();
  height = tft.height();  
  
  //Pins setup
  pinMode(PIN_LED, OUTPUT); 
  myservo.attach(PIN_STEER_SERVO);
  
  Serial.begin(9600);
}

void loop() {
  int throttle = analogRead(PIN_X);
  accelValue = map(throttle, 0, 1023, 0, 179);  
  delay(40); //this small pause is needed between reading analog pins, otherwise we get the same value twice
  
  int steer = analogRead(PIN_Y) + CALIBRATED_OFFSET_Y;  
  steerValue = map(steer, 0, 1023, 0, 179);

  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > LCD_REFRESH_INTERVAL) {
    previousMillis = currentMillis;
    
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;
      
      digitalWrite(PIN_LED, ledState);
      
      //LCD thread
      tftDrawFrame();
  }

  Serial.print(throttle);
  Serial.print("\t");
  Serial.print(steer);
  if (throttle > 475 && throttle < 525) {
    Serial.print(" X centered");
  }
  
  if (throttleState == FRONT && throttle > 525) {
    throttleState = BACK;
    Serial.print(" Running backward");
    delay(BACK_DELAY_INTERVAL);
    Serial.print(" NOW");
  } else {
    throttleState = FRONT;
  }
  
  if (steer > 475 && steer < 525) {
    Serial.print(" Y centered");
    if (previousSteerValue != steerValue) {
      previousSteerValue = steerValue;
      myservo.attach(PIN_STEER_SERVO); 
      myservo.write(steerValue);
      delay(40);
      myservo.detach();
    }
  } else {
    myservo.attach(PIN_STEER_SERVO); 
    myservo.write(steerValue);
    delay(30);
    myservo.detach();  
  }
  
  if (steer > 525) {
    Serial.print(" Running leftward");
  }
  Serial.println("");
  delay(30);
}

void tftDrawFrame() {
  //Clean-up previous frame
  tft.setCursor(0, 0);
  if (graph_pos >= width) { //If at the edge of the window, go back to the beginning:
    tft.fillScreen(ST7735_BLACK);
    graph_pos = 1;
  } else {
    tft.setTextColor(ST7735_BLACK);

    tftPrintDbgInfo(false);
  }
  
  //Draw new frame
  tft.setCursor(0, 0);
  tftPrintDbgInfo(true);
}

inline void tftPrintDbgInfo(boolean useColor) { //useColor = false used only in clean LCD function
  if (useColor) {
    prev_steerValue = steerValue;
    prev_accelValue = accelValue; 
  }

  if (useColor) tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(3);
  tft.print(prev_accelValue); tft.print(" L/R:"); tft.println(prev_steerValue); 
}

