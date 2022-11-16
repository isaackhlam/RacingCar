// SENDER
#include <esp_now.h>
#include <WiFi.h>

// Define Pin Port and Preference
#define VRy 33
#define LEFT 12
#define RIGHT 27
#define WIFI_CHANNEL 0
#define MOTOR_MAX_SPEED 255
#define MOTOR_MIN_SPEED 60
#define INF_LOOP for (;;);

// Set send target and send data structure
esp_now_peer_info_t peerInfo;
uint8_t sendTargetMAC[] = {0xC8, 0xF0, 0x9E, 0x9C, 0x57, 0xFC};

int defaultValue = 0;

struct carData {
    int x;
    int y;
};

void OnDataSent(const uint8_t *macAddress, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS)
        return;

    Serial.print("Failed sent to device, target MAC address: ");
    for (int i = 0; i < 5; i++) {
        Serial.print(macAddress[i], HEX);
        Serial.print(":");
    }
    Serial.println(macAddress[6], HEX);
}

void setup() {
    Serial.begin(115200);
    pinMode(VRy, INPUT);
    pinMode(LEFT, INPUT);
    pinMode(RIGHT, INPUT);
    // Serve as VCC of button / joystick
    pinMode(14, OUTPUT);
    pinMode(26, OUTPUT);
    pinMode(32, OUTPUT);

    digitalWrite(14, HIGH);
    digitalWrite(26, HIGH);
    digitalWrite(32, HIGH);


    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW, Please Restart board.");
        INF_LOOP
    }

    memcpy(peerInfo.peer_addr, sendTargetMAC, 6);
    peerInfo.channel = WIFI_CHANNEL;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer, Please Restart board.");
        INF_LOOP
    }

    for (int i = 0; i < 10; i++)
        defaultValue += analogRead(VRy);

    defaultValue /= 10;

    Serial.println("Finish initialization, default joystick value: %d");
    Serial.println(defaultValue);

    // This function act as event listener
    esp_now_register_send_cb(OnDataSent);
}

void loop() {
    // map read data to -MOTOR_MAX_SPEED to MOTOR_MAX_SPEED
    int x = 0;
    int l = digitalRead(LEFT);
    int r = digitalRead(RIGHT);
    int y = map(analogRead(VRy) - defaultValue, 0 - defaultValue, 4098 - defaultValue, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);

    // Limit output range (<= 60 -> 0, >MOTOR_MAX_SPEED -> MOTOR_MAX_SPEED)
    if (abs(y) < MOTOR_MIN_SPEED)
        y = 0;
    // The button is active HIGH
    if (l == LOW && r == HIGH)
        x = -1;
    if (l == HIGH && r == LOW)
        x = 1;

    carData sendData;
    sendData.x = x;
    sendData.y = y;
    Serial.printf("X-direction: %d, Y-direction: %d\n", sendData.x, sendData.y);

    esp_err_t result = esp_now_send(sendTargetMAC, (uint8_t *)&sendData, sizeof(sendData));
}
