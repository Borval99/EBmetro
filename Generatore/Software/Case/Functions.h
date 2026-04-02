volatile int16_t real_A_out = 0;
int16_t real_V_out = 0;
int16_t readed_V_ADC = 0;
int16_t readed_A_ADC = 0;
int16_t current_Offset = 67;
int16_t voltage_Offset = 24;
int16_t counterMedie = 0;
int32_t media1 = 0;
int32_t media2 = 0;


volatile int64_t mean_handle = 0;
volatile uint32_t count_handle = 0;
volatile bool TimerReadAdcManico_enable_mean = false;

uint8_t valori_grafico[1522] = {0};
int32_t valori_campionati[1800] = {0};
volatile uint32_t counter_campionati = 0;

volatile int32_t valoriMedi[20] = {0};

int counter_temp = 0;

bool _AlimStatus = false;
//   ###############  Funzione per l'Accensione del DCDC  ###############
void accendiAlim(){
  _AlimStatus = true;
  digitalWrite(PIN_POWER_ON_DCDC,1);
}


//   ###############  Funzione per lo spegnimento del DCDC  ###############
void spegniAlim(){
   _AlimStatus = false;
  digitalWrite(PIN_POWER_ON_DCDC,0);
}

//   ###############  Funzione per capire se Acceso o spento il DCDC  ###############
bool getDCDCStatus(){
   return _AlimStatus;
}

//   ###############  Imposta una corrente DC sul Wire  ###############
void setWireCurrent(uint16_t desideredCurrent){
  if(desideredCurrent < AbsoluteMinCurrent){
    dac.setValue(0);
  }else{
  dac.setValue(((desideredCurrent - AbsoluteMinCurrent)*4095)/(AbsoluteMaxCurrent - AbsoluteMinCurrent));
  }
}


int newCalibration(){ //Ricevuta dal display, risponde quando ha finito, eseguito sul secondo core
  dac.setValue(0);
  accendiAlim();
  delay(1000);
  
  if(real_A_out > 20 && real_A_out < 50){
    AbsoluteMinCurrent = real_A_out;
    uint16_t temp_min = AbsoluteMinCurrent;
    EEPROM.put(EEPROM_ADDRESS_CURRENT_MIN, temp_min);
  }else{
    spegniAlim();
    return 1; //Low Current not LOW or TOO LOW
  } 

  delay(200);
  for(int i = 0; i < 8; i++){
    dac.setValue(585*i);
    delay(200);
  }
   delay(1000);
  if(real_A_out > 100 && real_A_out < 3200){
    AbsoluteMaxCurrent = real_A_out; 
    uint16_t temp_max = AbsoluteMaxCurrent;
    EEPROM.put(EEPROM_ADDRESS_CURRENT_MAX, temp_max);
  }else{
    spegniAlim();
    return 2; // Current too LOW or TOO HIGH
  } 

  delay(200);
  dac.setValue(0);
  spegniAlim();
  return 0;
}



//Effettua il Test per diversi valori di corrente e ne esegue un controllo dell'errore.
//Restituisce 1 se in Errore, Restituisce 0 se tutto è avvenuto correttamente
bool SelfTestCurrent(uint16_t correnteMax, uint16_t acceptedError,bool commit = 0){
  accendiAlim();
  delay(300);
  //Eegue il test per i valori correnteMax/2, /4, /8, /16
  for(int i = 0; i < 5; i++){
    if(correnteMax >> (5-i) > correnteMax){
      setWireCurrent(correnteMax >> (5-i));
      delay(300);
      if(abs((real_A_out - (int16_t)(correnteMax >> (5-i)))) > acceptedError){
        spegniAlim();
        return 1;
      }
      delay(300);
    }
  }
   //Eegue il test per i valori correnteMax/2 + correnteMax/2, /4, /8, /16
  for(int i = 0; i < 5; i++){
    setWireCurrent((correnteMax >> 1) + (correnteMax >> (5-i)));
    delay(300);
    if(abs(((int16_t)(correnteMax >> 1) + (correnteMax >> (5-i))) - real_A_out) > acceptedError){
      spegniAlim();
      return 1;
    }
    delay(300);
  }
  spegniAlim();
  if(commit) EEPROM.commit();
  return 0;
}



