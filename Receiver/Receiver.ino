// RECEIVER
#include <esp_now.h>
#include <WiFi.h>

// Define Pin Port and Preference
#define MOTOR_BACK_WHEEL_A 17		// B1A
#define MOTOR_BACK_WHEEL_B 16		// B1B
#define MOTOR_FRONT_WHEEL_GO_LEFT 26	// A1A
#define MOTOR_FRONT_WHEEL_GO_RIGHT 27 // A1B
#define WIFI_CHANNEL 0
#define INF_LOOP for (;;);

// Set send target and receive data structure
esp_now_peer_info_t peerInfo;
uint8_t sendTargetMAC[] = {0xF0, 0x08, 0xD1, 0xC7, 0xAA, 0xF8};

struct carData {
	int x;
	int y;
};

void OnDataRecv(const uint8_t *macAddress, const uint8_t *incomingData, int len) {
	carData receiveData;
	memcpy(&receiveData, incomingData, sizeof(receiveData));

	// Set Direction (-1: Backwards, 0: Stop, 1: Forward)
	// If Speed = 0, STOP, angle < 0 BACKWARDS, angle > 0, Forward
	frontWheel(receiveData.x);
	backWheel(receiveData.y);

	Serial.printf("X-direction: %d, Y-direction: %d\n", receiveData.x, receiveData.y);
}

// TODO: hardware limitation, need rework?
void frontWheel(int x) {
	// Motor A: Forward Speed, Motor B: Backwards Speed
	if (x == -1)
		digitalWrite(MOTOR_FRONT_WHEEL_GO_LEFT, HIGH);
	else if (x == 1)
		digitalWrite(MOTOR_FRONT_WHEEL_GO_RIGHT, HIGH);
	else {
		digitalWrite(MOTOR_FRONT_WHEEL_GO_LEFT, LOW);
		digitalWrite(MOTOR_FRONT_WHEEL_GO_RIGHT, LOW);
	}
}

void backWheel(int y) {
	// Set Direction (-1:Backwards, 0: Stop, 1: Forward)

	// Motor A: Forward Speed, Motor B: Backwards Speed
	if(y > 0)
		analogWrite(MOTOR_BACK_WHEEL_A, abs(y));
	else if(y < 0)
		analogWrite(MOTOR_BACK_WHEEL_B, abs(y));
	else{
		analogWrite(MOTOR_BACK_WHEEL_A, 0);
		analogWrite(MOTOR_BACK_WHEEL_B, 0);
	}
}

void setup() {
	Serial.begin(115200);

	pinMode(MOTOR_BACK_WHEEL_A, OUTPUT);
	pinMode(MOTOR_BACK_WHEEL_B, OUTPUT);
	pinMode(MOTOR_FRONT_WHEEL_GO_LEFT, OUTPUT);
	pinMode(MOTOR_FRONT_WHEEL_GO_RIGHT, OUTPUT);

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

	// The two function act as Event Listener
	esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
}
