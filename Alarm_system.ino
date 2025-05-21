#include <Keypad.h>
#include <LiquidCrystal.h>

/*---------------------------------------------------------------
  definizione dei pin:
    - trigPin e echoPin per il sensore ultrasonico HC-SR04
    - buzzer per il piezo
    - alarm pin di output per segnalare che il sistema è attivo (led)
---------------------------------------------------------------*/
#define trigPin 12
#define echoPin 13
#define buzzer   3
#define alarm    2

/*---------------------------------------------------------------
  definizione pincode:
    - pin_len: lunghezza del codice PIN (6 cifre)
    - pin_codice: array contenente il PIN (inizialmente hard-coded)
    - tmp_codice: array in cui vengono memorizzati i caratteri inseriti dall'utente
    - copertura: array per visualizzare sul LCD un "*" per ogni carattere inserito
---------------------------------------------------------------*/
#define pin_len 6
char pin_codice[pin_len] = {'1', '2', '3', '4', '5', '6'};  // default al primo avvio
char tmp_codice[pin_len];
char copertura[pin_len + 1];

/*
  stato attuale del PIN in memoria (RAM) 
  -> verrà sovrascritto quando si cambia PIN
*/
bool pin_modificato = false;

/*---------------------------------------------------------------
  configurazione del Keypad:
    - definizione righe e colonne
    - key[righe][colonne]: mappa dei tasti fisici
    - righe_pin e colonne_pin: numeri di pin dell'Arduino a cui sono collegate righe e colonne
    - oggetto pad che fornisce il metodo getKey() per leggere un tasto premuto
---------------------------------------------------------------*/
#define righe   4
#define colonne 4
char key[righe][colonne] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte righe_pin[righe]   = {11, 10, 9, 8};
byte colonne_pin[colonne] = {7, 6, 5, 4};
Keypad pad = Keypad(makeKeymap(key), righe_pin, colonne_pin, righe, colonne);

/*---------------------------------------------------------------
  configurazione schermo LCD:
    - i sei argomenti corrispondono ai pin a cui sono collegati
      rispettivamente RS, E, D4, D5, D6, D7 (qui usiamo A0..A5)
---------------------------------------------------------------*/
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

/*---------------------------------------------------------------
  variabili di controllo:
    - ultimo_char: ultimo carattere letto dal keypad
    - numero_cifre: numero di cifre inserite nel tmp_codice
    - stato_sistema: stato del sistema: true = allarme attivato
    - in_cambio_pin: true se siamo entrati nella modalità “cambia PIN”
    - step_cambio: 0 = attesa vecchio PIN, 
                  1 = attesa nuovo PIN (prima volta), 
                  2 = attesa nuovo PIN (conferma)
---------------------------------------------------------------*/
char   ultimo_char;
byte   numero_cifre = 0;
bool   stato_sistema = false;
bool   in_cambio_pin = false;
byte   step_cambio = 0;

/*---------------------------------------------------------------
  buffer per la modalità “cambia PIN”:
    - old_pin_tmp: per memorizzare temporaneamente la digitazione dell’attuale PIN
    - new_pin_tmp1: primo inserimento del nuovo PIN
    - new_pin_tmp2: secondo inserimento (conferma) del nuovo PIN
    - copertura_cambio: maschera a “*” per la modalità di inserimento PIN durante il cambio
---------------------------------------------------------------*/
char old_pin_tmp[pin_len];
char new_pin_tmp1[pin_len];
char new_pin_tmp2[pin_len];
char copertura_cambio[pin_len + 1];

/*---------------------------------------------------------------
  funzione di supporto display -> pulire e scrivere una stringa su una riga dell’LCD
    - param: riga (0 o 1), text -> testo da visualizzare (fino a 16 caratteri)
    - prima cancella la riga (16 spazi), poi riscrive il testo
---------------------------------------------------------------*/
void display(byte riga, const char *text) {
  lcd.setCursor(0, riga);
  lcd.print("                "); // 16 spazi
  lcd.setCursor(0, riga);
  lcd.print(text);
}

