
int wire_last_selected = 0;
uint8_t wire_current_value = 1;

int wire_var_clipping( int value){
  if(value > 30) value = 30;
  if(value < 1) value = 1;
  wire_current_value = value;
  return value;
}

void wire_display_select(int selected){
  if(selected < 0 || selected > 3) return;
  lcd.setFont(fontList[7]);
  lcd.setTextSize(0.35);
  int temp = wire_last_selected;
  for(int i = 0; i < 2; i++){
    if(selected == wire_last_selected) i = 1;
    if(i == 0){
      lcd.setColor(TFT_WHITE);
      lcd.setTextColor(TFT_WHITE,TFT_BLACK);
    }else {
      lcd.setColor(TFT_BLUE);
      lcd.setTextColor(TFT_BLUE,TFT_BLACK);
      temp = selected;
    }
    switch(temp){
      case 0: //home
        lcd.drawRect(405,20,60,60);
        break;
      case 1: //Stop
        lcd.drawRect(405,240,60,60);
        break;
      case 2: //Start
        lcd.drawRect(325,240,60,60);
        break;
      case 3: //valore centrale e laterali

        lcd.setTextDatum(bottom_center);
        lcd.setTextSize(0.75);
        lcd.setFont(fontList[7]);
        
        lcd.setCursor(132,270);
        lcd.printf("%02d",wire_current_value);
      
        lcd.setTextColor(TFT_WHITE,TFT_BLACK);
        if(selected == 3)  {//Clear text with black rect
    lcd.setColor(TFT_BLACK);
     
  }
        if(wire_current_value - 1 != 0){
          lcd.setCursor(40,270);
          lcd.printf("%02d",wire_current_value - 1);
        }else{
          lcd.fillRect(38,230,50,40);
        }
        if(wire_current_value + 1 != 31){
          lcd.setCursor(224,270);
          lcd.printf("%02d",wire_current_value + 1);
        }else{
          lcd.fillRect(222,230,50,40);
        }
        break;
      default: break;
    }
  }
  wire_last_selected = selected;
}

void wire_display_initial(){
  //delimitazione
  lcd.setTextDatum(top_left);
  lcd.setColor(TFT_WHITE);
  lcd.drawLine(0,150,310,150);
  lcd.drawLine(310,0,310,150);
  lcd.drawLine(310,150,310,319);
  
  //Icone di START / STOP
  lcd.setTextDatum(bottom_center);
  lcd.setColor(TFT_WHITE);
  lcd.drawRect(325,240,60,60);
  lcd.drawRect(405,240,60,60);
  lcd.fillTriangle(338,253,338,287,372,270,TFT_GREEN);
  lcd.fillRect(418,252,34,34,TFT_RED);
  lcd.setCursor(340,225);
  lcd.setFont(fontList[2]);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_RED);
  lcd.printf("Stopped");

  //HOME o TORNA INDIETRO
  lcd.setColor(TFT_WHITE);
  lcd.drawRect(405,20,60,60);
  lcd.drawBitmap(418,34,menuButton,32,32,TFT_WHITE);

  //Multimetro
  lcd.setTextDatum(top_left);
  lcd.setColor(TFT_WHITE);
  lcd.setTextSize(1);
  lcd.setFont(fontList[4]);
  lcd.setTextColor(TFT_YELLOW);
  lcd.setCursor(250,10);
  lcd.print("V");
  lcd.setCursor(250,80);
  lcd.print("A");
  
  lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  lcd.setTextSize(0.7);
  lcd.setCursor(8,15);
  lcd.printf("+"); 
  lcd.setCursor(8,85);
  lcd.printf("+");


  //3 Value current
  lcd.setTextDatum(bottom_center);
  lcd.setColor(TFT_WHITE);
  lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  lcd.drawRect(125,220,60,60);
  lcd.setTextSize(1);
  lcd.setFont(fontList[2]);
  lcd.setCursor(25,200);
  lcd.printf("Set Current on Wire");
  lcd.setTextSize(0.75);
  lcd.setFont(fontList[7]);
  
  if(wire_current_value - 1 != 0){
    lcd.setCursor(40,270);
    lcd.printf("%02d",wire_current_value - 1);
  }
  lcd.setCursor(132,270);
  lcd.printf("%02d",wire_current_value);
  if(wire_current_value + 1 != 31){
    lcd.setCursor(224,270);
    lcd.printf("%02d",wire_current_value + 1);
  }
}