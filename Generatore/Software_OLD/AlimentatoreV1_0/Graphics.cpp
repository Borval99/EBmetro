#include "Graphics.h"

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#ifdef DISPLAY_ON_BOARD
// Definizione delle variabili per il MENU
const byte Menu_gfx_size = 3;                                                           // Dimensione delle etichette del menu principale
const char Menu_gfx_label[4][6] = { "Mode", "Wire", "Ramp", "Calib" };                  // Etichette del menu principale
const byte Menu_gfx_cursor[8] = { 30, 10, 30, 45, 30, 86, 20, 130 };                    // Cursore del menu principale (Ax,Ay,Bx,By,..)
const int Menu_gfx_color[4] = { ST77XX_WHITE, ST77XX_RED, ST77XX_GREEN, ST77XX_BLUE };  // Colori del menu principale

//Definizione delle variabili per WIRE
const byte Wire_gfx_cursor[2] = { 20, 110 };  //Wire page output Cursor  (Ax,Ay,Bx,By,..)

//Definizione delle variabili per RAMP
const byte Ramp_gfx_cursor_output[2] = { 76, 123 };                         //Ramp page output Cursor  (Ax,Ay,Bx,By,..)
const byte Ramp_gfx_cursor_input[8] = { 10, 65, 26, 98, 51, 98, 40, 150 };  //Ramp page input Cursor   (Ax,Ay,Bx,By,..)
const byte Ramp_gfx_label_significantDigits[4] = { 1, 1, 1, 0 };
const byte Ramp_gfx_label_divisor[4] = { 10, 10, 10, 1 };

//Definizione delle variabili per Wire-Ramp
const byte WireRamp_gfx_cursor[4] = { 10, 6, 10, 31 };  //WireRamp page output Cursor  (Ax,Ay,Bx,By,..)

//Home Icon Bitmap
const unsigned char Menu_gfx_returnButton[32] = {
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000001, B00000000,
  B00000011, B10110000,
  B00000111, B11110000,
  B00001111, B11110000,
  B00011111, B11110000,
  B00111111, B11111000,
  B01111111, B11111100,
  B11111111, B11111110,
  B00111111, B11111000,
  B00111100, B01111000,
  B00111100, B01111000,
  B00111100, B01111000,
  B00111100, B01111000
};
#else

#endif

void GFX_Menu_Init() {
#ifdef DISPLAY_ON_BOARD
  tft.setTextSize(Menu_gfx_size);
  tft.setCursor(Menu_gfx_cursor[0], Menu_gfx_cursor[1]);
  tft.setTextColor(Menu_gfx_color[0]);
  tft.print(Menu_gfx_label[0]);

  tft.fillRect(15, 40, 98, 32, ST77XX_BLACK);
  tft.drawRect(15, 40, 98, 32, ST77XX_WHITE);
#else

#endif
}

void GFX_Menu_Cursor(int _selectedCursor) {
#ifdef DISPLAY_ON_BOARD
  tft.setTextSize(Menu_gfx_size);
  for (byte i = 0; i < 3; i++) {
    if (i == _selectedCursor) {
      tft.fillRect(15, 40 + i * 42, 98, 32, ST77XX_WHITE);
    } else {
      tft.fillRect(15, 40 + i * 42, 98, 32, ST77XX_BLACK);
      tft.drawRect(15, 40 + i * 42, 98, 32, ST77XX_WHITE);
    }

    tft.setCursor(Menu_gfx_cursor[(i+1) * 2], Menu_gfx_cursor[(i+1) * 2 + 1]);
    tft.setTextColor(Menu_gfx_color[i+1]);
    tft.print(Menu_gfx_label[i+1]);
  }
#else

#endif
}

void GFX_Wire_Init(int _wireCurrentValue) {
#ifdef DISPLAY_ON_BOARD
  tft.fillScreen(ST77XX_BLACK);
  tft.drawRect(0, 0, 100, 50, ST77XX_WHITE);
  tft.drawFastHLine(0, 25, 100, ST77XX_WHITE);

  tft.drawRect(104, 0, 21, 20, ST77XX_WHITE);
  tft.drawBitmap(107, 0, Menu_gfx_returnButton, 16, 16, ST77XX_WHITE);

  tft.drawRect(39, 60, 50, 35, ST77XX_WHITE);
  tft.drawChar(75, 70, 'A', ST77XX_YELLOW, NULL, 2);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);

  tft.setCursor(20, 110);
  tft.print("Running");

  tft.drawRect(70, 133, 25, 25, ST77XX_WHITE);
  tft.drawRect(100, 133, 25, 25, ST77XX_WHITE);
  tft.fillTriangle(75, 138, 75, 152, 89, 145, ST77XX_GREEN);
  tft.fillRect(104, 138, 15, 15, ST77XX_RED);

  tft.drawChar(80, 6, 'V', ST77XX_YELLOW, NULL, 2);
  tft.drawChar(80, 31, 'A', ST77XX_YELLOW, NULL, 2);


  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

  tft.setCursor(8, 70);
  if (_wireCurrentValue == 1) tft.print("  ");
  else tft.print(_wireCurrentValue - 1);

  tft.setCursor(48, 70);
  tft.print(_wireCurrentValue);

  tft.setCursor(96, 70);
  if (_wireCurrentValue == 30) tft.print("  ");
  else tft.print(_wireCurrentValue + 1);
