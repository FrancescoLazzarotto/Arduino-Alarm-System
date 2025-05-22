/*
  import delle librerie necessarie
    -Keypad
      - necessaria per gestire input da tastiere a matrice permette di rilevare quale tasto è stato premuto in modo semplice
    -LiquidCrystal
      - serve per controllare un display LCD utilizzando il controller HD44780
*/
#include <Keypad.h>
#include <LiquidCrystal.h>


/*
  definizione dei pin:
    - trigPin e echoPin per il sensore ultrasonico HC-SR04
    - buzzer per il piezo
    - alarm pin di output per segnalare che il sistema è attivo (led)
*/
#define trigPin 12
#define echoPin 13
#define buzzer 3
#define alarm 2


/*
  definizione pincode:
    - pin_len: lunghezza del codice PIN (6 cifre)
    - pin_codice: array contenente il PIN (inizialmente hard-coded)
    - tmp_codice: array in cui vengono memorizzati i caratteri inseriti dall'utente
    - copertura: array per visualizzare sul LCD un "*" per ogni carattere inserito
*/
#define pin_len 6
char pin_codice[pin_len] = {'1', '2', '3', '4', '5', '6'}; // default al primo avvio
char tmp_codice[pin_len];
char copertura[pin_len + 1];


/*
  stato attuale del PIN in memoria -> verrà sovrascritto quando si cambia PIN
*/
bool pin_modificato = false;


/*
  configurazione del Keypad:
    - definizione righe e colonne
    - key[righe][colonne]: mappa dei tasti fisici
    - righe_pin e colonne_pin: numeri di pin dell'Arduino a cui sono collegate righe e colonne
    - oggetto pad che fornisce il metodo getKey() per leggere un tasto premuto
*/
#define righe 4
#define colonne 4
char key[righe][colonne] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte righe_pin[righe] = {11, 10, 9, 8};
byte colonne_pin[colonne] = {7, 6, 5, 4};
Keypad pad = Keypad(makeKeymap(key), righe_pin, colonne_pin, righe, colonne);


/*
  configurazione schermo LCD:
    - i sei argomenti corrispondono ai pin a cui sono collegati
      rispettivamente RS, E, D4, D5, D6, D7 (qui usiamo A0..A5)
*/
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);


/*
  variabili di controllo:
    - ultimo_char: ultimo carattere letto dal keypad
    - numero_cifre: numero di cifre inserite nel tmp_codice
    - stato_sistema: stato del sistema: true = allarme attivato
    - in_cambio_pin: true se siamo entrati nella modalità “cambia PIN”
    - step_cambio:
        -0 = attesa vecchio PIN,
        -1 = attesa nuovo PIN (prima volta),
        -2 = attesa nuovo PIN (conferma)
*/
char ultimo_char;
byte numero_cifre = 0;
bool stato_sistema = false;
bool in_cambio_pin = false;
byte step_cambio = 0;


/*
  buffer per la modalità “cambia PIN”:
    - old_pin_tmp: per memorizzare temporaneamente la digitazione dell’attuale PIN
    - new_pin_tmp1: primo inserimento del nuovo PIN
    - new_pin_tmp2: secondo inserimento (conferma) del nuovo PIN
    - copertura_cambio: maschera a “*” per la modalità di inserimento PIN durante il cambio
*/
char old_pin_tmp[pin_len];
char new_pin_tmp1[pin_len];
char new_pin_tmp2[pin_len];
char copertura_cambio[pin_len + 1];


/*
  funzione di supporto display -> pulire e scrivere una stringa su una riga dell’LCD
    - si passa riga (0 o 1), text -> testo da visualizzare 
    - stampa 16 spazi vuoti per pulire la riga
    - stampa il testo
*/
void display(byte riga, const char *text) {
  
  lcd.setCursor(0, riga);
  lcd.print("                "); // 16 spazi
  lcd.setCursor(0, riga);
  lcd.print(text);
}


