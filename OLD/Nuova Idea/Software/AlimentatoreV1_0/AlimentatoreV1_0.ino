#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0

// Ho provato ad attivare il dma per lo schermo teoricamente se funziona è x2 piu veloce
#define USE_SPI_DMA
#define ARDUINO_SAMD_ZERO

#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735
#include <Adafruit_ADS1X15.h> // ADC Library 
#include <Tone.cpp>           // Modified, add a new function to check tone status
//TFT Pins
#define TFT_CS 2
#define TFT_RST -1
#define TFT_DC 3
//Encoder Pins
#define ENC_CLK 0
#define ENC_DT 1
#define ENC_BUT 4

// TFT Setup
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

//ADS Setup
Adafruit_ADS1115 ads;

// 0 V+curr
// 1 V-curr(1Vref)
// 2 V+volt
// 3 V-volt

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

unsigned long previousMillisScreen = 0;
unsigned long currentMillisScreen = 0;

unsigned long lastRampTime = 0; //used for ramp mode, counting the millis that current stay al teh same level

//Variables for Button and Encoder
bool buttonState = 0, lastButtonState = 1, reading = 1;
unsigned long lastDebounceTime = 0;
unsigned int debounceDelay = 250;  //time in ms

volatile bool prevClk, currClk, currDt;
volatile int counterPosition = 0, counterValue = 0;

volatile bool updateScreenEncoder = 0;

volatile bool buttonPressed = 0;
//States for GUI implementation
byte selectedMode = 0, modifyMode = 0;;
volatile bool ScreenUpdate = 0;

//Variables for ADC
uint16_t adcReadType[] = {ADS1X15_REG_CONFIG_MUX_DIFF_2_3,ADS1X15_REG_CONFIG_MUX_DIFF_1_3, ADS1X15_REG_CONFIG_MUX_SINGLE_3,ADS1X15_REG_CONFIG_MUX_SINGLE_0};
byte adcType = 0;
int counter_ADC_conversion = 0;

bool backSelected[] = { 0, 1, 0, 0, 0, 0, 0, 0, 0 };                         //Array used for all Background status in every GUI page
const char menuName[4][6] = { "Mode", "Wire", "Ramp", "Calib" };      //Main menu labels (selectedMode = 0)
int menuColors[] = { ST77XX_WHITE, ST77XX_RED, ST77XX_GREEN, ST77XX_BLUE };  //Main menu colors
byte menuCursor[] = { 30, 10, 30, 45, 30, 86, 20, 130 };                     //Main menu Cursor (Ax,Ay,Bx,By,..)

int OneVoltRef = 8000;
double rampValuesConst[] = { 0, 0, 0, 0, 0 };                            //Ramp page output Values  (V,A,VMax,Vmin,StateStatus)
float rampValuesCursor[] = { 120, 25, 25, 5 };                           //Ramp page input Values   (I,T1,T2,Cyc,Play,Stop,Home)
byte rampCursorConst[] = { 10, 6, 10, 31, 35, 110, 35, 120, 76, 123 };  //Ramp page output Cursor  (Ax,Ay,Bx,By,..)
byte rampCursor[] = { 10, 65, 26, 98, 51, 98, 40, 150 };                //Ramp page input Cursor   (Ax,Ay,Bx,By,..)
byte rampValuesMinMax[] = { 20, 150, 5, 50, 5, 50, 1, 9 };              //Ramp page min and max for Values (AMin,AMax,BMin,BMax,..)
byte rampPrintDots[] = { 2, 1, 1, 0 };
byte rampPrintDivisor[] = { 10, 10, 10, 1 };

double wireValuesConst[] = { 0, 0, 0,};                            //Ramp page output Values  (V,A,StateStatus)
byte wireCursorConst[] = { 10, 6, 10, 31, 20, 110 };  //Wire page output Cursor  (Ax,Ay,Bx,By,..)
byte wireCurrentValue = 1;
byte wireValuesMinMax[] = { 1, 30 };

