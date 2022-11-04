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
int errorCount = 0;

struct carData {
  int speed;
  int angle;
};

void OnDataSent(const uint8_t *macAddress, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) return;
  Serial.print("FAILED: ");
  Serial.println(WiFi.macAddress());
}

//* Debug only
// void OnDataRecv(const uint8_t *macAddress, const uint8_t *incomingData, int len) {
//   int receiveData;
//   memcpy(&receiveData, incomingData, sizeof(receiveData));
//   Serial.printf("Receive Time: %d\n", receiveData);
// }

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    for(;;);
  }

  memcpy(peerInfo.peer_addr, sendTargetMAC, 6);
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    for(;;);
  }

  // This function act as event listener
  esp_now_register_send_cb(OnDataSent);
  //* debug only
  // esp_now_register_recv_cb(OnDataRecv);
}



void loop() {
  // TODO: Need refactor joystick calibration and mapping
  // map read data to -MOTOR_MAX_SPEED to MOTOR_MAX_SPEED
  double x = map(analogRead(VRx) - 1975, -1960, 2160, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
  double y = map(analogRead(VRy) - 1975, -1960, 2160, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);

  // Limit output range (<= 60 -> 0, >MOTOR_MAX_SPEED -> MOTOR_MAX_SPEED)
  x = abs(x) < MOTOR_MIN_SPEED ? 0 : (
        x > MOTOR_MAX_SPEED ? MOTOR_MAX_SPEED : (
        x < -MOTOR_MAX_SPEED ? -MOTOR_MAX_SPEED : x
      ));
  y = abs(y) < MOTOR_MIN_SPEED ? 0 : (
        y > MOTOR_MAX_SPEED ? MOTOR_MAX_SPEED : (
        y < -MOTOR_MAX_SPEED ? -MOTOR_MAX_SPEED : y
      ));

  // Create output data (cal the angle & speed)
  // TODO: maybe rework calculation
  carData sendData;
  sendData.angle = y ? atan2(y, x) * 180 / PI //pos not on x-axis
                      : (x < 0 ? 180 : 0);
  sendData.speed = sqrt(x * x + y * y);
  Serial.printf("[%.2f,%.2f] Speed: %d, Angle: %d\n", x, y, sendData.speed, sendData.angle);

  //send Data
  esp_err_t result = esp_now_send(sendTargetMAC, (uint8_t *)&sendData, sizeof(sendData));
  //* error count is for Debug
  errorCount = result == ESP_OK ? 0 : errorCount + 1;
  if (!(errorCount % 100)) Serial.println(result);
}
