#include <WiFi.h>

const char* ssid = "SSID";
const char* password = "password";
const char* desktop_ip = "192.168.1.100";
const uint16_t port = 8888;

const int LDR = 32;	// photoresistor_pin

void setup() {
	Serial.begin(115200);
	WiFi.begin(ssid, password);
	Serial.println("Connecting...");

	while(WiFi.status() != WL_CONNECTED) {
		delay(500);
	}
	
	Serial.println("Connected to Wi-Fi");
}

void loop() {
	WiFiClient client;
	if (client.connect(host, port)) {
		int lux = analogRead(LDR);

		String msg = "ESP01," + String(millis()) + 
			     "LDR=" + String(val1) + "\n";
		client.pring(msg);
		Serial.println("Sent: " + msg);
		client.stop();
	} else{
		Serial.println("Connection failed");
	}
	delay(5000);
} 


