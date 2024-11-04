#include <Arduino.h>
#include <string.h>

#define LED_ROSU 6
#define LED_VERDE 5
#define LED_ALBASTRU 4
#define BUTON_START 3
#define BUTON_DIFICULTATE 2
#define NUMAR_MAXIM_CUVINTE 15

// Funcțiile pentru gestionarea butoanelor și rundelor
void toggleButonStart();
void schimbaDificultate();
void incepeJoc();
void executaRunda();

/**
 * Variabile pentru gestionarea stării și dificultății jocului
 */
bool jocInDesfasurare = false;       // Indică dacă jocul este activ
bool stareButonStart = true;         // Starea curentă a butonului de start
int intervalAfisareCuvinte = 10000;  // Intervalul de timp între cuvinte, se ajustează pe baza dificultății
bool rundaInceputa = false;          // Indică dacă o rundă este în desfășurare
int timpCurent = millis();           // Timpul curent folosit pentru verificări de temporizare
int indexCuvantCurent = 0;           // Indexul cuvântului curent în dicționar
int timpUltimaActualizare = timpCurent; // Timpul ultimei actualizări
int timpUltimCuvant = timpCurent;    // Timpul ultimei afișări de cuvânt
bool esteGresit = false;             // Indică dacă a fost tastată o literă greșită
int nivelDificultate = 1;            // Nivelul de dificultate (1 = ușor, 2 = mediu, 3 = greu)
bool dificultateModificata = false;  // Indică dacă dificultatea a fost schimbată
unsigned int indexLiteraCurenta = 0; // Indexul literei curente într-un cuvânt
int scor = 0;                        // Scorul jucătorului
bool selectareDificultate = false;   // Indică dacă jucătorul selectează dificultatea

// Dicționarul cuvintelor pentru joc
const char* dictionar[NUMAR_MAXIM_CUVINTE] = {
    "abandon", "abilitate", "abstract", "accesibil", "activitate",
    "adaptabil", "ambiguu", "analogie", "antagonist", "anxietate",
    "baza", "beneficiu", "biblioteca", "binefacere", "blocaj",
};

/**
 *  Schimbă starea butonului de start la fiecare apăsare
 * 
 * Această funcție este apelată în cadrul unei întreruperi care detectează o
 * apăsare pe butonul de start, alternând între stările de ON și OFF.
 */
void toggleButonStart() {
  stareButonStart = !stareButonStart;
}

/**
 *  Crește dificultatea jocului și ciclizează între nivelurile 1-3
 * 
 * Când este apelată, funcția crește dificultatea cu 1. Dacă nivelul depășește 3,
 * dificultatea revine la 1, ciclizând prin nivele: ușor, mediu, greu.
 */
void schimbaDificultate() {
  dificultateModificata = true;
  nivelDificultate++;
  if (nivelDificultate > 3) nivelDificultate = 1;
}

/**
 *  Inițializează jocul și afișează o numărătoare inversă de start
 * 
 * Setează `jocInDesfasurare` pe true și începe o numărătoare inversă de 3 secunde,
 * cu LED-urile clipind pentru a indica pornirea jocului. Amestecă cuvintele din dicționar
 * pentru a le afișa într-o ordine aleatorie pe parcursul jocului.
 */
void incepeJoc() {
  if (!jocInDesfasurare) {
    jocInDesfasurare = true;
    timpCurent = millis();
    timpUltimaActualizare = timpCurent;
    int timpUltimSecundar = timpCurent;
    
    // LED-urile clipesc pentru numărătoarea inversă
    digitalWrite(LED_ROSU, HIGH);
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_ALBASTRU, HIGH);
    char numar = '2';
    Serial.println("3\n");

    while (timpCurent - timpUltimaActualizare < 3000) {
      timpCurent = millis();
      if (timpCurent - timpUltimSecundar >= 1000 && numar != '0') {
        Serial.print(numar);
        Serial.println("\n");
        numar--;
        timpUltimSecundar = timpCurent;
      }
      
      // LED-urile alternează între ON și OFF
      digitalWrite(LED_ROSU, !digitalRead(LED_ROSU));
      digitalWrite(LED_VERDE, !digitalRead(LED_VERDE));
      digitalWrite(LED_ALBASTRU, !digitalRead(LED_ALBASTRU));
      delay(500);
    }

    // Setare LED-uri și amestecarea cuvintelor din dicționar
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_ROSU, LOW);
    digitalWrite(LED_ALBASTRU, LOW);
    randomSeed(millis());

    // Amestecare dicționar cu metoda Fisher-Yates
    for (int i = NUMAR_MAXIM_CUVINTE - 1; i > 0; i--) {
      int j = random(i + 1);
      const char* temp = dictionar[i];
      dictionar[i] = dictionar[j];
      dictionar[j] = temp;
    }

    indexCuvantCurent = 0;
    scor = 0;
  }
}

/**
 *  Execută o rundă în care jucătorul tastează literele cuvântului afișat
 * 
 * Funcția verifică dacă jucătorul introduce corect literele cuvântului afișat. Dacă 
 * un cuvânt este complet, jocul trece la următorul. Dacă jucătorul tastează greșit, 
 * variabila `esteGresit` se setează la true, și LED-ul roșu se aprinde.
 */
