//Sources based on article https://learn.adafruit.com/thermistor/
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#define PIN_THERMISTOR A3
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000 //10k

int samples[NUMSAMPLES];

//LCD pin cs   10
//LCD pin dc   9
//LCD pin rst  8
//Using hardware SPI pins for maximum speed
Adafruit_ST7735 tft = Adafruit_ST7735(); //Library was optimized for max speed, sources available here: https://github.com/Artem-Mamchych/Adafruit-ST7735-Library

#define MIN_LCD_REFRESH_DELAY 300 //ms
//int width = tft.width();
//int height = tft.height();
unsigned long previousFrameMillis = 0;
float temperature, prev_temperature;

void setup(void) {
  tft.initR(INITR_BLACKTAB); //initialize a ST7735S LCD controller
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(3);

//  Serial.begin(9600);
}

void loop() {
  unsigned long currentFrameMillis = millis();
  if (currentFrameMillis - previousFrameMillis > MIN_LCD_REFRESH_DELAY) {
    previousFrameMillis = currentFrameMillis; 
    //LCD thread
    
    tftClean();
    tftDrawFrame();

//    uartPrintValues();
  }
    //Main thread for calculations, analog signal filtering, etc.
    uint8_t i;

    for (i=0; i < NUMSAMPLES; i++) {
      samples[i] = analogRead(PIN_THERMISTOR);
      delay(10);
    }
    
    float average = 0;
    for (i=0; i< NUMSAMPLES; i++) {
      average += samples[i];
    }
    average /= NUMSAMPLES;
    
    average = 1023 / average - 1;
    average = SERIESRESISTOR / average;
    
    float steinhart;
    steinhart = average / THERMISTORNOMINAL; // (R/Ro)
    steinhart = log(steinhart); // ln(R/Ro)
    steinhart /= BCOEFFICIENT; // 1/B * ln(R/Ro)
    steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart; // Invert
    steinhart -= 273.15; // convert to C
    
    temperature = steinhart;
    delay(10); //Main thread delay
}

//void uartPrintValues() { //UART debug output
//}

void tftClean() {
  tft.setTextColor(ST7735_BLACK);
  tftPrintText(false);
}

void tftDrawFrame() {
  tftPrintText(true);
}

inline void tftPrintText(boolean useColor) { //useColor = false used only in clean LCD function
  if (useColor) {
    prev_temperature = temperature;
  }
  tft.setCursor(0, 0);

  tft.setTextSize(3);
  if (useColor) tft.setTextColor(ST7735_BLUE);
  tft.print(prev_temperature); tft.println(" *C");
}
