#include <Wire.h>
#include <sam.h>

// Dimensione del buffer di dati da inviare
#define BUFFER_SIZE 32
// Buffer di dati da inviare
uint8_t dataBuffer[BUFFER_SIZE] = { /* dati da inviare */ };
// Flag per indicare il completamento del trasferimento DMA
volatile bool dmaTransferComplete = false;

// Puntatore alla sezione del descrittore DMA
DmacDescriptor descriptor_section[1] __attribute__((aligned(16))); 
// Puntatore alla sezione di write-back DMA
DmacDescriptor write_back_section[1] __attribute__((aligned(16))); 

// Funzione per configurare il DMA per la trasmissione I2C
void configure_dma_for_i2c_tx() {
  // Abilita il clock per il DMA Controller
  PM->AHBMASK.reg |= PM_AHBMASK_DMAC;
  PM->APBBMASK.reg |= PM_APBBMASK_DMAC;

  // Ottiene un puntatore al descrittore DMA
  DmacDescriptor *descriptor = &descriptor_section[0];

  // Configura il descrittore DMA
  descriptor->BTCTRL.bit.VALID = true;  // Il descrittore è valido
  descriptor->BTCTRL.bit.EVOSEL = DMAC_BTCTRL_EVOSEL_DISABLE_Val;  // Disabilita l'evento
  descriptor->BTCTRL.bit.BLOCKACT = DMAC_BTCTRL_BLOCKACT_NOACT_Val;  // Nessuna azione al termine del blocco
  descriptor->BTCTRL.bit.BEATSIZE = DMAC_BTCTRL_BEATSIZE_BYTE_Val;  // Dimensione del beat è un byte
  descriptor->BTCTRL.bit.SRCINC = true;  // Incrementa l'indirizzo sorgente
  descriptor->BTCTRL.bit.DSTINC = false; // Non incrementa l'indirizzo destinazione
  descriptor->BTCTRL.bit.STEPSEL = DMAC_BTCTRL_STEPSEL_SRC_Val;  // Seleziona il sorgente per l'incremento
  descriptor->BTCNT.reg = BUFFER_SIZE;  // Numero di beat da trasferire
  descriptor->SRCADDR.reg = (uint32_t)dataBuffer + BUFFER_SIZE;  // Indirizzo sorgente
  descriptor->DSTADDR.reg = (uint32_t)&SERCOM2->I2CM.DATA.reg;  // Indirizzo destinazione
  descriptor->DESCADDR.reg = 0;  // Non c'è un descrittore successivo

  // Seleziona il canale DMA per la configurazione
  DMAC->CHID.reg = DMAC_CHID_ID(0);  // Usa il canale meno significativo disponibile (canale 0)

  // Configura il canale DMA
  DMAC->CHCTRLB.reg = DMAC_CHCTRLB_LVL(0) |  // Livello di priorità del canale
                      DMAC_CHCTRLB_TRIGSRC(SERCOM2_DMAC_ID_TX) |  // Sorgente di trigger
                      DMAC_CHCTRLB_TRIGACT_BEAT;  // Tipo di trigger
  DMAC->CHCTRLA.reg = DMAC_CHCTRLA_ENABLE;  // Abilita il canale DMA

  // Abilita l'interruzione per il completamento del trasferimento del canale
  DMAC->CHINTENSET.reg = DMAC_CHINTENSET_TCMPL;

  // Abilita il controller DMA
  DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xf);

  // Abilita le interruzioni globali
  NVIC_EnableIRQ(DMAC_IRQn);
}

// Gestore dell'interruzione DMA
void DMAC_Handler() {
  // Verifica se l'interruzione è per il completamento del trasferimento del canale 0
  if (DMAC->CHID.reg == 0 && (DMAC->CHINTFLAG.reg & DMAC_CHINTFLAG_TCMPL)) {
    // Pulisce il flag dell'interruzione
    DMAC->CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
    // Segnala che il trasferimento è completato
    dmaTransferComplete = true;
  }
}

// Funzione di setup
void setup() {
  // Inizializza la comunicazione I2C
  Wire.begin();
  // Configura il DMA per la trasmissione I2C
  configure_dma_for_i2c_tx();
}

// Loop principale
void loop() {
  // Avvia il trasferimento DMA
  Wire.beginTransmission(0x50);  // Inizia la trasmissione al dispositivo con indirizzo 0x50
  DMAC->CHID.reg = DMAC_CHID_ID(0);  // Seleziona il canale DMA
  DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;  // Abilita il canale DMA per avviare il trasferimento

  // La CPU può eseguire altre operazioni mentre il DMA gestisce il trasferimento
  // Esegui altre operazioni...

  // Controlla se il trasferimento DMA è completato
  if (dmaTransferComplete) {
    // Conclude la trasmissione I2C
    Wire.endTransmission();
    // Resetta il flag per il prossimo trasferimento
    dmaTransferComplete = false;
    // Stampa un messaggio di completamento del trasferimento
    Serial.println("DMA Transfer Complete");
  }

  // Attesa per esempio, può essere rimosso o modificato in base alle esigenze
  delay(1000);
}
