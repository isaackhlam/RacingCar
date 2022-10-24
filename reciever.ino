##RECIVER

#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xF0, 0x08, 0xD1, 0xC7, 0xAA, 0xF8};

// Variable to store if sending data was successful
String success;
typedef struct car_data {
    double speed;
    double angle;
} car_data;
car_data RemoteDatas;
car_data incomingReadings;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.printf("%.5f,%.5f\n", incomingReadings.speed,incomingReadings.angle);
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}


 
void loop() {

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &RemoteDatas, sizeof(RemoteDatas));

  delay(10);
}
