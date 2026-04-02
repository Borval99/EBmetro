#include <Adafruit_ST7735.h>
//  Funzioni che permettono l'interfacciamento con il display o un secondo Arduino utilizzato come Slave per pilotare un display
/*
GFX
  MENU:
  -a:Init
  -b:Cursor
    -0:WIRE 
      -a:Init
      -b:Vars
      -c:Cursor
    -1:RAMP
      -a:Init
      -b:Vars
      -c:Cursor
    -2:CONFIG
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define DISPLAY_ON_BOARD  // Define se il display è pilotato da questo Arduino

#define TFT_CS 2
#define TFT_RST -1
#define TFT_DC 3

extern Adafruit_ST7735 tft;

#ifdef DISPLAY_ON_BOARD
// Variabili per il MENU
extern const byte Menu_gfx_size;         // Dimensione delle etichette del menu principale
extern const char Menu_gfx_label[4][6];  // Etichette del menu principale
extern const byte Menu_gfx_cursor[8];    // Cursore del menu principale (Ax,Ay,Bx,By,..)
extern const int Menu_gfx_color[4];      // Colori del menu principale

//Definizione delle variabili per WIRE
extern const byte Wire_gfx_cursor[2];  //Wire page output Cursor  (Ax,Ay,Bx,By,..)

//Definizione delle variabili per RAMP
extern const byte Ramp_gfx_cursor_output[2];  //Ramp page output Cursor  (Ax,Ay,Bx,By,..)
extern const byte Ramp_gfx_cursor_input[8];   //Ramp page input Cursor   (Ax,Ay,Bx,By,..)
extern const byte Ramp_gfx_label_significantDigits[4];
extern const byte Ramp_gfx_label_divisor[4];

//Definizione delle variabili per Wire-Ramp
extern const byte WireRamp_gfx_cursor[4];  //WireRamp page output Cursor  (Ax,Ay,Bx,By,..)

extern const unsigned char Menu_gfx_returnButton[32];


void GFX_Menu_Init();
void GFX_Menu_Cursor(int _selectedCursor);

void GFX_Wire_Init(int _wireCurrentValue);
void GFX_Wire_Cursor(int _selectedCursor, bool _modifyMode, int _wireCurrentValue);
void GFX_Wire_Vars(int _operationStatus);

void GFX_Ramp_Init(int _selectedCursor, float _rampLabelValues[]);
void GFX_Ramp_Cursor(int _selectedCursor, float _rampLabelValues[]);
void GFX_Ramp_Vars(int _operationStatus);

void GFX_WireRamp_Vars(float _voltage, float _current);


#else

#endif

#endif
