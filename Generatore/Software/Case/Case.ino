#include <Adafruit_ADS1X15.h>
#include "MCP4725.h"
#include <Wire.h>
#include <RPi_Pico_TimerInterrupt.h>
#include "Adafruit_HX711.h"
#include <EEPROM.h>
#include "Comunication.h"
#include "Setup.h"
#include "Functions.h"
//Filtro fir Media Manico
volatile int32_t sampleBuffer[64]; // Buffer per i campioni
volatile int readIndex = 0;
volatile int circularIndex;

#define N_TAPPI 16
//Gaussian 64
//int16_t tappi[N_TAPPI] = {92, 112, 135, 163, 194, 230, 271, 317, 369, 426, 489, 559, 634, 714, 800, 890, 985, 1082, 1182, 1283, 1383, 1483, 1579, 1671, 1758, 1837, 1908, 1969, 2019, 2057, 2083, 2097, 2097, 2083, 2057, 2019, 1969, 1908, 1837, 1758, 1671, 1579, 1483, 1383, 1283, 1182, 1082, 985, 890, 800, 714, 634, 559, 489, 426, 369, 317, 271, 230, 194, 163, 135, 112, 92};
//Blackman 64
//int16_t tappi[N_TAPPI] = {0, 2, 9, 21, 37, 60, 89, 126, 171, 224, 287, 360, 443, 536, 639, 752, 873, 1002, 1137, 1277, 1418, 1560, 1700, 1835, 1963, 2081, 2187, 2280, 2356, 2414, 2454, 2474, 2474, 2454, 2414, 2356, 2280, 2187, 2081, 1963, 1835, 1700, 1560, 1418, 1277, 1137, 1002, 873, 752, 639, 536, 443, 360, 287, 224, 171, 126, 89, 60, 37, 21, 9, 2, 0};
int16_t tappi[N_TAPPI] ={386, 840, 1635, 2850, 4446, 6204, 7748, 8659, 8659, 7748, 6204, 4446, 2850, 1635, 840, 386};

//////
uint8_t volatile ramp_var_value_recived[6]={0,0,0,0,0,0};
uint16_t wire_current_value_recived = 0;
uint8_t com_recived_message_flag = 0;
uint8_t data_recived_valid = 0;
uint8_t com_recived_mode = 0;
uint8_t temp_mode_recived = 0;
uint8_t data_to_send[6];

uint8_t data_recived[6] = {0,0,0,0,0,0};

bool volatile request_calibration_flag = false;
bool volatile request_selfcheck_flag = false;
bool volatile request_commit = false;
bool volatile recived_Ramp_Command = false;
void setup() {
  //Setup RP2040
  Serial.begin(115200);
  delay(1000);
  Initial_Setup();

  spegniAlim();
}

