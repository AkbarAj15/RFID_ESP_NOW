#include <esp_now.h>
#include <WiFi.h>

const char* ssid = "UTY123"; // WiFi yang digunakan untuk mengirim
const char* password = "07091998"; // Password WiFi

String uid = "";

// Struct untuk menyimpan data yang diterima
typedef struct struct_message {
  char uid[12];
} struct_message;

struct_message myData;

// Fungsi yang akan dieksekusi saat data diterima
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("UID: ");
  Serial.println(myData.uid);

  uid = myData.uid; // Nilai UID akan sesuai dengan ID yang diterima
}

void setup() {
  // Inisialisasi Serial Monitor
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  // Inisialisasi ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Setelah ESP-NOW berhasil diinisialisasi, daftarkan callback untuk menerima data
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (!uid.isEmpty()) {
    Serial.println("UID Ada");

    // Lakukan sesuatu dengan UID yang diterima
    // Misalnya, aktifkan selenoid atau relay
    // digitalWrite(s, HIGH);

    // Setelah selesai, kosongkan nilai UID
    uid = "";
  }
}
