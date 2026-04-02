#include "Setup.h"
#include "Function.h"
#include "Grafica.h"


#include <Arduino.h>
#include <CoreMutex.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <hardware/clocks.h>
#include <hardware/pll.h>
#include <hardware/adc.h>

int Menu_Mode_Selected = 0;

volatile float voltage_to_display = 0;
volatile float voltage_pre_zeroing = 0;

uint16_t dimezzamentoFreqDiAggiornamento = 0;

volatile int Rotation = 8;

float readedValue[360] = {0};
uint16_t pixelGraph[360] = {0};

void setup(void){
  //Setup RP2040
  SB(250000);
  delay(1000);

  Initial_Setup();

  //Setup Display
  display_bordoEsterno();
  display_main_initial();
  display_main_select(Encoder_pos_menu);
}

void loop(void){
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
          display_main_select(Encoder_pos_menu);
          break;
        case 1: //Settings
          if(Encoder_pos_menu > 4) Encoder_pos_menu = 0;
          if(Encoder_pos_menu < 0) Encoder_pos_menu = 4;
          display_settings_select(Encoder_pos_menu);
          break;
        case 2: //Graph
          //Nulla
          break;
        default: break;
      }
    }else{                  //////MODALITA MODIFICA VALORI //////
      switch(Menu_Mode_Selected){
        case 1: //Settings
          switch(Encoder_pos_menu){
              case 1: //Zero E Setup
                Encoder_pos_value = function_settings_zero_clipping(Encoder_pos_value);
                ZERO_E = (float)Encoder_pos_value/10;
                display_settings_select(Encoder_pos_menu);
              break;
              case 0: //Zero B Setup
                Encoder_pos_value = function_settings_zero_clipping(Encoder_pos_value);
                ZERO_B = (float)Encoder_pos_value/10;
                display_settings_select(Encoder_pos_menu);
              break;
            }
          
          
          break;
        default: break;
      }
    }
  }

  if (SW_flag) {
    if(millis() - lastDebounce > 50){
      SW_flag = false;  // Reset SW flag
      if(digitalRead(ENC_SW)){ //SW pressed Correctly
        switch(Menu_Mode_Selected){
          case 0: //################# SW PRESS MENU  ########################## 
            switch(Encoder_pos_menu){
              case 0  ://Settings Button
                Encoder_pos_menu = 3;
                lcd.fillScreen(TFT_BLACK);
                display_bordoEsterno();
                display_settings_init();
                display_settings_select(Encoder_pos_menu);
                Menu_Mode_Selected = 1;
                break;
              case 1: //Hold Button
                valoriHold[2] = valoriHold[1];
                valoriHold[1] = valoriHold[0];
                valoriHold[0] = voltage_to_display;
                
                display_main_hold_value(valoriHold);
                break;
              case 2: //Zero Button
                if(!E_B_State)ZERO_E = voltage_pre_zeroing;
                else ZERO_B = voltage_pre_zeroing;
                break;
              case 3: //On/Off Button
                Motor_State = !Motor_State; 
                digitalWrite(MOTOR_PIN, Motor_State);       // Accende o spegne il motore
                display_main_motor_OnOff(Motor_State);
                break;
              case 4: //UM Button
                /*
                Rotation = Rotation<<1;
                if(Rotation == 16) Rotation = 1;
                valoriHold[0] = Rotation;
                
                display_main_hold_value(valoriHold);
                */
                UnitaMisura_State = !UnitaMisura_State;
                display_main_mode(UnitaMisura_State, E_B_State);
                function_main_adc_mode(E_B_State);
                
                break;
              case 5: //EB Button
                E_B_State = !E_B_State;       // 0 = E, 1= B 
                //digitalWrite(CAPACITOR_PIN, E_B_State ^ Udine);
                display_main_mode(UnitaMisura_State, E_B_State);
                function_main_adc_mode(E_B_State);
                break;
              
              default: break;
            }
            break;
          case 1: ////################# SW PRESS Settings  ##########################
            switch(Encoder_pos_menu){
              case 4: //Home
                Encoder_pos_menu = 3;
                lcd.fillScreen(TFT_BLACK);
                display_bordoEsterno();
                display_main_initial();
                display_main_select(Encoder_pos_menu);
                Menu_Mode_Selected = 0;
              break;
              
              case 3: //Graph
                lcd.fillScreen(TFT_BLACK);
                display_bordoEsterno();
                display_graph_init();
                Menu_Mode_Selected = 2;
              break;

              case 2: //E Multiplier
                E_Reading_Multiplier = !E_Reading_Multiplier;
                display_settings_E_Mul(E_Reading_Multiplier,true);
                function_main_adc_mode(E_B_State);
                break;
              default: break;
              case 1: //Zero E Setup
                Encoder_pos_value =(int)(ZERO_E*10);
                Modify_mode = !Modify_mode;
              break;
              case 0: //Zero B Setup
                Encoder_pos_value =(int)(ZERO_B*10);
                Modify_mode = !Modify_mode;
              break;
            }
            break;

          case 2: ////################# SW PRESS Graph  ##########################
            Encoder_pos_menu = 3;
            lcd.fillScreen(TFT_BLACK);
            display_bordoEsterno();
            display_main_initial();
            display_main_select(Encoder_pos_menu);
            Menu_Mode_Selected = 0;
            break;
          default: break;
        }
      }
    }
  }



  if(TimerUpdateDisplay_Flag){
    TimerUpdateDisplay_Flag = false;
    Serial.println(voltage_to_display); /// PER SCRIVERE A PC
    if(Menu_Mode_Selected == 0){
      display_main_mul(voltage_to_display);
    }
    if(Menu_Mode_Selected == 2){
      if(dimezzamentoFreqDiAggiornamento++ > 1){
        dimezzamentoFreqDiAggiornamento = 0;
        for(int i = 1; i < 360; i++) readedValue[i-1] = readedValue[i];
        readedValue[359] = voltage_to_display;
        float max_value_graph = readedValue[0];
        float min_value_graph = readedValue[0];
        for (int i = 0; i < 360; i++){
          if (readedValue[i] > max_value_graph) {
              max_value_graph = readedValue[i];
          }
          if (readedValue[i] < min_value_graph) {
              min_value_graph = readedValue[i];
          }
        }
        for(int i = 0; i < 360; i++){
          lcd.drawPixel(i+62,pixelGraph[i],TFT_BLACK);
          pixelGraph[i] = 310-map(readedValue[i]*10,min_value_graph*10,max_value_graph*10,20,290);
          lcd.drawPixel(i+62,pixelGraph[i],TFT_YELLOW);
        }
        lcd.setTextDatum(top_left);
        lcd.setTextColor(TFT_YELLOW,TFT_BLACK);
        lcd.setFont(fontList[7]);
        lcd.setTextSize(0.33);
        
        snprintf(bufferV, sizeof(bufferV), "%05.1f", fabs(max_value_graph));
        lcd.drawString((String)bufferV,13,15);
        snprintf(bufferV, sizeof(bufferV), "%05.1f", fabs(min_value_graph));
        lcd.drawString((String)bufferV,13,280);
        lcd.setFont(fontList[1]);
        lcd.setTextSize(0.65);
        lcd.setTextColor(TFT_YELLOW);
        lcd.drawString("+",3,16);
        lcd.drawString("+",3,281);
        lcd.setColor(TFT_BLACK);
        if(max_value_graph<0){
          lcd.drawRect(7,17,1,3);
          lcd.drawRect(7,22,1,3);
        }
        if(min_value_graph<0){
          lcd.drawRect(7,282,1,3);
          lcd.drawRect(7,287,1,3);
        }
      }
    }
  }
}



