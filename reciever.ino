//RECIVER
#include <esp_now.h>
#include <WiFi.h>

//Define Pin Port and Preference
#define SERVO 14
#define MOTOR_LEFT_A 26
#define MOTOR_LEFT_B 27
#define MOTOR_RIGHT_A 16
#define MOTOR_RIGHT_B 17
#define WIFI_CHANNEL 0

//Set send target and recieve data structure
uint8_t sendTargetMAC[] = {0xF0, 0x08, 0xD1, 0xC7, 0xAA, 0xF8};
struct car_data {
    int speed;
    int angle;
};

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) return;
  Serial.print("FAILED:");
  Serial.println(WiFi.macAddress());
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  //Get cardata
  car_data Recieve_Data;
  memcpy(&Recieve_Data, incomingData, sizeof(Recieve_Data));

  //Set Direction (-1:Backwards, 0: Stop, 1: Foward)
  //If Speed = 0, STOP, angle < 0 BACKWARDS, angle > 0, FOWARD
  int direction = !Recieve_Data.speed ? 0 : (
                    Recieve_Data.angle > 0 ? 1 : -1
                  );
  backwheel(direction, abs(Recieve_Data.speed));

  Serial.printf("|%d| speed: %d, angle: %d\n", direction, Recieve_Data.speed, Recieve_Data.angle);
}

void backwheel(int direction, int speed) { 
  //Set Direction (-1:Backwards, 0: Stop, 1: Foward)

  //Motor A: Forward Speed, Motor B: Backwards Speed
  analogWrite(MOTOR_LEFT_A, direction == 1 ? speed : 0);
  analogWrite(MOTOR_RIGHT_A, direction == 1 ? speed : 0);

  analogWrite(MOTOR_LEFT_B, direction == -1 ? speed : 0);
  analogWrite(MOTOR_RIGHT_B, direction == -1 ? speed : 0);
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  //Init Motor
  pinMode(MOTOR_LEFT_A, OUTPUT);
  pinMode(MOTOR_LEFT_B, OUTPUT);
  pinMode(MOTOR_RIGHT_A, OUTPUT);
  pinMode(MOTOR_RIGHT_B, OUTPUT);

  //Init Wifi
  WiFi.mode(WIFI_STA);
  Serial.flush();
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  //Create RSP Channel
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, sendTargetMAC, 6);
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  //Set Up Event Listener on Send and Recieve
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  //send device boot time
  int Send_Data = millis();
  Serial.printf("Send Time: %d\n", Send_Data);
  esp_err_t result = esp_now_send(sendTargetMAC, (uint8_t *) &Send_Data, sizeof(Send_Data));
  if (result != ESP_OK) Serial.println("Error sending the data");
}
