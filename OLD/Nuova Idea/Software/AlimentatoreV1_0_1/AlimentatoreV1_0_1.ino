#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735
#include <Adafruit_ADS1X15.h>
#include <SPI.h>
//TFT Pins
#define TFT_CS 3
#define TFT_RST -1   
#define TFT_DC 4 
//Encoder Pins
#define ENC_CLK 0
#define ENC_DT 1
#define ENC_BUT 2

// TFT Setup
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

//ADS Setup
Adafruit_ADS1115 ads;


//Home Icon Bitmap
const unsigned char menuButton[] = {
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000001, B00000000,
  B00000011, B10110000,
  B00000111, B11110000,
  B00001111, B11110000,
  B00011111, B11110000,
  B00111111, B11111000,
  B01111111, B11111100,
  B11111111, B11111110,
  B00111111, B11111000,
  B00111100, B01111000,
  B00111100, B01111000,
  B00111100, B01111000,
  B00111100, B01111000
};
//Variables for Button and Encoder
bool buttonState = 0, lastButtonState = 1, reading = 1;
unsigned long lastDebounceTime = 0;
unsigned int debounceDelay = 100;  //time in ms

bool prevClk, prevDt, currClk, currDt;
int counterPosition = 0, counterValue = 0;

//States for GUI implementation
byte selectedMode = 0, modifyMode = 0;;
volatile bool ScreenUpdate = 0;

//Variables for ADC
uint16_t adcReadType[] = {ADS1X15_REG_CONFIG_MUX_DIFF_2_3, ADS1X15_REG_CONFIG_MUX_SINGLE_3};
byte adcType = 0;


bool backSelected[] = { 0, 1, 0, 0, 0, 0, 0, 0, 0 };                         //Array used for all Background status in every GUI page
const char menuName[4][6] = { "Mode", "Wire", "Ramp", "Calib" };      //Main menu labels (selectedMode = 0)
int menuColors[] = { ST77XX_WHITE, ST77XX_RED, ST77XX_GREEN, ST77XX_BLUE };  //Main menu colors
byte menuCursor[] = { 30, 10, 30, 45, 30, 86, 20, 130 };                     //Main menu Cursor (Ax,Ay,Bx,By,..)

int OneVoltRef = 8000;
double rampValuesConst[] = { 0, 0, 0, 0, 0 };                            //Ramp page output Values  (V,A,VMax,Vmin,StateStatus)
byte rampValuesCursor[] = { 120, 25, 25, 5 };                           //Ramp page input Values   (V,T1,T2,Cyc,Play,Stop,Home)
byte rampCursorConst[] = { 10, 6, 10, 31, 35, 110, 35, 120, 76, 123 };  //Ramp page output Cursor  (Ax,Ay,Bx,By,..)
byte rampCursor[] = { 10, 65, 26, 98, 51, 98, 40, 150 };                //Ramp page input Cursor   (Ax,Ay,Bx,By,..)
byte rampValuesMinMax[] = { 20, 150, 5, 50, 5, 50, 1, 9 };              //Ramp page min and max for Values (AMin,AMax,BMin,BMax,..)
byte rampPrintDots[] = { 2, 1, 1, 0 };
byte rampPrintDivisor[] = { 10, 10, 10, 1 };

double wireValuesConst[] = { 0, 0, 0,};                            //Ramp page output Values  (V,A,StateStatus)
byte wireCursorConst[] = { 10, 6, 10, 31, 20, 110 };  //Wire page output Cursor  (Ax,Ay,Bx,By,..)
byte wireCurrentValue = 1;
byte wireValuesMinMax[] = { 1, 30 };
int WiperCPosition = 0;

void setup(void) {

  //Encoder init
  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_BUT, INPUT_PULLUP);

  prevClk = digitalRead(ENC_CLK);
  prevDt = digitalRead(ENC_DT);

  //Serial init
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initialize..");

  // Init ads11
  if (!ads.begin()) {
    Serial.println("Failed Init ADS.");
    while (1);
  }
  ads.setGain(GAIN_TWO);                                                       // 1x gain   +/- 4.096V  1 bit =  0.125mV
  ads.setDataRate(RATE_ADS1115_8SPS);                                          //16 Sample per Second
  ads.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_0_1, /*continuous=*/false);  //Current

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);


  menu();
}

