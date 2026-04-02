void menu_display_bordoEsterno(){
  lcd.setColor(TFT_WHITE);
  lcd.drawLine(0,0,0,319);
  lcd.drawLine(1,0,1,318);
  lcd.drawLine(0,0,479,0);
  lcd.drawLine(0,1,478,1);
  lcd.drawLine(479,0,479,319);
  lcd.drawLine(478,1,478,318);
  lcd.drawLine(0,319,479,319);
  lcd.drawLine(1,318,478,318);
}

void menu_display_initial(){
  //Titolo Menu
  lcd.setTextColor(TFT_WHITE);
  lcd.setFont(fontList[6]);
  lcd.setTextSize(1);
  lcd.setTextDatum(top_center);
  lcd.drawString("EBmetro Case 2024",240,30);
  
  //Scritte Rettangoli Menu
  lcd.setTextDatum(bottom_center);
  lcd.setTextSize(1);
  lcd.setFont(fontList[3]);
  lcd.drawString("Ramp",130,175);
  lcd.drawString("Wire",350,175);
  lcd.drawString("Settings",240,265);
}

void menu_display_rect(int numeroRett){
  
  switch(numeroRett){
    case 1:
      lcd.setColor(TFT_WHITE);
      lcd.drawRect(50,120,160,60);
      lcd.setColor(TFT_BLUE);
      lcd.drawRect(270,120,160,60);
      lcd.setColor(TFT_WHITE);
      lcd.drawRect(130,210,220,60);
      break;
    case 2:
      lcd.setColor(TFT_WHITE);
      lcd.drawRect(50,120,160,60);
      lcd.drawRect(270,120,160,60);
      lcd.setColor(TFT_BLUE);
      lcd.drawRect(130,210,220,60);
      break;
    default:
      lcd.setColor(TFT_BLUE);
      lcd.drawRect(50,120,160,60);
      lcd.setColor(TFT_WHITE);
      lcd.drawRect(270,120,160,60);
      lcd.drawRect(130,210,220,60);
      break;

  }
}



