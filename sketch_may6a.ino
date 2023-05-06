#include <ESP8266WiFi.h>

// Define pins
#define DO_COIL1 5
#define DO_COIL2 4
#define DO_COIL3 0
#define DO_COIL4 2
#define PWR_MAIN 14

// Define device ID
const char* DEVICE_ID = "WaterMain";

// Define WiFi credentials
const char* ssid = "Interim";
const char* password = "BandK8*06";

// Create WiFi server
WiFiServer server(80);

void setup() {
  Serial.begin(9600);

  // Initialize digital pins
  pinMode(DO_COIL1, OUTPUT);
  pinMode(DO_COIL2, OUTPUT);
  pinMode(DO_COIL3, OUTPUT);
  pinMode(DO_COIL4, OUTPUT);
  pinMode(PWR_MAIN, OUTPUT);

  // Set all Coil pins LOW
  digitalWrite(DO_COIL1, LOW);
  digitalWrite(DO_COIL2, LOW);
  digitalWrite(DO_COIL3, LOW);
  digitalWrite(DO_COIL4, LOW);

  // Set pwr_main HIGH for 1 second
  digitalWrite(PWR_MAIN, HIGH);
  delay(1000);
  digitalWrite(PWR_MAIN, LOW);

  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // Check for new client connections
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\\n') {
          // Process received message
          if (currentLine.startsWith("ID?")) {
            client.println(DEVICE_ID);
          } else if (currentLine.startsWith("<")) {
            // Parse channel and time from command
            int channel = currentLine.charAt(1) - '0';
            int time = currentLine.substring(2, currentLine.length() - 2).toInt();

            // Turn on channel for given time
            digitalWrite(getCoilPin(channel), HIGH);
            digitalWrite(PWR_MAIN, HIGH);
            delay(500);
            digitalWrite(PWR_MAIN, LOW);
            delay(time * 1000);
            digitalWrite(getCoilPin(channel), LOW);
            digitalWrite(PWR_MAIN, HIGH);
            delay(500);
            digitalWrite(PWR_MAIN, LOW);

            // Send completion message
            client.println("Command Completed");

            // Update indicator light on Python GUI
            Serial.println(channel);
          } else {
            client.println("Error: Unknown Command");
          }
          currentLine = "";
        } else if (c != '\\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}

int getCoilPin(int channel) {
  switch (channel) {
    case 1:
      return DO_COIL1;
    case 2:
      return DO_COIL2;
    case 3:
      return DO_COIL3;
    case 4:
      return DO_COIL4;
    default:
      return -1;
  }
}
