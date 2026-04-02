
int last_selected = -1;

char bufferV[6];

//###################################   GENERAL   ################################

const unsigned char settingsButton[] = {
  0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000001, 0b10000000, 0b00000000,
  0b00000000, 0b00000011, 0b11000000, 0b00000000,
  0b00000000, 0b00000011, 0b11000000, 0b00000000,
  0b00000000, 0b11000011, 0b11000011, 0b00000000,
  0b00000001, 0b11101111, 0b11110111, 0b10000000,
  0b00000001, 0b11111111, 0b11111111, 0b10000000,
  0b00000000, 0b11111111, 0b11111111, 0b00000000,
  0b00000000, 0b11111000, 0b00011111, 0b00000000,
  0b00000000, 0b11110000, 0b00001111, 0b00000000,
  0b00000000, 0b11100000, 0b00000111, 0b00000000,
  0b00000111, 0b11100000, 0b00000111, 0b11100000,
  0b00001111, 0b11100001, 0b10000011, 0b11110000,
  0b00001111, 0b11100001, 0b10000011, 0b11110000,
  0b00000111, 0b11100000, 0b10000111, 0b11110000,
  0b00000000, 0b11100000, 0b00000111, 0b00000000,
  0b00000000, 0b11110000, 0b00001111, 0b00000000,
  0b00000000, 0b01111000, 0b00011111, 0b00000000,
  0b00000000, 0b11111110, 0b01111111, 0b00000000,
  0b00000001, 0b11111111, 0b11111111, 0b10000000,
  0b00000001, 0b11111111, 0b11111111, 0b10000000,
  0b00000000, 0b11000011, 0b11000011, 0b00000000,
  0b00000000, 0b00000011, 0b11000000, 0b00000000,
  0b00000000, 0b00000011, 0b11000000, 0b00000000,
  0b00000000, 0b00000001, 0b11000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000
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

//###################################   MAIN   ################################

void display_main_select(int selected) {
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
        lcd.drawRect(387, 230, 70, 70);
        break;
      case 2:  //Zero
        lcd.drawRect(296, 230, 70, 70);
        break;
      case 3:  //On/Off Motor
        lcd.drawRect(205, 230, 70, 70);
        break;
      case 4:  //UM
        lcd.drawRect(114, 230, 70, 70);
        break;
      case 5:  //EB
        lcd.drawRect(23, 230, 70, 70);
        break;

      default: break;
    }
  }
  last_selected = selected;
}

//Funzione per mostrare a video il Multimetro
void display_main_mul(float voltage){
  if(fabs(voltage) > 999.9) voltage = (voltage < 0)? -999.9 : 999.9; 

  lcd.setTextDatum(top_left);
  lcd.setTextSize(2);
  lcd.setFont(fontList[7]);
 
  //888.8
  snprintf(bufferV, sizeof(bufferV), "%05.1f", fabs(voltage));

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
}

//Funzione per la modifica delle unità di misura
void display_main_mode(bool UM, bool EB){
  lcd.setColor(TFT_BLACK);          //Pulisco i pulsanti e lunità di misura
  lcd.fillRect(346,55,100,65);
  lcd.fillRect(116, 232, 65, 65);
  lcd.fillRect(25, 232, 65, 65);

  lcd.setTextColor(TFT_YELLOW);
  if(EB){
    lcd.setFont(fontList[3]);     
    lcd.setTextSize(1.3);         //se modalità B
    lcd.setCursor(41, 242);
    lcd.print("B");
  } else {
    lcd.setFont(fontList[3]);   //Valore nel pulsante
    lcd.setTextSize(1.3);
    lcd.setCursor(41, 242);
    lcd.print("E");
  }
  if (!UM) {
      lcd.setFont(fontList[3]);     //Valore nel pulsante
      lcd.setTextSize(0.9);
      lcd.setCursor(120, 250);
      lcd.print("mV");          

      lcd.setTextSize(1);           //Valore di fianco al mul
      lcd.setCursor(345,80);
      lcd.print("m");

      lcd.setFont(fontList[4]);
      lcd.setCursor(374,55);
      lcd.print("V");

    } else {
      lcd.setFont(fontList[3]);   //Valore nel pulsante 
      lcd.setTextSize(0.9);
      lcd.setCursor(118, 250);
      lcd.print("A/U");
      }
}

