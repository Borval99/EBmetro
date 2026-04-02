#include <Wire.h>

#define SLAVE_ADDRESS 0x50 // Indirizzo I2C del dispositivo slave

// Funzione di setup
void setup() {
  Wire.begin(SLAVE_ADDRESS); // Inizializza il dispositivo come slave con l'indirizzo specificato
  Wire.onReceive(receiveEvent); // Registra la funzione di callback per la ricezione dei dati

  Serial.begin(9600); // Inizializza la comunicazione seriale per il debug
  Serial.println("I2C Slave Ready");
}

// Funzione di loop (vuota in questo caso)
void loop() {
  // Il dispositivo slave attende passivamente che il master invii i dati
}

// Funzione di callback per la ricezione dei dati
void receiveEvent(int byteCount) {
  while (Wire.available()) { // Controlla se ci sono dati disponibili
    char c = Wire.read(); // Legge un byte di dati
    Serial.print(c); // Stampa il byte ricevuto sulla porta seriale per il debug
  }
  Serial.println(); // Nuova linea dopo aver letto tutti i dati
}