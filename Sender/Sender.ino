// SENDER
#include <esp_now.h>
#include <WiFi.h>

// Define Pin Port and Preference
#define VRy 35
#define LEFT 33
#define RIGHT 34
#define WIFI_CHANNEL 0
#define MOTOR_MAX_SPEED 255
#define MOTOR_MIN_SPEED 60
#define INF_LOOP for (;;);

// Set send target and send data structure
esp_now_peer_info_t peerInfo;
uint8_t sendTargetMAC[] = {0x10, 0x52, 0x1C, 0x5C, 0xD1, 0x74};

struct carData{
    int x;
    int y;
};

void OnDataSent(const uint8_t *macAddress, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS)
        return;
    Serial.print("FAILED: ");
    Serial.println(WiFi.macAddress());
}

void setup() {
    Serial.begin(115200);
    pinMode(VRy, INPUT);
    pinMode(LEFT, INPUT);
    pinMode(RIGHT, INPUT);

    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        INF_LOOP
    }

    memcpy(peerInfo.peer_addr, sendTargetMAC, 6);
    peerInfo.channel = WIFI_CHANNEL;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        INF_LOOP
    }

    // This function act as event listener
    esp_now_register_send_cb(OnDataSent);
}

void loop() {
    // TODO: Need refactor joystick calibration and mapping
    // map read data to -MOTOR_MAX_SPEED to MOTOR_MAX_SPEED
    int y = map(analogRead(VRy) - 1975, -1960, 2160, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
    int l = digitalRead(LEFT);
    int r = digitalRead(RIGHT);
    int x = 0;

    // Limit output range (<= 60 -> 0, >MOTOR_MAX_SPEED -> MOTOR_MAX_SPEED)
    if(abs(y) < MOTOR_MIN_SPEED)
        y = 0;
    if(l == HIGH && r == LOW)
        x = -1;
    if(l == LOW && r == HIGH)
        x = 1;

    // Create output data (cal the angle & speed)
    // TODO: maybe rework calculation
    carData sendData;
    sendData.y = y;
    sendData.x = x;
    Serial.printf("X-direction: %d, Y-direction: %d\n", sendData.x, sendData.y);

    // send Data
    esp_err_t result = esp_now_send(sendTargetMAC, (uint8_t *)&sendData, sizeof(sendData));
}