void loop() {
  reading = digitalRead(ENC_BUT);
  currClk = digitalRead(ENC_CLK);
  currDt = digitalRead(ENC_DT);

  //Button debounce and call system
  if (reading != lastButtonState) lastDebounceTime = millis();
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState && reading == 0) {
      Serial.print("Button Pressed ");
      Serial.println(counterPosition);
      switch (selectedMode) {
        case 0:  //Main Menu
          selectedMode = counterPosition;
          if (counterPosition == 1) {
            counterPosition = 3;
            initWire();
            wireCurs();
          }
          if (counterPosition == 2) {
            counterPosition = 0;
            initRamp();
            rampCurs();
          }
          break;
        case 1:  //Wire Page
          if (!modifyMode) {
            switch(counterPosition){
              case 3:
                modifyMode = 1;
                counterValue = wireCurrentValue;
                break;
              case 4:
              wireValuesConst[2] = 2;
              break;
              case 5:
              wireValuesConst[2] = 0;
              break;
              case 6:
                selectedMode = 0;
                tft.fillScreen(ST77XX_BLACK);
                counterPosition = 0;
                menu();
                break;
              default:
                break;
            }
          } else {
            modifyMode = 0;
          }
          break;

        case 2:  //Ramp page
          if (!modifyMode) {
            if (counterPosition < 4) {
              modifyMode = 1;
              counterValue = rampValuesCursor[counterPosition];
            } else if (counterPosition == 6) {
              selectedMode = 0;
              tft.fillScreen(ST77XX_BLACK);
              counterPosition = 0;
              menu();
            }
          } else {
            modifyMode = 0;
          }
          break;

        case 3:  //Calib page
          selectedMode = 0;
          break;

        default:
          selectedMode = 0;
          break;
      }
    }
    buttonState = reading;
  }
  lastButtonState = reading;


  //Encoder reading for changes system
  if (currClk != prevClk) {
    if (currDt == currClk) {
      modifyMode ? counterValue-- : counterPosition--;
    } else {
      modifyMode ? counterValue++ : counterPosition++;
    }
    prevClk = currClk;
    prevDt = currDt;
    switch (selectedMode) {
      case 0:
        if (counterPosition > 3) counterPosition = 1;
        if (counterPosition < 1) counterPosition = 3;
        menu();
        break;
      case 1:
        if (!modifyMode) {
          if (counterPosition > 6) counterPosition = 3;
          if (counterPosition < 3) counterPosition = 6;
          wireCurs();
        } else {
          if (counterValue < wireValuesMinMax[0]) counterValue = wireValuesMinMax[0];
          if (counterValue > wireValuesMinMax[1]) counterValue = wireValuesMinMax[1];
          wireCurrentValue = counterValue;
          WiperCPosition = (int)(8.5*counterValue);
          wireCurs();
        }

        break;
      case 2:
        if (!modifyMode) {
          if (counterPosition > 6) counterPosition = 0;
          if (counterPosition < 0) counterPosition = 6;
          rampCurs();
        } else {
          if (counterValue < rampValuesMinMax[counterPosition * 2]) counterValue = rampValuesMinMax[counterPosition * 2];
          if (counterValue > rampValuesMinMax[counterPosition * 2 + 1]) counterValue = rampValuesMinMax[counterPosition * 2 + 1];
          rampValuesCursor[counterPosition] = counterValue;
          rampCurs();
        }
        break;
      case 3: break;
      default: break;
    }
  }

  //ADC Conversion call
  if (ads.conversionComplete()) {
    //cycle voltage and current reading
    if(adcType == 0){Serial.print(adcType);Serial.print("  ");Serial.println((double)(ads.getLastConversionResults()+34)*0.0000625*0.3245033*400,5);}

    switch (adcType) {
      case 0:
        rampValuesConst[1] = (ads.getLastConversionResults()+34)*0.0000625*0.3245033*400 ; //Current mesure (value + offset) * conv_to_volt *conv_to_real_volt* conv_to_amp
        wireValuesConst[1] = (ads.getLastConversionResults()+34)*0.0000625*0.3245033*400 ; //Current mesure (value + offset) * conv_to_volt *conv_to_real_volt* conv_to_amp
        break;
      case 2:
        rampValuesConst[adcType] = (ads.getLastConversionResults()+35)*0.000*7.6354*1.056;//16 bit * conversion * coefficent to fix for each resistor shunt
        break;
      case 3:
        OneVoltRef = ads.getLastConversionResults();
        break;
      default:
        break;
    }
    adcType++;
    if (adcType > 2) adcType = 0;
    ads.startADCReading(adcReadType[adcType], false);

  }

  //Screen Update call
  if (ScreenUpdate) {
    if (selectedMode == 1) {
      
      wireConst();
      controlCurrentWire();
    }
    if (selectedMode == 2) {
      
      rampConst();
    }

    ScreenUpdate = 0;
  }
}

