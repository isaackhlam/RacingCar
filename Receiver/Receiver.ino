//RECEIVER
#include <esp_now.h>
#include <WiFi.h>

//Define Pin Port and Preference
#define MOTOR_BACKWHEEL_A 17 //B1A
#define MOTOR_BACKWHEEL_B 16 //B1B
#define MOTOR_FRONTWHEEL_GOLEFT 26 //A1A
#define MOTOR_FRONTWHEEL_GORIGHT 27 //A1B
#define WIFI_CHANNEL 0

//Set send target and receive data structure
esp_now_peer_info_t peerInfo;
uint8_t sendTargetMAC[] = {0xF0, 0x08, 0xD1, 0xC7, 0xAA, 0xF8};
struct carData {
    int speed;
    int angle;
};
int errorCount = 0;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) return;
  Serial.print("Joystick Wi-Fi Not found; Local MAC: ");
  Serial.println(WiFi.macAddress());
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  //Get car data
  carData receiveData;
  memcpy(&receiveData, incomingData, sizeof(receiveData));

  //Set Direction (-1:Backwards, 0: Stop, 1: Forward)
  //If Speed = 0, STOP, angle < 0 BACKWARDS, angle > 0, Forward
  int direction = !receiveData.speed ? 0 : (
                    receiveData.angle > 0 ? 1 : -1
                  );
  if(receiveData.speed)
    frontWheel(abs(receiveData.angle));
  backWheel(direction, abs(receiveData.speed * sin(receiveData.angle * PI / 180)));

  Serial.printf("|%d| speed: %d, angle: %d\n", direction, receiveData.speed, receiveData.angle);
}

void backWheel(int direction, int speed) {
  //Set Direction (-1:Backwards, 0: Stop, 1: Forward)

  //Motor A: Forward Speed, Motor B: Backwards Speed
  Serial.printf(" {PIN:[%d - %d]} ", MOTOR_BACKWHEEL_A, direction == 1 ? speed : 0);
  analogWrite(MOTOR_BACKWHEEL_A, direction == 1 ? speed : 0);


  Serial.printf(" {PIN:[%d - %d]} ", MOTOR_BACKWHEEL_B, direction == -1 ? speed : 0);
  analogWrite(MOTOR_BACKWHEEL_B, direction == -1 ? speed : 0);
}

void frontWheel(int angle) {
  //Set Direction (-1:Backwards, 0: Stop, 1: Forward)

  if(80 <= abs(angle) && abs(angle) <= 100) return;

  //Motor A: Forward Speed, Motor B: Backwards Speed
  Serial.printf(" {PIN:[%d - %d]} ", MOTOR_FRONTWHEEL_GOLEFT, abs(angle) > 90 ? 255 : 0);
  analogWrite(MOTOR_FRONTWHEEL_GOLEFT, abs(angle) >= 90 ? 255 : 0);


  Serial.printf(" {PIN:[%d - %d]} ", MOTOR_FRONTWHEEL_GORIGHT, abs(angle) <= 90 ? 255 : 0);
  analogWrite(MOTOR_FRONTWHEEL_GORIGHT,  abs(angle) < 90 ? 255 : 0);
}

void outputDelay(int OK, char *Output, int frequency, int showCounter) {
  static int counter = 0;
  counter = OK ? 0 : counter + 1;
  if (!(counter % frequency)) {
    if(showCounter)
      Serial.printf("|%d| ", counter);
    Serial.println(Output);
  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  //Init Motor
  pinMode(MOTOR_BACKWHEEL_A, OUTPUT);
  pinMode(MOTOR_BACKWHEEL_B, OUTPUT);
  pinMode(MOTOR_FRONTWHEEL_GOLEFT, OUTPUT);
  pinMode(MOTOR_FRONTWHEEL_GORIGHT, OUTPUT);

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
  //send device boot time
  char outputText[30];
  int Send_Data = millis();
  sprintf(outputText, "Send Time: %d", Send_Data);
  //outputDelay(1, outputText, 1000, 0);

  esp_err_t result = esp_now_send(sendTargetMAC, (uint8_t *) &Send_Data, sizeof(Send_Data));
  errorCount = result == ESP_OK ? 0 : errorCount + 1;
  if (!(errorCount % 100)) Serial.println(result);
}
