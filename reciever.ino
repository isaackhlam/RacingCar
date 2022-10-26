//RECIVER
#include <esp_now.h>
#include <WiFi.h>

//Define Pin Port and Preference
#define SERVO 14
#define MOTOR_BACKWHEEL_A 16 //B1A
#define MOTOR_BACKWHEEL_B 17 //B1B
#define MOTOR_FRONTWHEEL_GOLEFT 26 //A1A
#define MOTOR_FRONTWHEEL_GORIGHT 27 //A2B
#define WIFI_CHANNEL 0

//Set send target and recieve data structure
esp_now_peer_info_t peerInfo;
uint8_t sendTargetMAC[] = {0xF0, 0x08, 0xD1, 0xC7, 0xAA, 0xF8};
struct car_data {
    int speed;
    int angle;
};

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) return;
  Serial.print("Joystick Wi-Fi Not found; Local MAC: ");
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
  if(direction != 0)
    frontwheel(abs(Recieve_Data.angle));
  backwheel(direction, abs(Recieve_Data.speed));

  Serial.printf("|%d| speed: %d, angle: %d\n", direction, Recieve_Data.speed, Recieve_Data.angle);
}

void backwheel(int direction, int speed) { 
  //Set Direction (-1:Backwards, 0: Stop, 1: Foward)

  //Motor A: Forward Speed, Motor B: Backwards Speed
  Serial.printf(" {PIN:[%d - %d]} ", MOTOR_BACKWHEEL_A, direction == 1 ? speed : 0);
  analogWrite(MOTOR_BACKWHEEL_A, direction == 1 ? speed : 0);


  Serial.printf(" {PIN:[%d - %d]} ", MOTOR_BACKWHEEL_B, direction == -1 ? speed : 0);
  analogWrite(MOTOR_BACKWHEEL_B, direction == -1 ? speed : 0);
}

void frontwheel(int angle) { 
  //Set Direction (-1:Backwards, 0: Stop, 1: Foward)

  //Motor A: Forward Speed, Motor B: Backwards Speed
  Serial.printf(" {PIN:[%d - %d]} ", MOTOR_FRONTWHEEL_GOLEFT, abs(angle) > 90 ? 255 : 0);
  analogWrite(MOTOR_FRONTWHEEL_GOLEFT, abs(angle) > 90 ? 255 : 0);


  Serial.printf(" {PIN:[%d - %d]} ", MOTOR_FRONTWHEEL_GORIGHT, abs(angle) <= 90 ? 255 : 0);
  analogWrite(MOTOR_FRONTWHEEL_GORIGHT,  abs(angle) <= 90 ? 255 : 0);
}

void outputdelay(int OK, char *Output, int fenquency, int showcounter) {
  static int counter = 0;
  counter = OK ? 0 : counter + 1;
  if (!(counter % fenquency)) {
    if(showcounter) 
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
  Serial.flush();
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

  //Set Up Event Listener on Send and Recieve
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  //send device boot time
  char outputtext[30];
  int Send_Data = millis();
  sprintf(outputtext, "Send Time: %d", Send_Data);
  //outputdelay(1, outputtext, 1000, 0);

  esp_err_t result = esp_now_send(sendTargetMAC, (uint8_t *) &Send_Data, sizeof(Send_Data));
  //outputdelay(result == ESP_OK, "Error sending the data", 100000, 1);
}
