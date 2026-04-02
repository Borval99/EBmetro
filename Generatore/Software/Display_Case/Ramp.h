

uint8_t ramp_mode = 0;
int ramp_last_selected = 0;

uint8_t selected_handle = 0;

uint8_t ramp_var_value[10]={0,0,1,0,0,10,10,10,100,0};
uint8_t ramp_var_max[10]  ={0,0,5,0,0,50,50,50,150,0};
uint8_t ramp_var_min[10]  ={0,0,1,0,0,1,1,10,70,0};

char bufferV[6];  
char bufferA[6];

int32_t valori_grafico_32[380] = {0}; //Gli ultimi 20 sono le medie
uint8_t valori_grafico[1522] = {0};

//Function List
int ramp_var_clipping(int index, int value);
void ramp_display_select(int selected);
void ramp_wire_display_status(int status);
void ramp_wire_display_mul(float voltage,float current, uint8_t mode);
void ramp_display_initial();

//Function Structure
int ramp_var_clipping(int index, int value){
  if(value > ramp_var_max[index]) value = ramp_var_max[index];
  if(value < ramp_var_min[index]) value = ramp_var_min[index];
  ramp_var_value[index] = value;
  return value;
}

void ramp_display_select(int selected){  
  if(selected < 0 || selected > 9) return;
  lcd.setFont(fontList[7]);
  lcd.setTextSize(0.35);
  int temp = ramp_last_selected;
  
  for(int i = 0; i < 2; i++){
    if(selected == ramp_last_selected) i = 1;
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
      case 1: //switch case/manico
        lcd.drawRect(325,100,140,60);
        break;
      case 2: //cycles
        lcd.setTextDatum(bottom_center);
        lcd.setCursor(415,187);
        lcd.printf("%02d",ramp_var_value[temp]);
        break;
      case 3: //Stop
        lcd.drawRect(405,240,60,60);
        break;
      case 4: //Start
        lcd.drawRect(325,240,60,60);
        break;
      case 5: //asse x numero destra
        lcd.setTextDatum(bottom_center);
        lcd.setCursor(124,295);
        lcd.printf("%03.1f", (float)ramp_var_value[temp]/10);
        lcd.setCursor(224,295);
        lcd.printf("%03.1f", (float)ramp_var_value[temp]/10);
        break;
      case 6: //asse x numero sinistra
        lcd.setTextDatum(bottom_center);
        lcd.setCursor(74,295);
        lcd.printf("%03.1f", (float)ramp_var_value[temp]/10);
        lcd.setCursor(174,295);
        lcd.printf("%03.1f", (float)ramp_var_value[temp]/10);
        break;
      case 7:
        lcd.setTextDatum(top_left);
        lcd.setCursor(15,252);
        lcd.printf("%3.1f", (float)ramp_var_value[temp]/10);
        break;
      case 8:
        lcd.setTextDatum(top_left);
        lcd.setCursor(8,182);
        lcd.printf("%04.1f", (float)ramp_var_value[temp]/10);
        break;
      case 9:
        lcd.drawRect(325,20,60,60);
        break;
      default: break;
    }
  }
  ramp_last_selected = selected;
}

//Valido per entrambi
void ramp_wire_display_status(int status){
  lcd.fillRect(338,195,105,30,TFT_BLACK);
  lcd.setTextDatum(bottom_center);
  lcd.setCursor(340,225);
  lcd.setFont(fontList[2]);
  lcd.setTextSize(1);
  if(status == 0){
    lcd.setTextColor(TFT_RED);
    lcd.printf("Stopped");
  }else{
    lcd.setTextColor(TFT_GREEN);
    lcd.printf("Started");
  }
}

void ramp_display_handle_selected(uint8_t selected_handle){
  lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  lcd.setTextDatum(bottom_center);
  lcd.setTextSize(1);
  lcd.setFont(fontList[1]);
  lcd.setCursor(327,150);
  switch(selected_handle){
    case 0: //Piccolo
      lcd.printf("Coil: 14cm    ");
      break;
    case 1: //Piccolo x5
      lcd.printf("Coil: 14cm x5");
      break;
    case 2: //Medio
      lcd.printf("Coil: 20cm    ");
      break;
    case 3: //Grande
      lcd.printf("Coil: 30cm    ");
      break;
    default: break;
  }
}

