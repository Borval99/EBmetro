#include "Setup.h"
#include "Grafica.h"

int16_t x = 0, y = 0,z = 0; // Coordinate del touchscreen

int Menu_Mode_Selected = 0;

float voltage_to_display = 0;

uint16_t misureEffettuate = 0;
int32_t sommaLetture = 0;
volatile int16_t adc_16voltage;

double filteredValue = 0;
volatile double filteredValue2 = 0;

void setup(void){
  //Setup RP2040
  SB(250000);
  delay(1000);

  Initial_Setup();

  
  int32_t padding = lcd.textWidth("00.00",fontList[7]); // get the width of the text in pixels
  lcd.setTextPadding(padding);

  //Setup Display
  display_bordoEsterno();
  display_initial();
  display_select(3);
}

void loop(void){
  if (IRQ_flag) {       // Controllo Touch
    IRQ_flag = false;   // Resetta il flag
    if(getInput()){
      SP("X: ");
      SP(x);
      SP(" Y: ");
      SPL(y);
    }
  }

  if(Encoder_flag){       // ENCODER
    Encoder_flag = false;
    if(!Modify_mode){     //////MODALITA NORMALE //////
      switch(Menu_Mode_Selected){
        case 0: //Display
          if(Encoder_pos_menu > 5) Encoder_pos_menu = 0;
          if(Encoder_pos_menu < 0) Encoder_pos_menu = 5;
          display_select(Encoder_pos_menu);
          break;
        case 1: //Settings
          break;


        default: break;
      }
    }else{                  //////MODALITA MODIFICA VALORI //////
      switch(Menu_Mode_Selected){
        case 1: //Settings
          break;
        
        default: break;
      }
    }
  }

  if (SW_flag) {
    SW_flag = false;  // Reset SW flag
    switch(Menu_Mode_Selected){
      case 0: //################# SW PRESS MENU  ########################## 
        switch(Encoder_pos_menu){
          case 0  ://Settings Button
            break;
          case 1: //Hold Button
            break;
          case 2: //Zero Button
            break;
          case 3: //On/Off Button
            break;
          case 4: //UM Button
            break;
          case 5: //EB Button
            break;
          
          default: break;
        }
        break;
      case 1: ////################# SW PRESS Settings  ##########################
        switch(Encoder_pos_menu){
          default: break;
        }
        break;  
      
      default: break;
    }
  }

  if(TimerSamplingADC_Flag){        // Letture ADC
    TimerSamplingADC_Flag = false;

    if (++misureEffettuate >= 250){
       display_mul(((float)sommaLetture/250.0)*256.0/32768.0,0); //calcola il segnale in mV segnale*2500mV Vref/16777.215 per visualizzare mV; 
      Serial.println(adc_16voltage);
      misureEffettuate = 0;
      sommaLetture = 0;
    } 
    sommaLetture += adc_16voltage;
    Serial.println(adc_16voltage);  // Stampa dati filtrati
  }

  if(TimerUpdateDisplay_Flag){
    TimerUpdateDisplay_Flag = false;
    if(Menu_Mode_Selected == 0){
      display_mul(voltage_to_display,0);
    }
  }
  
  
}

uint8_t getInput() {
  SPI.beginTransaction(SPISettings(TOUCH_SPI_SPEED, MSBFIRST, SPI_MODE0));

  digitalWrite(T_CS, LOW);

  SPI.transfer(0xB1);
  int16_t z1 = SPI.transfer16(0xC1) >> 3;
  int16_t z2 = SPI.transfer16(0x91) >> 3;
  x = SPI.transfer16(0xD0) >> 3;
  y = SPI.transfer16(0x00) >> 3;

  digitalWrite(T_CS, HIGH);

  SPI.endTransaction();

  z = z1 + 4095 - z2;

  if (z > 500) {
    return 1;  // Indica che il touchscreen è premuto
  } else {
    return 0;  // Indica che il touchscreen non è premuto
  }
}

// SECONDO CORE
#define FILTER_TAP_NUM 25
static double filter_taps2[FILTER_TAP_NUM] = {
  -0.0008556673703384017,
  -0.016551961023958676,
  0.002338295899390626,
  0.012798161822572016,
  -0.023841738625919606,
  0.013865071149987872,
  0.019496638803684633,
  -0.05244549999000805,
  0.04584162042087082,
  0.02516032838065368,
  -0.14502317930683475,
  0.25909054520566527,
  0.6939918270842895,
  0.25909054520566527,
  -0.14502317930683475,
  0.02516032838065368,
  0.04584162042087082,
  -0.05244549999000805,
  0.019496638803684633,
  0.013865071149987872,
  -0.023841738625919606,
  0.012798161822572016,
  0.002338295899390626,
  -0.016551961023958676,
  -0.0008556673703384017
};

static double filter_taps[FILTER_TAP_NUM] = { 0.004437209876278955, 0.004641199138578884, 0.0015687698602571659, -0.007637548356863077, -0.021458662821347986, -0.03371596859924979, -0.03486295299528442, -0.015883735075987372, 0.026675299092274746, 0.08718985140255735, 0.151244109489587, 0.2002313762124777, 0.21857732760316803, 0.2002313762124777, 0.151244109489587, 0.08718985140255735, 0.026675299092274746, -0.015883735075987372, -0.03486295299528442, -0.03371596859924979, -0.021458662821347986, -0.007637548356863077, 0.0015687698602571659, 0.004641199138578884, 0.004437209876278955 };
double samples[FILTER_TAP_NUM] = {0};
void setup1(){
  while(!Serial);
}

void loop1(){
  if(TimerSamplingADC_Flag1){
    TimerSamplingADC_Flag1 = false;
    
    adc_16voltage = adc.getLastConversionResults();
    samples[0] = (double)(adc_16voltage)/65536.0;

    filteredValue = 0;

    for (int i = 0; i < FILTER_TAP_NUM; i++) {
      filteredValue += filter_taps[i] * samples[i];
    }
    filteredValue2 = filteredValue;
    for (int i = FILTER_TAP_NUM - 1; i > 0; i--) {
      samples[i] = samples[i - 1];
    }

  }

}






