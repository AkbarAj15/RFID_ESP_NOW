#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <esp_now.h>

#define SS_PIN 5  // ESP32 pin GPIO5
#define RST_PIN 27 // ESP32 pin GPIO27

MFRC522 rfid(SS_PIN, RST_PIN);
#define MAX_RFID_TAGS 2 // Maksimum jumlah RFID yang dapat disimpan
byte storedRFID[MAX_RFID_TAGS][12]; // Array untuk menyimpan RFID yang terbaca
int numStoredRFID = 0; // Jumlah RFID yang tersimpan saat ini

bool isRegistering = false; // Flag untuk menandai apakah dalam mode pendaftaran

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x40, 0x22, 0xD8, 0xEA, 0x65, 0x2C};//40:22:D8:EA:65:2C

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char uid[12];
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  SPI.begin(); // Initialize SPI bus
  rfid.PCD_Init(); // Initialize MFRC522

   // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  // Read RFID
  Serial.println("No rfid Detected...");
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    bool isNewRFID = true;
    // Check if RFID already exists in the array
    for (int i = 0; i < numStoredRFID; i++) {
      if (memcmp(storedRFID[i], rfid.uid.uidByte, 4) == 0) {
        isNewRFID = false;
         // Send RFID data via ESP-NOW
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    Serial.print("RFID/NFC Tag Type: ");
    Serial.println(rfid.PICC_GetTypeName(piccType));
    char uid[12]; // Character array to store UID
    // Convert UID to a string in hex format
    for (byte i = 0; i < rfid.uid.size; i++) {
      sprintf(uid + (i * 2), "%02X", rfid.uid.uidByte[i]);
    }
    uid[rfid.uid.size * 2] = '\0'; // Add string termination
    Serial.println("UID: " + String(uid));
    strcpy(myData.uid, uid);
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Data sent successfully");
    } else {
      Serial.println("Error sending data");
    }
        break;
      }
    }
    // If RFID is new, register and save it to the array
    if (isNewRFID) {
      if (numStoredRFID < MAX_RFID_TAGS) {
        memcpy(storedRFID[numStoredRFID], rfid.uid.uidByte, 4);
        numStoredRFID++;
        Serial.println("New RFID/NFC tag has been registered.");
      } else {
        Serial.println("Maximum number of RFID tags reached. Cannot register new tag.");
      }
    }
  }
  delay(2000);
}