//Funzione per la stampa dei 3 valori all'interno dell'array
void display_main_hold_value(float values[]){

  for(int i = 0; i < 3; i++){
    if(fabs(values[i]) > 999.9) values[i] = (values[i] < 0)? -999.9 : 999.9; 
    lcd.setTextDatum(top_left);
    lcd.setTextColor(TFT_WHITE,TFT_BLACK);
    lcd.setFont(fontList[7]);
    lcd.setTextSize(0.8);
    snprintf(bufferV, sizeof(bufferV), "%05.1f", fabs(values[i]));
    lcd.drawString((String)bufferV,35+i*160,156);

    lcd.setFont(fontList[3]);
    lcd.setTextSize(0.7);
    lcd.setCursor(10+160*i, 161);
    lcd.printf("+");
    if(values[i] <0){
    lcd.setColor(TFT_BLACK);
    lcd.fillRect(19+160*i,165,2,8);
    lcd.fillRect(19+160*i,175,2,8);
    }
  }


}

//Funnzione per il cambio di grafica dei diversi stati del motore
void display_main_motor_OnOff(bool motorState){
  lcd.setTextDatum(top_left);
  lcd.setTextSize(1);
  lcd.setFont(fontList[2]);
  lcd.setTextColor(TFT_CYAN);
  if(motorState){
    lcd.setCursor(221, 270);
    lcd.print("Off");
    lcd.setTextColor(TFT_GREEN);
    lcd.setCursor(223, 240);
    lcd.print("On");
  }else{
    lcd.setCursor(223, 240);
    lcd.print("On");
    lcd.setTextColor(TFT_RED);
    lcd.setCursor(221, 270);
    lcd.print("Off");
  }
}

//Inizializzazione Pagina principale
void display_main_initial() {

  last_selected = -1;

  //delimitazione
  lcd.setColor(TFT_WHITE);
  //Orizzontali
  lcd.drawLine(0, 130, 479, 130);
  lcd.drawLine(0, 210, 479, 210);
  //Verticali
  lcd.drawLine(160, 130, 160, 210);
  lcd.drawLine(320, 130, 320, 210);

  //Hold Values
  display_main_hold_value(valoriHold);

  //ICONE

  //Icone di EB / UM
  lcd.setColor(TFT_WHITE);
  lcd.drawRect(23, 230, 70, 70);
  lcd.drawRect(114, 230, 70, 70);

  //Icona di ON/OFF
  lcd.drawRect(205, 230, 70, 70);

  //Icona di Zero
  lcd.drawRect(296, 230, 70, 70);

  //Icona di Hold
  lcd.drawRect(387, 230, 70, 70);


  //SETTINGS
  lcd.drawRect(429, 10, 40, 40);
  lcd.drawBitmap(433,14,settingsButton,32,32,TFT_WHITE);

  //lcd.drawBitmap(418,34,menuButton,32,32,TFT_WHITE);

  //Valori Fissi
  lcd.setTextDatum(top_left);
  lcd.setTextSize(1);
  lcd.setFont(fontList[2]);
  lcd.setTextColor(TFT_CYAN);
  lcd.setCursor(223, 240);
  lcd.print("On");
  lcd.drawLine(215, 265, 265, 265);
  lcd.setCursor(221, 270);
  lcd.print("Off");
  display_main_motor_OnOff(Motor_State);
  lcd.setTextColor(TFT_CYAN);
  lcd.setCursor(304, 252);
  lcd.print("Zero");
  lcd.setCursor(395, 252);
  lcd.print("Hold");

  display_main_mode(UnitaMisura_State, E_B_State);
  function_main_adc_mode(E_B_State,E_Reading_Multiplier);

  lcd.setTextColor(TFT_WHITE);
  lcd.setFont(fontList[4]);
  lcd.setTextSize(1);
  lcd.setCursor(5, 30);
  lcd.printf("+");
}

//###################################   SETTINGS   ################################

