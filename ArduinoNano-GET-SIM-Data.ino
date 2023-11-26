#include <SoftwareSerial.h>

SoftwareSerial sim800l(7, 8);

void setup() {
  Serial.begin(9600);
  sim800l.begin(9600);
  delay(2000);
  
  Serial.println("Inicializacia SIM800L...");
  delay(2000);

  getSimCardInfo();
}

void loop() {
  // Vaša ďalšia logika môže ísť sem
}

void getSimCardInfo() {
  // Získanie IMSI čísla
  sim800l.println("AT+CIMI");
  delay(1000);
  printResponse("IMSI");

  // Získanie ICCID čísla
  sim800l.println("AT+CCID");
  delay(1000);
  printResponse("ICCID");

  // Získanie Názvu operátora
  sim800l.println("AT+CSPN?");
  delay(1000);
  printResponse("Názov operátora");

  // Získanie telefónneho čísla
  sim800l.println("AT+CNUM");
  delay(1000);
  printResponse("Telefónne číslo");

  // Získanie aktuálnej polohy - predpokladáme, že modul podporuje túto funkciu
  sim800l.println("AT+CIPGSMLOC=1,1");
  delay(1000);
  printResponse("Aktuálna poloha");

  // Získanie kreditu - príklad s USSD príkazom
  sim800l.println("AT+CUSD=1,\"*101#\"");
  delay(1000);
  printResponse("Kredit");

  // Získanie uložených čísel - predpokladáme, že modul podporuje túto funkciu
  sim800l.println("AT+CPBR=1,999");
  delay(1000);
  printResponse("Uložené čísla");

  // Získanie výrobného čísla alebo IDENTIFIKÁTORA SIM karty - predpokladáme, že modul podporuje túto funkciu
  sim800l.println("AT+GSN");
  delay(1000);
  printResponse("Výrobné číslo");

}

void printResponse(const char* prefix) {
  while (sim800l.available()) {
    char c = sim800l.read();
    Serial.print(c);

    if (c == '\n') {
      // Po každom riadku výstupu pridá prefix
      Serial.print(prefix);
      Serial.print(" : ");
    }
  }
}
