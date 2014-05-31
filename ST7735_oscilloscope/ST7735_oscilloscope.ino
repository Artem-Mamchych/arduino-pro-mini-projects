#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <EEPROM.h>
 
// Variables you might want to play with
byte useThreshold = 1;                  // 0 = Off, 1 = Rising, 2 = Falling
byte theThreshold = 128;                // 0-255, Multiplied by voltageConst
unsigned int timePeriod = 200;          // 0-65535, us or ms per measurement (max 0.065s or 65.535s)
byte voltageRange = 1;                  // 1 = 0-3.3V, 2 = 0-1.65V, 3 = 0-0.825V
byte ledBacklight = 100;
 
boolean autoHScale = true;              // Automatic horizontal (time) scaling
boolean linesNotDots = true;            // Draw lines between data points
 
// Variables that can probably be left alone
const byte vTextShift = 65;             // Vertical text shift (to vertically align info)
const byte numOfSamples = 128;          // Leave at 100 for 128x64 pixel display
unsigned int HQadcReadings[numOfSamples];
byte adcReadings[numOfSamples];
byte adcOldReadings[numOfSamples];
byte thresLocation = 0;                 // Threshold bar location
float voltageConst = 0.052381;          // Scaling factor for converting 0-63 to V
float avgV = 0.0;    
float maxV = 0.0;
float minV = 0.0;
float ptopV = 0.0;
float theFreq = 0;

byte OlduseThreshold = 1;                  
byte OldtheThreshold = 128;                
unsigned int OldtimePeriod = 200;
float OldavgV = 0.0;    
float OldmaxV = 0.0;
float OldminV = 0.0;
float OldptopV = 0.0;
float OldtheFreq = 0;
 
const byte theAnalogPin = 7; // Data read pin (A7)

//#define sclk 13
//#define mosi 11
//#define cs   10
//#define dc   8
//#define rst  7 
//Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst); //Original Adafruit_ST7735 library

Adafruit_ST7735 tft = Adafruit_ST7735(); //My speed-optimized fork, available on https://github.com/Artem-Mamchych/Adafruit-ST7735-Library
//Note: check Adafruit_ST7735.cpp sources for LCD pin mapping
 
// High speed ADC code
// From: http://forum.arduino.cc/index.php?PHPSESSID=e21f9a71b887039092c91a516f9b0f36&topic=6549.15
#define FASTADC 1
// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
 
