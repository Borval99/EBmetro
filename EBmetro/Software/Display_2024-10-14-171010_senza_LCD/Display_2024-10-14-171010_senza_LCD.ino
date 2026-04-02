#include "Setup.h"
#include "Grafica.h"
//#include "Scheduler.h"

int16_t x = 0, y = 0,z = 0; // Coordinate del touchscreen

int Menu_Mode_Selected = 0;

float voltage_RAW = 0.0;
float voltage_to_display = 0;

int16_t last_adc_16voltage = 0;
volatile int16_t adc_16voltage;

uint16_t misureEffettuate = 0;
int32_t sommaLetture = 0;


void setup(void){
  //Setup RP2040
  SB(250000);
  delay(1000);

  Initial_Setup();

  digitalWrite(CAPACITOR_PIN,0);

  
  int32_t padding = lcd.textWidth("0",fontList[7]); // get the width of the text in pixels
  lcd.setTextPadding(padding);

  //Setup Display
  display_bordoEsterno();
  display_initial();
  display_select(3);
}

void loop(void){
/*  unsigned long tempo = millis();
  Serial.println(tempo);
  for (int i = 0; i<1000; i++){
    float h = random (10.0, 200.0); 
    display_mul(h,1);
  } //
//}   // Togliere questa graffa dopo il test scrittura
*/
  if (IRQ_flag) {
    IRQ_flag = false;  // Resetta il flag
    if(getInput()){
      SP("X: ");
      SP(x);
      SP(" Y: ");
      SPL(y);
    }
  }

  if(Encoder_flag){
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
            lcd.setTextColor(TFT_WHITE,TFT_BLACK);
            lcd.setFont(fontList[3]);
            lcd.setTextSize(1);
            lcd.setCursor(10, 156);
            lcd.print(voltage_to_display, 2);
            break;
          case 2: //Zero Button
            Zero_State = !Zero_State;
            if (Zero_State)  ZERO = voltage_RAW; else ZERO = 0.0;
            break;
          case 3: //On/Off Motor Button
            Motor_State = !Motor_State; 
            digitalWrite(CAPACITOR_PIN, Motor_State);   // collega condensatore da 1 uF
            digitalWrite(MOTOR_PIN, Motor_State);       // Accende o spegne il motore
            break;
          case 4: //U.a.V/m uT Button
            UnitaMisura_State = !UnitaMisura_State; // cambia tra Unità arbitrarie e Unità del campo
            lcd.setTextColor(TFT_YELLOW,TFT_BLACK);
            lcd.setFont(fontList[2]);
            lcd.setCursor(114, 250);
            if (!UnitaMisura_State) {
                lcd.setCursor(116, 250);
                lcd.setTextSize(1.3);
                lcd.print("mV");
              } else
              if (E_B_State) {
                  lcd.setCursor(116, 250);
                  lcd.setTextSize(1.7,1.3);
                  lcd.print("uT");
                } else {  
                  lcd.setCursor(116, 250);
                  lcd.setTextSize(1,1.3);
                  lcd.print("V/m");
                  }
            break;
          case 5: //EB Button 
            E_B_State = !E_B_State;       // 0 = E, 1= B 
            digitalWrite(CAPACITOR_PIN, !E_B_State);
            lcd.setTextColor(TFT_YELLOW,TFT_BLACK);
            lcd.setFont(fontList[3]);
            lcd.setTextSize(1.3);
            lcd.setCursor(38, 242);
            if (E_B_State) lcd.print("B"); else lcd.print("E");
            if (UnitaMisura_State){
              if (E_B_State) {
                  lcd.setFont(fontList[2]);
                  lcd.setCursor(116, 250);
                  lcd.setTextSize(1.7,1.3);
                  lcd.print("uT");
                  // Unità di misura vicino al valore
                  lcd.setColor(TFT_BLACK);
                  lcd.fillRect(346,55,100,65);
                  lcd.setTextSize(1);
                  lcd.setFont(fontList[3]);
                  lcd.setTextSize(1.3);
                  lcd.setTextColor(TFT_YELLOW);
                  lcd.setCursor(346,70);
                  lcd.print("u");
                  lcd.setFont(fontList[4]);
                  lcd.setCursor(375,55);
                  lcd.setTextSize(1);
                  lcd.print("T");
                } else { 
                  lcd.setFont(fontList[2]);
                  lcd.setCursor(116, 250);
                  lcd.setTextSize(1,1.3);
                  lcd.print("V/m");
                  // Unità di misura vicino al valore
                  lcd.setColor(TFT_BLACK);
                  lcd.fillRect(346,55,100,65);
                  lcd.setTextSize(1);
                  lcd.setFont(fontList[4]);
                  lcd.setTextColor(TFT_YELLOW);
                  lcd.setCursor(346,55);
                  lcd.print("V");
                  lcd.setTextSize(1);
                  lcd.setFont(fontList[3]);
                  lcd.setCursor(385,80);
                  lcd.print("/m");
                }
            } else {
                lcd.setColor(TFT_BLACK);
                lcd.fillRect(346,55,100,65);
                lcd.setTextSize(1);
                lcd.setFont(fontList[4]);
                lcd.setTextColor(TFT_YELLOW);
                lcd.setCursor(374,55);
                lcd.print("V");

                lcd.setTextSize(1);
                lcd.setFont(fontList[3]);
                lcd.setCursor(345,80);
                lcd.print("m");
                }
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

  if(Sample_flag){
    Sample_flag = false;

    adc_16voltage = adc.getLastConversionResults();
    
/*    int16_t temp = adc_16voltage - last_adc_16voltage;
    if(abs(temp) > 30){
      adc_16voltage = last_adc_16voltage+temp/16;
    }
    last_adc_16voltage = adc_16voltage;
*/

    if (++misureEffettuate >= 60){
      voltage_RAW = ((float)sommaLetture/60.0)*256.0/32768.0; //calcola il segnale in mV segnale*2500mV Vref/16777.215 per visualizzare mV; 
      misureEffettuate = 0;
      sommaLetture = 0;
      voltage_to_display = voltage_RAW - ZERO;
      Serial.println(voltage_to_display,3);
    } 
    sommaLetture += adc_16voltage;
    
//    Serial.println(voltage_to_display);
    
  } 

  if(TimerUpdateDisplay_Flag){
    TimerUpdateDisplay_Flag = false;
    if(Menu_Mode_Selected == 0){
    display_mul(voltage_to_display,0);
    }
  } 
}     // End LOOP


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