//Filtro FIR Mean

#define N_TAPPI 256

volatile int16_t sampleBuffer[256]; // Buffer per i campioni
volatile int readIndex = 0;
volatile int circularIndex;


//Gaussian 64
int16_t tappi32[N_TAPPI/8] ={72, 122, 200, 315, 481, 708, 1006, 1380, 1829, 2339, 2888, 3443, 3963, 4404, 4725, 4894, 4894, 4725, 4404, 3963, 3443, 2888, 2339, 1829, 1380, 1006, 708, 481, 315, 200, 122, 72};
int16_t tappi64[N_TAPPI/4] = {92, 112, 135, 163, 194, 230, 271, 317, 369, 426, 489, 559, 634, 714, 800, 890, 985, 1082, 1182, 1283, 1383, 1483, 1579, 1671, 1758, 1837, 1908, 1969, 2019, 2057, 2083, 2097, 2097, 2083, 2057, 2019, 1969, 1908, 1837, 1758, 1671, 1579, 1483, 1383, 1283, 1182, 1082, 985, 890, 800, 714, 634, 559, 489, 426, 369, 317, 271, 230, 194, 163, 135, 112, 92};
int16_t tappi128[N_TAPPI/2] = {349, 355, 361, 367, 373, 379, 386, 392, 398, 404, 410, 416, 422, 428, 434, 440, 446, 452, 458, 464, 469, 475, 481, 486, 491, 497, 502, 507, 512, 517, 522, 527, 532, 536, 541, 545, 550, 554, 558, 562, 565, 569, 572, 576, 579, 582, 585, 588, 590, 593, 595, 597, 599, 601, 603, 604, 606, 607, 608, 609, 609, 610, 610, 610, 610, 610, 610, 609, 609, 608, 607, 606, 604, 603, 601, 599, 597, 595, 593, 590, 588, 585, 582, 579, 576, 572, 569, 565, 562, 558, 554, 550, 545, 541, 536, 532, 527, 522, 517, 512, 507, 502, 497, 491, 486, 481, 475, 469, 464, 458, 452, 446, 440, 434, 428, 422, 416, 410, 404, 398, 392, 386, 379, 373, 367, 361, 355, 349};
int16_t tappi256[N_TAPPI] = {103, 105, 107, 109, 111, 114, 116, 118, 120, 123, 125, 127, 129, 132, 134, 137, 139, 141, 144, 146, 149, 151, 154, 156, 159, 162, 164, 167, 169, 172, 175, 177, 180, 183, 185, 188, 191, 194, 196, 199, 202, 205, 207, 210, 213, 216, 218, 221, 224, 227, 230, 232, 235, 238, 241, 243, 246, 249, 252, 254, 257, 260, 263, 265, 268, 271, 273, 276, 278, 281, 283, 286, 289, 291, 293, 296, 298, 301, 303, 305, 308, 310, 312, 314, 317, 319, 321, 323, 325, 327, 329, 331, 333, 334, 336, 338, 340, 341, 343, 344, 346, 347, 349, 350, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 363, 364, 364, 365, 365, 366, 366, 366, 367, 367, 367, 367, 367, 367, 367, 367, 366, 366, 366, 365, 365, 364, 364, 363, 363, 362, 361, 360, 359, 358, 357, 356, 355, 354, 353, 352, 350, 349, 347, 346, 344, 343, 341, 340, 338, 336, 334, 333, 331, 329, 327, 325, 323, 321, 319, 317, 314, 312, 310, 308, 305, 303, 301, 298, 296, 293, 291, 289, 286, 283, 281, 278, 276, 273, 271, 268, 265, 263, 260, 257, 254, 252, 249, 246, 243, 241, 238, 235, 232, 230, 227, 224, 221, 218, 216, 213, 210, 207, 205, 202, 199, 196, 194, 191, 188, 185, 183, 180, 177, 175, 172, 169, 167, 164, 162, 159, 156, 154, 151, 149, 146, 144, 141, 139, 137, 134, 132, 129, 127, 125, 123, 120, 118, 116, 114, 111, 109, 107, 105, 103};
void loop1(){

  if(Sample_flag){
    Sample_flag = false;
    sampleBuffer[readIndex] = adc.getLastConversionResults();
    //Serial.println(sampleBuffer[readIndex]);
    readIndex = (readIndex + 1) % (32*(Rotation));  // Aggiorna readIndex in modo circolare
    
    int64_t tempResult = 0;

    for (int i = 0; i < (32*(Rotation)); i++) {
        circularIndex = (readIndex - i + (32*(Rotation))-1) % (32*(Rotation));
        switch(Rotation){
          case 1:
          tempResult += (int32_t) sampleBuffer[circularIndex] * tappi32[i];
          break;
          case 2:
          tempResult += (int32_t) sampleBuffer[circularIndex] * tappi64[i];
          break;
          case 4:
          tempResult += (int32_t) sampleBuffer[circularIndex] * tappi128[i];
          break;
          case 8: 
          tempResult += (int32_t) sampleBuffer[circularIndex] * tappi256[i];
          break;
        }
      }
    
    if(E_Reading_Multiplier){
      if(!E_B_State) {//E
        voltage_pre_zeroing = ((float)(tempResult >> 16))*10240.0/32768.0; //10240
        voltage_to_display = voltage_pre_zeroing - ZERO_E;
      }//B
      else {
        voltage_pre_zeroing = ((float)(tempResult >> 16))*10240.0/32768.0; //2560 Old
        voltage_to_display = voltage_pre_zeroing - ZERO_B;
      }
    } 
    else {
      if(!E_B_State) {//E
        voltage_pre_zeroing = ((float)(tempResult >> 16))*1024.0/32768.0; //1024
        voltage_to_display = voltage_pre_zeroing - ZERO_E;
      }//B
      else {
        voltage_pre_zeroing = ((float)(tempResult >> 16))*1024.0/32768.0; //256 Old
        voltage_to_display = voltage_pre_zeroing - ZERO_B;
      }
    }
    
  }

}
