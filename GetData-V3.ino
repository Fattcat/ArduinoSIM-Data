#include <SoftwareSerial.h>

// RX=10, TX=11 — pripoj SIM800L TX→pin10, RX→pin11 (cez delič!)
SoftwareSerial sim(10, 11);

// ====== NASTAV SVOJ PIN SEM (alebo nechaj prázdne ak nemáš) ======
const String SIM_PIN = "1234";   // "" = žiadny PIN
// =================================================================

void setup() {
  Serial.begin(9600);
  sim.begin(9600);

  Serial.println(F("============================="));
  Serial.println(F("   SIM800L - SIM Info Reader"));
  Serial.println(F("============================="));

  // Čakaj na boot modulu
  Serial.println(F("[*] Čakám na boot SIM800L..."));
  delay(5000);

  // Sync
  sendAT("AT", 1000);
  sendAT("ATE0", 500);         // vypni echo
  sendAT("AT+CMEE=2", 500);   // verbose chyby

  // Skontroluj SIM
  checkAndUnlockSIM();

  // Počkaj na registráciu do siete
  waitForNetwork();

  // Vypíš všetky dostupné údaje
  printAllSIMInfo();
}

void loop() {
  // Nič — jednorazový výpis
  if (sim.available()) Serial.write(sim.read());
}

// ─────────────────────────────────────────────
void checkAndUnlockSIM() {
  Serial.println(F("\n[1] Stav SIM karty (AT+CPIN?)..."));
  String resp = sendATwithResponse("AT+CPIN?", 2000);

  if (resp.indexOf("READY") >= 0) {
    Serial.println(F("    ✓ SIM je odomknutá"));
  } else if (resp.indexOf("SIM PIN") >= 0) {
    Serial.println(F("    ! SIM vyžaduje PIN — odomkýnam..."));
    if (SIM_PIN == "") {
      Serial.println(F("    ✗ PIN nie je nastavený v kóde! Nastav SIM_PIN."));
      return;
    }
    String pinCmd = "AT+CPIN=" + SIM_PIN;
    String pinResp = sendATwithResponse(pinCmd, 5000);
    if (pinResp.indexOf("OK") >= 0) {
      Serial.println(F("    ✓ PIN prijatý, čakám na inicializáciu..."));
      delay(3000);
    } else {
      Serial.println(F("    ✗ Zlý PIN alebo chyba!"));
      Serial.println(pinResp);
    }
  } else if (resp.indexOf("NOT INSERTED") >= 0) {
    Serial.println(F("    ✗ SIM nie je vložená alebo nie je detekovaná!"));
  } else {
    Serial.print(F("    ? Neznámy stav: "));
    Serial.println(resp);
  }
}

// ─────────────────────────────────────────────
void waitForNetwork() {
  Serial.println(F("\n[2] Čakám na registráciu do siete..."));
  for (int i = 0; i < 15; i++) {
    String resp = sendATwithResponse("AT+CREG?", 1000);
    // ,1 = domáca sieť, ,5 = roaming
    if (resp.indexOf(",1") >= 0 || resp.indexOf(",5") >= 0) {
      Serial.println(F("    ✓ Zaregistrovaný v sieti"));
      return;
    }
    Serial.print(F("    ... pokus "));
    Serial.println(i + 1);
    delay(2000);
  }
  Serial.println(F("    ✗ Nepodarilo sa zaregistrovať (timeout)"));
}

