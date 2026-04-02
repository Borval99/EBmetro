#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include <RPi_Pico_TimerInterrupt.h>
#include "Setup.h"
#include "Menu.h"
#include "Ramp.h"
#include "Wire.h"
#include "Comunication.h"
#include "Settings.h"

int16_t x = 0, y = 0,z = 0; // Coordinate del touchscreen

int Menu_Mode_Selected = 0;

uint8_t data_recived[6] ={0,0,0,0,0,0};

void setup(void){
  Serial.begin(115200);
  delay(1000);

  Initial_Setup();
  if (!com_init_serialcom(19200)) {
    SPL("Failed to initialize Serial1 (Case)");
    while (1);
  }
  com_case_send_off();
  SPL("Serial1 (Case) successfully Initialized");

  //SETUP DISPLAY
  menu_display_bordoEsterno();
  menu_display_initial();
  menu_display_rect(Encoder_pos_menu);
}

void loop(void){
  if (IRQ_flag) {
    IRQ_flag = false;  // Resetta il flag
    // Leggi le coordinate dal touchscreen
    if(getInput()){
    // Stampa le coordinate
    Serial.print("X: ");
    Serial.print(x);
    Serial.print(" Y: ");
    Serial.println(y);
    }
  }
  
  if(Encoder_flag){
    Encoder_flag = false;
    if(!Modify_mode){     //////MODALITA NORMALE //////
      switch(Menu_Mode_Selected){
        case 0: //MENU
          if(Encoder_pos_menu > 2) Encoder_pos_menu = 0;
          if(Encoder_pos_menu < 0) Encoder_pos_menu = 2;
          menu_display_rect(Encoder_pos_menu);
          break;
        case 1: //RAMP
          if(Encoder_pos_menu > 9) Encoder_pos_menu = 0;
          if(Encoder_pos_menu < 0) Encoder_pos_menu = 9;
          ramp_display_select(Encoder_pos_menu);
          break;
        case 2: //Wire
          if(Encoder_pos_menu > 3) Encoder_pos_menu = 0;
          if(Encoder_pos_menu < 0) Encoder_pos_menu = 3;
          wire_display_select(Encoder_pos_menu);
          break;
        case 3: //Settings
          if(Encoder_pos_menu > 3) Encoder_pos_menu = 0;
          if(Encoder_pos_menu < 0) Encoder_pos_menu = 3;
          settings_display_select(Encoder_pos_menu);
          break;

        default: break;
      }
    }else{                  //////MODALITA MODIFICA VALORI //////
      switch(Menu_Mode_Selected){
        case 1: //RAMP 
          Encoder_pos_value = ramp_var_clipping(Encoder_pos_menu,Encoder_pos_value);
          ramp_display_select(Encoder_pos_menu);
          break;  
        case 2: //WIRE
          Encoder_pos_value = wire_var_clipping(Encoder_pos_value);
          wire_display_select(Encoder_pos_menu);
          break;
        case 3: //Settings
          Encoder_pos_value = settings_var_clipping(Encoder_pos_value);
          settings_display_select(Encoder_pos_menu);
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
          case 0  ://RAMP Button
            Encoder_pos_menu = 0;                         //Reset Encoder
            Menu_Mode_Selected = 1;                       //Seleziona la Pagina Ramp
            lcd.fillScreen(TFT_BLACK);                    //Reset schermo
            menu_display_bordoEsterno();                  //Stampa il Bordo bianco
            TimerUpdateDisplay.enableTimer();             //Avvio timer aggiornamento display
            ramp_display_initial();                       //Avvia la procedura iniziale
            ramp_display_handle_selected(selected_handle);//Scrivo Handle 
            ramp_wire_display_status(0);                  //Sicuramente partirà da Stopped
            ramp_display_select(0);                       //Seleziona la posizione home
            com_case_send_on();                           //Richiedi al case di inviare i dati
            break;
          case 1: //WIRE Button
            Encoder_pos_menu = 0;                     //Reset Encoder
            Menu_Mode_Selected = 2;                   //Seleziona la Pagina Wire
            lcd.fillScreen(TFT_BLACK);                //Reset schermo
            menu_display_bordoEsterno();              //Stampa il Bordo binco
            TimerUpdateDisplay.enableTimer();         //Avvio timer aggiornamento display
            wire_display_initial();                   //Avvia la procedura iniziale
            ramp_wire_display_status(0);              //Sicuramente partirà da Stopped
            wire_display_select(0);                   //Seleziona la posizione home
            com_case_send_on();                         //Richiedi al case di inviare i dati
            break;
          case 2: //SETTINGS Button
            Encoder_pos_menu = 1;                     //Reset Encoder
            Menu_Mode_Selected = 3;                   //Seleziona la Pagina Wire
            lcd.fillScreen(TFT_BLACK);                //Reset schermo
            menu_display_bordoEsterno();
            settings_display_initial();
            settings_display_select(1);
            settings_display_print_maxmin(8888,8888);
            break;

          default: break;
        }
        break;
      case 1: ////################# SW PRESS RAMP  ##########################
        switch(Encoder_pos_menu){
          case 0  ://HOME Button
            Encoder_pos_menu = 0;
            Menu_Mode_Selected = 0;
            lcd.fillScreen(TFT_BLACK);
            menu_display_bordoEsterno();
            menu_display_initial();
            menu_display_rect(0);
            com_case_send_off();
            TimerUpdateDisplay.disableTimer();
            break;
          case 1: //Case/Handle Button
            selected_handle++;
            if (selected_handle >3) selected_handle = 0;
            ramp_display_handle_selected(selected_handle);
            break;
          case 2: //Cycles
            Encoder_pos_value = ramp_var_value[2];
            Modify_mode = !Modify_mode;
            break;
          case 3: //Stop Button
            com_stop_all();
            ramp_wire_display_status(0);
            break;
          case 4: //Start Button
            com_start_ramp();
            ramp_wire_display_status(1);
            break;
          case 5: //Ramp time
            Encoder_pos_value = ramp_var_value[5];
            Modify_mode = !Modify_mode;
            break;
          case 6: //Hold time
            Encoder_pos_value = ramp_var_value[6];
            Modify_mode = !Modify_mode;
            break;
          case 7: //Min curren
            Encoder_pos_value = ramp_var_value[7];
            Modify_mode = !Modify_mode;
            break;
          case 8: //Max current
            Encoder_pos_value = ramp_var_value[8];
            Modify_mode = !Modify_mode;
            break;
          case 9: //LastGraph Button
            com_case_send_off();
            Menu_Mode_Selected = 4;
            com_send_lastgraph();
            lcd.fillScreen(TFT_BLACK);
            menu_display_bordoEsterno();
            display_graph_init();
            break;
          
          default: break;
        }
        break;  
      case 2: //################# SW PRESS WIRE  ##########################
        switch(Encoder_pos_menu){
          case 0  ://HOME Button
            Encoder_pos_menu = 0;
            Menu_Mode_Selected = 0;
            lcd.fillScreen(TFT_BLACK);
            menu_display_bordoEsterno();
            menu_display_initial();
            menu_display_rect(0);
            com_case_send_off();
            TimerUpdateDisplay.disableTimer();
            break;
          case 1: //Stop Button
            com_stop_all();
            ramp_wire_display_status(0);
            break;
          case 2: //Start Button
            com_start_wire();
            ramp_wire_display_status(1);
            break;
          case 3: //Current value
            Encoder_pos_value = wire_current_value;
            Modify_mode = !Modify_mode;
            break;
          
          default: break;
        }
        break;
      case 3: //################# SW PRESS SETTINGS  ##########################
        switch(Encoder_pos_menu){
          case 0  ://HOME Button
            Encoder_pos_menu = 0;
            Menu_Mode_Selected = 0;
            lcd.fillScreen(TFT_BLACK);
            menu_display_bordoEsterno();
            menu_display_initial();
            menu_display_rect(0);
            com_case_send_off();
            TimerUpdateDisplay.disableTimer();
            break;
          case 1:
          com_send_request_calibration();
          settings_display_text(1);
          Menu_Mode_Selected = -1;
            break;
          case 2: 
          com_send_request_selfcheck();
          settings_display_text(2);
          Menu_Mode_Selected = -1;
            break;
          case 3:
            Encoder_pos_value = settings_current_value;
            Modify_mode = !Modify_mode;
            break;
          
          default: break;
        }
        
        break;
      case 4:
        //mi porta a 5
        lcd.fillScreen(TFT_BLACK);                    //Reset schermo
        menu_display_bordoEsterno();
        Menu_Mode_Selected = 5;
        lcd.setColor(TFT_BLUE);
        lcd.drawRect(429, 10, 40, 40);
        lcd.drawBitmap(433,14,arrowBack,32,32,TFT_WHITE);
        lcd.setColor(TFT_WHITE);
        lcd.drawRect(30,25,360,270);
        lcd.drawLine(120,25,120,295);
        lcd.drawLine(210,25,210,295);
        lcd.drawLine(300,25,300,295);

        lcd.drawLine(30,70,390,70);
        lcd.drawLine(30,115,390,115);
        lcd.drawLine(30,160,390,160);
        lcd.drawLine(30,205,390,205);
        lcd.drawLine(30,250,390,250);

        lcd.setTextDatum(top_left);
        lcd.setTextColor(TFT_WHITE,TFT_BLACK);
        lcd.setTextSize(1);
        lcd.setFont(fontList[1]);
        lcd.setCursor(40,40);
        lcd.print("Down");
        lcd.setCursor(130,40);
        lcd.print("Rise");
        lcd.setCursor(220,40);
        lcd.print("Up");
        lcd.setCursor(310,40);
        lcd.print("Fall");

        for(int i = 0; i < 5; i++){
          for(int j = 0; j < 4;j++){
            lcd.setCursor(40+90*j,90+45*i);
            //lcd.drawNumber(valori_grafico[1441+j+i*4],40+90*j,75*i);
            lcd.printf("%05.1f",(float)(valori_grafico_32[360+j+i*4]>>5)*0.2980232238);
          }
        }
      break;
      case 5:
        //mi porta a 6
        lcd.fillScreen(TFT_BLACK);                    //Reset schermo
        menu_display_bordoEsterno();
        Menu_Mode_Selected = 6;
        lcd.setColor(TFT_BLUE);
        lcd.drawRect(429, 10, 40, 40);
        lcd.drawBitmap(433,14,arrowBack,32,32,TFT_WHITE);
        lcd.setColor(TFT_WHITE);
        lcd.drawRect(30,25,360,270);
        lcd.drawLine(120,25,120,295);
        lcd.drawLine(210,25,210,295);
        lcd.drawLine(300,25,300,295);

        lcd.drawLine(30,70,390,70);
        lcd.drawLine(30,115,390,115);
        lcd.drawLine(30,160,390,160);
        lcd.drawLine(30,205,390,205);
        lcd.drawLine(30,250,390,250);

        lcd.setTextDatum(top_left);
        lcd.setTextColor(TFT_WHITE,TFT_BLACK);
        lcd.setTextSize(1);
        lcd.setFont(fontList[1]);
        lcd.setCursor(40,40);
        lcd.print("Down");
        lcd.setCursor(130,40);
        lcd.print("Rise");
        lcd.setCursor(220,40);
        lcd.print("Up");
        lcd.setCursor(310,40);
        lcd.print("Fall");

        for(int i = 0; i < 5; i++){
          for(int j = 0; j < 4;j++){
            lcd.setCursor(40+90*j,90+45*i);
            //lcd.drawNumber(valori_grafico[1441+j+i*4],40+90*j,75*i);
            lcd.printf("%05.1f",(float)((valori_grafico_32[360+j+i*4]-valori_grafico_32[360+i*4])>>5)*0.2980232238);
          }
        }
      break;
      case 6:
        //Mi riporta a Ramp
          Encoder_pos_menu = 0;                         //Reset Encoder
          Menu_Mode_Selected = 1;                       //Seleziona la Pagina Ramp
          lcd.fillScreen(TFT_BLACK);                    //Reset schermo
          menu_display_bordoEsterno();                  //Stampa il Bordo bianco
          TimerUpdateDisplay.enableTimer();             //Avvio timer aggiornamento display
          ramp_display_initial();                       //Avvia la procedura iniziale
          ramp_display_handle_selected(selected_handle);//Scrivo Handle 
          ramp_wire_display_status(0);                  //Sicuramente partirà da Stopped
          ramp_display_select(0);                       //Seleziona la posizione home
          com_case_send_on();                           //Richiedi al case di inviare i dati
      break;
      default: break;
    }
  }

  if(Serial1.available()>5){ //Errori ricevuto o WriteBack
    data_recived[0] = Serial1.read();
    switch(data_recived[0]){
      case 0xFF:
        while(Serial1.peek()== 0 && Serial1.available())Serial1.read();
        SPL("Error");
        com_count_errors++;
        switch(com_last_ID_sent){
          case 0: //NULL
            break;
          case 1:
            com_start_ramp();
            break;
          case 2:
            com_start_wire();
            break;
          case 97:
          case 98:
          case 99:
          case 100:
          case 101:
          case 102:
          case 103:
            com_send_data_subID(3,com_last_ID_sent & 0x1F);
            break;
          default:
            com_stop_all();
            break;
        }
        if(com_count_errors > 3) {
          com_count_errors = 0;
          SPL("Throw an Error for Serial1 Comunication Interrupted");
          //QUESTA PARTE é DA FARE GRAFICAMENTE; LO SI FARà POI
        }
        break;
      case 0xA1:
        Serial1.readBytes(data_recived + 1, 5);
        if(data_recived[1] == CRC8(data_recived,1)){
          ramp_wire_display_status(0);
         }else{
          Serial1.write(0xFF); //Errore
         }
        break;
      case 0xA2:
      case 0xA3:
      case 0xA4:
      case 0xA5:
        Serial1.readBytes(data_recived + 1, 5);
        if(data_recived[1] == CRC8(data_recived,1)){
          settings_display_text((data_recived[0] & 0x0F)+1);
          delay(3000);
          Encoder_pos_menu = 1;                     //Reset Encoder
          Menu_Mode_Selected = 3;                   //Seleziona la Pagina Wire
          lcd.fillScreen(TFT_BLACK);                //Reset schermo
          menu_display_bordoEsterno();
          settings_display_initial();
          settings_display_select(1);
          settings_display_print_maxmin(8888,8888);
         }else{
          Serial1.write(0xFF); //Errore
         }
        break;
      case 0xC1://MAX MIN
        Serial1.readBytes(data_recived + 1, 5);
        if(data_recived[5] == CRC8(data_recived,5)){
          Serial.println("OK");
          settings_display_print_maxmin( data_recived[1] << 8 | data_recived[2],data_recived[3] << 8 | data_recived[4]);
         }else{
          Serial1.write(0xFF); //Errore
         }
        break;
      case 0x81:
      case 0x82://UPDATE MULTIMETRO
        Serial1.readBytes(data_recived + 1, 5);
        break;
     case 0xC2:{
        valori_grafico[0] = data_recived[0];
        Serial1.readBytes(valori_grafico + 1, 1521);
        if(valori_grafico[1521] == CRC8(valori_grafico,1521)){
          Serial.println("ok");
          
        }else{
          Serial.println("bad");
        }
        //for(int i = 0; i<1462; i++)Serial.print(valori_grafico[i],HEX);
        
        ramp_show_last_graph();
     break;
     }
      default:
        while(Serial1.peek() == 0 && Serial1.available())Serial1.read();
        break;
    } 
    //Serial.println(data_recived[0],HEX);
  }

  if(TimerUpdateDisplay_Flag){
    TimerUpdateDisplay_Flag = false;
    if(data_recived[0] >> 5 == 4){
        if(Menu_Mode_Selected == 1){
          if((data_recived[0] & 0b00011111) == 1){
            uint16_t voltage = (data_recived[1] << 8) | data_recived[2];
            uint16_t current = (data_recived[3] << 8) | data_recived[4];
            ramp_wire_display_mul((float)voltage/100, (float)current/100, 0);
          }
          if((data_recived[0] & 0b00011111) == 2){
            int voltage = ((data_recived[1] << 8) | data_recived[2]) - 16384;
            uint16_t current = (data_recived[3] << 8) | data_recived[4];
            ramp_wire_display_mul((float)voltage/10, (float)current/100, 1);
          }
        }else if(Menu_Mode_Selected == 2){
          if((data_recived[0] & 0b00011111) == 1){
            uint16_t voltage = (data_recived[1] << 8) | data_recived[2];
            uint16_t current = (data_recived[3] << 8) | data_recived[4];
            ramp_wire_display_mul((float)voltage/100, (float)current/100, 0);
          }
        }
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
