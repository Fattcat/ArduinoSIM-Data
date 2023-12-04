#include <SoftwareSerial.h>
#include <SD.h>

SoftwareSerial sim800l(7, 8); // Piny RX a TX pre komunikáciu so SIM800L modulom
File simDataFile;

const int ledPin = 13;        // Pin pre LED diódu
const int blinkInterval = 100; // Interval pre blikanie (v milisekundách)

void setup() {
  Serial.begin(9600);
  sim800l.begin(9600);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Vypni LED diódu

  if (SD.begin(10)) {
    Serial.println("SD karta inicializovaná.");
  } else {
    Serial.println("Chyba pri inicializácii SD karty.");
    return;
  }
}

void loop() {
  if (sim800l.available()) {
    String message = sim800l.readString();
    if (message.indexOf("Get_SIM_Data") != -1) {
      if (isSimCardInserted()) {
        digitalWrite(ledPin, HIGH); // Zapni LED diódu (stála svetlo)
        saveSimDataToFile();
      } else {
        blinkLed(); // Blikaj LED diódou
      }
    }
  }
}

bool isSimCardInserted() {
  sim800l.println("AT+CSMINS?"); // Príkaz na kontrolu, či je SIM karta vložená
  delay(2000);
  String response = sim800l.readString();
  return response.indexOf("+CSMINS: 1") != -1;
}

void saveSimDataToFile() {
  simDataFile = SD.open("sim_data.txt", FILE_WRITE);

  if (simDataFile) {
    simDataFile.println("Informácie o SIM karte:");
    simDataFile.println("Číslo SIM karty: " + getSimCardNumber());
    simDataFile.println("Názov operátora: " + getOperatorName());
    simDataFile.println("Frekvencia: " + getFrequency());
    simDataFile.println("Kredit na karte: " + getCredit());
    simDataFile.println("Uložené kontakty:");

    String contacts = getStoredContacts();
    simDataFile.println(contacts);

    simDataFile.close();
    Serial.println("Informácie o SIM karte boli uložené na SD kartu.");
  } else {
    Serial.println("Chyba pri otváraní súboru na SD karte.");
  }
}

void blinkLed() {
  while (!isSimCardInserted()) {
    digitalWrite(ledPin, HIGH); // Zapni LED diódu
    delay(blinkInterval);
    digitalWrite(ledPin, LOW); // Vypni LED diódu
    delay(blinkInterval);
  }
}

String getSimCardNumber() {
  sim800l.println("AT+CICCID"); // Príkaz na získanie ICCID (číslo SIM karty)
  delay(2000);
  return sim800l.readString();
}

String getOperatorName() {
  sim800l.println("AT+COPS?"); // Príkaz na získanie informácií o operátore
  delay(2000);
  return sim800l.readString();
}

String getFrequency() {
  sim800l.println("AT+CBC"); // Príkaz na získanie informácií o frekvencie
  delay(2000);
  return sim800l.readString();
}

String getCredit() {
  sim800l.println("AT+CUSD=1,\"*101#\""); // Príkaz na získanie informácií o kredite
  delay(2000);
  return sim800l.readString();
}

String getStoredContacts() {
  sim800l.println("AT+CPBR=1,250"); // Príkaz na získanie uložených kontaktov (do 250)
  delay(2000);
  return sim800l.readString();
}
