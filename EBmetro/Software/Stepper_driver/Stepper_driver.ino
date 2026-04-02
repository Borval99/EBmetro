

// Pin 0 per la nota e pin 4 per il verso di rotazione del motore 
#define PIN_OUTPUT 0  // Definisce il pin di uscita nota per step driver
#define PIN_motore 4  // Direzione di rotazione del motore (Antiorario)

volatile unsigned int counter = 0;

//Valore base 71 
void setup() {
  // Configura il pin come output
  pinMode(PIN_OUTPUT, OUTPUT);

  pinMode(PIN_motore, OUTPUT);
  digitalWrite(PIN_motore, 1); // Direzione standard del motore Antiorario


  // Configura il Timer 0 in modalità CTC
  TCCR0A = 0;  // Resetta il registro TCCR0A
  TCCR0B = 0;  // Resetta il registro TCCR0B
  OCR0A = 0;   // Inizializza il registro OCR0A

  // Calcola il valore di OCR0A per ottenere una frequenza di 9216Hz
  //byte ocrValue = (byte) ((16500000.0 / (2.0 * 64.0 * 9216.0)) - 1) = 13 (era 19)
  OCR0A = 255;

  // Imposta il Timer 0 in modalità CTC (Clear Timer on Compare Match)
  TCCR0A |= (1 << WGM01);

  // Attiva il timer con il prescaler 64
  TCCR0B |= (1 << CS01);

  // Abilita l'uscita su OCR0A (PB0)
  TCCR0A |= (1 << COM0A0);

  // Abilita l'interrupt di Compare Match su Timer0
  TIMSK |= (1 << OCIE0A);

  // Abilita globalmente gli interrupt
  sei();
}

void loop() {
  // Il loop è vuoto perché la frequenza è già configurata
}


ISR(TIMER0_COMPA_vect) {
  counter++;
  if(counter >= 100){
    counter = 0;
    OCR0A = OCR0A - 1;
    if (OCR0A <= 71) {
      // Disabilita l'interrupt di Compare Match su Timer0
      TIMSK &= ~(1 << OCIE0A);
    }
  }
}

// Controllo velocità per il motore PP dell'EBmetro su scheda
// ATtiny85 (Istruzioni su https://www.etechnophiles.com/digispark-attiny85-with-arduino-tutorial/) e driver TMC2209
// Premere Carica e inserire cavo USB entro 60 sec, quando il compilatore lo richiede

/*
int Frequenza;  // frequenza

void setup() {
  pinMode(4, OUTPUT);     // Direzione motore
  digitalWrite(4, HIGH);  // Direzione standard (Antiorario)

  pinMode(0, OUTPUT);  // Nota x step driver

  for (int i = 1; i <= 12; i++) {
    Frequenza = 384 * i * 2;  // 1 giri/sec * i  -- 768 step/giro
    delay(250);
    tone(0, Frequenza);  // la velocità cresce in 10  passi fino al massimo di 12 giri/s
  }
  tone(0, Frequenza);  // 12 giri/sec,	driver con 16 microstep e motore con 7,5°/step
                       // e 48 step/giro equivale a (Frequenza)9216/768(Step/giro)
}

void loop() {
}

*/