void menu() {
  tft.setTextSize(3);
  for (byte i = 0; i < 4; i++) {
    if (i == counterPosition - 1) {
      tft.fillRect(15, 40 + i * 42, 98, 32, ST77XX_WHITE);
    } else {
      tft.fillRect(15, 40 + i * 42, 98, 32, ST77XX_BLACK);
      tft.drawRect(15, 40 + i * 42, 98, 32, ST77XX_WHITE);
    }

    tft.setCursor(menuCursor[i * 2], menuCursor[i * 2 + 1]);
    tft.setTextColor(menuColors[i]);
    tft.print(menuName[i]);
  }
}
/*
void writeRect(int8_t x, int8_t y, int8_t w, int8_t h,
               uint16_t color) {
  tft.writeFillRect(x, y, w, 1, color);
  tft.writeFillRect(x, y + h - 1, w, 1, color);
  tft.writeFillRect(x, y, 1, h, color);
  tft.writeFillRect(x + w - 1, y, 1, h, color);
}
*/

void rampCurs() {
  tft.setTextSize(1);
  switchColorHomeStartStop();
  for (byte i = 0; i < 4; i++) {
    if (rampValuesCursor[i] == 99) {
      tft.setCursor(rampCursor[i * 2], rampCursor[i * 2 + 1]);
      tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
      tft.print("      ");
    }
    if (i == counterPosition) {
      tft.setTextColor(ST77XX_BLACK, ST77XX_YELLOW);
    } else {
      tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    }

    tft.setCursor(rampCursor[i * 2], rampCursor[i * 2 + 1]);
    tft.print((float)rampValuesCursor[i] / rampPrintDivisor[i], rampPrintDots[i]);

    if (i == 1 || i == 2) {
      tft.setCursor(rampCursor[i * 2] + 50, rampCursor[i * 2 + 1]);
      tft.print((float)rampValuesCursor[i] / rampPrintDivisor[i], rampPrintDots[i]);
    }
  }
}

void switchColorHomeStartStop() {
  if (counterPosition == 6) tft.drawRect(104, 0, 21, 20, ST77XX_BLUE);
  else tft.drawRect(104, 0, 21, 20, ST77XX_WHITE);
  if (counterPosition == 4) tft.drawRect(70, 133, 25, 25, ST77XX_BLUE);
  else tft.drawRect(70, 133, 25, 25, ST77XX_WHITE);
  if (counterPosition == 5) tft.drawRect(100, 133, 25, 25, ST77XX_BLUE);
  else tft.drawRect(100, 133, 25, 25, ST77XX_WHITE);
}

void wireConst() {
  tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
  for (byte i = 0; i < 3; i++) {
    tft.setTextSize(2);
    tft.setCursor(wireCursorConst[i * 2], wireCursorConst[i * 2 + 1]);
    if (i == 2) {
      tft.print("       ");
      tft.setCursor(wireCursorConst[i * 2], wireCursorConst[i * 2 + 1]);
      switch((int)wireValuesConst[i]){
      case 0:
        tft.setTextColor(ST77XX_RED);
        tft.print("Stopped");
        break;
      case 2:
        tft.setTextColor(ST77XX_YELLOW);
        tft.print("Pending");
        break;
      case 1:
        tft.setTextColor(ST77XX_GREEN);
        tft.print("Running");
        break;
      default:
        break;
      }
    } else {
      tft.print("     ");
      tft.setCursor(wireCursorConst[i * 2], wireCursorConst[i * 2 + 1]);
      tft.print(wireValuesConst[i]);

    }
  }
}

void rampConst() {
  tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);
  for (byte i = 0; i < 5; i++) {
    if (i < 2) tft.setTextSize(2);
    else tft.setTextSize(1);
    tft.setCursor(rampCursorConst[i * 2], rampCursorConst[i * 2 + 1]);
    if (i == 4) {
      if (rampValuesConst[i] == 0) {
        tft.print("       ");
        tft.setCursor(rampCursorConst[i * 2], rampCursorConst[i * 2 + 1]);
        tft.setTextColor(ST77XX_RED);
        tft.print("Stopped");
      } else {
        tft.print("       ");
        tft.setCursor(rampCursorConst[i * 2], rampCursorConst[i * 2 + 1]);
        tft.setTextColor(ST77XX_GREEN);
        tft.print("Running");
      }
    } else {
      tft.print("     ");
      tft.setCursor(rampCursorConst[i * 2], rampCursorConst[i * 2 + 1]);
      tft.print(rampValuesConst[i]);

    }
  }
}