bool rampFunction(uint16_t max_var, uint16_t min_var, uint16_t hTime_var, uint16_t fdTime_var, uint16_t cycles_var){
  uint16_t divider = fdTime_var/10;
  uint16_t time_scale = (fdTime_var)/divider;
  
  setWireCurrent(min_var);
  accendiAlim();
  
  for(int i = 0; i< 20;i++) valoriMedi[i] = 0;

  //Attesa Iniziale di 3 Secondi
  unsigned long initial_time = millis();
  while(millis() - initial_time < 1500){
    if(!getDCDCStatus()) return false;
  }
  //Avvio dopo 1.5 Secondi la lettura dell adc
  counter_campionati = 0;
  TimerReadAdcManico_enable = true;
  initial_time = millis();
  while(millis() - initial_time < 1500){
    if(!getDCDCStatus()) return false;
  }
  //Effettuo N Cicli
  for(int j = 0;j < cycles_var; j++){
    //Inizio Ciclo
    //Divido il tempo in 3 e faccio la media solo del tempo in mezzo
    initial_time = millis();
    while(millis() - initial_time <  hTime_var/3){
      if(!getDCDCStatus()) return false;
    }
    TimerReadAdcManico_enable_mean = true;
    initial_time = millis();
     while(millis() - initial_time <  hTime_var/3){
      if(!getDCDCStatus()) return false;
    }
    TimerReadAdcManico_enable_mean = false;
    valoriMedi[j*4] = mean_handle/count_handle;
    mean_handle = 0;
    count_handle = 0;
    initial_time = millis();
     while(millis() - initial_time <  hTime_var/3){
      if(!getDCDCStatus()) return false;
    }

    //Effettuo la rampa di salita e mi salvo solo i valori in mezzo
    for(int i = 0; i < divider+1; i++){
      initial_time = millis();
      while(millis() - initial_time <  time_scale){
        if(!getDCDCStatus()) return false;
      }
      if (i == (divider+1)/4){
        TimerReadAdcManico_enable_mean = true;
      }
      if (i == (divider+1)*3/4){
        TimerReadAdcManico_enable_mean = false;
        valoriMedi[j*4+1] = mean_handle/count_handle;
        mean_handle = 0;
        count_handle = 0;
      }
      setWireCurrent(min_var + i*(max_var-min_var)/divider);
    }

    //Divido il tempo in 3 e faccio la media solo del tempo in mezzo
    initial_time = millis();
    while(millis() - initial_time <  hTime_var/3){
      if(!getDCDCStatus()) return false;
    }
    TimerReadAdcManico_enable_mean = true;
    initial_time = millis();
     while(millis() - initial_time <  hTime_var/3){
      if(!getDCDCStatus()) return false;
    }
    TimerReadAdcManico_enable_mean = false;
    valoriMedi[j*4+2] = mean_handle/count_handle;
    mean_handle = 0;
    count_handle = 0;
    initial_time = millis();
     while(millis() - initial_time <  hTime_var/3){
      if(!getDCDCStatus()) return false;
    }
   
    //Effettuo la rampa di discesa e mi salvo solo i valori in mezzo
    for(int i = 0; i < divider+1; i++){
      initial_time = millis();
      while(millis() - initial_time < time_scale){
        if(!getDCDCStatus()) return false;
      }
      if (i == (divider+1)/4){
        TimerReadAdcManico_enable_mean = true;
      }
      if (i == (divider+1)*3/4){
        TimerReadAdcManico_enable_mean = false;
        valoriMedi[j*4+3] = mean_handle/count_handle;
        mean_handle = 0;
        count_handle = 0;
      }
      setWireCurrent(max_var-i*(max_var-min_var)/divider);
    }
    
  }

  //Attesa Iniziale di 3 Secondi
  initial_time = millis();
  while(millis() - initial_time < 2000){
    if(!getDCDCStatus()) return false;
  }
  //Fermo la lettura del timer dopo 2 secondi per dare il tempo al fir di fare le operazioni
  TimerReadAdcManico_enable = false;
  initial_time = millis();
  while(millis() - initial_time < 1000){
    if(!getDCDCStatus()) return false;
  }
  spegniAlim();

  for(int i = 0; i < cycles_var*4; i++) Serial.println(valoriMedi[i]);

  return true;
}
uint16_t lastAdcHandleReaded = -1;
int32_t readHandleNumber(int number){
  if(lastAdcHandleReaded != number){
    switch(number){
    case 3:
      adc0_manico.powerDown(false);
      adc1_manico.powerDown(true);
      break;
    case 0:
      adc0_manico.powerDown(true);
      adc1_manico.powerDown(false);
      break;
    case 2:
      adc0_manico.powerDown(false);
      adc1_manico.powerDown(true);
      break;
    case 1:
      adc0_manico.powerDown(true);
      adc1_manico.powerDown(false);
      break;
    default:
      adc0_manico.powerDown(true);
      adc1_manico.powerDown(true);
      break;
    lastAdcHandleReaded = number;
    }
  }
  switch(number){
    case 3: //Grande
       return adc0_manico.readChannelRaw(CHAN_A_GAIN_64)>>1;
      break;
    case 0: //Piccola x1
      return adc1_manico.readChannelRaw(CHAN_A_GAIN_64)>>1;
      break;
    case 2: //Media
      return adc0_manico.readChannelRaw(CHAN_B_GAIN_32);
      break;
    case 1: //Piccola x5
      return adc1_manico.readChannelRaw(CHAN_B_GAIN_32);
      break;
    default:
    return 0;
     break;
  }

}