/*---------------------------------------------------------------
  funzione privata per resettare i buffer di inserimento PIN “normale”
---------------------------------------------------------------*/
void resetBufferPIN() {
  for (byte i = 0; i < pin_len; i++) {
    tmp_codice[i] = '.';
    copertura[i] = ' ';
  }
  copertura[pin_len] = '\0';
  numero_cifre = 0;
}

/*---------------------------------------------------------------
  funzione verificaPINInserito -> controlla il tmp_codice (lunghezza pin_len)
  - se corrisponde al pin_codice, restituisce true, altrimenti false
---------------------------------------------------------------*/
bool verificaPINInserito() {
  for (byte i = 0; i < pin_len; i++) {
    if (pin_codice[i] != tmp_codice[i]) {
      return false;
    }
  }
  return true;
}

/*---------------------------------------------------------------
  funzione verifica -> converte tmp_codice in autorizzazione 
  per armare/spegnere il sistema
---------------------------------------------------------------*/
void verifica() {
  bool corretto = verificaPINInserito();
  lcd.clear();

  if (corretto) {
    
    stato_sistema = !stato_sistema;
    if (stato_sistema) {
      display(0, "Pin corretto");
      display(1, "Allarme Attivo");
      digitalWrite(alarm, HIGH);
    }
    else {
      display(0, "Pin corretto");
      display(1, "Allarme Disattivo");
      noTone(buzzer);
      digitalWrite(alarm, LOW);
    }
    delay(1000);
  }
  else {
    display(0, "Pin Errato");
    display(1, "Riprova");
    delay(1000);
  }

  
  lcd.clear();
  display(0, "Inserisci Pin");
  resetBufferPIN();
}

/*---------------------------------------------------------------
  funzione inserimento -> gestisce l’inserimento di un nuovo 
  carattere per il PIN “normale”
---------------------------------------------------------------*/
void inserimento() {
  tmp_codice[numero_cifre] = ultimo_char;
  copertura[numero_cifre] = '*';
  numero_cifre++;

  if (numero_cifre == pin_len) {
    verifica();
  }
  else {
    display(0, "Inserisci Pin");
    display(1, copertura);
  }
}

/*---------------------------------------------------------------
  funzione cancella -> rimuove l’ultimo carattere inserito
  nel PIN “normale”
---------------------------------------------------------------*/
void cancella() {
  if (numero_cifre >= 1) {
    copertura[numero_cifre - 1] = ' ';
    numero_cifre--;
  }
  lcd.clear();
  display(0, "Inserisci Pin");
  display(1, copertura);
}

/*---------------------------------------------------------------
  funzione resetBufferCambioPIN -> resettare buffer e maschera 
  usata nelle varie fasi della modalità cambio PIN
---------------------------------------------------------------*/
void resetBufferCambioPIN() {
  for (byte i = 0; i < pin_len; i++) {
    old_pin_tmp[i]     = '.';
    new_pin_tmp1[i]    = '.';
    new_pin_tmp2[i]    = '.';
    copertura_cambio[i] = ' ';
  }
  copertura_cambio[pin_len] = '\0';
  numero_cifre = 0;
}

