//SENDER
#include <esp_now.h>
#include <WiFi.h>

//Define Pin Port and Preference
#define VRx 34
#define VRy 35
#define SW 33
#define WIFI_CHANNEL 0
#define MOTOR_MAX_SPEED 255
#define MOTOR_MIN_SPEED 60

//Set send target and send data structure
esp_now_peer_info_t peerInfo;
uint8_t sendTargetMAC[] = { 0x10, 0x52, 0x1C, 0x5C, 0xD1, 0x74 };
//uint8_t sendTargetMAC[] = {0xF0, 0x08, 0xD1, 0xC7, 0xAA, 0xF8};
struct car_data {
  int speed;
  int angle;
};
int errorCount = 0;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) return;
  Serial.print("FAILED:");
  Serial.println(WiFi.macAddress());
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  //get receiver boot time
  int receiveData;
  memcpy(&receiveData, incomingData, sizeof(receiveData));
  Serial.printf("Recieve Time: %d\n", receiveData);
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  //Init Wifi
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    for(;;);
  }

  //Create RSP Channel
  memcpy(peerInfo.peer_addr, sendTargetMAC, 6);
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    for(;;);
  }

  //Set Up Event Listener on Send and Receive
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}



void loop() {
  //map read data to -MOTOR_MAX_SPEED to MOTOR_MAX_SPEED
  double x = map(analogRead(VRx) - 1975, -1960, 2160, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
  double y = map(analogRead(VRy) - 1975, -1960, 2160, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);

  //Limit output range (<= 60 -> 0, >MOTOR_MAX_SPEED -> MOTOR_MAX_SPEED)
  x = abs(x) < MOTOR_MIN_SPEED ? 0 : (
        x > MOTOR_MAX_SPEED ? MOTOR_MAX_SPEED : (
        x < -MOTOR_MAX_SPEED ? -MOTOR_MAX_SPEED : x
      ));
  y = abs(y) < MOTOR_MIN_SPEED ? 0 : (
        y > MOTOR_MAX_SPEED ? MOTOR_MAX_SPEED : (
        y < -MOTOR_MAX_SPEED ? -MOTOR_MAX_SPEED : y
      ));

  //Create output data (cal the angle & speed)
  car_data Send_Data;
  Send_Data.angle = y ? atan2(y, x) / PI * 180  //pos not on x-axis
                      : (x < 0 ? 180 : 0);
  Send_Data.speed = sqrt(sq(x)+sq(y));
  Serial.printf("[%.2f,%.2f] Speed: %d, Angle: %d\n", x, y, Send_Data.speed, Send_Data.angle);

  //send Data
  esp_err_t result = esp_now_send(sendTargetMAC, (uint8_t *)&Send_Data, sizeof(Send_Data));
  errorCount = result == ESP_OK ? 0 : errorCount + 1;
  if (!(errorCount % 100)) Serial.println(result);
}
