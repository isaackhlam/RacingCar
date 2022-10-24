##SENDER

#include <esp_now.h>
#include <WiFi.h>
int VRx = 34;
int VRy = 35;
int SW = 33;
int mapX = 0, mapY = 0, SW_state = 0;
double x = 0, y = 0, speed = 0, angle = 0, defaultx = 0, defaulty = 0;

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xB8, 0xD6, 0x1A, 0xB3, 0x16, 0x78};

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
  Serial.print("\nLast Packet Send Status: ");
  if(status != ESP_NOW_SEND_SUCCESS) Serial.println( "Delivery Fail");
  success = status == 0 ? "Delivery Success :)" : "Delivery Fail :(";
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  defaultx = analogRead(VRx);
  defaulty = analogRead(VRy);

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
}


 
void loop() {

  x = map((int)(analogRead(VRx) - defaultx) / 32 * 32, -1900, 1900, -1000, 1000);
  y = map((int)(analogRead(VRy) - defaulty) / 32 * 32, -1900, 1900, -1000, 1000);
  x = abs(x) > 1000 ? x / abs(x) * 1000 : x;
  y = abs(y) > 1000 ? y / abs(y) * 1000 : y;
  RemoteDatas.angle = atan(y / (x == 0 ? 0.00001 : x)) * 180 / PI;
  RemoteDatas.speed = (angle > 0 ? 1 : -1) * sqrt(x * x + y * y) / sqrt(2000000);

  Serial.printf("speed: %.5f, angle: %.5f\n",RemoteDatas.angle,RemoteDatas.speed);

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &RemoteDatas, sizeof(RemoteDatas));
  if (result != ESP_OK) 
    Serial.println("Error sending the data");
  
  delay(10);
}
