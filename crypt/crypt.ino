/*
Elias Westesson 
02/03/2025

Detta program är skrivet som en del av ett gymnasiearbete i samarbete med Ystad Gymnasium Österport.

Detta program använder sig av Bluetooth Low Energy (BLE) för att ta emot en text, 
kryptera den med ett Vigenére-baserat chiffer och sedan skicka både den krypterade och dekrypterade texten tillbaka via BLE.

Programmet fungerar enligt följande steg:
1. Startar BLE och annonserar en tjänst med tre egenskaper:
   - `textChar`: Tar emot text från en ansluten BLE-enhet.
   - `encryptChar`: Skickar tillbaka den krypterade texten.
   - `decryptChar`: Skickar tillbaka den dekrypterade texten.
2. När en text tas emot, krypteras den baserat på en nyckel och skickas tillbaka.
3. En ansluten enhet kan läsa den krypterade och dekrypterade texten.
4. LED-lampan på enheten lyser vid anslutning och släcks vid frånkoppling.
*/

#include <ArduinoBLE.h>

// Skapar en BLE-tjänst för att hantera textkommunikation
BLEService textService("181C"); 

// Skapar BLE-egenskaper för att skicka och ta emot text
BLEStringCharacteristic textChar("2A3D", BLEWrite, 50); // Tar emot text (max 50 tecken)
BLEStringCharacteristic encryptChar("12345678-1234-5678-1234-56789abcdef1", BLERead, 50); // Skickar krypterad text
BLEStringCharacteristic decryptChar("12345678-1234-5678-1234-56789abcdef2", BLERead, 50); // Skickar dekrypterad text

// Variabler för lagring av textdata
String receivedText;
String encrypted;
String decrypted;

// Nyckel för kryptering och dekryptering
String key = "SECRET"; // Ingen begränsning på längd

void setup() {
  Serial.begin(9600); // Startar seriell kommunikation

  pinMode(LED_BUILTIN, OUTPUT); // Sätter inbyggda LED som utgång

  // Startar BLE och kontrollerar om det fungerar
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  // Konfigurerar BLE-tjänsten och lägger till egenskaper
  BLE.setLocalName("TextReceiver"); 
  BLE.setAdvertisedService(textService);
  textService.addCharacteristic(textChar);
  textService.addCharacteristic(encryptChar);
  textService.addCharacteristic(decryptChar);
  BLE.addService(textService);

  BLE.advertise(); // Startar annonsering av BLE-enheten
  Serial.println("Waiting for BLE connection...");
}

void loop() {
  BLEDevice central = BLE.central(); // Letar efter en ansluten enhet

  if (central) { // Om en enhet ansluter
    Serial.print("Connected to: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH); // Tänd LED vid anslutning

    while (central.connected()) { // Så länge enheten är ansluten
      if (textChar.written()) { // Kollar om ny text har mottagits
        receivedText = textChar.value();
        Serial.print("Received: ");
        Serial.println(receivedText);

        encrypted = encrypt(receivedText, key); // Kryptera meddelandet
        decrypted = decrypt(receivedText, key); // Dekryptera meddelandet

        Serial.print("enCrypt: ");
        Serial.println(encrypted);
        encryptChar.writeValue(encrypted); // Skicka krypterad text via BLE
        
        Serial.print("deCrypt: ");
        Serial.println(decrypted);
        decryptChar.writeValue(decrypted); // Skicka dekrypterad text via BLE
      }
      delay(500); // Vänta en stund för att minska belastning
    }

    digitalWrite(LED_BUILTIN, LOW); // Släck LED vid frånkoppling
    Serial.println("Disconnected");
  }
}

// Krypteringsfunktion med en variant av Vigenére chiffer
String encrypt(String text, String key) {
  String result = "";
  int keyLength = key.length();

  for (int i = 0; i < text.length(); i++) {
    char c = text[i];
    int shift = (toupper(key[i % keyLength]) - 'A'); // Beräknar skiftvärde från nyckeln

    // Kryptera stora bokstäver
    if (c >= 'A' && c <= 'Z') {
      result += char(((c - 'A' + shift) % 26) + 'A');
    }
    // Kryptera små bokstäver
    else if (c >= 'a' && c <= 'z') {
      result += char(((c - 'a' + shift) % 26) + 'a');
    }
    // Kryptera siffror (0-9)
    else if (c >= '0' && c <= '9') {
      result += char(((c - '0' + shift) % 10) + '0');
    }
    // Lämna andra tecken oförändrade
    else {
      result += c;
    }
  }
  return result;
}

// Dekrypteringsfunktion som använder Vigenére chiffer baklänges
String decrypt(String text, String key) {
  String result = "";
  int keyLength = key.length();

  for (int i = 0; i < text.length(); i++) {
    char c = text[i];
    int shift = (toupper(key[i % keyLength]) - 'A'); // Beräknar skiftvärde från nyckeln
    shift = 26 - shift; // Omvänd skiftning för dekryptering

    // Dekryptera stora bokstäver
    if (c >= 'A' && c <= 'Z') {
      result += char(((c - 'A' + shift) % 26) + 'A');
    }
    // Dekryptera små bokstäver
    else if (c >= 'a' && c <= 'z') {
      result += char(((c - 'a' + shift) % 26) + 'a');
    }
    // Dekryptera siffror (0-9)
    else if (c >= '0' && c <= '9') {
      result += char(((c - '0' + shift) % 10) + '0');
    }
    // Lämna andra tecken oförändrade
    else {
      result += c;
    }
  }
  return result;
}
