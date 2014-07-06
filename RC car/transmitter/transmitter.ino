
#define PIN_LED 13
int ledState = LOW;
long previousMillis = 0;
#define LCD_REFRESH_INTERVAL 1000

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

void setup() {
  pinMode(PIN_LED, OUTPUT); 
  myservo.attach(PIN_STEER_SERVO);
  
  Serial.begin(9600);
}

void loop() {
  int throttle = analogRead(PIN_X);
  delay(30); //this small pause is needed between reading analog pins, otherwise we get the same value twice
  int steer = analogRead(PIN_Y) + CALIBRATED_OFFSET_Y;
  
  int steerValue = map(steer, 0, 1023, 0, 179);

  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > LCD_REFRESH_INTERVAL) {
    previousMillis = currentMillis;
    
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;
      
      digitalWrite(PIN_LED, ledState);
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
  
  //TODO add trim functionality
  //thermal protection
  //add unger-voltage protection
}