#else

#endif
}

void GFX_Wire_Cursor(int _selectedCursor, bool _modifyMode, int _wireCurrentValue) {
#ifdef DISPLAY_ON_BOARD

  if (_selectedCursor == 3) tft.drawRect(39, 60, 50, 35, ST77XX_BLUE);
  else tft.drawRect(39, 60, 50, 35, ST77XX_WHITE);

  if (_selectedCursor == 4) tft.drawRect(70, 133, 25, 25, ST77XX_BLUE);
  else tft.drawRect(70, 133, 25, 25, ST77XX_WHITE);

  if (_selectedCursor == 5) tft.drawRect(100, 133, 25, 25, ST77XX_BLUE);
  else tft.drawRect(100, 133, 25, 25, ST77XX_WHITE);

  if (_selectedCursor == 6) tft.drawRect(104, 0, 21, 20, ST77XX_BLUE);
  else tft.drawRect(104, 0, 21, 20, ST77XX_WHITE);

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  if (_modifyMode) {
    tft.setCursor(8, 70);
    tft.print("  ");
    tft.setCursor(48, 70);
    tft.print("  ");
    tft.setCursor(96, 70);
    tft.print("  ");
  }

  tft.setCursor(8, 70);
  if (_wireCurrentValue == 1) tft.print("  ");
  else tft.print(_wireCurrentValue - 1);

  tft.setCursor(48, 70);
  tft.print(_wireCurrentValue);

  tft.setCursor(96, 70);
  if (_wireCurrentValue == 30) tft.print("  ");
  else tft.print(_wireCurrentValue + 1);

#else

#endif
}


void GFX_Wire_Vars(int _operationStatus) {
#ifdef DISPLAY_ON_BOARD
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(Wire_gfx_cursor[0], Wire_gfx_cursor[1]);
  tft.print("       ");
  tft.setCursor(Wire_gfx_cursor[0], Wire_gfx_cursor[1]);
  switch (_operationStatus) {
    case 0:
      tft.setTextColor(ST77XX_RED);
      tft.print("Stopped");
      break;
    case 1:
      tft.setTextColor(ST77XX_GREEN);
      tft.print("Running");
      break;
    case 2:
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("Pending");
      break;
  }

#else

#endif
}

