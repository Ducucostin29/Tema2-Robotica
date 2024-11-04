# Tema2-Robotica
În această temă veți crea un joc asemanator cu TypeRacer

Jocul este în repaus. LED-ul RGB are culoarea albă.
Se alege dificultatea jocului folosind butonul de dificultate, iar în terminal va apărea “Easy/Medium/Hard mode on!”.
Se apasă butonul de start/stop.
LED-ul clipește timp de 3 secunde, iar în terminal se va afișa numărătoarea înversă: 3, 2, 1.
LED-ul devine verde și încep să apară cuvinte de tastat.
La tastarea corectă, următorul cuvânt apare imediat. Dacă nu se tasteaza cuvântul în timpul stabilit de dificultate, va apărea un nou cuvânt.
O greșeală face LED-ul să devină roșu. Pentru a corecta cuvântul, se va folosi tasta BackSpace.
Dupa 30 de secunde, runda se termină, iar în terminal se va afișa scorul: numărul total de cuvinte scrise corect.
Jocul se poate opri oricând cu butonul de start/stop.

## Componentele utilizate
- 1x LED RGB (pentru starea de liber sau ocupat)
- 2x Butoane (pentru start încărcare și stop încărcare)
- 5x Rezistoare (7x 220/330ohm, 2x 2K)
- Breadboard
- Linii de legătură
- Placa Arduino Uno R3 ATmega328P/PA

## Descriere

### Setup-ul fizic

# void toggleButonStart()

Această funcție este apelată în cadrul unei întreruperi care detectează o apăsare pe butonul de start, alternând între stările de ON și OFF.
```
void toggleButonStart() {
  stareButonStart = !stareButonStart;
}
```
# void schimbaDificultate()

 Când este apelată, funcția crește dificultatea cu 1. Dacă nivelul depășește 3, dificultatea revine la 1, ciclizând prin nivele: ușor, mediu, greu.

```
void schimbaDificultate() {
  dificultateModificata = true;
  nivelDificultate++;
  if (nivelDificultate > 3) nivelDificultate = 1;
}
```
# void incepeJoc()

Setează `jocInDesfasurare` pe true și începe o numărătoare inversă de 3 secunde, cu LED-urile clipind pentru a indica pornirea jocului. Amestecă cuvintele din dicționar pentru a le afișa într-o ordine aleatorie pe parcursul jocului.

```
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
```

# void executaRunda()

Funcția verifică dacă jucătorul introduce corect literele cuvântului afișat. Dacă  un cuvânt este complet, jocul trece la următorul. Dacă jucătorul tastează greșit, variabila `esteGresit` se setează la true, și LED-ul roșu se aprinde.

```
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
```

## Poze ale setup-ului fizic

## Video cu funcționalitatea montajului fizic

## Schema electrică (TinkerCAD)
- Conectarea LED-ului RGB la pinii 4, 5 și 6.
- Conectarea butonului de start/stop la pinul 3 și a butonului de dificultate la pinul 2.
- Utilizarea rezistențelor pentru protecția LED-urilor și pentru stabilizarea butoanelor.
![image](https://github.com/user-attachments/assets/c1c928e0-20b4-4a91-a511-1710d7bc57bf)
![image](https://github.com/user-attachments/assets/76d42988-ca8d-46ec-88a9-ba43b72f44e1)




