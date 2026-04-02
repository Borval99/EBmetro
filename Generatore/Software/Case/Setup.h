//Define per debug
#define SP(x) Serial.print(x)
#define SPL(x) Serial.println(x)

//PIN DECLARATION
#define HX711_0_External_CLK 0
#define HX711_1_External_CLK 11
#define HX711_External_DT 1

#define I2C_Internal_SDA 2
#define I2C_Internal_SCL 3

#define Serial_Display_TX 12
#define Serial_Display_RX 13
#define Serial_Display_Speed 19200

#define PIN_Alert_ADC0 14
#define PIN_Alert_ADC1 15

#define PIN_POWER_ON_DCDC 8

#define PIN_VREF_2V5 26
#define PIN_VREF_5V 27
#define PIN_VReading 28

#define PIN_Vent_IN   4
#define PIN_Vent_OUT  5
#define PIN_Vent_DISS 6
#define PIN_Vent_ALIM 7
//Internal I2C 400kHz
#define ADS1115_ADDRESS_0 (0x48)
#define ADS1115_ADDRESS_1 (0x49)
#define MCP4725_ADDRESS (0x60)

#define EEPROM_ADDRESS_CURRENT_MAX 10 //2 Bytes
#define EEPROM_ADDRESS_CURRENT_MIN 12 //2 Bytes


Adafruit_HX711 adc0_manico(HX711_External_DT, HX711_0_External_CLK);
Adafruit_HX711 adc1_manico(HX711_External_DT, HX711_1_External_CLK);

Adafruit_ADS1115 adc0;
Adafruit_ADS1115 adc1;

Adafruit_ADS1115 adc0_Ext;

MCP4725 dac(MCP4725_ADDRESS, &Wire1);

RPI_PICO_Timer TimerSendDataToDisplay(0);
RPI_PICO_Timer TimerCheckCurrentOnWire(1);
RPI_PICO_Timer TimerReadAdcManico(2);

//Valori per la Fake EEPROM
volatile uint16_t AbsoluteMaxCurrent = 100;
volatile uint16_t AbsoluteMinCurrent = 0; 

bool volatile ADC0_current_flag = false, ADC1_temp_flag = false;
// Funzione di callback per l'interrupt
void gpio_callback(uint gpio, uint32_t events) {
  switch(gpio){
    case PIN_Alert_ADC0:
      ADC0_current_flag = true;
      break;
    case PIN_Alert_ADC1:
      ADC1_temp_flag = true;
      break;
    default: break;
  }
}
bool volatile TimerSendDataToDisplay_enable = false, TimerReadAdcManico_enable = false;
bool volatile TimerSendDataToDisplay_flag = false, TimerCheckCurrentOnWire_flag = false, TimerReadAdcManico_flag = false;
bool OnTimerSendDataToDisplay (struct repeating_timer *t){
    TimerSendDataToDisplay_flag = true;
  return true;
}

bool OnTimerCheckCurrentOnWire (struct repeating_timer *t){
    TimerCheckCurrentOnWire_flag = true;
  return true;
}

bool OnTimerReadAdcManico(struct repeating_timer *t){
    TimerReadAdcManico_flag = true;
  return true;
}