void GFX_Ramp_Init(int _selectedCursor, float _rampLabelValues[]) {
#ifdef DISPLAY_ON_BOARD
  tft.setTextSize(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.drawRect(0, 0, 100, 50, ST77XX_WHITE);
  tft.drawFastHLine(0, 25, 100, ST77XX_WHITE);

  tft.drawRect(104, 0, 21, 20, ST77XX_WHITE);
  tft.drawBitmap(107, 0, Menu_gfx_returnButton, 16, 16, ST77XX_WHITE);


  tft.drawFastHLine(5, 55, 7, ST77XX_WHITE);
  tft.drawFastVLine(8, 55, 30, ST77XX_WHITE);
  tft.drawFastHLine(5, 85, 7, ST77XX_WHITE);

  tft.drawFastHLine(23, 85, 25, ST77XX_WHITE);
  tft.drawLine(48, 85, 72, 55, ST77XX_WHITE);
  tft.drawFastHLine(72, 55, 25, ST77XX_WHITE);
  tft.drawLine(98, 55, 122, 85, ST77XX_WHITE);

  tft.drawFastHLine(23, 93, 100, ST77XX_WHITE);
  for (byte i = 0; i < 5; i++) tft.drawFastVLine(23 + i * 25, 90, 7, ST77XX_WHITE);

  for (byte i = 0; i < 4; i++) {
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    
    if (_selectedCursor == i) tft.setTextColor(ST77XX_BLACK, ST77XX_YELLOW);
    else tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

    if (_rampLabelValues[i] <= 99 && i == 0) {
      tft.setCursor(Ramp_gfx_cursor_input[i * 2], Ramp_gfx_cursor_input[i * 2 + 1]);
      tft.print("0");
      tft.print((float)_rampLabelValues[i] / Ramp_gfx_label_divisor[i], Ramp_gfx_label_significantDigits[i]);
    } else {
      tft.setCursor(Ramp_gfx_cursor_input[i * 2], Ramp_gfx_cursor_input[i * 2 + 1]);
      tft.print((float)_rampLabelValues[i] / Ramp_gfx_label_divisor[i], Ramp_gfx_label_significantDigits[i]);
    }

    if (i == 1 || i == 2) {
      tft.setCursor(Ramp_gfx_cursor_input[i * 2] + 50, Ramp_gfx_cursor_input[i * 2 + 1]);
      tft.print((float)_rampLabelValues[i] / Ramp_gfx_label_divisor[i], Ramp_gfx_label_significantDigits[i]);
    }
  }

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  //Se di vuole Max Min aggiungere anche 35, 110, 35, 120, in Ramp_gfx_cursor_output
  //tft.setCursor(0, 110);
  //tft.print("V Max:");
  //tft.setCursor(0, 120);
  //tft.print("I Max:");
  tft.setCursor(0, 150);
  tft.print("Cycles:");

  tft.drawRect(70, 133, 25, 25, ST77XX_WHITE);
  tft.drawRect(100, 133, 25, 25, ST77XX_WHITE);
  tft.fillTriangle(75, 138, 75, 152, 89, 145, ST77XX_GREEN);
  tft.fillRect(104, 138, 15, 15, ST77XX_RED);

  tft.drawChar(83, 6, 'V', ST77XX_YELLOW, NULL, 2);
  tft.drawChar(83, 31, 'A', ST77XX_YELLOW, NULL, 2);

#else

#endif
}

void GFX_Ramp_Cursor(int _selectedCursor, float _rampLabelValues[]) {
#ifdef DISPLAY_ON_BOARD

  tft.setTextSize(1);
  if (_selectedCursor == 4) tft.drawRect(70, 133, 25, 25, ST77XX_BLUE);
  else tft.drawRect(70, 133, 25, 25, ST77XX_WHITE);

  if (_selectedCursor == 5) tft.drawRect(100, 133, 25, 25, ST77XX_BLUE);
  else tft.drawRect(100, 133, 25, 25, ST77XX_WHITE);

  if (_selectedCursor == 6) tft.drawRect(104, 0, 21, 20, ST77XX_BLUE);
  else tft.drawRect(104, 0, 21, 20, ST77XX_WHITE);

  for (byte i = 0; i < 4; i++) {

    if (_selectedCursor == i) tft.setTextColor(ST77XX_BLACK, ST77XX_YELLOW);
    else tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  

    if (_rampLabelValues[i] <= 99 && i == 0) {
      tft.setCursor(Ramp_gfx_cursor_input[i * 2], Ramp_gfx_cursor_input[i * 2 + 1]);
      tft.print("0");
      tft.print((float)_rampLabelValues[i] / Ramp_gfx_label_divisor[i], Ramp_gfx_label_significantDigits[i]);
    } else {
      tft.setCursor(Ramp_gfx_cursor_input[i * 2], Ramp_gfx_cursor_input[i * 2 + 1]);
      tft.print((float)_rampLabelValues[i] / Ramp_gfx_label_divisor[i], Ramp_gfx_label_significantDigits[i]);
    }

    if (i == 1 || i == 2) {
      tft.setCursor(Ramp_gfx_cursor_input[i * 2] + 50, Ramp_gfx_cursor_input[i * 2 + 1]);
      tft.print((float)_rampLabelValues[i] / Ramp_gfx_label_divisor[i], Ramp_gfx_label_significantDigits[i]);
    }
    
  }
#else

#endif
}

void GFX_Ramp_Vars(int _operationStatus) {
#ifdef DISPLAY_ON_BOARD
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor(Ramp_gfx_cursor_output[0], Ramp_gfx_cursor_output[1]);
  tft.print("       ");
  tft.setCursor(Ramp_gfx_cursor_output[0], Ramp_gfx_cursor_output[1]);
  if (_operationStatus == 0) {
    tft.setTextColor(ST77XX_RED);
    tft.print("Stopped");
  } else {
    tft.setTextColor(ST77XX_GREEN);
    tft.print("Running");
  }

#else

#endif
}


void GFX_WireRamp_Vars(float _voltage, float _current) {
#ifdef DISPLAY_ON_BOARD
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(2);

  tft.setCursor(WireRamp_gfx_cursor[0], WireRamp_gfx_cursor[1]);
  tft.print("     ");
  tft.setCursor(WireRamp_gfx_cursor[0], WireRamp_gfx_cursor[1]);
  tft.print(_voltage);

  tft.setCursor(WireRamp_gfx_cursor[2], WireRamp_gfx_cursor[3]);
  tft.print("     ");
  tft.setCursor(WireRamp_gfx_cursor[2], WireRamp_gfx_cursor[3]);
  tft.print(_current);

#else

#endif
}
