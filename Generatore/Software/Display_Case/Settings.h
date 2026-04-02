
uint8_t settings_current_value = 2;

int settings_var_clipping(int value){
  if(value > 9) value = 9;
  if(value < 1) value = 1;
  settings_current_value = value;
  return value;
}

int settings_last_selected = 0;
void settings_display_select(int selected){
  if(selected < 0 || selected > 3) return;
  lcd.setFont(fontList[7]);
  lcd.setTextSize(0.35);
  int temp = settings_last_selected;
  for(int i = 0; i < 2; i++){
    if(selected == settings_last_selected) i = 1;
    if(i == 0){
      lcd.setColor(TFT_WHITE);
      lcd.setTextColor(TFT_WHITE,TFT_BLACK);
    }else {
      lcd.setColor(TFT_BLUE);
      lcd.setTextColor(TFT_BLUE,TFT_BLACK);
      temp = selected;
    }
    switch(temp){
      case 0: //Home
        lcd.drawRect(405,20,60,60);
        break;
      case 1: //Calibrazione
        lcd.drawRect(15,20,150,50);
        break;
      case 2: //SelfCheck
         lcd.drawRect(15,80,120,50);
        break;
      case 3: //Errore SelfCheck
        lcd.setTextDatum(top_left);
        lcd.setTextSize(0.45);
        lcd.setFont(fontList[7]);
        lcd.setCursor(290,93);
        lcd.printf("0.%d",settings_current_value);
        break;
      default: break;
    }
  }
  settings_last_selected = selected;
}

void settings_display_text(uint16_t text_to_show){
  lcd.setColor(TFT_BLACK);
  lcd.fillRect(80,50,320,150);
  lcd.setColor(TFT_WHITE);
  lcd.drawRect(80,50,320,150);
  lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  lcd.setTextSize(1);
  lcd.setFont(fontList[2]);
  lcd.setTextDatum(bottom_center);
  
  switch(text_to_show){
    case 1:
      lcd.setCursor(100,140);
      lcd.printf("Calibration is Running");
    break;
    case 2:
      lcd.setCursor(100,140);
      lcd.printf("Self-Test is Running");
    break;
    case 4:
      lcd.setCursor(100,100);
      lcd.printf("Calibration: OK");
      lcd.setCursor(100,140);
      lcd.printf("If you want to save,");
      lcd.setCursor(100,180);
      lcd.printf("Click on Self-Test");
    break;
    case 3:
      lcd.setCursor(100,140);
      lcd.printf("Calibration: Error");
    break;
    case 6:
      lcd.setCursor(100,140);
      lcd.printf("Self-Test: OK");
    break;
    case 5:
      lcd.setCursor(100,140);
      lcd.printf("Self-Test: Error");
    break;
    default: break;
  }
  lcd.setTextDatum(top_left);
}

void settings_display_print_maxmin(uint16_t min, uint16_t max){
  lcd.setTextDatum(top_left);
  lcd.setTextSize(0.45);
  lcd.setFont(fontList[7]);
  lcd.setTextColor(TFT_WHITE,TFT_BLACK);

  lcd.setCursor(245,19);
  lcd.printf("%05.2f",(float) min/100);  
  lcd.setCursor(245,49);
  lcd.printf("%05.2f",(float) max/100);
  }

void settings_display_initial(){
  //HOME o TORNA INDIETRO
  lcd.setTextDatum(top_left);
  lcd.setColor(TFT_WHITE);
  lcd.drawRect(405,20,60,60);
  lcd.drawBitmap(418,34,menuButton,32,32,TFT_WHITE);

  lcd.setTextSize(1);
  lcd.setFont(fontList[2]);

  //Calibrazione
  lcd.drawRect(15,20,150,50);
  
  lcd.setCursor(20,35);
  lcd.printf("Calibration");

  lcd.setCursor(180,20);
  lcd.printf("Min:");
  lcd.setCursor(180,50);
  lcd.printf("Max:");
  //SelfTest e Errore Max
  lcd.drawRect(15,80,120,50);

  lcd.setCursor(20,95);
  lcd.printf("Self-Test");
  
  lcd.setCursor(150,95);
  lcd.printf("Max Error:");
  lcd.setCursor(330,95);
  lcd.printf("A");

  lcd.setCursor(310,20);
  lcd.printf("A");
  lcd.setCursor(310,50);
  lcd.printf("A");

  lcd.setTextSize(0.45);
  lcd.setFont(fontList[7]);

  lcd.setCursor(290,93);
  lcd.printf("0.%d", settings_current_value);
}