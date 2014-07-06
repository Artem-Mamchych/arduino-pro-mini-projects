//ST7735 LCD library
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

//LCD pin cs   10
//LCD pin dc   9
//LCD pin rst  8
//Using hardware SPI pins for max speed
Adafruit_ST7735 tft = Adafruit_ST7735(); //Library was optimized for max speed, sources available here: https://github.com/Artem-Mamchych/Adafruit-ST7735-Library
#define LCD_REFRESH_INTERVAL 200
unsigned long currentFrameMillis, previousFrameMillis = 0;
int width, height;
int graph_pos = 0;

//Other code
//#define PIN_LED 13
//int ledState = LOW;
long previousMillis = 0;

#define PIN_X A0
#define PIN_Y A3
#define PIN_SW A2 //Joystick button
long lastDebounceTime = 0; // the last time the output pin was toggled
#define DEBOUNCE_DELAY 150  // the debounce time; increase if the output flickers
int joystickButtonState = 0;
int lastJoystickButtonState = LOW; // the previous reading from the input pin

#define PIN_STEER_SERVO 2

#define ADC_SAMPLES 4 //how many samples to take and average ADC readings
int samples[ADC_SAMPLES];

#define BACK_DELAY_INTERVAL 1000
#define FRONT 1
#define BACK 0
int throttleState = FRONT;
int previousSteerValue = 0;

#include <Servo.h>
Servo myservo;

int steer, throttle;
int prev_steer, prev_throttle;

#define NUMBER_ACCELERATION_SEPEEDLITITS 5
int steerValue, accelValue, accelLimit = NUMBER_ACCELERATION_SEPEEDLITITS, ledState;
int prev_steerValue, prev_accelValue, prev_accelLimit, prev_ledState;

#define SERIAL_DEBUG 1

void setup() {
  //LCD init
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, LCD controller
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(3);
  width = tft.width();
  height = tft.height();  
  
  //Pins setup
//  pinMode(PIN_LED, OUTPUT); 
  pinMode(PIN_SW, INPUT);
  myservo.attach(PIN_STEER_SERVO);
  
  #ifdef SERIAL_DEBUG
  Serial.begin(9600);
  #endif
}

void handleJoystickButtonPress() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(PIN_SW);
    #ifdef SERIAL_DEBUG
    Serial.print("DigitalRead(PIN_SW)=");
    Serial.print(reading);
    
    int reading2 = analogRead(PIN_SW);
    Serial.print(" analogRead(PIN_SW)=");
    Serial.println(reading2);    
    #endif

  // check to see if you just pressed the button (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (reading != lastJoystickButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
 
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != joystickButtonState) {
      joystickButtonState = reading;

      // only toggle the LED if the new button state is LOW
      if (joystickButtonState == LOW) {
        --accelLimit;
        #ifdef SERIAL_DEBUG
        Serial.print("accelLimit=");
        Serial.print(accelLimit); 
        Serial.print(" "); 
        Serial.println(" ");
        #endif
        
        if (accelLimit < 0) {
          #ifdef SERIAL_DEBUG
            Serial.println("Reset to NUMBER_ACCELERATION_SEPEEDLITITS ");
          #endif
          accelLimit = NUMBER_ACCELERATION_SEPEEDLITITS;
        }
        
        if (joystickButtonState == LOW) {
          ledState = !ledState;
        }
      }
    }
  }
 
  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastJoystickButtonState = reading;  
}

int smoothAnalogRead(int pin) {
  uint8_t i;
   // take N samples in a row, with a slight delay
  for (i=0; i < ADC_SAMPLES; i++) {
    samples[i] = analogRead(pin);
    delay(10); //this small pause is needed between reading analog pins, otherwise we get the same value twice
  }
  
  // average all the samples out  
  int average = 0;
  for (i=0; i< ADC_SAMPLES; i++) {
    average += samples[i];
  }
  average /= ADC_SAMPLES;
  return average;
}

void loop() {
  handleJoystickButtonPress();
  throttle = smoothAnalogRead(PIN_X);
  accelValue = map(throttle, 0, 1023, 0, 179);  
  
  steer = smoothAnalogRead(PIN_Y);
  steerValue = map(steer, 0, 1023, 0, 179);
  
  if (throttleState == FRONT && throttle > 525) {
    throttleState = BACK;
//    #ifndef SERIAL_DEBUG Serial.print(" Running backward"); #endif
//    delay(BACK_DELAY_INTERVAL);
//    #ifndef SERIAL_DEBUG Serial.print(" NOW"); #endif
  } else {
    throttleState = FRONT;
  }
  
  if (steer > 475 && steer < 525) {
//    Serial.print(" Y centered");
    if (previousSteerValue != steerValue) {
      previousSteerValue = steerValue;
      myservo.attach(PIN_STEER_SERVO); 
      myservo.write(steerValue);
      delay(20);
      myservo.detach();
    }
  } else {
    myservo.attach(PIN_STEER_SERVO); 
    myservo.write(steerValue);
    delay(20);
    myservo.detach();  
  }
  
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > LCD_REFRESH_INTERVAL) {
    previousMillis = currentMillis;
    
//    if (ledState == LOW)
//      ledState = HIGH;
//    else
//      ledState = LOW;
//      
//      digitalWrite(PIN_LED, ledState);
      
      //LCD thread
      tftDrawFrame();
  }
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
    prev_steer = steer;
    prev_throttle = throttle;
    prev_accelLimit = accelLimit;
    prev_ledState = ledState;
  }

  if (useColor) tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(2);
  tft.print(prev_accelValue); tft.print(" L/R:"); tft.println(prev_steerValue);
  tft.setTextSize(1);
  tft.print("X:"); tft.print(prev_throttle); tft.print(" Y:"); tft.print(prev_steer); tft.print(" Div:"); tft.print(prev_accelLimit);
  tft.print(" led:"); tft.println(prev_ledState);
    
  if (prev_throttle > 475 && prev_throttle < 525) {
    tft.println("X centered");
  }
  if (prev_steer > 475 && prev_steer < 525) {
    tft.println("Y centered");
  }  
  
//  if (throttleState == BACK) {
  if (prev_throttle < 475) {  
    tft.println("BACK");
  }

  if (prev_steer > 525) {
    tft.println("<left");
  }
  if (prev_steer < 475) {
    tft.println("right>");
  }  
}

