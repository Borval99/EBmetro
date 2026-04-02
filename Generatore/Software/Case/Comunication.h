#define CRC8_POLYNOMIAL 0x07

uint8_t com_count_errors = 0;
uint8_t com_last_ID_sent = 0;


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

bool com_init_serialcom(uint8_t TX, uint8_t RX, unsigned long baudrate) {
  Serial1.setTX(TX);
  Serial1.setRX(RX);
  Serial1.begin(baudrate);

  //PER IL MOMENTO DA SEMPRE TRUE. MA BISOGNA IMPLEMENTARE UN CHECK
  return true;
}

void com_send_data(uint8_t *data, uint16_t length ){
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

// 5 - 1
void com_alim_stopped(){
  com_send_data_subID(5,1);
}

// 5 - 2
void com_calibration_bad(){
  com_send_data_subID(5,2);
}

// 5 - 3
void com_calibration_ok(){
  com_send_data_subID(5,3);
}

// 5 - 4
void com_selfcheck_bad(){
  com_send_data_subID(5,4);
}

// 5 - 5
void com_selfcheck_ok(){
  com_send_data_subID(5,5);
}

// 6 - 1
void com_send_min_max(uint16_t min,uint16_t max){ //DA SISTEMARE
  if(com_count_errors < 4){
    uint8_t data_to_send[5];
    data_to_send[0] = (0x06 << 5) | 0x01;
    data_to_send[1] = (min >> 8) & 0xFF;
    data_to_send[2] =  min & 0xFF;
    data_to_send[3] = (max >> 8) & 0xFF;
    data_to_send[4] = max & 0xFF;
    com_last_ID_sent = 10;
    com_send_data(data_to_send, 5);
  }
}