/*
  funzione resetBufferPIN -> resettare i buffer di inserimento PIN per pulire i dati precedenti 
    funzionamento:
      - imposta tutti i caratteri dell'array tmp_codice al valore '.' e copertura a ' '
      - azzera il contatore numero cifre 
*/
void resetBufferPIN() {
  for (byte i = 0; i < pin_len; i++) {
    tmp_codice[i] = '.';
    copertura[i] = ' ';
  }
  copertura[pin_len] = '\0';
  numero_cifre = 0;
}


/*
  funzione verificaPINInserito -> controlla se il codice inserito corrisponde a quello memorizzato
    funzionamento:
      - confronta carattere per carattere 
      - se corrisponde al pin_codice, restituisce true, altrimenti false
*/
bool verificaPINInserito() {
  for (byte i = 0; i < pin_len; i++) {
    if (pin_codice[i] != tmp_codice[i]) {
      return false;
    }
  }
  return true;
}


/*
  funzione verifica -> converte tmp_codice in autorizzazione per armare/spegnere il sistema
    funzionamento: 
      - se il pin è corretto (tramite chiamata alla funzione verificaPINInserito)
        - aggiorna il display e i componenti fisici (buzzer, led)
          - se sistema allarmato 
            - disattiva il sistema e spegne il led e buzzer
          - se sistema non allarmato 
            - attiva il sistema, accende led e buzzer
      - se il pin è errato 
        - mostra messaggio di errore e consente nuovo tentativo
      - infine ripristina schermaza iniziale per ripristino pin 
*/
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