void collectData(void) {
  unsigned int tempThres = 0;
  unsigned int i = 0;
 
  if (autoHScale == true) {
    // With automatic horizontal (time) scaling enabled,
    // scale quickly if the threshold location is far, then slow down
    if (thresLocation > 5 * numOfSamples / 8) {
      timePeriod = timePeriod + 10;
    } else if (thresLocation < 3 * numOfSamples / 8) {
      timePeriod = timePeriod - 10;
    } else if (thresLocation > numOfSamples / 2) {
      timePeriod = timePeriod + 2;
    } else if (thresLocation < numOfSamples / 2) {
      timePeriod = timePeriod - 2;
    }
  }
  // Enforce minimum time periods
  if (timePeriod < 35) {
    timePeriod = 35;
  }
   
  // Adjust voltage contstant to fit the voltage range
  if (voltageRange == 1) {
    voltageConst = 0.0523810; // 0-3.30V
  } else if (voltageRange == 2) {
    voltageConst = 0.0261905; // 0-1.65V
  } else if (voltageRange == 3) {
    voltageConst = 0.0130952; //0-0.825V
  }
   
  // If using threshold, wait until it has been reached
  if (voltageRange == 1) tempThres = theThreshold << 2;
  else if (voltageRange == 2) tempThres = theThreshold << 1;
  else if (voltageRange == 3) tempThres = theThreshold;
  if (useThreshold == 1) {
     i = 0; while ((analogRead(theAnalogPin) > tempThres) && (i<32768)) i++;
     i = 0; while ((analogRead(theAnalogPin) < tempThres) && (i<32768)) i++;
  }
  else if (useThreshold == 2) {
     i = 0; while ((analogRead(theAnalogPin) < tempThres) && (i<32768)) i++;
     i = 0; while ((analogRead(theAnalogPin) > tempThres) && (i<32768)) i++;
  }
 
  // Collect ADC readings
  for (i=0; i < numOfSamples; i++) {
    // Takes 35 us with high speed ADC setting
    HQadcReadings[i] = analogRead(theAnalogPin);
    if (timePeriod > 35) delayMicroseconds(timePeriod-35);
  }
  for (i=0; i < numOfSamples; i++) {
    // Scale the readings to 0-63 and clip to 63 if they are out of range.
    if (voltageRange == 1) {
      if (HQadcReadings[i]>>4 < 0b111111) adcReadings[i] = HQadcReadings[i]>>4 & 0b111111;
      else adcReadings[i] = 0b111111;
    } else if (voltageRange == 2) {
      if (HQadcReadings[i]>>3 < 0b111111) adcReadings[i] = HQadcReadings[i]>>3 & 0b111111;
      else adcReadings[i] = 0b111111;
    } else if (voltageRange == 3) {
      if (HQadcReadings[i]>>2 < 0b111111) adcReadings[i] = HQadcReadings[i]>>2 & 0b111111;
      else adcReadings[i] = 0b111111;
    }
    // Invert for display
    adcReadings[i] = 63 - adcReadings[i];
  }
   
  // Calculate and display frequency of signal using zero crossing
  if (useThreshold != 0) {
     if (useThreshold == 1) {
        thresLocation = 1;
        while ((adcReadings[thresLocation] < (63-(theThreshold>>2))) && (thresLocation < numOfSamples-1)) (thresLocation++);
        thresLocation++;
        while ((adcReadings[thresLocation] > (63-(theThreshold>>2))) && (thresLocation < numOfSamples-1)) (thresLocation++);
     }
     else if (useThreshold == 2) {
        thresLocation = 1;
        while ((adcReadings[thresLocation] > (63-(theThreshold>>2))) && (thresLocation < numOfSamples-1)) (thresLocation++);
        thresLocation++;
        while ((adcReadings[thresLocation] < (63-(theThreshold>>2))) && (thresLocation < numOfSamples-1)) (thresLocation++);
     }
 
     theFreq = (float) 1000 / (thresLocation * timePeriod) * 1000;
  }
   
  // Average Voltage
  avgV = 0;
  for (i=0; i < numOfSamples; i++)
     avgV = avgV + adcReadings[i];
  avgV = (63-(avgV / numOfSamples)) * voltageConst;
 
  // Maximum Voltage
  maxV = 63;
  for (i=0; i < numOfSamples; i++)
     if (adcReadings[i]<maxV) maxV = adcReadings[i];
  maxV = (63-maxV) * voltageConst;
 
  // Minimum Voltage
  minV = 0;
  for (i=0; i < numOfSamples; i++)
     if (adcReadings[i]>minV) minV = adcReadings[i];
  minV = (63-minV) * voltageConst;
 
  // Peak-to-Peak Voltage
  ptopV = maxV - minV;
}
 
