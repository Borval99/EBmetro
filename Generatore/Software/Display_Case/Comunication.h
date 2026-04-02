/*
TX - SCRITTURA su Serial1 
Realizzazione a Richieste. ogni qual'volta il display avvia un qualche tipo di Esecuzione invia tutti i dati necessari.
gestisce sempre il display le richieste di ricezione dei dati in modo toggle per i valori letti a schermo.
Connessione TX RX con CRC

ID: 0 Riservato per settings
####### RAMPA ########
Dati da inviare:
Start Ramp:                 ID :1
  -ID               3 bit
  -Corrente Max     8 bit           4
  -Corrente Min     7 bit           3
  -Tempo rise/fall  6 bit           2
  -Tempo hold       6 bit           1
  -Cicli            4 bit           0
  -Handle           2 bit
  -Impl Future      4 bit

####### WIRE ########
Start Wire:                 ID:2
  -ID               3 bit
  -Corrente         5 bit

####### ALTRO ########      ID: 3 - 0 (Not Used)
-Stop Generale              ID: 3 - 1
-Send Last Grafico          ID: 3 - 2


TOGGLE:
Modalità Case/Wire Handle Off ID 3 - 3/4/5

RECEZIONE 
*/

#define CRC8_POLYNOMIAL 0x07

uint8_t com_last_ID_sent = 0;
uint8_t com_count_errors = 0;

uint8_t CRC8(uint8_t *data, uint16_t length) {
  uint8_t crc = 0x00; // Inizializza il CRC a 0

  for (size_t i = 0; i < length; i++) {
    crc ^= data[i]; // XOR con il byte di input

    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x80) { // Se il bit più alto è 1
        crc = (crc << 1) ^ CRC8_POLYNOMIAL; // Shift a sinistra e XOR con il polinomio
      } else {
        crc <<= 1; // Shift a sinistra
      }
    }
  }
  return crc; // Restituisce il valore del CRC8
}

bool com_init_serialcom(unsigned long baudrate) {
  Serial1.setFIFOSize(2048);
  Serial1.setTX(0);
  Serial1.setRX(1);
  Serial1.begin(baudrate);

  //PER IL MOMENTO DA SEMPRE TRUE. MA BISOGNA IMPLEMENTARE UN CHECK
  return true;
}

void com_send_data(uint8_t *data, uint8_t length ){
  uint8_t CRC_val = CRC8(data, length);
  Serial1.write(data, length);
  Serial1.write(CRC_val);

}
void com_send_data(uint8_t data){
  uint8_t data_temp[1] = {data};
  uint8_t CRC_val = CRC8(data_temp, 1);
  Serial1.write(data_temp, 1);
  Serial1.write(CRC_val);
  Serial1.write(0xFF);
  Serial1.write(0xFF);
  Serial1.write(0xFF);
  Serial1.write(0xFF);
}
void com_send_data_subID(uint8_t ID_to_send, uint8_t subID_to_send){
  if(com_count_errors < 4){
    com_last_ID_sent = (ID_to_send << 5)  | ((subID_to_send) & 0b00011111);
    com_send_data(com_last_ID_sent);
  }
}

//ID 1
void com_start_ramp() {
  if(com_count_errors < 4){
  uint8_t data_to_send[5];

  uint64_t result = 0;
  result |= ((uint64_t) 1 & 0x07) << 37;
  result |= ((uint64_t) ramp_var_value[8] & 0xFF) << 29;
  result |= ((uint64_t) ramp_var_value[7] & 0x7F) << 22;
  result |= ((uint64_t) ramp_var_value[6] & 0x3F) << 16;
  result |= ((uint64_t) ramp_var_value[5] & 0x3F) << 10;
  result |= ((uint64_t) ramp_var_value[2] & 0x0F) << 6;
  result |= ((uint64_t) selected_handle & 0x03)<<4;
  result |= ((uint64_t) 0 & 0x0F);
  for (int i = 0; i < 5; i++) {
    data_to_send[i] = (result >> (32 - i * 8)) & 0xFF;
  }
  
    com_last_ID_sent = 1;
    com_send_data(data_to_send, 5);
  }
}

//ID 2
void com_start_wire(){
  if(com_count_errors < 4){
    uint8_t result = 0;
    result |= ((uint8_t) 2 & 0x07) << 5;
    result |= ((uint8_t) wire_current_value & 0x1F);
    com_last_ID_sent = 2;
    com_send_data(result);
  }
}

//ID 3 - 1
void com_stop_all(){
  com_send_data_subID(3,1);
}


//ID 3 - 2
void com_send_lastgraph(){
  com_send_data_subID(3,2);
}

//ID 3 - 3/4/5
void com_case_send_on(){
  com_send_data_subID(3,3);
}

void com_case_send_off(){
  com_send_data_subID(3,5);
}

void com_send_request_calibration(){
    com_send_data_subID(3,6);
}

void com_send_request_selfcheck(){
    com_send_data_subID(3,7);
}

/* RX - LETTURA
I dati da leggere sono:
CASE/WIRE:
  - Tensione 0/20 V errore 10mV 2000 punti -> Almeno 11 bit
  - Corrente 0/30 A errore 10mA            -> Almeno 12 bit
  FORMAT:
      ID - 3 Bit - 5bit di controllo
    - V 12 Bit
    - A 12 Bit
Handle
  - Tensione -999/999 uV errore 0.1uV      -> Almeno 15 bit
  - Corrente 0/30 A errore 10mA            -> Almeno 12 bit
  FORMAT:
    ID - 3 Bit - 5bit di controllo
    - V 16 Bit
    - A 16 Bit
Grafico:
  -Dati a parte probabilemte qualche kb di roba
*/

