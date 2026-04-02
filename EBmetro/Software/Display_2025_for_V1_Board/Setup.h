#include <LovyanGFX.hpp>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include <RPi_Pico_TimerInterrupt.h>

#define Udine false //Se è di udine mettere true

//I2C Setup:
#define SDA_PIN  0
#define SCL_PIN  1
#define I2C_SPEED 1000000
#define ADS1115_ADDRESS (0x48)



//Define per debug
#define SB(x) Serial.begin(x)
#define SP(x) Serial.print(x)
#define SPL(x) Serial.println(x)

//#define SB(x) x
//#define SP(x) x
//#define SPL(x) x


//Configurazione Motore
#define MOTOR_PIN 7  

//Configurazione Condensatore
#define CAPACITOR_PIN 29

//Configurazione 2.5V
#define VREF2V5_PIN 28

//Configurazione Encoder
#define ENC_CLK 27  
#define ENC_D 26   
#define ENC_SW 15 

// Configurazione Touch
#define T_IRQ 2  // IRQ
#define T_CS 5   // Chip Select per SPI
#define T_MOSI 3 // MOSI per SPI
#define T_MISO 4  // MISO per SPI
#define T_SCK 6  // SCK per SPI
#define TOUCH_SPI_SPEED 2000000 // Velocità SPI (2 MHz)

//Configurazione ADC1115
#define SAMPLE_PIN 9     //Pin retrolluminazione

//Configurazione ZERO
volatile float ZERO_E = 0.0;
volatile float ZERO_B = 0.0;

//Configurazione Unità di misura
bool UnitaMisura_State = 0;  // 0=mV, 1=V/m || 1=uT

//Configurazione Condensatore
volatile bool E_B_State =0;      // 0=Campo E, 1=Campo B

//Configurazione Hold
float valoriHold[3] = {0,0,0};

//Configurazione Motore On/Off
bool Motor_State = 0;

volatile bool E_Reading_Multiplier = false;

class LGFX : public lgfx::LGFX_Device{
  //Selezione del driver
  //lgfx::Panel_ILI9488     _panel_instance; NON SO PERCHè MA è PIU LENTA
  lgfx::Panel_ST7796      _panel_instance;

  lgfx::Bus_SPI        _bus_instance;
  public:
  LGFX(void){
    { 
      auto cfg = _bus_instance.config(); 
      cfg.spi_host = 1;                   //SPI type : SPI1
      cfg.spi_mode = 0;                   
      cfg.freq_write = 25000000;          
      cfg.freq_read  = 20000000;    
      cfg.pin_sclk = 10;
      cfg.pin_mosi = 11;
      cfg.pin_miso = 8;
      cfg.pin_dc   = 12;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    { 
      auto cfg = _panel_instance.config();

      cfg.pin_cs           =    14;  
      cfg.pin_rst          =    13; 
      cfg.pin_busy         =    -1;  

      cfg.panel_width      =   320;   
      cfg.panel_height     =   480;   
      cfg.offset_x         =     0;   
      cfg.offset_y         =     0;   
      cfg.offset_rotation  =     0;  
      cfg.dummy_read_pixel =     8;   
      cfg.dummy_read_bits  =     1;   
      cfg.readable         =  true;    
      cfg.invert           = false;  
      cfg.rgb_order        = false;  
      cfg.dlen_16bit       = false;   
      cfg.bus_shared       = false;

      _panel_instance.config(cfg);
    }


    setPanel(&_panel_instance); 
  }
};

LGFX lcd;
Adafruit_ADS1115 adc;


//FONTS
const lgfx::IFont* fontList [] =
{ 
    &DejaVu12,          //0
    &DejaVu18,          //1
    &DejaVu24,          //2
    &DejaVu40,          //3
    &DejaVu72,          //4
    &Orbitron_Light_24, //5
    &AsciiFont24x48,    //6
    &Font7              //7
};

RPI_PICO_Timer TimerUpdateDisplay(0);
volatile bool TimerUpdateDisplay_Flag = false;


//Funzoine per l'encoder counter Update 
volatile bool currClk, currDt;
volatile int Encoder_pos_menu = 3, Encoder_pos_value = 0;
int Modify_mode = 0;
void updateEncoderCall() {
  currClk = digitalRead(ENC_CLK);
  currDt = digitalRead(ENC_D);
  if (currDt ^ currClk)
    if(!Modify_mode) Encoder_pos_menu--;
    else Encoder_pos_value++;
  else 
    if(!Modify_mode) Encoder_pos_menu++;
    else Encoder_pos_value--;
}
uint32_t lastDebounce = 0;
bool volatile IRQ_flag = false, SW_flag , Encoder_flag= false, Sample_flag = false;
// Funzione di callback per l'interrupt
void gpio_callback(uint gpio, uint32_t events) {
  switch(gpio){
    case T_IRQ:
      IRQ_flag = true;  // Imposta il flag per indicare che è stato rilevato un tocco
      break;
    case ENC_CLK:
      updateEncoderCall();
      Encoder_flag = true;  // Imposta il flag per indicare che è stato rilevato un tocco
      break;
    case ENC_SW:
      lastDebounce = millis();
      SW_flag = true;  // Imposta il flag per indicare che è stato rilevato un tocco
      break;
    case SAMPLE_PIN:
      Sample_flag = true;  // Imposta il flag per indicare che è avvenuto un sampling
      break;
    default: break;
  }
}

bool OnTimerUpdateDisplay(struct repeating_timer *t){
  TimerUpdateDisplay_Flag = true;
  return true;
}