/*---------------------------------------------------------------
  funzione gestisciCambioPIN -> gestisce la logica a stati per
  cambiare il PIN (step_cambio = 0,1,2)
---------------------------------------------------------------*/
void gestisciCambioPIN() {
  // step_cambio = 0 -> sto aspettando la digitazione dell’attuale PIN
  // step_cambio = 1 -> sto digitando il primo nuovo PIN
  // step_cambio = 2 -> sto digitando la conferma del nuovo PIN

  switch (step_cambio) {
    case 0:
      
      if (numero_cifre < pin_len) {
        old_pin_tmp[numero_cifre] = ultimo_char;
        copertura_cambio[numero_cifre] = '*';
        numero_cifre++;
        display(0, "Vecchio PIN:");
        display(1, copertura_cambio);
        if (numero_cifre == pin_len) {
          
          bool old_ok = true;
          for (byte i = 0; i < pin_len; i++) {
            if (old_pin_tmp[i] != pin_codice[i]) {
              old_ok = false;
              break;
            }
          }
          if (old_ok) {
            
            step_cambio = 1;
            resetBufferCambioPIN();
            lcd.clear();
            display(0, "Nuovo PIN:");
            display(1, copertura_cambio);
          }
          else {
            lcd.clear();
            display(0, "Vecchio PIN Errato");
            display(1, "Riprova");
            delay(1500);
            
            resetBufferCambioPIN();
            lcd.clear();
            display(0, "Cambiare PIN");
            display(1, "Vecchio PIN:");
          }
        }
      }
      break;

        case 1:
      
      if (numero_cifre < pin_len) {
        new_pin_tmp1[numero_cifre] = ultimo_char;
        copertura_cambio[numero_cifre] = '*';
        numero_cifre++;
        display(0, "Nuovo PIN:");
        display(1, copertura_cambio);
        if (numero_cifre == pin_len) {
          step_cambio = 2;
          numero_cifre = 0;
          for (byte i = 0; i < pin_len; i++) copertura_cambio[i] = ' ';
          lcd.clear();
          display(0, "Conferma PIN:");
          display(1, copertura_cambio);
        }
      }
      break;

    case 2:
     
      if (numero_cifre < pin_len) {
        new_pin_tmp2[numero_cifre] = ultimo_char;
        copertura_cambio[numero_cifre] = '*';
        numero_cifre++;
        display(0, "Conferma PIN:");
        display(1, copertura_cambio);
        if (numero_cifre == pin_len) {
          bool match = true;
          for (byte i = 0; i < pin_len; i++) {
            if (new_pin_tmp1[i] != new_pin_tmp2[i]) {
              match = false;
              break;
            }
          }
          if (match) {
            for (byte i = 0; i < pin_len; i++) {
              pin_codice[i] = new_pin_tmp1[i]; 
            }
            pin_modificato = true;
            lcd.clear();
            display(0, "PIN cambiato!");
            display(1, "Con successo");
            delay(1500);
          } else {
            lcd.clear();
            display(0, "PIN non coincide");
            display(1, "Riprova tutto");
            delay(1500);
          }
          
          in_cambio_pin = false;
          step_cambio = 0;
          resetBufferCambioPIN();
          lcd.clear();
          display(0, "Inserisci Pin");
        }
      }
      break;
    default:
     
      in_cambio_pin = false;
      step_cambio = 0;
      resetBufferPIN();
      lcd.clear();
      display(0, "Inserisci Pin");
      break;
  }
}

void setup() {
  lcd.begin(16, 2);
  pinMode(alarm, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);

  
  display(0, "Inserisci Pin");
  resetBufferPIN();
}

void loop() {
  ultimo_char = pad.getKey();
  
  /***************************************************************
   * GESTIONE TASTI PREMUTI
   ***************************************************************/
  if (ultimo_char) {
    
    if (stato_sistema) {
      Serial.print("Tasto premuto (allarme attivo): ");
      Serial.println(ultimo_char);
    }

    
    if (in_cambio_pin) {
      gestisciCambioPIN();
      return; 
    }

    
    if (!stato_sistema && ultimo_char == 'A') {
      
      in_cambio_pin = true;
      step_cambio = 0;
      resetBufferCambioPIN();
      lcd.clear();
      display(0, "Cambiare PIN");
      display(1, "Vecchio PIN:");
      return; 
    }

    
    switch (ultimo_char) {
      case '*':
        verifica();
        break;

      case '#':
        cancella();
        break;

      default:
        inserimento();
        break;
    }
  }

  /***************************************************************
   * GESTIONE SENSORE AD ULTRASUONI (SE ALARME ATTIVO)
   ***************************************************************/
  if (stato_sistema) {
    
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long durata = pulseIn(echoPin, HIGH);
    int distanza = durata * 0.034 / 2;  

    if (distanza > 0 && distanza < 100) {
      
      if (numero_cifre == 0 && !in_cambio_pin) {
        display(0, "Movimento rilevato");
        display(1, "Inserisci Pin");
      }
      tone(buzzer, 1000);
    }
    else {
      noTone(buzzer);
      
    }

    delay(300);
  }
  else {
   
    noTone(buzzer);
    
  }
}
