int16_t x = 0, y = 0,z = 0; // Coordinate del touchscreen
//TouchScreen
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

//funzione per la Implementazione del Gain nell'adc e per il passaggio da E a B
void function_main_adc_mode(bool EB){//FORSE A UDINE HO GLI INPUT AL CONTRARIO
  if(EB){
    adc.setGain(GAIN_FOUR);//SIXTEEN old
  } else {
    adc.setGain(GAIN_FOUR);
  }
    //adc.setDataRate(RATE_ADS1115_64SPS);
    adc.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_0_1, true);
}

//Valori massimi dello zero che si possono impostare pari a +-250.0
int function_settings_zero_clipping(int value){
  if(value > 2500) value = 2500;
  if(value < -2500) value = -2500;
  return value;
}