/*
  funzione inserimento -> gestisce l’inserimento di un nuovo carattere per il PIN 
    funzionamento: 
      - inserisce in tmp_codice il carattere contenuto in ultimo_char
      - aggiorna la copertura con un '*'
      - quando il numero raggiunge lunghezza di pin_len 
        - chiama verifica
      - continua a mostrare inserisci pin e la copertura
*/
void inserimento()
{
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


/*
  funzione cancella -> rimuove l’ultimo carattere inserito
    funzionamento: 
      - se ci sono cifre
        - riduce il contatore di 1 e la copertura
      - aggiorna il display per riflettere lo stato attuale del pin
*/
void cancella()
{
  if (numero_cifre >= 1) {
    copertura[numero_cifre - 1] = ' ';
    numero_cifre--;
  }
  lcd.clear();
  display(0, "Inserisci Pin");
  display(1, copertura);
}



/*
  funzione resetBufferCambioPIN -> resettare buffer e maschera usata nelle varie fasi della modalità cambio pin
    funzionamento:
      - pulisce gli array old_pin_tmp, new_pin_tmp1 e new_pin_tmp2 (vecchio, nuovo e conferma del pin)
      -resetta la maschera di copertura del cambio pin 
      - azzera numero cifre
*/
void resetBufferCambioPIN()
{
  for (byte i = 0; i < pin_len; i++) {
    old_pin_tmp[i] = '.';
    new_pin_tmp1[i] = '.';
    new_pin_tmp2[i] = '.';
    copertura_cambio[i] = ' ';
  }
  copertura_cambio[pin_len] = '\0';
  numero_cifre = 0;
}



/*
  funzione gestisciCambioPIN -> gestisce la logica a stati per cambiare il pin (step_cambio = 0,1,2)
    funzionamento:

      - step_cambio = 0 -> sta aspettando la digitazione dell’attuale pin
        - se il numero delle cifre e minore alla lunghezza del pin
          - salva l'ultimo carattere inserito in old_pin_tmp e maschera con '*', aumenta il numero di cifre
          - quando le cifre del pin corrispondono alla lunghezza
            - confronta ogni cifra inserita con quelle memorizzate
            - se corretto passa a stemp_cambio = 1
            - se errato mostra errore, resetta i buffer e riparte da 0

      - step cambio = 1 -> sto digitando il primo nuovo pin
        - se il numero delle cifre è minore della lunghezza del pin
          - salva l'ultimo carattere inserito in new_pin_tmp1 e copre con '*', aumenta il numero di cifre
          - quando le cifre corrispondono alla lunghezza del pin
            - passa allo step 2
            - azzera il numero di cifre, pulisce lo schermo e stampa messaggio di conferma

      - step_cambio = 2 -> digitazione pin di conferma
        - se il numero delle cifre e minore alla lunghezza del pin
          - salva l'ultimo carattere inserito in new_pin_tmp2 e maschera con '*', aumenta il numero di cifre
          - quando le cifre del pin corrispondono alla lunghezza
            - imposta a true la variabile di match dei pin
            - confronta ogni cifra inserita con quelle memorizzate
              - se diverse
                - imposta match e false e interrompe
            - se match rimane true
              - assegno a pin_codice i valori di new_pin_tmp1
              - assegno true a pin modificato
              - pulisco lo schermo, mostro messaggio di successo
            - se match è false
              - messaggio di errore
    - default
      - in caso di valore di step_cambio non previsto resetta tutto
*/
void gestisciCambioPIN() {

  switch (step_cambio) {

  case 0:
    
    if (numero_cifre < pin_len) {
      old_pin_tmp[numero_cifre] = ultimo_char;
      copertura_cambio[numero_cifre] = '*';
      numero_cifre++;

      display(0, "Vecchio PIN:");
      display(1, copertura_cambio);
      if (numero_cifre == pin_len)
      {
        
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
        for (byte i = 0; i < pin_len; i++)
          copertura_cambio[i] = ' ';
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
        }
        else {
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


/*
  setup -> inizializza il sistema all'avvio 
    - imposta la modalità del pin (alarm, buzzer, sensore a ultrasuoni)
    - inizializza la comunicazione seriale
    - inizializza il display lcd
    - stampa la schermata iniziale e resetta il buffer del pin 
*/
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



/*
loop -> ciclo continuo che gestisce input da keypad e sensore ultrasonico
    -funzionamento:
        -lettura tasto
            -leggo il tasto premuto
            -debug nel monitor seriale

            - se il sistema si trova nella modalità di cambio pin (bool in_cambio_pin)
              -chiama la funzione gestisciCambioPIN() per gestire il carattere inserito
              - return per uscire immediatamente da questa funzione e non venga eseguito altro

            - se il sistema non è attivo e l'utente preme A
              - avvio modalità cambio pin -> flag in_cambio_pin in true 
              - inizializza lo step cambio a 0 
              - pulisce i buffer, il display e stampa messaggio Cambiare pin e vecchio pin
              -return per uscire dal loop e evitare altre parti del codice 

            -gestisco i casi (inserimento carattere, cancellazione, verifica)
                - '*' -> verifico codice
                - '#' -> cancello carattere
                - default -> aggiungo carattere

        -gestione del sensore (allarme attivato)
            -se sistema attivo ->  monitorare l’ambiente tramite il sensore per rilevare movimenti
                -prepara il sensore
                    -si assicura che il pin trig sia spento
                    -pausa di 2 millesimi di secondo
                    -manda un impulso al pin trig per 10 millisecondi
                    -si spegne
                -calcola la distanza -> misura quanto tempo ci mette l'onda a tornare indietro incontrando un oggetto
                    -pulseIn(echoPin, HIGH) aspetta che echoPin diventi HIGH
                    -misura quanto tempo rimane HIGH, in microsecondi (andata e ritorno del suono)
                    -distanza -> durata * velocità del suono in aria / 2 (andata e ritorno)
                -se la distanza è maggiore di 0 e minore di 100 (1m)
                    -accende il buzzer
                    -visualizza "Movimento rilevato" e "Inserisci pin"
            -nessun oeggetto rilevato
                    -spegne il buzzer
*/
void loop() {
  ultimo_char = pad.getKey();

  
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