void handleSerial(void) {
  char inByte;
  char dataByte;
  boolean exitLoop = false;
  do {
    // Clear out buffer
    do {
      inByte = Serial.read();
    } while (Serial.available() > 0);
   
    Serial.print("\nArduino LCD Oscilloscope\n");
    Serial.print(" 1 - Change threshold usage (currently: ");
      if (useThreshold == 0) Serial.print("Off)\n");
      else if (useThreshold == 1) Serial.print("Rise)\n");
      else if (useThreshold == 2) Serial.print("Fall)\n");
    Serial.print(" 2 - Change threshold value (currently: ");
      Serial.print(theThreshold, DEC); Serial.print(")\n");
    Serial.print(" 3 - Change sampling period (currently: ");
      Serial.print(timePeriod, DEC); Serial.print(")\n");
    Serial.print(" 4 - Change voltage range (currently: ");
      if (voltageRange == 1) Serial.print("0-3.3V)\n");
      else if (voltageRange == 2) Serial.print("0-1.65V)\n");
      else if (voltageRange == 3) Serial.print("0-0.825V)\n");
    Serial.print(" 5 - Toggle auto horizontal (time) scaling (currently: ");
      if (autoHScale == true) Serial.print("On)\n");
      else if (autoHScale == false) Serial.print("Off)\n");
    Serial.print(" 8 - Exit\n");
     
    // Wait for input/response, refresh display while in menu
    do {
      collectData();
      draw1();
      // Picture Display Loop
//!!!      u8g.firstPage();  
//      do { draw(); } while( 
//!!!      u8g.nextPage() 
//      );
    } while (Serial.available() == 0);
    inByte = Serial.read();
     
    if (inByte == '1') {
      Serial.print("Change threshold usage\n");
      Serial.print(" 0 - Off\n");
      Serial.print(" 1 - Rise\n");
      Serial.print(" 2 - Fall\n");
      do { } while (Serial.available() == 0);
      dataByte = Serial.read();
      if (dataByte == '0') useThreshold = 0;
      else if (dataByte == '1') useThreshold = 1;
      else if (dataByte == '2') useThreshold = 2;
    } else if (inByte == '2') {
      Serial.print("Change threshold value (thresholds for 0-3.3V,0-1.65V,0-0.825V ranges)\n");
      Serial.print(" 0 - 32 (0.41V, 0.21V, 0.10V)\n");
      Serial.print(" 1 - 80 (1.04V, 0.52V, 0.26V)\n");
      Serial.print(" 2 - 128 (1.66V, 0.83V, 0.41V)\n");
      Serial.print(" 3 - 176 (2.28V, 1.14V, 0.57V)\n");
      Serial.print(" 4 - 224 (2.90V, 1.45V, 0.72V)\n");
      do { } while (Serial.available() == 0);
      dataByte = Serial.read();
      if (dataByte == '0') theThreshold = 32;
      else if (dataByte == '1') theThreshold = 80;
      else if (dataByte == '2') theThreshold = 128;
      else if (dataByte == '3') theThreshold = 176;
      else if (dataByte == '4') theThreshold = 224;
    } else if (inByte == '3') {
      Serial.print("Change sampling frequency\n");
      Serial.print(" 0 - 28 kHz (35 us/sample)\n");
      Serial.print(" 1 - 20 kHz (50 us/sample)\n");
      Serial.print(" 2 - 10 kHz (100 us/sample)\n");
      Serial.print(" 3 - 5 kHz (200 us/sample)\n");
      Serial.print(" 4 - 2.5 kHz (400 us/sample)\n");
      do { } while (Serial.available() == 0);
      dataByte = Serial.read();
      if (dataByte == '0') timePeriod = 35;
      else if (dataByte == '1') timePeriod = 50;
      else if (dataByte == '2') timePeriod = 100;
      else if (dataByte == '3') timePeriod = 200;
      else if (dataByte == '4') timePeriod = 400;
    } else if (inByte == '4') {
      Serial.print("Change voltage range\n");
      Serial.print(" 1 - 0-3.3V\n");
      Serial.print(" 2 - 0-1.65V\n");
      Serial.print(" 3 - 0-0.825V\n");
      do { } while (Serial.available() == 0);
      dataByte = Serial.read();
      if (dataByte == '1') voltageRange = 1;
      else if (dataByte == '2') voltageRange = 2;
      else if (dataByte == '3') voltageRange = 3;
    } else if (inByte == '5') {
      Serial.print("Toggle auto horizontal (time) scaling\n");
      Serial.print(" 0 - Off\n");
      Serial.print(" 1 - On\n");
      do { } while (Serial.available() == 0);
      dataByte = Serial.read();
      if (dataByte == '0') autoHScale = false;
      else if (dataByte == '1') autoHScale = true;
    } else if (inByte == '8') {
      Serial.print("Bye!\n");
      exitLoop = true;
    }
    draw();
  } while (exitLoop == false);
}