void ramp_wire_display_mul(float voltage,float current, uint8_t mode){
  if(voltage > 99.99 && mode == 0) voltage = 99.99;
  if(voltage > 999.9 && mode == 1) voltage = 999.9;
  if(current > 99.99) current = 99.99;
  if(voltage < 0 && mode == 0) voltage = 0;
  if(voltage < -999.9 && mode == 1) voltage = -999.9;
  if(current < 0) current = 0;
  lcd.setTextDatum(top_left);
  lcd.setTextSize(1.3);
  lcd.setFont(fontList[7]);    

  if(mode == 0){    //Case
    snprintf(bufferV, sizeof(bufferV), "%05.2f", voltage);

  }else{           //Handle
    snprintf(bufferV, sizeof(bufferV), "%05.1f", fabs(voltage));
    
    if(voltage >= 0){
      lcd.setTextColor(TFT_WHITE,TFT_BLACK);
      lcd.setFont(fontList[4]);
      lcd.setTextSize(0.7);
      lcd.setCursor(8,15);
      lcd.printf("+");
    }else{
      lcd.setColor(TFT_BLACK);
      lcd.fillRect(26,23,5,14);
      lcd.fillRect(26,41,5,14);
    }
    
  }
  lcd.setTextDatum(top_left);
  lcd.setTextSize(1.3);
  lcd.setFont(fontList[7]);
  lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  lcd.drawString((String)bufferV,51,10); 

  snprintf(bufferA, sizeof(bufferA), "%05.2f", current);
  lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  lcd.drawString((String)bufferA,51,80);
}

void deserialize_to_int32(const uint8_t *input, int input_size, int32_t *output) {
    int output_size = input_size / 4;

    for (int i = 0; i < output_size; i++) {
        // Ricostruisci il numero a 32 bit dai 4 byte successivi
        output[i] = (int32_t)(input[i * 4] << 24) | 
                    (int32_t)(input[i * 4 + 1] << 16) |
                    (int32_t)(input[i * 4 + 2] << 8) |
                    (int32_t)(input[i * 4 + 3]);
    }
}

void ramp_show_last_graph(){
  deserialize_to_int32(valori_grafico+1, 1520,valori_grafico_32);
  int32_t max_value_graph = valori_grafico_32[0];
  int32_t min_value_graph = valori_grafico_32[0];
  for (int i = 0; i < 360; i++){
    if (valori_grafico_32[i] > max_value_graph) {
        max_value_graph = valori_grafico_32[i];
    }
    if (valori_grafico_32[i] < min_value_graph) {
        min_value_graph = valori_grafico_32[i];
    }
  }
  for(int i = 0; i < 360; i++){
  lcd.drawPixel(i+62,310-map(valori_grafico_32[i],min_value_graph,max_value_graph,20,290),TFT_YELLOW);
  }
  lcd.setTextDatum(top_left);
  lcd.setTextColor(TFT_YELLOW,TFT_BLACK);
  lcd.setFont(fontList[7]);
  lcd.setTextSize(0.33);

  snprintf(bufferV, sizeof(bufferV), "%05d", fabs(max_value_graph));
  lcd.drawString((String)bufferV,13,15);
  snprintf(bufferV, sizeof(bufferV), "%05d", fabs(min_value_graph));
  lcd.drawString((String)bufferV,13,280);
  lcd.setFont(fontList[1]);
  lcd.setTextSize(0.65);
  lcd.setTextColor(TFT_YELLOW);
  lcd.drawString("+",3,16);
  lcd.drawString("+",3,281);
  lcd.setColor(TFT_BLACK);
  if(max_value_graph<0){
    lcd.drawRect(7,17,1,3);
    lcd.drawRect(7,22,1,3);
  }
  if(min_value_graph<0){
    lcd.drawRect(7,282,1,3);
    lcd.drawRect(7,287,1,3);
  }
}

void display_graph_init(){

  lcd.setColor(TFT_BLUE);
  lcd.drawRect(429, 10, 40, 40);
  lcd.drawBitmap(433,14,arrowBack,32,32,TFT_WHITE);

  lcd.setColor(TFT_WHITE);
  lcd.drawLine(59,19,59,300);
  lcd.drawLine(59,300,430,300);

}

