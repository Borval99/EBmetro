#include "Setup.h"
#include "Function.h"
#include "Grafica.h"

int Menu_Mode_Selected = 0;

volatile float voltage_to_display = 0;
volatile float voltage_pre_zeroing = 0;

uint16_t dimezzamentoFreqDiAggiornamento = 0;


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
                UnitaMisura_State = !UnitaMisura_State;
                display_main_mode(UnitaMisura_State, E_B_State);
                function_main_adc_mode(E_B_State,E_Reading_Multiplier);
                break;
              case 5: //EB Button
                E_B_State = !E_B_State;       // 0 = E, 1= B 
                digitalWrite(CAPACITOR_PIN, E_B_State ^ Udine);
                display_main_mode(UnitaMisura_State, E_B_State);
                function_main_adc_mode(E_B_State,E_Reading_Multiplier);
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
                function_main_adc_mode(E_B_State,E_Reading_Multiplier);
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
    Serial.println(voltage_to_display);
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
volatile int16_t sampleBuffer[64]; // Buffer per i campioni
volatile int readIndex = 0;
volatile int circularIndex;

#define N_TAPPI 64
//Gaussian 64
int16_t tappi[N_TAPPI] = {92, 112, 135, 163, 194, 230, 271, 317, 369, 426, 489, 559, 634, 714, 800, 890, 985, 1082, 1182, 1283, 1383, 1483, 1579, 1671, 1758, 1837, 1908, 1969, 2019, 2057, 2083, 2097, 2097, 2083, 2057, 2019, 1969, 1908, 1837, 1758, 1671, 1579, 1483, 1383, 1283, 1182, 1082, 985, 890, 800, 714, 634, 559, 489, 426, 369, 317, 271, 230, 194, 163, 135, 112, 92};
//Blackman 64
//int16_t tappi[N_TAPPI] = {0, 2, 9, 21, 37, 60, 89, 126, 171, 224, 287, 360, 443, 536, 639, 752, 873, 1002, 1137, 1277, 1418, 1560, 1700, 1835, 1963, 2081, 2187, 2280, 2356, 2414, 2454, 2474, 2474, 2454, 2414, 2356, 2280, 2187, 2081, 1963, 1835, 1700, 1560, 1418, 1277, 1137, 1002, 873, 752, 639, 536, 443, 360, 287, 224, 171, 126, 89, 60, 37, 21, 9, 2, 0};

void loop1(){

  if(Sample_flag){
    Sample_flag = false;
    if(E_B_State || (!E_B_State && E_Reading_Multiplier))
      sampleBuffer[readIndex] = -adc.getLastConversionResults();
    else
      sampleBuffer[readIndex] = adc.getLastConversionResults();


    readIndex = (readIndex + 1) % N_TAPPI;  // Aggiorna readIndex in modo circolare
    
    int64_t tempResult = 0;

    for (int i = 0; i < N_TAPPI; i++) {
        circularIndex = (readIndex - i + N_TAPPI-1) % N_TAPPI;
        tempResult += (int32_t) sampleBuffer[circularIndex] * tappi[i];
      }

    if(!E_B_State) {//E
      voltage_pre_zeroing = ((float)(tempResult >> 16))*1024.0/32768.0; 
      voltage_to_display = voltage_pre_zeroing - ZERO_E;
    }//B
    else {
      voltage_pre_zeroing = ((float)(tempResult >> 16))*256.0/32768.0; 
      voltage_to_display = voltage_pre_zeroing - ZERO_B;
    }
  }

}