void initRamp() {
  tft.fillScreen(ST77XX_BLACK);
  tft.drawRect(0, 0, 100, 50, ST77XX_WHITE);
  tft.drawFastHLine(0, 25, 100, ST77XX_WHITE);

  tft.drawRect(104, 0, 21, 20, ST77XX_WHITE);
  tft.drawBitmap(107, 0, menuButton, 16, 16, ST77XX_WHITE);


  tft.drawFastHLine(5, 55, 7, ST77XX_WHITE);
  tft.drawFastVLine(8, 55, 30, ST77XX_WHITE);
  tft.drawFastHLine(5, 85, 7, ST77XX_WHITE);

  tft.drawFastHLine(23, 85, 25, ST77XX_WHITE);
  tft.drawLine(48, 85, 72, 55, ST77XX_WHITE);
  tft.drawFastHLine(72, 55, 25, ST77XX_WHITE);
  tft.drawLine(98, 55, 122, 85, ST77XX_WHITE);

  tft.drawFastHLine(23, 93, 100, ST77XX_WHITE);
  for (byte i = 0; i < 5; i++) tft.drawFastVLine(23 + i * 25, 90, 7, ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  tft.setCursor(0, 110);
  tft.print("V Max:");
  tft.setCursor(0, 120);
  tft.print("I Max:");
  tft.setCursor(0, 150);
  tft.print("Cycles:");

  tft.drawRect(70, 133, 25, 25, ST77XX_WHITE);
  tft.drawRect(100, 133, 25, 25, ST77XX_WHITE);
  tft.fillTriangle(75, 138, 75, 152, 89, 145, ST77XX_GREEN);
  tft.fillRect(104, 138, 15, 15, ST77XX_RED);

  tft.drawChar(83, 6, 'V', ST77XX_YELLOW, NULL, 2);
  tft.drawChar(83, 31, 'A', ST77XX_YELLOW, NULL, 2);
}

void wireCurs() {
  switchColorHomeStartStop();
  if (counterPosition == 3) tft.drawRect(39, 60, 50, 35, ST77XX_BLUE);
  else tft.drawRect(39, 60, 50, 35, ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  if (modifyMode) {
    tft.setCursor(8, 70);
    tft.print("  ");
    tft.setCursor(48, 70);
    tft.print("  ");
    tft.setCursor(96, 70);
    tft.print("  ");
  }

  tft.setCursor(8, 70);
  if (wireCurrentValue == 1) {
    tft.print("  ");
  } else {
    tft.print(wireCurrentValue - 1);
  }
  tft.setCursor(48, 70);
  tft.print(wireCurrentValue);
  tft.setCursor(96, 70);
  if (wireCurrentValue == 30) {
    tft.print("  ");
  } else {
    tft.print(wireCurrentValue + 1);
  }
}

void initWire() {
  tft.fillScreen(ST77XX_BLACK);
  tft.drawRect(0, 0, 100, 50, ST77XX_WHITE);
  tft.drawFastHLine(0, 25, 100, ST77XX_WHITE);

  tft.drawRect(104, 0, 21, 20, ST77XX_WHITE);
  tft.drawBitmap(107, 0, menuButton, 16, 16, ST77XX_WHITE);

  tft.drawRect(39, 60, 50, 35, ST77XX_WHITE);
  tft.drawChar(75, 70, 'A', ST77XX_YELLOW, NULL, 2);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);


  tft.setCursor(20, 110);
  tft.print("Running");

  tft.drawRect(70, 133, 25, 25, ST77XX_WHITE);
  tft.drawRect(100, 133, 25, 25, ST77XX_WHITE);
  tft.fillTriangle(75, 138, 75, 152, 89, 145, ST77XX_GREEN);
  tft.fillRect(104, 138, 15, 15, ST77XX_RED);

  tft.drawChar(80, 6, 'V', ST77XX_YELLOW, NULL, 2);
  tft.drawChar(80, 31, 'A', ST77XX_YELLOW, NULL, 2);
}

void controlCurrentWire(){
  if(wireValuesConst[2] != 0){
    if(wireValuesConst[1] - wireCurrentValue > 0) WiperCPosition--;
    else WiperCPosition++;

  }
}

/*

String charToString(const char* str) {
  char buffer[6];
  strcpy_P(buffer, str);
  return String(buffer);
}
*/