#define TARGET_SIZE 360
void resize_and_serialize(int32_t *input, int N, uint8_t *output) {
    int32_t resized_array[TARGET_SIZE];

    // Step 1: Ridimensiona l'array a 360 elementi
    if (N < TARGET_SIZE) {
        int repeat_factor = TARGET_SIZE / N;
        int remainder = TARGET_SIZE % N;
        int k = 0;

        // Riempimento ripetendo i dati
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < repeat_factor; j++) {
                resized_array[k++] = input[i];
            }
        }
        // Riempimento con zeri per il resto
        for (int i = 0; i < remainder; i++) {
            resized_array[k++] = input[i];
        }
    } else if (N > TARGET_SIZE) {
        float factor = (float)N / TARGET_SIZE;
        for (int i = 0; i < TARGET_SIZE; i++) {
            int start = (int)(i * factor);
            int end = (int)((i + 1) * factor);

            int64_t sum = 0;
            for (int j = start; j < end; j++) {
                sum += input[j];
            }
            resized_array[i] = (int32_t)(sum / (end - start));
        }
    } else {
        for (int i = 0; i < TARGET_SIZE; i++) {
            resized_array[i] = input[i];
        }
    }

    // Step 2: Converti l'array ridimensionato in uint8_t (serializzazione)
    for (int i = 0; i < TARGET_SIZE; i++) {
        int32_t value = resized_array[i];

        // Suddividi in 4 byte (da 32 bit a 4 x 8 bit)
        output[i * 4]     = (value >> 24) & 0xFF; // Byte più significativo
        output[i * 4 + 1] = (value >> 16) & 0xFF;
        output[i * 4 + 2] = (value >> 8) & 0xFF;
        output[i * 4 + 3] = value & 0xFF;         // Byte meno significativo
    }
}