void drawD(void) {
  char buffer[16];
  if(avgV != OldavgV) {
    tft.setTextColor(ST7735_BLACK);
    dtostrf(OldavgV, 3, 2, buffer);
    tft.setCursor(15, 5+vTextShift);
    tft.print(buffer);
    tft.setTextColor(ST7735_RED);
    dtostrf(avgV, 3, 2, buffer);
    tft.setCursor(15, 5+vTextShift);
    tft.print(buffer);
    OldavgV=avgV;
  }
  if(maxV != OldmaxV) {
    tft.setTextColor(ST7735_BLACK);
    dtostrf(OldmaxV, 3, 2, buffer);
    tft.setCursor(15, 2+11+vTextShift);
    tft.print(buffer);
    tft.setTextColor(ST7735_RED);
    dtostrf(maxV, 3, 2, buffer);
    tft.setCursor(15, 2+11+vTextShift);
    tft.print(buffer);
    OldmaxV=maxV;
  }
  if(minV != OldminV) {
    tft.setTextColor(ST7735_BLACK);
    dtostrf(OldminV, 3, 2, buffer);
    tft.setCursor(15, 4+17+vTextShift);
    tft.print(buffer);
    tft.setTextColor(ST7735_RED);
    dtostrf(minV, 3, 2, buffer);
    tft.setCursor(15, 4+17+vTextShift);
    tft.print(buffer);
    OldminV=minV;
  }
  if(ptopV != OldptopV) {  
    tft.setTextColor(ST7735_BLACK);
    dtostrf(OldptopV, 3, 2, buffer);
    tft.setCursor(15, 6+23+vTextShift);
    tft.print(buffer);
    tft.setTextColor(ST7735_RED);
    dtostrf(ptopV, 3, 2, buffer);
    tft.setCursor(15, 6+23+vTextShift);
    tft.print(buffer);
    OldptopV=ptopV;
  }
  if(theFreq != OldtheFreq) {
    tft.setTextColor(ST7735_BLACK);
    dtostrf(OldtheFreq, 5, 0, buffer);
    tft.setCursor(0, 16+53+vTextShift);
    tft.print(buffer);
    tft.setTextColor(ST7735_RED);
    dtostrf(theFreq, 5, 0, buffer);
    tft.setCursor(0, 16+53+vTextShift);
    tft.print(buffer);
    OldtheFreq=theFreq;
  }
  /*if (useThreshold == 0) {
    tft.setCursor(15, 8+29+vTextShift);
    tft.print("Off");
  } else if (useThreshold == 1) {
    tft.setCursor(15, 8+29+vTextShift);
    tft.print("Rise");
    dtostrf((float) (theThreshold>>2) * voltageConst, 3, 2, buffer);
  } else if (useThreshold == 2) {
    tft.setCursor(15, 8+29+vTextShift);
    tft.print("Fall");
    dtostrf((float) (theThreshold>>2) * voltageConst, 3, 2, buffer);
  }
  tft.setCursor(11, 10+35+vTextShift);
  tft.print(buffer);*/
  // Correctly format the text so that there are always 4 characters
if(timePeriod != OldtimePeriod) {
  tft.setTextColor(ST7735_BLACK);
  if (OldtimePeriod < 400) {
    dtostrf((float) OldtimePeriod/1000 * 25, 3, 2, buffer);
  } else if (OldtimePeriod < 4000) {
    dtostrf((float) OldtimePeriod/1000 * 25, 3, 1, buffer);
  } else if (OldtimePeriod < 40000) {
    dtostrf((float) OldtimePeriod/1000 * 25, 3, 0, buffer);
  } else { // Out of range
    dtostrf((float) 0.00, 3, 2, buffer);
  }
  tft.setCursor(15, 12+41+vTextShift);
  tft.print(buffer);
  
  tft.setTextColor(ST7735_RED);
  if (timePeriod < 400) {
    dtostrf((float) timePeriod/1000 * 25, 3, 2, buffer);
  } else if (timePeriod < 4000) {
    dtostrf((float) timePeriod/1000 * 25, 3, 1, buffer);
  } else if (timePeriod < 40000) {
    dtostrf((float) timePeriod/1000 * 25, 3, 0, buffer);
  } else { // Out of range
    dtostrf((float) 0.00, 3, 2, buffer);
  }
  tft.setCursor(15, 12+41+vTextShift);
  tft.print(buffer);
  OldtimePeriod=timePeriod;
}
  /*if (voltageRange == 1) {
    tft.setCursor(7, 18+59+vTextShift);
    tft.print("0-3.30");
  } else if (voltageRange == 2) {
    tft.setCursor(7, 18+59+vTextShift);
    tft.print("0-1.65");
  } else if (voltageRange == 3) {
    tft.setCursor(7, 18+59+vTextShift);
    tft.print("0-0.83");
  } */
}
void drawT(void) {
  char buffer[16];
  tft.fillRect(0,vTextShift+5,45,150,ST7735_BLACK);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
  tft.setCursor(0, 5+vTextShift);
  tft.print("Av");
  tft.setCursor(0, 2+11+vTextShift);
  tft.print("Mx");
  tft.setCursor(0, 4+17+vTextShift);
  tft.print("Mn");
  tft.setCursor(0, 6+23+vTextShift);
  tft.print("PP");
  tft.setCursor(0, 8+29+vTextShift);
  tft.print("Th");
  tft.setCursor(0, 10+35+vTextShift);
  tft.print("V");
  tft.setCursor(0, 12+41+vTextShift);
  tft.print("Tm");
  tft.setCursor(4, 14+47+vTextShift);
  tft.print("ms/div");
  tft.setCursor(31, 16+53+vTextShift);
  tft.print("Hz");
  tft.setCursor(0, 18+59+vTextShift);
  tft.print("R");
 
  // Draw dynamic text
  if (autoHScale == true) {
    tft.setCursor(120, 2);
    tft.print("A");
  }
  dtostrf(avgV, 3, 2, buffer);
  tft.setCursor(15, 5+vTextShift);
  tft.print(buffer);

  dtostrf(maxV, 3, 2, buffer);
  tft.setCursor(15, 2+11+vTextShift);
  tft.print(buffer);

  dtostrf(minV, 3, 2, buffer);
  tft.setCursor(15, 4+17+vTextShift);
  tft.print(buffer);

  dtostrf(ptopV, 3, 2, buffer);
  tft.setCursor(15, 6+23+vTextShift);
  tft.print(buffer);

  dtostrf(theFreq, 5, 0, buffer);
  tft.setCursor(0, 16+53+vTextShift);
  tft.print(buffer);

  if (useThreshold == 0) {
    tft.setCursor(15, 8+29+vTextShift);
    tft.print("Off");
  } else if (useThreshold == 1) {
    tft.setCursor(15, 8+29+vTextShift);
    tft.print("Rise");
    dtostrf((float) (theThreshold>>2) * voltageConst, 3, 2, buffer);
  } else if (useThreshold == 2) {
    tft.setCursor(15, 8+29+vTextShift);
    tft.print("Fall");
    dtostrf((float) (theThreshold>>2) * voltageConst, 3, 2, buffer);
  }
  tft.setCursor(11, 10+35+vTextShift);
  tft.print(buffer);
  // Correctly format the text so that there are always 4 characters
  if (timePeriod < 400) {
    dtostrf((float) timePeriod/1000 * 25, 3, 2, buffer);
  } else if (timePeriod < 4000) {
    dtostrf((float) timePeriod/1000 * 25, 3, 1, buffer);
  } else if (timePeriod < 40000) {
    dtostrf((float) timePeriod/1000 * 25, 3, 0, buffer);
  } else { // Out of range
    dtostrf((float) 0.00, 3, 2, buffer);
  }
  tft.setCursor(15, 12+41+vTextShift);
  tft.print(buffer);
  if (voltageRange == 1) {
    tft.setCursor(7, 18+59+vTextShift);
    tft.print("0-3.30");
  } else if (voltageRange == 2) {
    tft.setCursor(7, 18+59+vTextShift);
    tft.print("0-1.65");
  } else if (voltageRange == 3) {
    tft.setCursor(7, 18+59+vTextShift);
    tft.print("0-0.83");
  } 
  OlduseThreshold = useThreshold;                  
  OldtheThreshold = theThreshold;                
  OldtimePeriod = timePeriod;
  OldavgV = avgV;    
  OldmaxV = maxV;
  OldminV = minV;
  OldptopV = ptopV;
  OldtheFreq = theFreq;
}
 