    //###########################  SETUP  ########################
void Initial_Setup(){
  //Setup RP2040
  SPL("Setup Started:");

  //Setup I2C
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();
  Wire.setClock(I2C_SPEED);
  SPL("I2C successfully Initialized");

  //Setup ADS1115
  if (!adc.begin(ADS1115_ADDRESS)) {
    SPL("Failed to initialize ADS1115 on Board.");
    while (1);
  }
  adc.setGain(GAIN_FOUR);      //  16x Gain +/- 0.256V
  adc.setDataRate(RATE_ADS1115_32SPS); //  128 samples per second
  adc.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_0_1, true); // , continuous=true

  SPL("ADS1115 on Board successfully Initialized");

  //Setup Encoder
  pinMode(ENC_D, INPUT);
  pinMode(ENC_CLK, INPUT);
  pinMode(ENC_SW, INPUT);

  gpio_set_irq_enabled(ENC_CLK, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 1);
  gpio_set_irq_enabled(ENC_SW, GPIO_IRQ_EDGE_RISE, 1);
  gpio_set_irq_enabled_with_callback(ENC_CLK, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, 1, &gpio_callback);
  gpio_set_irq_enabled_with_callback(ENC_SW, GPIO_IRQ_EDGE_RISE, 1, &gpio_callback);
  
  SPL("Encoder successfully Initialized");

  //Setup display
  if (!lcd.begin()) {
    SPL("Failed to initialize Display Comunication");
    while (1);
  }
    int32_t padding = lcd.textWidth("0",fontList[7]); // get the width of the text in pixels
  lcd.setTextPadding(padding);
  lcd.fillScreen(TFT_BLACK);
  lcd.setRotation(1);
  

  SPL("Display successfully Initialized");

  //Setup Touch
  pinMode(T_CS, OUTPUT);
  pinMode(T_IRQ, INPUT);

  gpio_set_irq_enabled(T_IRQ, GPIO_IRQ_EDGE_RISE, 1);
  gpio_set_irq_enabled_with_callback(T_IRQ, GPIO_IRQ_EDGE_RISE, 1, &gpio_callback);

  digitalWrite(T_CS, HIGH);
  SPI.setTX(T_MOSI);
  SPI.setRX(T_MISO);
  SPI.setSCK(T_SCK);
  SPI.begin();
  SPL("Touch successfully Initialized");

  //Setup TimerUpdateDisplay
  if (!TimerUpdateDisplay.attachInterrupt(16 /*Hz*/, OnTimerUpdateDisplay)) {
    SPL("Failed to initialize Timer TimerUpdateDisplay");
    while (1); 
  }
  SPL("TimerUpdateDisplay successfully Initialized");
  pinMode(SAMPLE_PIN, INPUT);
  gpio_set_irq_enabled(SAMPLE_PIN, GPIO_IRQ_EDGE_FALL, 1);
  gpio_set_irq_enabled_with_callback(SAMPLE_PIN, GPIO_IRQ_EDGE_FALL, 1, &gpio_callback);
  SPL("IRQ successfully Initialized");

  //Setup motore
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN,0);
  SPL("Motor successfully Initialized");
  
  //Setup conensatore
  pinMode(CAPACITOR_PIN, OUTPUT);
  digitalWrite(CAPACITOR_PIN, Udine);
  SPL("Capacitor successfully Initialized");

  //Setup Vref 2.5V
  pinMode(VREF2V5_PIN, INPUT);
  analogReadResolution(12);
  if(analogRead(VREF2V5_PIN)>3000 && analogRead(VREF2V5_PIN)<3200 )
    SPL("VREF 2.5V successfully Initialized");
  else
    SPL("VREF 2.5V too Low or too High!! ERROR!");

  SPL("Setup Finished");
}