void executaRunda() {
  if (rundaInceputa) {
    if (timpCurent - timpUltimaActualizare < 30000) {
      timpCurent = millis();
      bool sarireCuvant = false;

      if (Serial.available() > 0) {
        char litera = Serial.read();
        
        // Verifică dacă litera tastată este corectă
        if (litera == 8) { // ASCII pentru backspace
          esteGresit = false;
          Serial.println("Backspace apăsat");
        }
        else if (litera == dictionar[indexCuvantCurent - 1][indexLiteraCurenta] && !esteGresit) {
          Serial.println("Litera corectă: " + String(litera));
          indexLiteraCurenta++;

          // Verifică dacă întregul cuvânt a fost tastat corect
          if (indexLiteraCurenta == strlen(dictionar[indexCuvantCurent - 1])) {
            Serial.println("Cuvânt complet: " + String(dictionar[indexCuvantCurent - 1]));
            scor++;
            sarireCuvant = true;
            indexLiteraCurenta = 0;
            indexCuvantCurent++;
            if (indexCuvantCurent >= NUMAR_MAXIM_CUVINTE) indexCuvantCurent = 0;
            timpUltimCuvant = timpCurent;
          }
        } else {
          Serial.println("Litera greșită: " + String(litera) + " Așteptat: " + String(dictionar[indexCuvantCurent - 1][indexLiteraCurenta]));
          esteGresit = true;
        }
      }

      if (indexCuvantCurent == 0) {
        Serial.println(dictionar[indexCuvantCurent]);
        indexCuvantCurent++;
        timpUltimCuvant = timpCurent;
      } else if (timpCurent - timpUltimCuvant >= intervalAfisareCuvinte || sarireCuvant) {
        Serial.println(dictionar[indexCuvantCurent]);
        timpUltimCuvant = timpCurent;
        indexCuvantCurent++;
        indexLiteraCurenta = 0;
        esteGresit = false;
      }
    } else {
      rundaInceputa = false;
      esteGresit = false;
      Serial.println("Runda încheiată");
      Serial.println("Scor: " + String(scor));
      timpUltimCuvant = timpCurent;
    }
  }
}

/**
 *  Configurarea inițială a componentelor de hardware și întreruperilor
 * 
 * Această funcție setează pinii pentru LED-uri și butoane, configurează rata de baud
 * pentru serial și atașează întreruperi pentru butoanele de start și dificultate.
 */
void setup() {
  pinMode(LED_ROSU, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ALBASTRU, OUTPUT);
  pinMode(BUTON_START, INPUT_PULLUP);
  pinMode(BUTON_DIFICULTATE, INPUT_PULLUP);
  Serial.begin(28800);
  attachInterrupt(digitalPinToInterrupt(BUTON_START), toggleButonStart, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTON_DIFICULTATE), schimbaDificultate, FALLING);
}

/**
 *  Bucla principală care gestionează starea jocului și afișarea rundelor
 * 
 * În funcție de starea jocului (`jocInDesfasurare`), bucla principală verifică 
 * starea butonului de start, controlează LED-urile și gestionează nivelurile de dificultate.
 */
void loop() {
  timpCurent = millis();
  if (jocInDesfasurare) {
    if (stareButonStart == LOW) {
      Serial.println("Buton de oprire apăsat");
      stareButonStart = true;
      jocInDesfasurare = false;
      digitalWrite(LED_ROSU, HIGH);
      digitalWrite(LED_VERDE, HIGH);
      digitalWrite(LED_ALBASTRU, HIGH);
      delay(500);
    }

    if (rundaInceputa) executaRunda();

    // Control LED-uri pentru corectitudine litere
    if (esteGresit) {
      digitalWrite(LED_ROSU, HIGH);
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_ALBASTRU, LOW);
    } else {
      digitalWrite(LED_ROSU, LOW);
      digitalWrite(LED_VERDE, HIGH);
      digitalWrite(LED_ALBASTRU, LOW);
    }
  } else {
    if (!selectareDificultate) {
      Serial.println("Selectați dificultatea: ");
      selectareDificultate = true;
    }

    if (dificultateModificata) {
      delay(200);
      dificultateModificata = false;
      switch (nivelDificultate) {
        case 1:
          intervalAfisareCuvinte = 10000;
          Serial.println("Ușor");
          break;
        case 2:
          intervalAfisareCuvinte = 5000;
          Serial.println("Mediu");
          break;
        case 3:
          intervalAfisareCuvinte = 3000;
          Serial.println("Greu");
          break;
      }
    }

    if (stareButonStart == LOW && !jocInDesfasurare) {
      Serial.println("Buton Start apăsat");
      stareButonStart = true;
      timpUltimaActualizare = timpCurent;
      timpUltimCuvant = timpCurent;
      rundaInceputa = true;
      incepeJoc();
      delay(500);
    }

    // LED-uri aprinse în mod static când jocul nu este activ
    digitalWrite(LED_ROSU, HIGH);
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_ALBASTRU, HIGH);
  }
}