byte rampStatus = 0; // Used for knowing the status of the ramp, 0 off, 1 ramp up, 2 up, 3 ramp down, 4 down

bool currentOnWireStatus = false; //Result of current Setting on wir:  true = Ready, false = Not Ready

int RampIteration = 0; //Variable for counting ramp position

//motore e controllo
int valuesMotor[] = {0,3000,10};  //Motor Pot Position   (Pos, Max, Min)
const uint8_t STEP_PIN = 7;
const uint8_t DIRECTION_PIN = 6;

// SETUP TIMER SCREEN UPDATE
void setupTimerTC3() {

  // Abilita il clock per il TC3
  PM->APBCMASK.reg |= PM_APBCMASK_TC3;

  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(4) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K;  // Sorgente del clock a 32.768 kHz
  while (GCLK->STATUS.bit.SYNCBUSY);
  // Abilita il generatore di clock per TC3
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK4 | GCLK_CLKCTRL_ID_TCC2_TC3;  // Connetti GCLK0 a TC3
  while (GCLK->STATUS.bit.SYNCBUSY);
  // Configura TC3 come un timer a 16 bit in modalità di conteggio (count-up mode)
  TC3->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_PRESCALER_DIV16 | TC_CTRLA_WAVEGEN_NFRQ;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
  // Imposta il periodo del timer per ottenere un intervallo di 200 ms
  uint16_t period = 400;  // Fino a 1000 conteggi
  TC3->COUNT16.CC[0].reg = period;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
  // Abilita l'interrupt per TC3
  TC3->COUNT16.INTENSET.reg = TC_INTENSET_MC0;
  NVIC_EnableIRQ(TC3_IRQn);
  // Abilita il timer TC3
  TC3->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY);

}

void setup(void) {

  Serial.begin(115200);
  //while(!Serial);
  delay(2000);
  Serial.println("Initialize..");

  setupTimerTC3();
  Serial.println("Setup Timer: OK");

  //Encoder init
  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_BUT, INPUT_PULLUP);

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIRECTION_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(ENC_CLK), updateCounterCall, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_DT), updateCounterCall, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_BUT), buttonPressedCall, FALLING);
  Serial.println("Setup Interrupt: OK");

  //Init ads11
  if (!ads.begin()) {
   Serial.println("Setup ADC: Not OK");
   while (1);
  }
  Serial.println("Setup ADC: OK");

  ads.setGain(GAIN_TWO);                                                       // 1x gain   +/- 4.096V  1 bit =  0.125mV
  ads.setDataRate(RATE_ADS1115_128SPS);                                          //16 Sample per Second
  ads.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_2_3, /*continuous=*/false);  //Current
  tft.initR(INITR_BLACKTAB);
  tft.setSPISpeed(24000000);
  tft.fillScreen(ST77XX_BLACK);
  
  //CONTROLLO INIZIALE CORRENTE
  while(wireValuesConst[1] > 0.35 || wireValuesConst[1] == 0 ){
    checkFinishedADCConversion();
    controlCurrentWire(0.3);

  }

  wireValuesConst[2] = 0;
  menu();
}

// INTERRUPT ENCODER
void updateCounterCall() {
  currClk = digitalRead(ENC_CLK);
  currDt = digitalRead(ENC_DT);
  if (currClk != prevClk) {
    if (currDt != currClk) { 
      modifyMode ? counterValue++ : counterPosition--;
    }
    else {
      modifyMode ? counterValue-- : counterPosition++;
    }
    updateScreenEncoder = 1;
    prevClk = currClk;
  }
}

// INTERRUPT BUTTON
void buttonPressedCall() {
  noInterrupts();
  if (millis() - lastDebounceTime > debounceDelay) {
    buttonPressed = 1;
    lastDebounceTime = millis();
  }
  interrupts();
}