void ramp_display_initial(){
  //delimitazione
  lcd.setTextDatum(top_left);
  lcd.setColor(TFT_WHITE);
  lcd.drawLine(0,150,310,150);
  lcd.drawLine(310,0,310,150);
  lcd.drawLine(310,150,310,319);
  //Assi
  lcd.drawLine(50,170,50,270);
  lcd.drawLine(50,270,280,270);
  //Frecce
  lcd.drawLine(50,170,45,175);
  lcd.drawLine(50,170,55,175);
  lcd.drawLine(275,265,280,270);
  lcd.drawLine(275,275,280,270);
  //Grafico
  lcd.drawLine(60,260,110,260);
  lcd.drawLine(110,260,160,190);
  lcd.drawLine(160,190,210,190);
  lcd.drawLine(210,190,260,260);
  //Dots Assi
  lcd.drawLine(47,190,53,190);
  lcd.drawLine(47,260,53,260);
  lcd.drawLine(60,267,60,273);
  lcd.drawLine(110,267,110,273);
  lcd.drawLine(160,267,160,273);
  lcd.drawLine(210,267,210,273);
  lcd.drawLine(260,267,260,273);
  //Estremi Assi
  lcd.setTextSize(1);
  lcd.setFont(fontList[1]);
  lcd.setTextColor(TFT_YELLOW);
  lcd.setCursor(25,160);
  lcd.print("A");
  lcd.setCursor(282,280);
  lcd.print("t");

  lcd.setTextDatum(top_left);
  lcd.setFont(fontList[4]);
  lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  lcd.setTextSize(0.7);
  lcd.setCursor(8,15);
  lcd.printf("+"); 
  lcd.setCursor(8,85);
  lcd.printf("+");

  //ICONE

  //Icone di START / STOP
  lcd.setTextDatum(bottom_center);
  lcd.setColor(TFT_WHITE);
  lcd.drawRect(325,240,60,60);
  lcd.drawRect(405,240,60,60);
  lcd.fillTriangle(338,253,338,287,372,270,TFT_GREEN);
  lcd.fillRect(418,252,34,34,TFT_RED);


  //cicli
  lcd.setTextColor(TFT_WHITE,TFT_BLACK);
  lcd.setTextSize(1);
  lcd.setFont(fontList[1]);
  lcd.setCursor(340,190);
  lcd.printf("Cycles:");
  lcd.setTextSize(0.35);
  lcd.setFont(fontList[7]);
  lcd.setCursor(415,187);
  lcd.printf("%02d",ramp_var_value[2]);

  //HOME o TORNA INDIETRO
  lcd.drawRect(405,20,60,60);
  lcd.drawBitmap(418,34,menuButton,32,32,TFT_WHITE);
  lcd.setColor(TFT_WHITE);

  //Pulsante ultimo grafico
  lcd.drawRect(325,20,60,60);
  lcd.drawLine(335,30,335,70);
  lcd.drawLine(335,70,375,70);
  lcd.drawLine(335,30,333,32);
  lcd.drawLine(335,30,337,32);
  lcd.drawLine(373,68,375,70);
  lcd.drawLine(373,72,375,70);
  lcd.drawLine(340,65,350,45);
  lcd.drawLine(350,45,355,55);
  lcd.drawLine(355,55,365,35);
  
  //Pulsante SWITCH CASE/MANICO
  lcd.drawRect(325,100,140,60);
  lcd.setTextDatum(bottom_center);
  lcd.setTextSize(1);
  lcd.setFont(fontList[1]);
  lcd.setCursor(333,125);
  lcd.printf("Measure On");



  //Asse x
  lcd.setTextDatum(bottom_center);
  lcd.setTextSize(0.35);
  lcd.setFont(fontList[7]);
  lcd.setCursor(74,295);
  lcd.printf("%03.1f", (float)ramp_var_value[6]/10);
  lcd.setCursor(124,295);
  lcd.printf("%03.1f", (float)ramp_var_value[5]/10);
  lcd.setCursor(174,295);
  lcd.printf("%03.1f", (float)ramp_var_value[6]/10);
  lcd.setCursor(224,295);
  lcd.printf("%03.1f", (float)ramp_var_value[5]/10);

  //Numeri Asse Y
  lcd.setTextDatum(top_left);
  lcd.setTextSize(0.35);
  lcd.setFont(fontList[7]);
  lcd.setCursor(8,182);
  lcd.printf("%04.1f", (float)ramp_var_value[8]/10);
  lcd.setCursor(15,252);
  lcd.printf("%3.1f", (float)ramp_var_value[7]/10);

  //Multimetro
  lcd.setTextSize(1);
  lcd.setFont(fontList[4]);
  lcd.setTextColor(TFT_YELLOW);
  lcd.setCursor(250,10);
  lcd.print("V");
  lcd.setCursor(250,80);
  lcd.print("A");
}

