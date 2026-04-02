#include "lwipopts.h"

int last_selected = 0;

char bufferV[6];



void display_bordoEsterno() {
  lcd.setColor(TFT_WHITE);
  lcd.drawLine(0, 0, 0, 319);
  lcd.drawLine(1, 0, 1, 318);
  lcd.drawLine(0, 0, 479, 0);
  lcd.drawLine(0, 1, 478, 1);
  lcd.drawLine(479, 0, 479, 319);
  lcd.drawLine(478, 1, 478, 318);
  lcd.drawLine(0, 319, 479, 319);
  lcd.drawLine(1, 318, 478, 318);
}

void display_select(int selected) {
  if (selected < 0 || selected > 5) return;

  int temp = last_selected;
  lcd.setColor(TFT_BLACK);

  for (int i = 0; i < 2; i++) {
    if (selected == last_selected) i = 1;
    if (i == 0) {
      lcd.setColor(TFT_WHITE);
      lcd.setTextColor(TFT_WHITE);
    } else {
      lcd.setColor(TFT_BLUE);
      lcd.setTextColor(TFT_BLUE);
      temp = selected;
    }

    switch (temp) {
      case 0:  //home
        lcd.drawRect(429, 10, 40, 40);
        break;
      case 1:  //Hold
        lcd.drawRect(390, 230, 70, 70);
        break;
      case 2:  //Zero
        lcd.drawRect(300, 230, 70, 70);
        break;
      case 3:  //On/Off Motor
        lcd.drawRect(210, 230, 70, 70);
        break;
      case 4:  //UM
        lcd.drawRect(110, 230, 70, 70);
        break;
      case 5:  //EB
        lcd.drawRect(20, 230, 70, 70);
        break;
      default: break;
    }
  }
  last_selected = selected;
}

void display_mul(float voltage, uint8_t mode){
  noInterrupts();
  if(fabs(voltage) > 99.99 && mode == 0) voltage = 99.99;
  if(fabs(voltage) > 999.9 && mode == 1) voltage = 999.9;

  lcd.setTextDatum(top_left);
  lcd.setTextSize(2);
  lcd.setFont(fontList[7]);
 
  if(mode == 0){    //88.88
    snprintf(bufferV, sizeof(bufferV), "%05.2f", fabs(voltage));
  }else{           //888.8
    snprintf(bufferV, sizeof(bufferV), "%05.1f", fabs(voltage));
  }
  
  lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  lcd.drawString((String)bufferV,65,18); 

   //Verifichiamo se è un numero positivo o negativo
  if (voltage >= 0) {
    lcd.setTextColor(TFT_WHITE);
    lcd.setFont(fontList[4]);
    lcd.setTextSize(1);
    lcd.setCursor(5, 30);
    lcd.printf("+");
  } else {
    lcd.setColor(TFT_BLACK);
    lcd.fillRect(33, 42, 6, 20);
    lcd.fillRect(33, 68, 6, 20);
  }
  interrupts();
}


    


/*

void ramp_display_mode(uint8_t mode){
  TimerUpdateDisplay.disableTimer();
  //lcd.fillRect(8,5,225,138,TFT_BLACK); //Clear screen from dots
  lcd.fillRect(327,130,135,20,TFT_BLACK);
  
  lcd.setTextDatum(top_left);
  lcd.setFont(fontList[4]);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(0.7);
  lcd.setCursor(8,15);
  lcd.printf("+"); 
  lcd.setCursor(8,85);
  lcd.printf("+");
  lcd.setTextDatum(bottom_center);
  lcd.setTextSize(1);
  lcd.setFont(fontList[1]);
  lcd.setCursor(327,150);

  if(mode == 0){    //Case
    lcd.printf("Mode: Case");
    lcd.setTextDatum(top_left);
    lcd.setFont(fontList[3]);
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_BLACK);
    lcd.setCursor(230,35);
    lcd.printf("u");
    

  }else{           //Handle
    lcd.printf("Mode: Handle");
    lcd.setTextDatum(top_left);
    lcd.setTextSize(1);
    lcd.setFont(fontList[3]);
    lcd.setTextColor(TFT_YELLOW);
    lcd.setCursor(230,35);
    lcd.printf("u");
  }
  TimerUpdateDisplay.enableTimer();
}

*/

void display_initial() {
  //delimitazione
  lcd.setColor(TFT_WHITE);
  lcd.drawLine(0, 130, 479, 130);
  lcd.drawLine(0, 210, 479, 210);
  lcd.drawLine(159, 130, 159, 210); // Barre verticali per Hold
  lcd.drawLine(319, 130, 319, 210); // Barre verticali per Hold


  //ICONE

  //Icone di EB / UM
  lcd.setColor(TFT_WHITE);
  lcd.drawRect(20, 230, 70, 70);
  lcd.drawRect(110, 230, 70, 70);

  //Icona di ON/OFF
  lcd.drawRect(210, 230, 70, 70);

  //Icona di Zero
  lcd.drawRect(300, 230, 70, 70);

  //Icona di Hold
  lcd.drawRect(390, 230, 70, 70);


  //SETTINGS
  lcd.drawRect(429, 10, 40, 40);

  //lcd.drawBitmap(418,34,menuButton,32,32,TFT_WHITE);

  //Valori Fissi
  lcd.setTextDatum(top_left);
  lcd.setTextSize(1);
  lcd.setFont(fontList[2]);
  lcd.setTextColor(TFT_CYAN);
  lcd.setCursor(228, 240);
  lcd.print("On");
  lcd.drawLine(220, 265, 270, 265);
  lcd.setCursor(226, 270);
  lcd.print("Off");
  lcd.setCursor(308, 252);
  lcd.print("Zero");
  lcd.setCursor(397, 252);
  lcd.print("Hold");
  lcd.setTextColor(TFT_YELLOW);
  lcd.setFont(fontList[3]);
  lcd.setTextSize(1.3);
  lcd.setCursor(38, 242);
  lcd.print("E");
  lcd.setFont(fontList[2]);
  lcd.setTextSize(1.3);
  lcd.setCursor(116, 250);
  lcd.print("mV");


  lcd.setTextColor(TFT_WHITE);
  lcd.setFont(fontList[4]);
  lcd.setTextSize(1);
  lcd.setCursor(5, 30);
  lcd.printf("+");

  lcd.setTextSize(1);
  lcd.setFont(fontList[4]);
  lcd.setTextColor(TFT_YELLOW);
  lcd.setCursor(374,55);
  lcd.print("V");

  lcd.setTextSize(1);
  lcd.setFont(fontList[3]);
  lcd.setTextColor(TFT_YELLOW);
  lcd.setCursor(345,80);
  lcd.print("m");
}

