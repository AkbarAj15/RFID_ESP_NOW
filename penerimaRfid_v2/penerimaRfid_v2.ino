#include <esp_now.h>
#include <WiFi.h>

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
     char uid[12];
} struct_message;

// Create a struct_message called myData
struct_message myData;
const int r = 18; // ini relay yang berada di pin 18
const int s = 15; // ini relay yang berada di pin 18
const int btn = 5; // ini button yang berada di pin 5


// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("UID: ");
  Serial.println(myData.uid);
  
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
   pinMode(s, OUTPUT);
   pinMode(r,OUTPUT);
   digitalWrite(r, LOW);
   pinMode(btn, INPUT_PULLUP);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  if (strlen(myData.uid) > 0) {
    // Open the solenoid by setting the pin to HIGH
    Serial.print("UID MASUK = ");
    Serial.println(myData.uid);
    digitalWrite(s, HIGH); // selenoid akan terbuka
    delay(2000);
    digitalWrite(s, LOW);
    digitalWrite(r, HIGH);
    Serial.println("relay menyala");
    delay(2000);
    // Turn on the relay by setting the pin to HIGH
  }
  int r1= digitalRead(btn);
  if(r1 == 1){
    digitalWrite(r, LOW);
    digitalWrite(s, HIGH);
    delay(2000);
    digitalWrite(s, LOW);
    Serial.println("button aktif");
    Serial.println("relay mati");
    
    myData.uid[0] = '\0';
    delay(1000);
   }
   myData.uid[0] = '\0';
}