//Funzione per mostrare l'hightlight di quando viene selezionato qualcosa nel settings
void display_settings_select(int selected) {
  if (selected < 0 || selected > 4) return;

  int temp = last_selected;
  lcd.setColor(TFT_BLACK);

  for (int i = 0; i < 2; i++) {
    if (selected == last_selected) i = 1;
    if (i == 0) {
      lcd.setColor(TFT_WHITE);
      lcd.setTextColor(TFT_WHITE,TFT_BLACK);
    } else {
      lcd.setColor(TFT_BLUE);
      lcd.setTextColor(TFT_BLUE,TFT_BLACK);
      temp = selected;
    }

    switch (temp) {
      case 4:  //home
        lcd.drawRect(429, 10, 40, 40);
        break;
      case 3:  //Graph
        lcd.drawRect(15, 15, 100, 40);
        break;
      case 2:  //E x1 / x10
        lcd.setTextDatum(top_left);
        lcd.setTextSize(1);
        lcd.setFont(fontList[2]);
        lcd.setCursor(150,75);

        if(!E_Reading_Multiplier){
          lcd.print("x1  ");
        }else{
          lcd.print("x10");
        }
        //lcd.drawRect(15, 65, 100, 40);

        break;
        
      case 1: //Zero E
        lcd.setCursor(150,125);
        lcd.printf("%05.1f",fabs(ZERO_E));
        if(ZERO_E<0){
          lcd.setCursor(140,125);
          lcd.printf("-");
        }else{
          lcd.setTextColor(TFT_BLACK);
          lcd.setCursor(140,125);
          lcd.printf("-");
        }
      break;

      case 0: //Zero B
        lcd.setCursor(150,175);
        lcd.printf("%05.1f",fabs(ZERO_B));
        if(ZERO_B<0){
          lcd.setCursor(140,175);
          lcd.printf("-");
        }else{
          lcd.setTextColor(TFT_BLACK);
          lcd.setCursor(140,175);
          lcd.printf("-");
        }
      break;

      default: break;
    }
  }
  last_selected = selected;
}

//Funzione per il cambio di stato del valore di Mul nei Settings
void display_settings_E_Mul(bool E_Mul,bool Selected){
  lcd.setTextDatum(top_left);
  if(Selected) lcd.setTextColor(TFT_BLUE, TFT_BLACK);
  else lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setTextSize(1);
  lcd.setFont(fontList[2]);
  lcd.setCursor(150,75);

  if(!E_Mul){
    lcd.print("x1  ");
  }else{
    lcd.print("x10");
  }
}

//Funzione per l'Inizializzazione dei Settings
void display_settings_init(){
  //SETTINGS
  last_selected = -1;
  lcd.setTextColor(TFT_WHITE);
  lcd.drawRect(429, 10, 40, 40);
  lcd.drawBitmap(433,14,menuButton,32,32,TFT_WHITE);

  //Graph
  lcd.drawRect(15, 15, 100, 40);

  lcd.setTextDatum(top_left);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setTextSize(1);
  lcd.setFont(fontList[2]);
  
  lcd.setCursor(25,25);
  lcd.print("Graph");

  lcd.setCursor(25,75);
  lcd.print("Gain E:");

  display_settings_E_Mul(E_Reading_Multiplier,false);

  lcd.setCursor(25,125);
  lcd.print("Zero E:");

  lcd.setCursor(150,125);
  lcd.printf("%05.1f", fabs(ZERO_E));

  if(ZERO_E<0){
    lcd.setCursor(140,125);
    lcd.printf("-");
  }else{
    lcd.setTextColor(TFT_BLACK);
    lcd.setCursor(140,125);
    lcd.printf("-");
  }

  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setCursor(25,175);
  lcd.print("Zero B:");

  lcd.setCursor(150,175);
  lcd.printf("%05.1f", fabs(ZERO_B));

  if(ZERO_B<0){
    lcd.setCursor(140,175);
    lcd.printf("-");
  }else{
    lcd.setTextColor(TFT_BLACK);
    lcd.setCursor(140,175);
    lcd.printf("-");
  }

} 

//###################################   GRAPH   ################################

//Funzione di Inizializzazione della pagina con il Grafico
void display_graph_init(){

  lcd.setColor(TFT_BLUE);
  lcd.drawRect(429, 10, 40, 40);
  lcd.drawBitmap(433,14,menuButton,32,32,TFT_WHITE);

  lcd.setColor(TFT_WHITE);
  lcd.drawLine(59,19,59,300);
  lcd.drawLine(59,300,430,300);

}