// ─────────────────────────────────────────────
void printAllSIMInfo() {
  Serial.println(F("\n===== INFORMÁCIE ZO SIM KARTY =====\n"));

  // IMSI
  Serial.print(F("IMSI              : "));
  String imsi = sendATwithResponse("AT+CIMI", 2000);
  imsi.trim();
  // vyber iba čísla
  String imsiClean = extractNumbers(imsi);
  Serial.println(imsiClean.length() > 0 ? imsiClean : "N/A");

  // ICCID (sériové číslo SIM karty)
  Serial.print(F("ICCID (s/n SIM)   : "));
  String iccid = sendATwithResponse("AT+CCID", 2000);
  Serial.println(extractLine(iccid, "+CCID:"));

  // Telefónne číslo (MSISDN) — nemusí byť vždy dostupné!
  Serial.print(F("Tel. číslo (MSISDN): "));
  String num = sendATwithResponse("AT+CNUM", 3000);
  if (num.indexOf("+CNUM:") >= 0) {
    // formát: +CNUM: "","+421...",145
    int q1 = num.indexOf("\",\"");
    int q2 = num.indexOf("\",", q1 + 3);
    if (q1 >= 0 && q2 >= 0) {
      Serial.println(num.substring(q1 + 3, q2));
    } else {
      Serial.println(F("(číslo skryté operátorom)"));
    }
  } else {
    Serial.println(F("(nedostupné — závisí od operátora)"));
  }

  // Operátor
  Serial.print(F("Operátor          : "));
  String cops = sendATwithResponse("AT+COPS?", 2000);
  Serial.println(extractLine(cops, "+COPS:"));

  // Sila signálu
  Serial.print(F("Sila signálu (RSSI): "));
  String csq = sendATwithResponse("AT+CSQ", 1000);
  String csqVal = extractLine(csq, "+CSQ:");
  csqVal.trim();
  // Preveď na dBm
  int comma = csqVal.indexOf(',');
  if (comma > 0) {
    int rssiRaw = csqVal.substring(0, comma).toInt();
    if (rssiRaw == 99) {
      Serial.println(F("Neznámy (99)"));
    } else {
      int dBm = -113 + (rssiRaw * 2);
      Serial.print(csqVal);
      Serial.print(F("  →  "));
      Serial.print(dBm);
      Serial.println(F(" dBm"));
    }
  } else {
    Serial.println(csqVal);
  }

  // Typ siete / pásmo
  Serial.print(F("Pásmo             : "));
  String band = sendATwithResponse("AT+CBAND?", 1000);
  Serial.println(extractLine(band, "+CBAND:"));

  // Verzia firmware modulu
  Serial.print(F("FW verzia modulu  : "));
  String fw = sendATwithResponse("AT+CGMR", 1000);
  fw.trim(); fw.replace("OK",""); fw.trim();
  Serial.println(fw);

  // IMEI modulu
  Serial.print(F("IMEI modulu       : "));
  String imei = sendATwithResponse("AT+CGSN", 1000);
  imei.trim(); imei.replace("OK",""); imei.trim();
  Serial.println(imei);

  Serial.println(F("\n==================================="));
  Serial.println(F("Hotovo!"));
}

// ─────────────────────────────────────────────
// Pomocné funkcie

void sendAT(const String& cmd, int waitMs) {
  sim.println(cmd);
  delay(waitMs);
  while (sim.available()) sim.read(); // flush
}

String sendATwithResponse(const String& cmd, int waitMs) {
  while (sim.available()) sim.read(); // flush pred poslaním
  sim.println(cmd);
  unsigned long start = millis();
  String resp = "";
  while (millis() - start < (unsigned long)waitMs) {
    while (sim.available()) {
      char c = sim.read();
      resp += c;
    }
  }
  return resp;
}

String extractNumbers(const String& s) {
  String out = "";
  for (int i = 0; i < (int)s.length(); i++) {
    if (isDigit(s[i])) out += s[i];
  }
  return out;
}

String extractLine(const String& s, const String& prefix) {
  int idx = s.indexOf(prefix);
  if (idx < 0) return F("N/A");
  int start = idx + prefix.length();
  // preskočiť medzery
  while (start < (int)s.length() && s[start] == ' ') start++;
  int end = s.indexOf('\n', start);
  if (end < 0) end = s.length();
  String out = s.substring(start, end);
  out.trim();
  return out;
}