void draw(void) {
  tft.fillScreen(ST7735_BLACK);
  drawT();
  draw1();
}

void draw1(void) {
  int i;
  if (useThreshold != 0)
     for (i=0; i < 127; i+=3) {
        tft.drawPixel(i,63-(theThreshold>>2),ST7735_YELLOW);
        tft.drawPixel(i,63,ST7735_YELLOW);
     }
  for (i=0; i < 63; i+=5) {
     tft.drawPixel(32,i,ST7735_YELLOW);
     tft.drawPixel(64,i,ST7735_YELLOW);
     tft.drawPixel(96,i,ST7735_YELLOW);
     tft.drawPixel(127,i,ST7735_YELLOW);
  }
  tft.drawFastLine(1+(128-numOfSamples)-1,adcOldReadings[0],adcOldReadings[1]-adcOldReadings[0],2, ST7735_BLACK);
  for (i=1; i < numOfSamples-1; i++) { // Draw using lines
      tft.drawFastLine(i+(128-numOfSamples),adcOldReadings[i],adcOldReadings[i+1]-adcOldReadings[i],2, ST7735_BLACK);
      tft.drawFastLine(i+(128-numOfSamples)-1,adcReadings[i-1],adcReadings[i]-adcReadings[i-1],2, ST7735_WHITE);
      adcOldReadings[i-1]=adcReadings[i-1];
  }
  tft.drawFastLine(127+(128-numOfSamples)-1,adcReadings[126],adcReadings[127]-adcReadings[126],2, ST7735_WHITE);
  adcOldReadings[126]=adcReadings[126];
  adcOldReadings[127]=adcReadings[127];
}
 
void setup() {
  Serial.begin(9600);
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK); 
  // Turn on LED backlight
  analogWrite(3, ledBacklight);
   
  #if FASTADC
    // set prescale to 16
    sbi(ADCSRA,ADPS2) ;
    cbi(ADCSRA,ADPS1) ;
    cbi(ADCSRA,ADPS0) ;
  #endif
  delay(100);
  draw();
}

int j=8;
 
void loop() {
  
  collectData();
  draw1();
  drawD();
  if (j == 100) {
    drawT();
    j=0;
  }
  j++;
 
  // If user sends any serial data, show menu
  if (Serial.available() > 0) {
    handleSerial();
  }
 
  // rebuild the picture after some delay
  delay(100); //Can be commented
}