void loop() {
  if(Serial1.available()>5){
    data_recived[0] = Serial1.read();
    // Serial.println(data_recived[0]);
    bool data_recived_valid = 0;
    uint8_t ID = (data_recived[0] & 0b11100000) >> 5;
    switch(ID){
      case 7://ERROR
        while(Serial1.peek()) Serial1.read();
        SPL("Error");
        com_count_errors++;
        switch(com_last_ID_sent){
          case 0: //NULL
            break;
          case 161:
          case 162:
          case 163:
          case 164:
          case 165:
            com_send_data_subID(5, com_last_ID_sent & 0x1F);
            break;

          case 10:
            com_send_min_max(AbsoluteMinCurrent, AbsoluteMaxCurrent);
            break;
          default: break;
        }
        if(com_count_errors > 3){
          com_count_errors = 0;
          SPL("Throw an Error for Serial1 Comunication Interrupted");
          //Si potrebb aggiungere un led di indicazione??
        }
        break;

      case 1:
        Serial1.readBytes(data_recived + 1, 5);
        if(data_recived[5] == CRC8(data_recived, 5)) data_recived_valid = 1;
        break;
      
      case 2:
      case 3:
        Serial1.readBytes(data_recived + 1, 5);
        if(data_recived[1] == CRC8(data_recived, 1)) data_recived_valid = 1;
        break;
      default: 
        while(Serial1.peek()) Serial1.read();
        break;
    }

    if(data_recived_valid){
      uint64_t recived = 0;
      for (int i = 0; i < 6; i++) {
        recived |= ((uint64_t) data_recived[i] & 0xFF) << (5-i)*8;
      }
      
      switch(ID){
        case 1: //RIcevuto ID = 1 Acquisisco i Parametri di RAMP
          recived_Ramp_Command = true;
          
          ramp_var_value_recived[4] = (recived >> 37) & 0xFF;
          ramp_var_value_recived[3] = (recived >> 30) & 0x7F;
          ramp_var_value_recived[2] = (recived >> 24) & 0x3F;
          ramp_var_value_recived[1] = (recived >> 18) & 0x3F;
          ramp_var_value_recived[0] = (recived >> 14) & 0x0F;
          ramp_var_value_recived[5] = (recived >> 12) & 0x03;
          break;

        case 2: //RIcevuto ID = 2 Acquisisco i Parametri di WIRE
          wire_current_value_recived = ((recived >> 40) & 0x1F)*100;
          if(wire_current_value_recived <= AbsoluteMaxCurrent){
            accendiAlim();
            setWireCurrent(wire_current_value_recived);
          }else{
            dac.setValue(0);
            spegniAlim();
          }
          break;

        case 3: //RIcevuto ID = 3 Acquisisco il Comando Ricevuto all'interno di com_recived_mode
          com_recived_mode = data_recived[0] & 0b00011111;
          switch(com_recived_mode){
            case 1://Comando: STOP
              dac.setValue(0);
              spegniAlim();
              break;

            case 2:{//Comando GRAFICO
              //Devo inviare 360 punti a 32 bit
              //modalità di Invio dati al display OFF e spengo DC-DC
              TimerSendDataToDisplay_enable = false;
              dac.setValue(0);
              spegniAlim();
              //Invio il primo byte come comando e gli altri in una botta sola
              
              //for(int i = 0; i<counter_campionati; i++)Serial.println(valori_campionati[i]);
              
              resize_and_serialize(valori_campionati,counter_campionati,valori_grafico+1);
              for(int j = 1441,i = 0; i < 20;i++,j=j+4){
                valori_grafico[j]   = (valoriMedi[i]>>24)&0xFF;
                valori_grafico[j+1] = (valoriMedi[i]>>16)&0xFF;
                valori_grafico[j+2] = (valoriMedi[i]>>8)&0xFF;
                valori_grafico[j+3] = (valoriMedi[i])&0xFF;

              }
              /*
              Serial.println("Start");
              for(int j = 1441;j < 1521;j++){
                Serial.println(valori_grafico[j]);
              }
                    */ 
              //for(int i = 0; i<360; i++)Serial.println((valori_grafico[i*4+1]<<24) |(valori_grafico[i*4+2]<<16)|(valori_grafico[i*4+3]<<8) | valori_grafico[i*4+4]);
              valori_grafico[0] = (0x06 << 5) | 0x02;
              valori_grafico[1521] = CRC8(valori_grafico, 1521);
              Serial1.write(valori_grafico, 1522);
              //for(int i = 0; i<1462; i++)Serial.print(valori_grafico[i],HEX);
              break;
            }
            case 3:
            case 4: //Modalità di Invio dati al display Normal / Handle
              temp_mode_recived = com_recived_mode;
              TimerSendDataToDisplay_enable = true;
              break;

            case 5: //modalità di Invio dati al display OFF e spengo DC-DC
              TimerSendDataToDisplay_enable = false;
              dac.setValue(0);
              spegniAlim();
              break;

            case 6:
              request_calibration_flag = true;
              break;

            case 7:
              request_selfcheck_flag = true;
              break;

            default: break;
          }   
          break;

        default: break;
      }
      SP(ID); SP("  ");
      SPL(com_recived_mode);
    }
    else{// Valore ricevuto Invalido
      for(int i = 0; i < 6; i++) data_to_send[i] = 0xFF;
      Serial1.write(data_to_send, 6);
      SPL("Error Serial1 Data recived Corrupted");
    }
  }

  if(ADC0_current_flag){ //Ricezione Flag di Campionamento dell'ADC0, leggo corrente e tensione
    ADC0_current_flag = false;
    
    //()* 0.0078125 * 0.3245033 * 533.3333 * 0.1=  (value + offset) * adc_conv_to_mV *differential_gain* coefficent_Shunt_conversion * 0.1;
    readed_A_ADC = adc0.getLastConversionResults();
    media1 += (readed_A_ADC- current_Offset) * 554; //= 0.1352097*2^12

    //() * 0.805664062 * 10 * 0.1 =   (value + offset) * adc_conv_to_mV *partitor_gain * 0.1;
    readed_V_ADC = analogRead(PIN_VReading);
    media2 += (readed_V_ADC - voltage_Offset) * 3300;// = 0.8056641*2^12

    if(counterMedie++ > 2){ // Effettuo una media di 4 valori per avere un dato più stabile ma con una frequenza di aggiornamento pari a 16SPS
      real_A_out = media1 >> 14;
      real_V_out = media2 >> 14;
      if(real_A_out < 0) real_A_out = 0;
      if(real_V_out < 0) real_V_out = 0;
      media1 = 0;
      media2 = 0;
     /*
      SP("V: ");
      SP(real_V_out);
      SP("  A: ");
      SPL(real_A_out);
      */
      counterMedie = 0;
    }
    
  }

  if(ADC1_temp_flag){ //Aggiorno i valori di Temperatura e le Ventole
    ADC1_temp_flag = false;
  }

  if(TimerCheckCurrentOnWire_flag){
    TimerCheckCurrentOnWire_flag = false;
  }

  if(TimerReadAdcManico_flag && TimerReadAdcManico_enable){
    TimerReadAdcManico_flag = false;
    int32_t temp = readHandleNumber(ramp_var_value_recived[5]);
    if(abs(temp) < 100000){
      if(TimerReadAdcManico_enable_mean){
        count_handle++;
        mean_handle += temp;
      }
    
    
    sampleBuffer[readIndex] = temp;  
    
    readIndex = (readIndex + 1) % N_TAPPI;  // Aggiorna readIndex in modo circolare
    
    int64_t tempResult = 0;

    for (int i = 0; i < N_TAPPI; i++) {
        circularIndex = (readIndex - i + N_TAPPI-1) % N_TAPPI;
        tempResult += (int64_t) sampleBuffer[circularIndex] * tappi[i];
      }
    valori_campionati[counter_campionati++] = tempResult >> 16;
    Serial.println(tempResult >> 16);
    }
  }

  if(TimerSendDataToDisplay_flag && TimerSendDataToDisplay_enable){
    TimerSendDataToDisplay_flag = false;
    uint16_t caseVoltage_toSend = real_V_out;
    uint16_t caseCurrent_toSend = real_A_out;
    uint16_t handleVoltage_toSend = real_V_out;
    switch(temp_mode_recived){
      case 3: //Case / Wire
        data_to_send[0] = ( 4 << 5) | 1;
        data_to_send[1] = (caseVoltage_toSend >> 8) & 0xFF;
        data_to_send[2] = caseVoltage_toSend & 0xFF;
        data_to_send[3] = (caseCurrent_toSend >> 8) & 0xFF;
        data_to_send[4] = caseCurrent_toSend & 0xFF;
        Serial1.write(data_to_send, 6);
        
        break;
      case 4: //Handle
        data_to_send[0] = (4 << 5) | 2;
        data_to_send[1] = (handleVoltage_toSend >> 8) & 0xFF;
        data_to_send[2] = handleVoltage_toSend & 0xFF;
        data_to_send[3] = (caseCurrent_toSend >> 8) & 0xFF;
        data_to_send[4] = caseCurrent_toSend & 0xFF;
        Serial1.write(data_to_send, 6);
        break;
      default: break;
    }
  }

}

void setup1(){
}
uint16_t ST_Max_Current = AbsoluteMaxCurrent;
void loop1(){
  if(request_calibration_flag){
    request_calibration_flag = false;
    if(newCalibration()){
      //Error
      com_calibration_bad();
    }else{
      //Not Error
      com_calibration_ok();
      com_send_min_max(AbsoluteMinCurrent, AbsoluteMaxCurrent);
      request_commit = true;
    }
  }

  if(request_selfcheck_flag){
    request_selfcheck_flag = false;
    if(SelfTestCurrent(ST_Max_Current,20,request_commit)){
      //Error
      com_selfcheck_bad();
    }else{
      //Not Error
      com_selfcheck_ok();
    }
    request_commit = false;
  }

  if(recived_Ramp_Command){
    recived_Ramp_Command = false;
    
    rampFunction(ramp_var_value_recived[4]*10,ramp_var_value_recived[3]*10,ramp_var_value_recived[2]*100,ramp_var_value_recived[1]*100,ramp_var_value_recived[0]);
    com_alim_stopped();
  }

}