// INTERRUPT SCREEN UPDATE
void TC3_Handler() {
  // Controlla se l'interrupt è stato causato dal timer TC3
  if (TC3->COUNT16.INTFLAG.bit.MC0) {
    TC3->COUNT16.INTFLAG.bit.MC0 = 1;  // Resetta la bandiera di interrupt
    ScreenUpdate = true;
    TC3->COUNT16.COUNT.reg = 0;  // Resetta il contatore
  }
}

void loop() {
  //Button call system
  if (buttonPressed) {
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
      wireValuesConst[2] = 0;
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
      wireValuesConst[2] = 0;
        if (!modifyMode) {
          if (counterPosition < 4) {
            modifyMode = 1;
            counterValue = rampValuesCursor[counterPosition];
          }else if(counterPosition == 4){
            wireValuesConst[2] = 2;
          
          }else if(counterPosition == 5){
            wireValuesConst[2] = 0;
          
          }else if (counterPosition == 6) {
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
    buttonPressed = 0;
  }
  
  //Encoder reading for changes system
  if (updateScreenEncoder) {
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
    updateScreenEncoder = 0;
  }

  //ADC Conversion call
  checkFinishedADCConversion();

  //Screen Update call
  if (ScreenUpdate) {
    if (selectedMode == 1) {
      
      wireConst();
    }
    if (selectedMode == 2) {
      
      rampConst();
    }

    ScreenUpdate = false;
  }

  // IF IN WIRE
  if(selectedMode == 1){
    //IF WIRE PENDIG AND RUNNING
    if(!wireValuesConst[2] == 0){
      controlCurrentWire(wireCurrentValue);
    }else{
      controlCurrentWire(0.3);
      wireValuesConst[2] = 0;
    }
  }

  // IF IN RAMP
  if(selectedMode == 2){
    if(!wireValuesConst[2] == 0){
      rampRunningRoutine();
      
    }else{
      controlCurrentWire(0.3);
      wireValuesConst[2] = 0;
    }
  }

}

// Men
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

// CONTROLLO MAX E MIN POTENZIOMETRO
void controlCurrentWire(float wireCurrent, int freq ){
  if (valuesMotor[1] <= valuesMotor[0]){
      digitalWrite(DIRECTION_PIN, 1);
  }
  if (valuesMotor[2] >= valuesMotor[0]){
      digitalWrite(DIRECTION_PIN, 0);
  }
  setCurrent(wireCurrent, freq);
}

void controlCurrentWire(float wireCurrent ){
  int freq = 60000;
  if (valuesMotor[1] <= valuesMotor[0]){
      digitalWrite(DIRECTION_PIN, 1);
  }
  if (valuesMotor[2] >= valuesMotor[0]){
      digitalWrite(DIRECTION_PIN, 0);
  }
  setCurrent(wireCurrent, freq);
}

// SET CURRENT BY MOTOR
void setCurrent(float curValue, int freq){
  
  if(curValue < 0.3) curValue = 0.3;
  if(curValue > 30) curValue = 30;
  if(wireValuesConst[1] < curValue) digitalWrite(DIRECTION_PIN, 0);
  if(wireValuesConst[1] >= curValue) digitalWrite(DIRECTION_PIN, 1);
  if(!istone() && abs(wireValuesConst[1] - curValue) > 0.05){
    wireValuesConst[2] = 2;
    currentOnWireStatus = false;
    if(abs(wireValuesConst[1] - curValue) < 0.5){
      tone(STEP_PIN,1000,10);
    }else{
      tone(STEP_PIN,freq,10);
    }
    
  }else{
      currentOnWireStatus = true;
      wireValuesConst[2] = 1;
  }
  if(wireValuesConst[2] == 2) {
    stopTimer(); 
  }else if(!isTimerRunning()) {
    startTimer(); 
    ScreenUpdate = 1;
  }
}

void rampRunningRoutine(){
  
  switch(rampStatus){
    case 0:   //1st RUN
      controlCurrentWire(1);
      if(currentOnWireStatus == true){
           lastRampTime = millis();
           rampStatus = 1;
           Serial.print(rampStatus);
            Serial.print(" al tempo: ");
            Serial.println(millis());
      }
      break;

    case 1:   //DOWN
      controlCurrentWire(1);
      if(currentOnWireStatus == true && millis() - lastRampTime > rampValuesCursor[2]*100) {
        
        rampStatus = 2;
        Serial.print(rampStatus);
        Serial.print(" al tempo: ");
        Serial.println(millis());
      }
      break;

    case 2:  //RISE    //FORMULA 3000 * I/T
      controlCurrentWire(rampValuesCursor[0]/10+1,3000*(rampValuesCursor[0])/rampValuesCursor[1]);
      if(currentOnWireStatus == true){
        rampStatus = 3;
        lastRampTime = millis();
        Serial.print(rampStatus);
        Serial.print(" al tempo: ");
        Serial.println(millis());
      }
      break;

    case 3: //UP
      controlCurrentWire(rampValuesCursor[0]/10+1);
      if(currentOnWireStatus == true && millis() - lastRampTime > rampValuesCursor[2]*100) {
        rampStatus = 4;
        Serial.print(rampStatus);
        Serial.print(" al tempo: ");
        Serial.println(millis());
      }
      break;

    case 4: //FALL
      controlCurrentWire(1,3000*(rampValuesCursor[0])/rampValuesCursor[1]);
      if(currentOnWireStatus == true) rampStatus = 4;
      
      if(currentOnWireStatus == true && RampIteration < rampValuesCursor[3]){
          RampIteration++;
          rampStatus = 1;
          lastRampTime = millis();
          Serial.print(rampStatus);
          Serial.print(" al tempo: ");
          Serial.println(millis());
        }
      if(currentOnWireStatus == true && RampIteration == rampValuesCursor[3]){
        RampIteration = 0;
        wireValuesConst[2] = 0;
        rampStatus = 0;

        Serial.print(rampStatus);
        Serial.print(" al tempo: ");
        Serial.println(millis());
      }
      
    break;

    default:
    break;

  }
}


void checkFinishedADCConversion() {
  //Controllo quando ha finito la converisone, ogni 1000 letture controllo anche il ref
  if (ads.conversionComplete()) {
    //cycle voltage and current reading
    //if(adcType == 1){Serial.print(adcType);Serial.print("  ");Serial.println((double)(ads.getLastConversionResults()+0));}//*0.0000625*0.3245033*400,5);}

    switch (adcType) {
      case 0:
        //lettura corrente
        rampValuesConst[1] = (-ads.getLastConversionResults() - 45) * 0.0000625 * 0.3245033 * 400;  //Current mesure (value + offset) * conv_to_volt *conv_to_real_volt* conv_to_amp
        wireValuesConst[1] = rampValuesConst[1];
        break;
      case 1:
        //lettura potenziometro
        valuesMotor[0] = ads.getLastConversionResults();
        break;
      case 2:
        //lettura vref 1V
        OneVoltRef = ads.getLastConversionResults();
        break;
      case 3:
        //lettura tensione
        break;
      default:
        break;
    }
    adcType++;
    if (adcType > 2) adcType = 0;
    if (++counter_ADC_conversion > 1000) {
      adcType = 2;
      counter_ADC_conversion = 0;
    }
    ads.startADCReading(adcReadType[adcType], false);
  }
}

void stopTimer() {
  // Disabilita il timer TC3
  TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
}

void startTimer() {
  // Riabilita il timer TC3
  TC3->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
  while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
}

bool isTimerRunning() {
  return TC3->COUNT16.CTRLA.bit.ENABLE; // Restituisce true se il timer è abilitato, false altrimenti
}