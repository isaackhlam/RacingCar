// RECEIVER
#include <esp_now.h>
#include <WiFi.h>

// Define Pin Port and Preference
#define MOTOR_BACK_WHEEL_PWM 5		  // PWMB
#define MOTOR_BACK_WHEEL_A 16		  // B1A
#define MOTOR_BACK_WHEEL_B 17		  // B1B
#define MOTOR_FRONT_WHEEL_GO_PWM 25	  // PWMA
#define MOTOR_FRONT_WHEEL_GO_LEFT 26  // A1A
#define MOTOR_FRONT_WHEEL_GO_RIGHT 27 // A1B
#define INF_LOOP for (;;);

struct carData {
	int x;
	int y;
};

void OnDataRecv(const uint8_t *macAddress, const uint8_t *incomingData, int len) {
	// Get data
	carData receiveData;
	memcpy(&receiveData, incomingData, sizeof(receiveData));
	// Adjust motor
	frontWheel(receiveData.x);
	backWheel(receiveData.y);
	Serial.printf("X-direction: %d, Y-direction: %d\n", receiveData.x, receiveData.y);
}

void frontWheel(int x) {
	// Set Direction (-1: Left, 0: Straight, 1: Right)
	// Motor A: Forward, Motor B: Backwards
	if (x == -1) {
		digitalWrite(MOTOR_FRONT_WHEEL_GO_PWM, HIGH);
		digitalWrite(MOTOR_FRONT_WHEEL_GO_LEFT, HIGH);
		digitalWrite(MOTOR_FRONT_WHEEL_GO_RIGHT, LOW);
	} else if (x == 1) {
		digitalWrite(MOTOR_FRONT_WHEEL_GO_PWM, HIGH);
		digitalWrite(MOTOR_FRONT_WHEEL_GO_LEFT, LOW);
		digitalWrite(MOTOR_FRONT_WHEEL_GO_RIGHT, HIGH);
	} else {
		digitalWrite(MOTOR_FRONT_WHEEL_GO_PWM, LOW);
		digitalWrite(MOTOR_FRONT_WHEEL_GO_LEFT, LOW);
		digitalWrite(MOTOR_FRONT_WHEEL_GO_RIGHT, LOW);
	}
}

void backWheel(int y) {
	// Set Direction (-1: Backwards, 0: Stop, 1: Forward)
	// Motor A: Forward Speed, Motor B: Backwards Speed
	if (y > 0) {
		analogWrite(MOTOR_BACK_WHEEL_PWM, abs(y));
		digitalWrite(MOTOR_BACK_WHEEL_A, 1);
		digitalWrite(MOTOR_BACK_WHEEL_B, 0);
	} else if (y < 0) {
		analogWrite(MOTOR_BACK_WHEEL_PWM, abs(y));
		digitalWrite(MOTOR_BACK_WHEEL_A, 0);
		digitalWrite(MOTOR_BACK_WHEEL_B, 1);
	} else{
		analogWrite(MOTOR_BACK_WHEEL_PWM, 0);
		digitalWrite(MOTOR_BACK_WHEEL_A, 0);
		digitalWrite(MOTOR_BACK_WHEEL_B, 0);
	}
}

void setup() {
	Serial.begin(115200);
	pinMode(MOTOR_BACK_WHEEL_A, OUTPUT);
	pinMode(MOTOR_BACK_WHEEL_B, OUTPUT);
	pinMode(MOTOR_BACK_WHEEL_PWM, OUTPUT);
	pinMode(MOTOR_FRONT_WHEEL_GO_LEFT, OUTPUT);
	pinMode(MOTOR_FRONT_WHEEL_GO_RIGHT, OUTPUT);
	pinMode(MOTOR_FRONT_WHEEL_GO_PWM, OUTPUT);

	WiFi.mode(WIFI_STA);
	if (esp_now_init() != ESP_OK) {
		Serial.println("Error initializing ESP-NOW, Please Restart board.");
		INF_LOOP
	}

	Serial.print("Finish initialization, local MAC: ");
	Serial.println(WiFi.macAddress());

	// The function act as Event Listener
	esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
}
