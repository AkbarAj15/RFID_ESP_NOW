#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <esp_now.h>

#define SS_PIN 5  // ESP32 pin GPIO5
#define RST_PIN 27 // ESP32 pin GPIO27

MFRC522 rfid(SS_PIN, RST_PIN);

// Replace with the MAC address of the receiving ESP32
uint8_t broadcastAddress[] = {0x30, 0xC6, 0xF7, 0x23, 0x31, 0x90}; // 30:C6:F7:23:31:90
const char* ssid = "UTY123"; // WiFi yang digunakan untuk mengirim
const char* password = "07091998"; // Password WiFi

// Struct for sending data to the receiving ESP32
typedef struct struct_message {
  char uid[12]; // Menggunakan array karakter untuk UID
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  SPI.begin(); // Initialize SPI bus
  rfid.PCD_Init(); // Initialize MFRC522

  WiFi.begin(ssid, password); // Connect to WiFi

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
  
  
}

void loop() {
  Serial.println("No RFID Detected...");
  if (rfid.PICC_IsNewCardPresent()) { // New tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been read
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));

      char uid[12]; // Menggunakan array karakter untuk UID

      // Convert UID to a string in hex format
      for (byte i = 0; i < rfid.uid.size; i++) {
        sprintf(uid + (i * 2), "%02X", rfid.uid.uidByte[i]);
      }
      uid[rfid.uid.size * 2] = '\0'; // Menambahkan terminasi string

      Serial.println("UID: " + String(uid));

      strcpy(myData.uid, uid);
//      rfid.PICC_HaltA(); // Halt PICC
//      rfid.PCD_StopCrypto1(); // Stop encryption on PCD

      // Send UID data to the receiving ESP32
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
      if (result == ESP_OK) {
        Serial.println("Sent with success");
      } else {
        Serial.println("Error sending the data");
      }
    }
  }
  
  delay(2000);
}