void Initial_Setup(){
  SPL("Setup Started:");

  //Setup I2C Internal
  Wire1.setSDA(I2C_Internal_SDA);
  Wire1.setSCL(I2C_Internal_SCL);
  Wire1.begin();
  Wire1.setClock(400000);

  //Setup for "Fake" EEPROM
  EEPROM.begin(256);
  uint16_t temp_max, temp_min;
  EEPROM.get(EEPROM_ADDRESS_CURRENT_MAX, temp_max);
  EEPROM.get(EEPROM_ADDRESS_CURRENT_MIN, temp_min); 
  AbsoluteMaxCurrent = temp_max;
  AbsoluteMinCurrent = temp_min;
  SPL("Succesfully Readed values from EEPROM");

  //Setup External Adc del manico

  adc0_manico.begin();
  adc0_manico.powerDown(true);
  adc1_manico.begin();
  adc1_manico.powerDown(true);

  SPL("Adc0 & Adc1 Manico successfully Initialized");

  //Setup ADS1115_0
  if (!adc0.begin(ADS1115_ADDRESS_0, &Wire1)) {
    SPL("Failed to initialize ADS1115_0 on Board.");
    while (1);
  }
  adc0.setGain(GAIN_SIXTEEN);      //  16x Gain +/- 0.256V
  adc0.setDataRate(RATE_ADS1115_64SPS); //  64 samples per second
  pinMode(PIN_Alert_ADC0,INPUT);
  gpio_set_irq_enabled(PIN_Alert_ADC0, GPIO_IRQ_EDGE_FALL, 1);
  gpio_set_irq_enabled_with_callback(PIN_Alert_ADC0, GPIO_IRQ_EDGE_FALL, 1, &gpio_callback);
  adc0.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_2_3, true); // , continuous=true
  SPL("ADS1115_0 on Board successfully Initialized");


  //Setup ADS1115_1
  if (!adc1.begin(ADS1115_ADDRESS_1, &Wire1)) {
    SPL("Failed to initialize ADS1115_1 on Board.");
    while (1);
  }
  adc1.setGain(GAIN_FOUR);      //  4x Gain +/- 1.024V
  adc1.setDataRate(RATE_ADS1115_8SPS); //  8 samples per second
  pinMode(PIN_Alert_ADC1,INPUT);
  gpio_set_irq_enabled(PIN_Alert_ADC1, GPIO_IRQ_EDGE_FALL, 1);
  gpio_set_irq_enabled_with_callback(PIN_Alert_ADC1, GPIO_IRQ_EDGE_FALL, 1, &gpio_callback);
  adc1.startADCReading(MUX_BY_CHANNEL[0],false);
  SPL("ADS1115_1 on Board successfully Initialized");


  //Setup MCO4725
  if (!dac.begin()) {
    SPL("Failed to initialize MCP4725 on Board.");
    while (1);
  }
  dac.setValue(0);
  SPL("MCP4725 on Board successfully Initialized");

  //Setup Serial1 to Display
  if(com_init_serialcom(Serial_Display_TX,Serial_Display_RX,Serial_Display_Speed)){
    SPL("Serial1 to Dsiaplay successfully Initialized");
  }

  //Setup Timer Send to Display
  if (!TimerSendDataToDisplay.attachInterrupt(8 /*Hz*/, OnTimerSendDataToDisplay)) {
    SPL("Failed to initialize Timer OnTimerSendDataToDisplay");
    while (1); 
  }
  SPL("Timer SendDataToDisplay successfully Initialized");

  //Setup Timer Check Current On Wire     //DA FARE
  if (!TimerCheckCurrentOnWire.attachInterrupt(1 /*Hz*/, OnTimerCheckCurrentOnWire)) {
    SPL("Failed to initialize Timer SendDataToDisplay");
    while (1); 
  }
  SPL("Timer CheckCurrentOnWire successfully Initialized");

  //Setup Timer Read ADC Manico
  if (!TimerReadAdcManico.attachInterrupt(80 /*Hz*/, OnTimerReadAdcManico)) {
    SPL("Failed to initialize Timer ReadAdcManico");
    while (1); 
  }
  SPL("Timer ReadAdcManico successfully Initialized");

  //Setup Vref 2.5V
  pinMode(PIN_VREF_2V5, INPUT);
  analogReadResolution(12);
  if(analogRead(PIN_VREF_2V5)>3000 && analogRead(PIN_VREF_2V5)<3200 )
    SPL("VREF 2.5V successfully Initialized");
  else
    SPL("VREF 2.5V too Low or too High!! ERROR!");


  //Setup Vref 5V
  pinMode(PIN_VREF_5V, INPUT);
  analogReadResolution(12);
  if(analogRead(PIN_VREF_5V)>3000 && analogRead(PIN_VREF_5V)<3200 )
    SPL("VREF 5V successfully Initialized");
  else
    SPL("VREF 5V too Low or too High!! ERROR!");
  

  //Setup Voltage Input
  pinMode(PIN_VReading, INPUT);

  //Setup Ventole
  pinMode(PIN_Vent_IN, OUTPUT);
  pinMode(PIN_Vent_OUT, OUTPUT);
  pinMode(PIN_Vent_DISS, OUTPUT);
  pinMode(PIN_Vent_ALIM, OUTPUT);
  digitalWrite(PIN_Vent_IN, 1);
  digitalWrite(PIN_Vent_OUT, 1);
  digitalWrite(PIN_Vent_DISS, 1);
  digitalWrite(PIN_Vent_ALIM, 1);

  //Setup DCDC Power On
  pinMode(PIN_POWER_ON_DCDC, OUTPUT);
  digitalWrite(PIN_POWER_ON_DCDC,0);


  SPL("Setup Finished");
}