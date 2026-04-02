
//I2C Setup:

//Define per debug
#define SP(x) Serial.print(x)
#define SPL(x) Serial.println(x)

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

//Configurazione lcd e SPI
#define LED_PIN 9     //Pin retrolluminazione
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
      cfg.freq_write = 20000000;          
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

const unsigned char menuButton[] = {
  0b00000000, 0b00000011, 0b11000000, 0b00000000, 
  0b00000000, 0b00000111, 0b11100000, 0b00000000, 
  0b00000000, 0b00001111, 0b11110011, 0b11110000, 
  0b00000000, 0b00011111, 0b11111011, 0b11110000, 
  0b00000000, 0b00111111, 0b11111111, 0b11110000, 
  0b00000000, 0b01111111, 0b11111111, 0b11110000, 
  0b00000000, 0b11111111, 0b11111111, 0b11110000, 
  0b00000001, 0b11111111, 0b11111111, 0b11110000, 
  0b00000011, 0b11111111, 0b11111111, 0b11110000, 
  0b00000111, 0b11111111, 0b11111111, 0b11110000, 
  0b00001111, 0b11111111, 0b11111111, 0b11110000, 
  0b00011111, 0b11111111, 0b11111111, 0b11111000, 
  0b00111111, 0b11111111, 0b11111111, 0b11111100, 
  0b01111111, 0b11111111, 0b11111111, 0b11111110, 
  0b11111111, 0b11111111, 0b11111111, 0b11111111, 
  0b11111111, 0b11111111, 0b11111111, 0b11111111, 
  0b00001111, 0b11111111, 0b11111111, 0b11100000, 
  0b00001111, 0b11111111, 0b11111111, 0b11100000, 
  0b00001111, 0b11111111, 0b11111111, 0b11100000, 
  0b00001111, 0b11111111, 0b11111111, 0b11100000, 
  0b00001111, 0b11111111, 0b11111111, 0b11100000, 
  0b00001111, 0b11111111, 0b11111111, 0b11100000, 
  0b00001111, 0b11111111, 0b11111111, 0b11100000, 
  0b00001111, 0b11111000, 0b00111111, 0b11100000, 
  0b00001111, 0b11111000, 0b00111111, 0b11100000, 
  0b00001111, 0b11111000, 0b00111111, 0b11100000, 
  0b00001111, 0b11111000, 0b00111111, 0b11100000, 
  0b00001111, 0b11111000, 0b00111111, 0b11100000, 
  0b00001111, 0b11111000, 0b00111111, 0b11100000, 
  0b00001111, 0b11111000, 0b00111111, 0b11100000, 
  0b00001111, 0b11111000, 0b00111111, 0b11100000, 
  0b00001111, 0b11111000, 0b00111111, 0b11100000
};
const unsigned char arrowBack[] = {
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 
  0b00000000, 0b00001000, 0b00000000, 0b00000000, 
  0b00000000, 0b00011000, 0b00000000, 0b00000000, 
  0b00000000, 0b01111000, 0b00000000, 0b00000000, 
  0b00000000, 0b11111000, 0b00000000, 0b00000000, 
  0b00000001, 0b11111000, 0b00000000, 0b00000000, 
  0b00000011, 0b11111000, 0b00000000, 0b00000000, 
  0b00001111, 0b11111111, 0b11000000, 0b00000000, 
  0b00011111, 0b11111111, 0b11111110, 0b00000000, 
  0b00111111, 0b11111111, 0b11111111, 0b10000000, 
  0b01111111, 0b11111111, 0b11111111, 0b11100000, 
  0b00111111, 0b11111111, 0b11111111, 0b11110000, 
  0b00011111, 0b11111111, 0b11111111, 0b11111000, 
  0b00000111, 0b11111000, 0b01111111, 0b11111000, 
  0b00000011, 0b11111000, 0b00000001, 0b11111100, 
  0b00000001, 0b11111000, 0b00000000, 0b00111100, 
  0b00000000, 0b01111000, 0b00000000, 0b00011110, 
  0b00000000, 0b00111000, 0b00000000, 0b00001110, 
  0b00000000, 0b00011000, 0b00000000, 0b00000110, 
  0b00000000, 0b00001000, 0b00000000, 0b00000010, 
  0b00000000, 0b00000000, 0b00000000, 0b00000010, 
  0b00000000, 0b00000000, 0b00000000, 0b00000010, 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 
  0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 
  0b00000000, 0b00000000, 0b00000000, 0b00000000
};


RPI_PICO_Timer TimerUpdateDisplay(0);
bool TimerUpdateDisplay_Flag = false;

//Funzoine per lencoder counter Update 
volatile bool currClk, currDt;
volatile int Encoder_pos_menu = 0, Encoder_pos_value = 0;
int Modify_mode = 0;
void updateEncoderCall() {
  currClk = digitalRead(ENC_CLK);
  currDt = digitalRead(ENC_D);
  if (currDt ^ currClk)
    if(!Modify_mode) Encoder_pos_menu++;
    else Encoder_pos_value++;
  else 
    if(!Modify_mode) Encoder_pos_menu--;
    else Encoder_pos_value--;
}

bool volatile IRQ_flag = false, SW_flag , Encoder_flag= false;
// Funzione di callback per linterrupt
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
      SW_flag = true;  // Imposta il flag per indicare che è stato rilevato un tocco
      break;
  }
}

bool OnTimerUpdateDisplay(struct repeating_timer *t){
  TimerUpdateDisplay_Flag = true;
  return true;
}

void Initial_Setup(){
  //Setup RP2040
  SPL("Setup Started:");

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
  
  lcd.fillScreen(TFT_BLACK);
  lcd.setRotation(1);
  int32_t padding = lcd.textWidth("0",fontList[7]); // get the width of the text in pixels
  lcd.setTextPadding(padding);
  lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  //Retrolliminazione ON
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
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

  if (!TimerUpdateDisplay.attachInterrupt(8 /*Hz*/, OnTimerUpdateDisplay)) {
    SPL("Failed to initialize Timer TimerUpdateDisplay");
    while (1); 
  }
  TimerUpdateDisplay.disableTimer();
  SPL("TimerUpdateDisplay successfully Initialized");


  SPL("Setup Finished");
}